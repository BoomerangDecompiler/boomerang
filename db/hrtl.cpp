/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       hrtl.cc
 * OVERVIEW:   Implementation of the classes that describe a high level RTL,
 *             such as HLJump, HLCall, etc.
 *============================================================================*/

/*
 * $Revision$
 * 17 May 02 - Mike: Split off from rtl.cc (was getting too large)
 * 26 Nov 02 - Mike: Generate code for HlReturn with semantics (eg SPARC RETURN)
 * 26 Nov 02 - Mike: In getReturnLoc test for null procDest
 * 03 Dec 02 - Mike: Made a small mod to HLCall::killDef for indirect calls
 * 19 Dec 02 - Mike: Fixed the expressions in HLJcond::setCondType()
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <iomanip>          // For setfill
#include <sstream>
#include "types.h"
#include "dataflow.h"
#include "exp.h"
#include "cfg.h"
#include "register.h"
#include "type.h"
#include "rtl.h"
#include "proc.h"           // For printing proc names
#include "prog.h"
#include "hllcode.h"
#include "util.h"
#include "signature.h"
#include "boomerang.h"

/******************************************************************************
 * HLJump methods
 *****************************************************************************/

/*==============================================================================
 * FUNCTION:        HLJump::HLJump
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr: native address of the RTL
 *                  listExp: a list of Exps (not the same as an RTL) to serve
 *                      as the initial list of Register Transfers
 * RETURNS:         N/a
 *============================================================================*/
HLJump::HLJump(ADDRESS instNativeAddr, std::list<Exp*>* listExp /*= NULL*/)
    : RTL(instNativeAddr, listExp), pDest(NULL), m_isComputed(false) {
    kind = JUMP_RTL;
}

/*==============================================================================
 * FUNCTION:        HLJump::HLJump
 * OVERVIEW:        Construct a jump to a fixed address
 * PARAMETERS:      instNativeAddr: native address of the jump RTL
 *                  uDest: native address of destination
 * RETURNS:         N/a
 *============================================================================*/
HLJump::HLJump(ADDRESS instNativeAddr, ADDRESS uDest) :
    RTL(instNativeAddr), m_isComputed(false) {
    kind = JUMP_RTL;
    // Note: we used to generate an assignment (pc := <dest>), but it gets
    // ignored anyway, and it causes us to declare pc as a variable in the back
    // end. So now the semantics of a HLJUMP are purely implicit
    pDest = new Const(uDest);
}

/*==============================================================================
 * FUNCTION:        HLJump::~HLJump
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLJump::~HLJump() {
    if (pDest) delete pDest;
}

/*==============================================================================
 * FUNCTION:        HLJump::getFixedDest
 * OVERVIEW:        Get the fixed destination of this CTI. Assumes destination
 *                  simplication has already been done so that a fixed dest will
 *                  be of the Exp form:
 *                     opIntConst dest
 * PARAMETERS:      <none>
 * RETURNS:         Fixed dest or NO_ADDRESS if there isn't one
 *============================================================================*/
ADDRESS HLJump::getFixedDest() {
    if (pDest->getOper() != opIntConst) return NO_ADDRESS;
    return ((Const*)pDest)->getAddr();
}

/*==============================================================================
 * FUNCTION:        HLJump::setDest
 * OVERVIEW:        Set the destination of this CTI to be a given address.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::setDest(Exp* pd) {
    if (pDest != NULL)
        delete pDest;
    pDest = pd;
}

/*==============================================================================
 * FUNCTION:        HLJump::setDest
 * OVERVIEW:        Set the destination of this CTI to be a given fixed address.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::setDest(ADDRESS addr) {
// This fails in FrontSparcTest, do you really want it to Mike? -trent
//  assert(addr >= prog.limitTextLow && addr < prog.limitTextHigh);
    // Delete the old destination if there is one
    if (pDest != NULL)
        delete pDest;

    pDest = new Const(addr);
}

/*==============================================================================
 * FUNCTION:        HLJump::getDest
 * OVERVIEW:        Returns the destination of this CTI.
 * PARAMETERS:      None
 * RETURNS:         Pointer to the SS representing the dest of this jump
 *============================================================================*/
Exp* HLJump::getDest() {
    return pDest;
}

/*==============================================================================
 * FUNCTION:        HLJump::adjustFixedDest
 * OVERVIEW:        Adjust the destination of this CTI by a given amount. Causes
 *                  an error is this destination is not a fixed destination
 *                  (i.e. a constant offset).
 * PARAMETERS:      delta - the amount to add to the destination (can be
 *                  negative)
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::adjustFixedDest(int delta) {
    // Ensure that the destination is fixed.
    if (pDest == 0 || pDest->getOper() != opIntConst)
        std::cerr << "Can't adjust destination of non-static CTI\n";

    ADDRESS dest = ((Const*)pDest)->getAddr();
    ((Const*)pDest)->setAddr(dest + delta);
}

/*==============================================================================
 * FUNCTION:        HLJump::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::searchAndReplace(Exp* search, Exp* replace) {
    RTL::searchAndReplace(search, replace);
    if (pDest) {
        bool change;
        pDest->searchReplaceAll(search, replace, change);
    }
}

/*==============================================================================
 * FUNCTION:        HLJump::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJump::searchAll(Exp* search, std::list<Exp*> &result) {
    return RTL::searchAll(search, result) ||
        ( pDest && pDest->searchAll(search, result) );
}

/*==============================================================================
 * FUNCTION:        HLJump::print
 * OVERVIEW:        Display a text reprentation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::print(std::ostream& os /*= cout*/, bool withDF) {
    // Returns can all have semantics (e.g. ret/restore)
    if (expList.size() != 0)
        RTL::print(os, withDF);

    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " " << std::setfill(' ');
    os << "     ";                  // Where the statement number would be
    if (getKind() == RET_RTL) {
        os << "RET\n";              // RET is a special case of a JUMP_RTL
        return;
    }

    os << "JUMP ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (pDest->getOper() != opIntConst)
         pDest->print(os);
    else
        os << "0x" << std::hex << getFixedDest();
    os << std::endl;
}

/*==============================================================================
 * FUNCTION:      HLJump::setIsComputed
 * OVERVIEW:      Sets the fact that this call is computed.
 * NOTE:          This should really be removed, once HLNwayJump and HLNwayCall
 *                  are implemented properly
 * PARAMETERS:    <none>
 * RETURNS:       <nothing>
 *============================================================================*/
void HLJump::setIsComputed(bool b) {
    m_isComputed = b;
}

/*==============================================================================
 * FUNCTION:      HLJump::isComputed
 * OVERVIEW:      Returns whether or not this call is computed.
 * NOTE:          This should really be removed, once HLNwayJump and HLNwayCall
 *                  are implemented properly
 * PARAMETERS:    <none>
 * RETURNS:       this call is computed
 *============================================================================*/
bool HLJump::isComputed() {
    return m_isComputed;
}

/*==============================================================================
 * FUNCTION:        HLJump::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this one
 *============================================================================*/
RTL* HLJump::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++)
        le.push_back((*it)->clone());
    HLJump* ret = new HLJump(nativeAddr, &le);
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

// visit this rtl
bool HLJump::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

#if 0
/*==============================================================================
 * FUNCTION:        HLJump::getUseDefLocations
 * OVERVIEW:        The DFA analysis of a jump RTL extends that of a
 *                    standard RTL in that it *uses* registers etc in its
 *                    destination expression. E.g. jump to r[25], r[25] is used
 * PARAMETERS:      locMap - a map between locations and integers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::getUseDefLocations(LocationMap& locMap, LocationFilter* filter,
    BITSET& defSet, BITSET& useSet, BITSET& useUndefSet, Proc* proc) const {
    // If jumps ever have semantics, then this call would be needed
    // RTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        // proc);

    if (pDest)
        searchExprForUses(pDest, locMap, filter, defSet, useSet, useUndefSet);
}
#endif

// serialize this rtl
bool HLJump::serialize_rest(std::ostream &ouf) {
    if (pDest && pDest->getOper() == opIntConst) {
        saveFID(ouf, FID_RTL_FIXDEST);
        saveValue(ouf, ((Const *)pDest)->getAddr());
    } else if (pDest) {
        saveFID(ouf, FID_RTL_JDEST);
        int l;
        pDest->serialize(ouf, l);
    }

    return true;
}

// deserialize an rtl
bool HLJump::deserialize_fid(std::istream &inf, int fid) {
    switch (fid) {
        case FID_RTL_FIXDEST:
            {
                ADDRESS a;
                loadValue(inf, a);
                pDest = new Const(a);
            }
            break;
        case FID_RTL_JDEST:
            {
                pDest = Exp::deserialize(inf);
                if (pDest->getOper() != opIntConst)
                    m_isComputed = true;
                else
                    m_isComputed = false;
            }
            break;
        default:
            return RTL::deserialize_fid(inf, fid);
    }

    return true;
}

void HLJump::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // dont generate any code for jumps, they will be handled by the BB
}

void HLJump::simplify() {
    if (isComputed()) {
        Exp *e = pDest->simplifyArith()->clone();
        delete pDest;
        pDest = e->simplify();
    }
}

/**********************************
 * HLJcond methods
 **********************************/

/*==============================================================================
 * FUNCTION:        HLJcond::HLJcond
 * OVERVIEW:        Constructor.
 * PARAMETERS:      instNativeAddr: ADDRESS of native instr
 *                  le: ptr to list of Exp* for the Jcond
 * RETURNS:         N/a
 *============================================================================*/
HLJcond::HLJcond(ADDRESS instNativeAddr, std::list<Exp*>* le /*= NULL*/) :
  HLJump(instNativeAddr, le), jtCond((JCOND_TYPE)0), pCond(NULL),
  bFloat(false) {
    kind = JCOND_RTL;
}

/*==============================================================================
 * FUNCTION:        HLJcond::~HLJcond
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLJcond::~HLJcond() {
    if (pCond)
        delete pCond;
}

/*==============================================================================
 * FUNCTION:        HLJcond::setCondType
 * OVERVIEW:        Sets the JCOND_TYPE of this jcond as well as the flag
 *                  indicating whether or not the floating point condition codes
 *                  are used.
 * PARAMETERS:      cond - the JCOND_TYPE
 *                  usesFloat - this condional jump checks the floating point
 *                    condition codes
 * RETURNS:         a semantic string
 *============================================================================*/
void HLJcond::setCondType(JCOND_TYPE cond, bool usesFloat /*= false*/) {
    jtCond = cond;
    bFloat = usesFloat;

    if (bFloat) return;

    // set pCond to a high level representation of this type
    Exp* p = NULL;
#if 0
    switch(cond) {
        case HLJCOND_JE:
            p = new Terminal(opZF);
            break;
        case HLJCOND_JNE:
            p = new Unary(opNot, new Terminal(opZF));
            break;
        case HLJCOND_JSL:
            // N xor V
            p = new Binary(opNotEqual, new Terminal(opNF), new Terminal(opOF));
            break;
        case HLJCOND_JSLE:
            // Z or (N xor V)
            p = new Binary(opOr,
                new Terminal(opZF),
                new Binary(opNotEqual, new Terminal(opNF), new Terminal(opOF)));
            break;
        case HLJCOND_JSGE:
            // not (N xor V) same as (N == V)
            p = new Binary(opEquals, new Terminal(opNF), new Terminal(opOF));
            break;
        case HLJCOND_JSG:
            // not (Z or (N xor V))
            p = new Unary(opNot,
                new Binary(opOr,
                    new Terminal(opZF),
                    new Binary(opNotEqual,
                        new Terminal(opNF), new Terminal(opOF))));
            break;
        case HLJCOND_JUL:
            // C
            p = new Terminal(opCF);
            break;
        case HLJCOND_JULE:
            // C or Z
            p = new Binary(opOr, new Terminal(opCF), 
                                 new Terminal(opZF));
            break;
        case HLJCOND_JUGE:
            // not C
            p = new Unary(opNot, new Terminal(opCF));
            break;
        case HLJCOND_JUG:
            // not (C or Z)
            p = new Unary(opNot,
                new Binary(opOr,
                    new Terminal(opCF),
                    new Terminal(opZF)));
            break;
        case HLJCOND_JMI:
            // N
            p = new Terminal(opNF);
            break;
        case HLJCOND_JPOS:
            // not N
            p = new Unary(opNot, new Terminal(opNF));
            break;
        case HLJCOND_JOF:
            // V
            p = new Terminal(opOF);
            break;
        case HLJCOND_JNOF:
            // not V
            p = new Unary(opNot, new Terminal(opOF));
            break;
        case HLJCOND_JPAR:
            // Can't handle (could happen as a result of a failure of Pentium
            // floating point analysis)
            assert(false);
            break;
    }
#else
    p = new Terminal(opFlags);
#endif
    assert(p);
    setCondExpr(p);
}

/*==============================================================================
 * FUNCTION:        HLJcond::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::makeSigned() {
    // Make this into a signed branch
    switch (jtCond)
    {
        case HLJCOND_JUL : jtCond = HLJCOND_JSL;  break;
        case HLJCOND_JULE: jtCond = HLJCOND_JSLE; break;
        case HLJCOND_JUGE: jtCond = HLJCOND_JSGE; break;
        case HLJCOND_JUG : jtCond = HLJCOND_JSG;  break;
        default:
            // Do nothing for other cases
            break;
    }
}

/*==============================================================================
 * FUNCTION:        HLJcond::getCondExpr
 * OVERVIEW:        Return the SemStr expression containing the HL condition.
 * PARAMETERS:      <none>
 * RETURNS:         ptr to an expression
 *============================================================================*/
Exp* HLJcond::getCondExpr() {
    return pCond;
}

/*==============================================================================
 * FUNCTION:        HLJcond::setCondExpr
 * OVERVIEW:        Set the SemStr expression containing the HL condition.
 * PARAMETERS:      Pointer to Exp to set
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::setCondExpr(Exp* e) {
    if (pCond) delete pCond;
    pCond = e;
}

bool HLJcond::search(Exp* search, Exp*& result) {
    if (pCond) return pCond->search(search, result);
    result = NULL;
    return false;
}

/*==============================================================================
 * FUNCTION:        HLJcond::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::searchAndReplace(Exp* search, Exp* replace) {
    HLJump::searchAndReplace(search, replace);
    bool change;
    if (pCond)
        pCond = pCond->searchReplaceAll(search, replace, change);
}

// update type for expression
Type *HLJcond::updateType(Exp *e, Type *curType) {
    if (jtCond == HLJCOND_JUGE || jtCond == HLJCOND_JULE ||
        jtCond == HLJCOND_JUG || jtCond == HLJCOND_JUL && 
        curType->isInteger()) {
        ((IntegerType*)curType)->setSigned(false);
    }
    return curType;
}

// Convert from SSA form
void HLJcond::fromSSAform(igraph& ig) {

}

/*==============================================================================
 * FUNCTION:        HLJCond::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJcond::searchAll(Exp* search, std::list<Exp*> &result) {
    return RTL::searchAll(search, result) ||
      (pCond && (pCond->searchAll(search, result)));
}


/*==============================================================================
 * FUNCTION:        HLJcond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         Nothing
 *============================================================================*/
void HLJcond::print(std::ostream& os /*= cout*/, bool withDF) {
    // These can have semantics (e.g. pa-risc add and (conditionally) branch)
    if (expList.size() != 0)
        RTL::print(os, withDF);
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " " << std::setfill(' ');
    os << std::setw(4) << std::dec << number << " ";
    os << "JCOND ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (!pDest->isIntConst())
        os << pDest;
    else {
        // Really we'd like to display the destination label here...
        os << "0x" << std::hex << getFixedDest();
    }
    os << ", condition ";
    switch (jtCond) {
        case HLJCOND_JE:    os << "equals"; break;
        case HLJCOND_JNE:   os << "not equals"; break;
        case HLJCOND_JSL:   os << "signed less"; break;
        case HLJCOND_JSLE:  os << "signed less or equals"; break;
        case HLJCOND_JSGE:  os << "signed greater or equals"; break;
        case HLJCOND_JSG:   os << "signed greater"; break;
        case HLJCOND_JUL:   os << "unsigned less"; break;
        case HLJCOND_JULE:  os << "unsigned less or equals"; break;
        case HLJCOND_JUGE:  os << "unsigned greater or equals"; break;
        case HLJCOND_JUG:   os << "unsigned greater"; break;
        case HLJCOND_JMI:   os << "minus"; break;
        case HLJCOND_JPOS:  os << "plus"; break;
        case HLJCOND_JOF:   os << "overflow"; break;
        case HLJCOND_JNOF:  os << "no overflow"; break;
        case HLJCOND_JPAR:  os << "parity"; break;
    }
    if (bFloat) os << " float";
    if (withDF) {
        os << "\tuses: ";
        uses.printNums(os);
    }
    os << std::endl;
    if (pCond) {
        os << "High level: " << pCond << std::endl;
    }
}

/*==============================================================================
 * FUNCTION:        HLJcond::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this HLJcond
 *============================================================================*/
RTL* HLJcond::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++)
        le.push_back((*it)->clone());
    HLJcond* ret = new HLJcond(nativeAddr, &le);
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    ret->jtCond = jtCond;
    if (pCond) ret->pCond = pCond->clone();
    else ret->pCond = NULL;
    ret->m_isComputed = m_isComputed;
    ret->bFloat = bFloat;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

// visit this rtl
bool HLJcond::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

// serialize this rtl
bool HLJcond::serialize_rest(std::ostream &ouf) {
    HLJump::serialize_rest(ouf);

    saveFID(ouf, FID_RTL_JCONDTYPE);
    saveValue(ouf, (char)jtCond);

    saveFID(ouf, FID_RTL_USESFLOATCC);
    saveValue(ouf, bFloat);

    if (pCond) {
        saveFID(ouf, FID_RTL_JCOND);
        int l;
        pCond->serialize(ouf, l);
    }

    return true;
}

// deserialize an rtl
bool HLJcond::deserialize_fid(std::istream &inf, int fid) {
    char ch;

    switch (fid) {
        case FID_RTL_JCONDTYPE:             
            loadValue(inf, ch);
            jtCond = (JCOND_TYPE)ch;
            break;
        case FID_RTL_USESFLOATCC:
            loadValue(inf, bFloat);
            break;
        case FID_RTL_JCOND:
            pCond = Exp::deserialize(inf);
            break;
        default:
            return HLJump::deserialize_fid(inf, fid);
    }

    return true;
}

void HLJcond::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // dont generate any code for jconds, they will be handled by the bb
}

bool HLJcond::usesExp(Exp *e) {
    Exp *tmp;
    return pCond && pCond->search(e, tmp);
}

// Deadness is killed by a use
void HLJcond::killDead(LocationSet &dead) {
    if (pCond)
        dead.remove(pCond);
}

// special print functions
#if 0
void HLJcond::printAsUse(std::ostream &os) {
    os << "JCOND ";
    if (pCond)
        pCond->print(os);
    else
        os << "<empty cond>";
}

void HLJcond::printAsUseBy(std::ostream &os) {
    os << "JCOND ";
    if (pCond)
        pCond->print(os);
    else
        os << "<empty cond>";
}
#endif

// process any constants in the statement
void HLJcond::processConstants(Prog *prog) {
}

void HLJcond::doReplaceUse(Statement *use) {
    bool change;
    assert(pCond);
    pCond = pCond->searchReplaceAll(use->getLeft(), use->getRight(), change);
    simplify();
}

void HLJcond::simplify() {
    if (pCond) {
        Exp *e = pCond->simplifyArith()->clone();
        delete pCond;
        pCond = e->simplify();

        std::stringstream os;
        pCond->print(os);
        std::string s = os.str();

        if (pCond->getOper() == opFlagCall && 
            !strncmp(((Const*)pCond->getSubExp1())->getStr(), 
                    "SUBFLAGS", 8)) {
            Exp *e = pCond;
            OPER op = opWild;
            switch (jtCond) {
                case HLJCOND_JE:    op = opEquals; break;
                case HLJCOND_JNE:   op = opNotEqual; break;
                case HLJCOND_JSL:   op = opLess; break;
                case HLJCOND_JSLE:  op = opLessEq; break;
                case HLJCOND_JSGE:  op = opGtrEq; break;
                case HLJCOND_JSG:   op = opGtr; break;
                case HLJCOND_JUL:   op = opLessUns; break;
                case HLJCOND_JULE:  op = opLessEqUns; break;
                case HLJCOND_JUGE:  op = opGtrEqUns; break;
                case HLJCOND_JUG:   op = opGtrUns; break;
                case HLJCOND_JMI:
                    pCond = new Binary(opLess,
                        pCond->getSubExp2()->getSubExp2()->getSubExp2()
                            ->getSubExp1()->clone(), new Const(0));
                    delete e;
                    break;
                case HLJCOND_JPOS:
                    pCond = new Binary(opGtrEq,
                        pCond->getSubExp2()->getSubExp2()->getSubExp2()
                            ->getSubExp1()->clone(), new Const(0));
                    delete e;
                    break;
                case HLJCOND_JOF:
                case HLJCOND_JNOF:
                case HLJCOND_JPAR:
                    break;
            }
            if (op != opWild) {
                pCond = new Binary(op,
                    pCond->getSubExp2()->getSubExp1()->clone(), 
                    pCond->getSubExp2()->getSubExp2()->getSubExp1()
                        ->clone());
                delete e;
            }
        }
        if (pCond->getOper() == opFlagCall && 
            !strncmp(((Const*)pCond->getSubExp1())->getStr(), 
                    "LOGICALFLAGS", 12)) {
            Exp *e = pCond;
            switch (jtCond) {
                case HLJCOND_JE:
                    pCond = new Binary(opEquals,
                        pCond->getSubExp2()->getSubExp1()->clone(), 
                        new Const(0));
                    break;
                case HLJCOND_JNE:
                    pCond = new Binary(opNotEqual,
                        pCond->getSubExp2()->getSubExp1()->clone(), 
                        new Const(0));
                    break;
                case HLJCOND_JMI:
                    pCond = new Binary(opLess,
                        pCond->getSubExp2()->getSubExp1()->clone(), 
                        new Const(0));
                    delete e;
                    break;
                case HLJCOND_JPOS:
                    pCond = new Binary(opGtrEq,
                        pCond->getSubExp2()->getSubExp1()->clone(), 
                        new Const(0));
                    delete e;
                    break;
                default:
                    break;
            }
        }
    }
}

void HLJcond::addUsedLocs(LocationSet& used) {
    if (pCond)
        pCond->addUsedLocs(used);
}

/**********************************
 * HLNwayJump methods
 **********************************/
/*==============================================================================
 * FUNCTION:        HLNwayJump::HLNwayJump
 * OVERVIEW:        Constructor.
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLNwayJump::HLNwayJump(ADDRESS instNativeAddr, std::list<Exp*>* le /*= NULL*/) :
    HLJump(instNativeAddr, le), pSwitchInfo(NULL) {
    kind = NWAYJUMP_RTL;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::~HLNwayJump
 * OVERVIEW:        Destructor
 * NOTE:            Don't delete the pSwitchVar; it's always a copy of something
 *                  else (so don't delete twice)
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLNwayJump::~HLNwayJump() {
    if (pSwitchInfo)
        delete pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::getSwitchInfo
 * OVERVIEW:        Return a pointer to a struct with switch information in it
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
SWITCH_INFO* HLNwayJump::getSwitchInfo() {
    return pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::setSwitchInfo
 * OVERVIEW:        Set a pointer to a SWITCH_INFO struct
 * PARAMETERS:      Pointer to SWITCH_INFO struct
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::setSwitchInfo(SWITCH_INFO* psi) {
    pSwitchInfo = psi;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::searchAndReplace(Exp* search, Exp* replace) {
    HLJump::searchAndReplace(search, replace);
    bool ch;
    if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, ch);
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLNwayJump::searchAll(Exp* search, std::list<Exp*> &result) {
    return HLJump::searchAll(search, result) ||
        ( pSwitchInfo && pSwitchInfo->pSwitchVar &&
          pSwitchInfo->pSwitchVar->searchAll(search, result) );
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void HLNwayJump::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " ";
    os << "NWAY_JUMP [";
    if (pDest == NULL)
        os << "*no dest*";
    else os << pDest;
    os << "] ";
    if (pSwitchInfo)
        os << "Switch variable: " << pSwitchInfo->pSwitchVar << std::endl;
}


/*==============================================================================
 * FUNCTION:        HLNwayJump::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this one
 *============================================================================*/
RTL* HLNwayJump::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++)
        le.push_back((*it)->clone());
    HLNwayJump* ret = new HLNwayJump(nativeAddr, &le);
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    ret->numNativeBytes = numNativeBytes;
    ret->pSwitchInfo = new SWITCH_INFO;
    *ret->pSwitchInfo = *pSwitchInfo;
    ret->pSwitchInfo->pSwitchVar = pSwitchInfo->pSwitchVar->clone();
    return ret;
}

// visit this rtl
bool HLNwayJump::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

// serialize this rtl
bool HLNwayJump::serialize_rest(std::ostream &ouf) {
    return true;
}

// deserialize an rtl
bool HLNwayJump::deserialize_fid(std::istream &inf, int fid) {
    switch (fid) {
        default:
            return RTL::deserialize_fid(inf, fid);
    }

    return true;
}

void HLNwayJump::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // dont generate any code for switches, they will be handled by the bb
}

void HLNwayJump::simplify() {
    // TODO
}

/**********************************
 *      HLCall methods
 **********************************/

/*============================================================================== * FUNCTION:         HLCall::HLCall
 * OVERVIEW:         Constructor for a call that we have extra information
 *                   for and is part of a prologue.
 * PARAMETERS:       instNativeAddr - the address of the call instruction
 *                   returnTypeSize - the size of a return union, struct or quad *                     floating point value
 * RETURNS:          <nothing>
 *============================================================================*/
HLCall::HLCall(ADDRESS instNativeAddr, int returnTypeSize /*= 0*/,
  std::list<Exp*>* le /*= NULL*/): HLJump(instNativeAddr, le), 
      returnTypeSize(returnTypeSize), returnAfterCall(false), 
      returnBlock(NULL), returnLoc(NULL) {
    kind = CALL_RTL;
    postCallExpList = NULL;
    procDest = NULL;
}

/*==============================================================================
 * FUNCTION:      HLCall::~HLCall
 * OVERVIEW:      Destructor
 * PARAMETERS:    BB - the enclosing basic block of this call
 * RETURNS:       <nothing>
 *============================================================================*/
HLCall::~HLCall() {
    std::list<Exp*>::iterator it;
    for (unsigned i = 0; i < arguments.size(); i++)
        delete arguments[i];
    if (postCallExpList) {
        for (it = postCallExpList->begin(); it != postCallExpList->end(); it++)
            delete *it;
        delete postCallExpList;
        postCallExpList = NULL;
    }
}

/*==============================================================================
 * FUNCTION:      HLCall::getArguments
 * OVERVIEW:      Return a copy of the locations that have been determined
 *                as the actual arguments for this call.
 * PARAMETERS:    <none>
 * RETURNS:       A reference to the list of arguments
 *============================================================================*/
std::vector<Exp*>& HLCall::getArguments() {
    return arguments;
}

Type *HLCall::getArgumentType(int i) {
    assert(i < (int)arguments.size());
    assert(procDest);
    return procDest->getSignature()->getParamType(i);
}

/*==============================================================================
 * FUNCTION:      HLCall::setArguments
 * OVERVIEW:      Set the arguments of this call.
 * PARAMETERS:    arguments - the list of locations that reach this call
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setArguments(std::vector<Exp*>& arguments) {
    this->arguments = arguments;
}

/*==============================================================================
 * FUNCTION:      HLCall::setSigArguments
 * OVERVIEW:      Set the arguments of this call based in signature info
 * PARAMETERS:    None
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setSigArguments() {
    int n = procDest->getSignature()->getNumParams();
    arguments.resize(n);
    for (int i = 0; i < n; i++) {
        Exp *e = procDest->getSignature()->getArgumentExp(i);
        assert(e);
        arguments[i] = e->clone();
    }
    if (procDest->getSignature()->hasEllipsis()) {
        // Just guess 10 parameters for now
        //for (int i = 0; i < 10; i++)
            arguments.push_back(procDest->getSignature()->
                            getArgumentExp(arguments.size())->clone());
    }
}

/*==============================================================================
 * FUNCTION:      HLCall::getReturnLoc
 * OVERVIEW:      Return the location that will be used to hold the value
 *                  returned by this call.
 * PARAMETERS:    <none>
 * RETURNS:       ptr to the location that will be used to hold the return value
 *============================================================================*/
Exp* HLCall::getReturnLoc() {
    return returnLoc;
}

void HLCall::setIgnoreReturnLoc(bool b) {
    if (b) { returnLoc = NULL; return; }
    assert(procDest);
    if (procDest->getSignature()->getReturnType()->isVoid())
        returnLoc = NULL;
    else
        returnLoc = procDest->getSignature()->getReturnExp()->clone();
}

Type* HLCall::getLeftType() {
    if (procDest == NULL || returnLoc == NULL)
        return new VoidType();
    return procDest->getSignature()->getReturnType();
}

#if 0
/*==============================================================================
 * FUNCTION:        HLCall::getUseDefLocations
 * OVERVIEW:        The DFA analysis of a call RTL extends that of a
 *                    standard RTL in that it *uses* its parameters.
 * PARAMETERS:      locMap - a map between locations and integers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::getUseDefLocations(LocationMap& locMap,
    LocationFilter* filter, BITSET& defSet, BITSET& useSet,
    BITSET& useUndefSet, Proc* proc) const {
    // Note: calls can have semantics now (mainly from restore instructions
    // in their delay slots).
    // So process the semantics (assignments) for this HLCall
    RTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        proc);

    // Calls are also jumps; the destination expression may use some locations
    HLJump::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        proc);

    // Get the set of locations that are parameters for the call
    // Use a type insensitive set
    setSgiExp params_set;
    for (std::list<Exp>::const_iterator it = params.begin();
      it != params.end(); it++)
        // We should not have vars here at this stage. These uses will be needed
        // for things like return location analysis, and the ReturnLocation
        // object will have strings like r[8], not v0
        if (it->getFirstIdx() != opVar)
            params_set.insert(*it);
    BITSET paramSet = locMap.toBitset(params_set);

    // Add each parameter to the use and if applicable the uneUndef set
    useSet |= paramSet;
    useUndefSet |= (paramSet & ~defSet);
}
#endif

/*==============================================================================
 * FUNCTION:         HLCall::returnsStruct
 * OVERVIEW:         Returns true if the function called by this call site
 *                   returns an aggregate value (i.e a struct, union or quad
 *                   floating point value).
 * PARAMETERS:       <none>
 * RETURNS:          the called function returns an aggregate value
 *============================================================================*/
bool HLCall::returnsStruct() {
    return (returnTypeSize != 0);
}

bool HLCall::search(Exp* search, Exp*& result) {
    result = NULL;
    if (returnLoc && returnLoc->search(search, result)) return true;
    for (unsigned i = 0; i < arguments.size(); i++)
        if (arguments[i]->search(search, result)) return true;
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin(); 
          it != postCallExpList->end(); it++)
            if ((*it)->search(search, result)) return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        HLCall::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::searchAndReplace(Exp* search, Exp* replace) {
    bool change;
    HLJump::searchAndReplace(search, replace);
    if (returnLoc != NULL)
        returnLoc = returnLoc->searchReplaceAll(search, replace, change);
    for (unsigned i = 0; i < arguments.size(); i++)
        arguments[i] = arguments[i]->searchReplaceAll(search, replace, change);
    // Also replace the postCall rtls, if any
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin();
          it != postCallExpList->end(); it++)
            *it = (*it)->searchReplaceAll(search, replace, change);
    }
}

/*==============================================================================
 * FUNCTION:        HLCall::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLCall::searchAll(Exp* search, std::list<Exp *>& result) {
    bool found = false;
    //if( HLJump::searchAll(search, result) ||
    //  (returnLoc != 0 && returnLoc->searchAll(search, result)))
    //    found = true;
    for (unsigned i = 0; i < arguments.size(); i++)
        if (arguments[i]->searchAll(search, result))
            found = true;
    // Also replace the postCall rtls, if any
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin();
          it != postCallExpList->end(); it++)
            if( (*it)->searchAll(search, result) )
                found = true;
    }
    return found;
}

/*==============================================================================
 * FUNCTION:        HLCall::print
 * OVERVIEW:        Write a text representation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 * RETURNS:         Nothing
 *============================================================================*/
void HLCall::print(std::ostream& os /*= cout*/, bool withDF) {
    // Calls can all have semantics (e.g. call/restore)
    if (expList.size() != 0)
        RTL::print(os, withDF);

    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " " << std::setfill(' ');
    os << std::dec << std::setw(4) << number << " ";    // Statement number

    // Print the return location if there is one
    if (getReturnLoc() != NULL)
        os << " " << getReturnLoc() << " := ";
 
    os << "CALL ";
    if (procDest)
        os << procDest->getName();
    else if (pDest == NULL)
            os << "*no dest*";
    else {
        // But Trent hacked out the opAddrConst (opCodeAddr) stuff... Sigh.
        // I'd like to retain the 0xHEX notation, if only to retain the
        // existing tests
        if (pDest->isIntConst())
            os << "0x" << std::hex << ((Const*)pDest)->getInt();
        else
            pDest->print(os);       // Could still be an expression
    }

    // Print the actual arguments of the call
    os << "(";    
    for (unsigned i = 0; i < arguments.size(); i++) {
        if (i != 0)
            os << ", ";
        os << arguments[i];
    }
    os << ")\n";

    // Print the post call RTLs, if any
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin();
          it != postCallExpList->end(); it++) {
            os << "         ";
            // Statement number
            os << std::dec << std::setw(4) <<
              dynamic_cast<Statement*>(*it)->getNumber() << " "; 
            (*it)->print(os, withDF);       // Don't use << to respect withDF
            os << "\n";
        }
    }
}

/*==============================================================================
 * FUNCTION:         HLCall::setReturnAfterCall
 * OVERVIEW:         Sets a bit that says that this call is effectively followed
 *                      by a return. This happens e.g. on Sparc when there is a
 *                      restore in the delay slot of the call
 * PARAMETERS:       b: true if this is to be set; false to clear the bit
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setReturnAfterCall(bool b) {
    returnAfterCall = b;
}

/*==============================================================================
 * FUNCTION:         HLCall::isReturnAfterCall
 * OVERVIEW:         Tests a bit that says that this call is effectively
 *                      followed by a return. This happens e.g. on Sparc when
 *                      there is a restore in the delay slot of the call
 * PARAMETERS:       none
 * RETURNS:          True if this call is effectively followed by a return
 *============================================================================*/
bool HLCall::isReturnAfterCall() {
    return returnAfterCall;
}

/*==============================================================================
 * FUNCTION:         HLCall::setPostCallExpList
 * OVERVIEW:         Sets the list of Exps to be emitted after the call
 * PARAMETERS:       Pointer to the list of Exps to be saved
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setPostCallExpList(std::list<Exp*>* le) {
    postCallExpList = le;
}

/*==============================================================================
 * FUNCTION:         HLCall::getPostCallExpList
 * OVERVIEW:         Gets the list of Exps to be emitted after the call
 * PARAMETERS:       <None>
 * RETURNS:          List of Exps to be emitted
 *============================================================================*/
std::list<Exp*>* HLCall::getPostCallExpList() {
    return postCallExpList;
}

/*==============================================================================
 * FUNCTION:        HLCall::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this HLCall
 *============================================================================*/
RTL* HLCall::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++)
        le.push_back((*it)->clone());
    HLCall* ret = new HLCall(nativeAddr, returnTypeSize, &le);
    ret->pDest = pDest->clone();
    ret->m_isComputed = m_isComputed;
    ret->arguments = arguments;
    ret->numNativeBytes = numNativeBytes;
    ret->returnLoc = returnLoc;
    return ret;
}

// visit this rtl
bool HLCall::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

// serialize this rtl
bool HLCall::serialize_rest(std::ostream &ouf) {
    HLJump::serialize_rest(ouf);

    if (procDest) {
        saveFID(ouf, FID_RTL_CALLDESTSTR);
        saveString(ouf, std::string(procDest->getName()));
    }

    return true;
}

// deserialize an rtl
bool HLCall::deserialize_fid(std::istream &inf, int fid) {
    switch (fid) {
        case FID_RTL_CALLDESTSTR:
            loadString(inf, destStr);           
            break;
        default:
            return HLJump::deserialize_fid(inf, fid);
    }

    return true;
}

Proc* HLCall::getDestProc() {
    return procDest; 
}

void HLCall::setDestProc(Proc* dest) { 
    assert(dest);
    assert(procDest == NULL);
    procDest = dest;
    destStr = procDest->getName();
}

void HLCall::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    LocationSet defs;
    getDefinitions(defs);

    // Generate code for low level semantics (if present)
    RTL::generateCode(hll, pbb, indLevel);
    
    Proc *p = getDestProc();

    if (p == NULL && isComputed()) {
        hll->AddIndCallStatement(indLevel, getReturnLoc(), pDest, arguments);
        return;
    }

#if 0
    std::cerr << "call: ";
    print(std::cerr, false);
    std::cerr << "in proc " << proc->getName() << std::endl;
#endif
    assert(p);
    hll->AddCallStatement(indLevel, getReturnLoc(), p, arguments, defs);
}

void HLCall::simplify() {
    HLJump::simplify();
    for (unsigned i = 0; i < arguments.size(); i++) {
        Exp *e = arguments[i]->simplifyArith()->clone();
        delete arguments[i];
        arguments[i] = e->simplify();
    }
}

void HLCall::decompile() {
    if (procDest) { 
        UserProc *p = dynamic_cast<UserProc*>(procDest);
        if (p != NULL)
            p->decompile();

        // FIXME: Likely there is a much better place to do this
        // init return location
        setIgnoreReturnLoc(false);
    } else {
        // TODO: indirect call
    }
}

void HLCall::clearLiveEntry() {
    if (procDest && procDest->isLib()) return;
    // Now is the time to let go of the summarised info
    liveEntry.clear();
    // Start parameters from scratch too
    arguments.clear();
}

void HLCall::truncateArguments() {
    // Needs a total rewrite
}

void HLCall::killDef(StatementSet &reach) {
    if (procDest == NULL) {
        // Will always be null for indirect calls
        // MVE: we may have a "candidate" callee in the future
        // Kill nothing. For calls, underestimating kills is safe
        return;
    }
    if (procDest->isLib()) {
        // A library function. We use the calling convention to find
        // out what is killed.
        Prog* prog = procDest->getProg();
        std::list<Exp*> *li = procDest->getSignature()->getCallerSave(prog);
        assert(li);
        std::list<Exp*>::iterator ll;
        for (ll = li->begin(); ll != li->end(); ll++) {
            // These statements do not reach the end of the call
            reach.removeIfDefines(*ll);
        }
        return;
    }

    // A UserProc
    // Don't kill anything. The interprocedural analysis handles the effects
    // of the callee now
}

void HLCall::killLive(LocationSet &live) {
    if (procDest == NULL) {
        // Will always be null for indirect calls
        // MVE: we may have a "candidate" callee in the future
        // Kills everything. Not clear that this is always "conservative"
        live.clear();
        return;
    }
    if (procDest->isLib()) {
        // A library function. We use the calling convention to find
        // out what is killed.
        Prog* prog = procDest->getProg();
        std::list<Exp*> *li = procDest->getSignature()->getCallerSave(prog);
        assert(li);
        std::list<Exp*>::iterator ll;
        for (ll = li->begin(); ll != li->end(); ll++) {
            // These locations are no longer live at the start of the call
            live.remove(*ll);
        }
        return;
    }

    // A UserProc
    // This call kills only those live locations that are defined
    // on all paths, which is the same set that is available at the exit
    // of the procedure
    live.removeIfDefines(*((UserProc*)procDest)->getCFG()->getAvailExit());
}

void HLCall::killDead(LocationSet &dead) {
    // All parameters kill deadness
    int n = arguments.size();
    for (int i = 0; i < n; i++)
        dead.remove(arguments[i]);
}

// MVE: Probably not needed, and probably not correct
void HLCall::getDeadStatements(StatementSet &dead) {
    StatementSet reach;
    getReachIn(reach, 2);
    StmtSetIter it;
    if (procDest && procDest->isLib()) {
        for (Statement* s = reach.getFirst(it); s; s = reach.getNext(it)) {
            bool isKilled = false;
            if (getReturnLoc() && s->getLeft() &&
                *s->getLeft() == *getReturnLoc())
                isKilled = true;
            if (s->getLeft() && getReturnLoc() && 
                s->getLeft()->isMemOf() && getReturnLoc()->isMemOf())
                isKilled = true; // might alias, very conservative
            if (isKilled && s->getNumUsedBy() == 0)
            dead.insert(s);
        }
    } else  {
        for (Statement* s = reach.getFirst(it); s; s = reach.getNext(it)) {
            if (s->getNumUsedBy() == 0)
                dead.insert(s);
        }
    }
}

// update type for expression
Type *HLCall::updateType(Exp *e, Type *curType) {
    return curType;
}

bool HLCall::usesExp(Exp *e) {
    Exp *where;
    for (unsigned i = 0; i < arguments.size(); i++) {
        if (arguments[i]->search(e, where)) {
            return true;
        }
    }
    if (returnLoc && returnLoc->isMemOf())
        return ((Unary*)returnLoc)->getSubExp1()->search(e, where);
    if (procDest == NULL)
        // No destination (e.g. indirect call)
        // For now, just return true (overstating uses is safe)
        return true;
    if (!procDest->isLib()) {
        // Get the info that was summarised on the way down
        if (liveEntry.find(e)) return true;
    }
    return false;
}

// Add all locations that this call uses
void HLCall::addUsedLocs(LocationSet& used) {
    for (unsigned i = 0; i < arguments.size(); i++)
        arguments[i]->addUsedLocs(used);
    if (returnLoc && returnLoc->isMemOf())
        ((Unary*)returnLoc)->getSubExp1()->addUsedLocs(used);
}

bool HLCall::isDefinition() 
{
    LocationSet defs;
    getDefinitions(defs);
    return defs.size() != 0;
}

// MVE: likely not correct to use this any more
void HLCall::getDefinitions(LocationSet &defs)
{
    if (procDest) {
        if (procDest->isLib()) {
            Exp *e = getLeft();
            if (e) defs.insert(e);
        } else {
            ((UserProc*)procDest)->getReturnSet(defs);
        }
    } else {
        // TODO: computed call
    }
}

void HLCall::doReplaceUse(Statement *use) {
    Exp *left = use->getLeft()->clone();        // Note: could be changed!
    Exp *right = use->getRight()->clone();
    assert(left);
    assert(right);
    bool change = false;

    for (unsigned i = 0; i < arguments.size(); i++) {
        arguments[i] = arguments[i]->searchReplaceAll(left, right, change);
        arguments[i] = arguments[i]->simplifyArith();
        arguments[i] = arguments[i]->simplify();
    }
#if 0       // Call later
    processConstants(proc->getProg());
    if (getDestProc() && getDestProc()->getSignature()->hasEllipsis()) {
        // functions like printf almost always have too many args
        std::string name(getDestProc()->getName());
        if ((name == "printf" || name == "scanf") &&
          getArgumentExp(0)->isStrConst()) {
            char *str = ((Const*)getArgumentExp(0))->getStr();
            // actually have to parse it
            int n = 1;      // Number of %s plus 1 = number of args
            char *p = str;
            while ((p = strchr(p, '%'))) {
                // special hack for scanf
                if (name == "scanf") {
                    setArgumentExp(n, new Unary(opAddrOf, 
                        new Unary(opMemOf, getArgumentExp(n))));
                }
                p++;
                switch(*p) {
                    case '%':
                        break;
                        // TODO: there's type information here
                    default: 
                        n++;
                }
                p++;
            }
            setNumArguments(n);
        }
    }
#endif
}

// MVE: is this needed after the merge?
void HLCall::setNumArguments(int n) {
    int oldSize = arguments.size();
    arguments.resize(n);
    // printf, scanf start with just 2 arguments
    for (int i = oldSize; i < n; i++) {
        arguments[i] = procDest->getSignature()->getArgumentExp(i)->clone();
    }
}

// Update the arguments to be in implicit SSA form (e.g. m[esp{1}]{2 3})
void HLCall::toSSAform(StatementSet& reachin, int memDepth) {
    int n = arguments.size();
    for (int i = 0; i < n; i++) {
        arguments[i] = arguments[i]->updateRefs(reachin, memDepth);
    }
}

// Convert from SSA form
void HLCall::fromSSAform(igraph& ig) {

}

void HLCall::processConstants(Prog *prog) {
    for (unsigned i = 0; i < arguments.size(); i++) {
        Type *t = getArgumentType(i);
        if (t == NULL) continue;
        // char* and a constant
        if (arguments[i]->isIntConst()) {
            if (t->isPointer()) {
                PointerType *pt = (PointerType*)t;
                if (pt->getPointsTo()->isChar()) {
                    char *str = 
                        prog->getStringConstant(((Const*)arguments[i])->getAddr());
                    if (str) {
                        std::string s(str);
                        while (s.find('\n') != (unsigned)-1)
                            s.replace(s.find('\n'), 1, "\\n");
                        delete arguments[i];
                        arguments[i] = new Const(strdup(s.c_str()));
                    }
                }
                if (pt->getPointsTo()->isFunc()) {
                    ADDRESS a = ((Const*)arguments[i])->getAddr();
                    prog->decode(a);
                }
            } else if (t->isFloat()) {
                arguments[i]->setOper(opFltConst);
            }
        }
#if 0
        if (t->isPointer() && arguments[i]->getOper() != opAddrOf) {
            arguments[i] = new Unary(opAddrOf, 
                                     new Unary(opMemOf, arguments[i]));
        }
#endif
    }

    // This code was in HLCall:doReplaceUse()
    if (getDestProc() && getDestProc()->getSignature()->hasEllipsis()) {
        // functions like printf almost always have too many args
        std::string name(getDestProc()->getName());
        if ((name == "printf" || name == "scanf") &&
          getArgumentExp(0)->isStrConst()) {
            char *str = ((Const*)getArgumentExp(0))->getStr();
            // actually have to parse it
            int n = 1;      // Number of %s plus 1 = number of args
            char *p = str;
            while ((p = strchr(p, '%'))) {
                // special hack for scanf
                if (name == "scanf") {
                    setArgumentExp(n, new Unary(opAddrOf,
                        new Unary(opMemOf, getArgumentExp(n))));
                }
                p++;
                switch(*p) {
                    case '%':
                        break;
                        // TODO: there's type information here
                    default:
                        n++;
                }
                p++;
            }
            setNumArguments(n);
        }
    }

}

/**********************************
 * HLReturn methods
 **********************************/

/*==============================================================================
 * FUNCTION:         HLReturn::HLReturn
 * OVERVIEW:         Constructor.
 * PARAMETERS:       instNativeAddr - the address of the return instruction
 *                   listRT - the RTs of the return
 * RETURNS:          <nothing>
 *============================================================================*/
HLReturn::HLReturn(ADDRESS instNativeAddr, std::list<Exp*>* le /*= NULL*/):
    HLJump(instNativeAddr, le), nBytesPopped(0), returnVal(NULL) {
    kind = RET_RTL;
}

/*==============================================================================
 * FUNCTION:         HLReturn::~HLReturn
 * OVERVIEW:         Destructor.
 * PARAMETERS:       <none>
 * RETURNS:          <nothing>
 *============================================================================*/
HLReturn::~HLReturn() {
    if (returnVal)
        delete returnVal;
}

/*==============================================================================
 * FUNCTION:        HLReturn::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this HLReturn
 *============================================================================*/
RTL* HLReturn::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++)
        le.push_back((*it)->clone());
    HLReturn* ret = new HLReturn(nativeAddr, &le);
    ret->pDest = NULL;                      // pDest should be null
    ret->m_isComputed = m_isComputed;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

// visit this rtl
bool HLReturn::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

// serialize this rtl
bool HLReturn::serialize_rest(std::ostream &ouf) {
    HLJump::serialize_rest(ouf);

    return true;
}

// deserialize an rtl
bool HLReturn::deserialize_fid(std::istream &inf, int fid) {
    switch (fid) {
        default:
            return HLJump::deserialize_fid(inf, fid);
    }

    return true;
}

void HLReturn::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    // There could be semantics, e.g. SPARC RETURN instruction
    // Most of the time, the list of RTs will be empty, and the
    // below does nothing
    RTL::generateCode(hll, pbb, indLevel);
}

void HLReturn::simplify() {
    if (returnVal)
        returnVal = returnVal->simplify();
}


/**********************************
 * HLScond methods
 **********************************/

/*==============================================================================
 * FUNCTION:         HLScond::HLScond
 * OVERVIEW:         Constructor.
 * PARAMETERS:       instNativeAddr - the address of the set instruction
 *                   listRT - the RTs of the instr
 * RETURNS:          <N/a>
 *============================================================================*/
HLScond::HLScond(ADDRESS instNativeAddr, std::list<Exp*>* le /*= NULL*/):
  RTL(instNativeAddr, le), jtCond((JCOND_TYPE)0), pCond(NULL), pDest(NULL) {
    kind = SCOND_RTL;
}

/*==============================================================================
 * FUNCTION:        HLScond::~HLScond
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLScond::~HLScond() {
    if (pCond)
        delete pCond;
}

/*==============================================================================
 * FUNCTION:        HLScond::setCondType
 * OVERVIEW:        Sets the JCOND_TYPE of this jcond as well as the flag
 *                  indicating whether or not the floating point condition codes
 *                  are used.
 * PARAMETERS:      cond - the JCOND_TYPE
 *                  usesFloat - this condional jump checks the floating point
 *                    condition codes
 * RETURNS:         a semantic string
 *============================================================================*/
void HLScond::setCondType(JCOND_TYPE cond, bool usesFloat /*= false*/) {
    jtCond = cond;
    bFloat = usesFloat;
    setCondExpr(new Terminal(opFlags));
    getDest();
}

/*==============================================================================
 * FUNCTION:        HLScond::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * NOTE:            Not sure if this is ever going to be used
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::makeSigned() {
    // Make this into a signed branch
    switch (jtCond)
    {
        case HLJCOND_JUL : jtCond = HLJCOND_JSL;  break;
        case HLJCOND_JULE: jtCond = HLJCOND_JSLE; break;
        case HLJCOND_JUGE: jtCond = HLJCOND_JSGE; break;
        case HLJCOND_JUG : jtCond = HLJCOND_JSG;  break;
        default:
            // Do nothing for other cases
            break;
    }
}

/*==============================================================================
 * FUNCTION:        HLScond::getCondExpr
 * OVERVIEW:        Return the Exp expression containing the HL condition.
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
Exp* HLScond::getCondExpr() {
    return pCond;
}

/*==============================================================================
 * FUNCTION:        HLScond::setCondExpr
 * OVERVIEW:        Set the Exp expression containing the HL condition.
 * PARAMETERS:      Pointer to semantic string to set
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::setCondExpr(Exp* pss) {
    if (pCond) delete pCond;
    pCond = pss;
}

/*==============================================================================
 * FUNCTION:        HLScond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         <Nothing>
 *============================================================================*/
void HLScond::print(std::ostream& os /*= cout*/, bool withDF) {
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " " << std::setfill(' ');
    os << std::setw(4) << number << " ";
    os << "SCOND ";
    getDest()->print(os);
    os << " := CC(";
    switch (jtCond)
    {
        case HLJCOND_JE:    os << "equals"; break;
        case HLJCOND_JNE:   os << "not equals"; break;
        case HLJCOND_JSL:   os << "signed less"; break;
        case HLJCOND_JSLE:  os << "signed less or equals"; break;
        case HLJCOND_JSGE:  os << "signed greater or equals"; break;
        case HLJCOND_JSG:   os << "signed greater"; break;
        case HLJCOND_JUL:   os << "unsigned less"; break;
        case HLJCOND_JULE:  os << "unsigned less or equals"; break;
        case HLJCOND_JUGE:  os << "unsigned greater or equals"; break;
        case HLJCOND_JUG:   os << "unsigned greater"; break;
        case HLJCOND_JMI:   os << "minus"; break;
        case HLJCOND_JPOS:  os << "plus"; break;
        case HLJCOND_JOF:   os << "overflow"; break;
        case HLJCOND_JNOF:  os << "no overflow"; break;
        case HLJCOND_JPAR:  os << "parity"; break;
    }
    os << ")";
    if (bFloat) os << ", float";
    os << std::endl;
    if (pCond) {
        os << "High level: ";
        pCond->print(os);
        os << std::endl;
    }
}

/*==============================================================================
 * FUNCTION:        HLScond::getDest
 * OVERVIEW:        Get the destination of the set. For now, we assume one
 *                  assignment Exp, and we take the left hand side of that.
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to the expression representing the lvalue location
 *============================================================================*/
Exp* HLScond::getDest() {
    if (pDest) return pDest;
    assert(expList.size());
    Exp* pAsgn = expList.front();
    assert(pAsgn->isAssign());
    pDest = ((Binary*)pAsgn)->getSubExp1()->clone();
    delete pAsgn;
    expList.erase(expList.begin());
    return pDest;
}

/*==============================================================================
 * FUNCTION:        HLScond::getSize
 * OVERVIEW:        Get the size of the set's assignment. For now, we assume
 *                  one assignment Exp, and we take the size of that.
 * PARAMETERS:      <none>
 * RETURNS:         The size
 *============================================================================*/
int HLScond::getSize() {
    assert(expList.size());
    Exp* first = expList.front();
    assert(first->isAssign());
    return ((AssignExp*)first)->getSize();
}

/*==============================================================================
 * FUNCTION:        HLScond::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this HLScond
 *============================================================================*/
RTL* HLScond::clone() {
    std::list<Exp*> le;
    std::list<Exp*>::iterator it;
    for (it = expList.begin(); it != expList.end(); it++)
        le.push_back((*it)->clone());
    HLScond* ret = new HLScond(nativeAddr, &le);
    ret->jtCond = jtCond;
    if (pCond) ret->pCond = pCond->clone();
    else ret->pCond = NULL;
    ret->bFloat = bFloat;
    ret->numNativeBytes = numNativeBytes;
    return ret;
}

// visit this rtl
bool HLScond::accept(RTLVisitor* visitor) {
    return visitor->visit(this);
}

// serialize this rtl
bool HLScond::serialize_rest(std::ostream &ouf) {
    return true;
}

// deserialize an rtl
bool HLScond::deserialize_fid(std::istream &inf, int fid) {
    switch (fid) {
        default:
            return RTL::deserialize_fid(inf, fid);
    }

    return true;
}

void HLScond::generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel) {
    RTL::generateCode(hll, pbb, indLevel);
}

void HLScond::simplify() {
    RTL::simplify();
}

void HLScond::killDef(StatementSet &reach)
{
    assert(pDest);
    StatementSet kills;
    StmtSetIter it;
    for (Statement* s = reach.getFirst(it); s; s = reach.getNext(it)) {
        if (s->getLeft() && *s->getLeft() == *pDest)
            kills.insert(s);
    }
    for (Statement* s = kills.getFirst(it); s; s = kills.getNext(it))
        reach.remove(s);
}

// Liveness is killed by a definition
void HLScond::killLive(LocationSet &live) {
    if (pDest == NULL) return;
    LocSetIter it;
    for (Exp* loc = live.getFirst(it); loc; loc = live.getNext(it)) {
        // MVE: do we need to consider aliasing?
        if (*loc == *pDest)
            live.remove(loc);
    }
}

// Deadness is killed by a use
void HLScond::killDead(LocationSet &dead) {
    if (pCond)
        dead.remove(pCond);
}


// Probably not needed, and probably not right
void HLScond::getDeadStatements(StatementSet &dead)
{
    assert(pDest);
    StatementSet reach;
    getReachIn(reach, 2);
    StmtSetIter it;
    for (Statement* s = reach.getFirst(it); s; s = reach.getNext(it)) {
        if (s->getLeft() && *s->getLeft() == *pDest && 
            s->getNumUsedBy() == 0)
            dead.insert(s);
    }
}

void HLScond::getDefinitions(LocationSet &defs) 
{
    defs.insert(getLeft());
}

Type* HLScond::getLeftType()
{
    return new BooleanType();
}

bool HLScond::usesExp(Exp *e)
{
    assert(pDest && pCond);
    Exp *where = 0;
    return (pCond->search(e, where) || (pDest->isMemOf() && 
        ((Unary*)pDest)->getSubExp1()->search(e, where)));
}

#if 0
void HLScond::printAsUse(std::ostream &os)
{
    os << "SCOND ";
    getDest()->print(os);
    os << " := ";
    if (pCond)
        pCond->print(os);
    else
        os << "<empty cond>";
}

void HLScond::printAsUseBy(std::ostream &os)
{
    printAsUse(os);
}
#endif

void HLScond::processConstants(Prog *prog)
{
}

bool HLScond::search(Exp *search, Exp *&result)
{
    assert(pDest);
    if (pDest->search(search, result)) return true;
    assert(pCond);
    return pCond->search(search, result);
}

void HLScond::searchAndReplace(Exp *search, Exp *replace)
{
    bool change;
    assert(pCond);
    assert(pDest);
    pCond = pCond->searchReplaceAll(search, replace, change);
    pDest = pDest->searchReplaceAll(search, replace, change);
}

Type* HLScond::updateType(Exp *e, Type *curType)
{
    delete curType;
    return new BooleanType();
}

// Convert from SSA form
void HLScond::fromSSAform(igraph& ig) {
    // To be completed
}

void HLScond::doReplaceUse(Statement *use)
{
    searchAndReplace(use->getLeft(), use->getRight());
    simplify();
}

void HLScond::addUsedLocs(LocationSet& used) {
    if (pCond)
        pCond->addUsedLocs(used);
}


/*==============================================================================
 * FUNCTION:         CallBB:setPhase1
 * OVERVIEW:         Set up for phase 1 of [SW93]. Basuically, keeps a copy
 *                   of the outedge in returnBlock, and points the outedge
 *                   to the actual callee entry BB
 * PARAMETERS:       none
 * RETURNS:          <nothing>
 *============================================================================*/
