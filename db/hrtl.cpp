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
 * 03 Dec 02 - Mike: Made a small mod to HLCall::killLive for indirect calls
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
    : RTL(instNativeAddr, listExp), pDest(NULL), m_isComputed(false)
{
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
    RTL(instNativeAddr), m_isComputed(false)
{
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
HLJump::~HLJump()
{
    if (pDest) delete pDest;
}

/*==============================================================================
 * FUNCTION:        HLJump::getFixedDest
 * OVERVIEW:        Get the fixed destination of this CTI. Assumes destination
 *                  simplication has already been done so that a fixed dest will
 *                  be of the Exp form:
 *                     opIntConst dest
 * PARAMETERS:      <none>
 * RETURNS:         Fixed dest or -1 if there isn't one
 *============================================================================*/
ADDRESS HLJump::getFixedDest() 
{
    if (pDest->getOper() != opAddrConst) return NO_ADDRESS;
    return ((Const*)pDest)->getAddr();
}

/*==============================================================================
 * FUNCTION:        HLJump::setDest
 * OVERVIEW:        Set the destination of this CTI to be a given address.
 * PARAMETERS:      addr - the new fixed address
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::setDest(Exp* pd)
{
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
void HLJump::setDest(ADDRESS addr)
{
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
Exp* HLJump::getDest() 
{
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
void HLJump::adjustFixedDest(int delta)
{
    // Ensure that the destination is fixed.
    if (pDest == 0 || pDest->getOper() != opAddrConst)
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
void HLJump::searchAndReplace(Exp* search, Exp* replace)
{
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
bool HLJump::searchAll(Exp* search, std::list<Exp*> &result)
{
    return RTL::searchAll(search, result) ||
        ( pDest && pDest->searchAll(search, result) );
}

/*==============================================================================
 * FUNCTION:        HLJump::print
 * OVERVIEW:        Display a text reprentation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::print(std::ostream& os /*= cout*/, bool withDF)
{
    // Returns can all have semantics (e.g. ret/restore)
    if (expList.size() != 0)
        RTL::print(os, withDF);

    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " ";
    if (getKind() == RET_RTL)
    {
        os << "RET\n";             // RET is a special case of a JUMP_RTL
        return;
    }

    os << "JUMP ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (pDest->getOper() != opIntConst)
         pDest->print(os);
    else
        os << std::hex << getFixedDest();
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
void HLJump::setIsComputed(bool b)
{
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
bool HLJump::isComputed() 
{
    return m_isComputed;
}

/*==============================================================================
 * FUNCTION:        HLJump::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this one
 *============================================================================*/
RTL* HLJump::clone() 
{
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
    BITSET& defSet, BITSET& useSet, BITSET& useUndefSet, Proc* proc) const
{
    // If jumps ever have semantics, then this call would be needed
    // RTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        // proc);

    if (pDest)
        searchExprForUses(pDest, locMap, filter, defSet, useSet, useUndefSet);
}
#endif

// serialize this rtl
bool HLJump::serialize_rest(std::ostream &ouf)
{
    if (pDest && pDest->getOper() == opAddrConst) {
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
bool HLJump::deserialize_fid(std::istream &inf, int fid)
{
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
                if (pDest->getOper() != opAddrConst)
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

void HLJump::generateCode(HLLCode &hll, BasicBlock *pbb)
{
    // dont generate any code for jumps, they will be handled by the BB
}

void HLJump::simplify()
{
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
    bFloat(false)
{
    kind = JCOND_RTL;
}

/*==============================================================================
 * FUNCTION:        HLJcond::~HLJcond
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLJcond::~HLJcond()
{
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
void HLJcond::setCondType(JCOND_TYPE cond, bool usesFloat /*= false*/)
{
    jtCond = cond;
    bFloat = usesFloat;

    if (bFloat) return;

    // set pCond to a high level representation of this type
    Exp* p = NULL;
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
    assert(p);
    setCondExpr(p);
}

/*==============================================================================
 * FUNCTION:        HLJcond::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::makeSigned()
{
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
Exp* HLJcond::getCondExpr()
{
    return pCond;
}

/*==============================================================================
 * FUNCTION:        HLJcond::setCondExpr
 * OVERVIEW:        Set the SemStr expression containing the HL condition.
 * PARAMETERS:      Pointer to Exp to set
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::setCondExpr(Exp* e)
{
    if (pCond) delete pCond;
    pCond = e;
}

/*==============================================================================
 * FUNCTION:        HLJcond::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::searchAndReplace(Exp* search, Exp* replace)
{
    HLJump::searchAndReplace(search, replace);
    bool change;
    if (pCond)
        pCond = pCond->searchReplaceAll(search, replace, change);
}

/*==============================================================================
 * FUNCTION:        HLJCond::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJcond::searchAll(Exp* search, std::list<Exp*> &result)
{
    return RTL::searchAll(search, result) ||
      (pCond && (pCond->searchAll(search, result)));
}


/*==============================================================================
 * FUNCTION:        HLJcond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         Nothing
 *============================================================================*/
void HLJcond::print(std::ostream& os /*= cout*/, bool withDF)
{
    // These can have semantics (e.g. pa-risc add and (conditionally) branch)
    if (expList.size() != 0)
        RTL::print(os, withDF);
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " ";
    os << "JCOND ";
    if (pDest == NULL)
        os << "*no dest*";
    else if (!pDest->isIntConst())
        os << pDest;
    else {
        // Really we'd like to display the destination label here...
        os << std::hex << getFixedDest();
    }
    os << ", condition ";
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
    if (bFloat) os << " float";
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
RTL* HLJcond::clone()
{
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
bool HLJcond::serialize_rest(std::ostream &ouf)
{
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
bool HLJcond::deserialize_fid(std::istream &inf, int fid)
{
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

void HLJcond::generateCode(HLLCode &hll, BasicBlock *pbb)
{
    // dont generate any code for jconds, they will be handled by the bb
}

bool HLJcond::usesExp(Exp *e)
{
    Exp *tmp;
    return pCond && pCond->search(e, tmp);
}

// custom printing functions
void HLJcond::printWithUses(std::ostream& os)
{
}

// special print functions
void HLJcond::printAsUse(std::ostream &os)
{
    if (pCond)
        pCond->print(os);
    else
    os << "<empty cond>";
}

void HLJcond::printAsUseBy(std::ostream &os)
{
    if (pCond)
        pCond->print(os);
    else
    os << "<empty cond>";
}

// inline any constants in the statement
void HLJcond::inlineConstants(Prog *prog)
{
}

void HLJcond::doReplaceUse(Statement *use)
{
    bool change;
    assert(pCond);
    pCond = pCond->searchReplaceAll(use->getLeft(), use->getRight(), change);
    simplify();
}

void HLJcond::simplify()
{
    if (pCond) {
        Exp *e = pCond->simplifyArith()->clone();
        delete pCond;
        pCond = e->simplify();

        std::stringstream os;
        pCond->print(os);
        std::string s = os.str();

        // special simplifications
        switch(jtCond) {
            case HLJCOND_JE:    // Jump if equals               
                break;
            case HLJCOND_JNE:   // Jump if not equals
                break;
            case HLJCOND_JSL:   // Jump if signed less
                break;
            case HLJCOND_JSLE:  // Jump if signed less or equal
                break;
            case HLJCOND_JSGE:  // Jump if signed greater or equal
                break;
            case HLJCOND_JSG:   // Jump if signed greater
                break;
            case HLJCOND_JUL:   // Jump if unsigned less
                break;
            case HLJCOND_JULE:  // Jump if unsigned less or equal
                break;
            case HLJCOND_JUGE:  // Jump if unsigned greater or equal
                break;
            case HLJCOND_JUG:   // Jump if unsigned greater
                break;
            case HLJCOND_JMI:   // Jump if result is minus
                break;
            case HLJCOND_JPOS:  // Jump if result is positive
                break;
            case HLJCOND_JOF:   // Jump if overflow
                break;
            case HLJCOND_JNOF:  // Jump if no overflow
                break;
            case HLJCOND_JPAR:  // Jump if parity even (Intel only)
                break;
        }
    }
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
    HLJump(instNativeAddr, le), pSwitchInfo(NULL)
{
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
HLNwayJump::~HLNwayJump()
{
    if (pSwitchInfo)
        delete pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::getSwitchInfo
 * OVERVIEW:        Return a pointer to a struct with switch information in it
 * PARAMETERS:      <none>
 * RETURNS:         a semantic string
 *============================================================================*/
SWITCH_INFO* HLNwayJump::getSwitchInfo()
{
    return pSwitchInfo;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::setSwitchInfo
 * OVERVIEW:        Set a pointer to a SWITCH_INFO struct
 * PARAMETERS:      Pointer to SWITCH_INFO struct
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::setSwitchInfo(SWITCH_INFO* psi)
{
    pSwitchInfo = psi;
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::searchAndReplace(Exp* search, Exp* replace)
{
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
bool HLNwayJump::searchAll(Exp* search, std::list<Exp*> &result)
{
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
void HLNwayJump::print(std::ostream& os /*= cout*/, bool withDF)
{
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
RTL* HLNwayJump::clone()
{
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
bool HLNwayJump::serialize_rest(std::ostream &ouf)
{
    return true;
}

// deserialize an rtl
bool HLNwayJump::deserialize_fid(std::istream &inf, int fid)
{
    switch (fid) {
        default:
            return RTL::deserialize_fid(inf, fid);
    }

    return true;
}

void HLNwayJump::generateCode(HLLCode &hll, BasicBlock *pbb)
{
    // dont generate any code for switches, they will be handled by the bb
}

void HLNwayJump::simplify()
{
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
  std::list<Exp*>* le /*= NULL*/):

    HLJump(instNativeAddr, le),returnTypeSize(returnTypeSize),
      returnAfterCall(false), returnLoc(NULL)
{
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
HLCall::~HLCall()
{
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
std::vector<Exp*>& HLCall::getArguments()
{
    return arguments;
}

Type *HLCall::getArgumentType(int i)
{
    assert(i < (int)arguments.size());
    assert(procDest);
    return procDest->getSignature()->getParamType(i);
}

/*==============================================================================
 * FUNCTION:      HLCall::setArguments
 * OVERVIEW:      Set the arguments of this call.
 * PARAMETERS:    arguments - the list of locations live at this call
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setArguments(std::vector<Exp*>& arguments)
{
    this->arguments = arguments;
}

/*==============================================================================
 * FUNCTION:      HLCall::getReturnLoc
 * OVERVIEW:      Return the location that will be used to hold the value
 *                  returned by this call.
 * PARAMETERS:    <none>
 * RETURNS:       ptr to the location that will be used to hold the return value
 *============================================================================*/
Exp* HLCall::getReturnLoc() 
{
    return returnLoc;
}

void HLCall::setIgnoreReturnLoc(bool b)
{
    if (b) { returnLoc = NULL; return; }
    assert(procDest);
    returnLoc = procDest->getSignature()->getReturnExp()->clone();
}

Type* HLCall::getLeftType()
{
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
    BITSET& useUndefSet, Proc* proc) const
{
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
bool HLCall::returnsStruct()
{
    return (returnTypeSize != 0);
}

/*==============================================================================
 * FUNCTION:        HLCall::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::searchAndReplace(Exp* search, Exp* replace)
{
    bool change;
    HLJump::searchAndReplace(search, replace);
    if (returnLoc != NULL)
        returnLoc = returnLoc->searchReplaceAll(search, replace, change);
    for (unsigned i = 0; i < arguments.size(); i++)
        arguments[i] = arguments[i]->searchReplaceAll(search, replace, change);
    // Also replace the postCall rtls, if any
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin(); it != postCallExpList->end(); it++)
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
bool HLCall::searchAll(Exp* search, std::list<Exp *>& result)
{
    bool found = false;
    //if( HLJump::searchAll(search, result) ||
    //  (returnLoc != 0 && returnLoc->searchAll(search, result)))
    //    found = true;
    for (unsigned i = 0; i < arguments.size(); i++)
        if (arguments[i]->searchAll(search, result))
            found = true;
    // Also replace the postCall rtls, if any
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin(); it != postCallExpList->end(); it++)
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
void HLCall::print(std::ostream& os /*= cout*/, bool withDF)
{
    // Calls can all have semantics (e.g. call/restore)
    if (expList.size() != 0)
        RTL::print(os, withDF);

    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " ";

    // Print the return location if there is one
    if (getReturnLoc() != NULL)
        os << " " << getReturnLoc() << " := ";
 
    os << "CALL ";
    if (procDest)
        os << procDest->getName();
    else if (pDest == NULL)
            os << "*no dest*";
    else {
        pDest->print(os);
    }

    // Print the actual arguments of the call
    os << "(";    
    for (unsigned i = 0; i < arguments.size(); i++) {
        if (i != 0)
            os << ", ";
        os << arguments[i];
    }
    os << ")" << std::endl;

    // Print the post call RTLs, if any
    if (postCallExpList) {
        for (std::list<Exp*>::iterator it = postCallExpList->begin(); it != postCallExpList->end(); it++)
        {
            os << " ";
            (*it)->print(os);
            os << "\n";
        }
    }
    
    if (withDF) {
    std::list<Statement*> &internal = getInternalStatements();
        for (std::list<Statement*>::iterator it = internal.begin(); 
             it != internal.end(); it++) {
            os << "internal ";
            (*it)->printWithUses(os);
            os << std::endl;
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
void HLCall::setReturnAfterCall(bool b)
{
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
bool HLCall::isReturnAfterCall() 
{
    return returnAfterCall;
}

/*==============================================================================
 * FUNCTION:         HLCall::setPostCallExpList
 * OVERVIEW:         Sets the list of Exps to be emitted after the call
 * PARAMETERS:       Pointer to the list of Exps to be saved
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setPostCallExpList(std::list<Exp*>* le)
{
    postCallExpList = le;
}

/*==============================================================================
 * FUNCTION:         HLCall::getPostCallExpList
 * OVERVIEW:         Gets the list of Exps to be emitted after the call
 * PARAMETERS:       <None>
 * RETURNS:          List of Exps to be emitted
 *============================================================================*/
std::list<Exp*>* HLCall::getPostCallExpList()
{
    return postCallExpList;
}

/*==============================================================================
 * FUNCTION:        HLCall::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this HLCall
 *============================================================================*/
RTL* HLCall::clone() 
{
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
bool HLCall::serialize_rest(std::ostream &ouf)
{
    HLJump::serialize_rest(ouf);

    if (procDest) {
        saveFID(ouf, FID_RTL_CALLDESTSTR);
        saveString(ouf, std::string(procDest->getName()));
    }

    return true;
}

// deserialize an rtl
bool HLCall::deserialize_fid(std::istream &inf, int fid)
{
    switch (fid) {
        case FID_RTL_CALLDESTSTR:
            loadString(inf, destStr);           
            break;
        default:
            return HLJump::deserialize_fid(inf, fid);
    }

    return true;
}

Proc* HLCall::getDestProc() 
{
    return procDest; 
}

void HLCall::setDestProc(Proc* dest) 
{ 
    assert(dest);
    assert(procDest == NULL);
    procDest = dest;
    destStr = procDest->getName();
}

void HLCall::generateCode(HLLCode &hll, BasicBlock *pbb)
{
    Proc *p = getDestProc();

    if (p == NULL && isComputed()) {
        hll.AddCallStatement(pbb, getReturnLoc(), pDest, arguments);
        return;
    }

    assert(p);

    hll.AddCallStatement(pbb, getReturnLoc(), p, arguments);
}

void HLCall::simplify()
{
    HLJump::simplify();
    for (unsigned i = 0; i < arguments.size(); i++) {
        Exp *e = arguments[i]->simplifyArith()->clone();
        delete arguments[i];
        arguments[i] = e->simplify();
    }
}

void HLCall::decompile()
{
    if (procDest) { 
    UserProc *p = dynamic_cast<UserProc*>(procDest);
    if (p != NULL)
            p->decompile();
        procDest->getInternalStatements(internal);
        // init arguments
        assert(arguments.size() == 0);
        arguments.resize(procDest->getSignature()->getNumParams());
        for (int i = 0; i < procDest->getSignature()->getNumParams(); i++)
            arguments[i] = procDest->getSignature()->getArgumentExp(i)->clone();
        if (procDest->getSignature()->hasEllipsis()) {
        for (int i = 0; i < 10; i++)
            arguments.push_back(procDest->getSignature()->
                getArgumentExp(arguments.size())->clone());
        }
    // init return location
    returnLoc = procDest->getSignature()->getReturnExp();
    if (returnLoc) returnLoc = returnLoc->clone();
    } else {
    // TODO
    }
}


void HLCall::printAsUse(std::ostream &os)
{
    // Print the return location if there is one
    if (getReturnLoc() != NULL)
        os << " " << getReturnLoc() << " := ";
 
    os << "CALL ";
    if (procDest)
        os << procDest->getName();
    else if (pDest == NULL)
            os << "*no dest*";
    else {
        pDest->print(os);
    }

    // Print the actual arguments of the call
    os << "(";    
    for (unsigned i = 0; i < arguments.size(); i++) {
        if (i != 0)
            os << ", ";
        os << arguments[i];
    }
    os << ")";
}

void HLCall::printAsUseBy(std::ostream &os)
{
    printAsUse(os);
}


void HLCall::killLive(std::set<Statement*> &live)
{
    if (procDest == NULL) {
    live.clear();
        return;
    }
    std::set<Statement*> kills;
    for (std::set<Statement*>::iterator it = live.begin(); it != live.end(); it++) {
        bool isKilled = false;
        if (getReturnLoc() && (*it)->getLeft() && 
        *(*it)->getLeft() == *getReturnLoc())
            isKilled = true;
        if (getReturnLoc() && (*it)->getLeft() &&
        (*it)->getLeft()->isMemOf() && getReturnLoc()->isMemOf())
            isKilled = true; // might alias, very conservative
        if (isKilled)
        kills.insert(*it);
    }
    for (std::set<Statement*>::iterator it = kills.begin(); it != kills.end(); it++)
        live.erase(*it);
}

void HLCall::getDeadStatements(std::set<Statement*> &dead)
{
    std::set<Statement*> live;
    getLiveIn(live);
    if (procDest && procDest->isLib()) {
        for (std::set<Statement*>::iterator it = live.begin(); 
         it != live.end(); it++) {
            bool isKilled = false;
            if (getReturnLoc() && (*it)->getLeft() &&
            *(*it)->getLeft() == *getReturnLoc())
                isKilled = true;
            if ((*it)->getLeft() && getReturnLoc() && 
                (*it)->getLeft()->isMemOf() && getReturnLoc()->isMemOf())
                isKilled = true; // might alias, very conservative
            if (isKilled && (*it)->getNumUseBy() == 0)
            dead.insert(*it);
        }
    } else  {
        for (std::set<Statement*>::iterator it = live.begin(); 
         it != live.end(); it++) 
        if ((*it)->getNumUseBy() == 0)
            dead.insert(*it);
    }
}

bool HLCall::usesExp(Exp *e)
{
    Exp *where = 0;
    for (unsigned i = 0; i < arguments.size(); i++)
        if (*arguments[i] == *e ||
        arguments[i]->search(e, where))
            return true;
    return false;
}

void HLCall::doReplaceUse(Statement *use)
{
    Exp *left = use->getLeft();
    Exp *right = use->getRight();
    assert(left);
    assert(right);
    bool change = false;
    for (unsigned i = 0; i < arguments.size(); i++) {
        if (*arguments[i] == *left) {
        arguments[i] = right->clone();
        change = true;
    } else {
            bool changeright = false;
            arguments[i]->searchReplaceAll(left, right->clone(), changeright);
        change |= changeright;
    }
    }
    assert(change);
    // simplify the arguments
    for (unsigned i = 0; i < arguments.size(); i++) {
        arguments[i] = arguments[i]->simplifyArith();
        arguments[i] = arguments[i]->simplify();
    }
}

void HLCall::printWithUses(std::ostream& os)
{
    // TODO
    assert(false);
}

void HLCall::inlineConstants(Prog *prog)
{
    for (unsigned i = 0; i < arguments.size(); i++) {
        Type *t = getArgumentType(i);
    // char* and a constant
    if ((arguments[i]->isAddrConst() || arguments[i]->isIntConst()) && 
        t && t->isPointer() && 
        ((PointerType*)t)->getPointsTo()->isChar()) {
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
    HLJump(instNativeAddr, le), nBytesPopped(0), returnVal(NULL)
{
    kind = RET_RTL;
}

/*==============================================================================
 * FUNCTION:         HLReturn::~HLReturn
 * OVERVIEW:         Destructor.
 * PARAMETERS:       <none>
 * RETURNS:          <nothing>
 *============================================================================*/
HLReturn::~HLReturn()
{
    if (returnVal)
        delete returnVal;
}

/*==============================================================================
 * FUNCTION:        HLReturn::clone
 * OVERVIEW:        Deep copy clone
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a new RTL that is a clone of this HLReturn
 *============================================================================*/
RTL* HLReturn::clone() 
{
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


#if 0
/*==============================================================================
 * FUNCTION:        HLReturn::getUseDefLocations
 * OVERVIEW:        Get the set of locations used and defined in this BB
 * NOTE:            The return location is considered to be used, even if this
 *                    use is not explicit (e.g. in Sparc might return the first
 *                    parameter)
 * PARAMETERS:      locMap - a map between locations and integer bit numbers
 *                  filter - a filter to restrict which locations are
 *                    considered
 *                  useSet - has added to it those locations used this BB
 *                  defSet - has added to it those locations defined this BB
 *                  useUndefSet - has added those locations used before defined
 *                  proc - pointer to the Proc object containing this RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void HLReturn::getUseDefLocations(LocationMap& locMap,
    LocationFilter* filter, BITSET& defSet, BITSET& useSet,
    BITSET& useUndefSet, Proc* proc) const
{
    // It is possible that any RTL, including a HLReturn, has semantics
    // So process the semantics (assignments) for this HLCall
    RTL::getUseDefLocations(locMap, filter, defSet, useSet, useUndefSet,
        proc);

    // Register a use for the return location. It may not be used anywhere
    // else; e.g. in the Sparc returnparam test, an empty procedure whose
    // integer return location is used must take a parameter
    const Exp* retl = proc->getReturnLoc();
    if (retl->len()) {
        int bit = locMap.toBit(*retl);
        useSet.set(bit);
        // Add this to the use-before-definition set if necessary
        if (!defSet.test(bit))
            useUndefSet.set(bit);
    }
}
#endif

// serialize this rtl
bool HLReturn::serialize_rest(std::ostream &ouf)
{
    HLJump::serialize_rest(ouf);

    return true;
}

// deserialize an rtl
bool HLReturn::deserialize_fid(std::istream &inf, int fid)
{
    switch (fid) {
        default:
            return HLJump::deserialize_fid(inf, fid);
    }

    return true;
}

void HLReturn::generateCode(HLLCode &hll, BasicBlock *pbb)
{
    // There could be semantics, e.g. SPARC RETURN instruction
    // Most of the time, the list of RTs will be empty, and the
    // below does nothing
    RTL::generateCode(hll, pbb);
}

void HLReturn::simplify()
{
    // TODO: return value?
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
  RTL(instNativeAddr, le), jtCond((JCOND_TYPE)0), pCond(NULL)
{
    kind = SCOND_RTL;
}

/*==============================================================================
 * FUNCTION:        HLScond::~HLScond
 * OVERVIEW:        Destructor
 * PARAMETERS:      None
 * RETURNS:         N/a
 *============================================================================*/
HLScond::~HLScond()
{
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
void HLScond::setCondType(JCOND_TYPE cond, bool usesFloat /*= false*/)
{
    jtCond = cond;
    bFloat = usesFloat;
}

/*==============================================================================
 * FUNCTION:        HLScond::makeSigned
 * OVERVIEW:        Change this from an unsigned to a signed branch
 * NOTE:            Not sure if this is ever going to be used
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::makeSigned()
{
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
Exp* HLScond::getCondExpr() 
{
    return pCond;
}

/*==============================================================================
 * FUNCTION:        HLScond::setCondExpr
 * OVERVIEW:        Set the Exp expression containing the HL condition.
 * PARAMETERS:      Pointer to semantic string to set
 * RETURNS:         <nothing>
 *============================================================================*/
void HLScond::setCondExpr(Exp* pss)
{
    if (pCond) delete pCond;
    pCond = pss;
}

/*==============================================================================
 * FUNCTION:        HLScond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         <Nothing>
 *============================================================================*/
void HLScond::print(std::ostream& os /*= cout*/, bool withDF)
{
    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " ";
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
Exp* HLScond::getDest() 
{
    assert(expList.size());
    Exp* pAsgn = expList.front();
    assert(pAsgn->isAssign());
    return ((Binary*)pAsgn)->getSubExp1();
}

/*==============================================================================
 * FUNCTION:        HLScond::getSize
 * OVERVIEW:        Get the size of the set's assignment. For now, we assume
 *                  one assignment Exp, and we take the size of that.
 * PARAMETERS:      <none>
 * RETURNS:         The size
 *============================================================================*/
int HLScond::getSize()
{
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
RTL* HLScond::clone()
{
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
bool HLScond::serialize_rest(std::ostream &ouf)
{
    return true;
}

// deserialize an rtl
bool HLScond::deserialize_fid(std::istream &inf, int fid)
{
    switch (fid) {
        default:
            return RTL::deserialize_fid(inf, fid);
    }

    return true;
}

void HLScond::generateCode(HLLCode &hll, BasicBlock *pbb)
{
    RTL::generateCode(hll, pbb);
}

void HLScond::simplify()
{
    RTL::simplify();
}
