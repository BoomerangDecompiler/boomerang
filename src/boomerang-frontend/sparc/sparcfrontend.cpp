/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       sparcfrontend.cpp
 * \brief   This file contains routines to manage the decoding of sparc instructions and the instantiation to RTLs,
 *                removing sparc dependent features such as delay slots in the process. These functions replace
 *                frontend.cpp for decoding sparc instructions.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/

#include "sparcfrontend.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/core/BinaryFileFactory.h" // E.g. IsDynamicallyLinkedProc
#include "boomerang/util/Log.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySymbols.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"

#include "boomerang-frontend/sparc/sparcdecoder.h"

#include <cassert>
#include <cstring>
#include <iomanip> // For setfill etc
#include <sstream>


void SparcFrontEnd::warnDCTcouple(Address uAt, Address uDest)
{
    LOG_STREAM() << "Error: DCTI couple at " << uAt << " points to delayed branch at " << uDest << "...\n";
    LOG_STREAM() << "Decompilation will likely be incorrect\n";
}


bool SparcFrontEnd::optimise_DelayCopy(Address src, Address dest, ptrdiff_t delta, Address uUpper)
{
    // Check that the destination is within the main test section; may not be when we speculatively decode junk
    if ((dest - 4) > uUpper) {
        return false;
    }

    unsigned delay_inst       = *((unsigned *)(src + 4 + delta).value());
    unsigned inst_before_dest = *((unsigned *)(dest - 4 + delta).value());
    return(delay_inst == inst_before_dest);
}


BasicBlock *SparcFrontEnd::optimise_CallReturn(CallStatement *call, RTL *rtl, RTL *delay, UserProc *pProc)
{
    if (call->isReturnAfterCall()) {
        // Constuct the RTLs for the new basic block
        std::list<RTL *> *rtls = new std::list<RTL *>();

        // The only RTL in the basic block is a ReturnStatement
        std::list<Instruction *> *ls = new std::list<Instruction *>;

        // If the delay slot is a single assignment to %o7, we want to see the semantics for it, so that preservation
        // or otherwise of %o7 is correct
        if ((delay->size() == 1) && delay->front()->isAssign() && ((Assign *)delay->front())->getLeft()->isRegN(15)) {
            ls->push_back(delay->front());
        }

        ls->push_back(new ReturnStatement);
        //        rtls->push_back(new RTL(rtl->getAddress() + 1, ls));
        //        Cfg* cfg = pProc->getCFG();
        //        BasicBlock* returnBB = cfg->newBB(rtls, RET, 0);
        BasicBlock *returnBB = createReturnBlock(pProc, rtls, new RTL(rtl->getAddress() + 1, ls));
        return returnBB;
    }
    else {
        // May want to put code here that checks whether or not the delay instruction redefines %o7
        return nullptr;
    }
}


void SparcFrontEnd::handleBranch(Address dest, Address hiAddress, BasicBlock *& newBB, Cfg *cfg, TargetQueue& tq)
{
    if (newBB == nullptr) {
        return;
    }

    if (dest < hiAddress) {
        tq.visit(cfg, dest, newBB);
        cfg->addOutEdge(newBB, dest, true);
    }
    else {
        LOG_STREAM() << "Error: branch to " << dest << " goes beyond section.\n";
    }
}


void SparcFrontEnd::handleCall(UserProc *proc, Address dest, BasicBlock *callBB, Cfg *cfg, Address address,
                               int offset /* = 0*/)
{
    if (callBB == nullptr) {
        return;
    }

    // If the destination address is the same as this very instruction, we have a call with iDisp30 == 0. Don't treat
    // this as the start of a real procedure.
    if ((dest != address) && (proc->getProg()->findProc(dest) == 0)) {
        // We don't want to call prog.visitProc just yet, in case this is a speculative decode that failed. Instead, we
        // use the set of CallStatements (not in this procedure) that is needed by CSR
        if (Boomerang::get()->traceDecoder) {
            LOG_STREAM() << "p" << dest << "\t";
        }
    }

    // Add the out edge if required
    if (offset != 0) {
        cfg->addOutEdge(callBB, address + offset);
    }
}


void SparcFrontEnd::case_unhandled_stub(Address addr)
{
    LOG_STREAM() << "Error: DCTI couple at " << addr << '\n';
}


bool SparcFrontEnd::case_CALL(Address& address, DecodeResult& inst, DecodeResult& delay_inst,
                              std::list<RTL *> *& BB_rtls, UserProc *proc, std::list<CallStatement *>& callList,
                              QTextStream& os, bool isPattern /* = false*/)
{
    // Aliases for the call and delay RTLs
    CallStatement *call_stmt = ((CallStatement *)inst.rtl->back());
    RTL           *delay_rtl = delay_inst.rtl;

    // Emit the delay instruction, unless the delay instruction is a nop, or we have a pattern, or are followed by a
    // restore
    if ((delay_inst.type != NOP) && !call_stmt->isReturnAfterCall()) {
        delay_rtl->setAddress(address);
        BB_rtls->push_back(delay_rtl);

        if (Boomerang::get()->printRtl) {
            delay_rtl->print(os);
        }
    }

    // Get the new return basic block for the special case where the delay instruction is a restore
    BasicBlock *returnBB = optimise_CallReturn(call_stmt, inst.rtl, delay_rtl, proc);

    int disp30 = (call_stmt->getFixedDest() - address).value() >> 2;

    // Don't test for small offsets if part of a move_call_move pattern.
    // These patterns assign to %o7 in the delay slot, and so can't possibly be used to copy %pc to %o7
    // Similarly if followed by a restore
    if (!isPattern && (returnBB == nullptr) && ((disp30 == 2) || (disp30 == 3))) {
        // This is the idiomatic case where the destination is 1 or 2 instructions after the delayed instruction.
        // Only emit the side effect of the call (%o7 := %pc) in this case.  Note that the side effect occurs before the
        // delay slot instruction (which may use the value of %o7)
        emitCopyPC(BB_rtls, address);
        address += disp30 << 2;
        return true;
    }
    else {
        assert(disp30 != 1);

        // First check for helper functions
        Address             dest  = call_stmt->getFixedDest();
        const IBinarySymbol *symb = SymbolTable->find(dest);

        // Special check for calls to weird PLT entries which don't have symbols
        if ((symb && symb->isImportedFunction()) && (m_program->getSymbolByAddress(dest) == nullptr)) {
            // This is one of those. Flag this as an invalid instruction
            inst.valid = false;
        }

        if (isHelperFunc(dest, address, BB_rtls)) {
            address += 8; // Skip call, delay slot
            return true;
        }

        // Emit the call
        BB_rtls->push_back(inst.rtl);

        // End the current basic block
        Cfg        *cfg    = proc->getCFG();
        BasicBlock *callBB = cfg->newBB(BB_rtls, BBType::Call, 1);

        if (callBB == nullptr) {
            return false;
        }

        // Add this call site to the set of call sites which need to be analysed later.
        // This set will be used later to call prog.visitProc (so the proc will get decoded)
        callList.push_back((CallStatement *)inst.rtl->back());

        if (returnBB) {
            // Handle the call but don't add any outedges from it just yet.
            handleCall(proc, call_stmt->getFixedDest(), callBB, cfg, address);

            // Now add the out edge
            cfg->addOutEdge(callBB, returnBB);
            // Put a label on the return BB; indicate that a jump is reqd
            cfg->setLabel(returnBB);
            callBB->setJumpRequired();

            address += inst.numBytes; // For coverage
            // This is a CTI block that doesn't fall through and so must
            // stop sequentially decoding
            return false;
        }
        else {
            // Else no restore after this call.  An outedge may be added to the lexical successor of the call which
            // will be 8 bytes ahead or in the case where the callee returns a struct, 12 bytes ahead. But the problem
            // is: how do you know this function returns a struct at decode time?
            // If forceOutEdge is set, set offset to 0 and no out-edge will be added yet
            int offset = !inst.forceOutEdge.isZero() ? 0 :
                         // (call_stmt->returnsStruct() ? 12 : 8);
                         // MVE: FIXME!
                         8;

            bool ret = true;
            // Check for _exit; probably should check for other "never return" functions
            QString name = m_program->getSymbolByAddress(dest);

            if (name == "_exit") {
                // Don't keep decoding after this call
                ret = false;
                // Also don't add an out-edge; setting offset to 0 will do this
                offset = 0;
                // But we have already set the number of out-edges to 1
                callBB->updateType(BBType::Call, 0);
            }

            // Handle the call (register the destination as a proc) and possibly set the outedge.
            handleCall(proc, dest, callBB, cfg, address, offset);

            if (!inst.forceOutEdge.isZero()) {
                // There is no need to force a goto to the new out-edge, since we will continue decoding from there.
                // If other edges exist to the outedge, they will generate the required label
                cfg->addOutEdge(callBB, inst.forceOutEdge);
                address = inst.forceOutEdge;
            }
            else {
                // Continue decoding from the lexical successor
                address += offset;
            }

            BB_rtls = nullptr;

            return ret;
        }
    }
}


void SparcFrontEnd::case_SD(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst,
                            DecodeResult& delay_inst, std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq,
                            QTextStream&)
{
    // Aliases for the SD and delay RTLs
    GotoStatement *SD_stmt   = static_cast<GotoStatement *>(inst.rtl->back());
    RTL           *delay_rtl = delay_inst.rtl;

    // Try the "delay instruction has been copied" optimisation, emitting the delay instruction now if the optimisation
    // won't apply
    if (delay_inst.type != NOP) {
        if (optimise_DelayCopy(address, SD_stmt->getFixedDest(), delta, hiAddress)) {
            SD_stmt->adjustFixedDest(-4);
        }
        else {
            // Move the delay instruction before the SD. Must update the address in case there is a branch to the SD
            delay_rtl->setAddress(address);
            BB_rtls->push_back(delay_rtl);
        }
    }

    // Update the address (for coverage)
    address += 8;

    // Add the SD
    BB_rtls->push_back(inst.rtl);

    // Add the one-way branch BB
    BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Oneway, 1);

    if (pBB == 0) {
        BB_rtls = nullptr;
        return;
    }

    // Visit the destination, and add the out-edge
       Address uDest = SD_stmt->getFixedDest();
    handleBranch(uDest, hiAddress, pBB, cfg, tq);
    BB_rtls = nullptr;
}


bool SparcFrontEnd::case_DD(Address& address, ptrdiff_t delta, DecodeResult& inst, DecodeResult& delay_inst,
                            std::list<RTL *> *& BB_rtls, TargetQueue& tq, UserProc *proc,
                            std::list<CallStatement *>& callList)
{
    Q_UNUSED(tq);
    Q_UNUSED(delta);
    Cfg *cfg = proc->getCFG();

    if (delay_inst.type != NOP) {
        // Emit the delayed instruction, unless a NOP
        delay_inst.rtl->setAddress(address);
        BB_rtls->push_back(delay_inst.rtl);
    }

    // Set address past this instruction and delay slot (may be changed later).  This in so that we cover the
    // jmp/call and delay slot instruction, in case we return false
    address += 8;

    BasicBlock  *newBB;
    bool        bRet      = true;
    Instruction *lastStmt = inst.rtl->back();

    switch (lastStmt->getKind())
    {
    case STMT_CALL:
        // Will be a computed call
        BB_rtls->push_back(inst.rtl);
        newBB = cfg->newBB(BB_rtls, BBType::CompCall, 1);
        break;

    case STMT_RET:
        //            newBB = cfg->newBB(BB_rtls, RET, 0);
        //            proc->setTheReturnAddr((ReturnStatement*)inst.rtl->back(), inst.rtl->getAddress());
        newBB = createReturnBlock(proc, BB_rtls, inst.rtl);
        bRet  = false;
        break;

    case STMT_CASE:
        {
            BB_rtls->push_back(inst.rtl);
            newBB = cfg->newBB(BB_rtls, BBType::CompJump, 0);
            bRet  = false;
            SharedExp pDest = ((CaseStatement *)lastStmt)->getDest();

            if (pDest == nullptr) { // Happens if already analysed (we are now redecoding)
                // SWITCH_INFO* psi = ((CaseStatement*)lastStmt)->getSwitchInfo();
                // processSwitch will update the BB type and number of outedges, decode arms, set out edges, etc
                newBB->processSwitch(proc);
            }

            break;
        }

    default:
        newBB = nullptr;
        break;
    }

    if (newBB == nullptr) {
        return false;
    }

    Instruction *last = inst.rtl->back();

    // Do extra processing for for special types of DD
    if (last->getKind() == STMT_CALL) {
        // Attempt to add a return BB if the delay instruction is a RESTORE
        CallStatement *call_stmt = (CallStatement *)(inst.rtl->back());
        BasicBlock    *returnBB  = optimise_CallReturn(call_stmt, inst.rtl, delay_inst.rtl, proc);

        if (returnBB != nullptr) {
            cfg->addOutEdge(newBB, returnBB);

            // We have to set the epilogue for the enclosing procedure (all proc's must have an
            // epilogue) and remove the RESTORE in the delay slot that has just been pushed to the list of RTLs
            // proc->setEpilogue(new CalleeEpilogue("__dummy",std::list<QString>()));
            // Set the return location; this is now always %o0
            // setReturnLocations(proc->getEpilogue(), 8 /* %o0 */);
            newBB->getRTLs()->remove(delay_inst.rtl);

            // Put a label on the return BB; indicate that a jump is reqd
            cfg->setLabel(returnBB);
            newBB->setJumpRequired();

            // Add this call to the list of calls to analyse. We won't be able to analyse its callee(s), of course.
            callList.push_back(call_stmt);

            return false;
        }
        else {
            // Instead, add the standard out edge to original address+8 (now just address)
            cfg->addOutEdge(newBB, address);
        }

        // Add this call to the list of calls to analyse. We won't be able to analyse its callee(s), of course.
        callList.push_back(call_stmt);
    }

    // Set the address of the lexical successor of the call that is to be decoded next and create a new list of
    // RTLs for the next basic block.
    BB_rtls = nullptr;
    return bRet;
}


bool SparcFrontEnd::case_SCD(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst,
                             DecodeResult& delay_inst, std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq)
{
    GotoStatement *stmt_jump = static_cast<GotoStatement *>(inst.rtl->back());
    Address       uDest      = stmt_jump->getFixedDest();

    // Assume that if we find a call in the delay slot, it's actually a pattern such as move/call/move
    // MVE: Check this! Only needed for HP PA/RISC
    bool delayPattern = delay_inst.rtl->isCall();

    if (delayPattern) {
        // Just emit the branch, and decode the instruction immediately following next.
        // Assumes the first instruction of the pattern is not used in the true leg
        BB_rtls->push_back(inst.rtl);
        BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

        if (pBB == 0) {
            return false;
        }

        handleBranch(uDest, hiAddress, pBB, cfg, tq);
        // Add the "false" leg
        cfg->addOutEdge(pBB, address + 4);
        address += 4; // Skip the SCD only
        // Start a new list of RTLs for the next BB
        BB_rtls = nullptr;
        LOG_STREAM() << "Warning: instruction at " << address << " not copied to true leg of preceeding branch\n";
        return true;
    }

    // If delay_insn decoded to empty list ( NOP) or if it isn't a flag assign => Put delay inst first
    if (delay_inst.rtl->empty() || !delay_inst.rtl->back()->isFlagAssign()) {
        if (delay_inst.type != NOP) {
            // Emit delay instr
            BB_rtls->push_back(delay_inst.rtl);
            // This is in case we have an in-edge to the branch. If the BB is split, we want the split to happen
            // here, so this delay instruction is active on this path
            delay_inst.rtl->setAddress(address);
        }

        // Now emit the branch
        BB_rtls->push_back(inst.rtl);
        BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

        if (pBB == 0) {
            return false;
        }

        handleBranch(uDest, hiAddress, pBB, cfg, tq);
        // Add the "false" leg; skips the NCT
        cfg->addOutEdge(pBB, address + 8);
        // Skip the NCT/NOP instruction
        address += 8;
    }
    else if (optimise_DelayCopy(address, uDest, delta, hiAddress)) {
        // We can just branch to the instr before uDest. Adjust the destination of the branch
        stmt_jump->adjustFixedDest(-4);
        // Now emit the branch
        BB_rtls->push_back(inst.rtl);
        BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

        if (pBB == 0) {
            return false;
        }

        handleBranch(uDest - 4, hiAddress, pBB, cfg, tq);
        // Add the "false" leg: point to the delay inst
        cfg->addOutEdge(pBB, address + 4);
        address += 4; // Skip branch but not delay
    }
    else {            // The CCs are affected, and we can't use the copy delay slot trick
        // SCD, must copy delay instr to orphan
        // Copy the delay instruction to the dest of the branch, as an orphan. First add the branch.
        BB_rtls->push_back(inst.rtl);
        // Make a BB for the current list of RTLs. We want to do this first, else ordering can go silly
        BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

        if (pBB == 0) {
            return false;
        }

        // Visit the target of the branch
        tq.visit(cfg, uDest, pBB);
        std::list<RTL *> *pOrphan = new std::list<RTL *>;
        pOrphan->push_back(delay_inst.rtl);
        // Change the address to 0, since this code has no source address (else we may branch to here when we want
        // to branch to the real BB with this instruction).
        // Note that you can't use an address that is a fixed function of the destination addr, because there can
        // be several jumps to the same destination that all require an orphan. The instruction in the orphan will
        // often but not necessarily be the same, so we can't use the same orphan BB. newBB knows to consider BBs
        // with address 0 as being in the map, so several BBs can exist with address 0
        delay_inst.rtl->setAddress(Address::ZERO);
        // Add a branch from the orphan instruction to the dest of the branch. Again, we can't even give the jumps
        // a special address like 1, since then the BB would have this getLowAddr.
        std::list<Instruction *> *gl = new std::list<Instruction *>;
        gl->push_back(new GotoStatement(uDest));
        pOrphan->push_back(new RTL(Address::ZERO, gl));
        BasicBlock *pOrBB = cfg->newBB(pOrphan, BBType::Oneway, 1);
        // Add an out edge from the orphan as well
        cfg->addOutEdge(pOrBB, uDest, true);
        // Add an out edge from the current RTL to the orphan. Put a label at the orphan
        cfg->addOutEdge(pBB, pOrBB, true);
        // Add the "false" leg to the NCT
        cfg->addOutEdge(pBB, address + 4);
        // Don't skip the delay instruction, so it will be decoded next.
        address += 4;
    }

    // Start a new list of RTLs for the next BB
    BB_rtls = nullptr;
    return true;
}


bool SparcFrontEnd::case_SCDAN(Address& address, ptrdiff_t delta, Address hiAddress, DecodeResult& inst,
                               DecodeResult& delay_inst, std::list<RTL *> *& BB_rtls, Cfg *cfg, TargetQueue& tq)
{
    // We may have to move the delay instruction to an orphan BB, which then branches to the target of the jump.
    // Instead of moving the delay instruction to an orphan BB, we may have a duplicate of the delay instruction just
    // before the target; if so, we can branch to that and not need the orphan. We do just a binary comparison; that
    // may fail to make this optimisation if the instr has relative fields.
    GotoStatement *stmt_jump = static_cast<GotoStatement *>(inst.rtl->back());
    Address       uDest      = stmt_jump->getFixedDest();
    BasicBlock    *pBB;

    if (optimise_DelayCopy(address, uDest, delta, hiAddress)) {
        // Adjust the destination of the branch
        stmt_jump->adjustFixedDest(-4);
        // Now emit the branch
        BB_rtls->push_back(inst.rtl);
        pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

        if (pBB == 0) {
            return false;
        }

        handleBranch(uDest - 4, hiAddress, pBB, cfg, tq);
    }
    else {   // SCDAN; must move delay instr to orphan. Assume it's not a NOP (though if it is, no harm done)
        // Move the delay instruction to the dest of the branch, as an orphan. First add the branch.
        BB_rtls->push_back(inst.rtl);
        // Make a BB for the current list of RTLs.  We want to do this first, else ordering can go silly
        pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

        if (pBB == 0) {
            return false;
        }

        // Visit the target of the branch
        tq.visit(cfg, uDest, pBB);
        std::list<RTL *> *pOrphan = new std::list<RTL *>;
        pOrphan->push_back(delay_inst.rtl);
        // Change the address to 0, since this code has no source address (else we may branch to here when we want to
        // branch to the real BB with this instruction).
        delay_inst.rtl->setAddress(Address::ZERO);
        // Add a branch from the orphan instruction to the dest of the branch
        std::list<Instruction *> *gl = new std::list<Instruction *>;
        gl->push_back(new GotoStatement(uDest));
        pOrphan->push_back(new RTL(Address::ZERO, gl));
        BasicBlock *pOrBB = cfg->newBB(pOrphan, BBType::Oneway, 1);
        // Add an out edge from the orphan as well. Set a label there.
        cfg->addOutEdge(pOrBB, uDest, true);
        // Add an out edge from the current RTL to
        // the orphan. Set a label there.
        cfg->addOutEdge(pBB, pOrBB, true);
    }

    // Both cases (orphan or not)
    // Add the "false" leg: point past delay inst. Set a label there (see below)
    cfg->addOutEdge(pBB, address + 8, true);
    // Could need a jump to the following BB, e.g. if uDest is the delay slot instruction itself! e.g. beq,a $+8
    pBB->setJumpRequired();
    address += 8;       // Skip branch and delay
    BB_rtls  = nullptr; // Start new BB return true;
    return true;
}


std::vector<SharedExp>& SparcFrontEnd::getDefaultParams()
{
    static std::vector<SharedExp> params;

    if (params.size() == 0) {
        // init arguments and return set to be all 31 machine registers
        // Important: because o registers are save in i registers, and
        // i registers have higher register numbers (e.g. i1=r25, o1=r9)
        // it helps the prover to process higher register numbers first!
        // But do r30 first (%i6, saves %o6, the stack pointer)
        params.push_back(Location::regOf(30));
        params.push_back(Location::regOf(31));

        for (int r = 29; r > 0; r--) {
            params.push_back(Location::regOf(r));
        }
    }

    return params;
}


std::vector<SharedExp>& SparcFrontEnd::getDefaultReturns()
{
    static std::vector<SharedExp> returns;

    if (returns.size() == 0) {
        returns.push_back(Location::regOf(30));
        returns.push_back(Location::regOf(31));

        for (int r = 29; r > 0; r--) {
            returns.push_back(Location::regOf(r));
        }
    }

    return returns;
}


bool SparcFrontEnd::processProc(Address uAddr, UserProc *proc, QTextStream& os, bool fragment /* = false */,
                                bool spec /* = false */)
{
    Q_UNUSED(fragment);
    // Declare an object to manage the queue of targets not yet processed yet.
    // This has to be individual to the procedure! (so not a global)
    TargetQueue _targetQueue;

    // Similarly, we have a set of CallStatement pointers. These may be
    // disregarded if this is a speculative decode that fails (i.e. an illegal
    // instruction is found). If not, this set will be used to add to the set
    // of calls to be analysed in the cfg, and also to call prog.visitProc()
    std::list<CallStatement *> callList;

    // Indicates whether or not the next instruction to be decoded is the
    // lexical successor of the current one. Will be true for all NCTs and for
    // CTIs with a fall through branch.
    bool sequentialDecode = true;

    // The control flow graph of the current procedure
    Cfg *cfg = proc->getCFG();

    // If this is a speculative decode, the second time we decode the same
    // address, we get no cfg. Else an error.
    if (spec && (cfg == 0)) {
        return false;
    }

    assert(cfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    _targetQueue.initial(uAddr);

    // Get the next address from which to continue decoding and go from
    // there. Exit the loop if there are no more addresses or they all
    // correspond to locations that have been decoded.
    while ((uAddr = _targetQueue.nextAddress(*cfg)) != Address::INVALID) {
        // The list of RTLs for the current basic block
        std::list<RTL *> *BB_rtls = new std::list<RTL *>();

        // Keep decoding sequentially until a CTI without a fall through branch
        // is decoded
        // ADDRESS start = address;
        DecodeResult inst;

        while (sequentialDecode) {
            // Decode and classify the current source instruction
            if (Boomerang::get()->traceDecoder) {
                LOG << "*" << uAddr << "\t";
            }

            // Check if this is an already decoded jump instruction (from a previous pass with propagation etc)
            // If so, we don't need to decode this instruction
            std::map<Address, RTL *>::iterator ff = m_previouslyDecoded.find(uAddr);

            if (ff != m_previouslyDecoded.end()) {
                inst.rtl   = ff->second;
                inst.valid = true;
                inst.type  = DD; // E.g. decode the delay slot instruction
            }
            else {
                decodeInstruction(uAddr, inst);
            }

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid) {
                return false;
            }

            // Check for invalid instructions
            if (!inst.valid) {
                LOG_STREAM() << "Invalid instruction at " << uAddr << ": ";
                ptrdiff_t delta = m_image->getTextDelta();

                const Byte* instructionData = (const Byte*)(uAddr + delta).value();
                for (int j = 0; j < inst.numBytes; j++) {
                    LOG_STREAM() << QString("0x%1").arg(instructionData[j], 2, 16, QChar('0')) << " ";
                }

                LOG_STREAM() << "\n";
                LOG_STREAM().flush();
                assert(false);
                return false;
            }

            // Don't display the RTL here; do it after the switch statement in case the delay slot instruction is moved
            // before this one

            // Need to construct a new list of RTLs if a basic block has just been finished but decoding is continuing
            // from its lexical successor
            if (BB_rtls == nullptr) {
                BB_rtls = new std::list<RTL *>();
            }

            // Define aliases to the RTLs so that they can be treated as a high level types where appropriate.
            RTL           *rtl       = inst.rtl;
            GotoStatement *stmt_jump = nullptr;
            Instruction   *last      = nullptr;

            if (rtl->size()) {
                last      = rtl->back();
                stmt_jump = static_cast<GotoStatement *>(last);
            }

#if BRANCH_DS_ERROR
            if ((last->getKind() == JUMP_RTL) || (last->getKind() == STMT_CALL) || (last->getKind() == JCOND_RTL) ||
                (last->getKind() == STMT_RET)) {
                ADDRESS dest = stmt_jump->getFixedDest();

                if ((dest != Address::INVALID) && (dest < hiAddress)) {
                    unsigned inst_before_dest = *((unsigned *)(dest - 4 + pBF->getTextDelta()));

                    unsigned bits31_30 = inst_before_dest >> 30;
                    unsigned bits23_22 = (inst_before_dest >> 22) & 3;
                    unsigned bits24_19 = (inst_before_dest >> 19) & 0x3f;
                    unsigned bits29_25 = (inst_before_dest >> 25) & 0x1f;

                    if ((bits31_30 == 0x01) ||                          // Call
                        ((bits31_30 == 0x02) && (bits24_19 == 0x38)) || // Jmpl
                        ((bits31_30 == 0x00) && (bits23_22 == 0x02) &&
                         (bits29_25 != 0x18))) {                        // Branch, but not (f)ba,a
                        // The above test includes floating point branches
                        std::cerr << "Target of branch at " << std::hex << rtl->getAddress()
                                  << " is delay slot of CTI at " << dest - 4 << '\n';
                    }
                }
            }
#endif

            switch (inst.type)
            {
            case NOP:
                // Always put the NOP into the BB. It may be needed if it is the
                // the destinsation of a branch. Even if not the start of a BB,
                // some other branch may be discovered to it later.
                BB_rtls->push_back(rtl);

                // Then increment the native address pointer
                uAddr = uAddr + 4;
                break;

            case NCT:
                // Ordinary instruction. Add it to the list of RTLs this BB
                BB_rtls->push_back(rtl);
                uAddr += inst.numBytes;

                // Ret/restore epilogues are handled as ordinary RTLs now
                if (last->getKind() == STMT_RET) {
                    sequentialDecode = false;
                }

                break;

            case SKIP:
                {
                    // We can't simply ignore the skipped delay instruction as there
                    // will most likely be a branch to it so we simply set the jump
                    // to go to one past the skipped instruction.
                    stmt_jump->setDest(uAddr + 8);
                    BB_rtls->push_back(rtl);

                    // Construct the new basic block and save its destination
                    // address if it hasn't been visited already
                    BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Oneway, 1);
                    handleBranch(uAddr + 8, m_image->getLimitTextHigh(), pBB, cfg, _targetQueue);

                    // There is no fall through branch.
                    sequentialDecode = false;
                    uAddr           += 8; // Update address for coverage
                    break;
                }

            case SU:
                {
                    // Ordinary, non-delay branch.
                    BB_rtls->push_back(inst.rtl);

                    BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Oneway, 1);
                    handleBranch(stmt_jump->getFixedDest(), m_image->getLimitTextHigh(), pBB, cfg, _targetQueue);

                    // There is no fall through branch.
                    sequentialDecode = false;
                    uAddr           += 8; // Update address for coverage
                    break;
                }

            case SD:
                {
                    // This includes "call" and "ba". If a "call", it might be a move_call_move idiom, or a call to .stret4
                    DecodeResult delay_inst;
                    decodeInstruction(uAddr + 4, delay_inst);

                    if (Boomerang::get()->traceDecoder) {
                        LOG << "*" << uAddr + 4 << "\t\n";
                    }

                    if (last->getKind() == STMT_CALL) {
                        // Check the delay slot of this call. First case of interest is when the instruction is a restore,
                        // e.g.
                        // 142c8:  40 00 5b 91          call           exit
                        // 142cc:  91 e8 3f ff          restore       %g0, -1, %o0
                        if (((SparcDecoder *)m_decoder)->isRestore(HostAddress(uAddr.value() + 4 + m_image->getTextDelta()))) {
                            // Give the address of the call; I think that this is actually important, if faintly annoying
                            delay_inst.rtl->setAddress(uAddr);
                            BB_rtls->push_back(delay_inst.rtl);
                            // The restore means it is effectively followed by a return (since the resore semantics chop
                            // off one level of return address)
                            ((CallStatement *)last)->setReturnAfterCall(true);
                            sequentialDecode = false;
                            case_CALL(uAddr, inst, nop_inst, BB_rtls, proc, callList, os, true);
                            break;
                        }

                        // Next class of interest is if it assigns to %o7 (could be a move, add, and possibly others). E.g.:
                        // 14e4c:  82 10 00 0f          mov           %o7, %g1
                        // 14e50:  7f ff ff 60          call           blah
                        // 14e54:  9e 10 00 01          mov           %g1, %o7
                        // Note there could be an unrelated instruction between the first move and the call
                        // (move/x/call/move in UQBT terms).  In boomerang, we leave the semantics of the moves there
                        // (to be likely removed by dataflow analysis) and merely insert a return BB after the call
                        // Note that if an add, there may be an assignment to a temp register first. So look at last RT
                        // TODO: why would delay_inst.rtl->empty() be empty here ?
                        Instruction *a = delay_inst.rtl->empty() ? nullptr : delay_inst.rtl->back(); // Look at last

                        if (a && a->isAssign()) {
                            SharedExp lhs = ((Assign *)a)->getLeft();

                            if (lhs->isRegN(15)) { // %o7 is r[15]
                                // If it's an add, this is special. Example:
                                //     call foo
                                //     add %o7, K, %o7
                                // is equivalent to call foo / ba .+K
                                SharedExp rhs = ((Assign *)a)->getRight();
                                auto      o7(Location::regOf(15));

                                if ((rhs->getOper() == opPlus) && (rhs->access<Exp, 2>()->getOper() == opIntConst) &&
                                    (*rhs->getSubExp1() == *o7)) {
                                    // Get the constant
                                    int K = rhs->access<Const, 2>()->getInt();
                                    case_CALL(uAddr, inst, delay_inst, BB_rtls, proc, callList, os, true);
                                    // We don't generate a goto; instead, we just decode from the new address
                                    // Note: the call to case_CALL has already incremented address by 8, so don't do again
                                    uAddr += K;
                                    break;
                                }
                                else {
                                    // We assume this is some sort of move/x/call/move pattern. The overall effect is to
                                    // pop one return address, we we emit a return after this call
                                    ((CallStatement *)last)->setReturnAfterCall(true);
                                    sequentialDecode = false;
                                    case_CALL(uAddr, inst, delay_inst, BB_rtls, proc, callList, os, true);
                                    break;
                                }
                            }
                        }
                    }

                    RTL *delay_rtl = delay_inst.rtl;

                    switch (delay_inst.type)
                    {
                    case NOP:
                    case NCT:

                        // Ordinary delayed instruction. Since NCT's can't affect unconditional jumps, we put the delay
                        // instruction before the jump or call
                        if (last->getKind() == STMT_CALL) {
                            // This is a call followed by an NCT/NOP
                            sequentialDecode = case_CALL(uAddr, inst, delay_inst, BB_rtls, proc, callList, os);
                        }
                        else {
                            // This is a non-call followed by an NCT/NOP
                            case_SD(uAddr, m_image->getTextDelta(), m_image->getLimitTextHigh(), inst, delay_inst,
                                    BB_rtls, cfg, _targetQueue, os);

                            // There is no fall through branch.
                            sequentialDecode = false;
                        }

                        if (spec && (inst.valid == false)) {
                            return false;
                        }

                        break;

                    case SKIP:
                        case_unhandled_stub(uAddr);
                        uAddr += 8;
                        break;

                    case SU:
                        {
                            // SD/SU.
                            // This will be either BA or CALL followed by BA,A. Our interpretation is that it is as if the SD
                            // (i.e. the BA or CALL) now takes the destination of the SU (i.e. the BA,A). For example:
                            //       call 1000, ba,a 2000
                            // is really like:
                            //       call 2000.

                            // Just so that we can check that our interpretation is correct the first time we hit this case...
                            case_unhandled_stub(uAddr);

                            // Adjust the destination of the SD and emit it.
                            GotoStatement *delay_jump = static_cast<GotoStatement *>(delay_rtl->back());
                                         Address       dest        = uAddr + 4 + delay_jump->getFixedDest();
                            stmt_jump->setDest(dest);
                            BB_rtls->push_back(inst.rtl);

                            // Create the appropriate BB
                            if (last->getKind() == STMT_CALL) {
                                handleCall(proc, dest, cfg->newBB(BB_rtls, BBType::Call, 1), cfg, uAddr, 8);

                                // Set the address of the lexical successor of the call that is to be decoded next. Set RTLs
                                // to nullptr so that a new list of RTLs will be created for the next BB.
                                BB_rtls = nullptr;
                                uAddr   = uAddr + 8;

                                // Add this call site to the set of call sites which need to be analyzed later.
                                callList.push_back((CallStatement *)inst.rtl->back());
                            }
                            else {
                                BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Oneway, 1);
                                handleBranch(dest, m_image->getLimitTextHigh(), pBB, cfg, _targetQueue);

                                // There is no fall through branch.
                                sequentialDecode = false;
                            }

                            break;
                        }

                    default:
                        case_unhandled_stub(uAddr);
                        uAddr += 8; // Skip the pair
                        break;
                    }

                    break;
                }

            case DD:
                {
                    DecodeResult delay_inst;

                    if (inst.numBytes == 4) {
                        // Ordinary instruction. Look at the delay slot
                        decodeInstruction(uAddr + 4, delay_inst);
                    }
                    else {
                        // Must be a prologue or epilogue or something.
                        delay_inst = nop_inst;
                        // Should be no need to adjust the coverage; the number of bytes should take care of it
                    }

                    RTL *delay_rtl = delay_inst.rtl;

                    // Display RTL representation if asked
                    if (Boomerang::get()->printRtl && (delay_rtl != nullptr)) {
                        delay_rtl->print(os);
                    }

                    switch (delay_inst.type)
                    {
                    case NOP:
                    case NCT:
                        sequentialDecode = case_DD(uAddr, m_image->getTextDelta(), inst, delay_inst, BB_rtls,
                                                   _targetQueue, proc, callList);
                        break;

                    default:
                        case_unhandled_stub(uAddr);
                        break;
                    }

                    break;
                }

            case SCD:
                {
                    // Always execute the delay instr, and branch if condition is met.
                    // Normally, the delayed instruction moves in front of the branch. But if it affects the condition
                    // codes, we may have to duplicate it as an orphan in the true leg of the branch, and fall through to
                    // the delay instruction in the "false" leg.
                    // Instead of moving the delay instruction to an orphan BB, we may have a duplicate of the delay
                    // instruction just before the target; if so, we can branch to that and not need the orphan.  We do
                    // just a binary comparison; that may fail to make this optimisation if the instr has relative fields.

                    DecodeResult delay_inst;
                    decodeInstruction(uAddr + 4, delay_inst);
                    RTL          *delay_rtl = delay_inst.rtl;

                    // Display low level RTL representation if asked
                    if (Boomerang::get()->printRtl && (delay_rtl != nullptr)) {
                        delay_rtl->print(os);
                    }

                    switch (delay_inst.type)
                    {
                    case NOP:
                    case NCT:
                        sequentialDecode = case_SCD(uAddr, m_image->getTextDelta(), m_image->getLimitTextHigh(), inst,
                                                    delay_inst, BB_rtls, cfg, _targetQueue);
                        break;

                    default:

                        if (delay_inst.rtl->back()->getKind() == STMT_CALL) {
                            // Assume it's the move/call/move pattern
                            sequentialDecode = case_SCD(uAddr, m_image->getTextDelta(), m_image->getLimitTextHigh(),
                                                        inst, delay_inst, BB_rtls, cfg, _targetQueue);
                            break;
                        }

                        case_unhandled_stub(uAddr);
                        break;
                    }

                    break;
                }

            case SCDAN:
                {
                    // Execute the delay instruction if the branch is taken; skip (anull) the delay instruction if branch
                    // not taken.
                    DecodeResult delay_inst;
                    decodeInstruction(uAddr + 4, delay_inst);
                    RTL          *delay_rtl = delay_inst.rtl;

                    // Display RTL representation if asked
                    if (Boomerang::get()->printRtl && (delay_rtl != nullptr)) {
                        delay_rtl->print(os);
                    }

                    switch (delay_inst.type)
                    {
                    case NOP:
                        {
                            // This is an ordinary two-way branch.  Add the branch to the list of RTLs for this BB
                            BB_rtls->push_back(rtl);
                            // Create the BB and add it to the CFG
                            BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Twoway, 2);

                            if (pBB == 0) {
                                sequentialDecode = false;
                                break;
                            }

                            // Visit the destination of the branch; add "true" leg
                                         Address uDest = stmt_jump->getFixedDest();
                            handleBranch(uDest, m_image->getLimitTextHigh(), pBB, cfg, _targetQueue);
                            // Add the "false" leg: point past the delay inst
                            cfg->addOutEdge(pBB, uAddr + 8);
                            uAddr  += 8;       // Skip branch and delay
                            BB_rtls = nullptr; // Start new BB
                            break;
                        }

                    case NCT:
                        sequentialDecode = case_SCDAN(uAddr, m_image->getTextDelta(), m_image->getLimitTextHigh(),
                                                      inst, delay_inst, BB_rtls, cfg, _targetQueue);
                        break;

                    default:
                        case_unhandled_stub(uAddr);
                        uAddr = uAddr + 8;
                        break;
                    }

                    break;
                }

            default: // Others are non sparc cases
                break;
            }

            // If sequentially decoding, check if the next address happens to be the start of an existing BB. If so,
            // finish off the current BB (if any RTLs) as a fallthrough, and no need to decode again (unless it's an
            // incomplete BB, then we do decode it).  In fact, mustn't decode twice, because it will muck up the
            // coverage, but also will cause subtle problems like add a call to the list of calls to be processed, then
            // delete the call RTL (e.g. Pentium 134.perl benchmark)
            if (sequentialDecode && cfg->existsBB(uAddr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    BasicBlock *pBB = cfg->newBB(BB_rtls, BBType::Fall, 1);

                    // Add an out edge to this address
                    if (pBB) {
                        cfg->addOutEdge(pBB, uAddr);
                        BB_rtls = nullptr; // Need new list of RTLs
                    }
                }

                // Pick a new address to decode from, if the BB is complete
                if (!cfg->isIncomplete(uAddr)) {
                    sequentialDecode = false;
                }
            }
        } // while (sequentialDecode)

        // Add this range to the coverage
        // proc->addRange(start, address);

        // Must set sequentialDecode back to true
        sequentialDecode = true;
    } // End huge while loop

    // Add the callees to the set of CallStatements to proces for parameter recovery, and also to the Prog object
    for (std::list<CallStatement *>::iterator it = callList.begin(); it != callList.end(); it++) {
              Address             dest  = (*it)->getFixedDest();
        const IBinarySymbol *symb = SymbolTable->find(dest);

        // Don't speculatively decode procs that are outside of the main text section, apart from dynamically linked
        // ones (in the .plt)
        if ((symb && symb->isImportedFunction()) || !spec || (dest < m_image->getLimitTextHigh())) {
            cfg->addCall(*it);

            // Don't visit the destination of a register call
            // if (dest != Address::INVALID) newProc(proc->getProg(), dest);
            if (dest != Address::INVALID) {
                proc->getProg()->createProc(dest);
            }
        }
    }

    // MVE: Not 100% sure this is the right place for this
    proc->setEntryBB();

    return true;
}


void SparcFrontEnd::emitNop(std::list<RTL *> *pRtls, Address uAddr)
{
    // Emit a null RTL with the given address. Required to cope with
    // SKIP instructions. Yes, they really happen, e.g. /usr/bin/vi 2.5
    RTL *pRtl = new RTL(uAddr);

    pRtls->push_back(pRtl);
}


void SparcFrontEnd::emitCopyPC(std::list<RTL *> *pRtls, Address uAddr)
{
    // Emit %o7 = %pc
    Assign *a = new Assign(Location::regOf(15), // %o7 == r[15]
                           Terminal::get(opPC));
    // Add the Exp to an RTL
    RTL *pRtl = new RTL(uAddr);

    pRtl->appendStmt(a);
    // Add the RTL to the list of RTLs, but to the second last position
    pRtls->insert(--pRtls->end(), pRtl);
}


void SparcFrontEnd::appendAssignment(const SharedExp& lhs, const SharedExp& rhs, SharedType type, Address addr, std::list<RTL *> *lrtl)
{
    Assign *a = new Assign(type, lhs, rhs);

    // Create an RTL with this one Statement
    std::list<Instruction *> *lrt = new std::list<Instruction *>;
    lrt->push_back(a);
    RTL *rtl = new RTL(addr, lrt);
    // Append this RTL to the list of RTLs for this BB
    lrtl->push_back(rtl);
}


void SparcFrontEnd::quadOperation(Address addr, std::list<RTL *> *lrtl, OPER op)
{
    SharedExp lhs = Location::memOf(Location::memOf(Binary::get(opPlus, Location::regOf(14), Const::get(64))));
    SharedExp rhs = Binary::get(op, Location::memOf(Location::regOf(8)), Location::memOf(Location::regOf(9)));

    appendAssignment(lhs, rhs, FloatType::get(128), addr, lrtl);
}


bool SparcFrontEnd::isHelperFunc(Address dest, Address addr, std::list<RTL *> *lrtl)
{
    const IBinarySymbol *sym = SymbolTable->find(dest);

    if (!(sym && sym->isImportedFunction())) {
        return false;
    }

    QString name = m_program->getSymbolByAddress(dest);

    if (name.isEmpty()) {
        LOG_STREAM() << "Error: Can't find symbol for PLT address " << dest << '\n';
        return false;
    }

    SharedExp rhs;

    if (name == ".umul") {
        // %o0 * %o1
        rhs = Binary::get(opMult, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".mul") {
        // %o0 *! %o1
        rhs = Binary::get(opMults, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".udiv") {
        // %o0 / %o1
        rhs = Binary::get(opDiv, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".div") {
        // %o0 /! %o1
        rhs = Binary::get(opDivs, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".urem") {
        // %o0 % %o1
        rhs = Binary::get(opMod, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".rem") {
        // %o0 %! %o1
        rhs = Binary::get(opMods, Location::regOf(8), Location::regOf(9));
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
    SharedExp lhs = Location::regOf(8);
    Assign *a     = new Assign(lhs, rhs);
    // Create an RTL with this one Exp
    std::list<Instruction *> *lrt = new std::list<Instruction *>;
    lrt->push_back(a);
    RTL *rtl = new RTL(addr, lrt);
    // Append this RTL to the list of RTLs for this BB
    lrtl->push_back(rtl);
    return true;
}


void SparcFrontEnd::gen32op32gives64(OPER op, std::list<RTL *> *lrtl, Address addr)
{
    std::list<Instruction *> *ls = new std::list<Instruction *>;
#if V9_ONLY
    // tmp[tmpl] = sgnex(32, 64, r8) op sgnex(32, 64, r9)
    Statement *a = new Assign(64, Location::tempOf(Const::get("tmpl")),
                              Binary::get(op, // opMult or opMults
                                          new Ternary(opSgnEx, Const(32), Const(64), Location::regOf(8)),
                                          new Ternary(opSgnEx, Const(32), Const(64), Location::regOf(9))));
    ls->push_back(a);
    // r8 = truncs(64, 32, tmp[tmpl]);
    a = new Assign(32, Location::regOf(8),
                   new Ternary(opTruncs, Const::get(64), Const::get(32), Location::tempOf(Const::get("tmpl"))));
    ls->push_back(a);
    // r9 = r[tmpl]@32:63;
    a = new Assign(32, Location::regOf(9),
                   new Ternary(opAt, Location::tempOf(Const::get("tmpl")), Const::get(32), Const::get(63)));
    ls->push_back(a);
#else
    // BTL: The .umul and .mul support routines are used in V7 code. We implsment these using the V8 UMUL and SMUL
    // instructions.
    // BTL: In SPARC V8, UMUL and SMUL perform 32x32 -> 64 bit multiplies.
    //        The 32 high-order bits are written to %Y and the 32 low-order bits are written to r[rd]. This is also true
    //        on V9 although the high-order bits are also written into the 32 high-order bits of the 64 bit r[rd].

    // r[tmp] = r8 op r9
    Assign *a = new Assign(Location::tempOf(Const::get(const_cast<char *>("tmp"))),
                           Binary::get(op, // opMult or opMults
                                       Location::regOf(8), Location::regOf(9)));
    ls->push_back(a);
    // r8 = r[tmp];     /* low-order bits */
    a = new Assign(Location::regOf(8), Location::tempOf(Const::get(const_cast<char *>("tmp"))));
    ls->push_back(a);
    // r9 = %Y;         /* high-order bits */
    a = new Assign(Location::regOf(8), Unary::get(opMachFtr, Const::get(const_cast<char *>("%Y"))));
    ls->push_back(a);
#endif /* V9_ONLY */
    RTL *rtl = new RTL(addr, ls);
    lrtl->push_back(rtl);
    delete ls;
}


bool SparcFrontEnd::helperFuncLong(Address dest, Address addr, std::list<RTL *> *lrtl, QString& name)
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
        rhs = Binary::get(opDiv, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".div") {
        // %o0 /! %o1
        rhs = Binary::get(opDivs, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".urem") {
        // %o0 % %o1
        rhs = Binary::get(opMod, Location::regOf(8), Location::regOf(9));
    }
    else if (name == ".rem") {
        // %o0 %! %o1
        rhs = Binary::get(opMods, Location::regOf(8), Location::regOf(9));
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
    lhs = Location::regOf(8);
    appendAssignment(lhs, rhs, IntegerType::get(32), addr, lrtl);
    return true;
}


SparcFrontEnd::SparcFrontEnd(IFileLoader *p_BF, Prog *prog, BinaryFileFactory *bff)
    : IFrontEnd(p_BF, prog, bff)
{
    m_decoder         = new SparcDecoder(prog);
    SymbolTable       = Boomerang::get()->getSymbols();
    nop_inst.numBytes = 0; // So won't disturb coverage
    nop_inst.type     = NOP;
    nop_inst.valid    = true;
    nop_inst.rtl      = new RTL();
}


SparcFrontEnd::~SparcFrontEnd()
{
}


Address SparcFrontEnd::getMainEntryPoint(bool& gotMain)
{
    gotMain = true;
       Address start = m_fileLoader->getMainEntryPoint();

    if (start != Address::INVALID) {
        return start;
    }

    start   = m_fileLoader->getEntryPoint();
    gotMain = false;

    if (start == Address::INVALID) {
        return Address::INVALID;
    }

    gotMain = true;
    return start;
}
