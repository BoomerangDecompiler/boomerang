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

#include <types.h>
#include <sstream>
#include <algorithm>        // For find()
#include "cluster.h"
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
#include "constraint.h"
#include "analysis.h"

typedef std::map<Statement*, int> RefCounter;

#define DEBUG_PROOF (Boomerang::get()->debugProof)
#define DEBUG_UNUSED_RETS (Boomerang::get()->debugUnusedRets)
#define DEBUG_UNUSED_STMT (Boomerang::get()->debugUnusedStmt)

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
     : prog(prog), signature(sig), address(uNative), m_firstCaller(NULL) 
{cluster = prog->getRootCluster();}

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

void Proc::printCallGraphXML(std::ostream &os, int depth, bool recurse)
{
    if (!Boomerang::get()->dumpXML)
        return;
    visited = true;
    for (int i = 0; i < depth; i++)
        os << "   ";
    os << "<proc name=\"" << getName() << "\"/>\n";
}

void UserProc::printCallGraphXML(std::ostream &os, int depth, bool recurse)
{
    if (!Boomerang::get()->dumpXML)
        return;
    bool wasVisited = visited;
    visited = true;
    int i;
    for (i = 0; i < depth; i++)
        os << "   ";
    os << "<proc name=\"" << getName() << "\">\n";
    if (recurse) {
        for (std::set<Proc*>::iterator it = calleeSet.begin(); 
                                       it != calleeSet.end();
                                       it++) 
            (*it)->printCallGraphXML(os, depth+1, !wasVisited && 
                                                  !(*it)->isVisited());
    }
    for (i = 0; i < depth; i++)
        os << "   ";
    os << "</proc>\n";
}

void Proc::printDetailsXML()
{
    if (!Boomerang::get()->dumpXML)
        return;
    std::ofstream out((Boomerang::get()->getOutputPath() + 
                      getName() + "-details.xml").c_str());
    out << "<proc name=\"" << getName() << "\">\n";
    int i;
    for (i = 0; i < signature->getNumParams(); i++) {
        out << "   <param name=\"" << signature->getParamName(i) << "\" "
            << "exp=\"" << signature->getParamExp(i) << "\" "
            << "type=\"" << signature->getParamType(i)->getCtype() << "\"";
        out << "/>\n";
    }
    for (i = 0; i < signature->getNumReturns(); i++)
        out << "   <return exp=\"" << signature->getReturnExp(i) << "\" "
            << "type=\"" << signature->getReturnType(i)->getCtype() << "\"/>\n";
    out << "</proc>\n";
    out.close();
}

void UserProc::printDecodedXML()
{
    if (!Boomerang::get()->dumpXML)
        return;
    std::ofstream out((Boomerang::get()->getOutputPath() + 
                      getName() + "-decoded.xml").c_str());
    out << "<proc name=\"" << getName() << "\">\n";
    out << "    <decoded>\n";
    std::ostringstream os;
    print(os, false);
    std::string s = os.str();
    escapeXMLChars(s);
    out << s;
    out << "    </decoded>\n";
    out << "</proc>\n";
    out.close();
}

void UserProc::printAnalysedXML()
{
    if (!Boomerang::get()->dumpXML)
        return;
    std::ofstream out((Boomerang::get()->getOutputPath() + 
                      getName() + "-analysed.xml").c_str());
    out << "<proc name=\"" << getName() << "\">\n";
    out << "    <analysed>\n";
    std::ostringstream os;
    print(os, false);
    std::string s = os.str();
    escapeXMLChars(s);
    out << s;
    out << "    </analysed>\n";
    out << "</proc>\n";
    out.close();
}

void UserProc::printSSAXML()
{
    if (!Boomerang::get()->dumpXML)
        return;
    std::ofstream out((Boomerang::get()->getOutputPath() + 
                      getName() + "-ssa.xml").c_str());
    out << "<proc name=\"" << getName() << "\">\n";
    out << "    <ssa>\n";
    std::ostringstream os;
    print(os, true);
    std::string s = os.str();
    escapeXMLChars(s);
    out << s;
    out << "    </ssa>\n";
    out << "</proc>\n";
    out.close();
}


void UserProc::printXML()
{
    if (!Boomerang::get()->dumpXML)
        return;
    printDetailsXML();
    printSSAXML();
    prog->printCallGraphXML();
    printUseGraph();
}

void UserProc::printUseGraph()
{
    std::ofstream out((Boomerang::get()->getOutputPath() + 
                      getName() + "-usegraph.dot").c_str());
    out << "digraph " << getName() << " {\n";
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->isPhi())
            out << s->getNumber() << " [shape=diamond];\n";
        LocationSet refs;
        s->addUsedLocs(refs);
        LocationSet::iterator rr;
        for (rr = refs.begin(); rr != refs.end(); rr++) {
            if (((Exp*)*rr)->isSubscript()) {
                RefExp *r = (RefExp*)*rr;
                if (r->getRef())
                    out << r->getRef()->getNumber() << " -> " 
                        << s->getNumber() << ";\n";
            }
        }
    }
    out << "}\n";
    out.close();
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

Exp *LibProc::getProven(Exp *left)
{
    // Just use the signature information (all we have, after all)
    return signature->getProven(left);
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
    cfg(new Cfg()), decoded(false), analysed(false),
    decompileSeen(false), decompiled(false), isRecursive(false),
    theReturnStatement(NULL) {
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

class lessEvaluate : public std::binary_function<SyntaxNode*, SyntaxNode*, bool> {
public:
    bool operator()(const SyntaxNode* x, const SyntaxNode* y) const
    {
        return ((SyntaxNode*)x)->getScore() > 
               ((SyntaxNode*)y)->getScore();
    }
};

SyntaxNode *UserProc::getAST()
{
    int numBBs = 0;
    BlockSyntaxNode *init = new BlockSyntaxNode();
    BB_IT it;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        BlockSyntaxNode *b = new BlockSyntaxNode();
        b->setBB(bb);
        init->addStatement(b);
        numBBs++;
    }
    
    // perform a best firs search for the nicest AST
    std::priority_queue<SyntaxNode*,  std::vector<SyntaxNode*>,
                        lessEvaluate > ASTs;
    ASTs.push(init);

    SyntaxNode *best = init;
    int best_score = init->getScore();
    int count = 0;
    while (ASTs.size()) {
        if (best_score < numBBs * 2)  {
            LOG << "exit early: " << best_score << "\n";
            break;
        }

        SyntaxNode *top = ASTs.top();
        ASTs.pop();
        int score = top->evaluate(top);

        printAST(top); // debug

        if (score < best_score) {
            if (best && top != best)
                delete best;
            best = top;
            best_score = score;
        }

        count++;
        if (count > 100)
            break;

        // add successors
        std::vector<SyntaxNode*> successors;
        top->addSuccessors(top, successors);
        for (unsigned i = 0; i < successors.size(); i++) {
            //successors[i]->addToScore(top->getScore());   // uncomment for A*
            successors[i]->addToScore(successors[i]->getDepth()); // or this
            ASTs.push(successors[i]);
        }

        if (top != best)
            delete top;
    }

    // clean up memory
    while(ASTs.size()) {
        SyntaxNode *top = ASTs.top();
        ASTs.pop();
        if (top != best)
            delete top;
    }
    
    return best;
}

int count = 1;

void UserProc::printAST(SyntaxNode *a)
{
    char s[1024];
    if (a == NULL)
        a = getAST();
    sprintf(s, "ast%i-%s.dot", count++, getName());
    std::ofstream of(s);
    of << "digraph " << getName() << " {" << std::endl;
    of << "  label=\"score: " << a->evaluate(a) << "\";" << std::endl;
    a->printAST(a, of);
    of << "}" << std::endl;
    of.close();
}

/*==============================================================================
 * FUNCTION:        UserProc::setDecoded
 * OVERVIEW:        
 * PARAMETERS:      
 * RETURNS:         
 *============================================================================*/
void UserProc::setDecoded() {
    decoded = true;
    printDecodedXML();
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
 * FUNCTION:        UserProc::addCallee
 * OVERVIEW:        Add this callee to the set of callees for this proc
 * PARAMETERS:      A pointer to the Proc object for the callee
 * RETURNS:         <nothing>
 *============================================================================*/
void UserProc::addCallee(Proc* callee) {
    calleeSet.insert(callee);
}

void UserProc::generateCode(HLLCode *hll) {
    assert(cfg);
    assert(getEntryBB());

    cfg->structure();
    replaceExpressionsWithGlobals();
    if (!Boomerang::get()->noLocals) {
//        while (nameStackLocations())
//            replaceExpressionsWithSymbols();
        while (nameRegisters())
            replaceExpressionsWithSymbols();
    }
    removeUnusedLocals();

    // Note: don't try to remove unused statements here; that requires the
    // RefExps, which are all gone now (transformed out of SSA form)!

    if (VERBOSE || Boomerang::get()->printRtl)
        printToLog();

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

void UserProc::printToLog(bool withDF)
{
    signature->printToLog();
    for (std::map<std::string, Type*>::iterator it = locals.begin();
      it != locals.end(); it++) {
        LOG << (*it).second->getCtype() << " " << (*it).first.c_str() << " ";
        Exp *e = getLocalExp((*it).first.c_str());
        // Beware: for some locals, getLocalExp() returns NULL
        if (e)
            LOG << e << "\n";
        else
            LOG << "-\n";
    }
    cfg->printToLog(withDF);
    LOG << "\n";
}

// initialise all statements
void UserProc::initStatements() {
    BB_IT it;
    BasicBlock::rtlit rit; StatementList::iterator sit;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        for (Statement* s = bb->getFirstStmt(rit, sit); s;
              s = bb->getNextStmt(rit, sit)) {
            s->setProc(this);
            s->setBB(bb);
            CallStatement* call = dynamic_cast<CallStatement*>(s);
            if (call) {
                // I think this should be done in analysis
                call->setSigArguments();
            }
            ReturnStatement *ret = dynamic_cast<ReturnStatement*>(s);
            if (ret)
                ret->setSigArguments();
        }
    }
}

void UserProc::numberStatements(int& stmtNum) {
    BB_IT it;
    BasicBlock::rtlit rit; StatementList::iterator sit;
    for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
        for (Statement* s = bb->getFirstStmt(rit, sit); s;
          s = bb->getNextStmt(rit, sit))
            s->setNumber(++stmtNum);
    }
}

void UserProc::numberPhiStatements(int& stmtNum) {
    BB_IT it;
    BasicBlock::rtlit rit; StatementList::iterator sit;
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
    // remove anything proven about this statement
    for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++) {
        LocationSet refs;
        (*it)->addUsedLocs(refs);
        LocationSet::iterator rr;
    bool usesIt = false;
        for (rr = refs.begin(); rr != refs.end(); rr++) {
            Exp* r = *rr;
            if (r->isSubscript() && ((RefExp*)r)->getRef() == stmt) {
        usesIt = true;
        break;
        }
    }
    if (usesIt) {
        LOG << "removing proven exp " << (*it) << " that uses statement being removed.\n";
        proven.erase(it);
        it = proven.begin();
        continue;
    }
    }

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
    std::list<Statement*>::iterator it;
    std::list<Statement*>* stmts;
    if (s == NULL) {
        // This means right is supposed to be a parameter. We can insert the
        // assignment at the start of the entryBB
        PBB entryBB = cfg->getEntryBB();
        std::list<RTL*> *rtls = entryBB->getRTLs();
        assert(rtls->size());       // Entry BB should have at least 1 RTL
        stmts = &rtls->front()->getList();
        it = stmts->begin();
    } else {
        // An ordinary definition; put the assignment at the end of s's BB
        PBB bb = s->getBB();         // Get the enclosing BB for s
        std::list<RTL*> *rtls = bb->getRTLs();
        assert(rtls->size());       // If s is defined here, there should be
                                    // at least 1 RTL
        stmts = &rtls->back()->getList();
        it = stmts->end();          // Insert before the end
    }
    std::ostringstream os;
    os << "local" << tempNum;
    Assign* as = new Assign(
        Location::local(strdup(os.str().c_str()), this),
        right);
    stmts->insert(it, as);
    return;
}

void UserProc::insertStatementAfter(Statement* s, Statement* a) {
    // Note: this procedure is designed for the front end, where enclosing
    // BBs are not set up yet
    // So this is an inefficient linear search!
    BB_IT bb;
    for (bb = cfg->begin(); bb != cfg->end(); bb++) {
        std::list<RTL*>::iterator rr;
        std::list<RTL*>* rtls = (*bb)->getRTLs();
        for (rr = rtls->begin(); rr != rtls->end(); rr++) {
            std::list<Statement*>& stmts = (*rr)->getList();
            std::list<Statement*>::iterator ss;
            for (ss = stmts.begin(); ss != stmts.end(); ss++) {
                if (*ss == s) {
                    ss++;       // This is the point to insert before
                    stmts.insert(ss, a);
                    return;
                }
            }
        }
    }
    assert(false);          // Should have found this statement in this BB
}

// Decompile this UserProc
std::set<UserProc*>* UserProc::decompile() {
    // Prevent infinite loops when there are cycles in the call graph
    if (decompiled) return NULL;

    // We have seen this proc
    decompileSeen = true;

    std::set<UserProc*>* cycleSet = new std::set<UserProc*>;
    if (!Boomerang::get()->noDecodeChildren) {
        // Recurse to children first, to perform a depth first search
        BB_IT it;
        // Look at each call, to do the DFS
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
    }

    if (VERBOSE) LOG << "decompiling: " << getName() << "\n";

    // Sort by address, so printouts make sense
    cfg->sortByAddress();
    // Initialise statements
    initStatements();

    // Compute dominance frontier
    cfg->dominators();

    // Number the statements
    int stmtNumber = 0;
    numberStatements(stmtNumber); 

    printXML();

    if (Boomerang::get()->noDecompile) {
        decompiled = true;
        return cycleSet;
    }

    // For each memory depth
    int maxDepth = findMaxDepth() + 1;
    if (Boomerang::get()->maxMemDepth < maxDepth)
        maxDepth = Boomerang::get()->maxMemDepth;
    int depth;
    for (depth = 0; depth <= maxDepth; depth++) {

        if (VERBOSE)
            LOG << "placing phi functions at depth " << depth << "\n";
        // Place the phi functions for this memory depth
        cfg->placePhiFunctions(depth, this);

        if (VERBOSE)
            LOG << "numbering phi statements at depth " << depth << "\n";
        // Number them
        numberPhiStatements(stmtNumber);

        if (VERBOSE)
            LOG << "renaming block variables at depth " << depth << "\n";
        // Rename variables
        cfg->renameBlockVars(0, depth);

        printXML();

        // Print if requested
        if (VERBOSE) {      // was if debugPrintSSA
            LOG << "=== Debug Print SSA for " << getName()
              << " at memory depth " << depth << " (no propagations) ===\n";
            printToLog(true);
            LOG << "=== End Debug Print SSA for " <<
              getName() << " at depth " << depth << " ===\n\n";
        }
        
        if (depth == 0) {
            trimReturns();
        }
        if (depth == maxDepth) {
            fixCallRefs();
            processConstants();
            removeRedundantPhis();
        }
        fixCallRefs();
        // recognising globals early prevents them from becoming parameters
        if (depth == maxDepth)      // Else Sparc problems... MVE
            replaceExpressionsWithGlobals();
        int nparams = signature->getNumParams();
        if (depth > 0) {
            addNewParameters();
            //trimParameters(depth);
        }

        // if we've added new parameters, need to do propagations up to this
        // depth.  it's a recursive function thing.
        if (nparams != signature->getNumParams()) {
            for (int depth_tmp = 0; depth_tmp < depth; depth_tmp++) {
                // Propagate at this memory depth
                for (int td = maxDepth; td >= 0; td--) {
                    if (VERBOSE)
                        LOG << "parameter propagating at depth " << depth_tmp <<
                            " to depth " << td << "\n";
                    propagateStatements(depth_tmp, td);
                    for (int i = 0; i <= depth_tmp; i++)
                        cfg->renameBlockVars(0, i, true);
                }
            }
            cfg->renameBlockVars(0, depth, true);
            printXML();
            if (VERBOSE) {
                LOG << "=== Debug Print SSA for " << getName()
                  << " at memory depth " << depth
                  << " (after adding new parameters) ===\n";
                printToLog(true);
                LOG << "=== End Debug Print SSA for " <<
                  getName() << " at depth " << depth << " ===\n\n";
            }
        }
        // replacing expressions with Parameters as we go
        if (!Boomerang::get()->noParameterNames) {
            replaceExpressionsWithParameters(depth);
            cfg->renameBlockVars(0, depth, true);
        }

        // recognising locals early prevents them from becoming returns
        // But with indirect procs in a loop, the propagaton is not yet complete
        // replaceExpressionsWithLocals(depth == maxDepth);
        addNewReturns(depth);
        cfg->renameBlockVars(0, depth, true);
        printXML();
        if (VERBOSE) {
            LOG << "=== Debug Print SSA for " << getName()
              << " at memory depth " << depth
              << " (after adding new returns) ===\n";
            printToLog(true);
            LOG << "=== End Debug Print SSA for " <<
              getName() << " at depth " << depth << " ===\n\n";
        }
        trimReturns();
        fixCallRefs();

        printXML();
        // Print if requested
        if (VERBOSE) {      // was if debugPrintSSA
            LOG << "=== Debug Print SSA for " << getName() <<
              " at memory depth " << depth <<
              " (after trimming return set) ===\n";
            printToLog(true);
            LOG << "=== End Debug Print SSA for " <<
              getName() << " at depth " << depth << " ===\n\n";
        }

#define RESTART_DATAFLOW 0
        // Propagate at this memory depth
        bool convert;           // True when indirect call converted to direct
#if RESTART_DATAFLOW
        do {
#endif
            convert = false;
            for (int td = maxDepth; td >= 0; td--) {
                if (VERBOSE)
                    LOG << "propagating at depth " << depth << " to depth " 
                              << td << "\n";
                convert |= propagateStatements(depth, td);
                for (int i = 0; i <= depth; i++)
                    cfg->renameBlockVars(0, i, true);
            }
            // If you have an indirect to direct call conversion, some
            // propagations that were blocked by the indirect call might now
            // succeed, and may be needed to prevent alias problems
            if (VERBOSE && convert)
                LOG << "\nAbout to restart propagations and dataflow at depth "
                    << depth << " due to conversion of indirect to direct "
                    "call(s)\n\n";
#if RESTART_DATAFLOW
            if (convert) {
                depth = 0;      // Start again from depth 0
                stripRefs();
                LOG << "\nAfter strip:\n";
                printToLog(true);
                LOG << "\nDone after strip:\n\n";
                cfg->renameBlockVars(0, 0, true);    // Initial dataflow level 0
                LOG << "\nAfter initial rename:\n";
                printToLog(true);
                LOG << "\nDone after initial rename:\n\n";
            }

        } while (convert);
#endif

        printXML();
        if (VERBOSE) {
            LOG << "=== After propagate for " << getName() <<
              " at memory depth " << depth << " ===\n";
            printToLog(true);
            LOG << "=== End propagate for " << getName() <<
              " at depth " << depth << " ===\n\n";
        }
    }

    // Check for indirect jumps or calls not already removed by propagation of
    // constants
    if (cfg->decodeIndirectJmp(this)) {
        // There was at least one indirect jump or call found and decoded.
        // That means that most of what has been done to this function so far
        // is invalid. So redo everything. Very expensive!!
        LOG << "=== About to restart decompilation of " << 
          getName() << " because indirect jumps or calls have been removed\n\n";
        Analysis a;
        a.analyse(this);        // Get rid of this soon
        return decompile();     // Restart decompiling this proc
    }

    // Only remove unused statements after decompiling as much as possible of
    // the proc
    for (depth = 0; depth <= maxDepth; depth++) {
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

        printXML();
        if (VERBOSE && !Boomerang::get()->noRemoveNull) {
            LOG << "===== After removing null and unused statements "
              "=====\n";
            printToLog(true);
            LOG << "===== End after removing unused "
              "statements =====\n\n";
        }
    }

    if (!Boomerang::get()->noParameterNames) {
        for (int i = maxDepth; i >= 0; i--) {
            replaceExpressionsWithParameters(i);
            replaceExpressionsWithLocals(true);
            cfg->renameBlockVars(0, i, true);
        }
        trimReturns();
        fixCallRefs();
        trimParameters();
        if (VERBOSE) {
            LOG << "=== After replacing expressions, trimming params "
              "and returns ===\n";
            printToLog(true);
            LOG << "=== End after replacing expressions, trimming params "
              "and returns ===\n";
            LOG << "===== End after replacing params =====\n\n";
        }
    }

    processConstants();

    printXML();

    decompiled = true;          // Now fully decompiled (apart from one final
                                // pass, and transforming out of SSA form)
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
    int maxDepth = 0;
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        // Assume only need to check assignments
        if (s->getKind() == STMT_ASSIGN) {
            int depth = ((Assign*)s)->getMemDepth();
            if (depth > maxDepth) maxDepth = depth;
        }
    }
    return maxDepth;
}

void UserProc::removeRedundantPhis() {
    if (VERBOSE || DEBUG_UNUSED_STMT)
        LOG << "removing redundant phi statements" << "\n";

    // some phis are just not used
    RefCounter refCounts;
    countRefs(refCounts);

    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->isPhi()) {
            bool unused = false;
            PhiExp *p = (PhiExp*)s->getRight();
            if (refCounts[s] == 0)
                unused = true;
            else if (refCounts[s] == 1) {
                /* This looks pretty good, if all the statements in a phi
                 * are either NULL or a call to this proc, then 
                 * the phi is redundant.  However, we only remove it if 
                 * the only use is in the return statement.
                 */
                RefExp *r = new RefExp(s->getLeft()->clone(), s);
                bool usedInRet = false;
                if (theReturnStatement)
                    usedInRet = theReturnStatement->usesExp(r);
                delete r;
                if (usedInRet) {
                    bool allZeroOrSelfCall = true;
                    StatementVec::iterator it1;
                    for (it1 = p->begin(); it1 != p->end(); it1++) {
                        Statement* s1 = *it1;
                        if (s1 && (!s1->isCall() || 
                              ((CallStatement*)s1)->getDestProc() != this))
                            allZeroOrSelfCall = false;
                    }
                    if (allZeroOrSelfCall) {
                        if (VERBOSE || DEBUG_UNUSED_STMT)
                            LOG << "removing phi using shakey hack:\n";
                        unused = true;
                        removeReturn(p->getSubExp1());
                    }
                }
            } else {
#if 0       // NO! This essentially knocks out ALL the phis (I think). MVE
            // More thought required.
                // Check to see if all the refs are to either the same thing
                // or to NULL. If so, they will (would have been?) removed in
                // the fromSSA code. Removing them here allows some pesky
                // statements (e.g. assignments to %pc) to be removed
                LocationSet refs;
                p->addUsedLocs(refs);
                Exp* first = *refs.begin();
                bool same = true;
                LocationSet::iterator rr;
                for (rr = refs.begin(); rr != refs.end(); rr++) {
                    if (!(**rr *= *first)) {       // Ref-insensitive compare
                        same = false;
                        break;
                    }
                }
                if (same) {
                    // Is the left of the phi assignment the same base variable
                    // as all the operands (or the operand is NULL) ?
                    if (*s->getLeft() *= *first) {
                        if (DEBUG_UNUSED_STMT)
                            LOG << "Removing phi: left and all refs same or 0: "
                              << s << "\n";
                        // Just removing the refs will work, or removing the
                        // whole phi
                        removeStatement(s);
                    }
                }
#endif
            }
            if (unused) {
                if (DEBUG_UNUSED_STMT)
                    LOG << "removing redundant phi " << s << "\n";
                removeStatement(s);
            }
        }
    }

#if 0
    stmts.clear();
    getStatements(stmts);

    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = (Statement*)*it;
        if (s->isPhi()) {
            if (VERBOSE) {
                LOG << "checking " << s << "\n";
            }
            // if we can prove that all the statements in the phi define
            // equal values then we can replace the phi with any one of 
            // the values, but there's not much point if they're all calls
            PhiExp *p = (PhiExp*)s->getRight();
            bool hasphi = false;
            StatementVec::iterator it;
            for (it = p->begin(); it != p->end(); it++)
                if (*it && (*it)->isPhi()) {
                    hasphi = true;
                }
            if (hasphi) {
                if (VERBOSE)
                    LOG << "contains a ref to a phi statement (skipping)\n";
                // Do we need this?
                //continue;
            }
            bool allsame = true;
            it = p->begin();
            assert(it != p->end());
            Statement* s1 = *it;
            Statement* noncall = s1;
            if (it != p->end())
                for (it++; it != p->end(); it++) {
                    Statement* s2 = *it;
                    if (noncall && s2 && 
                        (noncall->isCall()
                                || s2->getNumber() < noncall->getNumber()) && 
                        !s2->isCall() && s2 != s)
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
#endif
}

void UserProc::trimReturns() {
    std::set<Exp*> preserved;
    bool stdsp = false;
    bool stdret = false;

    if (VERBOSE)
        LOG << "Trimming return set for " << getName() << "\n";

    int sp = signature->getStackRegister(prog);

    for (int n = 0; n < 2; n++) {   
        // may need to do multiple times due to dependencies

        // Special case for 32-bit stack-based machines (e.g. Pentium).
        // RISC machines generally preserve the stack pointer (so no special
        // case required)
        for (int p = 0; !stdsp && p < 8; p++) {
            if (DEBUG_PROOF)
                LOG << "attempting to prove sp = sp + " << p*4 << 
                             " for " << getName() << "\n";
            stdsp = prove(new Binary(opEquals,
                          Location::regOf(sp),
                          new Binary(opPlus,
                              Location::regOf(sp),
                              new Const(p * 4))));
        }

        // Prove that pc is set to the return value
        if (DEBUG_PROOF)
            LOG << "attempting to prove %pc = m[sp]\n";
        stdret = prove(new Binary(opEquals, new Terminal(opPC), 
                       Location::memOf(Location::regOf(sp))));

        // prove preservation for each parameter
        for (int i = 0; i < signature->getNumReturns(); i++) {
            Exp *p = signature->getReturnExp(i);
            Exp *e = new Binary(opEquals, p->clone(), p->clone());
            if (DEBUG_PROOF)
                LOG << "attempting to prove " << p << " is preserved by " 
                          << getName() << "\n";
            if (prove(e)) {
                preserved.insert(p);    
            }
        }
    }
    if (stdsp) {
        Unary *regsp = Location::regOf(sp);
        // I've been removing sp from the return set as it makes 
        // the output look better, but this only works for recursive
        // procs (because no other proc call them and fixCallRefs can
        // replace refs to the call with a valid expression).  Not
        // removing sp will make basically every procedure that doesn't
        // preserve sp return it, and take it as a parameter.  Maybe a 
        // later pass can get rid of this.  Trent 22/8/2003
        // We handle this now by doing a final pass which removes any
        // unused returns of a procedure.  So r28 will remain in the 
        // returns set of every procedure in the program until such
        // time as this final pass is made, and then they will be 
        // removed.  Trent 22/9/2003
        // Instead, what we can do is replace r28 in the return statement
        // of this procedure with what we've proven it to be.
        //removeReturn(regsp);
        // also check for any locals that slipped into the returns
        for (int i = 0; i < signature->getNumReturns(); i++) {
            Exp *e = signature->getReturnExp(i);
            if (e->getOper() == opMemOf && 
                e->getSubExp1()->getOper() == opMinus &&
                *e->getSubExp1()->getSubExp1() == *regsp &&
                e->getSubExp1()->getSubExp2()->isIntConst())
                preserved.insert(e);
            if (*e == *regsp) {
                assert(theReturnStatement);
                Exp *e = getProven(regsp)->clone();
                e = e->expSubscriptVar(new Terminal(opWild), NULL);
                if (!(*e == *theReturnStatement->getReturnExp(i))) {
                    if (VERBOSE)
                        LOG << "replacing in return statement " 
                            << theReturnStatement->getReturnExp(i) <<
                            " with " << e << "\n";
                    theReturnStatement->setReturnExp(i, e);
                }
            }
        }
    }
    if (!signature->isPromoted()) {
        if (stdret)
            removeReturn(new Terminal(opPC));
        for (std::set<Exp*>::iterator it = preserved.begin(); 
             it != preserved.end(); it++)
            removeReturn(*it);
    }
    removeRedundantPhis();
}

void UserProc::fixCallRefs()
{
    if (VERBOSE)
        LOG << "\nfixCallRefs for " << getName() << "\n";
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        s->fixCallRefs();
    }
    simplify();
    if (VERBOSE)
        LOG << "end fixCallRefs for " << getName() << "\n\n";
}

void UserProc::addNewReturns(int depth) {

    if (signature->isPromoted())
        return;

    if (VERBOSE)
        LOG << "Adding new returns for " << getName() << "\n";

    StatementList stmts;
    getStatements(stmts);

    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = (Statement*)*it;
        Exp *left = s->getLeft();
        if (left) {
            bool allZero = true;
            Exp *e = left->clone()->removeSubscripts(allZero);
            if (allZero && signature->findReturn(e) == -1 &&
                getProven(e) == NULL) {
                if (e->getOper() == opLocal) {
                    if (VERBOSE)
                        LOG << "ignoring local " << e << "\n";
                    continue;
                }
                if (e->getOper() == opGlobal) {
                    if (VERBOSE)
                        LOG << "ignoring global " << e << "\n";
                    continue;
                }
                if (e->getOper() == opRegOf && 
                    e->getSubExp1()->getOper() == opTemp) {
                    if (VERBOSE)
                        LOG << "ignoring temp " << e << "\n";
                    continue;
                }
                if (e->getOper() == opFlags) {
                    if (VERBOSE)
                        LOG << "ignoring flags " << e << "\n";
                    continue;
                }
                if (e->getMemDepth() != depth) {
                    continue;
                }
                if (VERBOSE)
                    LOG << "Found new return " << e << "\n";
                addReturn(e);
            }
        }
    }
}

// m[WILD]{0}
static RefExp *memOfWild = new RefExp(
    Location::memOf(new Terminal(opWild)), NULL);
// r[WILD INT]{0}
static RefExp* regOfWild = new RefExp(
    Location::regOf(new Terminal(opWildIntConst)), NULL);

void UserProc::addNewParameters() {

    if (signature->isPromoted())
        return;

    if (VERBOSE)
        LOG << "Adding new parameters for " << getName() << "\n";

    StatementList stmts;
    getStatements(stmts);

    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        // For now, assume that all parameters will be m[]{0} or r[]{0}
        // (Seems pretty reasonable)
        std::list<Exp*> results;
        s->searchAll(memOfWild, results);
        s->searchAll(regOfWild, results);
        while (results.size()) {
            bool allZero;
            Exp *e = results.front()->clone()->removeSubscripts(allZero);
            results.erase(results.begin());     // Remove first result
            if (allZero && signature->findParam(e) == -1
                  // ? Often need to transfer from implit to explicit:
                  // && signature->findImplicitParam(e) == -1
                  ) {
                if (signature->isStackLocal(prog, e) ||
                      e->getOper() == opLocal)  {
                    if (VERBOSE)
                        LOG << "ignoring local " << e << "\n";
                    continue;
                }
                if (e->getOper() == opGlobal) {
                    if (VERBOSE)
                        LOG << "ignoring global " << e << "\n";
                    continue;
                }
                if (e->getMemDepth() > 1) {
                    if (VERBOSE)
                        LOG << "ignoring complex " << e << "\n";
                    continue;
                }
                if (e->getOper() == opMemOf && 
                    e->getSubExp1()->getOper() == opGlobal) {
                    if (VERBOSE)
                        LOG << "ignoring m[global] " << e << "\n";
                    continue;
                }
                if (e->getOper() == opMemOf &&
                    e->getSubExp1()->getOper() == opParam) {
                    if (VERBOSE)
                        LOG << "ignoring m[param] " << e << "\n";
                    continue;
                }
                if (e->getOper() == opMemOf &&
                    e->getSubExp1()->getOper() == opPlus &&
                    e->getSubExp1()->getSubExp1()->getOper() == opGlobal &&
                    e->getSubExp1()->getSubExp2()->getOper() == opIntConst) {
                    if (VERBOSE)
                        LOG << "ignoring m[global + int] " << e << "\n";
                    continue;
                }
#if 1
                if ((e->getOper() != opMemOf ||
                    e->getSubExp1()->getOper() != opPlus ||
                    !(*e->getSubExp1()->getSubExp1() == *Location::regOf(28)) ||
                    e->getSubExp1()->getSubExp2()->getOper() != opIntConst)
                    && e->getOper() != opRegOf) {
                    if (VERBOSE)
                        LOG << "ignoring non pentium " << e << "\n";
                    continue;
                }
#endif
                if (VERBOSE)
                    LOG << "Found new parameter " << e << "\n";
                addParameter(e);
            }
        }
    }
}

void UserProc::trimParameters(int depth) {

    if (signature->isPromoted())
        return;

    if (VERBOSE)
        LOG << "Trimming parameters for " << getName() << "\n";

    StatementList stmts;
    getStatements(stmts);

    // find parameters that are referenced (ignore calls to this)
    int nparams = signature->getNumParams();
    int totparams = nparams + signature->getNumImplicitParams();
    std::vector<Exp*> params;
    bool referenced[64];
    assert(totparams <= (int)(sizeof(referenced)/sizeof(bool)));
    int i;
    for (i = 0; i < nparams; i++) {
        referenced[i] = false;
        // We want the 
        params.push_back(signature->getParamExp(i)->clone()->
                            expSubscriptVar(new Terminal(opWild), NULL));
    }
    for (i = 0; i < signature->getNumImplicitParams(); i++) {
        referenced[i + nparams] = false;
        params.push_back(signature->getImplicitParamExp(i)->clone()->
                            expSubscriptVar(new Terminal(opWild), NULL));
    }

    std::set<Statement*> excluded;
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->isCall() && ((CallStatement*)s)->getDestProc() == this) {
            CallStatement *call = (CallStatement*)s;
            for (int i = 0; i < signature->getNumImplicitParams(); i++) {
                Exp *e = call->getImplicitArgumentExp(i);
                if (e->isSubscript()) {
                    Statement *ref = ((RefExp*)e)->getRef();
                    if (ref != NULL)
                        excluded.insert(ref);
                }
            }
        }
    }
    

    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (!s->isCall() || ((CallStatement*)s)->getDestProc() != this) {
            for (int i = 0; i < totparams; i++) {
                Exp *p, *pe;
                if (i < nparams) {
                    p = Location::param(signature->getParamName(i), this);
                    pe = signature->getParamExp(i);
                } else {
                    p = Location::param(signature->getImplicitParamName(
                                i - nparams), this);
                    pe = signature->getImplicitParamExp(i - nparams);
                }
                if (!referenced[i] && excluded.find(s) == excluded.end() && 
                    // Search for the named parameter (e.g. param1), and just
                    // in case, also for the expression (e.g. r8{0})
                    (s->usesExp(p) || s->usesExp(params[i])))
                    referenced[i] = true;
                if (!referenced[i] && excluded.find(s) == excluded.end() &&
                    s->isPhi() && *s->getLeft() == *pe) {
                    if (VERBOSE)
                        LOG << "searching " << s << " for uses of " 
                                  << params[i] << "\n";
                    PhiExp *ph = (PhiExp*)s->getRight();
                    StatementVec::iterator it1;
                    for (it1 = ph->begin(); it1 != ph->end(); it1++)
                        if (*it1 == NULL) {
                            referenced[i] = true;
                            break;
                        }
                }
                delete p;
            }
        }
    }

    for (i = 0; i < totparams; i++) {
        if (!referenced[i] && (depth == -1 || 
              params[i]->getMemDepth() == depth)) {
            bool allZero;
            Exp *e = params[i]->removeSubscripts(allZero);
            if (VERBOSE) 
                LOG << "removing unused parameter " << e << "\n";
            removeParameter(e);
        }
    }
}

void Proc::removeReturn(Exp *e)
{
    signature->removeReturn(e);
    for (std::set<CallStatement*>::iterator it = callerSet.begin();
         it != callerSet.end(); it++) {
            if (VERBOSE)
                LOG << "removing return " << e << " from " << *it 
                          << "\n";
            (*it)->removeReturn(e);
    }
}

void UserProc::removeReturn(Exp *e)
{
    int n = signature->findReturn(e);
    if (n != -1) {
        Proc::removeReturn(e);
        if (theReturnStatement)
            theReturnStatement->removeReturn(n);
    }
}

void Proc::removeParameter(Exp *e)
{
    int n = signature->findParam(e);
    if (n != -1) {
        signature->removeParameter(n);
        for (std::set<CallStatement*>::iterator it = callerSet.begin();
             it != callerSet.end(); it++) {
            if (VERBOSE)
                LOG << "removing argument " << e << " in pos " << n 
                          << " from " << *it << "\n";
            (*it)->removeArgument(n);
        }
    }
    n = signature->findImplicitParam(e);
    if (n != -1) {
        signature->removeImplicitParameter(n);
        for (std::set<CallStatement*>::iterator it = callerSet.begin();
             it != callerSet.end(); it++) {
            if (VERBOSE)
                LOG << "removing implicit argument " << e << " in pos " << n 
                          << " from " << *it << "\n";
            (*it)->removeImplicitArgument(n);
        }
    }
}

void Proc::addReturn(Exp *e)
{
    for (std::set<CallStatement*>::iterator it = callerSet.begin();
         it != callerSet.end(); it++)
            (*it)->addReturn(e);
    signature->addReturn(e);
}

void UserProc::addReturn(Exp *e)
{
    Exp *e1 = e->clone();
    if (e1->getOper() == opMemOf) {
        e1->refSubExp1() = e1->getSubExp1()->expSubscriptVar(
                                                new Terminal(opWild), NULL);
    }
    if (theReturnStatement)
        theReturnStatement->addReturn(e1);
    Proc::addReturn(e);
}

void Proc::addParameter(Exp *e)
{
    // In case it's already an implicit argument:
    removeParameter(e);

    for (std::set<CallStatement*>::iterator it = callerSet.begin();
         it != callerSet.end(); it++)
            (*it)->addArgument(e);
    signature->addParameter(e);
}

void UserProc::processFloatConstants()
{
    StatementList stmts;
    getStatements(stmts);

    Exp *match = new Ternary(opFsize, new Terminal(opWild), 
                                      new Terminal(opWild), 
                                Location::memOf(new Terminal(opWild)));
    
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement *s = *it;

        std::list<Exp*> results;
        s->searchAll(match, results);
        for (std::list<Exp*>::iterator it1 = results.begin(); 
                                       it1 != results.end(); it1++) {
            Ternary *fsize = (Ternary*) *it1;
            if (fsize->getSubExp3()->getOper() == opMemOf &&
                fsize->getSubExp3()->getSubExp1()->getOper() 
                    == opIntConst) {
                Exp *memof = fsize->getSubExp3();
                ADDRESS u = ((Const*)memof->getSubExp1())->getInt();
                bool ok;
                double d = prog->getFloatConstant(u, ok);
                if (ok) {
                    LOG << "replacing " << memof << " with " << d 
                        << " in " << fsize << "\n";
                    fsize->setSubExp3(new Const(d));
                }
            }
        }
        s->simplify();
    }
}

void UserProc::replaceExpressionsWithGlobals() {
    StatementList stmts;
    getStatements(stmts);

    if (VERBOSE)
        LOG << "replacing expressions with globals\n";

    // start with calls because that's where we have the most types
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) 
        if ((*it)->isCall()) {
            CallStatement *call = (CallStatement*)*it;
            for (int i = 0; i < call->getNumArguments(); i++) {
                Type *ty = call->getArgumentType(i);
                Exp *e = call->getArgumentExp(i);
                // Temporary: the below assumes that the address of a global is
                // an integer constant
                if (ty && ty->resolvesToPointer() && 
                      e->getOper() == opIntConst) {
                    Type *pty = ty->asPointer()->getPointsTo();
                    if (pty->resolvesToArray() && 
                        pty->asArray()->isUnbounded()) {
                        ArrayType *a = (ArrayType*)pty->asArray()->clone();
                        pty = a;
                        a->setLength(1024);   // just something arbitrary
                        if (i+1 < call->getNumArguments()) {
                            Type *nt = call->getArgumentType(i+1);
                            if (nt->isNamed())
                                nt = ((NamedType*)nt)->resolvesTo();
                            if (nt->isInteger() && call->getArgumentExp(i+1)->
                              isIntConst())
                                a->setLength(((Const*)call->
                                  getArgumentExp(i+1))->getInt());
                        }
                    }
                    ADDRESS u = ((Const*)e)->getInt();
                    prog->globalUsed(u);
                    const char *gloName = prog->getGlobalName(u);
                    if (gloName) {
                        ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
                        Exp *ne;
                        if (r) {
                            Location *g = Location::global(strdup(gloName),
                              this);
                            ne = new Binary(opPlus,
                                new Unary(opAddrOf, g),
                                new Const(r));
                        } else {
                            prog->setGlobalType((char*)gloName, pty);
                            Location *g = Location::global(strdup(gloName),
                              this);
                            ne = new Unary(opAddrOf, g);
                        }
                        call->setArgumentExp(i, ne);
                        if (VERBOSE)
                            LOG << "replacing argument " << e << " with " <<
                              ne << " in " << call << "\n";
                    }
                }
            }
        }


    // replace expressions with symbols
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;

        // (a) Definitions
        LocationSet defs;
        s->getDefinitions(defs);
        LocationSet::iterator rr;
        for (rr = defs.begin(); rr != defs.end(); rr++) {
            if ((*rr)->getOper() == opMemOf &&
                (*rr)->getSubExp1()->getOper() == opIntConst) {
                Exp *memof = *rr;
                ADDRESS u = ((Const*)memof->getSubExp1())->getInt();
                prog->globalUsed(u);
                const char *gloName = prog->getGlobalName(u);
                if (gloName) {
                    ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
                    Exp *ne;
                    if (r) {
                        Location *g = Location::global(strdup(gloName), this);
                        ne = Location::memOf(
                            new Binary(opPlus,
                                new Unary(opAddrOf, g),
                                new Const(r)), this);
                    } else {
                        Type *ty = prog->getGlobalType((char*)gloName);
                        if (s->isAssign() && ((Assign*)s)->getType()) {
                            int bits = ((Assign*)s)->getType()->getSize();
                            if (ty == NULL || ty->getSize() == 0)
                                prog->setGlobalType((char*)gloName,
                                    new IntegerType(bits));
                        }
                        ty = prog->getGlobalType((char*)gloName);
                        Location *g = Location::global(strdup(gloName), this);
                        if (ty && ty->isArray()) 
                            ne = new Binary(opArraySubscript, g, new Const(0));
                        else 
                            ne = g;
                    }
                    s->searchAndReplace(memof->clone(), ne);
                }
            }
        }

        // (b) Uses
        LocationSet refs;
        s->addUsedLocs(refs);
        for (rr = refs.begin(); rr != refs.end(); rr++) {
            if (((Exp*)*rr)->isSubscript()) {
                Statement *ref = ((RefExp*)*rr)->getRef();
                Exp *r1 = (*rr)->getSubExp1();
                // Is it m[CONSTANT]{0}
                if (ref == NULL && 
                    r1->getOper() == opMemOf &&
                    r1->getSubExp1()->getOper() == opIntConst) {
                    Exp *memof = r1;
                    ADDRESS u = ((Const*)memof->getSubExp1())->getInt();
                    prog->globalUsed(u);
                    const char *gloName = prog->getGlobalName(u);
                    if (gloName) {
                        ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
                        Exp *ne;
                        if (r) {
                            Unary *g = Location::global(strdup(gloName), this);
                            ne = Location::memOf(
                                new Binary(opPlus,
                                    new Unary(opAddrOf, g),
                                    new Const(r)), this);
                        } else {
                            Type *ty = prog->getGlobalType((char*)gloName);
                            Unary *g = Location::global(strdup(gloName), this);
                            if (ty && ty->isArray()) 
                                ne = new Binary(opArraySubscript,
                                    g,
                                    new Const(0));
                            else 
                                ne = g;
                        }
                        s->searchAndReplace(memof->clone(), ne);
                    }
                // look for m[(blah * K1 + K2)]
                } else if (ref == NULL && r1->getOper() == opMemOf &&
                       r1->getSubExp1()->getOper() == opPlus &&
                       r1->getSubExp1()->getSubExp1()->getOper() 
                                                      == opMult &&
                       r1->getSubExp1()->getSubExp1()->getSubExp2()
                                          ->getOper() == opIntConst &&
                       r1->getSubExp1()->getSubExp2()->getOper() 
                                                            == opIntConst) {
                    Exp *memof = r1;
                    // K1 is the stride
                    int stride = ((Const*)memof->getSubExp1()->getSubExp1()
                                               ->getSubExp2())->getInt();
                    // u is K2
                    ADDRESS u = ((Const*)memof->getSubExp1()->getSubExp2())
                                                            ->getInt();
                    LOG << "detected array ref with stride " << stride << "\n";
                    prog->globalUsed(u);
                    const char *gloName = prog->getGlobalName(u);
                    if (gloName) {
                        ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
                        Exp *ne = NULL;
                        if (r) {
                            // TOO HARD
                        } else {
                            Type *ty = prog->getGlobalType((char*)gloName);
                            Location *g = Location::global(strdup(gloName), this);
                            if (ty == NULL || ty->getSize() == 0) {
                                LOG << "setting type of global to array\n";
                                ty = new ArrayType(new IntegerType(stride*8),1);
                                prog->setGlobalType((char*)gloName, ty);
                            }

                            if (ty)
                                LOG << "got type: " << ty->getCtype() << "\n";

                            if (ty && ty->isArray() && 
                                ty->asArray()->getBaseType()->getSize() 
                                                            != stride*8) {
                                LOG << "forcing array base type size to stride\n";
                                ty->asArray()->setLength(ty->asArray()->getLength() *
                                    ty->asArray()->getBaseType()->getSize() /
                                    (stride * 8));
                                ty->asArray()->setBaseType(new IntegerType(stride*8));
                                prog->setGlobalType((char*)gloName, ty);
                            }

                            if (ty)
                                LOG << "got type: " << ty->getCtype() << "\n";

                            if (ty && ty->isArray() && 
                                ty->asArray()->getBaseType()->getSize() 
                                                            == stride*8) {
                                LOG << "setting new exp to array ref\n";
                                ne = new Binary(opArraySubscript, g, 
                                            memof->getSubExp1()->getSubExp1()->getSubExp1()
                                                               ->clone());
                                LOG << "set to " << ne << "\n";
                            }
                            /* else 
                                ne = Location::memOf(new Binary(opPlus, 
                                        new Unary(opAddrOf, g), 
                                        memof->getSubExp1()->getSubExp1()->clone()
                                        )); */
                        }
                        if (ne)
                            s->searchAndReplace(memof->clone(), ne);
                    }
                }
            }
        }

        s->simplify();
    }
}

void UserProc::replaceExpressionsWithSymbols() {
    StatementList stmts;
    getStatements(stmts);

    // replace expressions in regular statements with symbols
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        for (std::map<Exp*, Exp*,lessExpStar>::iterator it1 = symbolMap.begin();
          it1 != symbolMap.end(); it1++) {
            bool ch = s->searchAndReplace((*it1).first, (*it1).second);
            if (ch && VERBOSE) {
                LOG << "std stmt: replace " << (*it1).first <<
                  " with " << (*it1).second << " result " << s << "\n";
            }
        }
    }
}

void UserProc::replaceExpressionsWithParameters(int depth) {
    StatementList stmts;
    getStatements(stmts);

    if (VERBOSE)
        LOG << "replacing expressions with parameters at depth " << depth 
            << "\n";

    bool found = false;
    // start with calls because that's where we have the most types
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        if ((*it)->isCall()) {
            CallStatement *call = (CallStatement*)*it;
            for (int i = 0; i < call->getNumArguments(); i++) {
                Type *ty = call->getArgumentType(i);
                Exp *e = call->getArgumentExp(i);
                if (ty && ty->resolvesToPointer() && e->getOper() != opAddrOf 
                       && e->getMemDepth() == 0) {
                    // Check for an expression representing the address of a
                    // local variable. NOTE: machine dependent
                    if (signature->isAddrOfStackLocal(prog, e)) {
                        // don't do locals here!
                        continue;
                    }

                    Location *pe = Location::memOf(e, this);
                    Exp *ne = new Unary(opAddrOf, pe);
                    if (VERBOSE)
                        LOG << "replacing argument " << e << " with " << ne <<
                          " in " << call << "\n";
                    call->setArgumentExp(i, ne);
                    found = true;
                }
            }
        }
    }
    if (found) {
        // Must redo all the subscripting!
        for (int d=0; d <= depth; d++)
            cfg->renameBlockVars(0, d /* Memory depth */, true);
    }

    // replace expressions in regular statements with parameters
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        for (int i = 0; i < signature->getNumParams(); i++) {
            if (signature->getParamExp(i)->getMemDepth() == depth ||
                  depth < 0) {
                Exp *r = signature->getParamExp(i)->clone();
                r = r->expSubscriptVar(new Terminal(opWild), NULL);
                if (r->getOper() == opSubscript)
                    r = r->getSubExp1();
                Exp* replace = Location::param(
                            strdup((char*)signature->getParamName(i)), this);
                Exp *n;
                if (s->search(r, n)) {
                    if (VERBOSE)
                        LOG << "replacing " << r << " with " << replace << 
                          " in " << s << "\n";
                    s->searchAndReplace(r, replace);
                    if (VERBOSE)
                        LOG << "after: " << s << "\n";
                }
            }
        }
    }
}

Exp *UserProc::getLocalExp(Exp *le, Type *ty)
{
    Exp *e = NULL;
    if (symbolMap.find(le) == symbolMap.end()) {
        if (le->getOper() == opMemOf && le->getSubExp1()->getOper() == opMinus
              && *le->getSubExp1()->getSubExp1() == *new 
              RefExp(Location::regOf(signature->getStackRegister(prog)), NULL)
              && le->getSubExp1()->getSubExp2()->getOper() == opIntConst) {
            int le_n = ((Const*)le->getSubExp1()->getSubExp2())->getInt();
            // now test all the locals to see if this expression 
            // is an alias to one of them (for example, a member
            // of a compound typed local)
            for (std::map<Exp*, Exp*,lessExpStar>::iterator it =
                 symbolMap.begin(); it != symbolMap.end(); it++) {
                Exp *base = (*it).first;
                assert(base);
                Exp *local = (*it).second;
                assert(local->getOper() == opLocal &&
                       local->getSubExp1()->getOper() == opStrConst);
                std::string name = ((Const*)local->getSubExp1())->getStr();
                Type *ty = locals[name];
                assert(ty);
                int size = ty->getSize() / 8;    // getSize() returns bits!
                if (base->getOper() == opMemOf && 
                    base->getSubExp1()->getOper() == opMinus &&
                    *base->getSubExp1()->getSubExp1() == 
                         *new RefExp(Location::regOf(
                            signature->getStackRegister(prog)), NULL) &&
                    base->getSubExp1()->getSubExp2()->getOper() == 
                                                            opIntConst) {
                    int base_n = ((Const*)base->getSubExp1()->getSubExp2())
                                                            ->getInt();
                    if (le_n <= base_n && le_n > base_n-size) {
                        if (VERBOSE)
                            LOG << "found alias to " << name.c_str() << ": "
                                << le << "\n";
                        int n = base_n - le_n;
                        return Location::memOf(
                            new Binary(opPlus, 
                                new Unary(opAddrOf, 
                                    local->clone()),
                                new Const(n)), this);
#if 0
                        if (ty->resolvesToCompound()) {
                            CompoundType *compound = ty->asCompound();
                            return new Binary(opMemberAccess, local->clone(), 
                               new Const((char*)compound->getNameAtOffset(
                               (base_n - le_n)*8)));
                        } else
                            assert(false);
#endif
                    }
                }
            }
        }
            
        if (ty) {
            // the default of just assigning an int type is bad.. 
            // if the locals is not an int then assigning it this type 
            // early results in aliases to this local not being recognised 
            e = newLocal(ty->clone());
            symbolMap[le->clone()] = e;
            e->clone();
        }
    } else {
        e = symbolMap[le]->clone();
        if (e->getOper() == opLocal && e->getSubExp1()->getOper() == opStrConst)
        {
            std::string name = ((Const*)e->getSubExp1())->getStr();
            Type *nty = ty;
            Type *ty = locals[name];
            assert(ty);
            if (nty && !(*ty == *nty) && nty->getSize() >= ty->getSize()) {
                if (VERBOSE)
                    LOG << "updating type of " << name.c_str() << " to " 
                        << nty->getCtype() << "\n";
                ty = nty;
                locals[name] = ty;
            }
            if (ty->resolvesToCompound()) {
                CompoundType *compound = ty->asCompound();
                if (VERBOSE)
                    LOG << "found reference to first member of compound " <<
                      name.c_str() << ": " << le << "\n";
                char* nam = (char*)compound->getName(0);
                if (nam == NULL) nam = "??";
                return new Binary(opMemberAccess, e, new Const(nam));
            }
        }
    }
    return e;
}

void UserProc::replaceExpressionsWithLocals(bool lastPass) {
    StatementList stmts;
    getStatements(stmts);

    if (VERBOSE) {
        LOG << "replacing expressions with locals";
        if (lastPass)
            LOG << " last pass";
        LOG << "\n";
    }

    int sp = signature->getStackRegister(prog);
    if (getProven(Location::regOf(sp)) == NULL) {
        if (VERBOSE)
            LOG << "Can't replace locals since sp unproven\n";
        return;    // can't replace if nothing proven about sp
    }

    // start with calls because that's where we have the most types
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) 
        if ((*it)->isCall()) {
            CallStatement *call = (CallStatement*)*it;
            for (int i = 0; i < call->getNumArguments(); i++) {
                Type *ty = call->getArgumentType(i);
                Exp *e = call->getArgumentExp(i);
                // If a pointer type and e is of the form m[sp{0} - K]:
                if (ty && ty->resolvesToPointer() && 
                      e->getOper() == opMinus && 
                      *e->getSubExp1() == *new RefExp(Location::regOf(sp), NULL)
                      && e->getSubExp2()->getOper() == opIntConst) {
                    Exp *olde = e->clone();
                    Type *pty = ty->asPointer()->getPointsTo();
                    if (pty->resolvesToArray() && 
                        pty->asArray()->isUnbounded()) {
                        ArrayType *a = (ArrayType*)pty->asArray()->clone();
                        pty = a;
                        a->setLength(1024);   // just something arbitrary
                        if (i+1 < call->getNumArguments()) {
                            Type *nt = call->getArgumentType(i+1);
                            if (nt->isNamed())
                                nt = ((NamedType*)nt)->resolvesTo();
                            if (nt->isInteger() && call->getArgumentExp(i+1)->
                              isIntConst())
                                a->setLength(((Const*)call->getArgumentExp(i+1))
                                  ->getInt());
                        }
                    }
                    e = getLocalExp(Location::memOf(e->clone(), this), pty);
                    if (e) {
                        Exp *ne = new Unary(opAddrOf, e);
                        if (VERBOSE)
                            LOG << "replacing argument " << olde << " with "
                                << ne << " in " << call << "\n";
                        call->setArgumentExp(i, ne);
                    }
                }
            }
        }

    // look for array locals
    // l = m[(sp{0} + WILD1) - K2]
    Exp *l = Location::memOf(new Binary(opMinus, 
                new Binary(opPlus,
                   new RefExp(Location::regOf(sp), NULL),
                   new Terminal(opWild)),
                new Terminal(opWildIntConst)));
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        std::list<Exp*> results;
        s->searchAll(l, results);
        for (std::list<Exp*>::iterator it1 = results.begin(); 
                                       it1 != results.end(); it1++) {
            Exp *result = *it1;
            // arr = m[sp{0} - K2]
            Location *arr = Location::memOf(new Binary(opMinus, 
                          new RefExp(Location::regOf(sp), NULL),
                          result->getSubExp1()->getSubExp2()->clone()));
            int n = ((Const*)result->getSubExp1()->getSubExp2())->getInt();
            arr->setProc(this);
            arr->setType(new ArrayType(new IntegerType(), n/4));
            if (VERBOSE)
                LOG << "found a local array using " << n << " bytes\n";
            Exp *replace = Location::memOf(new Binary(opPlus,
                new Unary(opAddrOf, arr),
                result->getSubExp1()->getSubExp1()->getSubExp2()->clone()),
                  this);
            if (VERBOSE)
                LOG << "replacing " << result << " with " << replace <<
                    " in " << s << "\n";
            s->searchAndReplace(result->clone(), replace);
        }
    }

    // Stack offsets for local variables could be negative (most machines),
    // positive (PA/RISC), or both (SPARC)
    if (signature->isLocalOffsetNegative())
        searchRegularLocals(opMinus, lastPass, sp, stmts);
    if (signature->isLocalOffsetPositive())
        searchRegularLocals(opPlus, lastPass, sp, stmts);
    // Ugh - m[sp] is a special case: neither positive or negative.
    // SPARC uses this to save %i0
    if (signature->isLocalOffsetPositive() &&
      signature->isLocalOffsetNegative())
        searchRegularLocals(opWild, lastPass, sp, stmts);

}

void UserProc::searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp,
      StatementList& stmts) {
    // replace expressions in regular statements with locals
    Location* l;
    if (minusOrPlus == opWild)
        // l = m[sp{0}]
        l = Location::memOf(
            new RefExp(Location::regOf(sp), NULL));
    else
        // l = m[sp{0} +/- K]
        l = Location::memOf(new Binary(minusOrPlus, 
            new RefExp(Location::regOf(sp), NULL),
            new Terminal(opWildIntConst)));
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        std::list<Exp*> results;
        s->searchAll(l, results);
        for (std::list<Exp*>::iterator it1 = results.begin(); 
                                       it1 != results.end(); it1++) {
            Exp *result = *it1;
            Type *ty = result->getType();
            if (ty == NULL && lastPass)
                ty = new IntegerType();
            Exp *e = getLocalExp(result, ty);
            if (e) {
                Exp* search = result->clone();
                if (VERBOSE)
                    LOG << "replacing " << search << " with " << e << " in " 
                        << s << "\n";
                s->searchAndReplace(search, e);
            }
        }
        s->simplify();
    }
}

bool UserProc::nameStackLocations() {
    Exp *match = signature->getStackWildcard();
    if (match == NULL) return false;

    bool found = false;
    StatementList stmts;
    getStatements(stmts);
    // create a symbol for every memory reference
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        Exp *memref; 
        if (s->search(match, memref)) {
            if (symbolMap.find(memref) == symbolMap.end()) {
                if (VERBOSE)
                    LOG << "stack location found: " << memref << "\n";
                symbolMap[memref->clone()] = newLocal(new IntegerType());
            }
            assert(symbolMap.find(memref) != symbolMap.end());
            std::string name = ((Const*)symbolMap[memref]->getSubExp1())
                    ->getStr();
            if (memref->getType() != NULL)
                locals[name] = memref->getType();
            locals[name] = s->updateType(memref, locals[name]);
            if (VERBOSE)
                LOG << "updating type of " << name.c_str() << " to " 
                    << locals[name]->getCtype() << "\n";
            found = true;
        }
    }
    delete match;
    return found;
}

bool UserProc::nameRegisters() {
    Exp *match = Location::regOf(new Terminal(opWild));
    if (match == NULL) return false;
    bool found = false;

    StatementList stmts;
    getStatements(stmts);
    // create a symbol for every register
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        Exp *memref; 
        if (s->search(match, memref)) {
            if (symbolMap.find(memref) == symbolMap.end()) {
                if (VERBOSE)
                    LOG << "register found: " << memref << "\n";
                symbolMap[memref->clone()] = newLocal(new IntegerType());
            }
            assert(symbolMap.find(memref) != symbolMap.end());
            std::string name = ((Const*)symbolMap[memref]->getSubExp1())->
              getStr();
            if (memref->getType() != NULL)
                locals[name] = memref->getType();
            locals[name] = s->updateType(memref, locals[name]);
            found = true;
            if (VERBOSE)
                LOG << "updating type of " << name.c_str() << " to " 
                    << locals[name]->getCtype() << "\n";
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
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->isNullStatement()) {
            // A statement of the form x := x
            if (VERBOSE) {
                LOG << "removing null statement: " << s->getNumber() <<
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

void UserProc::processConstants() {
    if (VERBOSE)
        LOG << "Process constants for " << getName() << "\n";
    StatementList stmts;
    getStatements(stmts);
    // process any constants in the statement
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        s->processConstants(prog);
    }
}

// Propagate statements, but don't remove
// Respect the memory depth (don't propagate FROM statements that have
// components of a higher memory depth than memDepth)
// Also don't propagate TO expressions of depth other than toDepth
// (unless toDepth == -1)
// Return true if an indirect call is converted to direct
bool UserProc::propagateStatements(int memDepth, int toDepth) {
    StatementList stmts;
    getStatements(stmts);
    // propagate any statements that can be
    StatementSet empty;
    StatementList::iterator it;
    bool convertedIndirect = false;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->isPhi()) continue;
        // We can propagate to ReturnStatements now, and "return 0"
        // if (s->isReturn()) continue;
        convertedIndirect |= s->propagateTo(memDepth, empty, toDepth);
    }
    simplify();
    return convertedIndirect;
}

void UserProc::promoteSignature() {
    signature = signature->promote(this);
}

Exp* UserProc::newLocal(Type* ty) {
    std::ostringstream os;
    os << "local" << locals.size();
    std::string name = os.str();
    locals[name] = ty;
    if (ty == NULL) {
        LOG << "null type passed to newLocal\n";
        assert(false);
    }
    if (VERBOSE)
        LOG << "assigning type " << ty->getCtype() << " to " << name.c_str()
            << "\n";
    // Note: this type of local (not representing memory) does not appear
    // in symbolMap
    return Location::local(strdup(name.c_str()), this);
}

Type *UserProc::getLocalType(const char *nam)
{
    if (locals.find(nam) == locals.end())
        return NULL;
    return locals[nam];
}

void UserProc::setLocalType(const char *nam, Type *ty)
{
    locals[nam] = ty;
    if (VERBOSE)
        LOG << "updating type of " << nam << " to " << ty->getCtype() << "\n";
}

void UserProc::setLocalExp(const char *nam, Exp *e)
{
    Exp *le = Location::local(strdup(nam), this);
    symbolMap[e] = le;
}

Exp *UserProc::getLocalExp(const char *nam)
{
    for (std::map<Exp*,Exp*,lessExpStar>::iterator it = symbolMap.begin();
         it != symbolMap.end(); it++)
        if ((*it).second->getOper() == opLocal &&
            !strcmp(((Const*)(*it).second->getSubExp1())->getStr(), nam))
            return (*it).first;
    return NULL;
}

// Add local variables local<b> .. local<n-1>
void UserProc::addLocals(int b, int n) {
    for (int i=b; i < n; i++) {
        std::ostringstream os;
        os << "local" << i;
        std::string name = os.str();
        if (locals.find(name) == locals.end()) {
            Exp *e = getLocalExp(name.c_str());
            if (e && e->getType())
                locals[name] = e->getType();
            else
                locals[name] = new IntegerType();   // Fixed by type analysis later
        }
    }
}

void UserProc::countRefs(RefCounter& refCounts) {
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->isPhi()) {
            ((PhiExp*)s->getRight())->simplifyRefs();
            s->simplify();
        }
        if (DEBUG_UNUSED_STMT || DEBUG_UNUSED_RETS && s->isReturn())
            LOG << "counting references in " << s << "\n";
        LocationSet refs;
#define IGNORE_IMPLICITS 1
#if IGNORE_IMPLICITS
        s->addUsedLocs(refs, true);
#else
        s->addUsedLocs(refs);
#endif
        LocationSet::iterator rr;
        for (rr = refs.begin(); rr != refs.end(); rr++) {
            if (((Exp*)*rr)->isSubscript()) {
                Statement *ref = ((RefExp*)*rr)->getRef();
                refCounts[ref]++;
                if (DEBUG_UNUSED_STMT || DEBUG_UNUSED_RETS && s->isReturn())
                    LOG << "counted ref to " << *rr << "\n";
                
            }
        }
    }
}

// Note: call the below after translating from SSA form
void UserProc::removeUnusedLocals() {
    std::set<std::string> usedLocals;
    StatementList stmts;
    getStatements(stmts);
    // First count any uses of the locals
    StatementList::iterator ss;
    for (ss = stmts.begin(); ss != stmts.end(); ss++) {
        Statement* s = *ss;
        LocationSet refs;
        s->addUsedLocs(refs);
        LocationSet::iterator rr;
        for (rr = refs.begin(); rr != refs.end(); rr++) {
            Exp* r = *rr;
            if (r->isSubscript())
                r = ((RefExp*)r)->getSubExp1();
            if (r->isLocal()) {
                Const* c = (Const*)((Unary*)r)->getSubExp1();
                std::string name(c->getStr());
                usedLocals.insert(name);
                if (VERBOSE) LOG << "Counted local " << name.c_str() <<
                  " in " << s << "\n";
            }
        }
    }
    // Now remove the unused ones
    std::map<std::string, Type*>::iterator it;
#if 0
    int nextLocal = 0;
#endif
    std::vector<std::string> removes;
    for (it = locals.begin(); it != locals.end(); it++) {
        std::string& name = const_cast<std::string&>(it->first);
        // LOG << "Considering local " << name << "\n";
        if (usedLocals.find(name) == usedLocals.end()) {
            if (VERBOSE)
                LOG << "Removed unused local " << name.c_str() << "\n";
            removes.push_back(name);
        }
#if 0   // Ugh - still have to rename the variables.
        else {
            if (name.substr(0, 5) == "local") {
                // Make the locals consequtive
                std::ostringstream os;
                os << "local" << nextLocal++;
                name = os.str();
            }
        }
#endif
    }
    for (std::vector<std::string>::iterator it1 = removes.begin(); it1 != removes.end(); it1++)
        locals.erase(*it1);
}

// Note: if depth < 0, consider all depths
void UserProc::removeUnusedStatements(RefCounter& refCounts, int depth) {
    StatementList stmts;
    getStatements(stmts);
    bool change;
    do {
        change = false;
        StatementList::iterator ll = stmts.begin();
        while (ll != stmts.end()) {
            Statement* s = *ll;
            if (s->isCall() && refCounts[s] == 0) {
                if (VERBOSE)
                    LOG << "clearing return set of unused call " << s << "\n";
                CallStatement *call = (CallStatement*)s;
                std::vector<Exp*> returns;
                int i;
                for (i = 0; i < call->getNumReturns(); i++)
                    returns.push_back(call->getReturnExp(i));
                for (i = 0; i < (int)returns.size(); i++)
                    if (depth < 0 || returns[i]->getMemDepth() <= depth)
                    //           if (returns[i]->getMemDepth() <= depth)
                        call->removeReturn(returns[i]);
                ll++;
                continue;
            }
            if (s->getKind() != STMT_ASSIGN && s->getKind() != STMT_BOOL) {
                // Never delete a statement other than an assignment or setstmt
                // (e.g. nothing "uses" a Jcond)
                ll++;
                continue;
            }
            if (s->getLeft() && depth >= 0 &&
                  s->getLeft()->getMemDepth() > depth) {
                ll++;
                continue;
            }
            if (s->getLeft() && s->getLeft()->getOper() == opGlobal) {
                // assignments to globals must always be kept
                ll++;
                continue;
            }
            if (s->getLeft()->getOper() == opMemOf &&
                !(*new RefExp(s->getLeft(), NULL) == *s->getRight())) {
                // assignments to memof anything must always be kept
                ll++;
                continue;
            }
            // if (s->getLeft()->getOper() == opParam) {
                // we actually want to remove this if no-one is using it
                // otherwise we'll create an interference that we can't
                // handle
                //ll++;
                //continue;
            // }
            if (s->getLeft()->getOper() == opMemberAccess ||
                s->getLeft()->getOper() == opArraySubscript) {
                // can't say with these
                ll++;
                continue;
            }
            if (refCounts[s] == 0) {
                // First adjust the counts. Need to be careful not to count
                // two refs as two; refCounts is a count of the number of
                // statements that use a definition, not the number of refs
                StatementSet refs;
                LocationSet components;
                s->addUsedLocs(components);
                LocationSet::iterator cc;
                for (cc = components.begin(); cc != components.end(); cc++) {
                    if ((*cc)->isSubscript()) {
                        refs.insert(((RefExp*)*cc)->getRef());
                    }
                }
                StatementSet::iterator dd;
                for (dd = refs.begin(); dd != refs.end(); dd++)
                    refCounts[*dd]--;
                if (DEBUG_UNUSED_STMT)
                    LOG << "Removing unused statement " << s->getNumber() 
                        << " " << s << "\n";
                removeStatement(s);
                ll = stmts.remove(ll);  // So we don't try to re-remove it
                change = true;
                continue;               // Don't call getNext this time
            }
            ll++;
        }
    } while (change);
}

//
//  SSA code
//

void UserProc::fromSSAform() {
    if (VERBOSE||1)
        LOG << "transforming " << getName() << " from SSA\n";

    StatementList stmts;
    getStatements(stmts);
    igraph ig;
    int tempNum = locals.size();
    int tempBase = tempNum;
    cfg->findInterferences(ig, tempNum);

    // First rename the variables (including phi's, but don't remove)
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        s->fromSSAform(ig);
    }

    // Now remove the phi's
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (!s->isPhi()) continue;
        // Check that the base variables are all the same
        PhiExp* p = (PhiExp*)s->getRight();
        LocationSet refs;
        p->addUsedLocs(refs);
        Exp* first = *refs.begin();
        bool same = true;
        LocationSet::iterator rr;
        for (rr = refs.begin(); rr != refs.end(); rr++) {
            if (!(**rr *= *first)) {       // Ref-insensitive compare
                same = false;
                break;
            }
        }
        if (same) {
            // Is the left of the phi assignment the same base variable as all
            // the operands?
            if (*s->getLeft() *= *first) {
                if (DEBUG_UNUSED_STMT)
                    LOG << "Removing phi: left and all refs same or 0: " <<
                      s << "\n";
                // Just removing the refs will work, or removing the whole phi
                // NOTE: Removing the phi here may cause other statments to be
                // not used. Soon I want to remove the phi's earlier, so this
                // code can be removed. - MVE
                removeStatement(s);
            } else
                // Just need to replace the phi by an expression,
                // e.g. local0 = phi(r24{3}, r24{5}) becomes 
                //      local0 = r24
                ((Assign*)s)->setRight(first->getSubExp1()->clone());
        }
        else {
            // Need copies
            if (Boomerang::get()->debugLiveness)
                LOG << "Phi statement " << s <<
                  " requires copies, using temp" << tempNum << "\n";
            // For each definition ref'd in the phi
            StatementVec::iterator rr;
            for (rr = p->begin(); rr != p->end(); p++) {
                // Start with the original name, in the left of the phi
                // (note: this has not been renamed above)
                Exp* right = p->getSubExp1()->clone();
                // Wrap it in a ref to *rr
                right = new RefExp(right, *rr);
                // Check the interference graph for a new name
                if (ig.find(right) != ig.end()) {
                    std::ostringstream os;
                    os << "local" << ig[right];
                    delete right;
                    right = Location::local(strdup(os.str().c_str()), this);
                } else {
                    // Just take off the reference
                    RefExp* old = (RefExp*)right;
                    right = right->getSubExp1();
                    old->setSubExp1ND(NULL);
                    delete old;
                }
                // Insert a new assignment, to local<tempNum>, from right
                insertAssignAfter(*rr, tempNum, right);
            }
            // Replace the RHS of the phi with the new temp
            std::ostringstream os;
            os << "local" << tempNum++;
            std::string name = os.str();
            ((Assign*)s)->setRight(Location::local(strdup(name.c_str()), this));
        }
    }

    // Add the resulting locals to the proc, so they will be declared
    addLocals(tempBase, tempNum);

}

void UserProc::insertArguments(StatementSet& rs) {
    cfg->insertArguments(rs);
}

bool inProve = false;

bool UserProc::canProveNow()
{
    return !inProve;
}

// this function is non-reentrant
bool UserProc::prove(Exp *query)
{
    if (inProve) {
        LOG << "attempted reentry of prove, returning false\n";
        return false;
    }
    inProve = true;
    if (proven.find(query) != proven.end()) {
        inProve = false;
        if (DEBUG_PROOF) LOG << "prove returns true\n";
        return true;
    }

    Exp *original = query->clone();

    assert(query->getOper() == opEquals);
    
    // subscript locs on the right with {0}
    LocationSet locs;
    query->getSubExp2()->addUsedLocs(locs);
    LocationSet::iterator xx;
    for (xx = locs.begin(); xx != locs.end(); xx++) {
        query->refSubExp2() = query->getSubExp2()->expSubscriptVar(*xx, NULL);
    }

    if (query->getSubExp1()->getOper() != opSubscript) {
        bool gotdef = false;
        // replace expression from return set with expression in return 
        if (theReturnStatement) {
            for (int i = 0; i < signature->getNumReturns(); i++) {
                Exp *e = signature->getReturnExp(i); 
                if (*e == *query->getSubExp1()) {
                    query->refSubExp1() = 
                        theReturnStatement->getReturnExp(i)->clone();
                    gotdef = true;
                    break;
                }
            }
        }
        if (!gotdef && DEBUG_PROOF) {
            LOG << "not in return set: " << query->getSubExp1() << "\n";
            LOG << "prove returns false\n";
            inProve = false;
            return false;
        }
    }

    proven.insert(original);
    std::set<PhiExp*> lastPhis;
    std::map<PhiExp*, Exp*> cache;
    if (!prover(query, lastPhis, cache)) {
        proven.erase(original);
        //delete original;
        inProve = false;
        if (DEBUG_PROOF) LOG << "prove returns false\n";
        return false;
    }
    //delete query;
   
    inProve = false;
    if (DEBUG_PROOF) LOG << "prove returns true\n";
    return true;
}

bool UserProc::prover(Exp *query, std::set<PhiExp*>& lastPhis, 
                      std::map<PhiExp*, Exp*> &cache, PhiExp* lastPhi)
{
    std::map<CallStatement*, Exp*> callwd;
    Exp *phiInd = query->getSubExp2()->clone();

    if (lastPhi && cache.find(lastPhi) != cache.end() &&
        *cache[lastPhi] == *phiInd) {
        if (DEBUG_PROOF)
            LOG << "true - in the cache\n";
        return true;
    } 

    std::set<Statement*> refsTo;

    query = query->clone();
    bool change = true;
    bool swapped = false;
    while (change) {
        if (DEBUG_PROOF) {
            LOG << query << "\n";
        }
    
        change = false;
        if (query->getOper() == opEquals) {

            // same left and right means true
            if (*query->getSubExp1() == *query->getSubExp2()) {
                query = new Terminal(opTrue);
                change = true;
            }

            // move constants to the right
            Exp *plus = query->getSubExp1();
            Exp *s1s2 = plus ? plus->getSubExp2() : NULL;
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
                        if (callwd.find(call) != callwd.end() && 
                            *callwd[call] == *query) {
                            LOG << "found call loop to " 
                                << call->getDestProc()->getName() << " "
                                << query << "\n";
                            query = new Terminal(opFalse);
                            change = true;
                        } else {
                            callwd[call] = query->clone();
                            if (DEBUG_PROOF)
                                LOG << "using proven (or induction) for " 
                                    << call->getDestProc()->getName() << " " 
                                    << r->getSubExp1() 
                                    << " = " << right << "\n";
                            right = call->substituteParams(right);
                            if (DEBUG_PROOF)
                                LOG << "right with subs: " << right << "\n";
                            query->setSubExp1(right);
                            change = true;
                        }
                    }
                } else if (s && s->getRight()) {
                    if (s->getRight()->getOper() == opPhi) {
                        // for a phi, we have to prove the query for every 
                        // statement
                        PhiExp *p = (PhiExp*)s->getRight();
                        StatementVec::iterator it;
                        bool ok = true;
                        if (lastPhis.find(p) != lastPhis.end() || p == lastPhi)
                        {
                            if (DEBUG_PROOF)
                                LOG << "phi loop detected ";
                            ok = (*query->getSubExp2() == *phiInd);
                            if (ok && DEBUG_PROOF)
                                LOG << "(set true due to induction)\n";
                            if (!ok && DEBUG_PROOF)
                                LOG << "(set false " << 
                                    query->getSubExp2() << " != " << 
                                    phiInd << ")\n";
                        } else {
                            if (DEBUG_PROOF)
                                LOG << "found " << s << " prove for each\n";
                            for (it = p->begin(); it != p->end(); it++) {
                                Exp *e = query->clone();
                                RefExp *r1 = (RefExp*)e->getSubExp1();
                                r1->setDef(*it);
                                if (DEBUG_PROOF)
                                    LOG << "proving for " << e << "\n";
                                lastPhis.insert(lastPhi);
                                if (!prover(e, lastPhis, cache, p)) { 
                                    ok = false; 
                                    //delete e; 
                                    break; 
                                }
                                lastPhis.erase(lastPhi);
                                //delete e;
                            }
                            if (ok)
                                cache[p] = query->getSubExp2()->clone();
                        }
                        if (ok)
                            query = new Terminal(opTrue);
                        else 
                            query = new Terminal(opFalse);
                        change = true;
                    } else {
                        if (s && refsTo.find(s) != refsTo.end()) {
                            LOG << "detected ref loop " << s << "\n";
                            assert(false);
                        } else {
                            refsTo.insert(s);
                            query->setSubExp1(s->getRight()->clone());
                            change = true;
                        }
                    }
                }
            }

            // remove memofs from both sides if possible
            if (!change && query->getSubExp1()->getOper() == opMemOf &&
                  query->getSubExp2()->getOper() == opMemOf) {
                query->refSubExp1() =
                  ((Unary*)query->getSubExp1())->becomeSubExp1();
                query->refSubExp2() =
                  ((Unary*)query->getSubExp2())->becomeSubExp1();
                change = true;
            }

            // is ok if both of the memofs is subscripted with NULL
            if (!change && query->getSubExp1()->getOper() == opSubscript &&
                  query->getSubExp1()->getSubExp1()->getOper() == opMemOf &&
                  ((RefExp*)query->getSubExp1())->getRef() == NULL &&
                  query->getSubExp2()->getOper() == opSubscript &&
                  query->getSubExp2()->getSubExp1()->getOper() == opMemOf &&
                  ((RefExp*)query->getSubExp2())->getRef() == NULL) {
                query->refSubExp1() =
                  ((Unary*)query->getSubExp1()->getSubExp1())->becomeSubExp1();
                query->refSubExp2() =
                  ((Unary*)query->getSubExp2()->getSubExp1())->becomeSubExp1();
                change = true;
            }

            // find a memory def for the right if there is a memof on the left
            if (!change && query->getSubExp1()->getOper() == opMemOf) {
                StatementList stmts;
                getStatements(stmts);
                StatementList::iterator it;
                for (it = stmts.begin(); it != stmts.end(); it++) {
                    Statement* s = *it;
                    if (s->getLeft() && s->getRight() && 
                        *s->getRight() == *query->getSubExp2() &&
                        s->getLeft()->getOper() == opMemOf) {
                        query->refSubExp2() = s->getLeft()->clone();
                        change = true;
                        break;
                    }
                }
            }

            // last chance, swap left and right if havn't swapped before
            if (!change && !swapped) {
                Exp *e = query->getSubExp1();
                query->refSubExp1() = query->getSubExp2();
                query->refSubExp2() = e;
                change = true;
                swapped = true;
                refsTo.clear();
            }
        } else if (query->isIntConst()) {
            Const *c = (Const*)query;
            query = new Terminal(c->getInt() ? opTrue : opFalse);
        }

        Exp *old = query->clone();

        query = query->clone()->simplify();

        if (change && !(*old == *query) && DEBUG_PROOF) {
            LOG << old << "\n";
        }
        //delete old;
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

// "Local" member function, used below
void UserProc::doCountReturns(Statement* def, ReturnCounter& rc, Exp* loc)
{
    if (def == NULL) return;
    CallStatement* call = dynamic_cast<CallStatement*>(def);
    if (call == NULL) return;
    // We have a reference to a return of the call statement
    UserProc* proc = (UserProc*) call->getDestProc();
    //if (proc->isLib()) return;
    if (DEBUG_UNUSED_RETS) {
        LOG << " @@ Counted use of return location " << loc 
            << " for call to ";
        if (proc) 
            LOG << proc->getName();
        else
            LOG << "(null)";
        LOG << " at " << def->getNumber() << " in " << getName() << "\n";
    }
    // we want to count the return that corresponds to this loc
    // this can be a different expression to loc because replacements
    // are done in the call's return list as part of decompilation
    int n = call->findReturn(loc);
    if (n != -1) {
        Exp *ret = NULL;
        if (proc)
            ret = proc->getSignature()->getReturnExp(n);
        else {
            assert(call->isComputed());
            std::vector<Exp*> &returns = getProg()->getDefaultReturns();
            assert(n < (int)returns.size());
            ret = returns[n];
        }
        rc[proc].insert(ret);
    }
}

void UserProc::countUsedReturns(ReturnCounter& rc) {
    if (DEBUG_UNUSED_RETS)
        LOG << " @@ Counting used returns in " << getName() << "\n";
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator ss;
    // For each statement this proc
    for (ss = stmts.begin(); ss != stmts.end(); ss++) {
        LocationSet used;
        (*ss)->addUsedLocs(used);
        LocationSet::iterator ll;
        // For each use this statement
        for (ll = used.begin(); ll != used.end(); ll++) {
            Statement* def;
            if ((*ll)->isSubscript()) {
                // for this one reference
                def = ((RefExp*)*ll)->getRef();
                doCountReturns(def, rc, ((RefExp*)*ll)->getSubExp1());
            } else if ((*ll)->isPhi()) {
                StatementVec::iterator rr;
                PhiExp& pe = (PhiExp&)**ll;
                // for each reference this phi expression
                for (rr = pe.begin(); rr != pe.end(); rr++)
                    doCountReturns(*rr, rc, pe.getSubExp1());
            }
        }
    }
}

bool UserProc::removeUnusedReturns(ReturnCounter& rc) {
    std::set<Exp*, lessExpStar> removes;    // Else iterators confused
    std::set<Exp*, lessExpStar>& useSet = rc[this];
    for (int i = 0; i < signature->getNumReturns(); i++) {
        Exp *ret = signature->getReturnExp(i);
        if (useSet.find(ret) == useSet.end())
            removes.insert(ret);
    }
    std::set<Exp*, lessExpStar>::iterator it;
    Exp* stackExp = NULL;
    if (signature->isPromoted()) {
        stackExp = Location::regOf(signature->getStackRegister());
        assert(stackExp);
    }
    bool removedOne = false;
    for (it = removes.begin(); it != removes.end(); it++) {
        if (signature->isPromoted() && !(*stackExp == **it))
            // Only remove stack pointer if promoted
            continue;
        if (DEBUG_UNUSED_RETS)
            LOG << " @@ Removing unused return " << *it <<
            " in " << getName() << "\n";
        removeReturn(*it);
        removedOne = true;
    }
    return removedOne;
}

void Proc::addCallers(std::set<UserProc*>& callers) {
    std::set<CallStatement*>::iterator it;
    for (it = callerSet.begin(); it != callerSet.end(); it++) {
        UserProc* callerProc = (*it)->getProc();
        callers.insert(callerProc);
    }
}

void UserProc::addCallees(std::set<UserProc*>& callees) {
    std::set<Proc*>::iterator it;
    for (it = calleeSet.begin(); it != calleeSet.end(); it++) {
        UserProc* callee = (UserProc*)(*it);
        if (callee->isLib()) continue;
        callees.insert(callee);
    }
}

void UserProc::typeAnalysis(Prog* prog) {
    if (DEBUG_TA)
        LOG << "Procedure " << getName() << "\n";
    Constraints consObj;
    LocationSet cons;
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator ss;
    // For each statement this proc
    int conscript = 0;
    for (ss = stmts.begin(); ss != stmts.end(); ss++) {
        cons.clear();
        // So we can co-erce constants:
        conscript = (*ss)->setConscripts(conscript);
        (*ss)->genConstraints(cons);
        consObj.addConstraints(cons);
        if (DEBUG_TA)
            LOG << (*ss) << "\n" << &cons << "\n";
    }

    std::list<ConstraintMap> solns;
    bool ret = consObj.solve(solns);
    if (VERBOSE || DEBUG_TA) {
        if (!ret)
            LOG << "** Could not solve type constraints for proc " <<
              getName() << "!\n";
        else if (solns.size() > 1)
            LOG << "** " << solns.size() << " solutions to type "
              "constraints for proc " << getName() << "!\n";
    }
        
    std::list<ConstraintMap>::iterator it;
    int solnNum = 0;
    ConstraintMap::iterator cc;
    if (DEBUG_TA) {
        for (it = solns.begin(); it != solns.end(); it++) {
            LOG << "Solution " << ++solnNum << " for proc " << getName()
              << "\n";
            ConstraintMap& cm = *it;
            for (cc = cm.begin(); cc != cm.end(); cc++)
                LOG << cc->first << " = " << cc->second << "\n";
            LOG << "\n";
        }
    }

    // Just use the first solution, if there is one
    if (solns.size()) {
        ConstraintMap& cm = *solns.begin();
        for (cc = cm.begin(); cc != cm.end(); cc++) {
            assert(cc->first->isTypeOf());
            Exp* loc = ((Unary*)cc->first)->getSubExp1();
            assert(cc->second->isTypeVal());
            Type* ty = ((TypeVal*)cc->second)->getType();
            if (loc->isSubscript())
                loc = ((RefExp*)loc)->getSubExp1();
            else if (loc->isGlobal()) {
                char* nam = ((Const*)((Unary*)loc)->getSubExp1())->getStr();
                prog->setGlobalType(nam, ty->clone());
            } else if (loc->isLocal()) {
                char* nam = ((Const*)((Unary*)loc)->getSubExp1())->getStr();
                setLocalType(nam, ty);
            } else if (loc->isIntConst()) {
                Const* con = (Const*)loc;
                int val = con->getInt();
                if (ty->isFloat()) {
                    // Need heavy duty cast here
                    con->setFlt(*(float*)&val);
                    con->setOper(opFltConst);
                } else if (ty->isPointer() &&
                  ((PointerType*)ty)->getPointsTo()->resolvesToChar()) {
                    // Convert to a string
                    char* str = prog->getStringConstant(val, true);
                    if (str) {
                        // Make a string
                        con->setStr(escapeStr(str));
                        con->setOper(opStrConst);
                    }
                }
            }
        }
    }
}

bool UserProc::searchAndReplace(Exp *search, Exp *replace)
{
    bool ch = false;
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        ch |= s->searchAndReplace(search, replace);   
    }
    return ch; 
}

unsigned fudge(StatementList::iterator x) {
  StatementList::iterator y = x;
  return *(unsigned*)&y;
}
void UserProc::stripRefs() {
    StatementList stmts, delList;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        if ((*it)->stripRefs())
            delList.append(*it);
    }
    // Now delete the phis; somewhat inefficient at present
    for (it = delList.begin(); it != delList.end(); it++)
        removeStatement(*it);
}

Exp *UserProc::getProven(Exp *left)
{
    for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); 
         it != proven.end(); it++) 
        if (*(*it)->getSubExp1() == *left)
            return (*it)->getSubExp2();
    // not found, try the signature
    // Shouldn't this only be for library functions?
    //return signature->getProven(left);
    return NULL;
}

