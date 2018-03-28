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


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinaryFile.h"
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
#include "boomerang/frontend/pentium/StringInstructionProcessor.h"
#include "boomerang/frontend/pentium/pentiumdecoder.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"

#include <cassert>
#include <cstring>
#include <sstream>


#define FSW    40 // Numeric registers
#define AH     12


bool PentiumFrontEnd::isStoreFsw(const Statement *s) const
{
    if (!s->isAssign()) {
        return false;
    }

    SharedExp rhs = static_cast<const Assign *>(s)->getRight();
    SharedExp result;
    bool      res = rhs->search(*Location::regOf(FSW), result);
    return res;
}


bool PentiumFrontEnd::isDecAh(const RTL *r) const
{
    // Check for decrement; RHS of middle Exp will be r[12]{8} - 1
    if (r->size() != 3) {
        return false;
    }

    auto      iter = r->begin();
    Statement *mid = *(++iter);

    if (!mid->isAssign()) {
        return false;
    }

    const Assign    *asgn = static_cast<const Assign *>(mid);
    SharedExp rhs   = asgn->getRight();
    Binary    ahm1(opMinus, Binary::get(opSize, Const::get(8), Location::regOf(12)), Const::get(1));
    return *rhs == ahm1;
}


bool PentiumFrontEnd::isSetX(const Statement *s) const
{
    // Check for SETX, i.e. <exp> ? 1 : 0
    // i.e. ?: <exp> Const 1 Const 0
    if (!s->isAssign()) {
        return false;
    }

    const Assign *asgn = static_cast<const Assign *>(s);
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


bool PentiumFrontEnd::isAssignFromTern(const Statement *s) const
{
    if (!s->isAssign()) {
        return false;
    }

    const Assign    *asgn = static_cast<const Assign *>(s);
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


bool PentiumFrontEnd::processProc(Address addr, UserProc *function, QTextStream& os, bool frag /* = false */,
                                  bool spec /* = false */)
{
    // Call the base class to do most of the work
    if (!IFrontEnd::processProc(addr, function, os, frag, spec)) {
        return false;
    }

    // Need a post-cfg pass to remove the FPUSH and FPOP instructions, and to transform various code after floating
    // point compares to generate floating point branches.
    // processFloatCode() will recurse to process its out-edge BBs (if not already processed)
    Cfg *cfg = function->getCFG();

    // This will get done twice; no harm
    function->setEntryBB();

    int tos = 0;
    processFloatCode(function->getEntryBB(), tos, cfg);
    processFloatCode(cfg);

    // Process away %rpt and %skip
    processStringInst(function);

    // Process code for side effects of overlapped registers
    processOverlapped(function);

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


void PentiumFrontEnd::processFloatCode(Cfg *cfg)
{
    for (BasicBlock *bb : *cfg) {
        Statement *st;

        // Loop through each RTL this BB
        RTLList *BB_rtls = bb->getRTLs();

        if (BB_rtls == nullptr) {
            // For example, incomplete BB
            return;
        }

        for (auto& rtl : *BB_rtls) {
            for (auto iter = rtl->begin(); iter != rtl->end(); /*incremented inside*/) {
                // Get the current Exp
                st = *iter;

                if (st->isFpush()) {
                    Assign *asgn = new Assign(FloatType::get(80),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9"))),
                                                 Location::regOf(39));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(39), Location::regOf(38));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(38), Location::regOf(37));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(37), Location::regOf(36));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(36), Location::regOf(35));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(35), Location::regOf(34));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(34), Location::regOf(33));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(33), Location::regOf(32));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(32),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9"))));
                    asgn->setBB(bb); rtl->insert(iter, asgn);

                    // Remove the FPUSH
                    iter = rtl->erase(iter);
                    continue;
                }
                else if (st->isFpop()) {
                    Assign *asgn = new Assign(FloatType::get(80),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9"))),
                                                 Location::regOf(32));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(32), Location::regOf(33));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(33), Location::regOf(34));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(34), Location::regOf(35));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(35), Location::regOf(36));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(36), Location::regOf(37));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(37), Location::regOf(38));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(38), Location::regOf(39));
                    asgn->setBB(bb); rtl->insert(iter, asgn);
                    asgn = new Assign(FloatType::get(80), Location::regOf(39),
                                                 Location::tempOf(Const::get(const_cast<char *>("tmpD9"))));
                    asgn->setBB(bb); rtl->insert(iter, asgn);

                    // Remove the FPOP
                    iter = rtl->erase(iter);
                    continue;
                }

                ++iter;
            }
        }
    }
}


void PentiumFrontEnd::processFloatCode(BasicBlock *bb, int& tos, Cfg *cfg)
{
    // Loop through each RTL this BB
    RTLList *BB_rtls = bb->getRTLs();

    if (BB_rtls == nullptr) {
        // For example, incomplete BB
        return;
    }

    Statement *st;

    for (const auto& rtl : *BB_rtls) {
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
                    Assign    *asgn = static_cast<Assign *>(st);
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

                for (cur = static_cast<Assign *>(st)->getRight(); !cur->isNil(); cur = cur->getSubExp2()) {
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

    m_floatProcessed.insert(bb);

    // Now recurse to process my out edges, if not already processed
    int oldNumSuccessors;

    do {
        oldNumSuccessors = bb->getNumSuccessors();

        for (BasicBlock *succ : bb->getSuccessors()) {
            if (isFloatProcessed(succ)) {
                continue; // nothing to do
            }

            processFloatCode(succ, tos, cfg);

            if (bb->getNumSuccessors() != oldNumSuccessors) {
                // During the processing, we have added or more likely deleted a BB, and the vector of out edges
                // has changed.  It's safe to just start the inner for loop again
                break;
            }
        }
    } while (bb->getNumSuccessors() != oldNumSuccessors);
}


bool PentiumFrontEnd::isHelperFunc(Address dest, Address addr, RTLList& lrtl)
{
    if (dest == Address::INVALID) {
        return false;
    }

    QString name = m_program->getSymbolNameByAddress(dest);

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
        std::unique_ptr<RTL> newRTL(new RTL(addr));
        newRTL->append(a);
        a = new Assign(Location::regOf(24), std::make_shared<Ternary>(opTruncs, Const::get(64), Const::get(32),
                                                                      Location::tempOf(Const::get(const_cast<char *>("tmpl")))));
        newRTL->append(a);
        a = new Assign(Location::regOf(26),
                       Binary::get(opShiftR, Location::tempOf(Const::get(const_cast<char *>("tmpl"))), Const::get(32)));
        newRTL->append(a);

        // Append this RTL to the list of RTLs for this BB
        lrtl.push_back(std::move(newRTL));

        // Return true, so the caller knows not to create a HLCall
        return true;
    }
    else if (name == "__mingw_allocstack") {
        std::unique_ptr<RTL> newRTL(new RTL(addr));
        newRTL->append(new Assign(Location::regOf(28), Binary::get(opMinus, Location::regOf(28), Location::regOf(24))));
        lrtl.push_back(std::move(newRTL));
        m_program->removeFunction(name);
        return true;
    }
    else if ((name == "__mingw_frame_init") || (name == "__mingw_cleanup_setup") || (name == "__mingw_frame_end")) {
        LOG_MSG("Found removable call to static lib proc %1 at address %2", name, addr);
        m_program->removeFunction(name);
        return true;
    }
    else {
        // Will be other cases in future
    }

    return false;
}


PentiumFrontEnd::PentiumFrontEnd(BinaryFile *binaryFile, Prog *prog)
    : IFrontEnd(binaryFile, prog)
{
    m_decoder.reset(new PentiumDecoder(prog));
}


PentiumFrontEnd::~PentiumFrontEnd()
{
}


Address PentiumFrontEnd::getMainEntryPoint(bool& gotMain)
{
    Address start = m_binaryFile->getMainEntryPoint();

    if (start != Address::INVALID) {
        gotMain = true;
        return start;
    }

    gotMain = false;
    start   = m_binaryFile->getEntryPoint();

    if (start.isZero() || (start == Address::INVALID)) {
        return Address::INVALID;
    }

    int     numInstructionsLeft = 100;
    Address addr      = start;

    BinarySymbolTable *symbols = m_program->getBinaryFile()->getSymbols();
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

        const CallStatement *cs = nullptr;

        if (!inst.rtl->empty()) {
            cs = (inst.rtl->back()->getKind() == StmtType::Call) ? static_cast<CallStatement *>(inst.rtl->back()) : nullptr;
        }

        const BinarySymbol *sym = (cs && cs->isCallToMemOffset()) ?
                                   symbols->findSymbolByAddress(cs->getDest()->access<Const, 1>()->getAddr()) : nullptr;

        if (sym && sym->isImportedFunction() && (sym->getName() == "GetModuleHandleA")) {
            const int oldInstLength = inst.numBytes;

            if (decodeInstruction(addr + oldInstLength, inst) && (inst.rtl->size() == 2)) {
                const Assign *asgn = dynamic_cast<Assign *>(inst.rtl->back()); // using back instead of rtl[1], since size()==2

                if (asgn && (*asgn->getRight() == *Location::regOf(24))) {
                    decodeInstruction(addr + oldInstLength + inst.numBytes, inst);

                    if (!inst.rtl->empty()) {
                        CallStatement *toMain = dynamic_cast<CallStatement *>(inst.rtl->back());

                        if (toMain && (toMain->getFixedDest() != Address::INVALID)) {
                            symbols->createSymbol(toMain->getFixedDest(), "WinMain");
                            gotMain = true;
                            return toMain->getFixedDest();
                        }
                    }
                }
            }
        }

        if ((cs && ((dest = (cs->getFixedDest())) != Address::INVALID))) {
            const QString destSym = m_program->getSymbolNameByAddress(dest);

            if (destSym == "__libc_start_main") {
                // This is a gcc 3 pattern. The first parameter will be a pointer to main.
                // Assume it's the 5 byte push immediately preceeding this instruction
                // Note: the RTL changed recently from esp = esp-4; m[esp] = K tp m[esp-4] = K; esp = esp-4
                decodeInstruction(addr - 5, inst);
                assert(inst.valid);
                assert(inst.rtl->size() == 2);
                Assign    *a  = static_cast<Assign *>(inst.rtl->front()); // Get m[esp-4] = K
                SharedExp rhs = a->getRight();
                assert(rhs->isIntConst());
                gotMain = true;
                return Address(rhs->access<Const>()->getInt()); // TODO: use getAddr ?
            }
        }

        const GotoStatement *gs = static_cast<const GotoStatement *>(cs);

        if (gs && (gs->getKind() == StmtType::Goto)) {
            // Example: Borland often starts with a branch around some debug
            // info
            addr = gs->getFixedDest();
        }
        else {
            addr += inst.numBytes;
        }
    } while (--numInstructionsLeft > 0);

    // Last chance check: look for _main (e.g. Borland programs)
    const BinarySymbol *sym = symbols->findSymbolByName("_main");

    if (sym) {
        return sym->getLocation();
    }

    // Not ideal; we must return start
    LOG_WARN("main function not found, falling back to entry point");

    if (symbols->findSymbolByAddress(start) == nullptr) {
        this->addSymbol(start, "_start");
    }

    return start;
}



void PentiumFrontEnd::processStringInst(UserProc *proc)
{
    StringInstructionProcessor(proc).processStringInstructions();
}


void PentiumFrontEnd::processOverlapped(UserProc *proc)
{
    // first, lets look for any uses of the registers
    std::set<int> usedRegs;
    StatementList stmts;
    proc->getStatements(stmts);

    for (Statement *s : stmts) {
        LocationSet locs;
        s->addUsedLocs(locs);

        for (SharedExp l : locs) {
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
    for (Statement *s : stmts) {
        if (isOverlappedRegsProcessed(s->getBB())) { // never redo processing
            continue;
        }

        bbs.insert(s->getBB());

        if (!s->isAssignment()) {
            continue;
        }

        SharedExp lhs = static_cast<Assignment *>(s)->getLeft();

        if (!lhs->isRegOf()) {
            continue;
        }

        auto c = lhs->access<Const, 1>();
        assert(c->isIntConst());
        int    r        = c->getInt();
        int    off      = r & 3; // Offset into the array of 4 registers
        int    off_mod8 = r & 7; // Offset into the array of 8 registers; for ebp, esi, edi

        switch (r)
        {
        case 24:
        case 25:
        case 26:
        case 27:

            //    eax         ecx      edx       ebx
            // Emit *16* r<off> := trunc(32, 16, r<24+off>)
            if (usedRegs.find(off) != usedRegs.end()) {
                Assign *a = new Assign(IntegerType::get(16), Location::regOf(off),
                               std::make_shared<Ternary>(opTruncu, Const::get(32), Const::get(16), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<8+off> := trunc(32, 8, r<24+off>)
            if (usedRegs.find(8 + off) != usedRegs.end()) {
                Assign *a = new Assign(IntegerType::get(8), Location::regOf(8 + off),
                               std::make_shared<Ternary>(opTruncu, Const::get(32), Const::get(8), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<12+off> := r<24+off>@[15:8]
            if (usedRegs.find(12 + off) != usedRegs.end()) {
                Assign *a = new Assign(IntegerType::get(8), Location::regOf(12 + off),
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
                Assign *a = new Assign(IntegerType::get(32), Location::regOf(24 + off),
                               Binary::get(opBitOr,
                                           std::make_shared<Ternary>(opAt, Location::regOf(24 + off), Const::get(31), Const::get(16)),
                                           std::make_shared<Ternary>(opZfill, Const::get(16), Const::get(32), Location::regOf(off))));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<8+off> := trunc(16, 8, r<off>)
            if (usedRegs.find(8 + off) != usedRegs.end()) {
                Assign *a = new Assign(IntegerType::get(8), Location::regOf(8 + off),
                               std::make_shared<Ternary>(opTruncu, Const::get(16), Const::get(8), Location::regOf(24 + off)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *8* r<12+off> := r<off>@[15:8]
            if (usedRegs.find(12 + off) != usedRegs.end()) {
                Assign *a = new Assign(IntegerType::get(8), Location::regOf(12 + off),
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
                Assign *a = new Assign(
                    IntegerType::get(32), Location::regOf(24 + off),
                    Binary::get(opBitOr, std::make_shared<Ternary>(opAt, Location::regOf(24 + off), Const::get(31), Const::get(8)),
                                std::make_shared<Ternary>(opZfill, Const::get(8), Const::get(32), Location::regOf(8 + off))));
                proc->insertStatementAfter(s, a);
            }

            // Emit *16* r<off> := r<off>@[15:8] | zfill(8, 16, r<8+off>)
            if (usedRegs.find(off) != usedRegs.end()) {
                Assign *a = new Assign(
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
                Assign *a = new Assign(IntegerType::get(32), Location::regOf(24 + off),
                               Binary::get(opBitOr, Location::regOf(24 + off),
                                           Binary::get(opShiftL, Location::regOf(12 + off), Const::get(8))));
                proc->insertStatementAfter(s, a);
                a = new Assign(IntegerType::get(32), Location::regOf(24 + off),
                               Binary::get(opBitAnd, Location::regOf(24 + off), Const::get(0xFFFF00FF)));
                proc->insertStatementAfter(s, a);
            }

            // Emit *16* r<off> := r<off> & 0x00FF
            //      *16* r<off> := r<off> | r<12+off> << 8
            if (usedRegs.find(off) != usedRegs.end()) {
                Assign *a = new Assign(IntegerType::get(16), Location::regOf(off),
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
                Assign *a = new Assign(
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
                Assign *a = new Assign(IntegerType::get(16), Location::regOf(off_mod8),
                               std::make_shared<Ternary>(opTruncu, Const::get(32), Const::get(16), Location::regOf(24 + off_mod8)));
                proc->insertStatementAfter(s, a);
            }

            break;
        }
    }

    // set a flag for every BB we've processed so we don't do them again
    m_overlappedRegsProcessed.insert(bbs.begin(), bbs.end());
}


bool PentiumFrontEnd::decodeSpecial_out(Address pc, DecodeResult& r)
{
    // out dx, al
    r.reset();
    r.numBytes = 1;
    r.valid    = true;
    r.type     = NCT;
    r.reDecode = false;
    r.rtl.reset(new RTL(pc));

    SharedExp     dx    = Location::regOf(m_decoder->getRegIdx("%dx"));
    SharedExp     al    = Location::regOf(m_decoder->getRegIdx("%al"));

    CallStatement *call = new CallStatement();
    call->setDestProc(m_program->getOrCreateLibraryProc("outp"));
    call->setArgumentExp(0, dx);
    call->setArgumentExp(1, al);
    r.rtl->append(call);

    return true;
}


bool PentiumFrontEnd::decodeSpecial_invalid(Address pc, DecodeResult& r)
{
    Byte n = m_program->getBinaryFile()->getImage()->readNative1(pc + 1);

    if (n != 0x0b) {
        return false;
    }

    r.reset();
    r.numBytes = 2;
    r.valid    = true;
    r.type     = NCT;
    r.reDecode = false;
    r.rtl.reset(new RTL(pc));

    CallStatement *call = new CallStatement();
    call->setDestProc(m_program->getOrCreateLibraryProc("invalid_opcode"));
    r.rtl->append(call);
    return true;
}


bool PentiumFrontEnd::decodeSpecial(Address pc, DecodeResult& r)
{
    Byte n = m_program->getBinaryFile()->getImage()->readNative1(pc);

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


void PentiumFrontEnd::extraProcessCall(CallStatement *call, const RTLList& BB_rtls)
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
                    if (compound->getTypeAtIdx(n)->resolvesToPointer() &&
                        compound->getTypeAtIdx(n)->as<PointerType>()->getPointsTo()->resolvesToFunc()) {
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
        unsigned int pushcount = 0;

        for (RTLList::const_reverse_iterator itr = BB_rtls.rbegin(); itr != BB_rtls.rend() && !found; ++itr) {
            RTL *rtl = itr->get();

            for (auto rtl_iter = rtl->rbegin(); rtl_iter != rtl->rend(); ++rtl_iter) {
                Statement *stmt = *rtl_iter;

                if (stmt->isAssign()) {
                    Assign *asgn = static_cast<Assign *>(stmt);

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
            Function *proc = m_program->createFunction(a);
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
            if (compound->getTypeAtIdx(n)->resolvesToPointer() &&
                compound->getTypeAtIdx(n)->as<PointerType>()->getPointsTo()->resolvesToFunc()) {
                Address d = Address(m_program->getBinaryFile()->getImage()->readNative4(a));
                LOG_VERBOSE("Found a new procedure at address %1 from inspecting parameters of call to %2",
                            d, call->getDestProc()->getName());

                Function *proc = m_program->createFunction(d);
                auto     sig   = compound->getTypeAtIdx(n)->as<PointerType>()->getPointsTo()->as<FuncType>()->getSignature()->clone();
                sig->setName(proc->getName());
                sig->setForced(true);
                proc->setSignature(sig);
            }

            a += compound->getTypeAtIdx(n)->getSize() / 8;
        }
    }

    // some pentium specific ellipsis processing
    if (calledSig->hasEllipsis()) {
        // count pushes backwards to find a push of 0
        bool found = false;
        int pushcount = 0;

        for (RTLList::const_reverse_iterator itr = BB_rtls.rbegin(); itr != BB_rtls.rend() && !found; ++itr) {
            RTL *rtl = itr->get();

            for (auto rtl_iter = rtl->rbegin(); rtl_iter != rtl->rend(); ++rtl_iter) {
                Statement *stmt = *rtl_iter;

                if (stmt->isAssign()) {
                    Assign *asgn = static_cast<Assign *>(stmt);

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
