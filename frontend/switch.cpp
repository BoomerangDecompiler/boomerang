/*
 * Copyright (C) 1998-2003, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       switch.cpp
 * OVERVIEW:   This file contains routines to determine whether a register
 *             jump instruction is likely to be part of a switch statement.
 *============================================================================*/

/*
 *  $Revision$
 *  Switch statements are generally one of the following three
 *  forms, when reduced to high level representation, where var is the
 *  switch variable, numl and numu are the lower and upper bounds of that
 *  variable that are handled by the switch, and T is the address of the
 *  jump table. Out represents a label after the switch statement.
 *  Each has this pattern to establish the upper bound:
 *      jcond (var > numu) out
 *  Most have a subtract (or add) to establish the lower bound; some (e.g.
 *  epc) have a compare and branch if lower to establish the lower bound;
 *  if this is the case, the table address needs to be altered
 * Here are the 4 types discovered so far; size of an address is 4:
 *  A) jmp m[<expr> * 4 + T]
 *  O) jmp m[<expr> * 4 + T] + T
 *  H) jmp m[(((<expr> & mask) * 8) + T) + 4]
 *  R) jmp %pc + m[%pc + ((<expr> * 4) + k)]        // O1
 *  r) jmp %pc + m[%pc + ((<expr> * 4) - k)] - k    // O2
 *  where for forms A, O, R, r <expr> is one of
 *      r[v]                            // numl == 0
 *      r[v] - numl                     // usual case, switch var is int
 *      ((r[v] - numl) << 24) >>A 24)   // switch var is char in lower byte
 *      etc
 * or in form H, <expr> is something like
 *      ((r[v] - numl) >> 4) + (r[v] - numl)
 *      ((r[v] - numl) >> 8) + ((r[v] - numl) >> 2) + (r[v] - numl)
 *      etc.
 *  Forms A and H have a table of pointers to code handling the switch
 *  values; forms O and r have a table of offsets from the start of the
 *  table itself (i.e. all values are the same as with form A, except
 *  that T is subtracted from each entry.) Form R has offsets relative
 *  to the CALL statement that defines "%pc".
 *  Form H tables are actually (<value>, <address>) pairs, and there are
 *  potentially more items in the table than the range of the switch var.
 *
 */

/* 24 Mar 03 - Mike: Converted from UQBT code
 * 27 Mar 03 - Mike: Fixed lack of clone() calls
 */


/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "exp.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"                   // For nextVar()
#include "prog.h"                   // For prog.cover
#include "frontend.h"
#include "signature.h"
#include "statement.h"
#include "boomerang.h"

extern char* operStrings[];

typedef std::list<RTL*>::iterator               RTLList_IT;

/*==============================================================================
 * Forward declarartions.
 *============================================================================*/
void setSwitchInfo(PBB pSwitchBB, char chForm, int iLower, int iUpper,
    ADDRESS uTable, int iNumTable, int iOffset, RTLList_IT itDefinesSw,
    UserProc* pProc);

/*==============================================================================
 * File globals.
 *============================================================================*/
static std::set<PBB> setPathDone;        // Set of PBBs already traversed

// Arrays for the three forms (five since O has 2 subforms)

// Pattern: m[<expr> * 4 + T ]
Location *formA  = Location::memOf(new Binary(opPlus,
            new Binary(opMult,
                new Terminal(opWild),
                new Const(4)),
            new Terminal(opWildIntConst)));
//static int arrA[] = {idMemOf, opPlus, opMult, -1 /* Whole subexpression */,
//    opIntConst, 4, opIntConst, -1};

// Pattern: m[<expr> * 4 + T ] + T
Binary *formO  = new Binary(opPlus,
    Location::memOf(new Binary(opPlus,
            new Binary(opMult,
                new Terminal(opWild),
                new Const(4)),
            new Terminal(opWild))),
    new Terminal(opWildIntConst));
//static int arrO[] = {opPlus, idMemOf, opPlus, opMult, -1 /* Whole
//    subexpression */, opIntConst, 4, opIntConst, -1, opIntConst, -1};

// Pattern example: m[(((<expr> & 63) * 8) + 12340) + 4]
// Simplifies to m[((<expr> & 63) * 8) + 12344]
Location *formH = Location::memOf(new Binary(opPlus,
        new Binary(opMult,
            new Binary(opBitAnd,
                new Terminal(opWild),
                new Terminal(opWildIntConst)
            ),
            new Const(8)
        ),
        new Terminal(opWildIntConst)
    )
);

//static int arrH[] = {idMemOf, opPlus, opMult, idBitAnd,
//    -1 /* whole subexpression */, opIntConst, -1, opIntConst, 8,
//    opIntConst, -1};

// Pattern: %pc + m[%pc  + (<expr> * 4) + k]
// where k is a small constant, typically 28 or 20
Binary *formO1 = new Binary(opPlus,
    new Terminal(opPC),
    Location::memOf(new Binary(opPlus,
            new Terminal(opPC),
            new Binary(opPlus,
                new Binary(opMult,
                    new Terminal(opWild),
                    new Const(4)
                ),
                new Const(opWildIntConst)
            )
        )
    )
);
                
//static int arrO1[] = {opPlus, opPC, idMemOf, opPlus, opPlus, opPC, 
//    opMult, -1 /* Whole subexpression */, opIntConst, 4, opIntConst, -1};

// Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
// where k is a smallish constant, e.g. 288 (/usr/bin/vi 2.6, 0c4233c).
Binary *formO2 = new Binary(opPlus,
    new Terminal(opPC),
    Location::memOf(new Binary(opPlus,
            new Terminal(opPC),
            new Binary(opMinus,
                new Binary(opMult,
                    new Terminal(opWild),
                    new Const(4)
                ),
                new Terminal(opWildIntConst)
            )
        )
    )
);
//static int arrO2[] = {opMinus, opPlus, opPC, idMemOf, opMinus, opPlus, opPC,
//    opMult, -1 /* Whole subexpression */, opIntConst, 4, opIntConst, -1,
//    opIntConst, -1};

// A subexpression representing `r[v] - k' needed in two places below
Binary *expRegMinus = new Binary(opMinus,
    new Terminal(opWildRegOf),
    new Terminal(opWild));
Binary *expRegPlus = new Binary(opPlus,
    new Terminal(opWildRegOf),
    new Terminal(opWild));
Location *expReg999 = Location::regOf(999);

// Pattern: r[iReg] + negativeConst
Binary *expRegPlusNegConst = new Binary(opPlus, new Terminal(opWildRegOf),
    new Terminal(opWild)); 

// Pattern: r[x] & const
Binary *expRegAndConst = new Binary(opBitAnd,
    new Terminal(opWildRegOf),
    new Terminal(opWildIntConst));

// Address of the last CALL instruction copying the PC to a register.
// Needed for form O2 / "r"
ADDRESS uCopyPC;

// Return true if this is a complete compare experssion (no nills)
bool isComplete(Exp* e) {
    if (e == NULL) return false;
    if (e->getOper() == opNil) return false;
    Binary* b = dynamic_cast<Binary*>(e);
    if (b == NULL) return false;
    if (b->getSubExp1()->getOper() == opNil) return false;
    if (b->getSubExp2()->getOper() == opNil) return false;
    return true;
}

// Insert e after assignment to temps in rtl
void insertAfterTemps(RTL* rtl, Statement* s) {
    Statement* cur;
    Exp* lhs;
    int n = rtl->getNumStmt();
    int i = 0;
    while (i < n) {
        cur = rtl->elementAt(i);
        if (!cur->getKind() == STMT_ASSIGN) break;
        lhs = cur->getLeft();
        if (!lhs->isRegOf()) break;
        lhs = ((Unary*)lhs)->getSubExp1();
        if (!lhs->isTemp()) break;
        i++;
    }
    rtl->insertStmt(s, i);
}

/*==============================================================================
 * FUNCTION:      getPrevRtl
 * OVERVIEW:      Get the preceeding RTL to the current one. If we are at the
 *                top of a BB, choose an in-edge to traverse. If there are more
 *                than one, the static global list of current paths is used to
 *                track down the next
 *                If the path is via out edge 0 of the previous BB (branch
 *                taken),then set bNegate.
 * PARAMETERS:    pCurBB - Pointer to the current BB
 *                itRtl - iterator to the current RTL this BB
 *                bNegate - Set true if the (new) current BB is a two-way BB
 *                  that goes to the previous BB via the "true" outedge. This
 *                  indicates logical negation as far as switch logic goes
 * RETURNS:       true if no more paths to try; also sets bNegate
 *============================================================================*/
bool getPrevRtl(PBB& pCurBB, RTLList_IT& itRtl, bool& bNegate) {
    bNegate = false;
    if (itRtl != pCurBB->getRTLs()->begin()) {
        itRtl--;                    // Just go to the previous RTL this BB
        return false;               // More to do
    }
    // No more RTLs this BB. We need to choose one of the in edges, if present
    if (pCurBB->getInEdges().size() == 0)
        return true;            // No in-edges
    PBB pOldBB = pCurBB;
    int numInEdges = pCurBB->getInEdges().size();
    if (numInEdges == 1) {
        // Just one in-edge. Use that.
        pCurBB = pCurBB->getInEdges()[0];
    }
    else {
        // More than one in-edge.
        if (Boomerang::get()->debugSwitch) {
            LOG << "Multiple in-edges for BB at " << 
              pCurBB->getLowAddr() << "\n";
        }
#if 0
        LOG << "Multiple in-edges for BB at " << 
            pCurBB->getLowAddr() << ":\n==\n";
        for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
            (pCurBB->getInEdges()[ii])->print();
        }
        LOG << "\n==\n";
#endif
#if 0       // This doesn't seem to suit form H switches...
        // Choose the most suitable in-edge. Best is a fall through,
        // followed by a one-way jump, followed by the false leg of a
        // two-way jump. The latter is because occasionally a test is
        // made for a specific case, and that (true) branch is optimised
        // directly to the jump. Example (from Pentium gcc spec benchmark):
        // 080fed8b cmpl   $0x7,%edx    ! Specific case 7
        // 080fed8e je     080fedb7     ! True branch to indexed jump!
        // 080fed90 movl   %esi,%eax
        // ...
        // 080feda6 testb  $0x1,%al
        // 080feda8 jne    080ff144     ! Exit switch
        // 080fedae cmpl   $0x7,%edx    ! Test lower bound
        // 080fedb1 ja     080ff13c     ! Exit if above (important)
        // Fall through if false
        // 080fedb7 jmp    *0x80fedc0(,%edx,4) ! Indexed jump
        
        bool found = false;
        // Try for fallthrough BB
        for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
            if (setPathDone.count(pCurBB->getInEdges()[ii]) != 0)
                // Already traversed this path!
                continue;
            if ((pCurBB->getInEdges()[ii])->getType() == FALL) {
                found = true;
                pCurBB = pCurBB->getInEdges()[ii];
                break;
            }
        }
        if (!found) {
            // Try for one way BB
            for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
                if (setPathDone.count(pCurBB->getInEdges()[ii]) != 0)
                    // Already traversed this path!
                    continue;
                if ((pCurBB->getInEdges()[ii])->getType() == ONEWAY) {
                    found = true;
                    pCurBB = pCurBB->getInEdges()[ii];
                    break;
                }
            }
        }
        if (!found) {
            // Try for false edge of two way BB
            for (unsigned ii=0; ii < pCurBB->getInEdges().size(); ii++) {
                if (setPathDone.count(pCurBB->getInEdges()[ii]) != 0)
                    // Already traversed this path!
                    continue;
                if ((pCurBB->getInEdges()[ii])->getType() == TWOWAY) {
                    PBB inEdge = pCurBB->getInEdges()[ii];
                    if ((inEdge->getOutEdges())[1] == pCurBB) {
                        found = true;
                        pCurBB = pCurBB->getInEdges()[ii];
                        break;
                    }
                }
            }
        }
        if (!found)
#endif
        {
            // Just pick the first in-edge (no better info)
            pCurBB = pCurBB->getInEdges()[0];
            int edge = 0;
            while ((setPathDone.count(pCurBB) != 0) && (edge+1 < numInEdges))
                // Already considered first path. Use the next
                pCurBB = pCurBB->getInEdges()[++edge];
        }
        if (Boomerang::get()->debugSwitch)
            LOG << "Chose in-edge at " << pCurBB->getLowAddr() << "\n";
    }

    // If we have already considered it, we have a loop, so exit
    if (setPathDone.count(pCurBB) != 0)
        // No more paths to try
        return true;        // Was return false; surely wrong
    setPathDone.insert(pCurBB); // Don't consider it again
    // Check if we have come via the first out edge of a TWOWAY BB
    if (pCurBB->getType() == TWOWAY &&
        pCurBB->getOutEdges().size() &&
        pCurBB->getOutEdges()[0] == pOldBB)
        // This is a two way BB, and the first out edge (the
        // taken branch) points to the BB we came from.
        // This indicates a logical negation, for the purposes
        // of switch detection
        bNegate = true;
    // Point to the last RTL
    itRtl = pCurBB->getRTLs()->end(); itRtl--;
    return false;
}


/*==============================================================================
 * FUNCTION:    isSwitch
 * OVERVIEW:    This is the main function for determining switch statements.
 *              Attempt to determine whether this DD instruction is a switch
 *              statement. If so, return true, and set iLower, uUpper to the
 *              switch range, and set uTable to the native address of the
 *              table. If it is form B (the table is an array of offsets from
 *              the start of the table), then chFormB will be 'B', etc.
 *              If it is form H (hash table), then iNumTable will be the
 *              number of entries in the table (not the number of cases,
 *              or max-min).
 * PARAMETERS:  pSwitchBB - pointer to the BB with the register branch at end
 *              pDest - ref to ptr to the Exp with the dest of the branch
 *              pProc - pointer to the current Proc object
 * SIDE EFFECT: Sets a SWITCH_INFO struct to the CaseStatement RTL at the end of
 *              the BB, with info like table address, upper and lower bound,
 *              etc
 * RETURNS:     True if the given instruction is a switch statement
 *                Also creates a new SWITCH_INFO struct and fills it with info
 *                about this switch; last RTL this BB retains a pointer to this
 *============================================================================*/
bool isSwitch(PBB pSwitchBB, Exp* pDest, UserProc* pProc, BinaryFile* pBF) {

    // return false;                // Use to disable switch analysis

    Exp* expJmp;                    // The expression representing the jump, or
                                    //  reresenting <expr> when bPostForm
    bool bStopped = false;          // True when search must stop
    bool bUpperFnd = false;         // True when have ss for upper bound
    bool bGotUpper = false;         // True when have final iUpper
    bool bGotLower = false;         // True when know lower bound
    bool bPostForm = false;         // Indicates that expJmp repr <expr> now
    bool bLoaded = false;           // True when the table memory is loaded
    bool bGotDefines = false;       // True when have set itDefinesSw
    PBB pCurBB = pSwitchBB;         // Current basic block
    Exp* expBound = NULL;           // Expression for the upper etc bound
    RTL* pRtl;                      // Current RTL
    int indexRT = 0;                // Current RT in current RTL
    char chForm = '?';              // Unknown form so far
    int iLower, iUpper = 0;         // Upper and lower bounds for switch var
    ADDRESS uTable = 0;             // Address of the table
    int iNumTable = 0;              // Number of items in table (form H only)
    int iOffset;                    // Offset from jump to table (form R only)
    RTLList_IT itDefinesSw;         // Iterator to RTL that defines switch var
    int iCompareReg = -1;           // The register involved in a compare, that
                                    //  might turn out to be of interest
    Exp* expCompare = NULL;         // Pointer to last compare argument
    // I'm not sure that bDestBound is a good idea!
    // It seems to be asserting that the current RTL affects expBound, and not
    // expJmp-> But it could affect both!
    bool bDestBound = false;
	int iReg;
    iOffset = 0;                    // Needed only for type R
    uCopyPC = 0;                    // Needed only for type "r"; no call yet
    // Get the register that contains the function return value
    // e.g. 8 for sparc
#if 0       // Problem is that the signature object is generic (class Signature)
            // until the signature is "promoted" (happens much later).
    Exp* retExp = pProc->getSignature()->getReturnExp();
#else
    // So for now, we call a special cludge in signature.cpp
    Exp* retExp = Signature::getReturnExp2(pBF);
#endif
    assert(retExp);
    const int iFuncRetReg = ((Const*)((Unary*)retExp)->getSubExp1())->getInt();
	int iDest;
	int n;
    Exp* pRHS;
    Exp* pLHS;
	Statement* currStmt;
    Exp* temp;

    // Clear the set used for detecting cycles
    setPathDone.erase(setPathDone.begin(), setPathDone.end());

    // Get the RTL for the jump
    RTLList_IT itCurRtl = (pSwitchBB->getRTLs()->end());
    pRtl = *--itCurRtl;
    ADDRESS uJump = pRtl->getAddress();     // Needed for type R

    expJmp = pDest->clone();                 // Copy the desintation of the jump

    if (Boomerang::get()->debugSwitch)
        LOG << uJump << ":expJmp begins with " << expJmp << "\n";

    // Check if the memory load is part of the jump instruction (e.g.
    // x86 jmp [%eax + %ebx*4]). If so, count this as the first memory
    // load
    if (expJmp->isMemOf())
        bLoaded = true;

    do {
        bool bNegate;
        if ((++indexRT) >= (*itCurRtl)->getNumStmt()) {
            bStopped = getPrevRtl(pCurBB, itCurRtl, bNegate);
            indexRT = 0;            // Start with first RT
        }
        if (bStopped) {
            // Maybe we have already detected the form, but just haven't
            // found a lower bound. Well, it must be zero!
            if (bPostForm) {
                iLower = 0;
                setSwitchInfo(pSwitchBB, chForm, iLower, iUpper,
                    uTable, iNumTable, iOffset, itDefinesSw, pProc);
                return true;
            }
            else return false;
        }
        pRtl = *itCurRtl;
        if (bNegate) {
            // We have come via a conditional branch. For the purposes
            // of switch detection, this means a logical negation
            if (expBound)
                expBound = new Unary(opNot, expBound);
            else
                expBound = new Unary(opNot, new Terminal(opNil));
        }

        // General approach: we look for the comparison that defines the
        // switch variable's upper bound separately.
        // We maintain a Expression, expJmp, that is the current
        // expression for the destination of the register jump. To make
        // it easier to compare with canonical forms of switch statements,
        // we impose an ordering on commutative partial expressions, such
        // that when one side is an integer constant, it will be placed
        // on the right; Exp::simplify() does this for us, as well as
        // folding constants.

        // Another expression, expBound, is used to keep track of the
        // upper (and occasionally lower) bounds. Mostly, we end up with
        // r[v] GT k, but occasionally if the upper bound is larger than
        // will fit into an imm13, we get r[v] GT r[x] and have to subs-
        // titute into expBound. For the epc compiler, we can get
        // r[v] GT k1 || r[v] LT k2, and have to add 4*k2 to the table
        // address.
        // Along the way, we check for subtracts that assign to a register
        // of interest.  This is assumed to set the lower bound, and is
        // therefore the ideal place to assign the switch variable.
        if (pRtl->getNumStmt() > indexRT) {
            currStmt = pRtl->elementAt(indexRT);
            // Only interested to assignements to registers (not temps, as in
            // compares)
            if (currStmt->getKind() == STMT_ASSIGN &&
              currStmt->getLeft()->isRegOfK()) {
                Exp* rhs = currStmt->getRight();
                // r[ int R] - iReg
                if (rhs && rhs->getOper() == opMinus) {
                    // We have a subtract; is it from a register?
                    Exp* sub1 = rhs->getSubExp1();
                    if (sub1->isRegOfK()) {
                        // We have a subtract from a register. But is the
                        // destination (lhs) a register of interest? That is,
                        // is the LHS register contained in expJmp?
                        Exp* lhs = currStmt->getLeft();
                        if (lhs->isRegOfK()) {
                            Exp* result;
                            if (expJmp->search(lhs, result)) {
                                // Yes, it is. Remember this RTL, since it
                                // defines the switch variable.
                                itDefinesSw = itCurRtl;
                                bGotDefines = true;
                                // Note that we don't set the lower bound here.
                                // This is done much later, after the whole
                                // form has been matched.
                            }
                        }
                    }
                }
                // Or it could be an add of a negative constant
                else if (rhs && *rhs == *expRegPlusNegConst) {
                    // Is the const negative?
                    Exp* sub = rhs->getSubExp2();
                    int iconst = ((Const*)sub)->getInt();
                    if (iconst < 0) {
                        // Is the destination a register of interest?
                        Exp* lhs = currStmt->getLeft();
                        sub = lhs->getSubExp1();
                        assert(sub->isIntConst());
                        Exp* result;
                        if (expJmp->search(lhs, result)) {
                            // Yes, it is. Remember this RTL, since it
                            // defines the switch variable
                            itDefinesSw = itCurRtl;
                            bGotDefines = true;
                        }
                    }
                }
            }
        }

        // See if we are defining the upper or lower bound with a compare
        // and branch. We need to do this first, because most other
        // cases require an assignment to a register of interest, and will
        // continue the loop if this is not found
        Location *regOfFuncRet = Location::regOf(iFuncRetReg);
        Exp* result;
        if (pRtl->isCall() && expJmp->search(regOfFuncRet, result)) {
            // We have come across a call, and the function return
            // register is used in expJmp-> We have to assume that
            // this call will define the switch variable. Replace it
            // with 999 to ensure it is not altered.
            bool changed;
            expJmp = expJmp->searchReplaceAll(regOfFuncRet, expReg999,
              changed);
            // Do the same to expBound
            expBound = expBound->searchReplaceAll(regOfFuncRet, expReg999,
              changed);
            continue;
        }
        else if (pRtl->isBranch()) {
            // It's a conditional branch
            BRANCH_TYPE jt = ((BranchStatement*)pRtl->getHlStmt())->getCond();
            OPER operBr = opNil;
            if (expBound && expBound->getOper() == opNot) {
                // Expect branch if unsigned lower or equals
                if (jt == BRANCH_JULE) {
                    // Replace the not with opUpper
                    expBound = new Binary(opUpper,
                        expBound,
                        new Terminal(opNil));
                }
                else {
                    // not really sure what to do... but it does recover
                    // from this by popping the opNot
//                  LOG << "Switch @ " << pRtl->getAddress()
//                    << " opNot and branch that is not low or equals\n";
                }
            }
            else {
                // No negation.
                // Most times we expect branch if GTu
                // Some compilers (e.g. epc modula 2) have a signed
                // greater than comparison
                // epc also checks for the lower bound with a signed BL instr
                if ((jt == BRANCH_JUG) || (jt == BRANCH_JSG))
                    operBr = opUpper;           // Defining upper bound
                else if (jt == BRANCH_JSL)
                    operBr = opLower;           // Defining lower bound
                if (operBr != opNil) {
#if 0
                    // If we already have an expression there, we have
                    // to prepend an opOr
                    if (expBound)
                        expBound->prep(opOr);
                    // Add the appropriate index
                    expBound->push(operBr);
#else
                    if (expBound == NULL)
                        expBound = new Binary(operBr,
                            new Terminal(opNil),
                            new Terminal(opNil));
                    else {
LOG << "FIXME: Supposed to OR current expBound (" << expBound << ") with a bare " << operStrings[operBr] << "\n";
                    }
#endif

                    if (Boomerang::get()->debugSwitch) {
                        LOG << "isSwitch @ " << pRtl->getAddress();
                        LOG << ": expBound now " << expBound << "\n";
                    }

                    continue;
                }
            }
        }

        // Check for a compare instruction for the upper or lower bound
        // Must already have seen an appropriate branch
        // FIXME: Check this!
        //if (expBound->getLastIdx() == opUpper ||
        //    expBound->getLastIdx() == opLower) {
        if (expBound &&
          (expBound->getOper() == opUpper || expBound->getOper() == opLower)) {
            int iReg;
            if ((*itCurRtl)->isCompare(iReg, expCompare)) {
                // It is a compare instruction. But it should be a comparison
                // to a register of interest
                Location *regOfK = Location::regOf(iReg);
                Exp* result;
                if (!expJmp->search(regOfK, result)) {
                    // It doesn't appear to be a register of interest. But it
                    // might be, if there is a copy ahead. Example from
                    // /usr/ccs/bin/dis (Sparc Solaris 2.6):
                    // 130c8:  90 10 00 0a        mov          %o2, %o0
                    // 130cc:  92 00 60 50        add          %g1, 80, %o1
                    // 130d0:  80 a2 a0 19        cmp          %o2, 25
                    // 130d4:  18 80 00 05        bgu          0x130e8
                    // So remember the register
                    iCompareReg = iReg;
                    continue;
                }
                // Append r[ int iReg
                expBound->setSubExp1(regOfK->clone());
                // Append the semantic string for the thing being compared
                // to
                expBound->setSubExp2(expCompare->clone());
                bUpperFnd = true;           // Trigger pattern matching

                if (Boomerang::get()->debugSwitch)
                    LOG << "isSwitch @ " << 
                      (*itCurRtl)->getAddress() << ": expBound now " <<
                      expBound << "\n";

                // Force a check. There may be no instructions after
                // this that assign to a register of interest, etc,
                // but we may have a complete switch statement by now.
                goto forcedCheck;
            }
        }

        // Check if this instruction affects the condition codes. If it
        // does, and there is something in expBound, then we need to pop
        // off the end of expBound, since (by virtue of the fact that we
        // have reached here at all) the instruction defining the CCs is
        // not a compare of interest, the branch is also not a branch of
        // interest. So we remove the opNot, if any
        n = pRtl->getNumStmt();
        if (n == 0) continue;
        // We assume that the last RTL will define the flags, if at all
        if (pRtl->elementAt(n-1)->isFlagAssgn()) {
            if (expBound && expBound->getOper() == opNot) {
                expBound = expBound->getSubExp1();
                if (Boomerang::get()->debugSwitch)
                    LOG << "Popping opNot from expBound: now " <<
                      expBound << "\n";
            }
        }

        // See if we have an RTL whereby the first significant RT is an
        // assignment, and the dest is a register of interest.
        // A register of interest is any register used by expJmp, except
        // 999 (don't want to change it any more)
        // We are also interested in any registers in expBound
        // See if the current RT is an assignment
        if (!pRtl->elementAt(indexRT)->isAssign()) continue;
        currStmt = pRtl->elementAt(indexRT);

        // Check for an instruction that uses %pc on the right hand side
        // (e.g. sparc call $+8). Assume that the first RT will have this use
        pRHS = currStmt->getRight();
        pLHS = currStmt->getLeft();
        if (pRHS->getOper() == opPC)
            uCopyPC = pRtl->getAddress();

        // If this is not a register we can't handle that
        if (!pLHS->isRegOf()) continue;
        if (!pLHS->getSubExp1()->isIntConst()) continue;
        iDest = ((Const*)pLHS->getSubExp1())->getInt();

        // Check that the destination is to a register used by the
        // current jump expression, or by the current bound
        // expression. Remember which expression iDest refers to in
        // bDestBound
        if (!expJmp->search(pLHS, result)) {
            if (expBound && expBound->search(pLHS, result))
                bDestBound = true;
            else continue;
        }

        // Check if we are loading from memory to a register of interest.
        temp = pRHS->clone();
        temp = temp->killFill();        // Remove sign extends, fills
        if (temp->isMemOf()) {
            if (bDestBound) {
                // Loading the upper expression from memory. Freeze the
                // bounds expression
                Location *regOfK = Location::regOf(iDest);
                bool changed;
                expBound = expBound->searchReplaceAll(regOfK, expReg999,
                  changed);
                continue;
            }
            // We record the fact that we have seen the m[] part of the switch
            // expression. All subsequent loads are assumed to be loading the
            // switch variable from memory, which is very different
            if (!bLoaded) {
                bLoaded = true;
                // We don't want to emit code for this RTL, since it is loading
                // from the original source program's jump table. Better to
                // leave the RTLs there, in case the interpreter will need them
                // pRtl->setCommented(true);
            }
            else {
                // We are loading our switch variable from memory.
                // To make sure we don't change this any more, replace
                // the r[iDest] in expJmp to r[999]
                Location *regOfK = Location::regOf(iDest);
                bool changed;
                expJmp = expJmp->searchReplaceAll(regOfK, expReg999, changed);
                // Do the same to expBound, if set
                if (expBound)
                    expBound = expBound->searchReplaceAll(regOfK, expReg999,
                      changed);
                iDest = 999;             // Don't subst again
                // This defines the switch variable, if not already set
                if (!bGotDefines) {
                    bGotDefines = true;
                    itDefinesSw = itCurRtl;
                }
            }
            // Don't exit the loop here; we may be done if we have the table
            // address
        }

        // If iCompareReg is not -1, check if the rhs simplifies to just
        // r[iCompareReg]. If so, and we don't have a proper compare now,
        // that means that we have a copy instruction (move), which means
        // that the compare could be on either register. We assume that
        // the last compare was valid (should really test that the register
        // isn't changed between here and the compare.)
        //if (iCompareReg != -1 && expBound->len() <= 2) {
        if (iCompareReg != -1 && !isComplete(expBound)) {
            Exp* rhs = pRHS;
            //rhs->machineSimplify();      // Could be sparc move: r[0] | r[x]
            rhs->simplify();             // 0 | r[x] -> r[x]
            if (rhs->isRegOf() && ((Unary*)rhs)->getSubExp1()->isIntConst()) {
                // Update expBound to be a valid compare
                if (expBound->getOper() == opNot) {
                    // This can happen when there is a branch not to do with
                    // the upper bound, e.g. /usr/ccs/bin/dis Solaris 2.6 13af0
                    expBound = ((Unary*)expBound)->getSubExp1();
                }
                ((Binary*)expBound)->setSubExp1(rhs);
                // Append the expression for the thing being compared to
                ((Binary*)expBound)->setSubExp2(expCompare);
                bUpperFnd = true;           // Trigger pattern matching
            }
        }

        // Check for r[x] & const; to get here, must assign to a register of
        // interest. If so, and we haven't already set the upper bound, we
        // assume that this `and' is setting the upper bound
        // In any case, the lower bound is 0
        // r[K1] & K2
        if (*pRHS == *expRegAndConst) {
            Exp* sub1 = ((Binary*)pRHS)->getSubExp1();
            Exp* sub2 = ((Binary*)pRHS)->getSubExp2();
            iReg = ((Const*)((Unary*)sub1)->getSubExp1())->getInt();
            if (!bUpperFnd) {
                // Found an upper bound
                bGotUpper = true;
                iUpper = ((Const*)sub2)->getInt();
                // This RT defines the switch variable
                bGotDefines = true;
                itDefinesSw = itCurRtl;
            }
            // Else don't assign itDefinesSw here; it's currently at the
            // compare that sets the upper bound
            // We won't see a lower bound; it's 0
            iLower = 0;
            bGotLower = true;
        }
#if 0
        if (bDestBound) {
            expBound->substReg(iDest, pRHS);
            expBound->simplify();
        }
        else if (iDest != 999) {
            // Substitute register iDest with the RHS
            expJmp->substReg(iDest, pRHS);
        }
#else
        // Substitute both expJmp and expBound, if affected
        if (iDest != 999) {
            Location *regOfK = Location::regOf(iDest);
            bool changed;
            if (expBound) {
                expBound = expBound->searchReplaceAll(regOfK, pRHS, changed);
                expBound->simplify();
            }
            if (expJmp)
                expJmp = expJmp->searchReplaceAll(regOfK, pRHS, changed);
        }
#endif

forcedCheck:
        if (Boomerang::get()->debugSwitch) {
            LOG << "expJmp @ " << (*itCurRtl)->getAddress()
              << ": " << expJmp << "\n";
            if (expBound) LOG << "expBound is " << expBound << "\n";
        }

        // Needn't do any checking until we have found the uppper bound
        if (!bPostForm && bUpperFnd) {
            // Simplify the expression, if possible, using contant folding
            // and various other techniques
            expJmp->simplify();

            if (Boomerang::get()->debugSwitch)
                LOG << "isSwitch @ " << 
                  (*itCurRtl)->getAddress() << ": " << expJmp << "\n";

            // Simplify the expression, by removing all {size} and sign extend
            // (!) operations. These are tangential to the switch form
            //expJmp->removeSize();
            expJmp->killFill();

            // Check for form A (addresses)
            // Pattern: m[<expr> * 4 + T ]
            if (*expJmp == *formA) {
                chForm = 'A';
                expJmp = ((Unary*) expJmp)->becomeSubExp1();     // <expr> * 4 + T
                uTable = (ADDRESS)
                  ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                expJmp = ((Binary*)expJmp)->becomeSubExp1();     // <expr> * 4
                expJmp = ((Binary*)expJmp)->becomeSubExp1();  // <expr>
            }

            // Check for form O (offsets)
            // Pattern: m[<expr> * 4 + T ] + T
            else if (*expJmp == *formO) {
                chForm = 'O';
                expJmp = ((Binary*)expJmp)->becomeSubExp1();
                // m[<expr> * 4 + T]
                expJmp = ((Unary*) expJmp)->becomeSubExp1();     // <expr> * 4 + T
                uTable = (ADDRESS)
                  ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                expJmp = ((Binary*)expJmp)->becomeSubExp1();     // <expr> * 4
                expJmp = ((Binary*)expJmp)->becomeSubExp1();     // <expr>
            }

            // Check for form H (hash table)
            // Note: the hash expression varies
            // Expect an expression like this:
            // m[(((<expr> & 63) * 8) + 12345) + 4]
            // Simplifies to m[((<expr> & 63) * 8) + 12344]
            else if (*expJmp == *formH) {
                chForm = 'H';
                expJmp = ((Unary*) expJmp)->becomeSubExp1();
                //((<expr> & K) * 8) + T
                uTable = (ADDRESS)
                  ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                expJmp = ((Binary*)expJmp)->becomeSubExp1();
                // ((<expr> & K) * 8)
                expJmp = ((Binary*)expJmp)->becomeSubExp1(); // <expr> & K
                expJmp = ((Binary*)expJmp)->becomeSubExp1(); // <expr>
            }

            // Check for form O1 (one of the relocatable versions of O)
            // Expect an expression like this:
            // %pc + m[%pc  + ((<expr> * 4) + k)]
            // where k is a small constant, typically 28
            else if (*expJmp == *formO1) {
                chForm = 'R';
                expJmp = ((Binary*)expJmp)->becomeSubExp2();
                // m[%pc + ((<expr> * 4) + k)]
                expJmp = ((Unary*) expJmp)->becomeSubExp1();
                // %pc + ((<expr> * 4) + k)
                expJmp = ((Binary*)expJmp)->becomeSubExp2(); // (<expr>*4) + k
                // iOffset needed to figure correct out edges
                iOffset = ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                expJmp = ((Binary*)expJmp)->becomeSubExp1(); // <expr> * 4
                expJmp = ((Binary*)expJmp)->becomeSubExp1(); // <expr>
                uTable = uJump + 8;         // Set way earlier
            }

            // Check for form O2 (one of the relocatable versions of O)
            // Expect an expression like this:
            // %pc + m[%pc  + ((<expr> * 4) - k)] - k
            // where k is a smallish constant, e.g. 288
            else if (*expJmp == *formO2) {
                chForm = 'r';
                expJmp = ((Binary*)expJmp)->becomeSubExp2();
                // m[%pc  + ((<expr> * 4) - k)] - k
                int k1 = ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                expJmp = ((Binary*)expJmp)->becomeSubExp1();
                // m[%pc + ((<expr> * 4) - k)]
                expJmp = ((Unary*) expJmp)->becomeSubExp1();
                // %pc + ((<expr> * 4) - k)
                expJmp = ((Binary*)expJmp)->becomeSubExp2();
                // (<expr> * 4) - k
                int k2 = ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                assert(k1 == k2);
                expJmp = ((Binary*)expJmp)->becomeSubExp1(); // <expr> * 4
                expJmp = ((Binary*)expJmp)->becomeSubExp1(); // <expr>
                uTable = uCopyPC - k1;           // uCopyPC set way earlier
            }


            // If we have a form pattern, then we are into the post form
            // phase now
            bPostForm = chForm != '?';
        }


        if (!bGotUpper || !bGotLower) {
            // Check if we have the upper and lower bounds together
            // ((r[v] GT k1] || (r[v] LT k2))
            Binary *expBoth = new Binary(opOr,
                new Binary(opUpper,
                    new Terminal(opWildRegOf),
                    new Terminal(opWildIntConst)),
                new Binary(opLower,
                    new Terminal(opWildRegOf),
                    new Terminal(opWildIntConst)));
            if (expBound && *expBound == *expBoth) {
                // We have both bounds
                Exp* sub1 = expBound->getSubExp1();
                Exp* sub2 = expBound->getSubExp2();
                sub1 = ((Binary*)sub1)->getSubExp2();
                sub2 = ((Binary*)sub2)->getSubExp2();
                iUpper = ((Const*)sub1)->getInt();
                iLower = ((Const*)sub2)->getInt();
                // Adjust the table by iLower*size
                int iSize = 4;
                // We may not know the form as yet!
                // But it's very unlikely to be form H anyway
                if (chForm == 'H') iSize = 8;
                uTable += iLower * iSize;
                if (chForm != '?') {
                    setSwitchInfo(pSwitchBB, chForm, iLower, iUpper,
                        uTable, iNumTable, iOffset, itDefinesSw, pProc);
                    return true;
                }
                bGotUpper = true; bGotLower = true;
            }
        }

        // Check for upper bound only
        if (!bGotUpper) {
            Binary *expUpper = new Binary(opUpper,
                new Terminal(opWildRegOf),
                new Terminal(opWildIntConst));
            if (expBound && *expBound == *expUpper) {
                bGotUpper = true;
                Exp* sub = ((Binary*)expBound)->getSubExp2();
                iUpper = ((Const*)sub)->getInt();
                // If we haven't seen the instruction defining the switch var
                // (usually a subtract), then this compare defines it
                if (!bGotDefines) {
                    bGotDefines = true;
                    itDefinesSw = itCurRtl;
                }
            }
        }

        // When bPostForm is true, we are in the second phase of the
        // algorithm where expJmp represents <expr>
        // We can check for the lower bound by subtraction now, but only
        // if we have the upper bound already
        if (bPostForm && !bGotLower && bGotUpper) {
            expJmp->simplify();

            if (Boomerang::get()->debugSwitch)
                LOG << "Post form @ " << 
                  (*itCurRtl)->getAddress() << ": " << expJmp << "\n";

            // Save the "switch variable" so we can return it in a SWITCH_INFO
//          pSwitchVar = new Exp*(expJmp);
            // Save an iterator to the current RTL, so we can save the switch
            // variable
            // But if we already have it defined (because we have defined the
            // lower bound) then leave it alone
//          if (!bGotDefines)
//              itDefinesSw = itCurRtl;

            // Now check if have `r[v] - k'
            Exp* result;
            bool bRet = expJmp->search(expRegMinus, result);
            if (bRet) {
                // We now have the lower bound, and all is done
                bGotLower = true;
                iLower = ((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                if (Boomerang::get()->debugSwitch)
                    LOG << "Got lower: expJmp " << expJmp <<
                      " -> iLower " << iLower << "\n";
                iUpper += iLower;
            }
            else {
                // Could be r[v] + k, where k is positive; in this case
                // the lower bound is -k
                bool bRet = expJmp->search(expRegPlus, temp);
                if (bRet) {
                    // We now have the lower bound, and all is done
                    iLower =
                      -((Const*)((Binary*)expJmp)->getSubExp2())->getInt();
                    if (Boomerang::get()->debugSwitch)
                        LOG << "Got lower: expJmp " << expJmp <<
                          " -> iLower " << iLower << "\n";
                    iUpper += iLower;
                    bGotLower = true;
                }
            }
        }

        // Check if we are done
        if (bGotLower && bGotUpper && chForm != '?') {
            setSwitchInfo(pSwitchBB, chForm, iLower, iUpper,
                uTable, iNumTable, iOffset, itDefinesSw, pProc);
            return true;
        }

    } while (!bStopped);

    return false;
}

/*==============================================================================
 * FUNCTION:    setSwitchInfo
 * OVERVIEW:    Initialises a new SWITCH_INFO struct with all the appropriate
 *              values, and causes the RTL at the end of this BB to
 *              store a pointer to it
 * PARAMETERS:  pSwitchBB: pointer to the original BB with the CaseStatement as
 *              the last RTL
 *              chForm: Switch form: 'A', 'O', 'R', or 'H'
 *              iLower: Lower bound of the switch variable
 *              iUpper: Upper bound for the switch variable
 *              uTable: Native address of the table
 *              iNumTable: Number of entries in the table (form H only)
 *              iOffset: Distance from jump to table (form R only)
 *              itDefinesSw: iterator to RTL that defines the switch var
 *              pProc: pointer to the current Proc object
 * RETURNS:       <nothing>
 *============================================================================*/
void setSwitchInfo(PBB pSwitchBB, char chForm, int iLower, int iUpper,
    ADDRESS uTable, int iNumTable, int iOffset, RTLList_IT itDefinesSw,
    UserProc* pProc) {
    SWITCH_INFO* pSwitchInfo = new SWITCH_INFO;
    pSwitchInfo->chForm = chForm;
    pSwitchInfo->iLower = iLower;
    pSwitchInfo->iUpper = iUpper;
    pSwitchInfo->uTable = uTable;
    pSwitchInfo->iNumTable = iNumTable;
    pSwitchInfo->iOffset = iOffset;
    RTL* last = pSwitchBB->getRTLs()->back();
    CaseStatement* jump = (CaseStatement*)last->getHlStmt();
    jump->setSwitchInfo(pSwitchInfo);

    // Now add an assignment to the RTL defining the switch variable, so we can
    // use that v[] in the final switch statement in the intermediate code.
    // If the assignment is a subtract, assume that this subtract defines the
    // lower bound, so we want the first subexpression of the RHS:
    // r[8] = r[8] - 2
    // Otherwise, assume that there is no lower bound, so we want the result:
    // r[9] = m[r[16] + 572]
    Exp* swLocal = pProc->newLocal(new IntegerType());

    // Want the defining assignment. Assume it's the last non flags Assign
    // of the RTL
    int n = (*itDefinesSw)->getNumStmt();
    int i=n-1;
    Statement* currStmt = (*itDefinesSw)->elementAt(i);
    while (!currStmt->getKind() == STMT_ASSIGN || currStmt->isFlagAssgn()) {
        assert(i > 0);
        currStmt = (*itDefinesSw)->elementAt(--i);
    }
    Exp* rhs = currStmt->getRight();
    if (rhs->getOper() == opMinus) {
        // We want to insert the var assignment before the defining assignment,
        // and from the first subexpression
        insertAfterTemps(*itDefinesSw, new Assign(
          swLocal, rhs->getSubExp1()->clone()));
    }
    // Check if it's adding a negative constant to a register. If so,
    // assume it's just like the subtract above
    else if ((*rhs == *expRegPlusNegConst) &&
      (((Binary*)rhs)->getSubExp2()->isIntConst()) &&
      (((Const*)((Binary*)rhs)->getSubExp2())->getInt() < 0)) {
        insertAfterTemps(*itDefinesSw, new Assign(
          swLocal, rhs->getSubExp1()->clone()));
    }
    else {
        // We assume that this assign is a load (or something) that defines the
        // switch variable
        Exp* lhs = currStmt->getLeft();
        (*itDefinesSw)->appendStmt(new Assign(
          swLocal, lhs->clone()));
    }
    pSwitchInfo->pSwitchVar = swLocal;
}

/*==============================================================================
 * FUNCTION:      processSwitch
 * OVERVIEW:      Called when a switch has been identified. Visits the
 *                  destinations of the switch, adds out edges to the BB, etc
 * PARAMETERS:    pBB - Pointer to the BB containing the register jump
 *                delta - (uHost - uNative)
 *                pCfg - Pointer to the Cfg object for the current procedure
 *                tq- queue of targets yet to be visited
 *                pBF - pointer to the BinaryFile object
 * RETURNS:       <nothing>
 *============================================================================*/
void processSwitch(PBB pBB, int delta, Cfg* pCfg, TargetQueue& tq,
  BinaryFile* pBF) {

    RTL* last = pBB->getRTLs()->back();
    CaseStatement* jump = (CaseStatement*)last->getHlStmt();
    SWITCH_INFO* si = jump->getSwitchInfo();
    // Update the delta field
    si->delta = delta;

    if (Boomerang::get()->debugSwitch) {
        LOG << "Found switch statement type " << si->chForm <<
          " with table at 0x" << si->uTable << ", ";
        if (si->iNumTable)
            LOG << si->iNumTable << " entries, ";
        LOG << "lo= " << si->iLower << ", hi= " << si->iUpper << "\n";
    }
    ADDRESS uSwitch;
    int iNumOut, iNum;
    if (si->chForm == 'H') {
        iNumOut = 0; int i, j=0;
        for (i=0; i < si->iNumTable; i++, j+=2) {
            // Endian-ness doesn't matter here; -1 is still -1!
            int iValue = ((ADDRESS*)(si->uTable+delta))[j];
            if (iValue != -1)
                iNumOut++;
        }
        iNum = si->iNumTable;
    }
    else {
        iNumOut = si->iUpper-si->iLower+1;
        iNum = iNumOut;
    }
    // Emit an NWAY BB instead of the COMPJUMP
    // Also update the number of out edges.
    pBB->updateType(NWAY, iNumOut);
    
    for (int i=0; i < iNum; i++) {
        // Get the destination address from the
        // switch table.
        if (si->chForm == 'H') {
            int iValue = pBF->readNative4(si->uTable + i*2);
            if (iValue == -1) continue;
            uSwitch = pBF->readNative4(si->uTable + i*8 + 4);
        }
        else
            uSwitch = pBF->readNative4(si->uTable + i*4);
        if ((si->chForm == 'O') || (si->chForm == 'R') || (si->chForm == 'r'))
            // Offset: add table address to make a real pointer to code
            // For type R, the table is relative to the branch, so take iOffset
            // For others, iOffset is 0, so no harm
            uSwitch += si->uTable - si->iOffset;
        if (uSwitch < pBF->getLimitTextHigh()) {
            tq.visit(pCfg, uSwitch, pBB);
            pCfg->addOutEdge(pBB, uSwitch, true);
        } else {
            LOG << "switch table entry branches to past end of text section " 
                << uSwitch << "\n";
            iNumOut--;
        }
    }

    // this can change now as a result of bad table entries
    pBB->updateType(NWAY, iNumOut);

}
