/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       proc.cc
 * OVERVIEW:   Implementation of the Proc hierachy (Proc, UserProc, LibProc).
 *             All aspects of a procedure, apart from the actual code in the
 *             Cfg, are stored here
 *
 * Copyright (C) 1997-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision$
 *
 * 14 Mar 02 - Mike: Fixed a problem caused with 16-bit pushes in richards2
 * 20 Apr 02 - Mike: Mods for boomerang
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <sstream>
#include <algorithm>        // For find()
#include "exp.h"
#include "cfg.h"
#include "register.h"
#include "type.h"
#include "rtl.h"
#include "proc.h"
#include "prog.h"
#include "dataflow.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "util.h"
#include "signature.h"

/************************
 * Proc methods.
 ***********************/

Proc::~Proc()
{}

/*==============================================================================
 * FUNCTION:        Proc::Proc
 * OVERVIEW:        Constructor with name, native address.
 * PARAMETERS:      uNative - Native address of entry point of procedure
 * RETURNS:         <nothing>
 *============================================================================*/
Proc::Proc(Prog *prog, ADDRESS uNative, Signature *sig)
     : prog(prog), address(uNative), signature(sig), m_firstCaller(NULL), 
       bytesPopped(0)
{
}

/*==============================================================================
 * FUNCTION:        Proc::getName
 * OVERVIEW:        Returns the name of this procedure
 * PARAMETERS:      <none>
 * RETURNS:         the name of this procedure
 *============================================================================*/
const char* Proc::getName()
{
	assert(signature);
    return signature->getName();
}

/*==============================================================================
 * FUNCTION:        Proc::setName
 * OVERVIEW:        Sets the name of this procedure
 * PARAMETERS:      new name
 * RETURNS:         <nothing>
 *============================================================================*/
void Proc::setName(const char *nam)
{
	assert(signature);
	signature->setName(nam);
}


/*==============================================================================
 * FUNCTION:        Proc::getNativeAddress
 * OVERVIEW:        Get the native address (entry point).
 * PARAMETERS:      <none>
 * RETURNS:         the native address of this procedure (entry point)
 *============================================================================*/
ADDRESS Proc::getNativeAddress()
{
    return address;
}

void Proc::setNativeAddress(ADDRESS a)
{
	address = a;
}

void Proc::setBytesPopped(int n)
{
	if (bytesPopped == 0) {
		bytesPopped = n;
	}
	assert(bytesPopped == n);
}

/*==============================================================================
 * FUNCTION:      Proc::containsAddr
 * OVERVIEW:      Return true if this procedure contains the given address
 * PARAMETERS:    address
 * RETURNS:       true if it does
 *============================================================================*/
bool UserProc::containsAddr(ADDRESS uAddr)
{
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it))
		if (bb->getRTLs() && bb->getLowAddr() <= uAddr && bb->getHiAddr() >= uAddr)
			return true;	
	return false;
}

/*==============================================================================
 * FUNCTION:        operator<<
 * OVERVIEW:        Output operator for a Proc object.
 * PARAMETERS:      os - output stream
 *                  proc -
 * RETURNS:         os
 *============================================================================*/
std::ostream& operator<<(std::ostream& os, Proc& proc)
{
    return proc.put(os);
}

/*==============================================================================
 * FUNCTION:       Proc::matchParams
 * OVERVIEW:       Adjust the given list of potential actual parameter
 *                   locations that are live at a call to this procedure to
 *                   match the formal parameters of this procedure.
 * NOTE:           This was previously a virtual function, implemented
 *                  separately for LibProc and UserProc
 * PARAMETERS:     actuals - an ordered list of locations of actual parameters
 *                 caller - Proc object for calling procedure (for message)
 *                 outgoing - ref to Parameters object which encapsulates the
 *                   PARAMETERS CALLER section of the .pal file
 * RETURNS:        <nothing>, but may add or delete elements from actuals
 *============================================================================*/
#if 0       // FIXME: Need to think about whether we have a Parameters class
bool isInt(const Exp* ss) {
    assert(ss->getOper() == opTypedExp);
    return ((TypedExp*)ss)->getType().getType() == INTEGER;}
bool isFlt(const Exp* ss) {
    assert(ss->getOper() == opTypedExp);
    Type& ty = ((TypedExp*)ss)->getType();
    return (ty.getType() == FLOATP) && (ty.getSize() == 32);}
bool isDbl(const Exp* ss) {
    assert(ss->getOper() == opTypedExp);
    Type& ty = ((TypedExp*)ss)->getType();
    return (ty.getType() == FLOATP) && (ty.getSize() == 64);}

void Proc::matchParams(std::list<Exp*>& actuals, UserProc& caller,
    const Parameters& outgoing) const
{
    int intSize = outgoing.getIntSize();    // Int size for the source machine

    int currSlot = -1;              // Current parameter slot number
    int currSize = 1;               // Size of current parameter, in slots
    int ordParam = 1;               // Param ordinal number (first=1, for msg)
    std::list<Exp*>::const_iterator it = parameters.begin();
    std::list<Exp*>::iterator ita = actuals.begin();
#if 0           // I believe this should be done later - MVE
    if (isAggregateUsed()) {
        // Need to match the aggregate parameter separately, before the main
        // loop
        if (ita == actuals.end())
            insertParams(1, actuals, ita, name, outgoing);
        else ita++;
        assert(it != parameters.end());
        it++;
        ordParam++;
    }
#endif
    // Loop through each formal parameter. There should be no gaps in the formal
    // parameters, because that's the job of missingParamCheck()
    int firstOff;
    for (; it != parameters.end(); it++) {
        // If the current formal is varargs, then leave the remaining actuals
        // as they are
        const Type& ty = it->getType();
        if (ty.getType() == VARARGS) return;

        // Note that we can't call outgoing.getParamSlot here because these are
        // *formal* parameters (could be different locations to outgoing params)
        // (Besides, it could be a library function with no parameter locations)
        currSlot += currSize;
        // Perform alignment, if needed. Note that it's OK to use the outgoing
        // parameters, as we assume that the alignment is the same for incoming
        outgoing.alignSlotNumber(currSlot, ty);
        currSize = ty.getSize() / 8 / intSize;  // Current size in slots
        // Ensure that small types still occupy one slot minimum
        if (currSize == 0) currSize = 1;
//cout << "matchParams: Proc " << name << ": formal " << *it << ", actual "; if (ita != actuals.end()) cout << *ita; cout << std::endl;  // HACK
        // We need to find the subset of actuals with the same slot number
        std::list<Exp*>::iterator itst = ita;      // Remember start of this set
        int numAct = 0;                         // The count of this set
        int actSlot, actSize = 0, nextActSlot;
        if (ita != actuals.end()) {
            actSize = 1;            // Example: int const 0
            nextActSlot = actSlot = outgoing.getParamSlot(*ita, actSize,
                ita == actuals.begin(), firstOff);
            ita++;
            numAct = 1;
        }
        while (ita != actuals.end()) {
            nextActSlot = outgoing.getParamSlot(*ita, actSize, false, firstOff);
            if (actSlot != nextActSlot) break;
            numAct++;
            ita++;
        }
        // if (actSize == 0) this means that we have run out of actual
        // parameters. If (currSlot < actSlot) it means that there is a gap
        // in the actual parameters. Either way, we need to insert one of the
        // dreaded "hidden" (actual)parameters appropriate to the formal
        // parameter (in size and type).
        if ((actSize == 0) || (currSlot < actSlot)) {
            const Exp** newActual = outgoing.getActParamLoc(ty, currSlot);
            actuals.insert(itst, *newActual);
            ita = itst;             // Still need to deal with this actual
            std::ostringstream ost;
            ost << "adding hidden parameter " << *newActual << 
              " to call to " << name;
            warning(str(ost));
            delete newActual;
            continue;               // Move to the next formal parameter
        }
        if (numAct > 1) {
            // This means that there are several actual parameters to choose
            // from, which all have the same slot number. This can happen in
            // architectures like pa-risc, where different registers are used
            // for different types of parameters, and they all could be live

            // The rules depend on the basic type. Integer parameters can
            // overlap (e.g. 68K, often pass one long to cover two shorts).
            // This doesn't happen with floats, because values don't concaten-
            // ate the same way. So the size can be used to choose the right
            // floating point location (e.g. pa-risc)
            std::list<Exp*>::iterator ch;  // Iterator to chosen item in actuals
            if (!it->getType()->isFloat())
                // Integer, pointer, etc. For now, assume all the same
                ch = find_if(itst, ita, isInt);
            else {
                int size = it->getType().getSize();
                if (size == 32)
                    ch = find_if(itst, ita, isFlt);
                else if (size == 64)
                    ch = find_if(itst, ita, isDbl);
                else assert(0);
            }
            if (ch == ita) {
                std::ostringstream ost;
                ost << "Parameter " << dec << ordParam << " of proc " << name <<
                  " has no actual parameter of suitable type (slot " <<
                  currSlot << ")";
                error(str(ost));
            } else {
                // Eliminate all entries in actuals from itst up to but not
                // including ita, except the ch one
                // In other words, of all the actual parameter witht the same
                // slot number, keep only ch
                for (; itst != ita; itst++)
                    if (itst != ch)
                        actuals.erase(itst);
            }
        }

        // Check that the sizes at least are compatible
        // For example, sometimes 2 ints are passed for a formal double or long
        if (currSize > actSize) {
            // Check for the 2 int case. itst would point to the first, and
            // ita (if not end) points to the second
            if ((actSize == 1) && (currSize == 2) && (ita != actuals.end()) &&
              (ita->getType().getSize() == itst->getType().getSize())) {
                // Let this through, by just skipping the second int
                // It's up to the back end to cope with this situation
                ita++;
            }
        }

        ordParam++;
    }
    // At this point, any excess actuals can be discarded
    actuals.erase(ita, actuals.end());
}
#endif

#if 0       // FIXME: Again, Parameters object used
/*==============================================================================
 * FUNCTION:        Proc::getParamTypeList
 * OVERVIEW:        Given a list of actual parameters, return a list of
 *                    Type objects representing the types that the actuals
 *                    need to be "cast to"
 * NOTE:            Have to take into account longs overlapping 2 shorts,
 *                    gaps for alignment, etc etc.
 * NOTE:            Caller must delete result
 * PARAMETERS:      actuals: list of actual parameters
 * RETURNS:         Ptr to a list of Types, same size as actuals
 *============================================================================*/
std::list<Type>* Proc::getParamTypeList(const std::list<Exp*>& actuals)
{
    std::list<Type>* result = new std::list<Type>;
    const Parameters& outgoing = prog.csrSrc.getOutgoingParamSpec();
    int intSize = outgoing.getIntSize();    // Int size for the source machine

    int currForSlot = -1;               // Current formal parameter slot number
    int currForSize = 1;                // Size of current formal, in slots
    int ordParam = 1;          // Actual param ordinal number (first=1, for msg)
    std::list<Exp*>::const_iterator it = parameters.begin();
    std::list<Exp*>::const_iterator ita = actuals.begin();
    std::list<Exp*>::const_iterator itaa;
    if (isAggregateUsed()) {
        // The first parameter is a DATA_ADDRESS
        result->push_back(Type(DATA_ADDRESS));
        if (it != parameters.end()) it++;
        if (ita != actuals.end()) ita++;
    }
    int firstOff;
    for (; it != parameters.end(); it++) {
        if (ita == actuals.end())
            // Run out of actual parameters. Can happen with varargs
            break;
        currForSlot += currForSize;
        // Perform alignment, if needed. Note that it's OK to use the outgoing
        // parameters, as we assume that the alignment is the same for incoming
        Type ty = it->getType();
        outgoing.alignSlotNumber(currForSlot, ty);
        currForSize = ty.getSize() / 8 / intSize;  // Current size in slots
        // Ensure that small types still occupy one slot minimum
        if (currForSize == 0) currForSize = 1;
        int actSize = 1;        // Default to 1 (e.g. int consts)
        // Look at the current actual parameter, to get its size
        if (ita->getFirstIdx() == idVar) {
            // Just use the size from the Exp*'s Type
            int bytes = ita->getType().getSize() / 8;
            if (bytes && (bytes < intSize)) {
                std::ostringstream ost;
                ost << "getParamTypelist: one of those evil sub-integer "
                    "parameter passings at call to " << name;
                warning(str(ost));
                actSize = 1;
            }
            else
                actSize = bytes / intSize;
        } else {
            // MVE: not sure that this is the best way to find the size
            outgoing.getParamSlot(*ita, actSize, ita == actuals.begin(),
              firstOff);
        }
        ita++;
        // If the current formal is varargs, that's a special case
        // Similarly, if all the arguments are unknown
        /*LOC_TYPE lt = ty.getType();
        if ((lt == VARARGS) || (lt == UNKNOWN)) {
            // We want to give all the remaining actuals their own type
            ita--;
            while (ita != actuals.end()) {
                result->push_back(ita->getType());
                ita++;
            }
            break;
        } */
        // If the sizes are the same, then we can use the formal's type
        if (currForSize == actSize)
            result->push_back(ty);
        // Else there is an overlap. We get the type of the first formal,
        // and widen it for the number of formals that this actual covers
        else if (actSize > currForSize) {
            Type first = ty;
            int combinedSize = ty.getSize();
            while ((actSize > currForSize) && (it != parameters.end())) {
                currForSlot += currForSize;
                ty = (++it)->getType();
                outgoing.alignSlotNumber(currForSlot, ty);
                currForSize += ty.getSize() / 8 / intSize;
                combinedSize += ty.getSize();
            }
            if (actSize != currForSize) {
                // Something has gone wrong with the matching process
                std::ostringstream ost;
                ost << "getParamTypeList: Actual parameter " << dec << ordParam
                  << " does not match with formals in proc " << name;
                error(str(ost));
            }
            first.setSize(combinedSize);
            result->push_back(first);
        }
        // Could be overlapping parameters, e.g. two ints passed as a
        // double or long. ita points to the second int (unless end)
        else if ((actSize == 1) && (currForSize == 2) && (ita != actuals.end())
          && (itaa = ita, (*--itaa).getType() == ita->getType())) {
            // Let this through, with the type of the formal
            ita++;
            ordParam++;
            result->push_back(ty);
        }
        else {
            assert(actSize > currForSize);
        }
        ordParam++;
    }
    return result;
}
#endif

Prog *Proc::getProg()
{
	return prog;
}

Proc *Proc::getFirstCaller()
{ 
	if (m_firstCaller == NULL && m_firstCallerAddr != NO_ADDRESS) {
		m_firstCaller = prog->findProc(m_firstCallerAddr);
		m_firstCallerAddr = NO_ADDRESS;
	}

	return m_firstCaller; 
}

Signature *Proc::getSignature()
{
	assert(signature);
	return signature;
}

// deserialize a procedure
Proc *Proc::deserialize(Prog *prog, std::istream &inf)
{
	/*
	 * These values are ordered in the save file because I think they are concrete 
	 * and necessary to create the specific subclass of Proc.  This is the only
	 * time that values should be ordered (instead of named) in the save file (I hope).	 
	 * - trent 17/6/2002
	 */
	char type;
	loadValue(inf, type, false);
	assert(type == 0 || type == 1);

	std::string nam;	
	loadString(inf, nam);
	ADDRESS uAddr;
	loadValue(inf, uAddr, false);

	Proc *p = NULL;
	if (type == 0)
		p = new LibProc(prog, nam, uAddr);
	else
		p = new UserProc(prog, nam, uAddr);
	assert(p);

	int fid;
	while ((fid = loadFID(inf)) != -1 && fid != FID_PROC_END)
		p->deserialize_fid(inf, fid);
	assert(loadLen(inf) == 0);

	return p;
}

bool Proc::deserialize_fid(std::istream &inf, int fid)
{
	switch(fid) {
		case FID_PROC_SIGNATURE:
			{
				int len = loadLen(inf);
				std::streampos pos = inf.tellg();
				signature = Signature::deserialize(inf);
				assert(signature);
				assert((int)(inf.tellg() - pos) == len);
			}
			break;
		case FID_PROC_FIRSTCALLER:
			loadValue(inf, m_firstCallerAddr);
			break;
		default:
			skipFID(inf, fid);
			return false;
	}

	return true;
}

/**********************
 * LibProc methods.
 *********************/

/*==============================================================================
 * FUNCTION:        LibProc::LibProc
 * OVERVIEW:        Constructor with name, native address.
 * PARAMETERS:      name - Name of procedure
 *                  uNative - Native address of entry point of procedure
 * RETURNS:         <nothing>
 *============================================================================*/
LibProc::LibProc(Prog *prog, std::string& name, ADDRESS uNative) : 
	Proc(prog, uNative, NULL)
{
    signature = prog->pFE->getLibSignature(name.c_str());
}

LibProc::~LibProc()
{
}

// serialize this procedure
bool LibProc::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	char type = 0;
	saveValue(ouf, type, false);
	saveValue(ouf, address, false);

	if (signature) {
		saveFID(ouf, FID_PROC_SIGNATURE);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		assert(signature->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		assert((int)(now - posa) == len);
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	if (m_firstCaller) {
		saveFID(ouf, FID_PROC_FIRSTCALLER);
		saveValue(ouf, m_firstCaller->getNativeAddress());
	}

	saveFID(ouf, FID_PROC_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

// deserialize the rest of this procedure
bool LibProc::deserialize_fid(std::istream &inf, int fid)
{
	switch (fid) {
		default:
			return Proc::deserialize_fid(inf, fid);
	}

	return true;
}


/*==============================================================================
 * FUNCTION:        LibProc::put
 * OVERVIEW:        Display on os.
 * PARAMETERS:      os -
 * RETURNS:         os
 *============================================================================*/
std::ostream& LibProc::put(std::ostream& os)
{
    os << "library procedure `" << signature->getName() << "' resides at 0x";
    return os << std::hex << address << std::endl;
}

/**********************
 * UserProc methods.
 *********************/

/*==============================================================================
 * FUNCTION:        UserProc::UserProc
 * OVERVIEW:        Constructor with name, native address.
 * PARAMETERS:      name - Name of procedure
 *                  uNative - Native address of entry point of procedure
 * RETURNS:         <nothing>
 *============================================================================*/
UserProc::UserProc(Prog *prog, std::string& name, ADDRESS uNative) :
	Proc(prog, uNative, new Signature(name.c_str())), 
	cfg(new Cfg()), decoded(false),
    	returnIsSet(false), isSymbolic(false), uniqueID(0)
{
    cfg->setProc(this);              // Initialise cfg.myProc
}

UserProc::~UserProc()
{
	if (cfg)
		delete cfg;	
}

/*==============================================================================
 * FUNCTION:        UserProc::isDecoded
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
bool UserProc::isDecoded()
{
    return decoded;
}

/*==============================================================================
 * FUNCTION:        UserProc::put
 * OVERVIEW:        Display on os.
 * PARAMETERS:      os -
 * RETURNS:         os
 *============================================================================*/
std::ostream& UserProc::put(std::ostream& os)
{
    os << "user procedure `" << signature->getName() << "' resides at 0x";
    return os << std::hex << address << std::endl;
}

#if 0       // Need toSymbolic
/*==============================================================================
 * FUNCTION:      UserProc::setParams
 * OVERVIEW:      Sets the parameters that have been recovered for this
 *                  procedure through CSR analysis.
 * PARAMETERS:    params - the list of locations used for the parameters
 *                aggUsed - true if the aggregate pointer location is used
 * RETURNS:       <nothing>
 *============================================================================*/
void UserProc::setParams(std::list<TypedExp*>& params, bool aggUsed /* = false */)
{
    parameters.clear();         // Could be called many times
    for (std::list<TypedExp*>::iterator it = params.begin(); it != params.end();it++)
    {
        TypedExp* symbolic;
        toSymbolic(*it, symbolic, false);
        parameters.push_back(symbolic);
        // Under some conditions, parameters are discovered after locals have
        // been created for the same location. That causes a local variable to
        // be delcared in the .c file, shadowing the parameter. So we check for
        // the parameter already being a local variable; if so, the local is
        // deleted
        // Note: can't use find() because we need a special equality for Exp*s
        std::vector<TypedExp*>::iterator ll;
        for (ll = locals.begin(); ll != locals.end(); ll++) {
            if (**ll == *symbolic) {
                delete *ll;
                locals.erase(ll);
                break;
            }
        }
    }
    aggregateUsed = aggUsed;
}
#endif

#if 0       // Was used for some weird CSR purpose
/*==============================================================================
 * FUNCTION:      UserProc::getParamSet
 * OVERVIEW:      Gets the parameters that have been recovered for this
 *                  procedure through CSR analysis, to a set of Semantic Strings
 *                  (not symbolic)
 * PARAMETERS:    <none>
 * RETURNS:       Type insensitive set of Exp*
 *============================================================================*/
setTiExp*& UserProc::getParamSet()
{
    setTiExp*& ret = *new setTiExp*;
    map<Exp*, Exp*>::iterator it;
    for (it = symbolMap.begin(); it != symbolMap.end(); it++) {
        ret.insert(it->first);
    }
    return ret;
}
#endif

#if 0
/*==============================================================================
 * FUNCTION:      UserProc::setPrologue
 * OVERVIEW:      Set the prologue of this procedure.
 * PARAMETERS:    prologue - a caller prologue
 * RETURNS:       <nothing>
 *============================================================================*/
void UserProc::setPrologue(CalleePrologue* prologue)
{
    if (this->prologue != NULL) {
        std::ostringstream ost;
        ost << "attempt to set prologue `" << prologue->getName()
          << "' for proc " << name << "' which already has one: `"
          << this->prologue->getName() << "'";
        if (prologue->getName() == this->prologue->getName()) {
            // This is a warning, not an error, because of the common case of
            // Sparc's same_reg_win, where the epilogue is the same as the
            // prologue, except for the sign of the operand, e.g.
            // add          %sp, -104, %sp      // Prologue
            // ...
            // add          %sp, +104, %sp      // Epilogue
            warning(str(ost));
        } else {
            // Assume that different logue names is always bad
            error(str(ost));
        }
    }
    else
        // Note: don't overwrite prologues. For example, the same_reg_win
        // case as above. If a prologue comes part way through a proc, it is
        // set then (there is no "default" prologue)
        this->prologue = prologue;
}

/*==============================================================================
 * FUNCTION:      UserProc::setEpilogue
 * OVERVIEW:      Set the epilogue of this procedure.
 * PARAMETERS:    epilogue - a caller epilogue
 * RETURNS:       <nothing>
 *============================================================================*/
void UserProc::setEpilogue(CalleeEpilogue* epilogue)
{
    // Only set the given epilogue to be the epilogue of this procedure if it
    // doesn't currently have one of the one it does have comes after the given
    // epilogue in an oredring between epilogues.
    if (this->epilogue == NULL ||
        this->epilogue->getOrder() > epilogue->getOrder())
        this->epilogue = epilogue;
}

/*==============================================================================
 * FUNCTION:      UserProc::getPrologue
 * OVERVIEW:      Get the prologue (if any) of this procedure.
 * PARAMETERS:    <none>
 * RETURNS:       a callee prologue
 *============================================================================*/
CalleePrologue* UserProc::getPrologue()
{
    return this->prologue;
}

/*==============================================================================
 * FUNCTION:      UserProc::getEpilogue
 * OVERVIEW:      Get the epilogue (if any) of this procedure.
 * PARAMETERS:    <none>
 * RETURNS:       a callee epilogue
 *============================================================================*/
CalleeEpilogue* UserProc::getEpilogue()
{
    return this->epilogue;
}
#endif

#if 0           // Needs getLocalsSize
/*==============================================================================
 * FUNCTION:      UserProc::printLocalsAsC
 * OVERVIEW:      Print the locals declaration in C style. This
 *                includes declarations for the block of memory set
 *                aside for local varaiables and the abstract frame
 *                pointer used to index into this block.
 *                Also delcares the symbolic locations (v[0]..v[n-1])
 * NOTE:          FIXME: C specific
 * PARAMETERS:    os - the output stream to use
 * RETURNS:       <nothing>
 *============================================================================*/
void UserProc::printLocalsAsC(std::ostream& os)
{
    // the block of memory for locals
    if (getLocalsSize() != 0) {

        os << "char _locals[" << dec << getLocalsSize() << "];\n";
    }

    // If this is main, and the analysis decided that there were more than
    // two parameters, declare them here so that at least the output will
    // compile
    if ((name == "main") && (parameters.size() > 2)) {
        std::list<TypedExp*>::iterator it = parameters.begin();
        for (it++, it++; it != parameters.end(); it++) {
            os << (*it)->getType().getCtype();      // Type, e.g. int32
            os << " ";
            (*it)->printAsHL(os);                    // Name, e.g. v2 or r8
            os << ";";
        }
        os << "\t/* Dummy parameters */\n";
    }

    // Declare the symbolic locations (v[0] etc)
    for (std::vector<TypedExp*>::iterator it = locals.begin();
      it != locals.end(); it++) {
        os << "\t" << (*it)->getType().getCtype();  // Type, e.g. int32
        os << " ";
        (*it)->printAsHL(os);                        // Name, e.g. v2 or r8
        os << ";\n";
    }
}
#endif

/*==============================================================================
 * FUNCTION:       UserProc::toSymbolic
 * OVERVIEW:       Given a machine dependent location, return a generated
 *                 symbolic representation for it.
 * NOTE:           loc will occasionally be of the forms
 *                  trunc(m[%afp - 20] >> 16, 32, 16) or
 *                  trunc(m[%afp - 20] & 0xFFFF, 16, 32)
 *                  must now cope with these
 * NOTE ALSO:      The fixComplex logic (overlapping parameters) should be
 *                  done in matchParameters, as one case already has
 * PARAMETERS:     loc - a machine dependent location
 *                 result - the symbolic representation for loc
 *                 local: if true, add this symbol to the vector of locals
 *                  result a copy of loc if the mapping isn't there)
 * RETURNS:        <nothing> (but parameter result is set)
 *============================================================================*/
// Simple procedure to effectively substitute result into loc if a complex
// location
// Example: loc = trunc(m[%afp - 20] & 0xFFFF, 32, 16), result = v3; then
// result changed to be trunc(v3 & 0xFFFF, 32, 16)
// Assumes 4 tokens at start, and 2 at end, to be transferred
// FIXME: This is really the most awful hack. Perhaps substitution using
// searchReplace() would be better?
#if 0
void fixComplex(Exp* loc, Exp* result)
{
    for (int i=4-1; i >= 0; i--)
        result.prep(loc.getIndex(i));
    for (int i1=0; i1 < 2; i1++)
        result.push(loc.getIndex(i1));
    // Now a different size; same size as the location we started with
    // Example: passed m[%afp-8]>>16 as 16 bits; result (v1) wass 32 bits;
    // now result (v1>>16) is back to 16 bits again
    result.setTypes(loc);
}

void UserProc::toSymbolic(TypedExp* loc, TypedExp* result,
    bool local /*= true*/)
{
    int idx = loc.getFirstIdx();
    if (idx == idIntConst) {
        // Occasionally pass constants now
        result = loc;      // Symbolic representation is itself
        return;
    }

    // loc2 is the expression to be converted to symbolic form. For simple
    // cases, loc2 == loc. For complex cases, loc2 is the first subexpression
    // of loc, and is converted to symbolic form. The result is effectively
    // substituted into loc
    bool complex = false;
    Exp* loc2 = loc;
    if (!loc.isRegOf() &&
        !loc.isMemOf() &&
        !loc.isVar()) {
            complex = true;
            Exp* tmp = ((Unary*)loc2)->getSubExp1();
            loc2 = tmp->getSubExpr(0, loc2);
            delete tmp;
            // Set the size of the subexpression to double what we are passed.
            // E.g. v1 is twice the size of (v1 >> 16)
            loc2.getType().setSize(loc.getType().getSize() * 2);
    }
            
    
    idx = loc2.getFirstIdx();
    assert(idx == idRegOf || idx == idMemOf || idx == idVar);

    if (idx == idVar) {
        if (find(locals.begin(), locals.end(), loc2) == locals.end()) {
            std::ostringstream ost;
            ost << "`" << loc << "' should already be in the set of locals";
            error(str(ost));
        }
        result = loc2;        // Symbolic representation is itself
        return;
    }

    map<Exp*,Exp*>::iterator it = symbolMap.find(loc2);
    if (it != symbolMap.end()) {
        result = it->second;
        if (complex) fixComplex(loc, result);
//        if (loc.getFirstIdx() == idRegOf) result = loc;   // HACK
        return;
    }
    // Else does not exist in the map
    if (isSymbolic) {
        // Not allowed to add a new (because we have already called
        // propagateSymbolics), and no existing. Return with result = loc
        result = loc;
        return;
    }

    // Don't convert r[] to symbolic (v[]); this would interfere with the
    // overlapping code logic
    if (loc.getFirstIdx() == idRegOf) {
        result = loc;
        return;           // Never add to locals
    }
    else {
        result.clear();
        result.push(idVar);
        result.push(uniqueID);
        if (complex)
            // Overlapping parameters. The var is twice the size of the location
            // that we are passed. E.g. v1 is twice the size of (v1 >> 16)
            result.getType().setSize(loc.getType().getSize() * 2);
        else
            result.setTypes(loc);  // Same size as location it represents
        uniqueID++;
        // Add a new entry
        symbolMap[loc2] = result;
    }

    // Add this to the locals if necessary.
    if (local)
        locals.push_back(result);

    if (complex) fixComplex(loc, result);
    return;
}
#endif

/*==============================================================================
 * FUNCTION:       UserProc::newLocal
 * OVERVIEW:       Return the next available local variable.
 * NOTE:           Caller is responsible for deleting this new Exp*
 * PARAMETERS:     ty: a Type for the local variable
 * RETURNS:        Pointer to the Exp representing the local
 *============================================================================*/
TypedExp* UserProc::newLocal(Type* ty)
{
    TypedExp* result = new TypedExp(ty, new Unary(opVar, new Const(uniqueID)));
    uniqueID++;

    locals.push_back(result);

    return result;
}

#if 0       // We will need this or something like it soon
/*==============================================================================
 * FUNCTION:      UserProc::propagateSymbolics
 * OVERVIEW:      Replace each instance of a location in this procedure with its
 *                  symbolic representation if it has one.
 *                  Also handle expressions taking the addresses of these
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void UserProc::propagateSymbolics()
{
    // First, do a pass that checks the sizes of memory references
    checkMemSizes();
    for (map<Exp*,Exp*>::iterator it = symbolMap.begin();
      it != symbolMap.end(); it++) {

        Exp*& search = (Exp*&)it->first;
        Exp*& replace = it->second;

        // Go through the RTLs. For now, we clear the "type sensitive" flag,
        // so that when floating point parameters are passed through memory,
        // they get substituted properly. NOTE: this will break Palm programs,
        // which have stack locations of different sizes needing different
        // symbolic variables! (But casting the parameters may fix this)
// Don't convert expressions with r[] to v[]
//if (search.getFirstIdx() != idRegOf)      // HACK
            cfg->searchAndReplace(search, replace, false);

        // Also remember to fix code taking addresses of these parameters
        // Example: addressparam test. Knock the memOf off the front of the
        // search, and prepend an idAddrOf to the replacement
        if (search.getFirstIdx() == idMemOf) {
            Exp** srch2 = search.getSubExpr(0);
            Exp* repl2(replace);
            repl2.prep(idAddrOf);
            cfg->searchAndReplace(*srch2, repl2, false);
            // Also check if sizeof(search) > sizeof(int).
            // The size of an int varies with the source machine. We use
            // the OFFSET value from the PAL file
            int intSize = prog.csrSrc.getIntSize();
            int size = getVarType(it->second.getSecondIdx()).getSize();
            if (size > intSize*8) {
                //  If so, also do a replacement of m[original+4] by
                // *((int*)&replace+1)
                // i.e. m[ + original int 4   by   m[ + (int*) & replace int 1
                // repl2 is already &replace
//cout << "Search " << search << " and replace " << replace << std::endl;  // HACK
                repl2.push(idIntConst); repl2.push(1);
                repl2.prep(idCastIntStar);
                repl2.prep(idPlus); repl2.prep(idMemOf);
                // *srch2 already has replace without the m[
                srch2->prep(idPlus); srch2->prep(idMemOf);
                srch2->push(idIntConst); srch2->push(4);
                srch2->simplify();
//cout << "Replacing " << *srch2 << " with " << repl2 << std::endl;    // HACK
                cfg->searchAndReplace(*srch2, repl2, false);
            }
            delete srch2;
        }
    }
    // Remember that this has been done
    isSymbolic = true;
}
#endif

/*==============================================================================
 * FUNCTION:        UserProc::getCFG
 * OVERVIEW:        Returns a pointer to the CFG.
 * PARAMETERS:      <none>
 * RETURNS:         a pointer to the CFG
 *============================================================================*/
Cfg* UserProc::getCFG()
{
    return cfg;
}

/*==============================================================================
 * FUNCTION:        UserProc::deleteCFG
 * OVERVIEW:        Deletes the whole CFG for this proc object. Also clears the
 *                  cfg pointer, to prevent strange errors after this is called
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::deleteCFG()
{
    delete cfg;
    cfg = NULL;
}

#if 0           // This should be done by some sort of max stack depth thing
/*==============================================================================
 * FUNCTION:        UserProc::getLocalsSize
 * OVERVIEW:        Sets the number of bytes allocated for locals on
 *                  the stack.
 * PARAMETERS:      <none>
 * RETURNS:         the number of bytes allocated for locals on
 *                  the stack
 *============================================================================*/
int UserProc::getLocalsSize()
{
    if (prologue != NULL)
        return prologue->getLocalsSize();
    else
        return 0;
}
#endif

/*==============================================================================
 * FUNCTION:    Proc::getFirstLocalIndex()
 * OVERVIEW:    Return the index of the first symbolic local declared.
 * PARAMETERS:  None
 * RETURNS:     An integer value of the first symbolic local declared. For e.g
                variable v12, it returns 12. If no locals, returns -1.
 *============================================================================*/
int UserProc::getFirstLocalIndex()
{
    std::vector<TypedExp*>::iterator it = locals.begin();
    if (it == locals.end()) {
        return -1;
    }
    return (*it)->getVarIndex();
}

#if 0       // This will work when all Exp's have types
/*==============================================================================
 * FUNCTION:    Proc::getLastLocalIndex()
 * OVERVIEW:    Return the index of the last symbolic local declared.
 * PARAMETERS:  None
 * RETURNS:     An integer value of the first symbolic local declared. For e.g
                variable v12, it returns 12. If no locals, returns -1.
 *============================================================================*/
int UserProc::getLastLocalIndex()
{
    std::vector<TypedExp*>::iterator it = locals.end(); // just after end
    if (it == locals.begin()) { // must be empty
        return -1;
    }
    it--;			// point to last element
    return it->getSecondIdx();
}
#endif

/*==============================================================================
 * FUNCTION:    UserProc::getSymbolicLocals()
 * OVERVIEW:    Return the list of symbolic locals for the procedure.
 * PARAMETERS:  None
 * RETURNS:     A reference to the list of the procedure's symbolic locals.
 *============================================================================*/
std::vector<TypedExp*>& UserProc::getSymbolicLocals()
{
    return locals;
}

/*==============================================================================
 * FUNCTION:        UserProc::setDecoded
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
void UserProc::setDecoded()
{
    decoded = true;
}

/*==============================================================================
 * FUNCTION:        UserProc::unDecode
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
void UserProc::unDecode()
{
	cfg->clear();
    decoded = false;
}

/*==============================================================================
 * FUNCTION:      UserProc::subAXP
 * OVERVIEW:      Given a map from registers to expressions, follow
 *                the control flow of the CFG replacing every use of a
 *                register in this map with the corresponding
 *                expression. Then for every definition of such a
 *                register, update its expression to be the RHS of
 *                the definition after the first type of substitution
 *                has been performed and remove that definition from
 *                the CFG.
 * PARAMETERS:    subMap - a map from register to expressions
 * RETURNS:       <nothing>
 *============================================================================*/
void UserProc::subAXP(std::map<Exp*,Exp*>& subMap)
{
    cfg->subAXP(subMap);
}

/*==============================================================================
 * FUNCTION:    UserProc::getEntryBB
 * OVERVIEW:    Get the BB with the entry point address for this procedure
 * PARAMETERS:  
 * RETURNS:     Pointer to the entry point BB, or NULL if not found
 *============================================================================*/
PBB UserProc::getEntryBB()
{
    return cfg->getEntryBB();
}

/*==============================================================================
 * FUNCTION:        UserProc::setEntryBB
 * OVERVIEW:        Set the entry BB for this procedure
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::setEntryBB()
{
    std::list<PBB>::iterator bbit;
    PBB pBB = cfg->getFirstBB(bbit);        // Get an iterator to the first BB
    // Usually, but not always, this will be the first BB, or at least in the
    // first few
    while (pBB && address != pBB->getLowAddr()) {
        pBB = cfg->getNextBB(bbit);
    }
    cfg->setEntryBB(pBB);
}

/*==============================================================================
 * FUNCTION:        UserProc::getCallees
 * OVERVIEW:        Get the set of callees (procedures called by this proc)
 * PARAMETERS:      <none>
 * RETURNS:         Constant reference to the set
 *============================================================================*/
std::set<Proc*>& UserProc::getCallees()
{
	if (calleeAddrSet.begin() != calleeAddrSet.end()) {
		for (std::set<ADDRESS>::iterator it = calleeAddrSet.begin(); it != calleeAddrSet.end(); it++) {
			Proc *p = prog->findProc(*it);
			if (p)
				calleeSet.insert(p);
		}
		calleeAddrSet.clear();
	}
    return calleeSet;
}

/*==============================================================================
 * FUNCTION:        UserProc::setCallee
 * OVERVIEW:        Add this callee to the set of callees for this proc
 * PARAMETERS:      A pointer to the Proc object for the callee
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::setCallee(Proc* callee)
{
    calleeSet.insert(callee);
}

// serialize this procedure
bool UserProc::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	char type = 1;
	saveValue(ouf, type, false);
	saveValue(ouf, address, false);

	if (signature) {
		saveFID(ouf, FID_PROC_SIGNATURE);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		assert(signature->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		assert((int)(now - posa) == len);
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	saveFID(ouf, FID_PROC_DECODED);
	saveValue(ouf, decoded);

	if (cfg) {
		saveFID(ouf, FID_CFG);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		assert(cfg->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		assert((int)(now - posa) == len);
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	if (m_firstCaller) {
		saveFID(ouf, FID_PROC_FIRSTCALLER);
		saveValue(ouf, m_firstCaller->getNativeAddress());
	}

	for (std::set<Proc *>::iterator it = calleeSet.begin(); it != calleeSet.end(); it++) {
		saveFID(ouf, FID_PROC_CALLEE);
		saveValue(ouf, (*it)->getNativeAddress());
	}

	for (std::map<std::string, TypedExp *>::iterator its = symbols.begin(); its != symbols.end(); its++) {
		saveFID(ouf, FID_PROC_SYMBOL);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		saveString(ouf, (*its).first);
		assert((*its).second->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		len = now - posa;
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	saveFID(ouf, FID_PROC_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool UserProc::deserialize_fid(std::istream &inf, int fid)
{
	ADDRESS a;

	switch (fid) {
		case FID_PROC_DECODED:
			loadValue(inf, decoded);
			break;
		case FID_CFG:
			{
				int len = loadLen(inf);
				std::streampos pos = inf.tellg();
				assert(cfg);
				assert(cfg->deserialize(inf));
				assert((int)(inf.tellg() - pos) == len);
			}
			break;
		case FID_PROC_SYMBOL:
			{
				int len = loadLen(inf);
				std::streampos pos = inf.tellg();
				std::string s;
				loadString(inf, s);
				Exp *e = Exp::deserialize(inf);
				assert(e->getOper() == opTypedExp);
				symbols[s] = (TypedExp*)e;
				assert((int)(inf.tellg() - pos) == len);
			}
			break;
		case FID_PROC_CALLEE:
			loadValue(inf, a);
			calleeAddrSet.insert(a);
			break;
		default:
			return Proc::deserialize_fid(inf, fid);
	}

	return true;
}

#if 0
/*==============================================================================
 * FUNCTION:        UserProc::addProcCoverage
 * OVERVIEW:        Add this proc's coverage to the program's coverage
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::addProcCoverage()
{
    prog.cover.addRanges(cover);
}

/*==============================================================================
 * FUNCTION:      UserProc::findVarEntry
 * OVERVIEW:      Return a pointer to the given var, i.e. if 2 is passed,
 *                  then on exit, points to the Exp* for v2
 * NOTE:          Private function; only used by get/setVarType
 * PARAMETERS:    idx - index of the variable, e.g. 2 for v2
 * RETURNS:       A pointer to the entry, as above, or NULL if not found
 *============================================================================*/
Exp** UserProc::findVarEntry(int idx)
{
    // The locals are sorted, so we can do a binary search of them
    int min = 0; int max = locals.size()-1;
    int i;
    if (max >= 0) {                  // Do nothing if no locals
        while (min <= max) {
            i = (min + max) >> 1;
            // We are only interested in vars (v[]). Some locals can now be
            // registers (e.g. Pentium function returning in r24)
            if (locals[i].getFirstIdx() != idVar) {
                // Assume that there are only r[] and v[], and that
                // idRegOf < idVar, so we need to move up the map
                min = i+1;
                continue;
            }
            int c = locals[i].getSecondIdx();
            if (c == idx) {
                return &locals[i];
            }
            else if (c > idx)
                max = i-1;
            else /* if (c < idx) */
                min = i+1;
        }
    }

    // Also search the parameters. Assume a linear search is quicker, for
    // the likely very small number of parameters
    std::list<Exp*>::iterator pp;
    for (pp = parameters.begin(); pp != parameters.end(); pp++) {
        if (pp->getFirstIdx() != idVar)
            // Could be a r[] now
            continue;
        if (pp->getSecondIdx() == idx) {
            return &(*pp);
        }
    }

    // Not found in locals or parameters
    std::ostringstream ost;
    ost << "Could not find v" << idx <<
        " in locals or prarameters for procedure " << name;
    error(str(ost));
    return NULL;
}

/*==============================================================================
 * FUNCTION:      UserProc::getVarType
 * OVERVIEW:      Return the type of the given variable
 * PARAMETERS:    idx - index of the variable, e.g. 2 for v2
 * RETURNS:       The Type
 *============================================================================*/
Type UserProc::getVarType(int idx)
{
    Exp* pss = findVarEntry(idx);
    if (pss == NULL)
        return Type();
    return pss->getType();
}

/*==============================================================================
 * FUNCTION:      UserProc::setVarSize
 * OVERVIEW:      Change the size of the given var (i.e. the size it will be
 *                  declared as)
 * PARAMETERS:    idx - the var number (e.g. 2 for v2)
 *                size - the new size
 * RETURNS:       Nothing
 *============================================================================*/
void UserProc::setVarSize(int idx, int size)
{
    Exp** pss = findVarEntry(idx);
    if (pss == NULL) return;
    Type ty = pss->getType();
    ty.setSize(size);
    pss->setType(ty);
}
#endif


#if 0
/*==============================================================================
 * FUNCTION:        UserProc:checkReturnPass
 * OVERVIEW:        Check if this return location is "passed through" this
 *                    function to one of its callees. For example in the
 *                    returncallee test, main uses the return value from add4,
 *                    and this use is "passed on" to add2, since add4 doesn't
 *                    define the return location after the call to add2
 * PARAMETERS:      returnLocBit - the bit for the location used by my caller
 *                  returnLoc - the Location used (as a Exp*)
 *                  retLocations - information about where types are returned
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::checkReturnPass(int returnLocBit, TypedExp* returnLoc)
{
    // We are looking for a path from a call BB to a return BB where no
    // BB (after the call) defines the return location
    std::set<HLCall*>& calls = cfg->getCalls();
    std::set<PBB> seen;          // Set of out edges already checked
    for (std::set<HLCall*>::iterator it = calls.begin(); it != calls.end(); it++) {
        if ((*it)->getReturnLoc() != 0)
            // This call already has a return location - no need to check it
            // or its callee
            continue;
        const PBB pBB = (*it)->getBB();
        checkReturnPassBB(pBB, *it, returnLocBit, returnLoc, seen);
    }
}

/*==============================================================================
 * FUNCTION:        UserProc:checkReturnPassBB
 * OVERVIEW:        Do the main work of checking for return locations being
 *                    "passed through" (see above)
 * PARAMETERS:      pBB - pointer to the current BB which is being checked
 *                  pCall - pointer to the HLCall RTL containing the call
 *                  seen - set of out edges already checked
 *                  others as above
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::checkReturnPassBB(const PBB pBB, HLCall* pCall, int returnLocBit,
    TypedExp* returnLoc, std::set<PBB>& seen)
{
    // We have a call BB. Check all possible out edges (beware of
    // loops!) till a return location is found.
    std::vector<PBB>::const_iterator ito;
    const std::vector<PBB>& outs = pBB->getOutEdges();
    for (ito = outs.begin(); ito != outs.end(); ito++) {
        if (seen.find(*ito) != seen.end())
            // Already checked this edge
            continue;
        seen.insert(*ito);
        if ((*ito)->isDefined(returnLocBit))
            // The return location is defined, killing the path from
            // the call to the return location
            continue;
        if ((*ito)->getType() == RET) {
            // This is what we are looking for! This callee also
            // returns to this location
            ADDRESS uDest = pCall->getFixedDest();
            UserProc* callee = (UserProc*)prog.findProc(uDest);
            if (callee && (int)callee != -1 && !callee->isLib())
                callee->setReturnType(returnLoc, retLocations);
            // We must also set the return location for the call (so that the
            // back end assigns the result of the call).
            if (isSymbolic) {
                // Use the symbolic version if the proc has already called
                // propagateSymbolic()
                Exp* symbolic;
                toSymbolic(returnLoc, symbolic);
                pCall->setReturnLoc(symbolic);
            }
            else pCall->setReturnLoc(returnLoc);
            // No further paths need be investigated
            return;
        }
        else {
            // Recurse through this BB's out edges
            const std::vector<PBB>& grandChildren = (*ito)->getOutEdges();
            std::vector<PBB>::const_iterator itg;
            for (itg = grandChildren.begin(); itg != grandChildren.end();
              itg++) {
                checkReturnPassBB(*itg, pCall, returnLocBit, returnLoc,
                    retLocations, seen);
            }
            
        }
    }
}
#endif

#if 0
/*==============================================================================
 * FUNCTION:        checkMemSize
 * OVERVIEW:        Search this ss, checking for sizes of vars, ensuring
 *                    that the largest size used in the proc is used
 * PARAMETERS:      ss - pointer to Exp* to be checked
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::checkMemSize(Exp** ss)
{
    Exp* memX;            // Wildcard memory
    memX.push(idMemOf); memX.push(-1);
    std::list<Exp**> result;
    if (ss->searchAll(memX, result)) {
        // We have at least one mem; go through the list
        std::list<Exp**>::iterator it;
        for (it=result.begin(); it != result.end(); it++) {
            // Find out what var this would be converted to, if any
            std::map<Exp*, Exp*>::iterator mm;
            mm = symbolMap.find(**it);
            if (mm == symbolMap.end()) continue;
            int vNum = (*mm).second.getSecondIdx();
            // Find out what size this memory is used as
            int size = (*it)->getType().getSize();
            if (size > getVarType(vNum).getSize()) {
                // Change to the larger size
                setVarSize(vNum, size);
                // If there is a mapping for the "other half", then delete it
                // so it will only be satisfied by special logic in
                // propagateSymbolics()
                Exp* memOf(**it);
                // Assume m[ + something int  K ]
                //        0  1               last
                if (memOf.getSecondIdx() == idPlus) {
                    int last = memOf.len() - 1;
                    int K = memOf.getIndex(last);
                    int intSize = prog.csrSrc.getIntSize();
                    memOf.substIndex(last, K + intSize);
                    mm = symbolMap.find(memOf);
                    if (mm != symbolMap.end())
                        symbolMap.erase(mm);
                }
            }
        }
    }
}

/*==============================================================================
 * FUNCTION:        checkMemSizes
 * OVERVIEW:        Loop through all BBs, checking for sizes of memory that
 *                    will soon be converted to vars, ensuring
 *                    that the largest size used in the proc is used for all
 * PARAMETERS:      None
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::checkMemSizes()
{
    std::list<PBB>::iterator it;
    PBB pBB = cfg->getFirstBB(it);
    while (pBB) {
        RTLList* pRtls = pBB->getRTLs();
        if (pRtls) {
            RTLList_IT rit;
            for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
                int n = (*rit)->getNumRT();
                for (int i=0; i<n; i++) {
                    RTAssgn* rt = (RTAssgn*)(*rit)->elementAt(i);
                    if (rt->getKind() != RTASSGN) continue;
                    checkMemSize(rt->getLHS());
                    checkMemSize(rt->getRHS());
                }
            }
        }
        pBB = cfg->getNextBB(it);
    }

}
#endif

bool UserProc::findSymbolFor(Exp *e, std::string &sym, TypedExp* &sym_exp)
{
	Exp *e1 = e;
	if (e->getOper() == opTypedExp)
		e1 = e->getSubExp1();
	for (std::map<std::string, TypedExp *>::iterator it = symbols.begin(); it != symbols.end(); it++) {
		assert((*it).second);
		if (*(*it).second->getSubExp1() == *e1)
		{
			sym = (*it).first;
			sym_exp = (*it).second;
			return true;
		}
	}
	return prog->findSymbolFor(e, sym, sym_exp);
}

bool UserProc::generateCode(HLLCode &hll)
{
	assert(cfg);
	cfg->establishDFTOrder();
	cfg->establishRevDFTOrder();
	assert(getEntryBB());

	cfg->unTraverse();
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		bb->setLabelNeeded(false);
	}
	getEntryBB()->generateCode(hll, NULL);

	// generate any BBs that are left
	bool change = true;
	while (change) {
		change = false;
		for (PBB left = cfg->getFirstBB(it); left; left = cfg->getNextBB(it)) 
			if (!left->isTraversed()) {
				left->generateCode(hll, NULL);
				change = true;
				break;
			}
	}

	return true;
}

bool UserProc::isSSAForm()
{
	DefSet defs;
	// TODO: add params to defs
	return cfg->getSSADefs(defs);
}

void UserProc::transformToSSAForm()
{
	DefSet defs;
	// TODO: add params to defs
	cfg->SSATransform(defs);
	// minimize the SSA form
	do cfg->simplify();
	while (cfg->minimizeSSAForm());	
}

void UserProc::transformFromSSAForm()
{
	cfg->revSSATransform();
}

void UserProc::removeUselessCode()
{
	DefSet defs;

	bool change = true;
	while (change) {
		change = false;
		defs.clear();
		assert(cfg->getSSADefs(defs));
		for (DefSet::iterator it = defs.begin(); it != defs.end(); it++) {
			Def &d = *it;
			UseSet uses;
			cfg->getAllUses(d.getLeft(), uses);
			if (uses.empty()) {
				d.remove();
				change = true;
			}
		}
	}

}

void UserProc::promoteSignature()
{
	signature = signature->promote(this);
}
