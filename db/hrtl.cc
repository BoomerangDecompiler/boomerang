/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
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
 */

#include <iomanip>          // For setfill
#include "types.h"
#include "rtl.h"
#include "proc.h"           // For printing proc names
#include "prog.h"

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
    : RTL(instNativeAddr, listExp), pDest(0), m_isComputed(false)
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
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJump::searchAndReplace(Exp* search, Exp* replace, bool typeSens)
{
    RTL::searchAndReplace(search, replace, typeSens);
    if (pDest) {
        bool change;
        pDest->searchReplaceAll(search, replace, change, typeSens);
    }
}

/*==============================================================================
 * FUNCTION:        HLJump::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJump::searchAll(Exp* search, std::list<Exp *> &result,
                       bool typeSens)
{
    return RTL::searchAll(search, result, typeSens) ||
        ( pDest && pDest->searchAll(search, result, typeSens) );
}

/*==============================================================================
 * FUNCTION:        HLJump::print
 * OVERVIEW:        Display a text reprentation of this RTL to the given stream
 * PARAMETERS:      os: stream to write to
 * RETURNS:         Nothing
 *============================================================================*/
void HLJump::print(std::ostream& os /*= cout*/)
{
    // Returns can all have semantics (e.g. ret/restore)
    if (expList.size() != 0)
        RTL::print(os);

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
void HLJump::setIsComputed()
{
    m_isComputed = true;
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
    pCond = e;
}

/*==============================================================================
 * FUNCTION:        HLJcond::searchAndReplace
 * OVERVIEW:        Replace all instances of search with replace.
 * PARAMETERS:      search - a location to search for
 *                  replace - the expression with which to replace it
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLJcond::searchAndReplace(Exp* search, Exp* replace, bool typeSens)
{
    HLJump::searchAndReplace(search, replace, typeSens);
    bool change;
    if (pCond)
        pCond = pCond->searchReplaceAll(search, replace, change, typeSens);
}

/*==============================================================================
 * FUNCTION:        HLJCond::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLJcond::searchAll(Exp* search, std::list<Exp *> &result, bool typeSens)
{
    return RTL::searchAll(search, result, typeSens) ||
      (pCond && (pCond->searchAll(search, result, typeSens)));
}


/*==============================================================================
 * FUNCTION:        HLJcond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         Nothing
 *============================================================================*/
void HLJcond::print(std::ostream& os /*= cout*/)
{
    // These can have semantics (e.g. pa-risc add and (conditionally) branch)
    if (expList.size() != 0)
        RTL::print(os);
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
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLNwayJump::searchAndReplace(Exp* search, Exp* replace, bool typeSens)
{
    HLJump::searchAndReplace(search, replace, typeSens);
    if (pSwitchInfo && pSwitchInfo->pSwitchVar)
        pSwitchInfo->pSwitchVar->searchReplaceAll(search, replace, typeSens);
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * NOTES:           search can't easily be made const
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLNwayJump::searchAll(Exp* search, std::list<Exp *> &result, bool typeSens)
{
    return HLJump::searchAll(search, result, typeSens) ||
        ( pSwitchInfo && pSwitchInfo->pSwitchVar &&
          pSwitchInfo->pSwitchVar->searchAll(search, result, typeSens) );
}

/*==============================================================================
 * FUNCTION:        HLNwayJump::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 *                  indent: number of columns to skip
 * RETURNS:         Nothing
 *============================================================================*/
void HLNwayJump::print(std::ostream& os /*= cout*/)
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
      returnAfterCall(false)
{
    kind = CALL_RTL;
    basicBlock = NULL;
    postCallExpList = NULL;
    pDestName = NULL;
    returnLoc = 0;
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
    for (it = params.begin(); it != params.end(); it++)
        delete *it;
    if (postCallExpList) {
        for (it = postCallExpList->begin(); it != postCallExpList->end(); it++)
            delete *it;
    }
    if (returnLoc)
        delete returnLoc;
}

/*==============================================================================
 * FUNCTION:      HLCall::setBB
 * OVERVIEW:      Sets the link from this call to its enclosing BB.
 * PARAMETERS:    BB - the enclosing basic block of this call
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setBB(PBB BB)
{
    basicBlock = BB;
}

/*==============================================================================
 * FUNCTION:      HLCall::getBB
 * OVERVIEW:      Get the enclosing BB of this call.
 * PARAMETERS:    <none>
 * RETURNS:       the enclosing basic block of this call
 *============================================================================*/
PBB HLCall::getBB()
{
    return basicBlock;
}

/*==============================================================================
 * FUNCTION:      HLCall::getParams
 * OVERVIEW:      Return a copy of the locations that have been determined
 *                as the actual parameters for this call.
 * PARAMETERS:    <none>
 * RETURNS:       A reference to the list of parameters
 *============================================================================*/
std::list<Exp*>& HLCall::getParams()
{
    return params;
}

/*==============================================================================
 * FUNCTION:      HLCall::setParams
 * OVERVIEW:      Set the parameters of this call.
 * NOTE:          Copies the list... is this what we want?
 * PARAMETERS:    params - the list of locations live at this call
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setParams(std::list<Exp*>& params)
{
    this->params = params;
}

/*==============================================================================
 * FUNCTION:      HLCall::setReturnLoc
 * OVERVIEW:      Set the location that will be used to hold
 *                the value returned by this call.
 * PARAMETERS:    loc - ptr to Exp that has return location
 * RETURNS:       <nothing>
 *============================================================================*/
void HLCall::setReturnLoc(Exp* loc)
{
    returnLoc = loc;
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

    // Add the return location to the def set
    if (returnLoc.len() != 0)
        defSet.set(locMap.toBit(returnLoc));
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
 *                  typeSens - if true, the search is sensitive to type
 * RETURNS:         <nothing>
 *============================================================================*/
void HLCall::searchAndReplace(Exp* search, Exp* replace, bool typeSens)
{
    bool change;
    HLJump::searchAndReplace(search, replace, typeSens);
    if (returnLoc != 0)
        returnLoc = returnLoc->searchReplaceAll(search, replace, change,
          typeSens);
    std::list<Exp*>::iterator it;
    for (it = params.begin(); it != params.end(); it++)
        *it = (*it)->searchReplaceAll(search, replace, change, typeSens);
    // Also replace the postCall rtls, if any
    if (postCallExpList) {
        for (it = postCallExpList->begin(); it != postCallExpList->end(); it++)
            *it = (*it)->searchReplaceAll(search, replace, change, typeSens);
    }
}

/*==============================================================================
 * FUNCTION:        HLCall::searchAll
 * OVERVIEW:        Find all instances of the search expression
 * PARAMETERS:      search - a location to search for
 *                  result - a list which will have any matching exprs
 *                           appended to it
 *                  typeSens - if true, consider type when matching
 * RETURNS:         true if there were any matches
 *============================================================================*/
bool HLCall::searchAll(Exp* search, std::list<Exp *>& result, bool typeSens)
{
    bool found = false;
    if( HLJump::searchAll(search, result, typeSens) ||
      (returnLoc != 0 && returnLoc->searchAll(search, result, typeSens)))
        found = true;
    std::list<Exp*>::iterator it;
    for (it = params.begin(); it != params.end(); it++)
        if( (*it)->searchAll(search, result, typeSens) )
            found = true;
    // Also replace the postCall rtls, if any
    if (postCallExpList) {
        for (it = postCallExpList->begin(); it != postCallExpList->end(); it++)
            if( (*it)->searchAll(search, result, typeSens) )
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
void HLCall::print(std::ostream& os /*= cout*/)
{
    // Calls can all have semantics (e.g. call/restore)
    if (expList.size() != 0)
        RTL::print(os);

    os << std::hex << std::setfill('0') << std::setw(8) << nativeAddr;
    os << " ";

    // Print the return location if there is one
    if (returnLoc != 0)
        os << " " << returnLoc << " :=";
 
    os << "CALL ";
    if (pDest == NULL) {
        if (pDestName)
            os << pDestName;
        else
            os << "*no dest*";
    }
    else if (pDest->getOper() != opAddrConst) {
        // Not just an address constant. Print as a semantic string
        os << pDest << std::endl;
        return;
    }
    else {
        Proc* pProc = prog.findProc(getFixedDest());
        if ((pProc == NULL) || (pProc == (Proc*)-1))
            os << "0x"<< std::hex << getFixedDest();
        else
            os << pProc->getName();
    }

    // Print the actual parameters of the call
    os << "(";
    std::list<Exp*>::iterator it;
    for (it = params.begin(); it != params.end(); it++) {
        if (it != params.begin())
            os << ", ";
        os << *it;
    }
    os << ")" << std::endl;

    // Print the post call RTLs, if any
    if (postCallExpList) {
        for (it = postCallExpList->begin(); it != postCallExpList->end(); it++)
        {
            os << " ";
            (*it)->print(os);
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
 * FUNCTION:         HLCall::setDestName
 * OVERVIEW:         Sets the name of the destination of the call
 * NOTE:             This is not for ordinary calls; their names are found by
 *                    looking up the native address of the destination in the
 *                    BinaryFile object
 * PARAMETERS:       Pointer to the name of the destination
 * RETURNS:          <nothing>
 *============================================================================*/
void HLCall::setDestName(const char* pName)
{
    pDestName = pName;
}

/*==============================================================================
 * FUNCTION:         HLCall::getDestNane
 * OVERVIEW:         Gets the name of the destination of the call
 * NOTE:             See note above
 * PARAMETERS:       <None>
 * RETURNS:          Pointer to the name
 *============================================================================*/
const char* HLCall::getDestName() 
{
    return pDestName;
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
    ret->basicBlock = basicBlock;
    ret->params = params;
    ret->returnLoc = returnLoc;         // Copies whole Exp
    ret->numNativeBytes = numNativeBytes;
    return ret;
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
    HLJump(instNativeAddr, le)
{
    kind = RET_RTL;
}

/*==============================================================================
 * FUNCTION:         HLReturn::~HLReturn
 * OVERVIEW:         Destructor.
 * PARAMETERS:       <none>
 * RETURNS:          <nothing>
 *============================================================================*/HLReturn::~HLReturn()
{}

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
    pCond = pss;
}

/*==============================================================================
 * FUNCTION:        HLScond::print
 * OVERVIEW:        Write a text representation to the given stream
 * PARAMETERS:      os: stream
 * RETURNS:         <Nothing>
 *============================================================================*/
void HLScond::print(std::ostream& os /*= cout*/)
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
    return ((TypedExp*)first)->getType().getSize();
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


