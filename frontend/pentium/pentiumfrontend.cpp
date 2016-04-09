/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       pentiumfrontend.cpp
  * \brief   This file contains routines to manage the decoding of pentium instructions and the instantiation to RTLs.
  *               These functions replace frontend.cpp for decoding pentium instructions.
  ******************************************************************************/

#include "pentiumfrontend.h"
#include "types.h"
#include "BinaryFile.h"
#include "IBinaryImage.h"
#include "IBinarySymbols.h"
#include "frontend.h"
#include "rtl.h"
#include "decoder.h" // prototype for decodeInstruction()
#include "pentiumdecoder.h"
#include "register.h"
#include "type.h"
#include "cfg.h"
#include "exp.h"
#include "proc.h"
#include "signature.h"
#include "prog.h"       // For findProc()
#include "BinaryFile.h" // For SymbolByAddress()
#include "boomerang.h"
#include "basicblock.h"
#include "log.h"

#include <cassert>
#include <cstring>
#include <sstream>
/***************************************************************************/ /**
  * Forward declarations.
  ******************************************************************************/

#define FSW 40 // Numeric registers
#define AH 12

/***************************************************************************/ /**
  * \fn      isStoreFsw
  * \brief      Return true if the given Statement is an assignment that stores the FSW (Floating point Status Word)
  *                    reg
  * \param      s - Ptr to the given Statement
  * \returns           True if it is
  ******************************************************************************/
bool PentiumFrontEnd::isStoreFsw(Instruction *s) {
    if (!s->isAssign())
        return false;
    Exp *rhs = ((Assign *)s)->getRight();
    Exp *result;
    bool res = rhs->search(*Location::regOf(FSW), result);
    return res;
}
/***************************************************************************/ /**
  * \brief      Return true if the given RTL is a decrement of register AH
  * \param r - Ptr to the given RTL
  * \returns           True if it is
  ******************************************************************************/
bool PentiumFrontEnd::isDecAh(RTL *r) {
    // Check for decrement; RHS of middle Exp will be r[12]{8} - 1
    if (r->size() != 3)
        return false;
    auto iter = r->begin();
    Instruction *mid = *(++iter);
    if (!mid->isAssign())
        return false;
    Assign *asgn = (Assign *)mid;
    Exp *rhs = asgn->getRight();
    Binary ahm1(opMinus, Binary::get(opSize, Const::get(8), Location::regOf(12)), Const::get(1));
    return *rhs == ahm1;
}
/***************************************************************************/ /**
  * \fn      isSetX
  * \brief      Return true if the given Statement is a setX instruction
  * \param      s - Ptr to the given Statement
  * \returns           True if it is
  ******************************************************************************/
bool PentiumFrontEnd::isSetX(Instruction *s) {
    // Check for SETX, i.e. <exp> ? 1 : 0
    // i.e. ?: <exp> Const 1 Const 0
    if (!s->isAssign())
        return false;
    Assign *asgn = (Assign *)s;
    Exp *lhs = asgn->getLeft();
    // LHS must be a register
    if (!lhs->isRegOf())
        return false;
    Exp *rhs = asgn->getRight();
    if (rhs->getOper() != opTern)
        return false;
    Exp *s2 = ((Ternary *)rhs)->getSubExp2();
    Exp *s3 = ((Ternary *)rhs)->getSubExp3();
    if (!s2->isIntConst() || s3->isIntConst())
        return false;
    return ((Const *)s2)->getInt() == 1 && ((Const *)s3)->getInt() == 0;
}
/***************************************************************************/ /**
  * \fn      isAssignFromTern
  * \brief      Return true if the given Statement is an expression whose RHS is a ?: ternary
  * \param      s - Ptr to the given Statement
  * \returns           True if it is
  ******************************************************************************/
bool PentiumFrontEnd::isAssignFromTern(Instruction *s) {
    if (!s->isAssign())
        return false;
    Assign *asgn = (Assign *)s;
    Exp *rhs = asgn->getRight();
    return rhs->getOper() == opTern;
}
/***************************************************************************/ /**
  * \fn        PentiumFrontEnd::bumpRegisterAll
  * \brief        Finds a subexpression within this expression of the form
  *                      r[ int x] where min <= x <= max, and replaces it with
  *                      r[ int y] where y = min + (x - min + delta & mask)
  * \param e - Expression to modify
  * \param min - minimum register numbers before any change is considered
  * \param max - maximum register numbers before any change is considered
  * \param delta: amount to bump up the register number by
  * \param mask: see above
  * APPLICATION:        Used to "flatten" stack floating point arithmetic (e.g. Pentium floating point code)
  *                      If registers are not replaced "all at once" like this, there can be subtle errors from
  *                      re-replacing already replaced registers
  *
  ******************************************************************************/
void PentiumFrontEnd::bumpRegisterAll(Exp *e, int min, int max, int delta, int mask) {
    std::list<Exp **> li;
    std::list<Exp **>::iterator it;
    Exp *exp = e;
    // Use doSearch, which is normally an internal method of Exp, to avoid problems of replacing the wrong
    // subexpression (in some odd cases)
    Exp::doSearch(*Location::regOf(Terminal::get(opWild)), exp, li, false);
    for (it = li.begin(); it != li.end(); it++) {
        int reg = ((Const *)((Unary *)**it)->getSubExp1())->getInt();
        if ((min <= reg) && (reg <= max)) {
            // Replace the K in r[ K] with a new K
            // **it is a reg[K]
            Const *K = (Const *)((Unary *)**it)->getSubExp1();
            K->setInt(min + ((reg - min + delta) & mask));
        }
    }
}
/***************************************************************************/ /**
  * \fn      PentiumFrontEnd::processProc
  * \brief      Process a procedure, given a native (source machine) address.
  * This is the main function for decoding a procedure.
  * This overrides the base class processProc to do source machine specific things (but often calls the base
  * class to do most of the work. Sparc is an exception)
  * \param  uAddr - the address at which the procedure starts
  * \param  pProc - the procedure object
  * \param  os - output stream for rtl output
  * \param  frag - true if decoding only a fragment of the proc
  * \param  spec - true if this is a speculative decode (so give up on any invalid instruction)
  * \returns           True if successful decode
  ******************************************************************************/
bool PentiumFrontEnd::processProc(ADDRESS uAddr, UserProc *pProc, QTextStream &os, bool frag /* = false */,
                                  bool spec /* = false */) {

    // Call the base class to do most of the work
    if (!FrontEnd::processProc(uAddr, pProc, os, frag, spec))
        return false;

    // Need a post-cfg pass to remove the FPUSH and FPOP instructions, and to transform various code after floating
    // point compares to generate floating point branches.
    // processFloatCode() will recurse to process its out-edge BBs (if not already processed)
    Cfg *pCfg = pProc->getCFG();
    pCfg->unTraverse(); // Reset all the "traversed" flags (needed soon)
    // This will get done twice; no harm
    pProc->setEntryBB();

    int tos = 0;
    processFloatCode(pProc->getEntryBB(), tos, pCfg);
    processFloatCode(pCfg);

    // Process away %rpt and %skip
    processStringInst(pProc);

    // Process code for side effects of overlapped registers
    processOverlapped(pProc);

    return true;
}

std::vector<Exp *> &PentiumFrontEnd::getDefaultParams() {
    static std::vector<Exp *> params;
    if (params.size() == 0) {
        params.push_back(Location::regOf(24 /*eax*/));
        params.push_back(Location::regOf(25 /*ecx*/));
        params.push_back(Location::regOf(26 /*edx*/));
        params.push_back(Location::regOf(27 /*ebx*/));
        params.push_back(Location::regOf(28 /*esp*/));
        params.push_back(Location::regOf(29 /*ebp*/));
        params.push_back(Location::regOf(30 /*esi*/));
        params.push_back(Location::regOf(31 /*edi*/));
        params.push_back(Location::memOf(Location::regOf(28)));
    }
    return params;
}

std::vector<Exp *> &PentiumFrontEnd::getDefaultReturns() {
    static std::vector<Exp *> returns;
    if (returns.size() == 0) {
        returns.push_back(Location::regOf(24 /*eax*/));
        returns.push_back(Location::regOf(25 /*ecx*/));
        returns.push_back(Location::regOf(26 /*edx*/));
        returns.push_back(Location::regOf(27 /*ebx*/));
        returns.push_back(Location::regOf(28 /*esp*/));
        returns.push_back(Location::regOf(29 /*ebp*/));
        returns.push_back(Location::regOf(30 /*esi*/));
        returns.push_back(Location::regOf(31 /*edi*/));
        returns.push_back(Location::regOf(32 /*st0*/));
        returns.push_back(Location::regOf(33 /*st1*/));
        returns.push_back(Location::regOf(34 /*st2*/));
        returns.push_back(Location::regOf(35 /*st3*/));
        returns.push_back(Location::regOf(36 /*st4*/));
        returns.push_back(Location::regOf(37 /*st5*/));
        returns.push_back(Location::regOf(38 /*st6*/));
        returns.push_back(Location::regOf(39 /*st7*/));
        returns.push_back(new Terminal(opPC));
    }
    return returns;
}
/**
 * Little simpler, just replaces FPUSH and FPOP with more complex
 * semantics.
 */
void PentiumFrontEnd::processFloatCode(Cfg *pCfg) {
    BB_IT it;
    for (BasicBlock *pBB = pCfg->getFirstBB(it); pBB; pBB = pCfg->getNextBB(it)) {
        Instruction *st;

        // Loop through each RTL this BB
        std::list<RTL *> *BB_rtls = pBB->getRTLs();
        if (BB_rtls == 0) {
            // For example, incomplete BB
            return;
        }
        for (RTL *rtl : *BB_rtls) {
            for (auto iter = rtl->begin(); iter != rtl->end(); /*incremented inside*/) {
                // Get the current Exp
                st = *iter;
                if (st->isFpush()) {
                    rtl->insert(iter, new Assign(FloatType::get(80),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9"))),
                                                 Location::regOf(39)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(39), Location::regOf(38)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(38), Location::regOf(37)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(37), Location::regOf(36)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(36), Location::regOf(35)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(35), Location::regOf(34)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(34), Location::regOf(33)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(33), Location::regOf(32)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(32),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9")))));
                    // Remove the FPUSH
                    iter = rtl->erase(iter);
                    continue;
                } else if (st->isFpop()) {
                    rtl->insert(iter, new Assign(FloatType::get(80),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9"))),
                                                 Location::regOf(32)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(32), Location::regOf(33)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(33), Location::regOf(34)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(34), Location::regOf(35)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(35), Location::regOf(36)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(36), Location::regOf(37)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(37), Location::regOf(38)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(38), Location::regOf(39)));
                    rtl->insert(iter, new Assign(FloatType::get(80), Location::regOf(39),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9")))));
                    // Remove the FPOP
                    iter = rtl->erase(iter);
                    continue;
                }
                ++iter;
            }
        }
    }
}

/***************************************************************************/ /**
  * \brief Process a basic block, and all its successors, for floating point code.
  *  Remove FPUSH/FPOP, instead decrementing or incrementing respectively the tos value to be used from
  *  here down.
  * \note tos has to be a parameter, not a global, to get the right value at any point in
  *  the call tree
  * \param pBB: pointer to the current BB
  * \param tos reference to the value of the "top of stack" pointer currently. Starts at zero, and is
  *        decremented to 7 with the first load, so r[39] should be used first, then r[38] etc. However, it is
  *        reset to 0 for calls, so that if a function returns a float, then it will always appear in r[32]
  * \param pCfg passed to processFloatCode
  *
  ******************************************************************************/
void PentiumFrontEnd::processFloatCode(BasicBlock *pBB, int &tos, Cfg *pCfg) {
    std::list<RTL *>::iterator rit;
    Instruction *st;

    // Loop through each RTL this BB
    std::list<RTL *> *BB_rtls = pBB->getRTLs();
    if (BB_rtls == 0) {
        // For example, incomplete BB
        return;
    }
    rit = BB_rtls->begin();
    for (RTL *rtl : *BB_rtls) {
        // Check for call.
        if (rtl->isCall()) {
            // Reset the "top of stack" index. If this is not done, then after a sequence of calls to functions
            // returning floats, the value will appear to be returned in registers r[32], then r[33], etc.
            tos = 0;
        }
        if (rtl->empty()) {
            continue;
        }
        for (auto iter = rtl->begin(); iter != rtl->end(); /*iter incremented inside loop*/) {
            // Get the current Exp
            st = *iter;
            if (!st->isFlagAssgn()) {
                // We are interested in either FPUSH/FPOP, or r[32..39] appearing in either the left or right hand
                // sides, or calls
                if (st->isFpush()) {
                    tos = (tos - 1) & 7;
                    // Remove the FPUSH
                    iter = rtl->erase(iter);
                    continue;
                } else if (st->isFpop()) {
                    tos = (tos + 1) & 7;
                    // Remove the FPOP
                    iter = rtl->erase(iter);
                    continue;
                } else if (st->isAssign()) {
                    Assign *asgn = (Assign *)st;
                    Exp *lhs = asgn->getLeft();
                    Exp *rhs = asgn->getRight();
                    if (tos != 0) {
                        // Substitute all occurrences of r[x] (where 32 <= x <= 39) with r[y] where
                        // y = 32 + (x + tos) & 7
                        bumpRegisterAll(lhs, 32, 39, tos, 7);
                        bumpRegisterAll(rhs, 32, 39, tos, 7);
                    }
                }
            } else {
                // st is a flagcall
                // We are interested in any register parameters in the range 32 - 39
                Exp *cur;
                for (cur = ((Assign *)st)->getRight(); !cur->isNil(); cur = cur->getSubExp2()) {
                    // I dont understand why we want typed exps in the flag calls so much. If we're going to replace
                    // opSize with TypedExps
                    // then we need to do it for everything, not just the flag calls.. so that should be in the
                    // sslparser.  If that is the
                    // case then we cant assume that opLists of flag calls will always contain TypedExps, so this code
                    // is wrong.
                    // - trent 9/6/2002
                    //                    TypedExp* te = (TypedExp*)cur->getSubExp1();
                    Exp *s = cur->getSubExp1();
                    if (s->isRegOfK()) {
                        Const *c = (Const *)((Unary *)s)->getSubExp1();
                        int K = c->getInt(); // Old register number
                        // Change to new register number, if in range
                        if ((K >= 32) && (K <= 39))
                            s->setSubExp1(Const::get(32 + ((K - 32 + tos) & 7)));
                    }
                }
            }
            ++iter;
        }
    }
    pBB->setTraversed(true);

    // Now recurse to process my out edges, if not already processed
    const std::vector<BasicBlock *> &outs = pBB->getOutEdges();
    unsigned n;
    do {
        n = outs.size();
        for (unsigned o = 0; o < n; o++) {
            BasicBlock *anOut = outs[o];
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
void PentiumFrontEnd::emitSet(std::list<RTL *> *BB_rtls, std::list<RTL *>::iterator &rit, ADDRESS uAddr, Exp *lhs,
                              Exp *cond) {

    Instruction *asgn = new Assign(lhs, new Ternary(opTern, cond, Const::get(1), Const::get(0)));
    RTL *pRtl = new RTL(uAddr);
    pRtl->appendStmt(asgn);
    //    std::cout << "Emit "; pRtl->print(); std::cout << '\n';
    // Insert the new RTL before rit
    BB_rtls->insert(rit, pRtl);
}

/***************************************************************************/ /**
  * \brief Checks for pentium specific helper functions like __xtol which have specific sematics.
  * \note This needs to be handled in a resourcable way.
  * \param dest - the native destination of this call
  * \param addr - the native address of this call instruction
  * \param lrtl - pointer to a list of RTL pointers for this BB
  * \returns true if a helper function is converted; false otherwise
  ******************************************************************************/
bool PentiumFrontEnd::helperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL *> *lrtl) {
    if (dest == NO_ADDRESS)
        return false;

    QString name = Program->symbolByAddress(dest);
    if (name.isEmpty())
        return false;
    // I believe that __xtol is for gcc, _ftol for earlier MSVC compilers, _ftol2 for MSVC V7
    if (name == "__xtol" || name == "_ftol" || name == "_ftol2") {
        // This appears to pop the top of stack, and converts the result to a 64 bit integer in edx:eax.
        // Truncates towards zero
        // r[tmpl] = ftoi(80, 64, r[32])
        // r[24] = trunc(64, 32, r[tmpl])
        // r[26] = r[tmpl] >> 32
        Instruction *a = new Assign(IntegerType::get(64), Location::tempOf(Const::get(const_cast<char *>("tmpl"))),
                                  new Ternary(opFtoi, Const::get(64), Const::get(32), Location::regOf(32)));
        RTL *pRtl = new RTL(addr);
        pRtl->appendStmt(a);
        a = new Assign(Location::regOf(24), new Ternary(opTruncs, Const::get(64), Const::get(32),
                                                        Location::tempOf(Const::get(const_cast<char *>("tmpl")))));
        pRtl->appendStmt(a);
        a = new Assign(Location::regOf(26),
                       Binary::get(opShiftR, Location::tempOf(Const::get(const_cast<char *>("tmpl"))), Const::get(32)));
        pRtl->appendStmt(a);
        // Append this RTL to the list of RTLs for this BB
        lrtl->push_back(pRtl);
        // Return true, so the caller knows not to create a HLCall
        return true;

    } else if (name == "__mingw_allocstack") {
        RTL *pRtl = new RTL(addr);
        Instruction *a = new Assign(Location::regOf(28), Binary::get(opMinus, Location::regOf(28), Location::regOf(24)));
        pRtl->appendStmt(a);
        lrtl->push_back(pRtl);
        Program->removeProc(name);
        return true;
    } else if (name == "__mingw_frame_init" || name == "__mingw_cleanup_setup" || name == "__mingw_frame_end") {
        LOG << "found removable call to static lib proc " << name << " at " << addr << "\n";
        Program->removeProc(name);
        return true;
    } else {
        // Will be other cases in future
    }
    return false;
}

/***************************************************************************/ /**
  * \brief      PentiumFrontEnd constructor
  * \note          Seems to be necessary to put this here; forces the vtable
  *                    entries to point to this dynamic linked library
  * \copydetails FrontEnd::FrontEnd
  * \returns           <N/A>
  ******************************************************************************/
PentiumFrontEnd::PentiumFrontEnd(QObject *p_BF, Prog *prog, BinaryFileFactory *bff)
    : FrontEnd(p_BF, prog, bff), idPF(-1) {
    decoder = new PentiumDecoder(prog);
}

// destructor
PentiumFrontEnd::~PentiumFrontEnd() {
    delete decoder;
    decoder = nullptr;
}

/***************************************************************************/ /**
  * \brief    Locate the starting address of "main" in the code section
  * \returns         Native pointer if found; NO_ADDRESS if not
  ******************************************************************************/
ADDRESS PentiumFrontEnd::getMainEntryPoint(bool &gotMain) {
    ADDRESS start = ldrIface->GetMainEntryPoint();
    if (start != NO_ADDRESS) {
        gotMain = true;
        return start;
    }

    gotMain = false;
    start = ldrIface->GetEntryPoint();
    if (start.isZero() || start == NO_ADDRESS)
        return NO_ADDRESS;

    int instCount = 100;
    int conseq = 0;
    ADDRESS addr = start;

    IBinarySymbolTable *Symbols  = Boomerang::get()->getSymbols();
    // Look for 3 calls in a row in the first 100 instructions, with no other instructions between them.
    // This is the "windows" pattern. Another windows pattern: call to GetModuleHandleA followed by
    // a push of eax and then the call to main.  Or a call to __libc_start_main
    ADDRESS dest;
    do {
        DecodeResult inst = decodeInstruction(addr);
        if (inst.rtl == nullptr)
            // Must have gotten out of step
            break;
        CallStatement *cs = nullptr;
        if (!inst.rtl->empty())
            cs = (CallStatement *)((inst.rtl->back()->getKind() == STMT_CALL) ? inst.rtl->back() : nullptr);
        const IBinarySymbol *sym = (cs && cs->isCallToMemOffset()) ?
                    Symbols->find(((Const *)cs->getDest()->getSubExp1())->getAddr()) : nullptr;
        if (sym && sym->isImportedFunction() && sym->getName() == "GetModuleHandleA" ) {
            int oNumBytes = inst.numBytes;
            inst = decodeInstruction(addr + oNumBytes);
            if (inst.valid && inst.rtl->size() == 2) {
                Assign *a = dynamic_cast<Assign *>(inst.rtl->back()); // using back instead of rtl[1], since size()==2
                if (a && *a->getRight() == *Location::regOf(24)) {
                    inst = decodeInstruction(addr + oNumBytes + inst.numBytes);
                    if (!inst.rtl->empty()) {
                        CallStatement *toMain = dynamic_cast<CallStatement *>(inst.rtl->back());
                        if (toMain && toMain->getFixedDest() != NO_ADDRESS) {
                            Symbols->create(toMain->getFixedDest(),"WinMain");
                            gotMain = true;
                            return toMain->getFixedDest();
                        }
                    }
                }
            }
        }
        if ((cs && (dest = (cs->getFixedDest())) != NO_ADDRESS)) {
            if (++conseq == 3 && 0) { // FIXME: this isn't working!
                // Success. Return the target of the last call
                gotMain = true;
                return cs->getFixedDest();
            }
            QString dest_sym = Program->symbolByAddress(dest);
            if (dest_sym=="__libc_start_main") {
                // This is a gcc 3 pattern. The first parameter will be a pointer to main.
                // Assume it's the 5 byte push immediately preceeding this instruction
                // Note: the RTL changed recently from esp = esp-4; m[esp] = K tp m[esp-4] = K; esp = esp-4
                inst = decodeInstruction(addr - 5);
                assert(inst.valid);
                assert(inst.rtl->size() == 2);
                Assign *a = (Assign *)inst.rtl->front(); // Get m[esp-4] = K
                Exp *rhs = a->getRight();
                assert(rhs->isIntConst());
                gotMain = true;
                return ADDRESS::g(((Const *)rhs)->getInt()); // TODO: use getAddr ?
            }
        } else
            conseq = 0; // Must be consequitive
        GotoStatement *gs = (GotoStatement *)cs;
        if (gs && gs->getKind() == STMT_GOTO)
            // Example: Borland often starts with a branch around some debug
            // info
            addr = gs->getFixedDest();
        else
            addr += inst.numBytes;
    } while (--instCount);

    // Last chance check: look for _main (e.g. Borland programs)
    const IBinarySymbol *sym = Symbols->find("_main");
    if (sym)
        return sym->getLocation();

    // Not ideal; we must return start
    LOG_STREAM(2) << "main function not found\n";
    if(Symbols->find(start)==nullptr)
        this->AddSymbol(start, "_start");

    return start;
}

void toBranches(ADDRESS a, bool /*lastRtl*/, Cfg *cfg, RTL *rtl, BasicBlock *bb, BB_IT &it) {
    BranchStatement *br1 = new BranchStatement;
    assert(rtl->size() >= 4); // They vary; at least 5 or 6
    Instruction *s1 = *rtl->begin();
    Instruction *s6 = *(--rtl->end());
    if (s1->isAssign())
        br1->setCondExpr(((Assign *)s1)->getRight());
    else
        br1->setCondExpr(nullptr);
    br1->setDest(a + 2);
    BranchStatement *br2 = new BranchStatement;
    if (s6->isAssign())
        br2->setCondExpr(((Assign *)s6)->getRight());
    else
        br2->setCondExpr(nullptr);
    br2->setDest(a);
    cfg->splitForBranch(bb, rtl, br1, br2, it);
}
/**
 * Process away %rpt and %skip in string instructions
 */
void PentiumFrontEnd::processStringInst(UserProc *proc) {
    Cfg::iterator it;
    Cfg *cfg = proc->getCFG();
    // For each BB this proc
    for (it = cfg->begin(); it != cfg->end(); /* no increment! */) {
        bool noinc = false;
        BasicBlock *bb = *it;
        std::list<RTL *> *rtls = bb->getRTLs();
        if (rtls == nullptr)
            break;
        ADDRESS prev, addr = ADDRESS::g(0L);
        bool lastRtl = true;
        // For each RTL this BB
        for (std::list<RTL *>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
            RTL *rtl = *rit;
            prev = addr;
            addr = rtl->getAddress();
            if (!rtl->empty()) {
                Instruction *firstStmt = rtl->front();
                if (firstStmt->isAssign()) {
                    Exp *lhs = ((Assign *)firstStmt)->getLeft();
                    if (lhs->isMachFtr()) {
                        Const *sub = (Const *)((Unary *)lhs)->getSubExp1();
                        QString str = sub->getStr();
                        if (str.startsWith("%SKIP")) {
                            toBranches(addr, lastRtl, cfg, rtl, bb, it);
                            noinc = true; // toBranches inc's it
                            // Abandon this BB; if there are other string instr this BB, they will appear in new BBs
                            // near the end of the list
                            break;
                        } else
                            LOG << "Unhandled machine feature " << str << "\n";
                    }
                }
            }
            lastRtl = false;
        }
        if (!noinc)
            it++;
    }
}
/**
 * Process for overlapped registers
 */
void PentiumFrontEnd::processOverlapped(UserProc *proc) {

    // first, lets look for any uses of the registers
    std::set<int> usedRegs;
    StatementList stmts;
    proc->getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Instruction *s = *it;
        LocationSet locs;
        s->addUsedLocs(locs);
        for (LocationSet::iterator li = locs.begin(); li != locs.end(); li++) {
            Exp *l = *li;
            if (!l->isRegOfK())
                continue;
            int n = ((Const *)l->getSubExp1())->getInt();
            usedRegs.insert(n);
        }
    }

    std::set<BasicBlock *> bbs;

    // For each statement, we are looking for assignments to registers in
    //     these ranges:
    //   eax - ebx (24-27) (eax, ecx, edx, ebx)
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
        Instruction *s = *it;
        if (s->getBB()->isOverlappedRegProcessingDone()) // never redo processing
            continue;
        bbs.insert(s->getBB());
        if (!s->isAssignment())
            continue;
        Exp *lhs = ((Assignment *)s)->getLeft();
        if (!lhs->isRegOf())
            continue;
        Const *c = (Const *)((Location *)lhs)->getSubExp1();
        assert(c->isIntConst());
        int r = c->getInt();
        int off = r & 3;      // Offset into the array of 4 registers
        int off_mod8 = r & 7; // Offset into the array of 8 registers; for ebp, esi, edi
        Assign *a;
        switch (r) {
        case 24:
        case 25:
        case 26:
        case 27:
            //    eax         ecx      edx       ebx
            // Emit *16* r<off> := trunc(32, 16, r<24+off>)
            if (usedRegs.find(off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(16), Location::regOf(off),
                               new Ternary(opTruncu, Const::get(32), Const::get(16), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<8+off> := trunc(32, 8, r<24+off>)
            if (usedRegs.find(8 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(8 + off),
                               new Ternary(opTruncu, Const::get(32), Const::get(8), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<12+off> := r<24+off>@[15:8]
            if (usedRegs.find(12 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(12 + off),
                               new Ternary(opAt, Location::regOf(24 + off), Const::get(15), Const::get(8)));
                proc->insertStatementAfter(s, a);
            }
            break;

        case 0:
        case 1:
        case 2:
        case 3:
            //    ax        cx        dx        bx
            // Emit *32* r<24+off> := r<24+off>@[31:16] | zfill(16, 32, r<off>)
            if (usedRegs.find(24 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(32), Location::regOf(24 + off),
                               Binary::get(opBitOr,
                                           new Ternary(opAt, Location::regOf(24 + off), Const::get(31), Const::get(16)),
                                           new Ternary(opZfill, Const::get(16), Const::get(32), Location::regOf(off))));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<8+off> := trunc(16, 8, r<off>)
            if (usedRegs.find(8 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(8 + off),
                               new Ternary(opTruncu, Const::get(16), Const::get(8), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<12+off> := r<off>@[15:8]
            if (usedRegs.find(12 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(12 + off),
                               new Ternary(opAt, Location::regOf(off), Const::get(15), Const::get(8)));
                proc->insertStatementAfter(s, a);
            }
            break;

        case 8:
        case 9:
        case 10:
        case 11:
            //    al        cl         dl          bl
            // Emit *32* r<24+off> := r<24+off>@[31:8] | zfill(8, 32, r<8+off>)
            if (usedRegs.find(24 + off) != usedRegs.end()) {
                a = new Assign(
                    IntegerType::get(32), Location::regOf(24 + off),
                    Binary::get(opBitOr, new Ternary(opAt, Location::regOf(24 + off), Const::get(31), Const::get(8)),
                                new Ternary(opZfill, Const::get(8), Const::get(32), Location::regOf(8 + off))));
                proc->insertStatementAfter(s, a);
            }

            // Emit *16* r<off> := r<off>@[15:8] | zfill(8, 16, r<8+off>)
            if (usedRegs.find(off) != usedRegs.end()) {
                a = new Assign(
                    IntegerType::get(16), Location::regOf(off),
                    Binary::get(opBitOr, new Ternary(opAt, Location::regOf(off), Const::get(15), Const::get(8)),
                                new Ternary(opZfill, Const::get(8), Const::get(16), Location::regOf(8 + off))));
                proc->insertStatementAfter(s, a);
            }
            break;

        case 12:
        case 13:
        case 14:
        case 15:
            //     ah          ch       dh        bh
            // Emit *32* r<24+off> := r<24+off> & 0xFFFF00FF
            //        *32* r<24+off> := r<24+off> | r<12+off> << 8
            if (usedRegs.find(24 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(32), Location::regOf(24 + off),
                               Binary::get(opBitOr, Location::regOf(24 + off),
                                           Binary::get(opShiftL, Location::regOf(12 + off), Const::get(8))));
                proc->insertStatementAfter(s, a);
                a = new Assign(IntegerType::get(32), Location::regOf(24 + off),
                               Binary::get(opBitAnd, Location::regOf(24 + off), Const::get(0xFFFF00FF)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *16* r<off> := r<off> & 0x00FF
            //        *16* r<off> := r<off> | r<12+off> << 8
            if (usedRegs.find(off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(16), Location::regOf(off),
                               Binary::get(opBitOr, Location::regOf(off),
                                           Binary::get(opShiftL, Location::regOf(12 + off), Const::get(8))));
                proc->insertStatementAfter(s, a);
                a = new Assign(IntegerType::get(16), Location::regOf(off),
                               Binary::get(opBitAnd, Location::regOf(off), Const::get(0x00FF)));
                proc->insertStatementAfter(s, a);
            }
            break;

        case 5:
        case 6:
        case 7:
            //    bp        si        di
            // Emit *32* r<24+off_mod8> := r<24+off_mod8>@[31:16] | zfill(16, 32, r<off_mod8>)
            if (usedRegs.find(24 + off_mod8) != usedRegs.end()) {
                a = new Assign(
                    IntegerType::get(32), Location::regOf(24 + off_mod8),
                    Binary::get(opBitOr,
                                new Ternary(opAt, Location::regOf(24 + off_mod8), Const::get(31), Const::get(16)),
                                new Ternary(opZfill, Const::get(16), Const::get(32), Location::regOf(off_mod8))));
                proc->insertStatementAfter(s, a);
            }
            break;

        case 29:
        case 30:
        case 31:
            //    ebp         esi      edi
            // Emit *16* r<off_mod8> := trunc(32, 16, r<24+off_mod8>)
            if (usedRegs.find(off_mod8) != usedRegs.end()) {
                a = new Assign(IntegerType::get(16), Location::regOf(off_mod8),
                               new Ternary(opTruncu, Const::get(32), Const::get(16), Location::regOf(24 + off_mod8)));
                proc->insertStatementAfter(s, a);
            }
            break;
        }
    }

    // set a flag for every BB we've processed so we don't do them again
    for (BasicBlock * bb : bbs)
        bb->setOverlappedRegProcessingDone();
}
bool PentiumFrontEnd::decodeSpecial_out(ADDRESS pc, DecodeResult &r) {
    // out dx, al
    r.reset();
    r.numBytes = 1;
    r.valid = true;
    r.type = NCT;
    r.reDecode = false;
    r.rtl = new RTL(pc);
    Exp *dx = Location::regOf(decoder->getRTLDict().RegMap["%dx"]);
    Exp *al = Location::regOf(decoder->getRTLDict().RegMap["%al"]);
    CallStatement *call = new CallStatement();
    call->setDestProc(Program->getLibraryProc("outp"));
    call->setArgumentExp(0, dx);
    call->setArgumentExp(1, al);
    r.rtl->appendStmt(call);
    return true;
}
bool PentiumFrontEnd::decodeSpecial_invalid(ADDRESS pc, DecodeResult &r) {

    int n = Image->readNative1(pc + 1);
    if (n != (int)(char)0x0b)
        return false;
    r.reset();
    r.numBytes = 2;
    r.valid = true;
    r.type = NCT;
    r.reDecode = false;
    r.rtl = new RTL(pc);
    CallStatement *call = new CallStatement();
    call->setDestProc(Program->getLibraryProc("invalid_opcode"));
    r.rtl->appendStmt(call);
    return true;
}
bool PentiumFrontEnd::decodeSpecial(ADDRESS pc, DecodeResult &r) {
    int n = Image->readNative1(pc);
    switch ((int)(char)n) {
    case 0xee:
        return decodeSpecial_out(pc, r);
    case 0x0f:
        return decodeSpecial_invalid(pc, r);
    }
    return false;
}
DecodeResult &PentiumFrontEnd::decodeInstruction(ADDRESS pc) {
    static DecodeResult r;
    if (decodeSpecial(pc, r))
        return r;
    return FrontEnd::decodeInstruction(pc);
}

// EXPERIMENTAL: can we find function pointers in arguments to calls this early?
void PentiumFrontEnd::extraProcessCall(CallStatement *call, std::list<RTL *> *BB_rtls) {
    if (!call->getDestProc())
        return;

    // looking for function pointers
    Signature *calledSig = call->getDestProc()->getSignature();
    for (unsigned int i = 0; i < calledSig->getNumParams(); i++) {
        // check param type
        SharedType paramType = calledSig->getParamType(i);
        SharedType points_to;
        std::shared_ptr<CompoundType> compound;
        bool paramIsFuncPointer = false, paramIsCompoundWithFuncPointers = false;
        if (paramType->resolvesToPointer()) {
            points_to = paramType->asPointer()->getPointsTo();
            if (points_to->resolvesToFunc())
                paramIsFuncPointer = true;
            else if (points_to->resolvesToCompound()) {
                compound = points_to->as<CompoundType>();
                for (unsigned int n = 0; n < compound->getNumTypes(); n++) {
                    if (compound->getType(n)->resolvesToPointer() &&
                        compound->getType(n)->asPointer()->getPointsTo()->resolvesToFunc())
                        paramIsCompoundWithFuncPointers = true;
                }
            }
        }
        if (paramIsFuncPointer == false && paramIsCompoundWithFuncPointers == false)
            continue;

        // count pushes backwards to find arg
        Exp *found = nullptr;
        std::list<RTL *>::reverse_iterator itr;
        unsigned int pushcount = 0;
        for (itr = BB_rtls->rbegin(); itr != BB_rtls->rend() && !found; itr++) {
            RTL *rtl = *itr;
            for (auto rtl_iter = rtl->rbegin(); rtl_iter != rtl->rend(); ++rtl_iter) {
                Instruction *stmt = *rtl_iter;
                if (stmt->isAssign()) {
                    Assign *asgn = (Assign *)stmt;
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
        if (found == nullptr)
            continue;

        ADDRESS a;
        if (found->isIntConst())
            a = ((Const *)found)->getInt();
        else if (found->isAddrOf() && found->getSubExp1()->isGlobal()) {
            QString name = ((Const *)found->getSubExp1()->getSubExp1())->getStr();
            if (Program->getGlobal(name) == nullptr)
                continue;
            a = Program->getGlobalAddr(name);
        } else
            continue;

        // found one.
        if (paramIsFuncPointer) {
            LOG_VERBOSE(1) << "found a new procedure at address " << a << " from inspecting parameters of call to "
                           << call->getDestProc()->getName() << ".\n";
            Function *proc = Program->setNewProc(a);
            Signature *sig = paramType->asPointer()->getPointsTo()->asFunc()->getSignature()->clone();
            sig->setName(proc->getName());
            sig->setForced(true);
            proc->setSignature(sig);
            continue;
        }

        // linkers putting rodata in data sections is a continual annoyance
        // we just have to assume the pointers don't change before we pass them at least once.
        // if (!prog->isReadOnly(a))
        //    continue;

        for (unsigned int n = 0; n < compound->getNumTypes(); n++) {
            if (compound->getType(n)->resolvesToPointer() &&
                compound->getType(n)->asPointer()->getPointsTo()->resolvesToFunc()) {
                ADDRESS d = ADDRESS::g(Image->readNative4(a));
                if (VERBOSE)
                    LOG << "found a new procedure at address " << d << " from inspecting parameters of call to "
                        << call->getDestProc()->getName() << ".\n";
                Function *proc = Program->setNewProc(d);
                Signature *sig = compound->getType(n)->asPointer()->getPointsTo()->asFunc()->getSignature()->clone();
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
        std::list<RTL *>::reverse_iterator itr;
        int pushcount = 0;
        for (itr = BB_rtls->rbegin(); itr != BB_rtls->rend() && !found; itr++) {
            RTL *rtl = *itr;
            for (auto rtl_iter = rtl->rbegin(); rtl_iter != rtl->rend(); ++rtl_iter) {
                Instruction *stmt = *rtl_iter;
                if (stmt->isAssign()) {
                    Assign *asgn = (Assign *)stmt;
                    if (asgn->getLeft()->isRegN(28) && asgn->getRight()->getOper() == opMinus)
                        pushcount++;
                    else if (asgn->getLeft()->isMemOf() && asgn->getLeft()->getSubExp1()->getOper() == opMinus &&
                             asgn->getLeft()->getSubExp1()->getSubExp1()->isRegN(28) &&
                             asgn->getLeft()->getSubExp1()->getSubExp2()->isIntConst()) {
                        if (asgn->getRight()->isIntConst()) {
                            int n = ((Const *)asgn->getRight())->getInt();
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
