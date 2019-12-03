#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SPARCFrontEnd.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/LowLevelCFG.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/binary/BinaryFile.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/decomp/IndirectJumpAnalyzer.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/CFGDotWriter.h"
#include "boomerang/util/log/Log.h"

#include <cassert>
#include <cstring>
#include <iomanip>
#include <sstream>


#define SPARC_INSTRUCTION_LENGTH (4)


SPARCFrontEnd::SPARCFrontEnd(Project *project)
    : DefaultFrontEnd(project)
{
    Plugin *plugin = project->getPluginManager()->getPluginByName("Capstone SPARC decoder plugin");
    if (plugin) {
        m_decoder = plugin->getIfc<IDecoder>();
        m_decoder->initialize(project);
    }

    nop_inst.appendRTL(std::make_unique<RTL>(Address::INVALID), 0);
}


bool SPARCFrontEnd::disassembleFragment(UserProc *proc, Address addr)
{
    LOG_VERBOSE("### Decoding proc '%1' at address %2 ###", proc->getName(), addr);

    LowLevelCFG *cfg = m_program->getCFG();
    assert(cfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    m_targetQueue.initial(addr);

    int numBytesDecoded = 0;
    Address startAddr   = addr;
    Address lastAddr    = addr;
    MachineInstruction insn;

    while ((addr = m_targetQueue.popAddress(*cfg)) != Address::INVALID) {
        std::list<MachineInstruction> bbInsns;

        // Indicates whether or not the next instruction to be decoded is the lexical successor of
        // the current one. Will be true for all NCTs and for CTIs with a fall through branch.
        bool sequentialDecode = true;

        while (sequentialDecode) {
            BasicBlock *existingBB = cfg->getBBStartingAt(addr).bb;
            if (existingBB) {
                if (!bbInsns.empty()) {
                    // if bbInsns is not empty, the previous instruction was not a CTI.
                    // Complete the BB as a fallthrough
                    BasicBlock *newBB = cfg->createBB(BBType::Fall, bbInsns);
                    bbInsns.clear();
                    cfg->addEdge(newBB, existingBB);
                }

                if (existingBB->isComplete()) {
                    break; // do not disassemble BB twice
                }
            }

            if (!disassembleInstruction(addr, insn)) {
                // We might have disassembled a valid instruction, but the disassembler
                // does not recognize it. Do not throw away previous instructions;
                // instead, create a new BB from them
                if (!bbInsns.empty()) {
                    cfg->createBB(BBType::Fall, bbInsns);
                }

                LOG_ERROR("Encountered invalid instruction");
                sequentialDecode = false;
                break; // try next instruction in queue
            }

            if (m_program->getProject()->getSettings()->traceDecoder) {
                LOG_MSG("*%1 %2 %3", addr, insn.m_mnem.data(), insn.m_opstr.data());
            }

            // all instructions have the same length
            assert(insn.m_size == SPARC_INSTRUCTION_LENGTH);

            // alert the watchers that we have decoded an instruction
            numBytesDecoded += insn.m_size;
            m_program->getProject()->alertInstructionDecoded(addr, insn.m_size);


            // classify the current instruction. If it is not a CTI,
            // continue disassembling sequentially
            const bool isCTI = insn.m_iclass != IClass::NCT && insn.m_iclass != IClass::NOP;

            if (!isCTI) {
                addr += insn.m_size;
                bbInsns.push_back(insn);

                lastAddr = std::max(lastAddr, addr);
                continue;
            }

            bbInsns.push_back(insn);
            sequentialDecode = handleCTI(bbInsns, proc);

            addr += insn.m_size;
            lastAddr = std::max(lastAddr, addr);
        }
    }

    tagFunctionBBs(proc);

    m_program->getProject()->alertFunctionDecoded(proc, startAddr, lastAddr, numBytesDecoded);
    proc->setDecoded();

    LOG_VERBOSE("### Finished decoding proc '%1' ###", proc->getName());
    return true;
}


bool SPARCFrontEnd::liftProc(UserProc *proc)
{
    m_callList.clear();

    LowLevelCFG *cfg = proc->getProg()->getCFG();
    ProcCFG *procCFG = proc->getCFG();

    for (LowLevelCFG::BBStart &b : *cfg) {
        if (!b.bb) {
            continue;
        }

        BasicBlock *delay = cfg->getBBStartingAt(b.bb->getHiAddr()).delay;
        liftBB(b.bb, delay, proc);
    }

    for (const std::shared_ptr<CallStatement> &callStmt : m_callList) {
        Address dest = callStmt->getFixedDest();

        // Don't visit the destination of a register call
        Function *np = callStmt->getDestProc();

        if ((np == nullptr) && (dest != Address::INVALID)) {
            // np = newProc(proc->getProg(), dest);
            np = proc->getProg()->getOrCreateFunction(dest);
        }

        if (np != nullptr) {
            proc->addCallee(np);
        }
    }


    // add edges for fragments
    for (IRFragment *frag : *procCFG) {
        const BasicBlock *bb = frag->getBB();

        for (BasicBlock *succ : bb->getSuccessors()) {
            if (succ->isType(BBType::DelaySlot) && succ->getNumSuccessors() == 1) {
                succ = succ->getSuccessor(0);
            }

            IRFragment *succFragment = procCFG->getFragmentByBB(succ);
            if (succFragment) {
                procCFG->addEdge(frag, succFragment);
            }
        }
    }

    procCFG->setEntryAndExitFragment(procCFG->getFragmentByAddr(proc->getEntryAddress()));

    IRFragment::RTLIterator rit;
    StatementList::iterator sit;

    for (IRFragment *bb : *procCFG) {
        for (SharedStmt stmt = bb->getFirstStmt(rit, sit); stmt != nullptr;
             stmt            = bb->getNextStmt(rit, sit)) {
            assert(stmt->getProc() == nullptr || stmt->getProc() == proc);
            stmt->setProc(proc);
            stmt->setFragment(bb);
        }
    }

    return true;
}


Address SPARCFrontEnd::findMainEntryPoint(bool &gotMain)
{
    gotMain       = true;
    Address start = m_binaryFile->getMainEntryPoint();

    if (start != Address::INVALID) {
        return start;
    }

    start   = m_binaryFile->getEntryPoint();
    gotMain = false;

    if (start == Address::INVALID) {
        return Address::INVALID;
    }

    gotMain = true;
    return start;
}


bool SPARCFrontEnd::handleCTI(std::list<MachineInstruction> &bbInsns, UserProc *proc)
{
    LiftedInstruction lifted;
    LowLevelCFG *cfg                  = m_program->getCFG();
    const Address addr                = bbInsns.back().m_addr;
    const Interval<Address> limitText = m_program->getBinaryFile()->getImage()->getLimitText();

    if (!liftInstruction(bbInsns.back(), lifted)) {
        return false;
    }

    // Define aliases to the RTLs so that they can be treated as a high level types where
    // appropriate.
    RTL *rtl        = lifted.getFirstRTL();
    SharedStmt last = !rtl->empty() ? rtl->back() : nullptr;

    const IClass ctiClass = bbInsns.back().m_iclass;

    switch (ctiClass) {
    case IClass::SKIP: {
        // We can't simply ignore the skipped delay instruction as there
        // will most likely be a branch to it so we simply set the jump
        // to go to one past the skipped instruction.
        const Address dest = addr + 2 * SPARC_INSTRUCTION_LENGTH;

        BasicBlock *newBB = cfg->createBB(BBType::Oneway, bbInsns);
        bbInsns.clear();
        assert(newBB);
        newBB->setProc(proc);

        if (!limitText.contains(dest)) {
            LOG_ERROR("Jump destination is outside text limits!");
            return false;
        }

        m_targetQueue.pushAddress(cfg, dest, newBB);
        cfg->addEdge(newBB, dest);
        return false;
    }

    case IClass::SU: {
        BasicBlock *jumpBB = cfg->createBB(BBType::Oneway, bbInsns);
        bbInsns.clear();
        jumpBB->setProc(proc);

        // Ordinary, non-delay branch.
        if (last && last->isGoto()) {
            const Address dest = last->as<GotoStatement>()->getFixedDest();
            if (!limitText.contains(dest)) {
                LOG_ERROR("Jump destination is outside text limits!");
                return false;
            }

            m_targetQueue.pushAddress(cfg, dest, jumpBB);
            cfg->addEdge(jumpBB, dest);
        }

        // There is no fall through branch.
        return false;
    }

    case IClass::SD: {
        // This includes "call" and "ba". If a "call", it might be a move_call_move idiom,
        // or a call to .stret4
        const Address delayAddr = addr + SPARC_INSTRUCTION_LENGTH;
        MachineInstruction delayInsn;
        LiftedInstruction delayLifted;

        if (!decodeInstruction(delayAddr, delayInsn, delayLifted)) {
            warnInvalidInstruction(delayAddr);
            return false;
        }

        if (m_program->getProject()->getSettings()->traceDecoder) {
            LOG_MSG("*%1 %2 %3", delayAddr, delayInsn.m_mnem.data(), delayInsn.m_opstr.data());
        }

        if (!last) {
            LOG_ERROR("Cannot decode Static Delayed branch at address %1: "
                      "semantics are empty",
                      rtl->getAddress());
            break;
        }

        const BBType bbType = last->isCall() ? BBType::Call : BBType::Oneway;

        BasicBlock *bb = cfg->createBB(bbType, bbInsns);
        bbInsns.clear();
        bb->setProc(proc);

        // delay BB
        bbInsns.push_back(delayInsn);
        BasicBlock *delayBB = cfg->createBB(BBType::DelaySlot, bbInsns);
        bbInsns.clear();
        delayBB->setProc(proc);

        cfg->addEdge(bb, delayBB);
        // TODO: Determine the destination address

        if (bbType == BBType::Call) {
            BasicBlock *afterCTI = cfg->createIncompleteBB(addr + SPARC_INSTRUCTION_LENGTH);
            cfg->addEdge(bb, afterCTI);
            return true;
        }
        else {
            // unconditional delayed jump
            const Address destAddr = last->as<GotoStatement>()->getFixedDest();

            if (m_binaryFile->getImage()->getLimitText().contains(destAddr)) {
                m_targetQueue.pushAddress(cfg, destAddr, bb);
                cfg->addEdge(delayBB, destAddr);
            }

            return false;
        }
    } break;

    case IClass::DD: {
        // Ret/restore epilogues are handled as ordinary RTLs now
        if (last && last->isReturn()) {
            BasicBlock *retBB = cfg->createBB(BBType::Ret, bbInsns);
            bbInsns.clear();
            retBB->setProc(proc);

            MachineInstruction delayInsn;
            if (!disassembleInstruction(addr + SPARC_INSTRUCTION_LENGTH, delayInsn)) {
                LOG_ERROR("Failed to disassemble delay instruction");
                return false;
            }

            bbInsns.push_back(delayInsn);
            BasicBlock *delayBB = cfg->createBB(BBType::DelaySlot, bbInsns);
            bbInsns.clear();
            delayBB->setProc(proc);
            cfg->addEdge(retBB, delayBB);
            return false;
        }

        LOG_ERROR("Not implemented.");
    } break;

    case IClass::SCD:
    case IClass::SCDAN: {
        // Always execute the delay instr if SCD, and branch if condition is met.
        const Address delayAddr = addr + SPARC_INSTRUCTION_LENGTH;

        BasicBlock *branchBB = cfg->createBB(BBType::Twoway, bbInsns);
        bbInsns.clear();
        branchBB->setProc(proc);

        MachineInstruction delayInsn;
        LiftedInstruction delayLifted;
        if (!decodeInstruction(delayAddr, delayInsn, delayLifted)) {
            warnInvalidInstruction(delayAddr);
            return false;
        }

        bbInsns.push_back(delayInsn);
        BasicBlock *delayBB = cfg->createBB(BBType::DelaySlot, bbInsns);
        bbInsns.clear();
        delayBB->setProc(proc);
        cfg->addEdge(branchBB, delayBB);

        // anul the delay slot if SCDAN
        const Address fallthroughAddr = delayAddr +
                                        (ctiClass == IClass::SCDAN) * SPARC_INSTRUCTION_LENGTH;
        cfg->addEdge(branchBB, fallthroughAddr);

        const Address destAddr = last->as<BranchStatement>()->getFixedDest();
        if (destAddr != Address::INVALID) {
            m_targetQueue.pushAddress(cfg, destAddr, branchBB);
            cfg->addEdge(delayBB, destAddr);
        }
        return true;
    } break;

    default: // Others are non SPARC cases
        LOG_WARN("Encountered instruction class '%1' which is invalid for SPARC",
                 (int)bbInsns.back().m_iclass);
        break;
    }

    return true;
}


bool SPARCFrontEnd::liftBB(BasicBlock *bb, BasicBlock *delay, UserProc *proc)
{
    if (!bb || bb->getProc() != proc) {
        return false;
    }
    else if (delay && delay->getProc() != proc) {
        return false;
    }

    assert(!delay || delay->getInsns().size() == 1);
    const IClass iclass = bb->getInsns().back().m_iclass;

    const MachineInstruction *delayInsn = delay ? &delay->getInsns().front() : nullptr;

    switch (iclass) {
    case IClass::SD: return liftSD(bb, delayInsn, proc);
    case IClass::DD: return liftDD(bb, delayInsn, proc);
    case IClass::SCD: return liftSCD(bb, delayInsn, proc);
    case IClass::SCDAN: return liftSCDAN(bb, delayInsn, proc);
    case IClass::SCDAT: return liftSCDAT(bb, delayInsn, proc);
    case IClass::SU: return liftSU(bb, delayInsn, proc);
    case IClass::SKIP: return liftSKIP(bb, delayInsn, proc);
    case IClass::NCT:
    case IClass::NOP: return liftNCT(bb, proc);
    default: assert(false);
    }

    return false;
}


std::unique_ptr<RTLList> SPARCFrontEnd::liftBBPart(BasicBlock *bb)
{
    std::unique_ptr<RTLList> bbRTLs(new RTLList);

    for (const MachineInstruction &insn : bb->getInsns()) {
        if (&insn == &bb->getInsns().back()) {
            // handle the CTI separately
            break;
        }

        LiftedInstruction lifted;
        if (!m_decoder->liftInstruction(insn, lifted)) {
            return nullptr;
        }

        bbRTLs->push_back(lifted.useSingleRTL());
    }

    return bbRTLs;
}


bool SPARCFrontEnd::liftSD(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc)
{
    ProcCFG *cfg = proc->getCFG();

    // This includes "call" and "ba". If a "call", it might be a move_call_move idiom,
    // or a call to .stret4
    std::unique_ptr<RTLList> bbRTLs = liftBBPart(bb);

    LiftedInstruction liftedCTI;
    LiftedInstruction liftedDelay;

    if (!liftInstruction(bb->getInsns().back(), liftedCTI)) {
        return false;
    }
    else if (delayInsn && !liftInstruction(*delayInsn, liftedDelay)) {
        return false;
    }
    else if (liftedCTI.getFirstRTL()->empty()) {
        return false;
    }

    SharedStmt hlStmt = liftedCTI.getFirstRTL()->back();
    if (!hlStmt) {
        return false;
    }

    if (hlStmt->isCall()) {
        // Check the delay slot of this call. First case of interest is when the
        // instruction is a restore, e.g.
        // 142c8:  40 00 5b 91          call exit
        // 142cc:  91 e8 3f ff          restore %g0, -1, %o0
        // The restore means it is effectively followed by a return (since the
        // restore semantics chop off one level of return address)
        if (m_decoder->isSPARCRestore(*delayInsn)) {
            hlStmt->as<CallStatement>()->setReturnAfterCall(true);
            bbRTLs->push_back(liftedCTI.useSingleRTL());
            cfg->createFragment(std::move(bbRTLs), bb);

            m_callList.push_back(hlStmt->as<CallStatement>()); // case_CALL()
            return true;
        }
    }

    // Add all statements bt the high level statement to the RTL list
    bbRTLs->push_back(liftedCTI.useSingleRTL());
    assert(bbRTLs->back()->back() == hlStmt);
    bbRTLs->back()->pop_back();

    // Next class of interest is if it assigns to %o7 (could be a move, add,
    // and possibly others). E.g.:
    // 14e4c:  82 10 00 0f      mov     %o7, %g1
    // 14e50:  7f ff ff 60      call    blah
    // 14e54:  9e 10 00 01      mov     %g1, %o7
    //
    // Note there could be an unrelated instruction between the first move and the
    // call (move/x/call/move in UQBT terms).  In boomerang, we leave the semantics
    // of the moves there (to be likely removed by dataflow analysis) and merely
    // insert a return BB after the call.
    // Note that if an add, there may be an assignment to a temp register first.
    // So look at last RTL
    SharedStmt delayAsgn = liftedDelay.getFirstRTL()->empty() ? nullptr
                                                              : liftedDelay.getFirstRTL()->back();

    if (delayAsgn && delayAsgn->isAssign()) {
        SharedExp lhs = delayAsgn->as<Assign>()->getLeft();

        if (lhs->isRegN(REG_SPARC_O7)) {
            SharedExp rhs = delayAsgn->as<Assign>()->getRight();
            auto o7(Location::regOf(REG_SPARC_O7));

            // If it's an add, this is special. Example:
            //   0x1000  call foo
            //   0x1004  add %o7, K, %o7
            // causes the call to return not to 0x1008 but to 0x1008 + K.
            if (rhs->getOper() == opPlus && rhs->access<Exp, 2>()->isIntConst() &&
                *rhs->getSubExp1() == *o7) {
                // Get the constant
                const int K = rhs->access<Const, 2>()->getInt();

                // put the call statement back, then make a new unconditional jump fragment
                // right after it
                bbRTLs->back()->append(hlStmt);
                IRFragment *callFrag = cfg->createFragment(std::move(bbRTLs), bb);

                bbRTLs.reset(new RTLList);
                bbRTLs->push_back(std::unique_ptr<RTL>(
                    new RTL(delayInsn->m_addr,
                            { std::make_shared<GotoStatement>(delayInsn->m_addr + K) })));

                IRFragment *delayBranch = cfg->createFragment(std::move(bbRTLs), bb);
                cfg->addEdge(callFrag, delayBranch);
                return true;
            }
        }
    }

    liftedDelay.getFirstRTL()->append(hlStmt);
    bbRTLs->push_back(liftedDelay.useSingleRTL());

    cfg->createFragment(std::move(bbRTLs), bb);

    if (hlStmt->isCall()) {
        m_callList.push_back(hlStmt->as<CallStatement>());
    }

    return true;
}


bool SPARCFrontEnd::liftDD(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *)
{
    std::unique_ptr<RTLList> bbRTLs = liftBBPart(bb);

    LiftedInstruction liftedCTI;
    LiftedInstruction liftedDelay;

    if (!liftInstruction(bb->getInsns().back(), liftedCTI)) {
        return false;
    }
    else if (delayInsn && !liftInstruction(*delayInsn, liftedDelay)) {
        return false;
    }
    else if (liftedCTI.getFirstRTL()->empty()) {
        return false;
    }

    SharedStmt hlStmt = liftedCTI.getFirstRTL()->back();
    if (!hlStmt) {
        return false;
    }

    if (bb->isType(BBType::Ret)) {
        liftedCTI.getFirstRTL()->pop_back();
        bbRTLs->push_back(liftedCTI.useSingleRTL());
        bbRTLs->push_back(liftedDelay.useSingleRTL());
        bbRTLs->push_back(std::unique_ptr<RTL>(new RTL(delayInsn->m_addr, { hlStmt })));
        createReturnBlock(std::move(bbRTLs), bb);
        return true;
    }

    LOG_ERROR("Not implemented");
    return false;
}


bool SPARCFrontEnd::liftSCD(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc)
{
    ProcCFG *cfg                    = proc->getCFG();
    std::unique_ptr<RTLList> bbRTLs = liftBBPart(bb);

    LiftedInstruction liftedCTI;
    LiftedInstruction liftedDelay;

    if (!liftInstruction(bb->getInsns().back(), liftedCTI)) {
        return false;
    }
    else if (delayInsn && !liftInstruction(*delayInsn, liftedDelay)) {
        return false;
    }
    else if (liftedCTI.getFirstRTL()->empty()) {
        return false;
    }

    SharedStmt hlStmt = liftedCTI.getFirstRTL()->back();
    if (!hlStmt) {
        return false;
    }

    liftedDelay.getFirstRTL()->append(hlStmt);
    bbRTLs->push_back(liftedDelay.useSingleRTL());

    cfg->createFragment(std::move(bbRTLs), bb);

    return true;
}


bool SPARCFrontEnd::liftSCDAN(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc)
{
    ProcCFG *cfg                    = proc->getCFG();
    std::unique_ptr<RTLList> bbRTLs = liftBBPart(bb);

    LiftedInstruction liftedCTI;
    LiftedInstruction liftedDelay;

    if (!liftInstruction(bb->getInsns().back(), liftedCTI)) {
        return false;
    }
    else if (delayInsn && !liftInstruction(*delayInsn, liftedDelay)) {
        return false;
    }
    else if (liftedCTI.getFirstRTL()->empty()) {
        return false;
    }

    SharedStmt hlStmt = liftedCTI.getFirstRTL()->back();
    if (!hlStmt) {
        return false;
    }

    liftedDelay.getFirstRTL()->append(hlStmt);
    bbRTLs->push_back(liftedDelay.useSingleRTL());

    cfg->createFragment(std::move(bbRTLs), bb);

    return true;
}


bool SPARCFrontEnd::liftSCDAT(BasicBlock * /*bb*/, const MachineInstruction * /*delayInsn*/,
                              UserProc * /*proc*/)
{
    LOG_ERROR("Not implemented");
    return false;
}


bool SPARCFrontEnd::liftSU(BasicBlock * /*bb*/, const MachineInstruction * /*delayInsn*/,
                           UserProc * /*proc*/)
{
    LOG_ERROR("Not implemented");
    return false;
}


bool SPARCFrontEnd::liftSKIP(BasicBlock * /*bb*/, const MachineInstruction * /*delayInsn*/,
                             UserProc * /*proc*/)
{
    LOG_ERROR("Not implemented");
    return false;
}


bool SPARCFrontEnd::liftNCT(BasicBlock *bb, UserProc *proc)
{
    ProcCFG *cfg                    = proc->getCFG();
    std::unique_ptr<RTLList> bbRTLs = liftBBPart(bb);

    LiftedInstruction lifted;
    if (!liftInstruction(bb->getInsns().back(), lifted)) {
        return false;
    }

    bbRTLs->push_back(lifted.useSingleRTL());
    cfg->createFragment(std::move(bbRTLs), bb);
    return true;
}


void SPARCFrontEnd::warnInvalidInstruction(Address pc)
{
    QString message;
    BinaryImage *image = m_program->getBinaryFile()->getImage();

    Byte insnBytes[4] = { 0 };

    for (int i = 0; i < 4; i++) {
        if (!image->readNative1(pc + i, insnBytes[i])) {
            LOG_WARN("Tried to disassemble out of image bounds at address %1", pc);
            return;
        }
    }

    // clang-format off
    message.sprintf("Encountered invalid or unrecognized instruction at address %s: "
                    "0x%02X 0x%02X 0x%02X 0x%02X",
                    qPrintable(pc.toString()),
                    insnBytes[0],
                    insnBytes[1],
                    insnBytes[2],
                    insnBytes[3]);
    // clang-format on

    LOG_WARN(message);
}

BOOMERANG_DEFINE_PLUGIN(PluginType::FrontEnd, SPARCFrontEnd, "SPARC FrontEnd plugin",
                        BOOMERANG_VERSION, "Boomerang developers")
