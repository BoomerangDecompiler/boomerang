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
 * OVERVIEW:   This file contains routines to manage the decoding of pentium instructions and the instantiation to RTLs.
 *               These functions replace frontend.cpp for decoding pentium instructions.
 *============================================================================*/

/*
 * $Revision$    // 1.51.2.3
 *
 * 21 Oct 98 - Mike: converted from frontsparc.cc
 * 21 May 02 - Mike: Mods for boomerang
 * 27 Nov 02 - Mike: Fixed a bug in the floating point fixup code, which was screwing up registers in flag calls
 * 30 Sep 03 - Mike: processFloatCode ORs mask with 0x04 for compilers that ignore the C1 status bit (e.g. MSVC).
 *                Also more JE cases
 * 04 Aug 04 - Mike: Quick and dirty hack for overlapped registers (X86 only)
 * 31 Jul 06 - Tamlin: Fixed overlapped register processing for esi/edi/ebp plus a bug in ah/bh/ch/dh processing
*/

#include <assert.h>
#include <cstring>
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
#include "prog.h"            // For findProc()
#include "BinaryFile.h"        // For SymbolByAddress()
#include "boomerang.h"
#include "log.h"

/*==============================================================================
 * Forward declarations.
 *============================================================================*/

#define FSW 40                // Numeric registers
#define AH 12

/*==============================================================================
 * FUNCTION:      isStoreFsw
 * OVERVIEW:      Return true if the given Statement is an assignment that stores the FSW (Floating point Status Word)
 *                    reg
 * PARAMETERS:      s - Ptr to the given Statement
 * RETURNS:          True if it is
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
 * PARAMETERS:      r - Ptr to the given RTL
 * RETURNS:          True if it is
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
 * PARAMETERS:      s - Ptr to the given Statement
 * RETURNS:          True if it is
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
 * OVERVIEW:      Return true if the given Statement is an expression whose RHS is a ?: ternary
 * PARAMETERS:      e - Ptr to the given Statement
 * RETURNS:          True if it is
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
 *                      r[ int x] where min <= x <= max, and replaces it with
 *                      r[ int y] where y = min + (x - min + delta & mask)
 * PARAMETERS:        e: Expression to modify
 *                    min, max: minimum and maximum register numbers before any change is considered
 *                    delta: amount to bump up the register number by
 *                    mask: see above
 * APPLICATION:        Used to "flatten" stack floating point arithmetic (e.g. Pentium floating point code)
 *                      If registers are not replaced "all at once" like this, there can be subtle errors from
 *                      re-replacing already replaced registers
 * RETURNS:            Nothing
 *============================================================================*/
void PentiumFrontEnd::bumpRegisterAll(Exp* e, int min, int max, int delta, int mask) {
    std::list<Exp**> li;
    std::list<Exp**>::iterator it;
    Exp* exp = e;
    // Use doSearch, which is normally an internal method of Exp, to avoid problems of replacing the wrong
    // subexpression (in some odd cases)
    Exp::doSearch(Location::regOf(new Terminal(opWild)), exp, li, false);
    for (it = li.begin(); it != li.end(); it++) {
        int reg = ((Const*)((Unary*)**it)->getSubExp1())->getInt();
        if ((min <= reg) && (reg <= max)) {
            // Replace the K in r[ K] with a new K
            // **it is a reg[K]
            Const* K = (Const*)((Unary*)**it)->getSubExp1();
            K->setInt(min + ((reg - min + delta) & mask));
        }
    }
}
/*==============================================================================
 * FUNCTION:      PentiumFrontEnd::processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:      address - the address at which the procedure starts
 *                  pProc - the procedure object
 *                  os - output stream for rtl output
 *                  frag - true if decoding only a fragment of the proc
 *                  spec - true if a speculative decode
 * RETURNS:          True if successful decode
 *============================================================================*/
bool PentiumFrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os, bool frag /* = false */,
                                  bool spec /* = false */) {

    // Call the base class to do most of the work
    if (!FrontEnd::processProc(uAddr, pProc, os, frag, spec))
        return false;

    // Need a post-cfg pass to remove the FPUSH and FPOP instructions, and to transform various code after floating
    // point compares to generate floating point branches.
    // processFloatCode() will recurse to process its out-edge BBs (if not already processed)
    Cfg* pCfg = pProc->getCFG();
    pCfg->unTraverse();            // Reset all the "traversed" flags (needed soon)
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
                                                  Location::tempOf(new Const(const_cast<char *>("tmpD9"))),
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
                                                  Location::tempOf(new Const(const_cast<char *>("tmpD9")))), i++);
                    // Remove the FPUSH
                    (*rit)->deleteStmt(i);
                    i--;
                    continue;
                }
                else if (st->isFpop()) {
                    (*rit)->insertStmt(new Assign(new FloatType(80),
                                                  Location::tempOf(new Const(const_cast<char *>("tmpD9"))),
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
                                                  Location::tempOf(new Const(const_cast<char *>("tmpD9")))), i++);
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
 * OVERVIEW:      Process a basic block, and all its successors, for floating point code.
 *                    Remove FPUSH/FPOP, instead decrementing or incrementing respectively the tos value to be used from
 *                    here down. Note: tos has to be a parameter, not a global, to get the right value at any point in
 *                    the call tree
 * PARAMETERS:      pBB: pointer to the current BB
 *                  tos: reference to the value of the "top of stack" pointer currently. Starts at zero, and is
 *                    decremented to 7 with the first load, so r[39] should be used first, then r[38] etc. However, it is
 *                    reset to 0 for calls, so that if a function returns a float, then it will always appear in r[32]
 * RETURNS:          <nothing>
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
            // Reset the "top of stack" index. If this is not done, then after a sequence of calls to functions
            // returning floats, the value will appear to be returned in registers r[32], then r[33], etc.
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
            // Else we have already skiped past the stsw, and/or any instructions that replace it, so process rest of
            // this BB
            continue;
        }
#endif
        for (int i=0; i < (*rit)->getNumStmt(); i++) {
            // Get the current Exp
            st = (*rit)->elementAt(i);
            if (!st->isFlagAssgn()) {
                // We are interested in either FPUSH/FPOP, or r[32..39] appearing in either the left or right hand
                // sides, or calls
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
                        // Substitute all occurrences of r[x] (where 32 <= x <= 39) with r[y] where
                        // y = 32 + (x + tos) & 7
                        bumpRegisterAll(lhs, 32, 39, tos, 7);
                        bumpRegisterAll(rhs, 32, 39, tos, 7);
                    }
                }
            }
            else {
                // st is a flagcall
                // We are interested in any register parameters in the range 32 - 39
                Binary* cur;
                for (cur = (Binary*)((Assign*)st)->getRight(); !cur->isNil(); cur = (Binary*)cur->getSubExp2()) {
                    // I dont understand why we want typed exps in the flag calls so much. If we're going to replace opSize with TypedExps
                    // then we need to do it for everything, not just the flag calls.. so that should be in the sslparser.  If that is the
                    // case then we cant assume that opLists of flag calls will always contain TypedExps, so this code is wrong.
                    // - trent 9/6/2002
                    //                    TypedExp* te = (TypedExp*)cur->getSubExp1();
                    Exp* s = cur->getSubExp1();
                    if (s->isRegOfK()) {
                        Const* c = (Const*)((Unary*)s)->getSubExp1();
                        int K = c->getInt();        // Old register number
                        // Change to new register number, if in range
                        if ((K >= 32) && (K <= 39))
                            s->setSubExp1(new Const(32 + ((K - 32 + tos) & 7)));
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
                    // During the processing, we have added or more likely deleted a BB, and the vector of out edges
                    // has changed.  It's safe to just start the inner for loop again
                    break;
            }
        }
    } while (outs.size() != n);
}



// Emit Rtl of the form *8* lhs = [cond ? 1 : 0]
// Insert before rit
void PentiumFrontEnd::emitSet(std::list<RTL*>* BB_rtls, std::list<RTL*>::iterator& rit, ADDRESS uAddr, Exp* lhs,
                              Exp* cond) {

    Statement* asgn = new Assign(
                          lhs,
                          new Ternary(opTern,
                                      cond,
                                      new Const(1),
                                      new Const(0)));
    RTL* pRtl = new RTL(uAddr);
    pRtl->appendStmt(asgn);
    //    std::cout << "Emit "; pRtl->print(); std::cout << std::endl;
    // Insert the new RTL before rit
    BB_rtls->insert(rit, pRtl);
}

/*==============================================================================
 * FUNCTION:        helperFunc
 * OVERVIEW:        Checks for pentium specific helper functions like __xtol which have specific sematics.
 * NOTE:            This needs to be handled in a resourcable way.
 * PARAMETERS:        dest - the native destination of this call
 *                    addr - the native address of this call instruction
 *                    lrtl - pointer to a list of RTL pointers for this BB
 * RETURNS:            true if a helper function is converted; false otherwise
 *============================================================================*/
bool PentiumFrontEnd::helperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl)
{
    if (dest == NO_ADDRESS) return false;

    const char* p = pBF->SymbolByAddress(dest);
    if (p == NULL) return false;
    std::string name(p);
    // I believe that __xtol is for gcc, _ftol for earlier MSVC compilers, _ftol2 for MSVC V7
    if (name == "__xtol" || name == "_ftol" || name == "_ftol2") {
        // This appears to pop the top of stack, and converts the result to a 64 bit integer in edx:eax.
        // Truncates towards zero
        // r[tmpl] = ftoi(80, 64, r[32])
        // r[24] = trunc(64, 32, r[tmpl])
        // r[26] = r[tmpl] >> 32
        Statement* a = new Assign(new IntegerType(64),
                                  Location::tempOf(new Const(const_cast<char *>("tmpl"))),
                                  new Ternary(opFtoi,
                                              new Const(64),
                                              new Const(32),
                                              Location::regOf(32)));
        RTL* pRtl = new RTL(addr);
        pRtl->appendStmt(a);
        a = new Assign(
                Location::regOf(24),
                new Ternary(opTruncs,
                            new Const(64),
                            new Const(32),
                            Location::tempOf(new Const(const_cast<char *>("tmpl")))));
        pRtl->appendStmt(a);
        a = new Assign(
                Location::regOf(26),
                new Binary(opShiftR,
                           Location::tempOf(new Const(const_cast<char *>("tmpl"))),
                           new Const(32)));
        pRtl->appendStmt(a);
        // Append this RTL to the list of RTLs for this BB
        lrtl->push_back(pRtl);
        // Return true, so the caller knows not to create a HLCall
        return true;

    } else if (name == "__mingw_allocstack") {
        RTL* pRtl = new RTL(addr);
        Statement* a = new Assign(
                           Location::regOf(28),
                           new Binary(opMinus,
                                      Location::regOf(28),
                                      Location::regOf(24)));
        pRtl->appendStmt(a);
        lrtl->push_back(pRtl);
        prog->removeProc(name.c_str());
        return true;
    } else if (name == "__mingw_frame_init" || name == "__mingw_cleanup_setup" || name == "__mingw_frame_end") {
        LOG << "found removable call to static lib proc " << name << " at " << addr << "\n";
        prog->removeProc(name.c_str());
        return true;
    } else {
        // Will be other cases in future
    }
    return false;
}

/*==============================================================================
 * FUNCTION:      construct
 * OVERVIEW:      Construct a new instance of PentiumFrontEnd
 * PARAMETERS:      Same as the FrontEnd constructor, except decoder is **
 * RETURNS:          <nothing>
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
 *                    entries to point to this dynamic linked library
 * PARAMETERS:      Same as the FrontEnd constructor
 * RETURNS:          <N/A>
 *============================================================================*/
PentiumFrontEnd::PentiumFrontEnd(BinaryFile *pBF, Prog* prog, BinaryFileFactory* pbff) : FrontEnd(pBF, prog, pbff),
    idPF(-1) {
    decoder = new PentiumDecoder(prog);
}

// destructor
PentiumFrontEnd::~PentiumFrontEnd()
{
}

/*==============================================================================
 * FUNCTION:    GetMainEntryPoint
 * OVERVIEW:    Locate the starting address of "main" in the code section
 * PARAMETERS:    None
 * RETURNS:        Native pointer if found; NO_ADDRESS if not
 *============================================================================*/
ADDRESS PentiumFrontEnd::getMainEntryPoint(bool& gotMain) {
    gotMain = true;
    ADDRESS start = pBF->GetMainEntryPoint();
    if( start != NO_ADDRESS ) return start;

    gotMain = false;
    start = pBF->GetEntryPoint();
    if ( start.isZero() || start == NO_ADDRESS)
        return NO_ADDRESS;

    int instCount = 100;
    int conseq = 0;
    ADDRESS addr = start;

    // Look for 3 calls in a row in the first 100 instructions, with no other instructions between them.
    // This is the "windows" pattern. Another windows pattern: call to GetModuleHandleA followed by
    // a push of eax and then the call to main.  Or a call to __libc_start_main
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
                pBF->IsDynamicLinkedProcPointer(((Const*)cs->getDest() ->getSubExp1())->getAddr()) &&
                !strcmp(pBF->GetDynamicProcName(((Const*)cs->getDest()->getSubExp1())->getAddr()), "GetModuleHandleA"))
        {
#if 0
            std::cerr << "consider " << std::hex << addr << " " <<
                         pBF->GetDynamicProcName(((Const*)cs->getDest()->getSubExp1())->getAddr()) << std::endl;
#endif
            int oNumBytes = inst.numBytes;
            inst = decodeInstruction(addr + oNumBytes);
            if (inst.valid && inst.rtl->getNumStmt() == 2) {
                Assign* a = dynamic_cast<Assign*>(inst.rtl->elementAt(1));
                if (a && *a->getRight() == *Location::regOf(24)) {
#if 0
                    std::cerr << "is followed by push eax.. " << "good" << std::endl;
#endif
                    inst = decodeInstruction(addr + oNumBytes + inst.numBytes);
                    if (inst.rtl->getList().size()) {
                        CallStatement *toMain = dynamic_cast<CallStatement*>(inst.rtl->getList().back());
                        if (toMain && toMain->getFixedDest() != NO_ADDRESS) {
                            pBF->AddSymbol(toMain->getFixedDest(), "WinMain");
                            gotMain = true;
                            return toMain->getFixedDest();
                        }
                    }
                }
            }
        }
        if ((cs && cs->getKind() == STMT_CALL) && ((dest = (cs->getFixedDest())) != NO_ADDRESS)) {
            if (++conseq == 3 && 0) { // FIXME: this isn't working!
                // Success. Return the target of the last call
                gotMain = true;
                return cs->getFixedDest();
            }
            if (pBF->SymbolByAddress(dest) &&
                    strcmp(pBF->SymbolByAddress(dest), "__libc_start_main") == 0) {
                // This is a gcc 3 pattern. The first parameter will be a pointer to main.
                // Assume it's the 5 byte push immediately preceeding this instruction
                // Note: the RTL changed recently from esp = esp-4; m[esp] = K tp m[esp-4] = K; esp = esp-4
                inst = decodeInstruction(addr-5);
                assert(inst.valid);
                assert(inst.rtl->getNumStmt() == 2);
                Assign* a = (Assign*) inst.rtl->elementAt(0);        // Get m[esp-4] = K
                Exp* rhs = a->getRight();
                assert(rhs->isIntConst());
                gotMain = true;
                return ADDRESS::g(((Const*)rhs)->getInt()); //TODO: use getAddr ?
            }
        }
        else
            conseq = 0;            // Must be consequitive
        GotoStatement* gs = (GotoStatement*)cs;
        if (gs && gs->getKind() == STMT_GOTO)
            // Example: Borland often starts with a branch around some debug
            // info
            addr = gs->getFixedDest();
        else
            addr += inst.numBytes;
    } while (--instCount);

    // Last chance check: look for _main (e.g. Borland programs)
    ADDRESS umain = pBF->GetAddressByName("_main");
    if (umain != NO_ADDRESS) return umain;

    // Not ideal; we must return start
    std::cerr << "main function not found\n";

    this->AddSymbol(start, "_start");

    return start;
}

void toBranches(ADDRESS a, bool lastRtl, Cfg* cfg, RTL* rtl, PBB bb, BB_IT& it)
{
    BranchStatement* br1 = new BranchStatement;
    assert(rtl->getList().size() >= 4);        // They vary; at least 5 or 6
    Statement* s1 = *rtl->getList().begin();
    Statement* s6 = *(--rtl->getList().end());
    if (s1->isAssign())
        br1->setCondExpr(((Assign*)s1)->getRight());
    else
        br1->setCondExpr(NULL);
    br1->setDest(a+2);
    BranchStatement* br2 = new BranchStatement;
    if (s6->isAssign())
        br2->setCondExpr(((Assign*)s6)->getRight());
    else
        br2->setCondExpr(NULL);
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
        if (rtls == NULL)
            break;
        ADDRESS prev, addr = ADDRESS::g(0L);
        bool lastRtl = true;
        // For each RTL this BB
        for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
            RTL *rtl = *rit;
            prev = addr;
            addr = rtl->getAddress();
            if (rtl->getList().size()) {
                Statement* firstStmt = *rtl->getList().begin();
                if (firstStmt->isAssign()) {
                    Exp* lhs = ((Assign*)firstStmt)->getLeft();
                    if (lhs->isMachFtr()) {
                        Const* sub = (Const*)((Unary*)lhs)->getSubExp1();
                        const char* str = sub->getStr();
                        if (strncmp(str, "%SKIP", 5) == 0) {
                            toBranches(addr, lastRtl, cfg, rtl, bb, it);
                            noinc = true;        // toBranches inc's it
                            // Abandon this BB; if there are other string instr this BB, they will appear in new BBs
                            // near the end of the list
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

    // first, lets look for any uses of the registers
    std::set<int> usedRegs;
    StatementList stmts;
    proc->getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        LocationSet locs;
        s->addUsedLocs(locs);
        for (LocationSet::iterator li = locs.begin(); li != locs.end(); li++) {
            Exp *l = *li;
            if (!l->isRegOfK())
                continue;
            int n = ((Const*)l->getSubExp1())->getInt();
            usedRegs.insert(n);
        }
    }

    std::set<PBB> bbs;

    // For each statement, we are looking for assignments to registers in
    //     these ranges:
    // eax - ebx (24-27) (eax, ecx, edx, ebx)
    //    ax -  bx ( 0- 3) ( ax,    cx,     dx,  bx)
    //    al -  bl ( 8-11) ( al,    cl,     dl,  bl)
    //    ah -  bh (12-15) ( ah,    ch,     dh,  bh)
    // if found we want to generate assignments to the overlapping registers,
    // but only if they are used in this procedure.
    //
    // TMN: 2006-007-31. This code had been completely forgotten about:
    // esi/si, edi/di and ebp/bp. For now, let's hope we never encounter esp/sp. :-)
    // ebp (29)  bp (5)
    // esi (30)  si (6)
    // edi (31)  di (7)
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement* s = *it;
        if (s->getBB()->overlappedRegProcessingDone)   // never redo processing
            continue;
        bbs.insert(s->getBB());
        if (!s->isAssignment()) continue;
        Exp* lhs = ((Assignment*)s)->getLeft();
        if (!lhs->isRegOf()) continue;
        Const* c = (Const*)((Location*)lhs)->getSubExp1();
        assert(c->isIntConst());
        int r = c->getInt();
        int off = r&3;          // Offset into the array of 4 registers
        int off_mod8 = r&7;      // Offset into the array of 8 registers; for ebp, esi, edi
        Assign* a;
        switch (r) {
            case 24: case 25: case 26: case 27:
                //    eax         ecx      edx       ebx
                // Emit *16* r<off> := trunc(32, 16, r<24+off>)
                if (usedRegs.find(off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(16),
                            Location::regOf(off),
                            new Ternary(opTruncu,
                                        new Const(32),
                                        new Const(16),
                                        Location::regOf(24+off)));
                    proc->insertStatementAfter(s, a);
                }

                // Emit *8* r<8+off> := trunc(32, 8, r<24+off>)
                if (usedRegs.find(8+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(8),
                            Location::regOf(8+off),
                            new Ternary(opTruncu,
                                        new Const(32),
                                        new Const(8),
                                        Location::regOf(24+off)));
                    proc->insertStatementAfter(s, a);
                }

                // Emit *8* r<12+off> := r<24+off>@[15:8]
                if (usedRegs.find(12+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(8),
                            Location::regOf(12+off),
                            new Ternary(opAt,
                                        Location::regOf(24+off),
                                        new Const(15),
                                        new Const(8)));
                    proc->insertStatementAfter(s, a);
                }
                break;

            case 0: case 1: case 2: case 3:
                //    ax        cx        dx        bx
                // Emit *32* r<24+off> := r<24+off>@[31:16] | zfill(16, 32, r<off>)
                if (usedRegs.find(24+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(32),
                            Location::regOf(24+off),
                            new Binary(opBitOr,
                                       new Ternary(opAt,
                                                   Location::regOf(24+off),
                                                   new Const(31),
                                                   new Const(16)),
                                       new Ternary(opZfill,
                                                   new Const(16),
                                                   new Const(32),
                                                   Location::regOf(off))));
                    proc->insertStatementAfter(s, a);
                }

                // Emit *8* r<8+off> := trunc(16, 8, r<off>)
                if (usedRegs.find(8+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(8),
                            Location::regOf(8+off),
                            new Ternary(opTruncu,
                                        new Const(16),
                                        new Const(8),
                                        Location::regOf(24+off)));
                    proc->insertStatementAfter(s, a);
                }

                // Emit *8* r<12+off> := r<off>@[15:8]
                if (usedRegs.find(12+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(8),
                            Location::regOf(12+off),
                            new Ternary(opAt,
                                        Location::regOf(off),
                                        new Const(15),
                                        new Const(8)));
                    proc->insertStatementAfter(s, a);
                }
                break;


            case 8: case 9: case 10: case 11:
                //    al        cl         dl          bl
                // Emit *32* r<24+off> := r<24+off>@[31:8] | zfill(8, 32, r<8+off>)
                if (usedRegs.find(24+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(32),
                            Location::regOf(24+off),
                            new Binary(opBitOr,
                                       new Ternary(opAt,
                                                   Location::regOf(24+off),
                                                   new Const(31),
                                                   new Const(8)),
                                       new Ternary(opZfill,
                                                   new Const(8),
                                                   new Const(32),
                                                   Location::regOf(8+off))));
                    proc->insertStatementAfter(s, a);
                }

                // Emit *16* r<off> := r<off>@[15:8] | zfill(8, 16, r<8+off>)
                if (usedRegs.find(off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(16),
                            Location::regOf(off),
                            new Binary(opBitOr,
                                       new Ternary(opAt,
                                                   Location::regOf(off),
                                                   new Const(15),
                                                   new Const(8)),
                                       new Ternary(opZfill,
                                                   new Const(8),
                                                   new Const(16),
                                                   Location::regOf(8+off))));
                    proc->insertStatementAfter(s, a);
                }
                break;

            case 12: case 13: case 14: case 15:
                //     ah          ch       dh        bh
                // Emit *32* r<24+off> := r<24+off> & 0xFFFF00FF
                //        *32* r<24+off> := r<24+off> | r<12+off> << 8
                if (usedRegs.find(24+off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(32),
                            Location::regOf(24+off),
                            new Binary(opBitOr,
                                       Location::regOf(24+off),
                                       new Binary(opShiftL,
                                                  Location::regOf(12+off),
                                                  new Const(8))));
                    proc->insertStatementAfter(s, a);
                    a = new Assign(
                            new IntegerType(32),
                            Location::regOf(24+off),
                            new Binary(opBitAnd,
                                       Location::regOf(24+off),
                                       new Const(0xFFFF00FF)));
                    proc->insertStatementAfter(s, a);
                }

                // Emit *16* r<off> := r<off> & 0x00FF
                //        *16* r<off> := r<off> | r<12+off> << 8
                if (usedRegs.find(off) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(16),
                            Location::regOf(off),
                            new Binary(opBitOr,
                                       Location::regOf(off),
                                       new Binary(opShiftL,
                                                  Location::regOf(12+off),
                                                  new Const(8))));
                    proc->insertStatementAfter(s, a);
                    a = new Assign(
                            new IntegerType(16),
                            Location::regOf(off),
                            new Binary(opBitAnd,
                                       Location::regOf(off),
                                       new Const(0x00FF)));
                    proc->insertStatementAfter(s, a);
                }
                break;

            case 5: case 6: case 7:
                //    bp        si        di
                // Emit *32* r<24+off_mod8> := r<24+off_mod8>@[31:16] | zfill(16, 32, r<off_mod8>)
                if (usedRegs.find(24+off_mod8) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(32),
                            Location::regOf(24+off_mod8),
                            new Binary(opBitOr,
                                       new Ternary(opAt,
                                                   Location::regOf(24+off_mod8),
                                                   new Const(31),
                                                   new Const(16)),
                                       new Ternary(opZfill,
                                                   new Const(16),
                                                   new Const(32),
                                                   Location::regOf(off_mod8))));
                    proc->insertStatementAfter(s, a);
                }
                break;

            case 29: case 30: case 31:
                //    ebp         esi      edi
                // Emit *16* r<off_mod8> := trunc(32, 16, r<24+off_mod8>)
                if (usedRegs.find(off_mod8) != usedRegs.end()) {
                    a = new Assign(
                            new IntegerType(16),
                            Location::regOf(off_mod8),
                            new Ternary(opTruncu,
                                        new Const(32),
                                        new Const(16),
                                        Location::regOf(24+off_mod8)));
                    proc->insertStatementAfter(s, a);
                }
                break;
        }
    }

    // set a flag for every BB we've processed so we don't do them again
    for (std::set<PBB>::iterator bit = bbs.begin(); bit != bbs.end(); bit++)
        (*bit)->overlappedRegProcessingDone = true;
}

DecodeResult& PentiumFrontEnd::decodeInstruction(ADDRESS pc)
{
    int n = pBF->readNative1(pc);
    if (n == (int)(char)0xee) {
        // out dx, al
        static DecodeResult r;
        r.reset();
        r.numBytes = 1;
        r.valid = true;
        r.type = NCT;
        r.reDecode = false;
        r.rtl = new RTL(pc);
        Exp *dx = Location::regOf(decoder->getRTLDict().RegMap["%dx"]);
        Exp *al = Location::regOf(decoder->getRTLDict().RegMap["%al"]);
        CallStatement *call = new CallStatement();
        call->setDestProc(prog->getLibraryProc("outp"));
        call->setArgumentExp(0, dx);
        call->setArgumentExp(1, al);
        r.rtl->appendStmt(call);
        return r;
    }
    if (n == (int)(char)0x0f && pBF->readNative1(pc+1) == (int)(char)0x0b) {
        static DecodeResult r;
        r.reset();
        r.numBytes = 2;
        r.valid = true;
        r.type = NCT;
        r.reDecode = false;
        r.rtl = new RTL(pc);
        CallStatement *call = new CallStatement();
        call->setDestProc(prog->getLibraryProc("invalid_opcode"));
        r.rtl->appendStmt(call);
        return r;
    }
    return FrontEnd::decodeInstruction(pc);
}

// EXPERIMENTAL: can we find function pointers in arguments to calls this early?
void PentiumFrontEnd::extraProcessCall(CallStatement *call, std::list<RTL*> *BB_rtls)
{
    if (call->getDestProc()) {

        // looking for function pointers
        Signature *calledSig = call->getDestProc()->getSignature();
        for (unsigned int i = 0; i < calledSig->getNumParams(); i++) {
            // check param type
            Type *paramType = calledSig->getParamType(i);
            Type *points_to;
            CompoundType *compound = NULL;
            bool paramIsFuncPointer = false, paramIsCompoundWithFuncPointers = false;
            if (paramType->resolvesToPointer()) {
                points_to = paramType->asPointer()->getPointsTo();
                if (points_to->resolvesToFunc())
                    paramIsFuncPointer = true;
                else if (points_to->resolvesToCompound()) {
                    compound = points_to->asCompound();
                    for (unsigned int n = 0; n < compound->getNumTypes(); n++) {
                        if (    compound->getType(n)->resolvesToPointer() &&
                                compound->getType(n)->asPointer()->getPointsTo()->resolvesToFunc())
                            paramIsCompoundWithFuncPointers = true;
                    }
                }
            }
            if (paramIsFuncPointer == false && paramIsCompoundWithFuncPointers == false)
                continue;

            // count pushes backwards to find arg
            Exp *found = NULL;
            std::list<RTL*>::reverse_iterator itr;
            unsigned int pushcount = 0;
            for (itr = BB_rtls->rbegin(); itr != BB_rtls->rend() && !found; itr++) {
                RTL *rtl = *itr;
                for (int n = rtl->getNumStmt() - 1; n >= 0; n--) {
                    Statement *stmt = rtl->elementAt(n);
                    if (stmt->isAssign()) {
                        Assign *asgn = (Assign*)stmt;
                        if (asgn->getLeft()->isRegN(28) && asgn->getRight()->getOper() == opMinus)
                            pushcount++;
                        else if (pushcount == i + 2 && asgn->getLeft()->isMemOf() &&
                                 asgn->getLeft()->getSubExp1()->getOper() == opMinus &&
                                 asgn->getLeft()->getSubExp1()->getSubExp1()->isRegN(28) &&
                                 asgn->getLeft()->getSubExp1()->getSubExp2()->isIntConst()) {
                            found = asgn->getRight();
                            break;
                        }
                    }
                }
            }
            if (found == NULL)
                continue;

            ADDRESS a;
            if (found->isIntConst())
                a = ((Const*)found)->getInt();
            else if (found->isAddrOf() && found->getSubExp1()->isGlobal()) {
                const char *name = ((Const*)found->getSubExp1()->getSubExp1())->getStr();
                if (prog->getGlobal((char*)name) == NULL)
                    continue;
                a = prog->getGlobalAddr((char*)name);
            } else
                continue;

            // found one.
            if (paramIsFuncPointer) {
                if (VERBOSE)
                    LOG << "found a new procedure at address " << a << " from inspecting parameters of call to " <<
                           call->getDestProc()->getName() << ".\n";
                Proc *proc = prog->setNewProc(a);
                Signature *sig = paramType->asPointer()->getPointsTo()->asFunc()->getSignature()->clone();
                sig->setName(proc->getName());
                sig->setForced(true);
                proc->setSignature(sig);
                continue;
            }

            // linkers putting rodata in data sections is a continual annoyance
            // we just have to assume the pointers don't change before we pass them at least once.
            //if (!prog->isReadOnly(a))
            //    continue;

            for (unsigned int n = 0; n < compound->getNumTypes(); n++) {
                if (compound->getType(n)->resolvesToPointer() &&
                        compound->getType(n)->asPointer()->getPointsTo()->resolvesToFunc()) {
                    ADDRESS d = ADDRESS::g(pBF->readNative4(a));
                    if (VERBOSE)
                        LOG << "found a new procedure at address " << d << " from inspecting parameters of call to " <<
                               call->getDestProc()->getName() << ".\n";
                    Proc *proc = prog->setNewProc(d);
                    Signature *sig = compound->getType(n)->asPointer()->getPointsTo()->asFunc()->getSignature()->
                                     clone();
                    sig->setName(proc->getName());
                    sig->setForced(true);
                    proc->setSignature(sig);
                }
                a += compound->getType(n)->getSize() / 8;
            }
        }

        // some pentium specific ellipsis processing
        if (calledSig->hasEllipsis()) {
            // count pushes backwards to find a push of 0
            bool found = false;
            std::list<RTL*>::reverse_iterator itr;
            int pushcount = 0;
            for (itr = BB_rtls->rbegin(); itr != BB_rtls->rend() && !found; itr++) {
                RTL *rtl = *itr;
                for (int n = rtl->getNumStmt() - 1; n >= 0; n--) {
                    Statement *stmt = rtl->elementAt(n);
                    if (stmt->isAssign()) {
                        Assign *asgn = (Assign*)stmt;
                        if (asgn->getLeft()->isRegN(28) && asgn->getRight()->getOper() == opMinus)
                            pushcount++;
                        else if (asgn->getLeft()->isMemOf() &&
                                 asgn->getLeft()->getSubExp1()->getOper() == opMinus &&
                                 asgn->getLeft()->getSubExp1()->getSubExp1()->isRegN(28) &&
                                 asgn->getLeft()->getSubExp1()->getSubExp2()->isIntConst()) {
                            if (asgn->getRight()->isIntConst()) {
                                int n = ((Const*)asgn->getRight())->getInt();
                                if (n == 0) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (found && pushcount > 1) {
                call->setSigArguments();
                call->setNumArguments(pushcount - 1);
            }
        }
    }
}
