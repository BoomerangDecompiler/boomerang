/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       frontend/pentiumfrontend.cpp
 * OVERVIEW:   This file contains routines to manage the decoding of pentium
 *             instructions and the instantiation to RTLs. These functions
 *             replace frontend.cc for decoding pentium instructions.
 *============================================================================*/

/*
 * $Revision$
 * 21 Oct 98 - Mike: converted from frontsparc.cc
 * 21 May 02 - Mike: Mods for boomerang
 * 27 Nov 02 - Mike: Fixed a bug in the floating point fixup code, which was
 *                  screwing up registers in flag calls
 * 30 Sep 03 - Mike: processFloatCode ORs mask with 0x04 for compilers that
 *                  ignore the C1 status bit (e.g. MSVC)
 *                  Also more JE cases
 * 04 Aug 04 - Mike: Quick and dirty hack for overlapped registers (X86 only)
*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <sstream>
#include "types.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "pentiumfrontend.h"
#include "rtl.h"
#include "decoder.h"        // prototype for decodeInstruction()
#include "pentiumdecoder.h"
#include "register.h"
#include "type.h"
#include "cfg.h"
#include "exp.h"
#include "proc.h"
#include "signature.h"
#include "prog.h"           // For findProc()
#include "BinaryFile.h"     // For SymbolByAddress()
#include "boomerang.h"

/*==============================================================================
 * Forward declarations.
 *============================================================================*/

#define FSW 40              // Numeric registers
#define AH 12

/*==============================================================================
 * FUNCTION:      isStoreFsw
 * OVERVIEW:      Return true if the given Statement is an assignment that
 *                  stores the FSW (Floating point Status Word) reg
 * PARAMETERS:    s - Ptr to the given Statement
 * RETURNS:       True if it is
 *============================================================================*/
bool PentiumFrontEnd::isStoreFsw(Statement* s) {
    if (!s->isAssign()) return false;
    Exp* rhs = ((Assign*)s)->getRight();
    Exp* result;
    bool res = rhs->search(Location::regOf(FSW), result);
    return res;
}
/*==============================================================================
 * FUNCTION:      isDecAh
 * OVERVIEW:      Return true if the given RTL is a decrement of register AH
 * PARAMETERS:    r - Ptr to the given RTL
 * RETURNS:       True if it is
 *============================================================================*/
bool PentiumFrontEnd::isDecAh(RTL* r) {
    // Check for decrement; RHS of middle Exp will be r[12]{8} - 1
    if (r->getNumStmt() != 3) return false;
    Statement* mid = r->elementAt(1);
    if (!mid->isAssign()) return false;
    Assign* asgn = (Assign*)mid;
    Exp* rhs = asgn->getRight();
    Binary ahm1(opMinus,
        new Binary(opSize,
            new Const(8),
            Location::regOf(12)),
        new Const(1));
    return *rhs == ahm1;
}
/*==============================================================================
 * FUNCTION:      isSetX
 * OVERVIEW:      Return true if the given Statement is a setX instruction
 * PARAMETERS:    s - Ptr to the given Statement
 * RETURNS:       True if it is
 *============================================================================*/
bool PentiumFrontEnd::isSetX(Statement* s) {
    // Check for SETX, i.e. <exp> ? 1 : 0
    // i.e. ?: <exp> Const 1 Const 0
    if (!s->isAssign()) return false;
    Assign* asgn = (Assign*)s;
    Exp* lhs = asgn->getLeft();
    // LHS must be a register
    if (!lhs->isRegOf()) return false;
    Exp* rhs = asgn->getRight();
    if (rhs->getOper() != opTern) return false;
    Exp* s2 = ((Ternary*)rhs)->getSubExp2();
    Exp* s3 = ((Ternary*)rhs)->getSubExp3();
    if (!s2->isIntConst() || s3->isIntConst()) return false;
    return ((Const*)s2)->getInt() == 1 && ((Const*)s3)->getInt() == 0;
}
/*==============================================================================
 * FUNCTION:      isAssignFromTern
 * OVERVIEW:      Return true if the given Statement is an expression whose RHS is
 *                 a ?: ternary
 * PARAMETERS:    e - Ptr to the given Statement
 * RETURNS:       True if it is
 *============================================================================*/
bool PentiumFrontEnd::isAssignFromTern(Statement* s) {
    if (!s->isAssign()) return false;
    Assign* asgn = (Assign*)s;
    Exp* rhs = asgn->getRight();
    return rhs->getOper() == opTern;
}
/*==============================================================================
 * FUNCTION:        PentiumFrontEnd::bumpRegisterAll
 * OVERVIEW:        Finds a subexpression within this expression of the form
 *                    r[ int x] where min <= x <= max, and replaces it with
 *                    r[ int y] where y = min + (x - min + delta & mask)
 * PARAMETERS:      e: Expression to modify
 *                  min, max: minimum and maximum register numbers before
 *                    any change is considered
 *                  delta: amount to bump up the register number by
 *                  mask: see above
 * APPLICATION:     Used to "flatten" stack floating point arithmetic (e.g.
 *                    Pentium floating point code)
 *                    If registers are not replaced "all at once" like this,
 *                    there can be subtle errors from re-replacing already
 *                    replaced registers
 * RETURNS:         Nothing
 *============================================================================*/
void PentiumFrontEnd::bumpRegisterAll(Exp* e, int min, int max, int delta, int mask) {
    std::list<Exp**> li;
    std::list<Exp**>::iterator it;
    Exp* exp = e;
    // Use doSearch, which is normally an internal method of Exp, to avoid
    // problems of replacing the wrong subexpression (in some odd cases)
    e->doSearch(Location::regOf(new Terminal(opWild)), exp, li, false);
    for (it = li.begin(); it != li.end(); it++) {
        int reg = ((Const*)((Unary*)**it)->getSubExp1())->getInt();
        if ((min <= reg) && (reg <= max)) {
            // Replace the K in r[ K] with a new K
            // **it is a reg[K]
            Const* K = (Const*)((Unary*)**it)->getSubExp1();
            K->setInt(min + (reg - min + delta & mask));
        }
    }
}
/*==============================================================================
 * FUNCTION:      PentiumFrontEnd::processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:    address - the address at which the procedure starts
 *                pProc - the procedure object
 *                os - output stream for rtl output
 *                frag - true if decoding only a fragment of the proc
 *                spec - true if a speculative decode
 * RETURNS:       True if successful decode
 *============================================================================*/
bool PentiumFrontEnd::processProc(ADDRESS uAddr, UserProc* pProc,
    std::ofstream &os, bool frag /* = false */, bool spec /* = false */) {

    // Call the base class to do most of the work
    if (!FrontEnd::processProc(uAddr, pProc, os, frag, spec))
        return false;

    // Need a post-cfg pass to remove the FPUSH and FPOP instructions,
    // and to transform various code after floating point compares to
    // generate floating point branches.
    // processFloatCode() will recurse to process its out-edge BBs (if not
    // already processed)
    Cfg* pCfg = pProc->getCFG();
    pCfg->unTraverse();
    // This will get done twice; no harm
    pProc->setEntryBB();

    processFloatCode(pCfg);

    int tos = 0;
    processFloatCode(pProc->getEntryBB(), tos, pCfg); 

    // Process away %rpt and %skip
    processStringInst(pProc);

    // Process code for side effects of overlapped registers
    processOverlapped(pProc);

    return true;
}

std::vector<Exp*> &PentiumFrontEnd::getDefaultParams()
{
    static std::vector<Exp*> params;
    if (params.size() == 0) {
        params.push_back(Location::regOf(24/*eax*/));
        params.push_back(Location::regOf(25/*ecx*/));
        params.push_back(Location::regOf(26/*edx*/));
        params.push_back(Location::regOf(27/*ebx*/));
        params.push_back(Location::regOf(28/*esp*/));
        params.push_back(Location::regOf(29/*ebp*/));
        params.push_back(Location::regOf(30/*esi*/));
        params.push_back(Location::regOf(31/*edi*/));
        params.push_back(Location::memOf(Location::regOf(28)));
    }
    return params;
}

std::vector<Exp*> &PentiumFrontEnd::getDefaultReturns()
{
    static std::vector<Exp*> returns;
    if (returns.size() == 0) {
        returns.push_back(Location::regOf(24/*eax*/));
        returns.push_back(Location::regOf(25/*ecx*/));
        returns.push_back(Location::regOf(26/*edx*/));
        returns.push_back(Location::regOf(27/*ebx*/));
        returns.push_back(Location::regOf(28/*esp*/));
        returns.push_back(Location::regOf(29/*ebp*/));
        returns.push_back(Location::regOf(30/*esi*/));
        returns.push_back(Location::regOf(31/*edi*/));
        returns.push_back(Location::regOf(32/*st0*/));
        returns.push_back(Location::regOf(33/*st1*/));
        returns.push_back(Location::regOf(34/*st2*/));
        returns.push_back(Location::regOf(35/*st3*/));
        returns.push_back(Location::regOf(36/*st4*/));
        returns.push_back(Location::regOf(37/*st5*/));
        returns.push_back(Location::regOf(38/*st6*/));
        returns.push_back(Location::regOf(39/*st7*/));
        returns.push_back(new Terminal(opPC));
    }
    return returns;
}

void PentiumFrontEnd::processFloatCode(Cfg* pCfg)
{
    BB_IT it;
    for (PBB pBB = pCfg->getFirstBB(it); pBB; pBB = pCfg->getNextBB(it)) {
        std::list<RTL*>::iterator rit;
        Statement* st;

        // Loop through each RTL this BB
        std::list<RTL*>* BB_rtls = pBB->getRTLs();
        if (BB_rtls == 0) {
            // For example, incomplete BB
            return;
        }
        rit = BB_rtls->begin();
        while (rit != BB_rtls->end()) {
            for (int i=0; i < (*rit)->getNumStmt(); i++) {
                // Get the current Exp
                st = (*rit)->elementAt(i);
                if (st->isFpush()) {
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::tempOf(new Const("tmpD9")), 
                        Location::regOf(39)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(39), 
                        Location::regOf(38)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(38), 
                        Location::regOf(37)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(37), 
                        Location::regOf(36)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(36), 
                        Location::regOf(35)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(35), 
                        Location::regOf(34)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(34), 
                        Location::regOf(33)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(33), 
                        Location::regOf(32)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(32), 
                            Location::tempOf(new Const("tmpD9"))), i++);
                    // Remove the FPUSH
                    (*rit)->deleteStmt(i);
                    i--;
                    continue;
                }
                else if (st->isFpop()) {
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(
                            Location::tempOf(new Const("tmpD9"))), 
                        Location::regOf(32)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(32), 
                        Location::regOf(33)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(33), 
                        Location::regOf(34)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(34), 
                        Location::regOf(35)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(35), 
                        Location::regOf(36)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(36), 
                        Location::regOf(37)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(37), 
                        Location::regOf(38)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(38), 
                        Location::regOf(39)), i++);
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                        Location::regOf(39), 
                           Location::tempOf(new Const("tmpD9"))), i++);
                    // Remove the FPOP
                    (*rit)->deleteStmt(i);
                    i--;
                    continue;
                }
            }
            rit++;
        }
    }
}

/*==============================================================================
 * FUNCTION:      processFloatCode
 * OVERVIEW:      Process a basic block, and all its successors, for floating
 *                  point code. Remove FPUSH/FPOP, instead decrementing or
 *                  incrementing respectively the tos value to be used from
 *                  here down. Note: tos has to be a parameter, not a global,
 *                  to get the right value at any point in the call tree
 * PARAMETERS:    pBB: pointer to the current BB
 *                tos: reference to the value of the "top of stack" pointer
 *                currently. Starts at zero, and is decremented to 7 with
 *                the first load, so r[39] should be used first, then r[38]
 *		  etc. However, it is reset to 0 for calls, so that if a
 *                function returns a float, the it will always appear in r[32]
 * RETURNS:       <nothing>
 *============================================================================*/
void PentiumFrontEnd::processFloatCode(PBB pBB, int& tos, Cfg* pCfg)
{
    std::list<RTL*>::iterator rit;
    Statement* st;

    // Loop through each RTL this BB
    std::list<RTL*>* BB_rtls = pBB->getRTLs();
    if (BB_rtls == 0) {
        // For example, incomplete BB
        return;
    }
    rit = BB_rtls->begin();
    while (rit != BB_rtls->end()) {
        // Check for call.
        if ((*rit)->isCall()) {
            // Reset the "top of stack" index. If this is not done, then after
            // a sequence of calls to functions returning floats, the value will
            // appear to be returned in registers r[32], then r[33], etc.
            tos = 0;
        }
        if ((*rit)->getNumStmt() == 0) { rit++; continue; }
#if PROCESS_FNSTSW
        // Check for f(n)stsw
        if (isStoreFsw((*rit)->elementAt(0))) {
            // Check the register - at present we only handle AX
            Exp* lhs = ((Assign*)(*rit)->elementAt(0))->getLeft();
            Exp* ax = Location::regOf(0);
            assert(*lhs == *ax);
            delete ax;

            // Process it
            if (processStsw(rit, BB_rtls, pBB, pCfg)) {
                // If returned true, must abandon this BB.
                break;
            }
            // Else we have already skiped past the stsw, and/or any
            // instructions that replace it, so process rest of this BB
            continue;
        }
#endif
        for (int i=0; i < (*rit)->getNumStmt(); i++) {
            // Get the current Exp
            st = (*rit)->elementAt(i);
            if (!st->isFlagAssgn()) {
                // We are interested in either FPUSH/FPOP, or r[32..39]
                // appearing in either the left or right hand sides, or calls
                if (st->isFpush()) {
                    tos = (tos - 1) & 7;
                    // Remove the FPUSH
                    (*rit)->deleteStmt(i);
                    i--;            // Adjust the index
                    continue;
                }
                else if (st->isFpop()) {
                    tos = (tos + 1) & 7;
                    // Remove the FPOP
                    (*rit)->deleteStmt(i);
                    i--;            // Adjust the index
                    continue;
                }
                else if (st->isAssign()) {
                    Assign* asgn = (Assign*)st;
                    Exp* lhs = asgn->getLeft();
                    Exp* rhs = asgn->getRight();
                    if (tos != 0) {
                        // Substitute all occurrences of r[x] (where
                        // 32 <= x <= 39) with r[y] where
                        // y = 32 + (x + tos) & 7
                        bumpRegisterAll(lhs, 32, 39, tos, 7);
                        bumpRegisterAll(rhs, 32, 39, tos, 7);
                    }
                }
            }
            else {
                // st is a flagcall
                // We are interested in any register parameters in the range
                // 32 - 39
                Binary* cur;
                for (cur = (Binary*)st->getRight(); !cur->isNil();
                  cur = (Binary*)cur->getSubExp2()) {
// I dont understand why we want typed exps in the flag calls so much.
// If we're going to replace opSize with TypedExps then we need to do it
// for everything, not just the flag calls.. so that should be in the 
// sslparser.  If that is the case then we cant assume that opLists of
// flag calls will always contain TypedExps, so this code is wrong.
// - trent 9/6/2002
//                    TypedExp* te = (TypedExp*)cur->getSubExp1();
                    Exp* s = cur->getSubExp1();
                    if (s->isRegOfK()) {
                        Const* c = (Const*)((Unary*)s)->getSubExp1();
                        int K = c->getInt();        // Old register number
                        // Change to new register number, if in range
						if ((K >= 32) && (K <= 39))
                        	s->setSubExp1(new Const(32 + (K - 32 + tos & 7)));
                    }
                }
                        
            }
        }
        rit++;
    }
    pBB->setTraversed(true);

    // Now recurse to process my out edges, if not already processed
    const std::vector<PBB>& outs = pBB->getOutEdges();
    unsigned n;
    do {
        n = outs.size();
        for (unsigned o=0; o<n; o++) {
            PBB anOut = outs[o];
            if (!anOut->isTraversed()) {
                processFloatCode(anOut, tos, pCfg);
                if (outs.size() != n)
                    // During the processing, we have added or more likely
                    // deleted a BB, and the vector of out edges has changed.
                    // It's safe to just start the inner for loop again
                    break;
            }
        }
    } while (outs.size() != n);            
}


#if 0       // This monstrosity no longer needed. Sniff.
/*
// Finite state machine for recognising code handling floating point CCs
//
//            test_45 or          Start=0
//         ____and_45____________/ |  \  \______sahf____________
//        /                        |   \_____and_5__________    \     ___ 
//       [1]__________cmp_1___    and 44                    \    \   /   |jp
//cmp_40/||\\___dec_[10]      \    [2]__________         __[3]   [23]____|
//  /    | \\__je_    \cmp 40 [20]  |   \xor 40 \jne    /  / |    | \ 
// [4]  jne se    \    \       |\   je   [7]     \     /  /  |    |  \ 
// | \   |   \    |    [11]  jne \    \   | \    [5]  je se  |   jx   sx
// je se  \   \   | jae|  \sb  \  se   \ jne setne   /  /   jne   |    \ 
// |   \   \   \   \   |   \    \  \    \ |    \    /  /     |    |     \ 
//[5]  [6][14][13][26][12] [15][21][22]  [8]   [9] [21][18] [19] [24]   [25]
//JE   SE  JLE  SG JG  JG  SLE JGE  SL   JNE   SNE JGE SGE   JL  Many   Many
*/

/*==============================================================================
 * FUNCTION:      processStsw
 * OVERVIEW:      Process a stsw instruction
 * PARAMETERS:    rit: iterator to the current RTL (with the stsw in it)
 *                BB_rtls: list of RTLs for this BB
 *                pCfg: pointer to Cfg for this proc
 * NOTE:          parameter rit may well be modified (incrementing it past the
 *                  STSW, and any instructions replacing it
 * RETURNS:       True if the current BB is deleted (because 2 BBs were joined)
 *                  Also returns true on error, so abandon this BB
 *============================================================================*/
bool PentiumFrontEnd::processStsw(std::list<RTL*>::iterator& rit,
  std::list<RTL*>* BB_rtls, PBB pBB, Cfg* pCfg) {
    int state = 0;              // Start in state 0
    Location *ah = Location::regOf(AH);
    Unary *notZf = new Unary(opNot, new Terminal(opZF));
    Ternary *ahAt7 = new Ternary(opTern,
        Location::regOf(AH),
        new Const(7),
        new Const(7));
    // Keep a list of iterators representing 
    // RTLs (this BB) that can be removed
    std::list<std::list<RTL*>::iterator> liIt;
    liIt.push_front(rit);   // f(n)stsw can be removed
    // Scan the rest of the RTLs this BB
    std::list<RTL*>::iterator rit2 = rit;      // Don't disturb curr loops
    // Scan each RTL this BB
    for (rit2++; rit2 != BB_rtls->end(); rit2++) {
//std::cout << "State now " << std::dec << state << std::endl;
        // Get the first Exp; only interested in assigns
        if ((*rit2)->getNumStmt() == 0)
            continue;
        Statement* st = (*rit2)->elementAt(0);
        if (!st->isAssign()) continue;
        // May need pLHS and uAddr later when reconstructing this SET instr
        ADDRESS uAddr = (*rit2)->getAddress();
        Assign* asgn = (Assign*)st;
        Exp* lhs = asgn->getLeft();
        Exp* rhs = asgn->getRight();
        Exp* result;
        // Check if uses register ah, and assigns to either ah or a temp
        if ((lhs->search(ah, result) || lhs->isTemp()) &&
          rhs->search(ah, result)) {
            // Should be an AND or XOR instruction
            OPER op = rhs->getOper();
            if ((op == opBitAnd) || (op == opBitXor)) {
                Exp* e;
                e = ((Binary*)rhs)->getSubExp2();
                if (e->isIntConst()) {
                    if (op == opBitAnd) {
                        // Note: C1 is the "Z or O/U" bit, and is mostly unused.
                        // Some compilers include this bit (mask 0x04) in their
                        // masks, some don't. To catch them all properly, the
                        // mask below is ORed with 0x04. That way, the test
                        // for 0x45 also tests for 0x41, and so on
                        int mask = ((Const*)e)->getInt() | 0x04;
                        if (state == 0 && mask == 0x45) {
                            state = 1;
                            liIt.push_front(rit2);
                        }
                        else if (state == 0 && mask == 0x44) {
                            state = 2;
                            liIt.push_front(rit2);
                        }
                        else if (state == 0 && mask == 0x05) {
                            state = 3;
                            liIt.push_front(rit2);
                        }
                        else {
                            std::cerr << "Problem with AND: state is " << state
                              << ", mask is 0x" << std::hex << mask << "\n";
                            return true;
                        }
                    }
                    else {
                        // op == opBitXor
                        int mask = ((Const*)e)->getInt();
                        if (state == 2 && mask == 0x40) {
                            state = 7;
                            liIt.push_front(rit2);
                        }
                        else {
                            std::cerr << "Problem with XOR\n";
                            return true;
                        }
                    }
                }
            }
            else std::cout << "! Unexpected operator!\n";
        }
        // Or might be a compare or decrement: uses ah, assigns to
        // temp register
        else if (lhs->isTemp() && rhs->search(ah, result)) {
            // Might be a compare, i.e. subtract
            if (rhs->getOper() == opMinus) {
                Exp* e;
                e = ((Binary*)e)->getSubExp2();
                if (e->isIntConst()) {
                    int mask = ((Const*)e)->getInt();
                    if (state == 1 && mask == 0x40) {
                        state = 4;
                        liIt.push_front(rit2);
                    }
                    else if (state == 10 && mask == 0x40) {
                        state = 11;
                        liIt.push_front(rit2);
                    }
                    else if (state == 1 && mask == 1) {
                        state = 20;
                        liIt.push_front(rit2);
                    }
                    else {
                        std::cerr << "Problem with cmp\n";
                        return true;
                    }
                }
            }
            // Check for decrement; RHS of next RT will be r[12]{8} - 1
            else {
                if (isDecAh(*rit2)) {
                    if (state == 1) {
                        state = 10;
                        liIt.push_front(rit2);
                    }
                    else {
                        std::cerr << "Problem with decrement\n";
                        return true;
                    }
                }
            }
        }
        // Check for SETX, i.e. <exp> ? 1 : 0
        // i.e. ?: <exp> int 1 int 0
        else if (isSetX(st)) {
            if (state == 23) {
                state = 25;
                // Don't add the set instruction until after the instrs
                // leading up to here are deleted. Else have problems with
                // iterators
            }
            else {
                // Check the expression
                Exp* e;
                e = rhs->getSubExp1();
                if (e->getOper() == opZF) {
                    if (state == 4) state = 6;
                    else if (state == 1) state = 13;
                    else if (state == 3) state = 18;
                    else if (state == 20) state = 22;
                    else {
                        std::cerr << "Problem with SETE\n";
                        return true;
                    }
                }
                else if (e->getOper() == opCF) {
                    if (state == 11) state = 15;
                    else {
                        std::cerr << "Problem with SETB\n";
                        return true;
                    }
                }
                else if (*e == *notZf) {
                    if (state == 7) state = 9;
                    else {
                        std::cerr << "Problem with SETNE\n";
                        return true;
                    }
                }
            }
        }
        // Check for sahf instr, i.e.
        // r[12]@7:7
        else if (*rhs == *ahAt7) {
            if (state == 0) {
                state = 23;
                liIt.push_front(rit2);
            }
            else {
                std::cerr << "Problem with sahf\n";
                return true;
            }
        }
        // Check for "set" terminating states
        switch (state) {
            case 6: case 13: case 15:
            case 22: case 9: case 18: case 25:
            // Remove the set instruction and those leading up to it.
            // The left hand side of the set instruction (modrm) is still
            // in lhs. It will be needed to build the new set instr below
            // Ditto for uAddr.
            // Must decrement rit to the previous RTL
            // so the next increment will be to an RTL interleaved with
            // the deleted ones (if any; otherwise, the one after the set)
            // Also, when inserting a replacement SET instruction, the correct
            // place is after *rit.
            rit--;
            if (state == 25) {
                // Keep a copy of the LHS in a new SS. Otherwise, the same SS
                // will be part of two RTLs, so when they are destroyed, this
                // SS will get deleted twice, causing segfaults.
                lhs = lhs->clone();
            }
            // Keep assigning to rit. In the end, it will point to the next
            // RTL after the erased items
            rit = BB_rtls->erase(rit2);
            while (liIt.size()) {
                rit = BB_rtls->erase(liIt.front());
                liIt.erase(liIt.begin());
            }
        }
        switch (state) {
        case 6:
            // Emit a floating point "set if Z"
            emitSet(BB_rtls, rit, uAddr, lhs, new Terminal(opFZF));
            return false;
        case 13:
            // Emit a floating point "set if G"
            emitSet(BB_rtls, rit, uAddr, lhs, new Terminal(opFGF));
            return false;
        case 15:
            // Emit a floating poin2t "set if LE"
            emitSet(BB_rtls, rit, uAddr, lhs, new Binary(opOr,
              new Terminal(opFLF), new Terminal(opFZF)));
            return false;
        case 22:
            // Emit a floating point "set if L"
            emitSet(BB_rtls, rit, uAddr, lhs, new Terminal(opFLF));
            return false;
        case 9:
            // Emit a floating point "set if NE"
            emitSet(BB_rtls, rit, uAddr, lhs, new Unary(opNot,
              new Terminal(opFZF)));
            return false;
        case 18:
            // Emit a floating point "set if GE"
            emitSet(BB_rtls, rit, uAddr, lhs, new Binary(opOr,
              new Terminal(opFGF), new Terminal(opFZF)));
            return false;
        case 25:
            State25(lhs, rhs, BB_rtls, rit, uAddr);
            return false;
        }

    }           // End of for loop (for each remaining RTL this BB)
                            
    // Check the branch
    RTL* pJumpRtl = *--rit2;
    BranchStatement* pJump = (BranchStatement*)(pJumpRtl->getList().back());
    Exp* lhs = 0;
    Exp* rhs = 0;
    ADDRESS uAddr;
    PBB pBBnext = 0;
    bool bJoin = false;         // Set if need to join BBs
    if (state == 23) {
        RTL* pRtl;
        Statement* st;
        if (pJump->getCond() == BRANCH_JPAR) {
            // Check the 2nd out edge. It should be the false case, and
            // should point to either a BB with just a branch in it (a
            // TWOWAY BB) or one starting with a SET instruction
            const std::vector<PBB>& v = pBB->getOutEdges();
            pBBnext = v[1];
            if ((pBBnext->getType() == TWOWAY) &&
              (pBBnext->getRTLs()->size() == 1)) {
                BranchStatement* pJ = (BranchStatement*)pBBnext->getRTLs()->
                  front()->getList().back();
                // Make it a floating branch
                pJ->setFloat(true);
                // Make it a signed branch
                pJ->makeSigned();
                // Special state, so DO delete the JP
                state = -24;
                bJoin = true;       // Need to join since will delete JP
            }
            else if (
              (pRtl = pBBnext->getRTLs()->front(),
              st = (pRtl->elementAt(0)),
              isAssignFromTern(st))) {
                lhs = ((Assign*)st)->getLeft();
                uAddr = pRtl->getAddress();
                state = 25;
                // Actually generate the set instruction later, after the
                // instruction leading up to it are deleted. Otherwise have
                // problems with iterators
                bJoin = true;       // Need to join since will delete JP
            }
            else {
                std::cerr << "Problem with JP at " << std::hex;
                std::cerr << pJumpRtl->getAddress();
                std::cerr << ".\nDoes not fall through to branch or set at ";
                std::cerr << pBBnext->getLowAddr() << std::endl;
                return true;
            }
        }                   
        else {              // Branch, but not a JP (Jump if parity)
            // Just need to change the branch to a float type
            pJump->setFloat(true);
            // and also make it a signed branch
            pJump->makeSigned();
            state = 24;
        }
    }       // if state == 23
    else if (pJump->getCond() == BRANCH_JE)
    {
        switch (state) {
            case 1: state = 26; break;
            case 2: state = 8;  break;
            case 3: state = 21; break;
            case 4: state = 5;  break;
            default:
                std::cerr << "Problem with JE: state is " << std::dec << state
                  << "\n";
                return true;
        }
    }
    else if (pJump->getCond() == BRANCH_JNE) {
        if (state == 1) state = 14;
        else if (state == 7) state = 8;
        else if (state == 3) state = 19;
        else if (state == 20) state = 21;
        else if (state == 2) state = 5;
        else {
            std::cerr << "Problem with JNE: state is " << state << "\n";
            std::cerr << "pJump to " << std::hex << pJump->getFixedDest() <<
              "\n";
            return true;
        }
    }
    else if (pJump->getCond() == BRANCH_JUGE) {
        if (state == 11) state = 12;
        else {
            std::cerr << "Problem with JAE";
            return true;
        }
    }
    else {
        std::cerr << "Problem with branch\n";
        return true;
    }

    Exp* pDest;
    switch (state) {
    case 5: case 14: case 12:
    case 21: case 8: case 19:
    case 24: case -24: case 25:
    case 26:
        // We can remove the branch and the instructions leading up to it
        // (exception: state 24, don't remove the branch).
        // We must decrement rit now, so that it points to a (hopefully)
        // valid RTL, and when incremented next time around the while
        // loop, it will process any instructions that were interspersed
        // with the ones that will be deleted.
        rit--;
        uAddr = pJumpRtl->getAddress();     // Save addr of branch
        pDest = pJump->getDest();           // Save dest of branch
        if (state == 25)
            // As before, keep a copy of the LHS in a new exp.
            lhs = lhs->clone();
        if (state != 24)
            rit = BB_rtls->erase(rit2);
        while (liIt.size()) {
            rit = BB_rtls->erase(liIt.front());
            liIt.erase(liIt.begin());
        }
        break;
    default:
        std::cerr << "Error: end of BB in state " << std::dec;
        std::cerr << state << std::endl;
        return true;
    }
    // Add a new branch, with the appropriate parameters
    BranchStatement* newJump;
    std::list<Statement*>* ls;
    switch (state) {
    case 5:         // Jump if equals
        newJump = new BranchStatement;
        newJump->setDest(pDest);
        newJump->setCondType(BRANCH_JE, true);
        ls = new std::list<Statement*>;
        ls->push_back(newJump);
        BB_rtls->push_back(new RTL(uAddr, ls));
        break;
    case 14:        // Jump if less or equals
        newJump = new BranchStatement;
        newJump->setDest(pDest);
        newJump->setCondType(BRANCH_JSLE, true);
        ls = new std::list<Statement*>;
        ls->push_back(newJump);
        BB_rtls->push_back(new RTL(uAddr, ls));
        break;
    case 12:        // Jump if greater
    case 26:        // Also jump if greater
        newJump = new BranchStatement;
        newJump->setDest(pDest);
        newJump->setCondType(BRANCH_JSG, true);
        ls = new std::list<Statement*>;
        ls->push_back(newJump);
        BB_rtls->push_back(new RTL(uAddr, ls));
        break;
    case 21:        // Jump if greater or equals
        newJump = new BranchStatement;
        newJump->setDest(pDest);
        newJump->setCondType(BRANCH_JSGE, true);
        ls = new std::list<Statement*>;
        ls->push_back(newJump);
        BB_rtls->push_back(new RTL(uAddr, ls));
        break;
    case 8:         // Jump if not equals
        newJump = new BranchStatement;
        newJump->setDest(pDest);
        newJump->setCondType(BRANCH_JNE, true);
        ls = new std::list<Statement*>;
        ls->push_back(newJump);
        BB_rtls->push_back(new RTL(uAddr, ls));
        break;
    case 19:        // Jump if less
        newJump = new BranchStatement;
        newJump->setDest(pDest);
        newJump->setCondType(BRANCH_JSL, true);
        ls = new std::list<Statement*>;
        ls->push_back(newJump);
        BB_rtls->push_back(new RTL(uAddr, ls));
        break;
    case 25:
        State25(lhs, rhs, BB_rtls, rit, uAddr);
        break;
    }

    if (bJoin) {
        // Need to join BBs, because we have deleted a branch
        // The RTLs for the first are appended to those of the second.
        // Since pBB could well have RTs that have already been adjusted
        // for floating point, and pBBnext probably is just a branch, it's
        // important to get the parameters this way around
        pCfg->joinBB(pBBnext, pBB);
    }

//std::cout << "Return in state " << std::dec << state << std::endl;
    return bJoin;           // If joined, abandon this BB
}
#endif

// Emit Rtl of the form *8* lhs = [cond ? 1 : 0]
// Insert before rit
void PentiumFrontEnd::emitSet(std::list<RTL*>* BB_rtls, std::list<RTL*>::iterator& rit,
  ADDRESS uAddr, Exp* lhs, Exp* cond) {

    Statement* asgn = new Assign(
        lhs,
        new Ternary(opTern,
            cond,
            new Const(1),
            new Const(0)));
    RTL* pRtl = new RTL(uAddr);
    pRtl->appendStmt(asgn);
//std::cout << "Emit "; pRtl->print(); std::cout << std::endl;
    // Insert the new RTL before rit
    BB_rtls->insert(rit, pRtl);
}

void PentiumFrontEnd::State25(Exp* lhs, Exp* rhs, std::list<RTL*>* BB_rtls,
  std::list<RTL*>::iterator& rit, ADDRESS uAddr) {
    Binary cfOrZf(opOr, new Terminal(opCF), new Terminal(opZF));
    Unary notZf(opNot, new Terminal(opZF));
    Unary notCf(opNot, new Terminal(opCF));
    Binary notCfAndNotZf(opAnd,
        new Unary(opNot, new Terminal(opCF)),
        new Unary(opNot, new Terminal(opZF)));
    // Assume this is a set instruction
    Exp* exp;
    exp = rhs->getSubExp1();
    if (exp->getOper() == opCF) {
        // Emit a "floating point set if L
        emitSet(BB_rtls, rit, uAddr, lhs, new Terminal(opFLF));
    }
    else if (exp->getOper() == opZF) {
        // Emit a floating point "set if Z"
        emitSet(BB_rtls, rit, uAddr, lhs, new Terminal(opFZF));
    }
    else if (*exp == notCf) {
        // Emit a floating point "set if GE"
        emitSet(BB_rtls, rit, uAddr, lhs, new Binary(opOr,
            new Terminal(opFGF), new Terminal(opFZF)));
    }
    else if (*exp == notZf) {
        // Emit a floating point "set if NZ"
        emitSet(BB_rtls, rit, uAddr, lhs, new Unary(opNot,
          new Terminal(opFZF)));
    }
    else if (*exp == cfOrZf) {
        // Emit a floating poin2t "set if LE"
        emitSet(BB_rtls, rit, uAddr, lhs, new Binary(opOr,
          new Terminal(opFLF), new Terminal(opFZF)));
    }
    else if (*exp == notCfAndNotZf) {
        // Emit a floating point "set if G"
        emitSet(BB_rtls, rit, uAddr, lhs, new Terminal(opFGF));
    }
    else {
        std::cerr << "Problem with STSW/SET\n";
        return;
    }
}

/*==============================================================================
 * FUNCTION:        helperFunc
 * OVERVIEW:        Checks for pentium specific helper functions like __xtol
 *                      which have specific sematics.
 * NOTE:            This needs to be handled in a resourcable way.
 * PARAMETERS:      dest - the native destination of this call
 *                  addr - the native address of this call instruction
 *                  lrtl - pointer to a list of RTL pointers for this BB
 * RETURNS:         true if a helper function is converted; false otherwise
 *============================================================================*/
bool PentiumFrontEnd::helperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl)
{
    if (dest == NO_ADDRESS) return false;
    const char* p = pBF->SymbolByAddress(dest);
    if (p == NULL) return false;
    std::string name(p);
    // I believe that __xtol is for gcc, _ftol for earlier MSVC compilers,
    // _ftol2 for MSVC V7
    if (name == "__xtol" || name == "_ftol" || name == "_ftol2") {
        // This appears to pop the top of stack, and converts the result to
        // a 64 bit integer in edx:eax. Truncates towards zero
        // r[tmpl] = ftoi(80, 64, r[32])
        // r[24] = trunc(64, 32, r[tmpl])
        // r[26] = r[tmpl] >> 32
        Statement* a = new Assign(new IntegerType(64),
            Location::tempOf(new Const("tmpl")),
            new Ternary(opFtoi, new Const(64), new Const(32),
                Location::regOf(32)));
        RTL* pRtl = new RTL(addr);
        pRtl->appendStmt(a);
        a = new Assign(
            Location::regOf(24),
            new Ternary(opTruncs, new Const(64), new Const(32),
                Location::tempOf(new Const("tmpl"))));
        pRtl->appendStmt(a);
        a = new Assign(
            Location::regOf(26),
            new Binary(opShiftR,
                Location::tempOf(new Const("tmpl")),
                new Const(32)));
        pRtl->appendStmt(a);
        // Append this RTL to the list of RTLs for this BB
        lrtl->push_back(pRtl);
        // Return true, so the caller knows not to create a HLCall
        return true;

    } else {
        // Will be other cases in future
    }
    return false;
}

/*==============================================================================
 * FUNCTION:      construct
 * OVERVIEW:      Construct a new instance of PentiumFrontEnd
 * PARAMETERS:    Same as the FrontEnd constructor, except decoder is **
 * RETURNS:       <nothing>
 *============================================================================*/
#ifdef DYNAMIC
extern "C" {
    PentiumFrontEnd* construct(Prog *prog, NJMCDecoder** decoder) {
        PentiumFrontEnd *fe = new PentiumFrontEnd(prog);
        *decoder = fe->getDecoder();
        return fe;
    }
}
#endif

/*==============================================================================
 * FUNCTION:      PentiumFrontEnd::PentiumFrontEnd
 * OVERVIEW:      PentiumFrontEnd constructor
 * NOTE:          Seems to be necessary to put this here; forces the vtable
 *                  entries to point to this dynamic linked library
 * PARAMETERS:    Same as the FrontEnd constructor
 * RETURNS:       <N/A>
 *============================================================================*/
PentiumFrontEnd::PentiumFrontEnd(BinaryFile *pBF)
  : FrontEnd(pBF), idPF(-1)
{
	decoder = new PentiumDecoder();
/*	for (std::map<int, Register, std::less<int> >::iterator it = prog->RTLDict.DetRegMap.begin(); 
		 it != prog->RTLDict.DetRegMap.end(); it++) {
		int i = (*it).first;
		Register &r = (*it).second;
		if (!strcmp(r.g_name(), "%esp"))
			prog->symbols[std::string(r.g_name())] = new TypedExp(Type(DATA_ADDRESS), Location:;regOf(i));
		else
			prog->symbols[std::string(r.g_name())] = new TypedExp(r.g_type(), Location::regOf(i));
	} */
}

// destructor
PentiumFrontEnd::~PentiumFrontEnd()
{
}

/*==============================================================================
 * FUNCTION:    GetMainEntryPoint
 * OVERVIEW:    Locate the starting address of "main" in the code section
 * PARAMETERS:  None
 * RETURNS:     Native pointer if found; NO_ADDRESS if not
 *============================================================================*/
ADDRESS PentiumFrontEnd::getMainEntryPoint( bool &gotMain ) 
{
	gotMain = true;
    ADDRESS start = pBF->GetMainEntryPoint();
    if( start != NO_ADDRESS ) return start;

	gotMain = false;
    start = pBF->GetEntryPoint();
    if( start == NO_ADDRESS ) return NO_ADDRESS;
    
    int instCount = 100;
    int conseq = 0;
    ADDRESS addr = start;
        
    // Look for 3 calls in a row in the first 100 instructions, with
    // no other instructions between them. This is the "windows" pattern
    // Another windows pattern: call to GetModuleHandleA followed by
    // a push of eax and then the call to main.
    // Or a call to __libc_start_main
    ADDRESS dest;
    do {
        DecodeResult inst = decodeInstruction(addr);
        if (inst.rtl == NULL)
            // Must have gotten out of step
            break;
        CallStatement* cs = NULL;
        if (inst.rtl->getList().size())
            cs = (CallStatement*)(inst.rtl->getList().back());
        if (cs && cs->getKind() == STMT_CALL &&
            cs->getDest()->getOper() == opMemOf &&
            cs->getDest()->getSubExp1()->getOper() == opIntConst &&
            pBF->IsDynamicLinkedProcPointer(((Const*)cs->getDest()
                      ->getSubExp1())->getAddr()) &&
            !strcmp(pBF->GetDynamicProcName(((Const*)cs->getDest()
                        ->getSubExp1())->getAddr()), "GetModuleHandleA")) {
#if 0
            std::cerr << "consider " << std::hex << addr << " " <<
                pBF->GetDynamicProcName(((Const*)cs->getDest()->getSubExp1())
                      ->getAddr()) << std::endl;
#endif
            int oNumBytes = inst.numBytes;
            inst = decodeInstruction(addr + oNumBytes);
            if (inst.valid && inst.rtl->getNumStmt() == 2) {
                Assign* a = dynamic_cast<Assign*>
                                (inst.rtl->elementAt(1));
                if (a && *a->getRight() == *Location::regOf(24)) {
#if 0
                    std::cerr << "is followed by push eax.. "
                              << "good" << std::endl;
#endif
                    inst = decodeInstruction(addr + oNumBytes + 
                                          inst.numBytes);
                    if (inst.rtl->getList().size()) {
                        CallStatement *toMain = 
                            dynamic_cast<CallStatement*>(inst.rtl
                                                    ->getList().back());
                        if (toMain && toMain->getFixedDest() 
                                                    != NO_ADDRESS) {
                            pBF->AddSymbol(toMain->getFixedDest(), 
                                           "WinMain");
                            gotMain = true;
                            return toMain->getFixedDest();
                        }
                    }
                }
            }
        }
        if ((cs && cs->getKind() == STMT_CALL) &&
          ((dest = (cs->getFixedDest())) != NO_ADDRESS)) {
            if (++conseq == 3 && 0) { // this isn't working
                // Success. Return the target of the last call
	        gotMain = true;
                return cs->getFixedDest();
            }
            if (pBF->SymbolByAddress(dest) &&
                strcmp(pBF->SymbolByAddress(dest), "__libc_start_main") == 0) {
                // This is a gcc 3 pattern. The first parameter will be
                // a pointer to main. Assume it's the 5 byte push
                // immediately preceeding this instruction
                inst = decodeInstruction(addr-5);
                assert(inst.valid);
                assert(inst.rtl->getNumStmt() == 2);
                Assign* a = (Assign*) inst.rtl->elementAt(1);
                Exp* rhs = a->getRight();
                assert(rhs->isIntConst());
                gotMain = true;
                return (ADDRESS)((Const*)rhs)->getInt();
            }
        }
        else 
            conseq = 0;         // Must be consequitive
        GotoStatement* gs = (GotoStatement*)cs;
        if (gs && gs->getKind() == STMT_GOTO)
            // Example: Borland often starts with a branch around some debug
            // info
            addr = gs->getFixedDest();
        else
            addr += inst.numBytes;
    } while (--instCount);
#if 0       // Was for finding main in DOS 286 programs
        // Try another pattern; this one is for DOS programs. In the first
        // 120 instructions, look for 3 or more pushes, then a call. These
        // will be setting up envp, argv, and argc
        instCount = 120; addr = start; conseq = 0;
        do {
            DecodeResult inst = decodeInstruction(addr);
            if ((conseq >= 3) && (inst.rtl->getKind() == CALL_HRTL) &&
                ((HLCall*)inst.rtl)->getFixedDest() != NO_ADDRESS) {
                    // Success. Return the target of the call					
                    return ((HLCall*)inst.rtl)->getFixedDest();
            }
            if (is286Push(inst.rtl))
                conseq++;
            else
                conseq = 0;
            addr += inst.numBytes;
        } while (--instCount);
#endif

    // Last chance check: look for _main (e.g. Borland programs)
    ADDRESS umain = pBF->GetAddressByName("_main");
    if (umain != NO_ADDRESS) return umain;

    // Not ideal; we must return start
    std::cerr << "main function not found\n";
    return start;
}

void toBranches(ADDRESS a, bool lastRtl, Cfg* cfg, RTL* rtl, PBB bb, BB_IT& it)
{
    BranchStatement* br1 = new BranchStatement;
    assert(rtl->getList().size() >= 4);     // They vary; at least 5 or 6
    Statement* s1 = *rtl->getList().begin();
    Statement* s6 = *(--rtl->getList().end());
    br1->setCondExpr(s1->getRight());
    br1->setDest(a+2);
    BranchStatement* br2 = new BranchStatement;
    br2->setCondExpr(s6->getRight());
    br2->setDest(a);
    cfg->splitForBranch(bb, rtl, br1, br2, it);
}

void PentiumFrontEnd::processStringInst(UserProc* proc) {
    Cfg::iterator it;
    Cfg* cfg = proc->getCFG();
    // For each BB this proc
    for (it = cfg->begin(); it != cfg->end(); /* no increment! */) {
        bool noinc = false;
        PBB bb = *it;
        std::list<RTL*> *rtls = bb->getRTLs();
        ADDRESS prev, addr = 0;
        bool lastRtl = true;
        // For each RTL this BB
        for (std::list<RTL*>::iterator rit = rtls->begin();
              rit != rtls->end(); rit++) {
            RTL *rtl = *rit;
            prev = addr;
            addr = rtl->getAddress();
            if (rtl->getList().size()) {
                Statement* firstStmt = *rtl->getList().begin();
                if (firstStmt->isAssign()) {
                    Exp* lhs = firstStmt->getLeft();
                    if (lhs->isMachFtr()) {
                        Const* sub = (Const*)((Unary*)lhs)->getSubExp1();
                        char* str = sub->getStr();
                        if (strncmp(str, "%SKIP", 5) == 0) {
                            toBranches(addr, lastRtl, cfg, rtl, bb, it);
                            noinc = true;       // toBranches inc's it
                            // Abandon this BB; if there are other string instr
                            // this BB, they will appear in new BBs near the
                            // end of the list
                            break;
                        }
                        else
                            LOG << "Unhandled machine feature " << str << "\n";
                    }
                }
            }
            lastRtl = false;
        }
        if (!noinc) it++;
    }
}

void PentiumFrontEnd::processOverlapped(UserProc* proc) {
    if (Boomerang::get()->overlapped == false) return;
    // For each statement, we are looking for assignments to registers in
    //   these ranges:
    // eax - ebx (24-27) (eax, ecx, edx, ebx)
    //  ax -  bx ( 0- 3) ( ax,  cx,  dx,  bx)
    //  al -  bl ( 8-11) ( al,  cl,  dl,  bl)
    //  ah -  bh (12-15) ( ah,  ch,  dh,  bh)
    StatementList stmts;
    proc->getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Assignment* as = dynamic_cast<Assignment*>(*it);
        if (as == NULL) continue;
        Exp* lhs = as->getLeft();
        if (!lhs->isRegOf()) continue;
        Const* c = (Const*)((Location*)lhs)->getSubExp1();
        assert(c->isIntConst());
        int r = c->getInt();
        int off = r&3;        // Offset into the array of 4 registers
        Assign* a;
        switch (r) {
            case 24: case 25: case 26: case 27:
            //  eax      ecx      edx      ebx
            // Emit *16* r<off> := trunc(32, 16, r<24+off>)
            a = new Assign(
                new IntegerType(16),
                Location::regOf(off),
                new Ternary(opTruncu,
                    new Const(32),
                    new Const(16),
                    Location::regOf(24+off)));
            proc->insertStatementAfter(as, a);

            // Emit *8* r<8+off> := trunc(32, 8, r<24+off>)
            a = new Assign(
                new IntegerType(8),
                Location::regOf(8+off),
                new Ternary(opTruncu,
                    new Const(32),
                    new Const(8),
                    Location::regOf(24+off)));
            proc->insertStatementAfter(as, a);

            // Emit *8* r<12+off> := r<24+off>@[15:8]
            a = new Assign(
                new IntegerType(8),
                Location::regOf(12+off),
                new Ternary(opAt,
                    Location::regOf(24+off),
                    new Const(15),
                    new Const(8)));
            proc->insertStatementAfter(as, a);
            break;

            case 0: case 1: case 2: case 3:
            //  ax      cx      dx      bx
            // Emit *32* r<24+off> := r<24+off> & 0xFFFF0000
            //      *32* r<24+off> := r<24+off> | zfill(16, 32, r<off>)
            // Note: emit backwards
            a = new Assign(
                new IntegerType(32),
                Location::regOf(24+off),
                new Binary(opBitOr,
                    Location::regOf(24+off),
                    new Ternary(opZfill,
                        new Const(16),
                        new Const(32),
                        Location::regOf(off))));
            proc->insertStatementAfter(as, a);
            a = new Assign(
                new IntegerType(32),
                Location::regOf(24+off),
                new Binary(opBitAnd,
                    Location::regOf(24+off),
                    new Const(0xFFFF0000)));
            proc->insertStatementAfter(as, a);
            
            // Emit *8* r<8+off> := trunc(16, 8, r<off>)
            a = new Assign(
                new IntegerType(8),
                Location::regOf(off),
                new Ternary(opTruncu,
                    new Const(16),
                    new Const(8),
                    Location::regOf(24+off)));
            proc->insertStatementAfter(as, a);

            // Emit *8* r<12+off> := r<off>@[15:8]
            a = new Assign(
                new IntegerType(8),
                Location::regOf(12+off),
                new Ternary(opAt,
                    Location::regOf(off),
                    new Const(15),
                    new Const(8)));
            proc->insertStatementAfter(as, a);
            break;


            case 8: case 9: case 10: case 11:
            //  al      cl       dl       bl
            // Emit *32* r<24+off> := r<24+off> & 0xFFFF0000
            //      *32* r<24+off> := r<24+off> | zfill(8, 32, r<8+off>)
            a = new Assign(
                new IntegerType(32),
                Location::regOf(24+off),
                new Binary(opBitOr,
                    Location::regOf(24+off),
                    new Ternary(opZfill,
                        new Const(8),
                        new Const(32),
                        Location::regOf(8+off))));
            proc->insertStatementAfter(as, a);
            a = new Assign(
                new IntegerType(32),
                Location::regOf(24+off),
                new Binary(opBitAnd,
                    Location::regOf(24+off),
                    new Const(0xFFFF0000)));
            proc->insertStatementAfter(as, a);

            // Emit *16* r<off> := r<off> & 0xFF00
            //      *16* r<off> := r<off> | zfill(8, 16, r<8+off>)
            a = new Assign(
                new IntegerType(16),
                Location::regOf(off),
                new Binary(opBitOr,
                    Location::regOf(off),
                    new Ternary(opZfill,
                        new Const(8),
                        new Const(16),
                        Location::regOf(8+off))));
            proc->insertStatementAfter(as, a);
            a = new Assign(
                new IntegerType(16),
                Location::regOf(off),
                new Binary(opBitAnd,
                    Location::regOf(off),
                    new Const(0xFF00)));
            proc->insertStatementAfter(as, a);
            break;

            case 12: case 13: case 14: case 15:
            //   ah       ch       dh       bh
            // Emit *32* r<24+off> := r<24+off> & 0xFFFF00FF
            //      *32* r<24+off> := r<24+off> | r<12+off> << 8
            a = new Assign(
                new IntegerType(32),
                Location::regOf(24+off),
                new Binary(opBitOr,
                    Location::regOf(24+off),
                    new Binary(opShiftL,
                        Location::regOf(12+off),
                        new Const(8))));
            proc->insertStatementAfter(as, a);
            a = new Assign(
                new IntegerType(32),
                Location::regOf(24+off),
                new Binary(opBitAnd,
                    Location::regOf(24+off),
                    new Const(0xFFFF00FF)));
            proc->insertStatementAfter(as, a);

            // Emit *16* r<off> := r<off> & 0x00FF
            //      *16* r<off> := r<off> | r<12+off> << 8
            a = new Assign(
                new IntegerType(16),
                Location::regOf(off),
                new Binary(opBitOr,
                    Location::regOf(off),
                    new Binary(opShiftL,
                        Location::regOf(12+off),
                        new Const(8))));
            proc->insertStatementAfter(as, a);
            a = new Assign(
                new IntegerType(16),
                Location::regOf(off),
                new Binary(opBitAnd,
                    Location::regOf(off),
                    new Const(0x00FF)));
            proc->insertStatementAfter(as, a);
            break;
        }
    }
}
