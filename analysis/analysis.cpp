/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       analysis.cpp
 * OVERVIEW:   Perform higher level analysis on a procedure. The procedure has
 *              been decoded, and parameter analysis is complete, but a few
 *              source machine idioms may remain, especially flags. This module
 *              attempts to remove the flag uses
 *============================================================================*/

/*
 * $Revision$
 *
 * 10 Jul 02 - Mike: Mods for boomerang
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <sstream>
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "decoder.h"
#include "analysis.h"

#define DEBUG_ANALYSIS 0        // Non zero for debugging

// A global array for initialising the below
int memXinit[] = {opMemOf, -1};
// A global Exp* representing a wildcard memory expression
//Exp* memX(memXinit, memXinit+2);

#if 0
/*==============================================================================
 * FUNCTION:        checkBBflags
 * OVERVIEW:        Check this BB for assignments that may use flags.
 *                    If present, calls findDefs to match with a definition
 * NOTE:            Not an error if no defs found (happens when duplicating
 *                    neutral BBs)
 * PARAMETERS:      pBB: Pointer to the BB to be checked
 *                  proc: Ptr to UserProc object for the current procedure
 * RETURNS:         <none>
 *============================================================================*/
void Analysis::checkBBflags(PBB pBB, UserProc* proc)
{
    std::list<RTL*>* pRtls = pBB->getRTLs();
    if (pRtls == 0)
        return;

    std::list<RTL*>::iterator rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        // Locate any uses of flags. These will be in HLJconds,
        // in certain kinds of arithmetic instructions, and in SET instrs
        RTL_KIND kd = (*rit)->getKind();
//        if ((kd == JCOND_RTL) || (kd == SCOND_RTL))
//            findDefs(pBB, proc, (int)kd, rit, false);
        if (kd == JCOND_RTL) {
            HLJcond* pj = (HLJcond*)(*rit);
            // Only find a definition if doesn't already have a high level
            // expression. (This can happen with all the recursion and gnarly
            // branches in some code, especially SPARC)
            if (pj->getCondExpr() == NULL)
                findDefs(pBB, proc, JCOND_RTL, pj->isFloat(),
                rit, false);
        }
        else if (kd == SCOND_RTL) {
            HLScond* pj = (HLScond*)(*rit);
            if (pj->getCondExpr() == NULL)
                findDefs(pBB, proc, SCOND_RTL, pj->isFloat(),
                rit, false);
        }
        else if (kd == HL_NONE) {
            // Check this ordinary RTL for assignments that use flags
            int n = (*rit)->getNumExp();
            for (int i=0; i < n; i++) {
                if (!(*rit)->elementAt(i)->isAssign())
                    continue;
                Exp* pRT = (*rit)->elementAt(i);
                Exp* pRHS = pRT->getSubExp2();
                if (pRHS) {
                    int flagUsed = anyFlagsUsed(pRHS);
                    if (flagUsed)
                        // We have a use of a flag
                        findDefs(pBB, proc, flagUsed, false, rit, true);
                }
            }
        }
    }
}
#endif

/*==============================================================================
 * FUNCTION:        simplify
 * OVERVIEW:        Perform any simplifications, post decoding.
 * PARAMETERS:      pBB: pointer to the BB to be simplified
 * RETURNS:         <none>
 *============================================================================*/
void Analysis::finalSimplify(PBB pBB)
{
    std::list<RTL*>* pRtls = pBB->getRTLs();
    std::list<RTL*>::iterator rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        for (int i=0; i < (*rit)->getNumExp(); i++) {
            Exp* rt = (*rit)->elementAt(i);
            if (!rt->isAssign()) continue;
            rt->simplifyAddr();
            // Also simplify everything; in particular, stack offsets are
            // often negative, so we at least canonicalise [esp + -8] to [esp-8]
            rt->simplify();
        }
    }
}

/*==============================================================================
 * FUNCTION:        analyse
 * OVERVIEW:        Perform any higher level analysis on a procedure. At
 *                  present, this is the removal of uses of flags (checkBBflags)
 *                  and a pass looking for constants in the text section
 *                  (checkBBconst)
 * PARAMETERS:      proc: pointer to a Proc object for the procedure being
 *                  analysed
 * RETURNS:         <none>
 *============================================================================*/
void Analysis::analyse(UserProc* proc) {
    Cfg* cfg = proc->getCFG();
    std::list<PBB>::iterator it;
    PBB pBB = cfg->getFirstBB(it);
    while (pBB)
    {
        // Match flags
        //checkBBflags(pBB, proc);
        // Perform final simplifications
        finalSimplify(pBB);
#if USE_PROCESS_CONST       // Fatally flawed. Somehow, conversion of
                            // strings and function pointers has disappeared!
        // Check for constants
        checkBBconst(proc->getProg(), pBB);
#endif // USE_PROCESS_CONST

        // analyse calls
        analyseCalls(pBB, proc);

        pBB = cfg->getNextBB(it);
    }

    cfg->simplify();
}

#if DEBUG_ANALYSIS
static int findDefsCallDepth = 0;
#endif

#if 0
/*==============================================================================
 * FUNCTION:        findDefs
 * OVERVIEW:        Finds a definition for this use (jcond, scond, or assign)
 *                  of the flags, and when found, calls a given function.
 * PARAMETERS:      pBB: pointer to current BB (with the use of the flag)
 *                  pRtls: pointer to list of RTL pointers for current BB
 *                  cfg: pointer to Cfg for current Proc
 *                  proc: pointer to the current Proc object
 *                  kd: Either the HL_KIND of the RTL, (either JCOND_RTL or
 *                  SCOND_RTL), or the id of the flag used (if an assignment)
 *                  flt: true if this involves the floating point CCs
 *                  rit: iterator to the instruction using the CCs
 *                  f: pointer to function to actually match def and use
 * RETURNS:         <nothing>
 *===========================================================================*/
void Analysis::findDefs(PBB pBB, UserProc* proc, int flagId, bool flt,
    std::list<RTL*>::iterator rit, bool withAssign)
{
    std::list<RTL*>* pRtls = pBB->getRTLs();
    Cfg *cfg = proc->getCFG();
    // We may follow a chain of BBs if there is only one in-edge, so keep
    // pointers to the original BB and list of RTLs
    PBB   pOrigBB   = pBB;
    std::list<RTL*>* pOrigRtls = pRtls;
    bool defFound = false;
    
#if DEBUG_ANALYSIS
    findDefsCallDepth++;
    std::cout << "\nfindDefs(" << findDefsCallDepth
      << "): Finding def for CC used at " << std::hex << (*rit)->getAddress()
      << " in " << proc->getName() << " BB 0x" << (unsigned int)pBB << endl;
#endif

    std::list<RTL*>::reverse_iterator rrit = pRtls->rbegin();
    // rit might be in the middle of the BB, so keep incrementing until we are
    // at rit
    while (rrit != pRtls->rend()) {
        if (*rrit == *rit) break;
        rrit++;
    }
    assert(rrit != pRtls->rend());      // Should find rit in this BB

    while (!defFound) {
        while (rrit != pRtls->rend()) {
            int n = (*rrit)->getNumExp();
            if (n > 0) {
                // Get the last RT
                Exp* pRT = (*rrit)->elementAt(n-1);
                // Ensure that the "floatnesses" agree; don't be fooled
                // by interleaved integer and float defines and uses`
                if ((pRT->isFlagCall()) &&
                  (std::string(((Const*)(pRT->getSubExp1()))->getStr())
                    .find("FLAG") != std::string::npos)
                  && (isFlagFloat(pRT, proc) == flt)) {
                    defFound = true;          // Found RTL that sets cc
                    break;
                }
                // Iterate through the RTs looking for definitions of flags
                // (e.g. X86 bit instructions). Assume assignments to flags
                // are only for integer instructions
                if (!flt) {
                    for (int i=n-1; i >=0; i--) {
                        Exp* pRT = (*rrit)->elementAt(i);
                        if (pRT->isAssign()) {
                            Exp* pLHS = pRT->getSubExp1();
                            if (anyFlagsUsed(pLHS)) {
                                defFound = true;
                                break;
                            }
                        }
                    }
                }
                if (defFound) break;
            }
            rrit++;
        }
        if (defFound) break;
        
        // If only one in-edge, then continue searching that BB
        std::vector<PBB>& inEdges = pBB->getInEdges();
        if (inEdges.size() != 1) break;
        pBB = inEdges[0];
        if ((pBB->getType() == CALL) || (pBB->getType() == COMPCALL)) {
            std::ostringstream ost;
#if DEBUG_ANALYSIS
            ost << "jcond at " << std::hex << pOrigBB->getHiAddr();
            ost << " has a CALL BB leading to it (call to ";
            std::list<RTL*>* prtls = pBB->getRTLs();
            HLCall* call = (HLCall*)prtls->back();
            assert(call->getKind() == CALL_RTL);
            ADDRESS dest = call->getFixedDest();
            if (dest != NO_ADDRESS)
                ost << "0x" << std::hex << ")" << dest;
            else
                ost << std::hex << "computed dest)";
            std::cerr << ost.str() << std::endl;
#endif
            // Just have to give up on this use of flags
#if DEBUG_ANALYSIS
            findDefsCallDepth--;
#endif
        return;
        }
        pRtls = pBB->getRTLs(); // repeat the search using the parent BB
        rrit = pRtls->rbegin();
    }

    if (defFound) {
        // Call the action function. This is either matchJScond (for
        // HLJconds or HLSconds) or matchAssign (for assignments that use
        // individual flags)
        if (withAssign)
            matchAssign(pOrigRtls, pOrigBB, rrit, proc, rit, flagId);
        else
            matchJScond(pOrigRtls, pOrigBB, rrit, proc, rit, (*rit)->getKind());
#if DEBUG_ANALYSIS
    findDefsCallDepth--;
#endif
        return;
    }

#if DEBUG_ANALYSIS
    std::cout << "findDefs(" << findDefsCallDepth
      << "): Multiple in-edges to BB 0x" << std::hex << (unsigned int)pBB
      << " at " << std::hex << pBB->getHiAddr() << ": ";
    for (unsigned z=0; z < pBB->getInEdges().size(); z++)
        std::cout << pBB->getInEdges()[z]->getLowAddr() << " ";
    std::cout << endl;
#endif
    // Replicate the BB using the flags in series with the in-edges.
    // Note: although CALLs and COMPCALLs look like fall-through BBs, they
    // are very likely to change the flags, and so (for now) are flagged as
    // errors. One-way jump in-edges can be handled by inserting the BB
    // using the flags before the final jump of that BB. Example:
    //
    // cmp1     cmp2    cmp3
    // jmp x    be x    fall thru to x
    //          f:
    //          ...
    //
    // x: a:=b      ! Does not affect flags
    // jcond y      ! Use of flags
    // z:
    //
    // becomes:
    //
    // cmp1         cmp2        cmp3 // Still falls through to x
    // a:=b         be new      x: a:=b
    // jcond y      f:          jcond y
    // jmp z        ...         z:
    //              jmp z
    //              new: a:=b
    //              jcond y
    //              jmp z
    //
    // Then the standard algorithm can be used to fix both flags uses, with
    // the (possibly) different compare expressions. Check each in edge of
    // the current BB (at the top of the chain of BBs with only 1 in-edge)
    // Note well: this use of vit is INVALID! When the in-edges are reallocated,
    // vit becomes invalid
    std::vector<PBB>::iterator vit = pBB->getInEdges().begin();
    if (vit == pBB->getInEdges().end()) {
#if DEBUG_ANALYSIS
        std::ostringstream os;
        os << "flag use at " << std::hex << pOrigBB->getHiAddr();
        os << " has no RTL defining the flags; current BB: ";
        pBB->print(os);
        std::cerr << os.str() << std::endl;
        // Continue with next BB this Cfg
    findDefsCallDepth--;
#endif
        return;
    }

    while (vit != pBB->getInEdges().end()) {
        if (pBB->getInEdges().size() == 1) {
        // There is only one in-edge left, so we are done
        break;
    }
    // The parent may define the flags, or one of its parents may do it
    PBB pParBB = *vit;
    if ((pParBB->getType() == CALL) || (pParBB->getType() == COMPCALL)) {
        // No error message here; we will issue one when we try to match
        // this use and def.
        vit++;              // Ignore this in-edge
        std::cout << "[!]";
        continue;
    }
    std::list<RTL*>* pCurRtls = pBB->getRTLs();
    // It is possible to have more than one "fall through" BB parent
    // when you start duplicating BBs (and some of those were fall
    // throughs). So fall through BBs are treated exactly like one-way
    // BBs now
    if ((pParBB->getType() == ONEWAY) || (pParBB->getType() == FALL)) {
        // As per the cmp1 case above, we need to replicate the BB using
        // the flag here, keeping the in and out edges correct.
        // Remove the branch RTL altogether (if present), as it points to
        // the wrong place, and will be superceeded by the added RTLs
        std::list<RTL*>* pParRtls = pParBB->getRTLs();
        // Check to see if it's a branch. Not all one-way BBs will have
        // a branch as the last RTL!
        std::list<RTL*>::iterator lastIt = --pParRtls->end();
        if ((*lastIt)->getKind() == JUMP_RTL) {
            pParRtls->erase(lastIt, pParRtls->end());
        }
        // Copy the out edges of the current BB to the parent BB
        std::vector<PBB>& destOuts = pBB->getOutEdges();
        for (unsigned i=0; i < destOuts.size(); i++) {
            PBB destBB = pBB->getOutEdges()[i];
            if (i == 0) {
                pParBB->setOutEdge(0, destBB);
            } else {
                // This function will make sure that a jump is generated,
                // since this is a new out-edge
                cfg->addNewOutEdge(pParBB, destBB);
            }
            destBB->addInEdge(pParBB);
        }
        // Now it's the same type of BB, with the same number of out edges,
        // as the current BB
        pParBB->updateType(pBB->getType(), destOuts.size());
        // Now insert a copy of the current BB at the end
        // Note: a deep copy is required!
        std::list<RTL*>::iterator it;
        for (it = pCurRtls->begin(); it != pCurRtls->end(); it++) {
            pParBB->getRTLs()->push_back((*it)->clone());
        }
        // Note that the original BB stays where it is, and code is
        // generated for it (we perform this loop only until there is
        // one in-edge left). If the BB that we copy is a fall through
        // or two-way type (with a fall through edge), we must force
        // a branch at the end of the original BB, because the new BB
        // may have the same "address" as the original BB, and so
        // it can be emitted after the original BB, thus causing the
        // fallthrough address to be wrong. Subtle!
        // Example: getarmyrn_pot() inlined into fixgralive() in 099.go
        // Spec benchmark (note: depends on how ties are handled in the
        // sort as to whether the problem actually shows up)
        if ((pBB->getType() == FALL) || (pBB->getType() == TWOWAY)) {
            pBB->setJumpReqd();
            if (pBB->getType() == FALL)
                cfg->setLabel(pBB->getOutEdges()[0]);
            else
                // Two way; first out edge is "true" edge, second is fall
                cfg->setLabel(pBB->getOutEdges()[1]);
        }

#if DEBUG_ANALYSIS
        std::cout << "\nfindDefs(" << findDefsCallDepth
          << "): copied 1W BB: " << std::hex;
        for (it = pParBB->getRTLs()->begin(); it != pParBB->getRTLs()->end();
          it++) {
            std::cout << *it << " ";
        }
        std::cout << std::endl;
        pParBB->print();
        std::cout << "Current BB: ";
        for (it = pCurRtls->begin(); it != pCurRtls->end(); it++) {
            std::cout << *it << " ";
        }
        std::cout << std::endl << std::flush;
        pBB->print();
#endif

            // Remove this in-edge
            pBB->deleteInEdge(vit);         // Also increments vit
            // Now analyse this new BB. It's now a different BB, and the def
            // and use will be in the same BB
//          findDefs(pParBB, pParBB->getRTLs(), cfg, proc, kd,
//              --pParBB->getRTLs()->end(), f);
            // The RTL using the flags is not necessarily the last RTL (e.g.
            // it could be an addx 0,%g0,%i0). So call checkBBflags to find
            // it and call findDefs with all the right arguments
            checkBBflags(pBB, proc);
            // Also check the BB that you have just copied, since it may also
            // have a use of the flags
            checkBBflags(pParBB, proc);
            continue;
        } else if (pParBB->getType() == TWOWAY) {
            // As per cmp2 in the example above, we need to change the
            // branch to point to a new BB, which may fall through to the
            // successor of the original BB
            // First create the new BB
            std::list<RTL*>* pNewRtls = new std::list<RTL*>;
            // Must do a deep copy, else will have problems deleting, and
            // also, setting the addresses of one RTLL to zero will affect
            // the other
            std::list<RTL*>::iterator src;
            for (src=pCurRtls->begin(); src!=pCurRtls->end(); src++) {
                pNewRtls->push_back((*src)->clone());
            }
            // Kill the addresses of the first two RTLs. Otherwise, the CFG
            // object will think that this BB is already created
            std::list<RTL*>::iterator it = pNewRtls->begin();
            (*it++)->updateAddress(0);
            if (it != pNewRtls->end()) {
                (*it)->updateAddress(0);
            }

#if DEBUG_ANALYSIS
            std::cout << "\nfindDefs(" << findDefsCallDepth << "): copied 2W BB: "
              << std::hex;
            for (it=pNewRtls->begin(); it!=pNewRtls->end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;
            pParBB->print();
            cout << "Current BB: ";
            for (it = pCurRtls->begin(); it!=pCurRtls->end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << std::endl << std::flush;
            pBB->print();
#endif

            std::vector<PBB>& destOuts = pBB->getOutEdges();
            // Create the new BB. It will have the same type and number of out
            // edges that the current BB has
            PBB pNewBB = cfg->newBB(pNewRtls, pBB->getType(), destOuts.size());
            // Copy the out edges of the current BB to the new BB
            for (unsigned i=0; i < destOuts.size(); i++) {
                cfg->addOutEdge(pNewBB, pBB->getOutEdges()[i]);
            }
            // Make sure this last one will be implemented as a branch
            pNewBB->setJumpReqd();
            cfg->setLabel(pNewBB);      // MVE: Is this correct?
            // Also add a label at the fallthrough address, if any
            if (destOuts.size() == 2) {
                cfg->setLabel(pBB->getOutEdges()[1]);
            }
            // Now change the parent BB to point to the new BB
            // Note: it could be the true (0) or false (1) edge that leads to
            // the current BB
            if (pParBB->getOutEdges()[0] == pBB) {
                pParBB->setOutEdge(0, pNewBB);
            } else {
                // Must be the false edge
                pParBB->setOutEdge(1, pNewBB);
            }
            // Add an in-edge to the new BB
            pNewBB->addInEdge(pParBB);
            // Remove this in-edge from the current BB
            pBB->deleteInEdge(vit);     // Increments vit as well
            // Now analyse this new BB. Again, the use could be anywhere in
            // the BB, or there may not be a use at all if this is a neutral BB
            checkBBflags(pBB, proc);
//          findDefs(pNewBB, pNewRtls, cfg, proc, kd,
//              --pNewBB->getRTLs()->end(), f);
            // Also check the new BB, since it also may have a use of the flags
            checkBBflags(pNewBB, proc);
            continue;
        } else {
            std::ostringstream os;
            os << "findDefs: unexpected parent BB type "
               << pParBB->getType() << " found:\n";
            pParBB->print(os);
            std::cerr << os.str() << std::endl;
            assert(0);
        }
        vit++;
    }
    // Recurse to handle the original jcond, which by now has only one in-edge
    findDefs(pOrigBB, proc, flagId, flt, rit, withAssign);
#if DEBUG_ANALYSIS
    findDefsCallDepth--;
#endif
}
#endif

/*==============================================================================
 * FUNCTION:        matchJScond
 * OVERVIEW:        Process a (define CC) and (HLJcond or HLScond) pair
 *                  This procedure basically makes the decision of whether to
 *                  treat the pair as add-like or sub-like
 * TERMS:           Add-like: r = a+b => if (r op 0) then ...
 *                  Sub-like: r = a-b => if (a op b) then ...
 *                    Makes no sense to combine unsigned uses of the CCs (e.g.
 *                    branch if higher or same) with add-like setting of the
 *                    condition codes
 * ASSUMPTIONS:     Assumes that sutract-like flag call functions will have
 *                    names starting with "SUB", or containing the string
 *                    "FFLAG"
 * PARAMETERS:      pOrigRtls: pointer to list of RTLs for BB containing the
 *                    instr using the CCs
 *                  rrit: reverse iterator to the instr defining the CCs
 *                  proc: pointer to the current Proc object
 *                  rit: iterator to instr using the CCs
 *                  kd: kind of the RTL using the CCs, either JCOND_RTL or
 *                    SCOND_RTL (as an integer)
 * RETURNS:         <nothing>
 *============================================================================*/
void Analysis::matchJScond(std::list<RTL*>* pOrigRtls, PBB pOrigBB, std::list<RTL*>::reverse_iterator rrit, UserProc* proc,
    std::list<RTL*>::iterator rit, int kd)
{
#if DEBUG_ANALYSIS
    std::cout << "CC used by j/scond near " << std::hex << pOrigBB->getHiAddr() <<
    " defined at " << (*rrit)->getAddress() << endl;
#endif

    // Get the last RT of the defining instr
    Exp* pRT = (*rrit)->elementAt((*rrit)->getNumExp()-1);
    if (pRT->isAssign()) {
        // In this case, a flag has been defined by an assignment.
        // The conditional expression is just the LHS of this assignment
        // Example: bit 5,eax // jc dest =>
        // CF = ((eax >> 5) & 1); if (CF) goto dest;
        HLJcond* jc = (HLJcond*)*rit;
        HLScond* sc = (HLScond*)*rit;
        // Make a copy of the LHS
        Exp* cond = pRT->getSubExp1()->clone();
        if (kd == JCOND_RTL)
            jc->setCondExpr(cond);
        else
            sc->setCondExpr(cond);
        return;
    }

    // Otherwise, pRT is an RTFlagCall
    std::string fname(((Const*)pRT->getSubExp1())->getStr());
    if ((fname.substr(0, 3) == "SUB") ||
        // Floating point compares are always "subtract like"
        (fname.find("FFLAG") != std::string::npos))
            processSubFlags(*rit, rrit, proc, (RTL_KIND)kd);
    else
        // Else name starts with ADD, or we have various possibilities like
        // MULT and LOGIC, which all act in an add-like form
        processAddFlags(*rit, rrit, proc, (RTL_KIND)kd);
}

/*==============================================================================
 * FUNCTION:        anyFlagsUsed
 * OVERVIEW:        Return nonzero if any of the "big 4" flags are used in this
 *                  semantic string
 * PARAMETERS:      s: pointer to semantic string being checked
 * RETURNS:         If no flags used, returns opWild. Otherwise, returns the op
 *                  (e.g. opCF) of the first flag found
 *============================================================================*/
OPER Analysis::anyFlagsUsed(Exp* s)
{
    Exp* res;
    Exp* srch = new Terminal(opCF);
    if (s->search(srch, res)) {
#if DEBUG_ANALYSIS
    std::cout << "Use of carry flag: res is "; res.print(); std::cout << "\n";
#endif
        return opCF;
    }
    delete srch;
    srch = new Terminal(opZF);
    if (s->search(srch, res)) {
#if DEBUG_ANALYSIS
    std::cout << "Use of zero flag: res is "; res.print(); std::cout << "\n";
#endif
        return opZF;
    }
    delete srch;
    srch = new Terminal(opNF);
    if (s->search(srch, res)) return opNF;
    delete srch;
    srch = new Terminal(opOF);
    if (s->search(srch, res)) return opOF;
    return (OPER)0;
}


/*==============================================================================
 * FUNCTION:        processSubFlags
 * OVERVIEW:        Matches a j/scond with an RT defining the flags with a
 *                  SUBFLAGS call.
 * ASSUMPTIONS:     Assumes that the second last RT defines the result of the
 *                    subtract-like operation (for cases HLJCOND_JMI and JPOS),
 *                    or (all other cases) that the last RT is a flag call, and
 *                    the first two operands of this function are the operands
 *                    being subtracted. (This includes SETFFLAGS as well as
 *                    SUBFLAGS)
 * PARAMETERS:      rtl: pointer to the RTL with the instr using the CCs
 *                  rrit: iterator to the CC defining RTL
 *                  proc: pointer to the current Proc object
 *                  kd: kind of the RTL using the CCs, either JCOND_RTL or
 *                    SCOND_RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void Analysis::processSubFlags(RTL* rtl, std::list<RTL*>::reverse_iterator rrit,
    UserProc* proc, RTL_KIND kd)
{
    HLJcond* jc = (HLJcond*)rtl;
    HLScond* sc = (HLScond*)rtl;
    JCOND_TYPE jt;
    if (kd == JCOND_RTL)
        jt = jc->getCond();
    else
        jt = sc->getCond();
    Exp *pHL = NULL;
    OPER op = opWild;
    switch (jt) {
        case HLJCOND_JMI:
        case HLJCOND_JPOS: {
            // These branches are special in that they always refer to the
            // result of the subtract, rather than a comparison of the
            // operands.
            // The second last RT should always be the one that defines the
            // result
            // Result is (lhs < 0) or (lhs >= 0) etc
            switch (jt) {
                case HLJCOND_JMI:   op = opLess; break;
                case HLJCOND_JPOS:  op = opGtrEq; break;
                default: /* Can't reach here */ break;
            }
            Exp* pRtDef = (*rrit)->elementAt((*rrit)->getNumExp()-2);

            pHL = new Binary(op, pRtDef->getSubExp2()->clone()->simplify(), new Const(0));
            break;
        }
        default: {
            // Otherwise, subtracts (even decrements) look like compares,
            // as far as the HL result is concerned. For example, after
            // r[8] = r[8] - 1 / jle dest, we want if (r[8] <= 1) goto dest
            // except that we want the original values of the operands. So
            // we use the parameters sent to the SUBFLAGS. They can readily
            // get changed between the def and the use, so we keep a copy of
            // the operands in two new vars
            switch (jt) {
                case HLJCOND_JE:    op = opEquals;  break;
                case HLJCOND_JNE:   op = opNotEqual;break;
                case HLJCOND_JSL:   op = opLess;    break;
                case HLJCOND_JSLE:  op = opLessEq;  break;
                case HLJCOND_JSGE:  op = opGtrEq;   break;
                case HLJCOND_JSG:   op = opGtr;     break;
                case HLJCOND_JUL:   op = opLessUns; break;
                case HLJCOND_JULE:  op = opLessEqUns;break;
                case HLJCOND_JUGE:  op = opGtrEqUns;break;
                case HLJCOND_JUG:   op = opGtrUns;  break;
                default: assert(0);
            }
            Exp* pRtf = (*rrit)->elementAt((*rrit)->getNumExp()-1);
            Exp* li = pRtf->getSubExp2();
            assert(li->getOper() == opList);
            Exp* fst = li->getSubExp1();
            li = li->getSubExp2();
            assert(li->getOper() == opList);
            Exp* sec = li->getSubExp1();

            assert(fst); assert(sec);
            pHL = new Binary(op, fst->clone()->simplify(),
              sec->clone()->simplify());
            break;
        }
    }   // end switch
    assert(pHL);
    if (kd == JCOND_RTL)
        jc->setCondExpr(pHL);
    else
        sc->setCondExpr(pHL);
}

/*==============================================================================
 * FUNCTION:        processAddFlags
 * OVERVIEW:        Matches a j/scond with an RT defining the flags with an
 *                  ADDFLAGS call.
 * ASSUMPTIONS:     Assumes that the second last RT is an assignment, and the
 *                    left hand side of this assignment is the result of the add
 * PARAMETERS:      jc: pointer to the HLJcond with the branch
 *                  rrit: iterator to the CC defining RTL
 *                  proc: pointer to the current Proc object
 *                  kd: kind of the RTL using the CCs, either JCOND_RTL or
 *                    SCOND_RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void Analysis::processAddFlags(RTL* rtl, std::list<RTL*>::reverse_iterator rrit,
    UserProc* proc, RTL_KIND kd)
{
    HLJcond* jc = (HLJcond*)rtl;
    HLScond* sc = (HLScond*)rtl;
    JCOND_TYPE jt;
    if (kd == JCOND_RTL)
        jt = jc->getCond();
    else
        jt = sc->getCond();
    Exp *pHL = NULL;
    // Result is (lhs op 0)
    AssignExp* pRtDef = (AssignExp*)((*rrit)->elementAt((*rrit)->getNumExp()-2));
    assert(pRtDef->getOper() == opAssignExp);
    OPER op = opWild;
    switch (jt) {
        case HLJCOND_JE:    op = opEquals; break;
        case HLJCOND_JNE:   op = opNotEqual; break;
        case HLJCOND_JSL:   // Can be valid after a test instruction
        case HLJCOND_JMI:   op = opLess; break;
        case HLJCOND_JSGE:  // Can be valid after a test instruction
        case HLJCOND_JPOS:  op = opGtrEq; break;
        // Next two can be valid after a test instr
        case HLJCOND_JSLE:  op = opLessEq; break;
        case HLJCOND_JSG:   op = opGtr; break;
        default:
            std::ostringstream ost;
            ost << "ADD flags coupled with jcond kind " << (int)jt;
            std::cerr << ost.str() << std::endl;
            return;
    }
    Exp* pRight = pRtDef->getSubExp2()->clone()->simplify();
    pHL = new Binary(op, pRight, new Const(0));
    assert(pHL);
    if (kd == JCOND_RTL)
        jc->setCondExpr(pHL);
    else
        sc->setCondExpr(pHL);
}

/*==============================================================================
 * FUNCTION:        analyse_assign
 * OVERVIEW:        Process a (define CC) and (assign that uses CC) pair
 *                  For example, cmp 0,a / d = 0-0-CF is handled by appending
 *                  an RT to the compare: CF = (a != 0);
 * PARAMETERS:      pOrigRtls: pointer to list of RTLs for BB containing the
 *                    instr using the CCs
 *                  rrit: reverse iterator to the instr defining the CCs
 *                  proc: pointer to the current Proc object
 *                  rit: iterator to instr using the CCs
 *                  kd: kind of the RTL using the CCs, either JCOND_RTL or
 *                    SCOND_RTL
 * RETURNS:         <nothing>
 *============================================================================*/
void Analysis::matchAssign(std::list<RTL*>* pOrigRtls, PBB pOrigBB,
  std::list<RTL*>::reverse_iterator rrit, UserProc* proc,
  std::list<RTL*>::iterator rit, int flag)
{
#if DEBUG_ANALYSIS
    std::cout << "Use of CC in this assignment: ";
    (*rit)->print();
    std::cout << endl;
    std::cout << "CC set at "; (*rrit)->print();
    std::cout <<  endl;
#endif
    OPER flagUsed = (OPER) flag;
    if (flagUsed == opCF) {
        int n = (*rrit)->getNumExp();
        Exp* pRT = (*rrit)->elementAt(n-1);
        char* fname;
        if (pRT->isFlagCall() &&
          (fname = ((Const*)pRT->getSubExp1())->getStr(),
          (std::string(fname).substr(0, 3) == "SUB"))) {
            // Get the assignment/comparison. Should be the last assignment,
            // which should be the second last RT (excluding added RTs).
            // Note: have to assume that it has an opMinus or opNeg in the RHS
            // (to exclude the added RTs, which are probably moves to vars)
            Exp* pRT; Exp* pRHS;
            OPER op;
            int i=n-2;
            do {
                if (i < 0) break;
                pRT = (*rrit)->elementAt(i);
                i--;
                if (!pRT->isAssign()) continue;
                pRHS = pRT->getSubExp2();
                op = pRHS->getOper();
            } while ((op != opMinus) && (op != opNeg));
            assert(pRT->isAssign());
            pRHS = pRT->getSubExp2();
            // Idiom 1: Compare 0 to X (or subtract X from 0), then use the
            // carry flag.
            // Transform to: CF = (X != 0);
            // Note: this will look like r[0] = -r[11] after simplification
            if (pRHS->getOper() == opNeg) {
                assert(pRT->isAssign());
                int size = ((AssignExp*)pRT)->getSize();
                Exp* pLHS = new Terminal(opCF);
                Exp* notEq0 = new Binary(opNotEqual,
                    pRHS->getSubExp1()->clone(),
                    new Const(0));
                // Note, we must prepend the assignment to the CF, because it
                // depends only on the value of the operands before the
                // subtract, and an operand could be modified by the subtract
                // Hence we pass "true" as parameter 3
//              (*rrit)->insertAssign(pLHS, notEq0, true, size);
                (*rrit)->insertAfterTemps(pLHS, notEq0, size);
#if DEBUG_ANALYSIS
    std::cout << "Result: "; (*rrit)->print(); std::cout << endl;
#endif
            }
            else if (pRHS->getOper() == opIntConst) {
                // Very rare case, e.g. fixgralive in 099.go:
                // 30d88:  90 a0 20 03        subcc        %g0, 3, %o0
                // 30d8c:  84 62 60 00        subx         %o1, 0, %g2
                // First rtl is v0 := -3
                // Add Cf := (3 != 0)!!
                assert(pRT->isAssign());
                int size = ((AssignExp*)pRT)->getSize();
                Exp* pLHS = new Terminal(opCF);
                Exp* notEq0 = new Binary(opNotEqual,
                    new Const(-((Const*)pRHS)->getInt()),
                    new Const(0));
                // As per above, we must prepend the assignment to CF
//              (*rrit)->insertAssign(pLHS, notEq0, true, size);
                (*rrit)->insertAfterTemps(pLHS, notEq0, size);
#if DEBUG_ANALYSIS
    std::cout << "Result: "; (*rrit)->print(); std::cout << endl;
#endif
            }
            else {
                // Idiom 2: Compare X to Y (or subtract Y from X), then use the
                // carry flag.
                // Transform to: CF = ((unsigned)X < (unsigned)Y);
                assert(pRHS->getOper() == opMinus);
                assert(pRT->isAssign());
                int size = ((AssignExp*)pRT)->getSize();
                Exp* pLHS = new Terminal(opCF);
                Exp* xLTy = new Binary(opLessUns,
                    pRHS->getSubExp1()->clone(),
                    pRHS->getSubExp2()->clone());
                // As per above, we must prepend the assignment to CF
//              (*rrit)->insertAssign(pLHS, xLTy, true, size);
                (*rrit)->insertAfterTemps(pLHS, xLTy, size);
#if DEBUG_ANALYSIS
    std::cout << "Result: "; (*rrit)->print(); std::cout << std::endl;
#endif
            }
            return;
        }
        else if ((pRT->isFlagCall()) &&
          (fname = ((Const*)pRT->getSubExp1())->getStr(),
           std::string(fname).substr(0, 3) == "ADD")) {
            std::cerr << "CC definition at " << std::hex <<
              (*rrit)->getAddress() << " is an ADD class instruction. "
              "64 bit arithmetic not supported yet" << std::endl;
        }
    }
    std::cerr << "CC use at " << std::hex << (*rit)->getAddress() <<
      " not paired" << std::endl;
}

/*==============================================================================
 * FUNCTION:        copySwap4
 * OVERVIEW:        Load a 4 byte word from little to big endianness,
 *                  or vice versa
 *                  Similar to _swap4 macro, but this for translate time
 * NOTE:            Processes char at a time, so avoids alignment hassles
 * PARAMETERS:      w: Pointer to source word
 * RETURNS:         The source word as an integer
 *============================================================================*/
int Analysis::copySwap4(int* w)
{
    char* p = (char*)w;
    int ret;
    char* q = (char*)(&ret+1);
    *--q = *p++; *--q = *p++; *--q = *p++; *--q = *p;
    return ret;
}

/*==============================================================================
 * FUNCTION:        copySwap2
 * OVERVIEW:        Load a 2 byte short word from little to big endianness,
 *                  or vice versa
 *                  Similar to _swap2 macro, but this for translate time
 * NOTE:            Processes char at a time, so avoids alignment hassles
 * PARAMETERS:      w: Pointer to source short word
 * RETURNS:         The source short word as an integer
 *============================================================================*/
int Analysis::copySwap2(short* h)
{
    char* p = (char*)h;
    short ret;
    char* q = (char*)(&ret);
    *++q = *p++; *--q = *p;
    return (int)ret;
}

#if USE_PROCESS_CONST
/*==============================================================================
 * FUNCTION:      processConst
 * OVERVIEW:      Replace the RHS of the current assignment with a floating
 *                  point constant (or integer constant in rare cases)
 * NOTE:          Assumes that the floating point constant binary format
 *                  is compatible with the compiler used to compile this code
 * PARAMETERS:    addr: native address of the constant
 *                memExp: reference to a Exp* which is the m[] to be replaced
 *                  with the constant
 *                RHS: pointer to the RHS to be changed
 * RETURNS:       <nothing>; note that the RHS Exp* gets changed
 *============================================================================*/
void Analysis::processConst(Prog *prog, ADDRESS addr, Exp*& memExp, Exp* RHS)
{
    const void* p;
    const char* last;
    int delta;
    union {
        double d;
        int i[2];
    } value;
    p = prog->getCodeInfo(addr, last, delta);
    if (p == NULL) return;      // No code section pointer there
    int size = memExp.getType().getSize();

    // Exp*'s are deemed to be same endianness as the source machine:
    // i.e., the constant is in src endianness.
    // Let "uqbt endianness" = endianness of machine UQBT is running on now.
    // We must swap the constant during translation if 
    //     uqbt endianness != src endianness
    // since then we must swap any constant read during translation.
    bool doSwaps;
    #if ((WORDS_BIGENDIAN == 1) && (SRCENDIAN == LITTLEE)) || ((WORDS_BIGENDIAN == 0) && (SRCENDIAN == BIGE))
        doSwaps = true;
    #else
        doSwaps = false;
    #endif
        
    Exp* con = 0;
    if (memExp.getType().getType() == FLOATP) {
        if (doSwaps) {
            if (size == sizeof(float)*8) {
                int i = copySwap4((int*)p);
                value.d = (double)*(float*)&i;
            } else if (size == sizeof(double)*8) {
                int ii[2];
                ii[1] = copySwap4((int*)p);
                ii[0] = copySwap4(((int*)p)+1);
                value.d = *(double*)ii;
            }
        } else {
            if (size == sizeof(float)*8)
                value.d = (double)*(float*)p;
            else if (size == sizeof(double)*8)
                value.d = *(double*)p;
        }
        con = new Const(value.d);
    } else {
        // The constant is integer.
        int k;
        if (doSwaps) {
            if (size == sizeof(int)*8) {
                k = copySwap4((int*)p);
            } else if (size == sizeof(short)*8) {
                k = copySwap2((short*)p);
            } else
                assert(0);
        } else {
            if (size == sizeof(int)*8)
                // Note: there could be alignment issues!
                k = *(int*)p;
            else if (size == sizeof(short)*8)
                k = *(short*)p;
            else
                assert(0);
        }
        con = new Const(k);
    }

    // Replace the right hand side with the constant
    // MVE: Does this have to be type sensitive sometimes? Doubtful.
    RHS->searchReplaceAll(memExp, *con);
    delete con;
}

/*==============================================================================
 * FUNCTION:        isRegister
 * OVERVIEW:        Return true if the given expression is a register, var, or
 *                      a temp. Only constant register numbers are considered
 * PARAMETERS:      exp: pointer to the expression to be checked
 * RETURNS:         True if matched
 *============================================================================*/
bool Analysis::isRegister(const Exp* exp)
{
    if (exp->getFirstIdx() == idRegOf)
        return exp->getSecondIdx() == idIntConst;
    if (exp->getFirstIdx() == idVar)
        return true;
    if (exp->getFirstIdx() == idTemp)
        return true;
    return false;
}

/*==============================================================================
 * FUNCTION:        isConstTerm
 * OVERVIEW:        Return true if the given expression is a constant or a
 *                      register that is already known to be const
 * PARAMETERS:      exp: pointer to the expression to be checked
 *                  constMap: reference to the map of existing constant regs
 *                  value: reference to an ADDRESS that will be set to the
 *                    value of the constant
 * RETURNS:         True if matched, and value is set
 *============================================================================*/
bool Analysis::isConstTerm(const Exp* exp, const KMAP& constMap, ADDRESS& value)
{
    if (exp->getFirstIdx() == idIntConst) {
        value = exp->getSecondIdx();
        return true;
    }
    if (isRegister(exp)) {
        KMAP::const_iterator it;
        it = constMap.find(*exp);
        if (it == constMap.end())
            // Not a constant register
            return false;
        // A constant register
        value = it->second;
        return true;
    }
    // Not a simple constant or a register
    return false;
}

/*==============================================================================
 * FUNCTION:        isConst
 * OVERVIEW:        Return true if the given expression is constant
 * PARAMETERS:      exp: pointer to the expression to be checked
 *                    constMap: reference to the map of existing constant regs
 *                    value: reference to an ADDRESS that will be set to the value
 *                      of the constant
 * RETURNS:         True if matched, and value is set
 *============================================================================*/
bool Analysis::isConst(const Exp* exp, const KMAP& constMap, ADDRESS& value)
{
    if (isConstTerm(exp, constMap, value))
        return true;
    int idx = exp->getFirstIdx();
    switch (idx) {
        case idPlus:
        case idMinus:
        case idBitOr:
        {
            // Expect op k1 k2 where k1 and k2 are constants    
            Exp* k1; ADDRESS value1;
            k1 = exp->getSubExpr(0);
            if (!isConstTerm(k1, constMap, value1)) {
                delete k1;
                return false;
            }
            Exp* k2; ADDRESS value2;
            k2 = exp->getSubExpr(1);
            if (!isConstTerm(k2, constMap, value2)) {
                delete k2;
                return false;
            }
            delete k1; delete k2;
            switch (idx) {
                case idPlus:  value = value1 + value2; break;
                case idMinus: value = value1 - value2; break;
                case idBitOr: value = value1 | value2; break;
            }
            return true;
        }
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        checkBBconst
 * OVERVIEW:        Check the current BB for the presence of constants; create
 *                  pointers to functions or floating point constants if needed
 * NOTE:            This is a temporary solution, which does not attempt to
 *                  do data flow analysis, and only checks the current BB
 * PARAMETERS:      pBB: Pointer to the BB to be checked
 *                  proc: Ptr to UserProc object for the current procedure
 * RETURNS:         <nothing>
 *============================================================================*/
void Analysis::checkBBconst(Prog *prog, PBB pBB)
{
    std::list<RTL*>* pRtls = pBB->getRTLs();
    if (pRtls == 0)
        return;
    // The following is a map from Exp* (representing the register, var, or
    // temp being assigned a constant, to ADDRESS (representing the constant
    // that is held there). This is needed because constant addresses are some-
    // time arrived at by operations like + or | on other constants.
    // For example, see the ninths test program (Sparc)
    KMAP constMap;

    std::list<RTL*>::iterator rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        int n = (*rit)->getNumExp();
        for (int i=0; i < n; i++) {
            RTAssgn* pRT = (RTAssgn*)(*rit)->elementAt(i);
            if (pRT->getKind() != RTASSGN) continue;
            // Locate any register loads where the RHS is constant. The constant
            // could be a simple constant, or a register that is already known
            // to be constant, or simple operation (+, -, |) on two of these
            Exp* LHS = pRT->getSubExp1();
            Exp* RHS = pRT->getSubExp2();
            ADDRESS value;
            if (isRegister(LHS)) {
                if (isConst(RHS, constMap, value))
                    constMap[*LHS] = value;
                else
                    // If the LHS is already in the map, delete it, because it's
                    // no longer constant
                    constMap.erase(*LHS);
            }

            // Locate any memory loads where the destination is a register.
            // We need to check more than just floating point registers, because
            // sometimes a double is loaded into two integer registers. In this
            // last case, the constants are treated as two integers.
            // The memory reference is not necessarily at the "top level", e.g.
            // r[32] := r[32] *f fsize(64, 80, m[const]);
            std::list<Exp*> result;
            if (!RHS->searchAll(memX, result))
                // No memOfs; ignore
                continue; 
            // We have at leat one memOf; go through the list
            std::list<Exp*>::iterator it;
            for (it=result.begin(); it != result.end(); it++) {
                Exp* address;
                address = (*it)->getSubExpr(0);
                if (isConst(address, constMap, value))
                    // This is what we are waiting for: assignment from a
                    // constant memory address. value is the const address
                    processConst(prog, value, **it, RHS);
                delete address;
            }
        }
    }
}
#endif // USE_PROCESS_CONST

#if 0
/*==============================================================================
 * FUNCTION:        isFlagFloat
 * OVERVIEW:        Return true if the RTFlagCall parameter refers to the
 *                    floating point flags
 * PARAMETERS:      rt: pointer to the flagcall object (i.e. a Binary*)
 *                  proc: points to the UserProc for the proc containing rt
 * RETURNS:         True if floating point (see overview)
 *============================================================================*/
bool Analysis::isFlagFloat(Exp* rt, UserProc* proc)
{
    Binary* b = (Binary*)rt;
    Exp* li = b->getSubExp2();
    if (li->getOper() == opNil) {
        return false;
    }
    // We assume it's FP if the first argument is a floating point register
    assert(li->getOper() == opList);
    Exp* first = li->getSubExp1();
    Exp* firstSub = first->getSubExp1();
    if (first->getOper() == opRegOf) {
        if (firstSub->getOper() == opIntConst) {
            int regNum = ((Const*)firstSub)->getInt();
            // Get the register's intrinsic type (note how ugly this is).
        RTLInstDict &dict = proc->getProg()->pFE->getDecoder()->getRTLDict();
        if (dict.DetRegMap.find(regNum) == dict.DetRegMap.end())
            return false;
        Register &reg = dict.DetRegMap[regNum];
            Type* rType = reg.g_type();
            bool ret = rType->isFloat();
        delete rType;
            return ret;
        } else if (first->getOper() == opTemp) {
            Type* rType = Type::getTempType(((Const*)firstSub)->getStr());
            bool ret = rType->isFloat();
            delete rType;
            return ret;
        }
    }
    // We also assume it's FP if the first argument is an FP variable
    if (first->getOper() == opVar) {
        // FIXME: This is completely broken in proc; assumes v1 etc names.
        // For now, assume integer
        //char* vname = ((Const*)firstSub)->getStr();
        return false;
    }       
    return false;
}
#endif

    // analyse calls
void Analysis::analyseCalls(PBB pBB, UserProc *proc)
{
    std::list<RTL*>* rtls = pBB->getRTLs();
    for (std::list<RTL*>::iterator it = rtls->begin(); it != rtls->end(); 
      it++) {
        if ((*it)->getKind() != CALL_RTL) continue;
        HLCall *call = (HLCall*)*it;
        if (call->getDestProc() == NULL && !call->isComputed()) {
            Proc *p = proc->getProg()->findProc(call->getFixedDest());
            assert(p);
            call->setDestProc(p);
        }
    }
}

