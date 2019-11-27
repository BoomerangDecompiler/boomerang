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

    nop_inst.iclass = IClass::NOP;
    nop_inst.rtl    = std::make_unique<RTL>(Address::INVALID);
}


bool SPARCFrontEnd::processProc(UserProc *proc, Address addr)
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

    m_program->getProject()->alertFunctionDecoded(proc, startAddr, lastAddr, numBytesDecoded);
    proc->setDecoded();

    LOG_VERBOSE("### Finished decoding proc '%1' ###", proc->getName());
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
    DecodeResult lifted;
    LowLevelCFG *cfg                  = m_program->getCFG();
    const Address addr                = bbInsns.back().m_addr;
    const Interval<Address> limitText = m_program->getBinaryFile()->getImage()->getLimitText();

    if (!liftInstruction(bbInsns.back(), lifted)) {
        return false;
    }

    // Define aliases to the RTLs so that they can be treated as a high level types where
    // appropriate.
    RTL *rtl        = lifted.rtl.get();
    SharedStmt last = !rtl->empty() ? rtl->back() : nullptr;

    if (lifted.reLift) {
        DecodeResult dummyLifted;
        bool ok;
        do {
            ok = m_decoder->liftInstruction(bbInsns.back(), dummyLifted);
        } while (ok && dummyLifted.reLift);
    }

    switch (lifted.iclass) {
    case IClass::SKIP: {
        // We can't simply ignore the skipped delay instruction as there
        // will most likely be a branch to it so we simply set the jump
        // to go to one past the skipped instruction.
        const Address dest = addr + 2 * SPARC_INSTRUCTION_LENGTH;

        BasicBlock *newBB = cfg->createBB(BBType::Oneway, bbInsns);
        bbInsns.clear();
        assert(newBB);
        newBB->setFunction(proc);

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
        jumpBB->setFunction(proc);

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
        DecodeResult delayLifted;

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
        bb->setFunction(proc);

        // delay BB
        bbInsns.push_back(delayInsn);
        BasicBlock *delayBB = cfg->createBB(BBType::DelaySlot, bbInsns);
        bbInsns.clear();
        delayBB->setFunction(proc);

        cfg->addEdge(bb, delayBB);
        // TODO: Determine the destination address

        if (bbType == BBType::Call) {
            BasicBlock *afterCTI = cfg->createIncompleteBB(addr + SPARC_INSTRUCTION_LENGTH);
            cfg->addEdge(bb, afterCTI);
            return true;
        }
        else {
            // unconditional delayed jump
            return false;
        }
    } break;

    case IClass::DD: {
        // Ret/restore epilogues are handled as ordinary RTLs now
        if (last && last->isReturn()) {
            BasicBlock *retBB = cfg->createBB(BBType::Ret, bbInsns);
            bbInsns.clear();
            retBB->setFunction(proc);

            MachineInstruction delayInsn;
            if (!disassembleInstruction(addr + SPARC_INSTRUCTION_LENGTH, delayInsn)) {
                LOG_ERROR("Failed to disassemble delay instruction");
                return false;
            }

            bbInsns.push_back(delayInsn);
            BasicBlock *delayBB = cfg->createBB(BBType::DelaySlot, bbInsns);
            bbInsns.clear();
            delayBB->setFunction(proc);
            cfg->addEdge(retBB, delayBB);
            return false;
        }

        LOG_ERROR("Not implemented.");
    } break;

    case IClass::SCD: {
        LOG_ERROR("Not implemented.");
    } break;

    case IClass::SCDAN: {
        LOG_ERROR("Not implemented.");
    } break;

    default: // Others are non SPARC cases
        LOG_WARN("Encountered instruction class '%1' which is invalid for SPARC",
                 (int)lifted.iclass);
        break;
    }

    return true;
    /*

    )
            else if (last->getKind() == StmtType::Call) {
                // Check the delay slot of this call. First case of interest is when the
                // instruction is a restore, e.g.
                // 142c8:  40 00 5b 91          call exit
                // 142cc:  91 e8 3f ff          restore %g0, -1, %o0
                if (m_decoder->isSPARCRestore(delayInsn)) {
                    // Give the address of the call; I think that this is actually important, if
                    // faintly annoying
                    delayLifted.rtl->setAddress(addr);
                    bbRTLs->push_back(std::move(delayLifted.rtl));

                    // The restore means it is effectively followed by a return (since the
                    // resore semantics chop off one level of return address)
                    last->as<CallStatement>()->setReturnAfterCall(true);
                    sequentialDecode = false;
                    case_CALL(addr, lifted, nop_inst, bbRTLs, proc, callList, true);
                    break;
                }

                // Next class of interest is if it assigns to %o7 (could be a move, add, and
                // possibly others). E.g.:
                // 14e4c:  82 10 00 0f      mov     %o7, %g1
                // 14e50:  7f ff ff 60      call    blah
                // 14e54:  9e 10 00 01      mov     %g1, %o7
                // Note there could be an unrelated instruction between the first move and the
                // call (move/x/call/move in UQBT terms). In Boomerang, we leave the semantics
                // of the moves there (to be likely removed by dataflow analysis) and merely
                // insert a return BB after the call. Note that if an add, there may be an
                // assignment to a temp register first. So look at last RT
                // TODO: why would delay_inst.rtl->empty() be empty here ?
                SharedStmt a = delayLifted.rtl->empty() ? nullptr : delayLifted.rtl->back();

                if (a && a->isAssign()) {
                    SharedExp lhs = a->as<Assign>()->getLeft();

                    if (lhs->isRegN(REG_SPARC_O7)) {
                        // If it's an add, this is special. Example:
                        //     call foo
                        //     add %o7, K, %o7
                        // is equivalent to call foo / ba .+K
                        SharedExp rhs = a->as<Assign>()->getRight();
                        auto o7(Location::regOf(REG_SPARC_O7));

                        if (rhs->getOper() == opPlus && rhs->access<Exp, 2>()->isIntConst() &&
                            *rhs->getSubExp1() == *o7) {
                            // Get the constant
                            const int K = rhs->access<Const, 2>()->getInt();
                            case_CALL(addr, lifted, delayLifted, bbRTLs, proc, callList, true);

                            // We don't generate a goto; instead, we just decode from the new
                            // address Note: the call to case_CALL has already incremented
                            // address by 8, so don't do again
                            addr += K;
                            break;
                        }
                        else {
                            // We assume this is some sort of move/x/call/move pattern. The
                            // overall effect is to pop one return address, we we emit a return
                            // after this call
                            last->as<CallStatement>()->setReturnAfterCall(true);
                            sequentialDecode = false;
                            case_CALL(addr, lifted, delayLifted, bbRTLs, proc, callList, true);
                            break;
                        }
                    }
                }
            }

            const RTL *delayRTL = delayLifted.rtl.get();

            switch (delayLifted.iclass) {
            case IClass::NOP:
            case IClass::NCT:

                // Ordinary delayed instruction. Since NCT's can't affect unconditional jumps,
                // we put the delay instruction before the jump or call
                if (last->getKind() == StmtType::Call) {
                    // This is a call followed by an NCT/NOP
                    sequentialDecode = case_CALL(addr, lifted, delayLifted, bbRTLs, proc,
                                                 callList);
                }
                else {
                    // This is a non-call followed by an NCT/NOP
                    case_SD(pc, m_program->getBinaryFile()->getImage()->getTextDelta(),
                            m_program->getBinaryFile()->getImage()->getLimitText(), lifted,
                            delayLifted, std::move(BB_rtls), cfg, _targetQueue);

                    // There is no fall through branch.
                    sequentialDecode = false;
                }

                break;

            case IClass::SKIP:
                case_unhandled_stub(addr);
                addr += 2 * SPARC_INSTRUCTION_LENGTH;
                break;

            case IClass::SU: {
                // SD/SU.
                // This will be either BA or CALL followed by BA,A. Our interpretation is that
                // it is as if the SD (i.e. the BA or CALL) now takes the destination of the SU
                // (i.e. the BA,A). For example:
                //       call 1000, ba,a 2000
                // is really like:
                //       call 2000.

                // Just so that we can check that our interpretation is correct the first time
                // we hit this case...
                case_unhandled_stub(addr);

                // Adjust the destination of the SD and emit it.
                std::shared_ptr<const GotoStatement> delayJump = delayRTL->back()
                                                                     ->as<GotoStatement>();
                const Address dest = addr + SPARC_INSTRUCTION_LENGTH + delayJump->getFixedDest();
                jumpStmt->setDest(dest);
                bbRTLs->push_back(std::move(lifted.rtl));

                // Create the appropriate BB
                if (last->getKind() == StmtType::Call) {
                    BasicBlock *newBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                    assert(newBB);

                    createCallToAddress(dest, pc, newBB, cfg, 2 * SPARC_INSTRUCTION_LENGTH);
                    addr += 2 * SPARC_INSTRUCTION_LENGTH;

                    // Add this call site to the set of call sites which need to be analyzed
                    // later.
                    callList.push_back(lifted.rtl->back()->as<CallStatement>());
                }
                else {
                    BasicBlock *newBB = cfg->createBB(BBType::Oneway, std::move(BB_rtls));
                    assert(newBB);
                    createJumpToAddress(dest, newBB, cfg, _targetQueue,
                                        m_program->getBinaryFile()->getImage()->getLimitText());

                    // There is no fall through branch.
                    sequentialDecode = false;
                }

                break;
            }

            default:
                case_unhandled_stub(addr);
                addr += 2 * SPARC_INSTRUCTION_LENGTH; // Skip the pair
                break;
            }

            break;
        }*/

    //             case IClass::DD: {
    //                 MachineInstruction delayInsn;
    //                 DecodeResult delayLifted;
    //                 if (!decodeInstruction(addr + SPARC_INSTRUCTION_LENGTH, delayInsn,
    //                 delayLifted)) {
    //                     warnInvalidInstruction(addr + SPARC_INSTRUCTION_LENGTH);
    //                     sequentialDecode = false;
    //                     continue;
    //                 }
    //
    //                 switch (delayLifted.iclass) {
    //                 case IClass::NOP:
    //                 case IClass::NCT:
    //                     sequentialDecode = case_DD(
    //                                            addr,
    //                                            m_program->getBinaryFile()->getImage()->getTextDelta(),
    //                                            lifted,
    //                         delayLifted, std::move(bbRTLs), _targetQueue, proc, callList);
    //                     break;
    //
    //                 default: case_unhandled_stub(addr); break;
    //                 }
    //
    //                 break;
    //             }
    //
    //             case IClass::SCD: {
    //                 // Always execute the delay instr, and branch if condition is met.
    //                 // Normally, the delayed instruction moves in front of the branch. But if it
    //                 affects
    //                 // the condition codes, we may have to duplicate it as an orphan in the true
    //                 leg of
    //                 // the branch, and fall through to the delay instruction in the "false" leg.
    //                 Instead
    //                 // of moving the delay instruction to an orphan BB, we may have a duplicate
    //                 of the
    //                 // delay instruction just before the target; if so, we can branch to that and
    //                 not
    //                 // need the orphan.  We do just a binary comparison; that may fail to make
    //                 this
    //                 // optimisation if the instr has relative fields.
    //
    //                 MachineInstruction delayInsn;
    //                 DecodeResult delayLifted;
    //                 if (!decodeInstruction(addr + SPARC_INSTRUCTION_LENGTH, delayInsn,
    //                 delayLifted)) {
    //                     warnInvalidInstruction(addr + SPARC_INSTRUCTION_LENGTH);
    //                     sequentialDecode = false;
    //                     continue;
    //                 }
    //
    //                 switch (delayLifted.iclass) {
    //                 case IClass::NOP:
    //                 case IClass::NCT:
    //                     sequentialDecode = case_SCD(
    //                         pc, m_program->getBinaryFile()->getImage()->getTextDelta(),
    //                         m_program->getBinaryFile()->getImage()->getLimitText(), lifted,
    //                         delayLifted, std::move(BB_rtls), cfg, _targetQueue);
    //                     break;
    //
    //                 default:
    //
    //                     if (delayLifted.rtl->back()->getKind() == StmtType::Call) {
    //                         // Assume it's the move/call/move pattern
    //                         sequentialDecode = case_SCD(
    //                             pc, m_program->getBinaryFile()->getImage()->getTextDelta(),
    //                             m_program->getBinaryFile()->getImage()->getLimitText(), lifted,
    //                             delayLifted, std::move(BB_rtls), cfg, _targetQueue);
    //                         break;
    //                     }
    //
    //                     case_unhandled_stub(addr);
    //                     break;
    //                 }
    //
    //                 break;
    //             }
    //
    //             case IClass::SCDAN: {
    //                 // Execute the delay instruction if the branch is taken; skip (anull) the
    //                 delay
    //                 // instruction if branch not taken.
    //                 MachineInstruction delayInsn;
    //                 DecodeResult delayLifted;
    //                 if (!decodeInstruction(addr + SPARC_INSTRUCTION_LENGTH, delayInsn,
    //                 delayLifted)) {
    //                     warnInvalidInstruction(addr + SPARC_INSTRUCTION_LENGTH);
    //                     sequentialDecode = false;
    //                     continue;
    //                 }
    //
    //                 switch (delayLifted.iclass) {
    //                 case IClass::NOP: {
    //                     // This is an ordinary two-way branch. Add the branch to the list of RTLs
    //                     for
    //                     // this BB
    //                     bbRTLs->push_back(std::move(lifted.rtl));
    //                     // Create the BB and add it to the CFG
    //                     BasicBlock *newBB = cfg->createBB(BBType::Twoway, std::move(BB_rtls));
    //                     assert(newBB);
    //
    //                     // Visit the destination of the branch; add "true" leg
    //                     const Address jumpDest = jumpStmt ? jumpStmt->getFixedDest() :
    //                     Address::INVALID; createJumpToAddress(jumpDest, newBB, cfg, _targetQueue,
    //                                         m_program->getBinaryFile()->getImage()->getLimitText());
    //
    //                     // Add the "false" leg: point past the delay inst
    //                     cfg->addEdge(newBB, pc + 8);
    //                     addr += 2 * SPARC_INSTRUCTION_LENGTH; // Skip branch and delay
    //                     break;
    //                 }
    //
    //                 case IClass::NCT:
    //                     sequentialDecode = case_SCDAN(
    //                         pc, m_program->getBinaryFile()->getImage()->getTextDelta(),
    //                         m_program->getBinaryFile()->getImage()->getLimitText(), lifted,
    //                         delayLifted, std::move(BB_rtls), cfg, _targetQueue);
    //                     break;
    //
    //                 default:
    //                     case_unhandled_stub(addr);
    //                     addr += 2 * SPARC_INSTRUCTION_LENGTH;
    //                     break;
    //                 }
    //
    //                 break;
    //             }
    //
    //             default: // Others are non SPARC cases
    //                 LOG_WARN("Encountered instruction class '%1' which is invalid for SPARC",
    //                          (int)lifted.iclass);
    //                 break;
    //             }
}


bool SPARCFrontEnd::canOptimizeDelayCopy(Address src, Address dest, ptrdiff_t delta,
                                         Interval<Address> textLimit) const
{
    // Check that the destination is within the main test section; may not be when we speculatively
    // decode junk
    if (!textLimit.contains(dest - SPARC_INSTRUCTION_LENGTH)) {
        return false;
    }

    const DWord delay_inst       = *reinterpret_cast<const DWord *>(src.value() + 4 + delta);
    const DWord inst_before_dest = *reinterpret_cast<const DWord *>(dest.value() - 4 + delta);

    return delay_inst == inst_before_dest;
}


BasicBlock *SPARCFrontEnd::optimizeCallReturn(std::shared_ptr<CallStatement> /*call*/,
                                              const RTL * /*rtl*/, const RTL * /*delay*/,
                                              UserProc * /*proc*/)
{
    //     if (call->isReturnAfterCall()) {
    //         // The only RTL in the basic block is a ReturnStatement
    //         std::list<SharedStmt> ls;
    //
    //         // If the delay slot is a single assignment to %o7, we want to see the semantics for
    //         it, so
    //         // that preservation or otherwise of %o7 is correct
    //         if (delay && (delay->size() == 1) && delay->front()->isAssign() &&
    //             delay->front()->as<Assign>()->getLeft()->isRegN(REG_SPARC_O7)) {
    //             ls.push_back(delay->front()->clone());
    //         }
    //
    //         ls.push_back(std::make_shared<ReturnStatement>());
    //
    //         // Constuct the RTLs for the new basic block
    //         std::unique_ptr<RTLList> rtls(new RTLList);
    //         BasicBlock *returnBB = createReturnBlock(
    //             proc, std::move(rtls), std::unique_ptr<RTL>(new RTL(rtl->getAddress() + 1,
    //             &ls)));
    //         return returnBB;
    //     }
    //     else {
    // May want to put code here that checks whether or not the delay instruction redefines %o7
    return nullptr;
    //     }
}


void SPARCFrontEnd::createJumpToAddress(Address /*dest*/, BasicBlock *& /*newBB*/,
                                        ProcCFG * /*cfg*/, TargetQueue & /*tq*/,
                                        Interval<Address> /*textLimit*/)
{
    //     if (!textLimit.contains(dest)) {
    //         LOG_ERROR("Branch to address %1 is beyond section limits", dest);
    //         return;
    //     }
    //     else if (newBB == nullptr) {
    //         return;
    //     }
    //
    //     tq.pushAddress(cfg, dest, newBB);
    //     cfg->addEdge(newBB, dest);
}


void SPARCFrontEnd::createCallToAddress(Address /*dest*/, Address /*address*/,
                                        BasicBlock * /*callBB*/, ProcCFG * /*cfg*/,
                                        int /*offset*/ /* = 0*/)
{
    //     if (callBB == nullptr) {
    //         return;
    //     }
    //
    //     const Prog *prog = cfg->getProc()->getProg();
    //
    //     // If the destination address is the same as this very instruction,
    //     // we have a call with iDisp30 == 0. Don't treat this as the start of a real procedure.
    //     if (dest != address && prog->getFunctionByAddr(dest) == nullptr) {
    //         // We don't want to call prog.visitProc just yet, in case this is a speculative
    //         decode that
    //         // failed. Instead, we use the set of CallStatements (not in this procedure) that is
    //         needed
    //         // by CSR
    //         if (m_program->getProject()->getSettings()->traceDecoder) {
    //             LOG_VERBOSE("p%1", dest);
    //         }
    //     }
    //
    //     // Add the out edge if required
    //     if (offset != 0) {
    //         cfg->addEdge(callBB, address + offset);
    //     }
}


void SPARCFrontEnd::case_unhandled_stub(Address addr)
{
    LOG_ERROR("Unhandled DCTI couple at address %1", addr);
}


bool SPARCFrontEnd::case_CALL(Address &, DecodeResult &, DecodeResult &, std::unique_ptr<RTLList> &,
                              UserProc *, std::list<std::shared_ptr<CallStatement>> &,
                              bool /* = false*/)
{
    //     // Aliases for the call and delay RTLs
    //     std::shared_ptr<CallStatement> callStmt = inst.rtl->back()->as<CallStatement>();
    //     RTL *delayRTL                           = delayInst.rtl.get();
    //
    //     // Emit the delay instruction, unless the delay instruction is a nop, or we have a
    //     pattern, or
    //     // are followed by a restore
    //     if (delayInst.iclass != IClass::NOP && !callStmt->isReturnAfterCall()) {
    //         delayRTL->setAddress(address);
    //         BB_rtls->push_back(std::move(delayInst.rtl));
    //     }
    //
    //     // Get the new return basic block for the special case where the delay instruction is a
    //     restore BasicBlock *returnBB = optimizeCallReturn(callStmt, inst.rtl.get(), delayRTL,
    //     proc);
    //
    //     int disp30 = (callStmt->getFixedDest().value() - address.value()) >> 2;
    //
    //     // Don't test for small offsets if part of a move_call_move pattern.
    //     // These patterns assign to %o7 in the delay slot, and so can't possibly be used to copy
    //     %pc to
    //     // %o7 Similarly if followed by a restore
    //     if (!isPattern && (returnBB == nullptr) && ((disp30 == 2) || (disp30 == 3))) {
    //         // This is the idiomatic case where the destination is 1 or 2 instructions after the
    //         delayed
    //         // instruction. Only emit the side effect of the call (%o7 := %pc) in this case. Note
    //         that
    //         // the side effect occurs before the delay slot instruction (which may use the value
    //         of %o7) emitCopyPC(*BB_rtls, address); address += disp30 << 2; return true;
    //     }
    //     else {
    //         assert(disp30 != 1);
    //
    //         // First check for helper functions
    //         const Address dest = callStmt->getFixedDest();
    //
    //         if (isHelperFunc(dest, address, *BB_rtls)) {
    //             address += 8; // Skip call, delay slot
    //             return true;
    //         }
    //
    //         // Emit the call
    //         RTL *rtl = inst.rtl.get();
    //         BB_rtls->push_back(std::move(inst.rtl));
    //
    //         // End the current basic block
    //         ProcCFG *cfg       = proc->getCFG();
    //         BasicBlock *callBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
    //
    //         if (callBB == nullptr) {
    //             return false;
    //         }
    //
    //         // Add this call site to the set of call sites which need to be analysed later.
    //         // This set will be used later to call prog.visitProc (so the proc will get decoded)
    //         callList.push_back(rtl->back()->as<CallStatement>());
    //
    //         if (returnBB) {
    //             // Handle the call but don't add any outedges from it just yet.
    //             createCallToAddress(callStmt->getFixedDest(), address, callBB, cfg);
    //
    //             // Now add the out edge
    //             cfg->addEdge(callBB, returnBB);
    //
    //             address += SPARC_INSTRUCTION_LENGTH;
    //             // This is a CTI block that doesn't fall through
    //             // and so we must stop decoding sequentially
    //             return false;
    //         }
    //         else {
    //             // Else no restore after this call.  An outedge may be added to the lexical
    //             successor of
    //             // the call which will be 8 bytes ahead or in the case where the callee returns a
    //             // struct, 12 bytes ahead. But the problem is: how do you know this function
    //             returns a
    //             // struct at decode time? If forceOutEdge is set, set offset to 0 and no out-edge
    //             will
    //             // be added yet
    //             // MVE: FIXME!
    //             int offset = 8;
    //             bool ret   = true;
    //
    //             // Check for _exit; probably should check for other "never return" functions
    //             QString name = m_program->getSymbolNameByAddr(dest);
    //
    //             if (name == "_exit") {
    //                 // Don't keep decoding after this call
    //                 ret = false;
    //                 // Also don't add an out-edge; setting offset to 0 will do this
    //                 offset = 0;
    //                 // But we have already set the number of out-edges to 1
    //                 callBB->setType(BBType::Call);
    //             }
    //
    //             // Handle the call (register the destination as a proc) and possibly set the
    //             outedge. createCallToAddress(dest, address, callBB, cfg, offset);
    //
    //             // Continue decoding from the lexical successor
    //             address += offset;
    //
    //             return ret;
    //         }
    //     }
    return false;
}


void SPARCFrontEnd::case_SD(Address &pc, ptrdiff_t delta, Interval<Address> textLimit,
                            DecodeResult &inst, DecodeResult &delay_inst,
                            std::unique_ptr<RTLList> BB_rtls, ProcCFG *cfg, TargetQueue &tq)
{
    // Aliases for the SD and delay RTLs
    std::shared_ptr<GotoStatement> SD_stmt = inst.rtl->back()->as<GotoStatement>();
    RTL *delay_rtl                         = delay_inst.rtl.get();

    // Try the "delay instruction has been copied" optimisation,
    // emitting the delay instruction now if the optimisation won't apply
    if (delay_inst.iclass != IClass::NOP) {
        if (canOptimizeDelayCopy(pc, SD_stmt->getFixedDest(), delta, textLimit)) {
            SD_stmt->adjustFixedDest(-4);
        }
        else {
            // Move the delay instruction before the SD. Must update the address in case there is a
            // branch to the SD
            delay_rtl->setAddress(pc);
            BB_rtls->push_back(std::move(delay_inst.rtl));
        }
    }

    // Update the address (for coverage)
    pc += 2 * SPARC_INSTRUCTION_LENGTH;

    // Add the SD
    BB_rtls->push_back(std::move(inst.rtl));

    // Add the one-way branch BB
    BasicBlock *newBB = nullptr; // cfg->createBB(BBType::Oneway, std::move(BB_rtls));

    if (newBB != nullptr) {
        // Visit the destination, and add the out-edge
        Address jumpDest = SD_stmt->getFixedDest();
        createJumpToAddress(jumpDest, newBB, cfg, tq, textLimit);
    }
}


bool SPARCFrontEnd::case_DD(Address & /*address*/, ptrdiff_t, DecodeResult & /*inst*/,
                            DecodeResult & /*delay_inst*/, std::unique_ptr<RTLList> /*BB_rtls*/,
                            TargetQueue &, UserProc * /*proc*/,
                            std::list<std::shared_ptr<CallStatement>> & /*callList*/)
{
    //     ProcCFG *cfg  = proc->getCFG();
    //     RTL *rtl      = inst.rtl.get();
    //     RTL *delayRTL = delay_inst.rtl.get();
    //
    //     if (delay_inst.iclass != IClass::NOP) {
    //         // Emit the delayed instruction, unless a NOP
    //         delayRTL->setAddress(address);
    //         BB_rtls->push_back(std::move(delay_inst.rtl));
    //     }
    //
    //     // Set address past this instruction and delay slot (may be changed later).  This in so
    //     that we
    //     // cover the jmp/call and delay slot instruction, in case we return false
    //     address += 8;
    //
    //     BasicBlock *newBB;
    //     bool isRetOrCase    = false;
    //     SharedStmt lastStmt = rtl->back();
    //
    //     switch (lastStmt->getKind()) {
    //     case StmtType::Call:
    //         // Will be a computed call
    //         BB_rtls->push_back(std::move(inst.rtl));
    //         newBB = cfg->createBB(BBType::CompCall, std::move(BB_rtls));
    //         break;
    //
    //     case StmtType::Ret:
    //         newBB       = createReturnBlock(proc, std::move(BB_rtls), std::move(inst.rtl));
    //         isRetOrCase = true;
    //         break;
    //
    //     case StmtType::Case: {
    //         BB_rtls->push_back(std::move(inst.rtl));
    //         newBB              = cfg->createBB(BBType::CompJump, std::move(BB_rtls));
    //         BB_rtls            = nullptr;
    //         isRetOrCase        = true;
    //         SharedExp jumpDest = lastStmt->as<CaseStatement>()->getDest();
    //
    //         if (jumpDest == nullptr) { // Happens if already analysed (we are now redecoding)
    //             // processSwitch will update the BB type and number of outedges, decode arms, set
    //             out
    //             // edges, etc.
    //             IndirectJumpAnalyzer().processSwitch(newBB, proc);
    //         }
    //
    //         break;
    //     }
    //
    //     default: newBB = nullptr; break;
    //     }
    //
    //     if (newBB == nullptr) {
    //         return false;
    //     }
    //
    //     SharedStmt last = newBB->getIR()->getLastRTL()->back();
    //
    //     // Do extra processing for for special types of DD
    //     if (last->getKind() == StmtType::Call) {
    //         // Attempt to add a return BB if the delay instruction is a RESTORE
    //         std::shared_ptr<CallStatement> call_stmt = last->as<CallStatement>();
    //         BasicBlock *returnBB = optimizeCallReturn(call_stmt, rtl, delayRTL, proc);
    //
    //         if (returnBB != nullptr) {
    //             cfg->addEdge(newBB, returnBB);
    //
    //             // We have to set the epilogue for the enclosing procedure (all proc's must have
    //             an
    //             // epilogue) and remove the RESTORE in the delay slot that has just been pushed
    //             to the
    //             // list of RTLs proc->setEpilogue(new
    //             CalleeEpilogue("__dummy",std::list<QString>()));
    //             // Set the return location; this is now always %o0
    //             // setReturnLocations(proc->getEpilogue(), 8 /* %o0 */);
    //             newBB->getIR()->removeRTL(delayRTL);
    //
    //             // Add this call to the list of calls to analyse. We won't be able to analyse its
    //             // callee(s), of course.
    //             callList.push_back(call_stmt);
    //
    //             return false;
    //         }
    //         else {
    //             // Instead, add the standard out edge to original address+8 (now just address)
    //             cfg->addEdge(newBB, address);
    //         }
    //
    //         // Add this call to the list of calls to analyse. We won't be able to analyse its
    //         callee(s),
    //         // of course.
    //         callList.push_back(call_stmt);
    //     }
    //
    //     // Set the address of the lexical successor of the call that is to be decoded next and
    //     create a
    //     // new list of RTLs for the next basic block.
    //     assert(BB_rtls == nullptr);
    //     return !isRetOrCase;
    return true;
}


bool SPARCFrontEnd::case_SCD(Address &address, ptrdiff_t delta, Interval<Address> textLimit,
                             DecodeResult &inst, DecodeResult &delay_inst,
                             std::unique_ptr<RTLList> BB_rtls, ProcCFG *cfg, TargetQueue &tq)
{
    std::shared_ptr<GotoStatement> jumpStmt = inst.rtl->back()->as<GotoStatement>();
    Address jumpDest                        = jumpStmt->getFixedDest();

    // Assume that if we find a call in the delay slot, it's actually a pattern such as
    // move/call/move MVE: Check this! Only needed for HP PA/RISC
    bool delayPattern = delay_inst.rtl->isCall();

    if (delayPattern) {
        // Just emit the branch, and decode the instruction immediately following next.
        // Assumes the first instruction of the pattern is not used in the true leg
        BB_rtls->push_back(std::move(inst.rtl));
        BasicBlock *newBB = nullptr; // cfg->createBB(BBType::Twoway, std::move(BB_rtls));

        if (newBB == nullptr) {
            return false;
        }

        createJumpToAddress(jumpDest, newBB, cfg, tq, textLimit);
        // Add the "false" leg
        //         cfg->addEdge(newBB, address + SPARC_INSTRUCTION_LENGTH);
        address += SPARC_INSTRUCTION_LENGTH; // Skip the SCD only
        // Start a new list of RTLs for the next BB
        BB_rtls = nullptr;
        LOG_WARN("Instruction at address %1 not copied to true leg of preceding branch", address);
        return true;
    }

    // If delay_insn decoded to empty list ( NOP) or if it isn't a flag assign => Put delay inst
    // first
    if (delay_inst.rtl->empty() || !delay_inst.rtl->back()->isFlagAssign()) {
        if (delay_inst.iclass != IClass::NOP) {
            // Emit delay instr
            // This is in case we have an in-edge to the branch. If the BB is split, we want the
            // split to happen here, so this delay instruction is active on this path
            delay_inst.rtl->setAddress(address);

            BB_rtls->push_back(std::move(delay_inst.rtl));
        }

        // Now emit the branch BB
        BB_rtls->push_back(std::move(inst.rtl));
        BasicBlock *newBB = nullptr; // cfg->createBB(BBType::Twoway, std::move(BB_rtls));

        if (newBB == nullptr) {
            return false;
        }

        // "taken" branch
        createJumpToAddress(jumpDest, newBB, cfg, tq, textLimit);

        // Skip the NCT/NOP instruction (we already emitted it)
        //         cfg->addEdge(newBB, address + 8);
        address += 8;
    }
    else if (canOptimizeDelayCopy(address, jumpDest, delta, textLimit)) {
        // We can just branch to the instr before jumpDest. Adjust the destination of the branch
        jumpStmt->adjustFixedDest(-4);
        // Now emit the branch
        BB_rtls->push_back(std::move(inst.rtl));
        BasicBlock *newBB = nullptr; // cfg->createBB(BBType::Twoway, std::move(BB_rtls));
        assert(newBB);

        createJumpToAddress(jumpDest - 4, newBB, cfg, tq, textLimit);
        // Add the "false" leg: point to the delay inst
        //         cfg->addEdge(newBB, address + 4);
        address += 4; // Skip branch but not delay
    }
    else { // The CCs are affected, and we can't use the copy delay slot trick
        // SCD, must copy delay instr to orphan
        // Copy the delay instruction to the dest of the branch, as an orphan. First add the branch.
        BB_rtls->push_back(std::move(inst.rtl));
        // Make a BB for the current list of RTLs. We want to do this first, else ordering can go
        // silly
        //         BasicBlock *newBB = cfg->createBB(BBType::Twoway, std::move(BB_rtls));
        //         assert(newBB);

        // Visit the target of the branch
        //         tq.pushAddress(cfg, jumpDest, newBB);
        std::unique_ptr<RTLList> orphanBBRTLs(new RTLList);

        // Add a branch from the orphan instruction to the dest of the branch.
        delay_inst.rtl->append(std::make_shared<GotoStatement>(jumpDest));
        orphanBBRTLs->push_back(std::move(delay_inst.rtl));

        //         BasicBlock *orphanBB = cfg->createBB(BBType::Oneway, std::move(orphanBBRTLs));
        //
        //         // Add an out edge from the orphan as well
        //         cfg->addEdge(orphanBB, jumpDest);
        //
        //         // Add an out edge from the current RTL to the orphan. Put a label at the orphan
        //         cfg->addEdge(newBB, orphanBB);
        //
        //         // Add the "false" leg to the NCT
        //         cfg->addEdge(newBB, address + 4);
        // Don't skip the delay instruction, so it will be decoded next.
        address += 4;
    }

    assert(BB_rtls == nullptr);
    return true;
}


bool SPARCFrontEnd::case_SCDAN(Address &address, ptrdiff_t delta, Interval<Address> textLimit,
                               DecodeResult &inst, DecodeResult &delayInst,
                               std::unique_ptr<RTLList> BB_rtls, ProcCFG *cfg, TargetQueue &tq)
{
    // We may have to move the delay instruction to an orphan BB, which then branches to the target
    // of the jump. Instead of moving the delay instruction to an orphan BB, we may have a duplicate
    // of the delay instruction just before the target; if so, we can branch to that and not need
    // the orphan. We do just a binary comparison; that may fail to make this optimisation if the
    // instr has relative fields.
    std::shared_ptr<GotoStatement> jumpStmt = inst.rtl->back()->as<GotoStatement>();
    Address jumpDest                        = jumpStmt->getFixedDest();
    BasicBlock *newBB                       = nullptr;

    if (canOptimizeDelayCopy(address, jumpDest, delta, textLimit)) {
        // Adjust the destination of the branch
        jumpStmt->adjustFixedDest(-4);
        // Now emit the branch
        BB_rtls->push_back(std::move(inst.rtl));
        newBB = nullptr; // cfg->createBB(BBType::Twoway, std::move(BB_rtls));
        assert(newBB);

        createJumpToAddress(jumpDest - 4, newBB, cfg, tq, textLimit);
    }
    else { // SCDAN; must move delay instr to orphan. Assume it's not a NOP (though if it is, no
           // harm done)
        // Move the delay instruction to the dest of the branch, as an orphan. First add the branch.
        BB_rtls->push_back(std::move(inst.rtl));

        // Make a BB for the current list of RTLs.  We want to do this first, else ordering can go
        // silly
        newBB = nullptr; // cfg->createBB(BBType::Twoway, std::move(BB_rtls));
        assert(newBB);

        // Visit the target of the branch
        //         tq.pushAddress(cfg, jumpDest, newBB);

        std::unique_ptr<RTLList> orphanRTL(new RTLList);

        // Also add a branch from the orphan instruction to the dest of the branch
        delayInst.rtl->append(std::make_shared<GotoStatement>(jumpDest));
        orphanRTL->push_back(std::move(delayInst.rtl));

        //         BasicBlock *orphanBB = cfg->createBB(BBType::Oneway, std::move(orphanRTL));
        //
        //         // Add an out edge from the orphan as well. Set a label there.
        //         cfg->addEdge(orphanBB, jumpDest);
        //
        //         // Add an out edge from the current RTL to
        //         // the orphan. Set a label there.
        //         cfg->addEdge(newBB, orphanBB);
    }

    //     // Both cases (orphan or not)
    //     // Add the "false" leg: point past delay inst. Set a label there (see below)
    //     cfg->addEdge(newBB, address + 8);

    // Could need a jump to the following BB, e.g. if jumpDest is the delay slot instruction itself!
    // e.g. beq,a $+8

    address += 8; // Skip branch and delay
    return true;
}


void SPARCFrontEnd::emitNop(RTLList &rtls, Address addr)
{
    // Emit a null RTL with the given address. Required to cope with
    // SKIP instructions. Yes, they really happen, e.g. /usr/bin/vi 2.5
    rtls.push_back(std::unique_ptr<RTL>(new RTL(addr)));
}


void SPARCFrontEnd::emitCopyPC(RTLList &rtls, Address addr)
{
    // Emit %o7 = %pc
    std::shared_ptr<Assign> asgn(new Assign(Location::regOf(REG_SPARC_O7), Terminal::get(opPC)));
    assert(!rtls.empty());

    // Add the RTL to the list of RTLs, but to the second last position
    rtls.insert(std::prev(rtls.end()), std::unique_ptr<RTL>(new RTL(addr, { asgn })));
}


void SPARCFrontEnd::appendAssignment(const SharedExp &lhs, const SharedExp &rhs, SharedType type,
                                     Address addr, RTLList &lrtl)
{
    lrtl.push_back(
        std::unique_ptr<RTL>(new RTL(addr, { std::make_shared<Assign>(type, lhs, rhs) })));
}


void SPARCFrontEnd::quadOperation(Address addr, RTLList &lrtl, OPER op)
{
    SharedExp lhs = Location::memOf(
        Location::memOf(Binary::get(opPlus, Location::regOf(REG_SPARC_SP), Const::get(64))));
    SharedExp rhs = Binary::get(op, Location::memOf(Location::regOf(REG_SPARC_O0)),
                                Location::memOf(Location::regOf(REG_SPARC_O1)));

    appendAssignment(lhs, rhs, FloatType::get(128), addr, lrtl);
}


bool SPARCFrontEnd::isHelperFunc(Address dest, Address addr, RTLList &lrtl)
{
    const BinarySymbol *sym = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(dest);

    if (!(sym && sym->isImportedFunction())) {
        return false;
    }

    QString name = m_program->getSymbolNameByAddr(dest);

    if (name.isEmpty()) {
        LOG_ERROR("Can't find symbol for PLT address %1", dest);
        return false;
    }

    SharedExp rhs;

    if (name == ".umul") {
        // %o0 * %o1
        rhs = Binary::get(opMult, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".mul") {
        // %o0 *! %o1
        rhs = Binary::get(opMults, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".udiv") {
        // %o0 / %o1
        rhs = Binary::get(opDiv, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".div") {
        // %o0 /! %o1
        rhs = Binary::get(opDivs, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".urem") {
        // %o0 % %o1
        rhs = Binary::get(opMod, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".rem") {
        // %o0 %! %o1
        rhs = Binary::get(opMods, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
        //    } else if (name.substr(0, 6) == ".stret") {
        //        // No operation. Just use %o0
        //        rhs->push(idRegOf); rhs->push(idIntConst); rhs->push(8);
    }
    else if (name == "_Q_mul") {
        // Pointers to args are in %o0 and %o1; ptr to result at [%sp+64]
        // So semantics is m[m[r[14]] = m[r[8]] *f m[r[9]]
        quadOperation(addr, lrtl, opFMult);
        return true;
    }
    else if (name == "_Q_div") {
        quadOperation(addr, lrtl, opFDiv);
        return true;
    }
    else if (name == "_Q_add") {
        quadOperation(addr, lrtl, opFPlus);
        return true;
    }
    else if (name == "_Q_sub") {
        quadOperation(addr, lrtl, opFMinus);
        return true;
    }
    else {
        // Not a (known) helper function
        return false;
    }

    // Need to make an RTAssgn with %o0 = rhs
    SharedExp lhs = Location::regOf(REG_SPARC_O0);

    lrtl.push_back(std::unique_ptr<RTL>(new RTL(addr, { std::make_shared<Assign>(lhs, rhs) })));
    return true;
}


void SPARCFrontEnd::gen32op32gives64(OPER op, RTLList &lrtl, Address addr)
{
    std::unique_ptr<RTL::StmtList> ls(new RTL::StmtList);
#if V9_ONLY
    // tmp[tmpl] = sgnex(32, 64, r8) op sgnex(32, 64, r9)
    Statement *a = new Assign(
        64, Location::tempOf(Const::get("tmpl")),
        Binary::get(op, // opMult or opMults
                    new Ternary(opSgnEx, Const(32), Const(64), Location::regOf(REG_SPARC_O0)),
                    new Ternary(opSgnEx, Const(32), Const(64), Location::regOf(REG_SPARC_O1))));
    ls->push_back(a);
    // r8 = truncs(64, 32, tmp[tmpl]);
    a = new Assign(32, Location::regOf(REG_SPARC_O0),
                   new Ternary(opTruncs, Const::get(64), Const::get(32),
                               Location::tempOf(Const::get("tmpl"))));
    ls->push_back(a);
    // r9 = r[tmpl]@32:63;
    a = new Assign(
        32, Location::regOf(REG_SPARC_O1),
        new Ternary(opAt, Location::tempOf(Const::get("tmpl")), Const::get(32), Const::get(63)));
    ls->push_back(a);
#else
    // BTL: The .umul and .mul support routines are used in V7 code. We implsment these using the V8
    // UMUL and SMUL instructions. BTL: In SPARC V8, UMUL and SMUL perform 32x32 -> 64 bit
    // multiplies.
    //        The 32 high-order bits are written to %Y and the 32 low-order bits are written to
    //        r[rd]. This is also true on V9 although the high-order bits are also written into the
    //        32 high-order bits of the 64 bit r[rd].

    // r[tmp] = r8 op r9
    std::shared_ptr<Assign> a(
        new Assign(Location::tempOf(Const::get(const_cast<char *>("tmp"))),
                   Binary::get(op, // opMult or opMults
                               Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1))));
    ls->push_back(a);
    // r8 = r[tmp];     /* low-order bits */
    a = std::make_shared<Assign>(Location::regOf(REG_SPARC_O0),
                                 Location::tempOf(Const::get(const_cast<char *>("tmp"))));
    ls->push_back(a);
    // r9 = %Y;         /* high-order bits */
    a = std::make_shared<Assign>(Location::regOf(REG_SPARC_O0),
                                 Unary::get(opMachFtr, Const::get(const_cast<char *>("%Y"))));
    ls->push_back(a);
#endif /* V9_ONLY */

    lrtl.push_back(std::make_unique<RTL>(addr, ls.get()));
}


bool SPARCFrontEnd::helperFuncLong(Address dest, Address addr, RTLList &lrtl, QString &name)
{
    Q_UNUSED(dest);
    SharedExp rhs;
    SharedExp lhs;

    if (name == ".umul") {
        gen32op32gives64(opMult, lrtl, addr);
        return true;
    }
    else if (name == ".mul") {
        gen32op32gives64(opMults, lrtl, addr);
        return true;
    }
    else if (name == ".udiv") {
        // %o0 / %o1
        rhs = Binary::get(opDiv, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".div") {
        // %o0 /! %o1
        rhs = Binary::get(opDivs, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".urem") {
        // %o0 % %o1
        rhs = Binary::get(opMod, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
    }
    else if (name == ".rem") {
        // %o0 %! %o1
        rhs = Binary::get(opMods, Location::regOf(REG_SPARC_O0), Location::regOf(REG_SPARC_O1));
        //    } else if (name.substr(0, 6) == ".stret") {
        //        // No operation. Just use %o0
        //        rhs->push(idRegOf); rhs->push(idIntConst); rhs->push(8);
    }
    else if (name == "_Q_mul") {
        // Pointers to args are in %o0 and %o1; ptr to result at [%sp+64]
        // So semantics is m[m[r[14]] = m[r[8]] *f m[r[9]]
        quadOperation(addr, lrtl, opFMult);
        return true;
    }
    else if (name == "_Q_div") {
        quadOperation(addr, lrtl, opFDiv);
        return true;
    }
    else if (name == "_Q_add") {
        quadOperation(addr, lrtl, opFPlus);
        return true;
    }
    else if (name == "_Q_sub") {
        quadOperation(addr, lrtl, opFMinus);
        return true;
    }
    else {
        // Not a (known) helper function
        return false;
    }

    // Need to make an RTAssgn with %o0 = rhs
    lhs = Location::regOf(REG_SPARC_O0);
    appendAssignment(lhs, rhs, IntegerType::get(32), addr, lrtl);
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
