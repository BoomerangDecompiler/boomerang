#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "pentiumfrontend.h"


/***************************************************************************/ /**
 * \file       pentiumfrontend.cpp
 * \brief   This file contains routines to manage the decoding of pentium instructions and the instantiation to RTLs.
 *               These functions replace frontend.cpp for decoding pentium instructions.
 ******************************************************************************/


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/Prog.h"              // For findProc()
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/loader/IFileLoader.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/frontend/pentium/pentiumdecoder.h"


#include <cassert>
#include <cstring>
#include <sstream>

#define FSW    40 // Numeric registers
#define AH     12

bool PentiumFrontEnd::isStoreFsw(Statement *s)
{
    if (!s->isAssign()) {
        return false;
    }

    SharedExp rhs = ((Assign *)s)->getRight();
    SharedExp result;
    bool      res = rhs->search(*Location::regOf(FSW), result);
    return res;
}


bool PentiumFrontEnd::isDecAh(RTL *r)
{
    // Check for decrement; RHS of middle Exp will be r[12]{8} - 1
    if (r->size() != 3) {
        return false;
    }

    auto        iter = r->begin();
    Statement *mid = *(++iter);

    if (!mid->isAssign()) {
        return false;
    }

    Assign    *asgn = (Assign *)mid;
    SharedExp rhs   = asgn->getRight();
    Binary    ahm1(opMinus, Binary::get(opSize, Const::get(8), Location::regOf(12)), Const::get(1));
    return *rhs == ahm1;
}


bool PentiumFrontEnd::isSetX(Statement *s)
{
    // Check for SETX, i.e. <exp> ? 1 : 0
    // i.e. ?: <exp> Const 1 Const 0
    if (!s->isAssign()) {
        return false;
    }

    Assign    *asgn = (Assign *)s;
    SharedExp lhs   = asgn->getLeft();

    // LHS must be a register
    if (!lhs->isRegOf()) {
        return false;
    }

    SharedExp rhs = asgn->getRight();

    if (rhs->getOper() != opTern) {
        return false;
    }

    SharedExp s2 = rhs->getSubExp2();
    SharedExp s3 = rhs->getSubExp3();

    if (!s2->isIntConst() || s3->isIntConst()) {
        return false;
    }

    return s2->access<Const>()->getInt() == 1 && s3->access<Const>()->getInt() == 0;
}


bool PentiumFrontEnd::isAssignFromTern(Statement *s)
{
    if (!s->isAssign()) {
        return false;
    }

    Assign    *asgn = (Assign *)s;
    SharedExp rhs   = asgn->getRight();
    return rhs->getOper() == opTern;
}


void PentiumFrontEnd::bumpRegisterAll(SharedExp e, int min, int max, int delta, int mask)
{
    std::list<SharedExp *> li;
    SharedExp              exp = e;
    // Use doSearch, which is normally an internal method of Exp, to avoid problems of replacing the wrong
    // subexpression (in some odd cases)
    Exp::doSearch(*Location::regOf(Terminal::get(opWild)), exp, li, false);

    for (SharedExp *it : li) {
        int reg = (*it)->access<Const, 1>()->getInt();

        if ((min <= reg) && (reg <= max)) {
            // Replace the K in r[ K] with a new K
            // **it is a reg[K]
            auto K = (*it)->access<Const, 1>();
            K->setInt(min + ((reg - min + delta) & mask));
        }
    }
}


bool PentiumFrontEnd::processProc(Address uAddr, UserProc *pProc, QTextStream& os, bool frag /* = false */,
                                  bool spec /* = false */)
{
    // Call the base class to do most of the work
    if (!IFrontEnd::processProc(uAddr, pProc, os, frag, spec)) {
        return false;
    }

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


std::vector<SharedExp>& PentiumFrontEnd::getDefaultParams()
{
    static std::vector<SharedExp> params;

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


std::vector<SharedExp>& PentiumFrontEnd::getDefaultReturns()
{
    static std::vector<SharedExp> returns;

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
        returns.push_back(Terminal::get(opPC));
    }

    return returns;
}


void PentiumFrontEnd::processFloatCode(Cfg *pCfg)
{
    BBIterator it;

    for (BasicBlock *pBB = pCfg->getFirstBB(it); pBB; pBB = pCfg->getNextBB(it)) {
        Statement *st;

        // Loop through each RTL this BB
        std::list<RTL *> *BB_rtls = pBB->getRTLs();

        if (BB_rtls == nullptr) {
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
                }
                else if (st->isFpop()) {
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


void PentiumFrontEnd::processFloatCode(BasicBlock *pBB, int& tos, Cfg *pCfg)
{
    std::list<RTL *>::iterator rit;
    Statement                *st;

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

            if (!st->isFlagAssign()) {
                // We are interested in either FPUSH/FPOP, or r[32..39] appearing in either the left or right hand
                // sides, or calls
                if (st->isFpush()) {
                    tos = (tos - 1) & 7;
                    // Remove the FPUSH
                    iter = rtl->erase(iter);
                    continue;
                }
                else if (st->isFpop()) {
                    tos = (tos + 1) & 7;
                    // Remove the FPOP
                    iter = rtl->erase(iter);
                    continue;
                }
                else if (st->isAssign()) {
                    Assign    *asgn = (Assign *)st;
                    SharedExp lhs   = asgn->getLeft();
                    SharedExp rhs   = asgn->getRight();

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
                SharedExp cur;

                for (cur = ((Assign *)st)->getRight(); !cur->isNil(); cur = cur->getSubExp2()) {
                    // I dont understand why we want typed exps in the flag calls so much. If we're going to replace
                    // opSize with TypedExps
                    // then we need to do it for everything, not just the flag calls.. so that should be in the
                    // sslparser.  If that is the
                    // case then we cant assume that opLists of flag calls will always contain TypedExps, so this code
                    // is wrong.
                    // - trent 9/6/2002
                    //                    TypedExp* te = (TypedExp*)cur->getSubExp1();
                    SharedExp s = cur->getSubExp1();

                    if (s->isRegOfK()) {
                        auto c = s->access<Const, 1>();
                        int  K = c->getInt(); // Old register number

                        // Change to new register number, if in range
                        if ((K >= 32) && (K <= 39)) {
                            s->setSubExp1(Const::get(32 + ((K - 32 + tos) & 7)));
                        }
                    }
                }
            }

            ++iter;
        }
    }

    pBB->setTraversed(true);

    // Now recurse to process my out edges, if not already processed
    const std::vector<BasicBlock *>& outs = pBB->getOutEdges();
    size_t n;

    do {
        n = outs.size();

        for (unsigned o = 0; o < n; o++) {
            BasicBlock *anOut = outs[o];

            if (!anOut->isTraversed()) {
                processFloatCode(anOut, tos, pCfg);

                if (outs.size() != n) {
                    // During the processing, we have added or more likely deleted a BB, and the vector of out edges
                    // has changed.  It's safe to just start the inner for loop again
                    break;
                }
            }
        }
    } while (outs.size() != n);
}


void PentiumFrontEnd::emitSet(std::list<RTL *> *BB_rtls, std::list<RTL *>::iterator& rit, Address uAddr, SharedExp lhs,
                              SharedExp cond)
{
    Statement *asgn = new Assign(lhs, std::make_shared<Ternary>(opTern, cond, Const::get(1), Const::get(0)));
    RTL         *pRtl = new RTL(uAddr);

    pRtl->appendStmt(asgn);
    //    std::cout << "Emit "; pRtl->print(); std::cout << '\n';
    // Insert the new RTL before rit
    BB_rtls->insert(rit, pRtl);
}


bool PentiumFrontEnd::isHelperFunc(Address dest, Address addr, std::list<RTL *> *lrtl)
{
    if (dest == Address::INVALID) {
        return false;
    }

    QString name = m_program->getSymbolByAddress(dest);

    if (name.isEmpty()) {
        return false;
    }

    // I believe that __xtol is for gcc, _ftol for earlier MSVC compilers, _ftol2 for MSVC V7
    if ((name == "__xtol") || (name == "_ftol") || (name == "_ftol2")) {
        // This appears to pop the top of stack, and converts the result to a 64 bit integer in edx:eax.
        // Truncates towards zero
        // r[tmpl] = ftoi(80, 64, r[32])
        // r[24] = trunc(64, 32, r[tmpl])
        // r[26] = r[tmpl] >> 32
        Statement *a = new Assign(IntegerType::get(64), Location::tempOf(Const::get(const_cast<char *>("tmpl"))),
                                    std::make_shared<Ternary>(opFtoi, Const::get(64), Const::get(32), Location::regOf(32)));
        RTL *pRtl = new RTL(addr);
        pRtl->appendStmt(a);
        a = new Assign(Location::regOf(24), std::make_shared<Ternary>(opTruncs, Const::get(64), Const::get(32),
                                                                      Location::tempOf(Const::get(const_cast<char *>("tmpl")))));
        pRtl->appendStmt(a);
        a = new Assign(Location::regOf(26),
                       Binary::get(opShiftR, Location::tempOf(Const::get(const_cast<char *>("tmpl"))), Const::get(32)));
        pRtl->appendStmt(a);
        // Append this RTL to the list of RTLs for this BB
        lrtl->push_back(pRtl);
        // Return true, so the caller knows not to create a HLCall
        return true;
    }
    else if (name == "__mingw_allocstack") {
        RTL         *pRtl = new RTL(addr);
        Statement *a    = new Assign(Location::regOf(28), Binary::get(opMinus, Location::regOf(28), Location::regOf(24)));
        pRtl->appendStmt(a);
        lrtl->push_back(pRtl);
        m_program->removeProc(name);
        return true;
    }
    else if ((name == "__mingw_frame_init") || (name == "__mingw_cleanup_setup") || (name == "__mingw_frame_end")) {
        LOG_MSG("Found removable call to static lib proc %1 at address %2", name, addr);
        m_program->removeProc(name);
        return true;
    }
    else {
        // Will be other cases in future
    }

    return false;
}


PentiumFrontEnd::PentiumFrontEnd(IFileLoader *p_BF, Prog *prog)
    : IFrontEnd(p_BF, prog)
    , idPF(-1)
{
    m_decoder = new PentiumDecoder(prog);
}


PentiumFrontEnd::~PentiumFrontEnd()
{
    delete m_decoder;
    m_decoder = nullptr;
}


Address PentiumFrontEnd::getMainEntryPoint(bool& gotMain)
{
    Address start = m_fileLoader->getMainEntryPoint();

    if (start != Address::INVALID) {
        gotMain = true;
        return start;
    }

    gotMain = false;
    start   = m_fileLoader->getEntryPoint();

    if (start.isZero() || (start == Address::INVALID)) {
        return Address::INVALID;
    }

    int     instCount = 100;
    int     conseq    = 0;
    Address addr      = start;

    IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();
    // Look for 3 calls in a row in the first 100 instructions, with no other instructions between them.
    // This is the "windows" pattern. Another windows pattern: call to GetModuleHandleA followed by
    // a push of eax and then the call to main.  Or a call to __libc_start_main
    Address dest;

    do {
        DecodeResult inst;
        decodeInstruction(addr, inst);

        if (inst.rtl == nullptr) {
            // Must have gotten out of step
            break;
        }

        CallStatement *cs = nullptr;

        if (!inst.rtl->empty()) {
            cs = (CallStatement *)((inst.rtl->back()->getKind() == STMT_CALL) ? inst.rtl->back() : nullptr);
        }

        const IBinarySymbol *sym = (cs && cs->isCallToMemOffset()) ?
                                   symbols->find(cs->getDest()->access<Const, 1>()->getAddr()) : nullptr;

        if (sym && sym->isImportedFunction() && (sym->getName() == "GetModuleHandleA")) {
            int oNumBytes = inst.numBytes;

            if (decodeInstruction(addr + oNumBytes, inst) && (inst.rtl->size() == 2)) {
                Assign *a = dynamic_cast<Assign *>(inst.rtl->back()); // using back instead of rtl[1], since size()==2

                if (a && (*a->getRight() == *Location::regOf(24))) {
                    decodeInstruction(addr + oNumBytes + inst.numBytes, inst);

                    if (!inst.rtl->empty()) {
                        CallStatement *toMain = dynamic_cast<CallStatement *>(inst.rtl->back());

                        if (toMain && (toMain->getFixedDest() != Address::INVALID)) {
                            symbols->create(toMain->getFixedDest(), "WinMain");
                            gotMain = true;
                            return toMain->getFixedDest();
                        }
                    }
                }
            }
        }

        if ((cs && ((dest = (cs->getFixedDest())) != Address::INVALID))) {
            ++conseq;

            QString dest_sym = m_program->getSymbolByAddress(dest);

            if (dest_sym == "__libc_start_main") {
                // This is a gcc 3 pattern. The first parameter will be a pointer to main.
                // Assume it's the 5 byte push immediately preceeding this instruction
                // Note: the RTL changed recently from esp = esp-4; m[esp] = K tp m[esp-4] = K; esp = esp-4
                decodeInstruction(addr - 5, inst);
                assert(inst.valid);
                assert(inst.rtl->size() == 2);
                Assign    *a  = (Assign *)inst.rtl->front(); // Get m[esp-4] = K
                SharedExp rhs = a->getRight();
                assert(rhs->isIntConst());
                gotMain = true;
                return Address(rhs->access<Const>()->getInt()); // TODO: use getAddr ?
            }
        }
        else {
            conseq = 0; // Must be consequitive
        }

        GotoStatement *gs = (GotoStatement *)cs;

        if (gs && (gs->getKind() == STMT_GOTO)) {
            // Example: Borland often starts with a branch around some debug
            // info
            addr = gs->getFixedDest();
        }
        else {
            addr += inst.numBytes;
        }
    } while (--instCount);

    // Last chance check: look for _main (e.g. Borland programs)
    const IBinarySymbol *sym = symbols->find("_main");

    if (sym) {
        return sym->getLocation();
    }

    // Not ideal; we must return start
    LOG_WARN("main function not found, falling back to entry point");

    if (symbols->find(start) == nullptr) {
        this->addSymbol(start, "_start");
    }

    return start;
}


void toBranches(Address a, bool /*lastRtl*/, Cfg *cfg, RTL *rtl, BasicBlock *bb, BBIterator& it)
{
    BranchStatement *br1 = new BranchStatement;

    assert(rtl->size() >= 4); // They vary; at least 5 or 6
    Statement *s1 = *rtl->begin();
    Statement *s6 = *(--rtl->end());

    if (s1->isAssign()) {
        br1->setCondExpr(((Assign *)s1)->getRight());
    }
    else {
        br1->setCondExpr(nullptr);
    }

    br1->setDest(a + 2);
    BranchStatement *br2 = new BranchStatement;

    if (s6->isAssign()) {
        br2->setCondExpr(((Assign *)s6)->getRight());
    }
    else {
        br2->setCondExpr(nullptr);
    }

    br2->setDest(a);
    cfg->splitForBranch(bb, rtl, br1, br2, it);
}


void PentiumFrontEnd::processStringInst(UserProc *proc)
{
    Cfg::iterator it;
    Cfg           *cfg = proc->getCFG();

    // For each BB this proc
    for (it = cfg->begin(); it != cfg->end(); /* no increment! */) {
        bool             noinc = false;
        BasicBlock       *bb   = *it;
        std::list<RTL *> *rtls = bb->getRTLs();

        if (rtls == nullptr) {
            break;
        }

        Address prev, addr = Address::ZERO;
        bool    lastRtl = true;

        // For each RTL this BB
        for (std::list<RTL *>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
            RTL *rtl = *rit;
            prev = addr;
            addr = rtl->getAddress();

            if (!rtl->empty()) {
                Statement *firstStmt = rtl->front();

                if (firstStmt->isAssign()) {
                    SharedExp lhs = ((Assign *)firstStmt)->getLeft();

                    if (lhs->isMachFtr()) {
                        auto    sub = lhs->access<Const, 1>();
                        QString str = sub->getStr();

                        if (str.startsWith("%SKIP")) {
                            toBranches(addr, lastRtl, cfg, rtl, bb, it);
                            noinc = true; // toBranches inc's it
                            // Abandon this BB; if there are other string instr this BB, they will appear in new BBs
                            // near the end of the list
                            break;
                        }
                        else {
                            LOG_VERBOSE("Unhandled machine feature %1", str);
                        }
                    }
                }
            }

            lastRtl = false;
        }

        if (!noinc) {
            it++;
        }
    }
}


void PentiumFrontEnd::processOverlapped(UserProc *proc)
{
    // first, lets look for any uses of the registers
    std::set<int> usedRegs;
    StatementList stmts;
    proc->getStatements(stmts);
    StatementList::iterator it;

    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement *s = *it;
        LocationSet locs;
        s->addUsedLocs(locs);

        for (LocationSet::iterator li = locs.begin(); li != locs.end(); li++) {
            SharedExp l = *li;

            if (!l->isRegOfK()) {
                continue;
            }

            int n = l->access<Const, 1>()->getInt();
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
        Statement *s = *it;

        if (s->getBB()->isOverlappedRegProcessingDone()) { // never redo processing
            continue;
        }

        bbs.insert(s->getBB());

        if (!s->isAssignment()) {
            continue;
        }

        SharedExp lhs = ((Assignment *)s)->getLeft();

        if (!lhs->isRegOf()) {
            continue;
        }

        auto c = lhs->access<Const, 1>();
        assert(c->isIntConst());
        int    r        = c->getInt();
        int    off      = r & 3; // Offset into the array of 4 registers
        int    off_mod8 = r & 7; // Offset into the array of 8 registers; for ebp, esi, edi
        Assign *a;

        switch (r)
        {
        case 24:
        case 25:
        case 26:
        case 27:

            //    eax         ecx      edx       ebx
            // Emit *16* r<off> := trunc(32, 16, r<24+off>)
            if (usedRegs.find(off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(16), Location::regOf(off),
                               std::make_shared<Ternary>(opTruncu, Const::get(32), Const::get(16), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<8+off> := trunc(32, 8, r<24+off>)
            if (usedRegs.find(8 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(8 + off),
                               std::make_shared<Ternary>(opTruncu, Const::get(32), Const::get(8), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<12+off> := r<24+off>@[15:8]
            if (usedRegs.find(12 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(12 + off),
                               std::make_shared<Ternary>(opAt, Location::regOf(24 + off), Const::get(15), Const::get(8)));
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
                                           std::make_shared<Ternary>(opAt, Location::regOf(24 + off), Const::get(31), Const::get(16)),
                                           std::make_shared<Ternary>(opZfill, Const::get(16), Const::get(32), Location::regOf(off))));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<8+off> := trunc(16, 8, r<off>)
            if (usedRegs.find(8 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(8 + off),
                               std::make_shared<Ternary>(opTruncu, Const::get(16), Const::get(8), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<12+off> := r<off>@[15:8]
            if (usedRegs.find(12 + off) != usedRegs.end()) {
                a = new Assign(IntegerType::get(8), Location::regOf(12 + off),
                               std::make_shared<Ternary>(opAt, Location::regOf(off), Const::get(15), Const::get(8)));
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
                    Binary::get(opBitOr, std::make_shared<Ternary>(opAt, Location::regOf(24 + off), Const::get(31), Const::get(8)),
                                std::make_shared<Ternary>(opZfill, Const::get(8), Const::get(32), Location::regOf(8 + off))));
                proc->insertStatementAfter(s, a);
            }

            // Emit *16* r<off> := r<off>@[15:8] | zfill(8, 16, r<8+off>)
            if (usedRegs.find(off) != usedRegs.end()) {
                a = new Assign(
                    IntegerType::get(16), Location::regOf(off),
                    Binary::get(opBitOr, std::make_shared<Ternary>(opAt, Location::regOf(off), Const::get(15), Const::get(8)),
                                std::make_shared<Ternary>(opZfill, Const::get(8), Const::get(16), Location::regOf(8 + off))));
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
                                std::make_shared<Ternary>(opAt, Location::regOf(24 + off_mod8), Const::get(31), Const::get(16)),
                                std::make_shared<Ternary>(opZfill, Const::get(16), Const::get(32), Location::regOf(off_mod8))));
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
                               std::make_shared<Ternary>(opTruncu, Const::get(32), Const::get(16), Location::regOf(24 + off_mod8)));
                proc->insertStatementAfter(s, a);
            }

            break;
        }
    }

    // set a flag for every BB we've processed so we don't do them again
    for (BasicBlock *bb : bbs) {
        bb->setOverlappedRegProcessingDone();
    }
}


bool PentiumFrontEnd::decodeSpecial_out(Address pc, DecodeResult& r)
{
    // out dx, al
    r.reset();
    r.numBytes = 1;
    r.valid    = true;
    r.type     = NCT;
    r.reDecode = false;
    r.rtl      = new RTL(pc);
    SharedExp     dx    = Location::regOf(m_decoder->getRegIdx("%dx"));
    SharedExp     al    = Location::regOf(m_decoder->getRegIdx("%al"));
    CallStatement *call = new CallStatement();
    call->setDestProc(m_program->getLibraryProc("outp"));
    call->setArgumentExp(0, dx);
    call->setArgumentExp(1, al);
    r.rtl->appendStmt(call);
    return true;
}


bool PentiumFrontEnd::decodeSpecial_invalid(Address pc, DecodeResult& r)
{
    int n = m_image->readNative1(pc + 1);

    if (n != (int)(char)0x0b) {
        return false;
    }

    r.reset();
    r.numBytes = 2;
    r.valid    = true;
    r.type     = NCT;
    r.reDecode = false;
    r.rtl      = new RTL(pc);

    CallStatement *call = new CallStatement();
    call->setDestProc(m_program->getLibraryProc("invalid_opcode"));
    r.rtl->appendStmt(call);
    return true;
}


bool PentiumFrontEnd::decodeSpecial(Address pc, DecodeResult& r)
{
    Byte n = m_image->readNative1(pc);

    switch (n)
    {
    case 0xee:
        return decodeSpecial_out(pc, r);

    case 0x0f:
        return decodeSpecial_invalid(pc, r);
    }

    return false;
}


bool PentiumFrontEnd::decodeInstruction(Address pc, DecodeResult& result)
{
    if (decodeSpecial(pc, result)) {
        return true;
    }

    return IFrontEnd::decodeInstruction(pc, result);
}


void PentiumFrontEnd::extraProcessCall(CallStatement *call, std::list<RTL *> *BB_rtls)
{
    if (!call->getDestProc()) {
        return;
    }

    // looking for function pointers
    auto calledSig = call->getDestProc()->getSignature();

    for (unsigned int i = 0; i < calledSig->getNumParams(); i++) {
        // check param type
        SharedType paramType = calledSig->getParamType(i);
        SharedType points_to;
        std::shared_ptr<CompoundType> compound;
        bool paramIsFuncPointer = false, paramIsCompoundWithFuncPointers = false;

        if (paramType->resolvesToPointer()) {
            points_to = paramType->as<PointerType>()->getPointsTo();

            if (points_to->resolvesToFunc()) {
                paramIsFuncPointer = true;
            }
            else if (points_to->resolvesToCompound()) {
                compound = points_to->as<CompoundType>();

                for (unsigned int n = 0; n < compound->getNumTypes(); n++) {
                    if (compound->getType(n)->resolvesToPointer() &&
                        compound->getType(n)->as<PointerType>()->getPointsTo()->resolvesToFunc()) {
                        paramIsCompoundWithFuncPointers = true;
                    }
                }
            }
        }

        if ((paramIsFuncPointer == false) && (paramIsCompoundWithFuncPointers == false)) {
            continue;
        }

        // count pushes backwards to find arg
        SharedExp found = nullptr;
        std::list<RTL *>::reverse_iterator itr;
        unsigned int pushcount = 0;

        for (itr = BB_rtls->rbegin(); itr != BB_rtls->rend() && !found; itr++) {
            RTL *rtl = *itr;

            for (auto rtl_iter = rtl->rbegin(); rtl_iter != rtl->rend(); ++rtl_iter) {
                Statement *stmt = *rtl_iter;

                if (stmt->isAssign()) {
                    Assign *asgn = (Assign *)stmt;

                    if (asgn->getLeft()->isRegN(28) && (asgn->getRight()->getOper() == opMinus)) {
                        pushcount++;
                    }
                    else if ((pushcount == i + 2) && asgn->getLeft()->isMemOf() &&
                             (asgn->getLeft()->getSubExp1()->getOper() == opMinus) &&
                             asgn->getLeft()->getSubExp1()->getSubExp1()->isRegN(28) &&
                             asgn->getLeft()->getSubExp1()->getSubExp2()->isIntConst()) {
                        found = asgn->getRight();
                        break;
                    }
                }
            }
        }

        if (found == nullptr) {
            continue;
        }

        Address a;

        if (found->isIntConst()) {
            a = Address(found->access<Const>()->getInt());
        }
        else if (found->isAddrOf() && found->getSubExp1()->isGlobal()) {
            QString name = found->access<Const, 1, 1>()->getStr();

            if (m_program->getGlobal(name) == nullptr) {
                continue;
            }

            a = m_program->getGlobalAddr(name);
        }
        else {
            continue;
        }

        // found one.
        if (paramIsFuncPointer) {
            LOG_VERBOSE("Found a new procedure at address %1 from inspecting parameters of call to '%2'.",
                        a, call->getDestProc()->getName());
            Function *proc = m_program->createProc(a);
            auto     sig   = paramType->as<PointerType>()->getPointsTo()->as<FuncType>()->getSignature()->clone();
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
                compound->getType(n)->as<PointerType>()->getPointsTo()->resolvesToFunc()) {

                Address d = Address(m_image->readNative4(a));
                LOG_VERBOSE("Found a new procedure at address %1 from inspecting parameters of call to %2",
                            d, call->getDestProc()->getName());

                Function *proc = m_program->createProc(d);
                auto     sig   = compound->getType(n)->as<PointerType>()->getPointsTo()->as<FuncType>()->getSignature()->clone();
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
                Statement *stmt = *rtl_iter;

                if (stmt->isAssign()) {
                    Assign *asgn = (Assign *)stmt;

                    if (asgn->getLeft()->isRegN(28) && (asgn->getRight()->getOper() == opMinus)) {
                        pushcount++;
                    }
                    else if (asgn->getLeft()->isMemOf() && (asgn->getLeft()->getSubExp1()->getOper() == opMinus) &&
                             asgn->getLeft()->getSubExp1()->getSubExp1()->isRegN(28) &&
                             asgn->getLeft()->getSubExp1()->getSubExp2()->isIntConst()) {
                        if (asgn->getRight()->isIntConst()) {
                            int n = asgn->getRight()->access<Const>()->getInt();

                            if (n == 0) {
                                found = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (found && (pushcount > 1)) {
            call->setSigArguments();
            call->setNumArguments(pushcount - 1);
        }
    }
}
