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
 * 31 Jan 03 - Mike: Tabs and indenting
 * 03 Feb 03 - Mike: removeStatement no longer linear searches for the BB
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
#include "statement.h"
#include "exp.h"
#include "cfg.h"
#include "register.h"
#include "type.h"
#include "rtl.h"
#include "proc.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "util.h"
#include "signature.h"
#include "hllcode.h"
#include "boomerang.h"

typedef std::map<Statement*, int> RefCounter;

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
{}

/*==============================================================================
 * FUNCTION:        Proc::getName
 * OVERVIEW:        Returns the name of this procedure
 * PARAMETERS:      <none>
 * RETURNS:         the name of this procedure
 *============================================================================*/
const char* Proc::getName() {
    assert(signature);
    return signature->getName();
}

/*==============================================================================
 * FUNCTION:        Proc::setName
 * OVERVIEW:        Sets the name of this procedure
 * PARAMETERS:      new name
 * RETURNS:         <nothing>
 *============================================================================*/
void Proc::setName(const char *nam) {
    assert(signature);
    signature->setName(nam);
}


/*==============================================================================
 * FUNCTION:        Proc::getNativeAddress
 * OVERVIEW:        Get the native address (entry point).
 * PARAMETERS:      <none>
 * RETURNS:         the native address of this procedure (entry point)
 *============================================================================*/
ADDRESS Proc::getNativeAddress() {
    return address;
}

void Proc::setNativeAddress(ADDRESS a) {
    address = a;
}

void Proc::setBytesPopped(int n) {
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
bool UserProc::containsAddr(ADDRESS uAddr) {
    BB_IT it;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it))
        if (bb->getRTLs() && bb->getLowAddr() <= uAddr &&
          bb->getHiAddr() >= uAddr)
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
std::ostream& operator<<(std::ostream& os, Proc& proc) {
    return proc.put(os);
}

/*==============================================================================
 * FUNCTION:       Proc::matchParams
 * OVERVIEW:       Adjust the given list of potential actual parameter
 *                   locations that reach a call to this procedure to
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
    const Parameters& outgoing) const {
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
std::list<Type>* Proc::getParamTypeList(const std::list<Exp*>& actuals) {
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

Prog *Proc::getProg() {
    return prog;
}

Proc *Proc::getFirstCaller() { 
    if (m_firstCaller == NULL && m_firstCallerAddr != NO_ADDRESS) {
        m_firstCaller = prog->findProc(m_firstCallerAddr);
        m_firstCallerAddr = NO_ADDRESS;
    }

    return m_firstCaller; 
}

Signature *Proc::getSignature() {
    assert(signature);
    return signature;
}

// deserialize a procedure
Proc *Proc::deserialize(Prog *prog, std::istream &inf) {
    /*
     * These values are ordered in the save file because I think they are
     * concrete and necessary to create the specific subclass of Proc.
     * This is the only time that values should be ordered (instead of named)
     * in the save file (I hope).  
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

bool Proc::deserialize_fid(std::istream &inf, int fid) {
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

Exp *Proc::getProven(Exp *left)
{
    for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); 
         it != proven.end(); it++) 
        if (*(*it)->getSubExp1() == *left)
            return (*it)->getSubExp2();
    // not found, try the signature
    return signature->getProven(left);
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
    Proc(prog, uNative, NULL) {
    signature = prog->getLibSignature(name.c_str());
}

LibProc::~LibProc()
{}

// serialize this procedure
bool LibProc::serialize(std::ostream &ouf, int &len) {
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
bool LibProc::deserialize_fid(std::istream &inf, int fid) {
    switch (fid) {
        default:
            return Proc::deserialize_fid(inf, fid);
    }

    return true;
}

void LibProc::getInternalStatements(StatementList &internal) {
     signature->getInternalStatements(internal);
}

/*==============================================================================
 * FUNCTION:        LibProc::put
 * OVERVIEW:        Display on os.
 * PARAMETERS:      os -
 * RETURNS:         os
 *============================================================================*/
std::ostream& LibProc::put(std::ostream& os) {
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
    cfg(new Cfg()), decoded(false), isSymbolic(false), uniqueID(0),
    decompileSeen(false), decompiled(false), isRecursive(false) {
    cfg->setProc(this);              // Initialise cfg.myProc
}

UserProc::~UserProc() {
    if (cfg)
        delete cfg; 
}

/*==============================================================================
 * FUNCTION:        UserProc::isDecoded
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
bool UserProc::isDecoded() {
    return decoded;
}

/*==============================================================================
 * FUNCTION:        UserProc::put
 * OVERVIEW:        Display on os.
 * PARAMETERS:      os -
 * RETURNS:         os
 *============================================================================*/
std::ostream& UserProc::put(std::ostream& os) {
    os << "user procedure `" << signature->getName() << "' resides at 0x";
    return os << std::hex << address << std::endl;
}

/*==============================================================================
 * FUNCTION:        UserProc::getCFG
 * OVERVIEW:        Returns a pointer to the CFG.
 * PARAMETERS:      <none>
 * RETURNS:         a pointer to the CFG
 *============================================================================*/
Cfg* UserProc::getCFG() {
    return cfg;
}

/*==============================================================================
 * FUNCTION:        UserProc::deleteCFG
 * OVERVIEW:        Deletes the whole CFG for this proc object. Also clears the
 *                  cfg pointer, to prevent strange errors after this is called
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::deleteCFG() {
    delete cfg;
    cfg = NULL;
}

/*==============================================================================
 * FUNCTION:        UserProc::setDecoded
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
void UserProc::setDecoded() {
    decoded = true;
}

/*==============================================================================
 * FUNCTION:        UserProc::unDecode
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
void UserProc::unDecode() {
    cfg->clear();
    decoded = false;
}

/*==============================================================================
 * FUNCTION:    UserProc::getEntryBB
 * OVERVIEW:    Get the BB with the entry point address for this procedure
 * PARAMETERS:  
 * RETURNS:     Pointer to the entry point BB, or NULL if not found
 *============================================================================*/
PBB UserProc::getEntryBB() {
    return cfg->getEntryBB();
}

/*==============================================================================
 * FUNCTION:        UserProc::setEntryBB
 * OVERVIEW:        Set the entry BB for this procedure
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::setEntryBB() {
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
std::set<Proc*>& UserProc::getCallees() {
    if (calleeAddrSet.begin() != calleeAddrSet.end()) {
        for (std::set<ADDRESS>::iterator it = calleeAddrSet.begin();
          it != calleeAddrSet.end(); it++) {
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
void UserProc::setCallee(Proc* callee) {
    calleeSet.insert(callee);
}

// serialize this procedure
bool UserProc::serialize(std::ostream &ouf, int &len) {
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

    for (std::set<Proc *>::iterator it = calleeSet.begin();
      it != calleeSet.end(); it++) {
        saveFID(ouf, FID_PROC_CALLEE);
        saveValue(ouf, (*it)->getNativeAddress());
    }

    saveFID(ouf, FID_PROC_END);
    saveLen(ouf, 0);

    len = ouf.tellp() - st;
    return true;
}

bool UserProc::deserialize_fid(std::istream &inf, int fid) {
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
        case FID_PROC_CALLEE:
            loadValue(inf, a);
            calleeAddrSet.insert(a);
            break;
        default:
            return Proc::deserialize_fid(inf, fid);
    }

    return true;
}

void UserProc::generateCode(HLLCode *hll) {
    assert(cfg);
    assert(getEntryBB());

    cfg->structure();
    replaceExpressionsWithGlobals();
    if (!Boomerang::get()->noLocals) {
        while (nameStackLocations())
            replaceExpressionsWithSymbols();
        while (nameRegisters())
            replaceExpressionsWithSymbols();
    }
    if (VERBOSE || Boomerang::get()->printRtl)
        print(std::cerr);

    hll->AddProcStart(signature);
    
    for (std::map<std::string, Type*>::iterator it = locals.begin();
         it != locals.end(); it++)
        hll->AddLocal((*it).first.c_str(), (*it).second);

    std::list<PBB> followSet, gotoSet;
    getEntryBB()->generateCode(hll, 1, NULL, followSet, gotoSet);
    
    hll->AddProcEnd();
  
    if (!Boomerang::get()->noRemoveLabels)
        cfg->removeUnneededLabels(hll);
}

// print this userproc, maining for debugging
void UserProc::print(std::ostream &out, bool withDF) {
    signature->print(out);
    cfg->print(out, withDF);
    out << "\n";
}

// initialise all statements
void UserProc::initStatements() {
    BB_IT it;
    BasicBlock::rtlit rit; stmtlistIt sit;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        for (Statement* s = bb->getFirstStmt(rit, sit); s;
              s = bb->getNextStmt(rit, sit)) {
            s->setProc(this);
            s->setBB(bb);
            CallStatement* call = dynamic_cast<CallStatement*>(s);
            if (call) {
                call->setSigArguments();
            }
            ReturnStatement *ret = dynamic_cast<ReturnStatement*>(s);
            if (ret) {
                ret->setSigArguments();
                returnStatements.push_back(ret);
            }
        }
    }
}

void UserProc::numberStatements(int& stmtNum) {
    BB_IT it;
    BasicBlock::rtlit rit; stmtlistIt sit;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        for (Statement* s = bb->getFirstStmt(rit, sit); s;
          s = bb->getNextStmt(rit, sit))
            s->setNumber(++stmtNum);
    }
}

void UserProc::numberPhiStatements(int& stmtNum) {
    BB_IT it;
    BasicBlock::rtlit rit; stmtlistIt sit;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        for (Statement* s = bb->getFirstStmt(rit, sit); s;
             s = bb->getNextStmt(rit, sit))
            if (s->isPhi() && s->getNumber() == 0)
                s->setNumber(++stmtNum);
    }
}


// get all statements
// Get to a statement list, so they come out in a reasonable and consistent
// order
void UserProc::getStatements(StatementList &stmts) {
    BB_IT it;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        std::list<RTL*> *rtls = bb->getRTLs();
        for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end();
          rit++) {
            RTL *rtl = *rit;
            for (std::list<Statement*>::iterator it = rtl->getList().begin(); 
              it != rtl->getList().end(); it++) {
                stmts.append(*it);
            }
        }
    }
}

// Remove a statement. This is somewhat inefficient - we have to search the
// whole BB for the statement. Should use iterators or other context
// to find out how to erase "in place" (without having to linearly search)
void UserProc::removeStatement(Statement *stmt) {
    // remove from BB/RTL
    PBB bb = stmt->getBB();         // Get our enclosing BB
    std::list<RTL*> *rtls = bb->getRTLs();
    for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end();
          rit++) {
        std::list<Statement*>& stmts = (*rit)->getList();
        for (std::list<Statement*>::iterator it = stmts.begin(); 
              it != stmts.end(); it++) {
            if (*it == stmt) {
                stmts.erase(it);
                return;
            }
        }
    }
}

void UserProc::insertAssignAfter(Statement* s, int tempNum, Exp* right) {
    PBB bb = s->getBB();         // Get our enclosing BB
    std::list<RTL*> *rtls = bb->getRTLs();
    for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end();
          rit++) {
        std::list<Statement*>& stmts = (*rit)->getList();
        for (std::list<Statement*>::iterator it = stmts.begin(); 
              it != stmts.end(); it++) {
            if (*it == s) {
                std::ostringstream os;
                os << "local" << tempNum;
                Assign* as = new Assign(
                    new Unary(opLocal,
                        new Const(strdup(os.str().c_str()))),
                    right);
                stmts.insert(++it, as);
                return;
            }
        }
    }
    assert(0);
}


// Decompile this UserProc
std::set<UserProc*>* UserProc::decompile() {
    // Prevent infinite loops when there are cycles in the call graph
    if (decompiled) return NULL;

    // We have seen this proc
    decompileSeen = true;


    std::set<UserProc*>* cycleSet = new std::set<UserProc*>;
    BB_IT it;
    // Look at each call, to perform a depth first search.
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        if (bb->getType() == CALL) {
            // The call Statement will be in the last RTL in this BB
            CallStatement* call = (CallStatement*)bb->getRTLs()->back()->
              getHlStmt();
            UserProc* destProc = (UserProc*)call->getDestProc();
            if (destProc->isLib()) continue;
            if (destProc->decompileSeen && !destProc->decompiled)
                // We have discovered a cycle in the call graph
                cycleSet->insert(destProc);
                // Don't recurse into the loop
            else {
                // Recurse to this child (in the call graph)
                std::set<UserProc*>* childSet = destProc->decompile();
                // Union this child's set into cycleSet
                if (childSet)
                    cycleSet->insert(childSet->begin(), childSet->end());
            }
        }
    }

    isRecursive = cycleSet->size() != 0;
    // Remove self from the cycle list
    cycleSet->erase(this);

    if (Boomerang::get()->noDecompileUp) {
        decompiled = true;
        return cycleSet;
    }

    if (VERBOSE) {
        std::cerr << "decompiling: " << getName() << std::endl;
    }


    // Sort by address, so printouts make sense
    cfg->sortByAddress();
    // Initialise statements
    initStatements();

    // Compute dominance frontier
    cfg->dominators();


    // Number the statements
    int stmtNumber = 0;
    numberStatements(stmtNumber); 

    // For each memory depth
    int maxDepth = findMaxDepth() + 1;
    if (Boomerang::get()->maxMemDepth < maxDepth)
        maxDepth = Boomerang::get()->maxMemDepth;
    for (int depth = 0; depth <= maxDepth; depth++) {

        // Place the phi functions for this memory depth
        cfg->placePhiFunctions(depth, this);

        // Number them
        numberPhiStatements(stmtNumber);

        // Rename variables
        cfg->renameBlockVars(0, depth);

        // Print if requested
        if (Boomerang::get()->debugPrintSSA) {
            std::cerr << "=== Debug Print SSA for " << getName()
              << " at memory depth " << depth << " (no propagations) ===\n";
            print(std::cerr, true);
            std::cerr << "=== End Debug Print SSA for " <<
              getName() << " at depth " << depth << " ===\n\n";
        }
        
        if (depth == 0) {
            trimReturns();
        }
        if (depth == maxDepth) {
            processConstants();
            removeRedundantPhis();
        }
        addNewParameters();
        cfg->renameBlockVars(0, depth);
        trimParameters();

        // Print if requested
        if (Boomerang::get()->debugPrintSSA && depth == 0) {
            std::cerr << "=== Debug Print SSA for " << getName()
              << " at memory depth " << depth << " (after trimming return set) ===\n";
            print(std::cerr, true);
            std::cerr << "=== End Debug Print SSA for " <<
              getName() << " at depth " << depth << " ===\n\n";
        }

         // Propagate at this memory depth
        propagateStatements(depth);
        if (VERBOSE) {
            std::cerr << "=== After propagate for " << getName() <<
              " at memory depth " << depth << " ===\n";
            print(std::cerr, true);
            std::cerr << "=== End propagate for " << getName() <<
              " at depth " << depth << " ===\n\n";
        }

        // Remove unused statements
        RefCounter refCounts;           // The map
        // Count the references first
        countRefs(refCounts);
        // Now remove any that have no used
        if (!Boomerang::get()->noRemoveNull)
            removeUnusedStatements(refCounts, depth);

        // Remove null statements
        if (!Boomerang::get()->noRemoveNull)
            removeNullStatements();

        if (VERBOSE && !Boomerang::get()->noRemoveNull) {
            std::cerr << "===== After removing null and unused statements "
              "=====\n";
            print(std::cerr, true);
            std::cerr << "===== End after removing unused "
              "statements =====\n\n";
        }
    }

    if (!Boomerang::get()->noParameterNames) {
        for (int i = maxDepth; i >= 0; i--) {
            replaceExpressionsWithParameters(i);
            replaceExpressionsWithLocals();
            trimReturns();
            trimParameters();
        }
        if (VERBOSE) {
            std::cerr << "===== After replacing params =====\n";
            print(std::cerr, true);
            std::cerr << "===== End after replacing params =====\n\n";
        }
    }

    fromSSAform();

    if (Boomerang::get()->vFlag) {
        std::cerr << "===== After transformation from SSA form =====\n";
        print(std::cerr, true);
        std::cerr << "===== End after transformation from SSA =====\n\n";
    }

    decompiled = true;          // Now fully decompiled
    return cycleSet;
}

void UserProc::complete() {
    cfg->compressCfg();
    processConstants();

    // Convert the signature object to one of a derived class, e.g.
    // SparcSignature.
//    if (!Boomerang::get()->noPromote)
//        promoteSignature();    // No longer needed?
    // simplify the procedure (currently just to remove a[m['s)
    // Not now! I think maybe only pa/risc needs this, and it nobbles
    // the a[m[xx]] that processConstants() does (just above)
    // If needed, move this to after m[xxx] are converted to variables
//    simplify();

}

int UserProc::findMaxDepth() {
    StatementList stmts;
    getStatements(stmts);
    StmtListIter it;
    int maxDepth = 0;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        // Assume only need to check assignments
        if (s->getKind() == STMT_ASSIGN) {
            int depth = ((Assign*)s)->getMemDepth();
            maxDepth = std::max(maxDepth, depth);
        }
    }
    return maxDepth;
}

void UserProc::removeRedundantPhis()
{
    if (VERBOSE)
        std::cerr << "removing redundant phi statements" << std::endl;

    // some phis are just not used
    RefCounter refCounts;
    countRefs(refCounts);

    StatementList stmts;
    getStatements(stmts);
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        if (s->isPhi() && refCounts[s] == 0) { 
            if (VERBOSE)
                std::cerr << "removing unused statement " << s << std::endl;
            removeStatement(s);
        }

    stmts.clear();
    getStatements(stmts);

    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        if (s->isPhi()) {
            if (VERBOSE)
                std::cerr << "checking " << s << std::endl;
            // if we can prove that all the statements in the phi define
            // equal values then we can replace the phi with any one of 
            // the values, but there's not much point if they're all calls
            PhiExp *p = (PhiExp*)s->getRight();
            StmtVecIter it;
            bool allsame = true;
            Statement *s1 = p->getFirstRef(it);  
            Statement *noncall = s1;
            if (!p->isLastRef(it))
                for (Statement *s2 = p->getNextRef(it); !p->isLastRef(it); 
                     s2 = p->getNextRef(it)) {
                    if (noncall && noncall->isCall() && s2 && !s2->isCall())
                        noncall = s2;
                    Exp *e = new Binary(opEquals, 
                                 new RefExp(s->getLeft()->clone(), s1),
                                 new RefExp(s->getLeft()->clone(), s2));
                    if (!prove(e)) {
                        allsame = false; break;
                    }
                }
            if (allsame && (noncall == NULL || !noncall->isCall())) {
                s->searchAndReplace(s->getRight(), 
                   new RefExp(s->getLeft(), noncall));
            }
        }
}

void UserProc::trimReturns() {
    std::set<Exp*> preserved;
    bool stdsp = false;

    if (VERBOSE)
        std::cerr << "Trimming return set for " << getName() << std::endl;

    for (int n = 0; n < 2; n++) {   
        // may need to do multiple times due to dependencies

        // Special case for 32-bit stack-based machines (e.g. Pentium).
        // RISC machines generally preserve the stack pointer (so special
        // case required)
        if (VERBOSE)
            std::cerr << "attempting to prove sp = sp + 4 for " << getName() <<
              std::endl;
        int sp = signature->getStackRegister(prog);
        stdsp = prove(new Binary(opEquals,
                      Unary::regOf(sp),
                      new Binary(opPlus,
                          Unary::regOf(sp),
                          new Const(4))));

        // prove preservation for each parameter
        for (int i = 0; i < signature->getNumReturns(); i++) {
            Exp *p = signature->getReturnExp(i);
            Exp *e = new Binary(opEquals, p->clone(), p->clone());
            if (VERBOSE)
                std::cerr << "attempting to prove " << p << " is preserved by " 
                          << getName() << std::endl;
            if (prove(e)) {
                preserved.insert(p);    
            }
        }
    }
    if (stdsp)
        removeReturn(new Unary(opRegOf, new Const(28)));
    for (std::set<Exp*>::iterator it = preserved.begin(); 
         it != preserved.end(); it++)
        removeReturn(*it);
    removeRedundantPhis();
    StatementList stmts;
    getStatements(stmts);
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        s->fixCallRefs();
}

void UserProc::addNewParameters() {

    if (VERBOSE)
        std::cerr << "Adding new parameters for " << getName() << std::endl;

    StatementList stmts;
    getStatements(stmts);

    StmtListIter it;
    RefExp *r = new RefExp(new Unary(opMemOf, new Terminal(opWild)), NULL);
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        Exp *result;
        if (s->search(r, result)) {
            bool allZero;
            Exp *e = result->clone()->removeSubscripts(allZero);
            if (allZero && signature->findParam(e) == -1) {
                int sp = signature->getStackRegister(prog);
                if (e->isMemOf() && e->getSubExp1()->getOper() == opMinus &&
                    *e->getSubExp1()->getSubExp1() == *Unary::regOf(sp) &&
                    e->getSubExp1()->getSubExp2()->isIntConst()) {
                    if (VERBOSE)
                        std::cerr << "ignoring local " << e << std::endl;
                    continue;
                }
                if (VERBOSE)
                    std::cerr << "Found new parameter " << e << std::endl;
                addParameter(e);
            }
        }
    }
}

void UserProc::trimParameters() {

    if (VERBOSE)
        std::cerr << "Trimming parameters for " << getName() << std::endl;

    StatementList stmts;
    getStatements(stmts);

    // find parameters that are referenced (ignore calls to this)
    int nparams = signature->getNumParams();
    std::vector<Exp*> params;
    bool referenced[nparams];
    for (int i = 0; i < nparams; i++) {
        referenced[i] = false;
        params.push_back(signature->getParamExp(i)->clone()->
                            expSubscriptVar(new Terminal(opWild), NULL));
    }

    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) 
        if (!s->isCall() || ((CallStatement*)s)->getDestProc() != this) {
            for (int i = 0; i < nparams; i++) {
                Exp *p = new Unary(opParam, 
                             new Const((char*)signature->getParamName(i)));
                if (!referenced[i] && (s->usesExp(params[i]) || s->usesExp(p)))
                    referenced[i] = true;
                delete p;
            }
        }

    for (int i = 0; i < nparams; i++)
        if (!referenced[i]) {
            bool allZero;
            Exp *e = params[i]->removeSubscripts(allZero);
            if (VERBOSE) 
                std::cerr << "removing unused parameter " << e << std::endl;
            removeParameter(e);
        }
}

void Proc::removeReturn(Exp *e)
{
    signature->removeReturn(e);
    for (std::set<CallStatement*>::iterator it = callerSet.begin();
         it != callerSet.end(); it++)
            (*it)->removeReturn(e);
}

void UserProc::removeReturn(Exp *e)
{
    int n = signature->findReturn(e);
    if (n != -1) {
        Proc::removeReturn(e);
        for (unsigned i = 0; i < returnStatements.size(); i++)
            returnStatements[i]->removeReturn(n);
    }
}

void Proc::removeParameter(Exp *e)
{
    int n = signature->findParam(e);
    if (n != -1) {
        signature->removeParameter(n);
        for (std::set<CallStatement*>::iterator it = callerSet.begin();
             it != callerSet.end(); it++)
            (*it)->removeArgument(n);
    }
}

void Proc::addParameter(Exp *e)
{
    for (std::set<CallStatement*>::iterator it = callerSet.begin();
         it != callerSet.end(); it++)
            (*it)->addArgument(e);
    signature->addParameter(e);
}

void UserProc::replaceExpressionsWithGlobals() {
    Exp *match = new Unary(opMemOf, new Terminal(opWild)); 
    StatementList stmts;
    getStatements(stmts);

    // replace expressions with symbols
    StmtListIter it;
    for (Statement*s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        Exp *memof;
        const char *global;
        
        if (s->search(match, memof)) { 
            if (memof->getSubExp1()->getOper() == opIntConst &&
                (global = 
                    prog->getGlobal(((Const*)memof->getSubExp1())->getInt()))) {
                s->searchAndReplace(memof, 
                    new Unary(opGlobal, new Const((char*)global)));
            }
        }
    }

    delete match;
}

void UserProc::replaceExpressionsWithSymbols() {
    StatementList stmts;
    getStatements(stmts);

    // replace expressions in regular statements with symbols
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        for (std::map<Exp*, Exp*>::iterator it1 = symbolMap.begin();
          it1 != symbolMap.end(); it1++) {
            bool ch = s->searchAndReplace((*it1).first, (*it1).second);
            if (ch && VERBOSE) {
                std::cerr << "std stmt: replace " << (*it1).first <<
                  " with " << (*it1).second << " result " << s << std::endl;
            }
        }
    }
}

void UserProc::replaceExpressionsWithParameters(int depth) {
    StatementList stmts;
    getStatements(stmts);

    // replace expressions in regular statements with parameters
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        for (int i = 0; i < signature->getNumParams(); i++) 
            if (signature->getParamExp(i)->getMemDepth() == depth) {
            Exp *r = signature->getParamExp(i)->clone();
            r = r->expSubscriptVar(new Terminal(opWild), NULL);
            s->searchAndReplace(r, new Unary(opParam, 
                         new Const((char*)signature->getParamName(i))));
        }
    }
}

void UserProc::replaceExpressionsWithLocals() {
    StatementList stmts;
    getStatements(stmts);

    // replace expressions in regular statements with locals
    StmtListIter it;
    int sp = signature->getStackRegister(prog);
    Exp *l = new Unary(opMemOf, new Binary(opMinus, 
                new RefExp(Unary::regOf(sp), NULL),
                new Terminal(opWild)));
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        Exp *result;
        bool ch = s->search(l, result);
        if (ch && 
            result->getSubExp1()->getSubExp2()->getOper() == opIntConst) {
            Exp *e;
            if (symbolMap.find(result) == symbolMap.end()) {
                e = newLocal(new IntegerType());
                symbolMap[result->clone()] = e;
            } else {
                e = symbolMap[result];
            }
            s->searchAndReplace(result, e);
        }
    }
}

bool UserProc::nameStackLocations() {
    Exp *match = signature->getStackWildcard();
    if (match == NULL) return false;

    bool found = false;
    StatementList stmts;
    getStatements(stmts);
    // create a symbol for every memory reference
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        Exp *memref; 
        if (s->search(match, memref)) {
            if (symbolMap.find(memref) == symbolMap.end()) {
                if (VERBOSE) {
                    std::cerr << "stack location found: ";
                    memref->print(std::cerr);
                    std::cerr << std::endl;
                }
                symbolMap[memref->clone()] = newLocal(new IntegerType());
            }
            assert(symbolMap.find(memref) != symbolMap.end());
            std::string name = ((Const*)symbolMap[memref]->getSubExp1())
					->getStr();
            locals[name] = s->updateType(memref, locals[name]);
            found = true;
        }
    }
    delete match;
    return found;
}

bool UserProc::nameRegisters() {
    Exp *match = new Unary(opRegOf, new Terminal(opWild));
    if (match == NULL) return false;
    bool found = false;

    StatementList stmts;
    getStatements(stmts);
    // create a symbol for every register
    StmtListIter it;
    for (Statement*s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        Exp *memref; 
        if (s->search(match, memref)) {
            if (symbolMap.find(memref) == symbolMap.end()) {
                if (VERBOSE)
                    std::cerr << "register found: " << memref << std::endl;
                symbolMap[memref->clone()] = newLocal(new IntegerType());
            }
            assert(symbolMap.find(memref) != symbolMap.end());
            std::string name = ((Const*)symbolMap[memref]->getSubExp1())->
              getStr();
            locals[name] = s->updateType(memref, locals[name]);
            found = true;
        }
    }
    delete match;
    return found;
}

bool UserProc::removeNullStatements() {
    bool change = false;
    StatementList stmts;
    getStatements(stmts);
    // remove null code
    StmtListIter it;
    for (Statement*s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        if (s->isNullStatement()) {
            // A statement of the form x := x
            if (VERBOSE) {
                std::cerr << "removing null statement: " << s->getNumber() <<
                " " << s << "\n";
            }
            removeStatement(s);
#if 0
            // remove from reach sets
            StatementSet &reachout = s->getBB()->getReachOut();
            if (reachout.remove(s))
                cfg->computeReaches();      // Highly sus: do all or none!
                recalcDataflow();
#endif
            change = true;
        }
    }
    return change;
}

#if 0
bool UserProc::removeDeadStatements() {
    bool change = false;
    StatementList stmts;
    getStatements(stmts);
    // remove dead code
    StmtListIter it;
    for (Statement*s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
#if 1       // This seems so simple... remove statements with no usedBy MVE
            // Trent: this is not dead code removal!  Dead code is statements
            // which define a location which is subsequently written to by
            // another statement, without first being used.  So although
            // having an empty usedBy set is a necessary condition, it is not
            // a sufficient condition.
        Assign* asgn = dynamic_cast<Assign*>(s);
        if (asgn == NULL) {
            // Never remove a call or jcond; they have important side effects
            CallStatement *call = dynamic_cast<CallStatement*>(s);
            if (call != NULL && call->getReturnLoc() != NULL) {
                if (VERBOSE) 
                    std::cerr << "Ignoring return value of statement " << s 
                        << std::endl;
                call->setIgnoreReturnLoc(true);
                change = true;
            }
            continue;
        }
        if (getCFG()->getReachExit()->exists(s))
            // Don't remove unused code that reaches the exit
            // These will be handled later by moveInternalStatements()
            continue;
        if (s->getNumUsedBy() == 0) {
            if (VERBOSE) 
                std::cerr << "Removing unused statement " << s->getNumber() <<
                  " " << s << std::endl;
            removeStatement(s);
            change = true;
        }
#else       // Why so complex? Is there a catch? MVE.. See above, Trent
        StatementSet dead;
        s->getDeadStatements(dead);
        StmtSetIter it1;
        for (Statement* s1 = dead.getFirst(it1); s1; s1 = dead.getNext(it1)) {
            if (getCFG()->getReachExit().exists(s1))
                continue;
            if (!s1->getLeft()->isMemOf()) {
                // hack: if the dead statement has a use which would make
                // this statement useless if propagated, leave it
                StatementSet uses;
                s1->getUses(uses);
                bool matchingUse = false;
                StmtSetIter it2;
                for (Statement* s2 = uses.getFirst(it2); s2;
                  s2 = uses.getNext(it2)) {
                    Assign *e = dynamic_cast<Assign*>(s2);
                    if (e == NULL || s1->getLeft() == NULL) continue;
                    if (*e->getSubExp2() == *s1->getLeft()) {
                        matchingUse = true;
                        break;
                    }
                }
                CallStatement *call = dynamic_cast<CallStatement*>(*it1);
                if (matchingUse && call == NULL) continue;
                if (VERBOSE|1) {
                    std::cerr << "removing dead code: ";
                    s1->printAsUse(std::cerr);
                    std::cerr << std::endl;
                }
                if (call == NULL) {
                    removeStatement(s1);
//std::cerr << "After remove, BB is"; s1->getBB()->print(std::cerr, true);
                } else {
                    call->setIgnoreReturnLoc(true);
                }
                // remove from reach sets
                StatementSet &reachout = s1->getBB()->getReachOut();
                reachout.remove(s1);
                //cfg->computeReaches();      // Highly sus: do all or none!
                //recalcDataflow();
                change = true;
            }
        }
#endif
    }
    if (VERBOSE && change)
        print(std::cerr, true);
    return change;
}
#endif

void UserProc::processConstants() {
    if (VERBOSE)
        std::cerr << "Process constants for " << getName() << "\n";
    StatementList stmts;
    getStatements(stmts);
    // process any constants in the statement
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        s->processConstants(prog);
}

#if 0
// bMemProp set to true if a memory location is propagated
bool UserProc::propagateAndRemoveStatements() {
    bool change = false;
    StatementList stmts;
    getStatements(stmts);
    // propagate any statements that can be removed
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        Assign *assign = dynamic_cast<Assign*>(*it);
        if (assign && *assign->getSubExp1() == *assign->getSubExp2())
            continue;
        Exp* rhs = s->getRight();
        if (s->canPropagateToAll()) {
            if (Boomerang::get()->numToPropagate >= 0) {
                if (Boomerang::get()->numToPropagate == 0) return change;
                Boomerang::get()->numToPropagate--;
            }
            if (cfg->getReachExit()->exists(s)) {
                if (s->getNumRefs() != 0) {
                    // tempories that store the results of calls are ok
                    if (rhs && 
                      s->findDef(rhs) &&
                      !s->findDef(rhs)->getRight()) {
                        if (VERBOSE) {
                            std::cerr << "allowing propagation of temporary: ";
                            s->printAsUse(std::cerr);
                            std::cerr << std::endl;
                        }
                    } else
                        continue;
                } else {
                    //if (Boomerang::get()->noRemoveInternal)
                        continue;
                    // new internal statement
                    if (VERBOSE) {
                        std::cerr << "new internal statement: ";
                        s->printAsUse(std::cerr);
                        std::cerr << std::endl;
                    }
                    internal.append(s);
                }
            }
            s->propagateToAll();
            removeStatement(s);
            // remove from reach sets
            StatementSet &reachout = s->getBB()->getReachOut();
#if 0
            if (reachout.remove(s))
                cfg->computeReaches();
#else
            reachout.remove(s);
            //recalcDataflow();       // Fix alias problems
#endif
            if (VERBOSE) {
                // debug: print
                print(std::cerr, true);
            }
            change = true;
        }
    }
    return change;
}
#endif

// Propagate statements, but don't remove
// Respect the memory depth (don't propagate statements that have components
// of a higher memory depth than memDepth)
void UserProc::propagateStatements(int memDepth) {
    StatementList stmts;
    getStatements(stmts);
    // propagate any statements that can be
    StmtListIter it;
    StatementSet empty;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        if (s->isPhi()) continue;
        if (s->isReturn()) continue;
        s->propagateTo(memDepth, empty);
    }
}

void UserProc::promoteSignature() {
    signature = signature->promote(this);
}

Exp* UserProc::newLocal(Type* ty) {
    std::ostringstream os;
    os << "local" << locals.size();
    std::string name = os.str();
    locals[name] = ty;
    return new Unary(opLocal, new Const(strdup(name.c_str())));
}

// Add local variables local<nextAvailable> .. local<n-1>
void UserProc::addLocals(int n) {
    for (int i=locals.size(); i < n; i++) {
        std::ostringstream os;
        os << "local" << i;
        std::string name = os.str();
        locals[name] = new IntegerType();   // FIXME one day
    }
}

#if 0
void UserProc::recalcDataflow() {
    if (VERBOSE) std::cerr << "Recalculating dataflow\n";
    cfg->computeLiveness();
    cfg->computeReaches();
    cfg->computeAvailable();
    StatementList stmts;
    getStatements(stmts);
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        s->clearUses();
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        s->calcUseLinks();
}

void UserProc::computeUses() {
    StatementList stmts;
    getStatements(stmts);
    StmtListIter it;
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        s->clearUses();
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it))
        s->calcUseLinks();
}
#endif

void UserProc::countRefs(RefCounter& refCounts) {
    StatementList stmts;
    getStatements(stmts);
    StmtListIter ll;
    for (Statement* s = stmts.getFirst(ll); s; s = stmts.getNext(ll)) {
        LocationSet refs;
        s->addUsedLocs(refs);
        LocSetIter rr;
        for (Exp* r = refs.getFirst(rr); r; r = refs.getNext(rr)) {
            if (r->isSubscript()) {
                Statement *ref = ((RefExp*)r)->getRef();
                refCounts[ref]++;
            }
        }
    }
}

void UserProc::removeUnusedStatements(RefCounter& refCounts, int depth) {
    StatementList stmts;
    getStatements(stmts);
    StmtListIter ll;
    bool change;
    do {
        change = false;
        Statement* s = stmts.getFirst(ll);
        while (s) {
            if (s->isCall() && refCounts[s] == 0) {
                if (VERBOSE)
                    std::cerr << "clearing return set of unused call " << s 
                              << std::endl;
                CallStatement *call = (CallStatement*)s;
                std::vector<Exp*> returns;
                for (int i = 0; i < call->getNumReturns(); i++)
                    returns.push_back(call->getReturnExp(i));
                for (int i = 0; i < (int)returns.size(); i++)
                    if (returns[i]->getMemDepth() <= depth)
                        call->removeReturn(returns[i]);
                s = stmts.getNext(ll);
                continue;
            }
            if (s->getKind() != STMT_ASSIGN && s->getKind() != STMT_BOOL) {
                // Never delete a statement other than an assignment or setstmt
                // (e.g. nothing "uses" a Jcond)
                s = stmts.getNext(ll);
                continue;
            }
            if (s->getLeft() && s->getLeft()->getMemDepth() > depth) {
                s = stmts.getNext(ll);
                continue;
            }
            if (refCounts[s] == 0) {
                // First adjust the counts. Need to be careful not to count
                // two refs as two; refCounts is a count of the number of
                // statements that use a definition, not the number of refs
                StatementSet refs;
                LocationSet components;
                s->addUsedLocs(components);
                LocSetIter cc;
                for (Exp* c = components.getFirst(cc); c;
                  c = components.getNext(cc)) {
                    if (c->isSubscript()) {
                        refs.insert(((RefExp*)c)->getRef());
                    }
                }
                StmtSetIter dd;
                for (Statement* def = refs.getFirst(dd); def;
                  def = refs.getNext(dd)) {
                    refCounts[def]--;
                }
                if (VERBOSE)
                    std::cerr << "Removing unused statement " <<
                      s->getNumber() << " " << s << std::endl;
                removeStatement(s);
                s = stmts.remove(ll);  // So we don't try to re-remove it
                change = true;
                continue;               // Don't call getNext this time
            }
            s = stmts.getNext(ll);
        }
    } while (change);
}

//
//  SSA code
//

void UserProc::fromSSAform() {
    StatementList stmts;
    getStatements(stmts);
    StmtListIter it;
    igraph ig;
    int tempNum = locals.size();
    cfg->findInterferences(ig, tempNum);

    // First rename the variables (including phi's, but don't remove)
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        s->fromSSAform(ig);
    }

    // Now remove the phi's
    for (Statement* s = stmts.getFirst(it); s; s = stmts.getNext(it)) {
        if (!s->isPhi()) continue;
        // Check that the base variables are all the same
        PhiExp* p = (PhiExp*)s->getRight();
        LocationSet refs;
        p->addUsedLocs(refs);
        LocSetIter rr;
        Exp* r = refs.getFirst(rr);
        Exp* first = r;
        bool same = true;
        while (r) {
            if (!(*r *= *first)) {       // Ref-insensitive compare
                same = false;
                break;
            }
            r = refs.getNext(rr);
        }
        if (same) {
            // Is the left of the phi assignment the same base variabe as all
            // the operands?
            if (*s->getLeft() *= *first)
                // Just removing the refs will work
                removeStatement(s);
            else
                // Just need to replace the phi by an expression,
                // e.g. local0 = phi(r24{3}, r24{5}) becomes 
                //      local0 = r24
                ((Assign*)s)->setRight(first->getSubExp1()->clone());
        }
        else {
            // Need copies
            if (Boomerang::get()->debugLiveness)
                std::cerr << "Phi statement " << s <<
                  " requires copies, using temp" << tempNum << "\n";
            // For each definition ref'd in the phi
            StmtVecIter rr;
            Statement* def;
            for (def = p->getFirstRef(rr); !p->isLastRef(rr);
                  def = p->getNextRef(rr)) {
                // Start with the original name, in the left of the phi
                // (note: this has not been renamed above)
                Exp* right = p->getSubExp1()->clone();
                // Wrap it in a ref to def
                right = new RefExp(right, def);
                // Check the interference graph for a new name
                if (ig.find(right) != ig.end()) {
                    std::ostringstream os;
                    os << "local" << ig[right];
                    delete right;
                    right = new Unary(opLocal,
                        new Const(strdup(os.str().c_str())));
                } else {
                    // Just take off the reference
                    RefExp* old = (RefExp*)right;
                    right = right->getSubExp1();
                    old->setSubExp1ND(NULL);
                    delete old;
                }
                // Insert a new assignment, to local<tempNum>, from right
                insertAssignAfter(def, tempNum, right);
            }
            // Replace the RHS of the phi with the new temp
            std::ostringstream os;
            os << "local" << tempNum++;
            std::string name = os.str();
            ((Assign*)s)->setRight(new Unary(opLocal,
              new Const(strdup(name.c_str()))));
        }
    }

    // Add the resulting locals to the proc, so they will be declared
    addLocals(tempNum);

}

void UserProc::insertArguments(StatementSet& rs) {
    cfg->insertArguments(rs);
}

bool UserProc::prove(Exp *query)
{
    if (proven.find(query) != proven.end())
        return true;

    Exp *original = query->clone();

    assert(query->getOper() == opEquals);
    
    // subscript locs on the right with {0}
    LocationSet locs;
    query->getSubExp2()->addUsedLocs(locs);
    LocSetIter xx;
    for (Exp* x = locs.getFirst(xx); x; x = locs.getNext(xx)) {
        query->refSubExp2() = query->getSubExp2()->expSubscriptVar(x, NULL);
    }

    // subscript locs on the left with the last statement that defined them
    locs.clear();
    query->getSubExp1()->addUsedLocs(locs);
    StatementList stmts;
    getStatements(stmts);
    for (Exp* x = locs.getFirst(xx); x; x = locs.getNext(xx)) {
        Statement *def = NULL;
        for (unsigned j = 0; j < returnStatements.size(); j++)
            for (int i = 0; i < returnStatements[j]->getNumReturns(); i++) {
                Exp *e = returnStatements[j]->getReturnExp(i); 
                RefExp *r = dynamic_cast<RefExp*>(e);
                assert(r);
                if (*r->getSubExp1() == *x) {
                    def = r->getRef();
                    break;
                }
            }
        query->refSubExp1() = query->getSubExp1()->expSubscriptVar(x, def);
    }

    proven.insert(original);
    if (!prover(query)) {
        proven.erase(original);
        delete original;
        return false;
    }
    delete query;
   
    return true;
}

bool UserProc::prover(Exp *query, PhiExp *lastPhi)
{
    query = query->clone();
    bool change = true;
    bool swapped = false;
    while (change) {
        if (VERBOSE) {
            query->print(std::cerr, true);
            std::cerr << std::endl;
        }
    
        change = false;
        if (query->getOper() == opEquals) {

            // move constants to the right
            Exp *plus = query->getSubExp1();
            Exp *s1s2 = plus->getSubExp2();
            if (!change && plus->getOper() == opPlus && s1s2->isIntConst()) {
                query->refSubExp2() = new Binary(opPlus, query->getSubExp2(),
                                        new Unary(opNeg, s1s2->clone()));
                query->refSubExp1() = ((Binary*)plus)->becomeSubExp1();
                change = true;
            }
            if (!change && plus->getOper() == opMinus && s1s2->isIntConst()) {
                query->refSubExp2() = new Binary(opPlus, query->getSubExp2(),
                                        s1s2->clone());
                query->refSubExp1() = ((Binary*)plus)->becomeSubExp1();
                change = true;
            }


            // substitute using a statement that has the same left as the query
            if (!change && query->getSubExp1()->getOper() == opSubscript) {
                RefExp *r = (RefExp*)query->getSubExp1();
                Statement *s = r->getRef();
                CallStatement *call = dynamic_cast<CallStatement*>(s);
                if (call) {
                    Exp *right = call->getProven(r->getSubExp1());
                    if (right) {
                        right = right->clone();
                        if (VERBOSE)
                            std::cerr << "using proven (or induction) for " 
                                      << call->getDestProc()->getName() << " " 
                                      << r->getSubExp1() 
                                      << " = " << right << std::endl;
                        right = call->substituteParams(right);
                        query->setSubExp1(right);
                        change = true;
                    }
                } else if (s && s->getRight()) {
                    if (s->getRight()->getOper() == opPhi) {
                        // for a phi, we have to prove the query for every 
                        // statement
                        PhiExp *p = (PhiExp*)s->getRight();
                        if (VERBOSE)
                            std::cerr << "found " << p << " prove for each" 
                                      << std::endl;
                        StmtVecIter it;
                        bool ok = true;
                        if (p == lastPhi) {
                            if (VERBOSE)
                                std::cerr << "phi loop detected" << std::endl;
                            ok = false;
                        } else {
                            for (Statement *s1 = p->getFirstRef(it); 
                                            !p->isLastRef(it);
                                            s1 = p->getNextRef(it)) {
                                Exp *e = query->clone();
                                RefExp *r1 = (RefExp*)e->getSubExp1();
                                r1->setDef(s1);
                                if (VERBOSE)
                                    std::cerr << "proving for " << e 
                                              << std::endl;
                                if (!prover(e, p)) { 
                                    ok = false; 
                                    delete e; 
                                    break; 
                                }
                                delete e;
                            }
                        }
                        if (ok) query = new Terminal(opTrue);
                        else query = new Terminal(opFalse);
                        change = true;
                    } else {
                        query->setSubExp1(s->getRight()->clone());
                        change = true;
                    }
                }
            }

            // remove memofs from both sides if possible
            if (!change && query->getSubExp1()->getOper() == opMemOf &&
                query->getSubExp2()->getOper() == opMemOf) {
                query->refSubExp1() = ((Unary*)query->getSubExp1())->becomeSubExp1();
                query->refSubExp2() = ((Unary*)query->getSubExp2())->becomeSubExp1();
                change = true;
            }

            // find a memory def for the right if there is a memof on the left
            if (!change && query->getSubExp1()->getOper() == opMemOf) {
                StatementList stmts;
                getStatements(stmts);
                StmtListIter it;
                for (Statement *s = stmts.getFirst(it); s; s = stmts.getNext(it))
                    if (s->getLeft() && s->getRight() && 
                        *s->getRight() == *query->getSubExp2() &&
                        s->getLeft()->getOper() == opMemOf) {
                        query->refSubExp2() = s->getLeft()->clone();
                        change = true;
                        break;
                    }
            }

            // last chance, swap left and right if havn't swapped before
            if (!change && !swapped) {
                Exp *e = query->getSubExp1();
                query->refSubExp1() = query->getSubExp2();
                query->refSubExp2() = e;
                change = true;
                swapped = true;
            }
        }

        Exp *old = query->clone();

        query = query->simplify();

        if (change && !(*old == *query) && VERBOSE) {
            old->print(std::cerr, true);
            std::cerr << std::endl;
        }
        delete old;
    }
    return query->getOper() == opTrue;
}

// Get the set of locations defined by this proc. In other words, the define set,
// currently called returns
void UserProc::getDefinitions(LocationSet& ls) {
    int n = signature->getNumReturns();
    for (int j=0; j < n; j++) {
        ls.insert(signature->getReturnExp(j));
    }
}
