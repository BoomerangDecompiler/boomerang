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

/*==============================================================================
 * FILE:       machine/sparc/frontendsrc.cc
 * OVERVIEW:   This file contains routines to manage the decoding of sparc
 *             instructions and the instantiation to RTLs, removing sparc
 *             dependent features such as delay slots in the process. These
 *             functions replace Frontend.cc for decoding sparc instructions.
 * NOTE:       File can't have "sparc" in the name because of limitations in
 *             the Makefile (pattern substitution can happen only once per name)
 *============================================================================*/

/*
 * 17 May 02 - Mike: Mods for boomerang
 * 22 Nov 02 - Mike: Added check for invalid instructions; prints opcode
 * 12 Dec 02 - Mike: Fixed various bugs in move/call/move (etc) pattern handling
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#include <iomanip>          // For setfill etc
#include <sstream>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "exp.h"
#include "register.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "decoder.h"
#include "sparcdecoder.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "sparcfrontend.h"
#include "BinaryFile.h"     // E.g. IsDynamicallyLinkedProc
#include "boomerang.h"
#include "signature.h"

/*==============================================================================
 * FUNCTION:         warnDCTcouple
 * OVERVIEW:         Emit a warning when encountering a DCTI couple.
 * PARAMETERS:       uAt - the address of the couple
 *                   uDest - the address of the first DCTI in the couple
 * RETURNS:          <nothing>
 *============================================================================*/
void SparcFrontEnd::warnDCTcouple(ADDRESS uAt, ADDRESS uDest)
{
    std::cerr << "Error: DCTI couple at " << std::hex;
    std::cerr << uAt << " points to delayed branch at " << uDest << "...\n";
    std::cerr << "Decompilation will likely be incorrect\n";
}

/*==============================================================================
 * FUNCTION:        optimise_DelayCopy
 * OVERVIEW:        Determines if a delay instruction is exactly the same as the
 *                  instruction immediately preceding the destination of a CTI;
 *                  i.e. has been copied from the real destination to the delay
 *                  slot as an optimisation
 * PARAMETERS:      src - the logical source address of a CTI
 *                  dest - the logical destination address of the CTI
 *                  delta - used to convert logical to real addresses
 *                  uUpper - first address past the end of the main text
 *                    section
 * SIDE EFFECT:     Optionally displays an error message if the target of the
 *                    branch is the delay slot of another delayed CTI
 * RETURNS:         can optimise away the delay instruction
 *============================================================================*/
bool SparcFrontEnd::optimise_DelayCopy(ADDRESS src, ADDRESS dest, int delta, ADDRESS uUpper)
{
    // Check that the destination is within the main test section; may not be
    // when we speculatively decode junk
    if ((dest - 4) > uUpper)
        return false;
    unsigned delay_inst = *((unsigned*)(src+4+delta));
    unsigned inst_before_dest = *((unsigned*)(dest-4+delta));
    return (delay_inst == inst_before_dest);
}

/*==============================================================================
 * FUNCTION:        optimise_CallReturn
 * OVERVIEW:        Determines if the given call and delay instruction
 *                  consitute a call where callee returns to the caller's
 *                  caller. That is:
 *                      ProcA:               ProcB:                ProcC:
 *                       ...                  ...                   ...
 *                       call ProcB           call ProcC            ret
 *                       ...                  restore               ...
 *
 *                  The restore instruction in ProcB will effectively set %o7 to
 *                  be %i7, the address to which ProcB will return. So in effect
 *                  ProcC will return to ProcA at the position just after the
 *                  call to ProcB. This is equivalent to ProcC returning to
 *                  ProcB which then immediately returns to ProcA.
 * NOTE:            We don't set a label at the return, because we also have
 *                  to force a jump at the call BB, and in some cases we don't
 *                  have the call BB as yet. So these two are up to the caller
 * NOTE ALSO:       The name of this function is now somewhat irrelevant. The
 *                  whole function is somewhat irrelevant; it dates from the
 *                  times when you would always find an actual restore in the
 *                  delay slot. With some patterns, this is no longer true.
 * PARAMETERS:      call - the RTL for the caller (e.g. "call ProcC" above)
 *                  delay - the RTL for the delay instruction (e.g. "restore")
 *                  cfg - the CFG of the procedure
 * RETURNS:         the basic block containing the single return instruction if
 *                    this optimisation applies, NULL otherwise.
 *============================================================================*/
BasicBlock* SparcFrontEnd::optimise_CallReturn(CallStatement* call, RTL* rtl, RTL* delay, Cfg* cfg)
{

    if (call->isReturnAfterCall()) {

        // Constuct the RTLs for the new basic block
        std::list<RTL*>* rtls = new std::list<RTL*>();

        // The only RTL in the basic block is a high level return that doesn't
        // have any low level Statements.
        std::list<Statement*>* ls = new std::list<Statement*>;
        ls->push_back(new ReturnStatement);
        rtls->push_back(new RTL(rtl->getAddress() + 1, ls));
        
        BasicBlock* returnBB = cfg->newBB(rtls, RET, 0);
        return returnBB;
    }
    else
        // May want to put code here that checks whether or not the delay
        // instruction redefines %o7
        return NULL;
}

/*==============================================================================
 * FUNCTION:        handleBranch
 * OVERVIEW:        Adds the destination of a branch to the queue of address
 *                  that must be decoded (if this destination has not already
 *                  been visited). 
 * PARAMETERS:      newBB - the new basic block delimited by the branch
 *                    instruction. May be NULL if this block has been built
 *                    before.
 *                  dest - the destination being branched to
 *                  hiAddress - the last address in the current procedure
 *                  cfg - the CFG of the current procedure
 *                  tq: Object managing the target queue
 * RETURNS:         <nothing>, but newBB may be changed if the destination of
 *                  the branch is in the middle of an existing BB. It will then
 *                  be changed to point to a new BB beginning with the dest
 *============================================================================*/
void SparcFrontEnd::handleBranch(ADDRESS dest, ADDRESS hiAddress, BasicBlock*& newBB, Cfg* cfg,
  TargetQueue& tq) {
    if (newBB == NULL)
        return;

    if (dest < hiAddress) {
        tq.visit(cfg, dest, newBB);
        cfg->addOutEdge(newBB, dest, true);
    }
    else
        std::cerr << "Error: branch to " << std::hex << dest << " goes beyond section.\n";
}

/*==============================================================================
 * FUNCTION:          handleCall
 * OVERVIEW:          Records the fact that there is a procedure at a given
 *                    address. Also adds the out edge to the
 *                    lexical successor of the call site (taking into
 *                    consideration the delay slot and possible UNIMP
 *                    instruction).
 * PARAMETERS:        dest - the address of the callee
 *                    callBB - the basic block delimited by the call
 *                    cfg - CFG of the enclosing procedure
 *                    address - the address of the call instruction
 *                    offset - the offset from the call instruction to which an
 *                      outedge must be added. A value of 0 means no edge is to
 *                      be added.
 * RETURNS:           <nothing>
 *============================================================================*/
void SparcFrontEnd::handleCall(UserProc *proc, ADDRESS dest, BasicBlock* callBB, Cfg* cfg, ADDRESS address,
    int offset/* = 0*/)
{
    if (callBB == NULL)
        return;

    // If the destination address is the same as this very instruction,
    // we have a call with iDisp30 == 0. Don't treat this as the start
    // of a real procedure.
    if ((dest != address) && proc->getProg()->findProc(dest) == 0) {
        // We don't want to call prog.visitProc just yet, in case this is
        // a speculative decode that failed. Instead, we use the set of
        // CallStatements (not in this procedure) that is needed by CSR
        if (Boomerang::get()->traceDecoder)
            std::cout << "p" << std::hex << dest << "\t";
    }

    // Add the out edge if required
    if (offset != 0)
        cfg->addOutEdge(callBB, address+offset);

}

/*==============================================================================
 * FUNCTION:         case_unhandled_stub
 * OVERVIEW:         This is the stub for cases of DCTI couples that we haven't
 *                   written analysis code for yet. It simply displays an
 *                   informative warning and returns.
 * PARAMETERS:       addr - the address of the first CTI in the couple
 * RETURNS:          <nothing>
 *============================================================================*/
void SparcFrontEnd::case_unhandled_stub(ADDRESS addr)
{
    std::cerr << "Error: DCTI couple at " << std::hex << addr << std::endl;
}

/*==============================================================================
 * FUNCTION:         case_CALL
 * OVERVIEW:         Handles a call instruction
 * PARAMETERS:       address - the native address of the call instruction
 *                   inst - the info summaries when decoding the call
 *                     instruction
 *                   delay_inst - the info summaries when decoding the delay
 *                     instruction
 *                   BB_rtls - the list of RTLs currently built for the BB under
 *                     construction
 *                   proc - the enclosing procedure
 *                   callSet - a set of pointers to CallStatements for procs yet to
 *                     be processed
 *                   os - output stream for rtls
 *                   isPattern - true if the call is an idiomatic pattern (e.g.
 *                      a move_call_move pattern)
 * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set NULL
 * RETURNS:          true if next instruction is to be fetched sequentially from
 *                      this one
 *============================================================================*/
bool SparcFrontEnd::case_CALL(ADDRESS& address, DecodeResult& inst,
    DecodeResult& delay_inst, std::list<RTL*>*& BB_rtls, 
    UserProc* proc, std::set<CallStatement*>& callSet, std::ofstream &os,
    bool isPattern/* = false*/) {

    // Aliases for the call and delay RTLs
    CallStatement* call_stmt = ((CallStatement*)inst.rtl->getList().back());
    RTL* delay_rtl = delay_inst.rtl;

    Cfg* cfg = proc->getCFG();

    // Emit the delay instruction, unless the delay instruction is a nop,
    // or we have a pattern, or are followed by a restore
    if ((delay_inst.type != NOP) && !call_stmt->isReturnAfterCall()) {
        delay_rtl->updateAddress(address);
        BB_rtls->push_back(delay_rtl);
        if (Boomerang::get()->printRtl)
            delay_rtl->print(os);
    }

    // Get the new return basic block for the special
    // case where the delay instruction is a restore
    BasicBlock* returnBB = optimise_CallReturn(call_stmt, inst.rtl, delay_rtl, cfg);

    int disp30 = (call_stmt->getFixedDest() - address) >> 2;
    // Don't test for small offsets if part of a move_call_move pattern.
    // These patterns assign to %o7 in the delay slot, and so can't possibly
    // be used to copy %pc to %o7
    // Similarly if followed by a restore
    if (!isPattern && (returnBB == NULL) && (disp30 == 2 || disp30 == 3)) {
        // This is the idiomatic case where the destination
        // is 1 or 2 instructions after the delayed instruction.
        // Only emit the side effect of the call (%o7 := %pc) in this case.
        // Note that the side effect occurs before the delay slot instruction
        // (which may use the value of %o7)
        emitCopyPC(BB_rtls, address);
        address += disp30 << 2;
        return true;
    }
    else {
        assert (disp30 != 1);

        // First check for helper functions
        ADDRESS dest = call_stmt->getFixedDest();
        // Special check for calls to weird PLT entries which don't have symbols
        if ((pBF->IsDynamicLinkedProc(dest)) && 
          (pBF->SymbolByAddress(dest) == NULL)) {
            // This is one of those. Flag this as an invalid instruction
            inst.valid = false;
        }
        if (helperFunc(dest, address, BB_rtls)) {
            address += 8;           // Skip call, delay slot
            return true;
        }

        // Emit the call
        BB_rtls->push_back(inst.rtl);

        // End the current basic block
        BasicBlock* callBB = cfg->newBB(BB_rtls, CALL, 1);
        if (callBB == NULL)
            return false;

        // Add this call site to the set of call sites which
        // need to be analysed later.
        // This set will be used later to call prog.visitProc (so the proc
        // will get decoded)
        callSet.insert((CallStatement*)inst.rtl->getList().back());

        if (returnBB) {
            // Handle the call but don't add any outedges from it just yet.
            handleCall(proc, call_stmt->getFixedDest(), callBB, cfg, address);

            // Now add the out edge
            cfg->addOutEdge(callBB, returnBB);
            // Put a label on the return BB; indicate that a jump is reqd
            cfg->setLabel(returnBB);
            callBB->setJumpReqd();

            // Note that we get here for certain types of patterns as well as
            // for call/restore pairs. This could all go away if we could
            // specify that some sparc Logues are caller-prologue and also
            // callee-epilogues!
            // This is a hack until we figure out how to match these
            // patterns using a .pat file. We have to set the epilogue
            // for the enclosing procedure (all proc's must have an
            // epilogue).
            //proc->setEpilogue(new CalleeEpilogue("__dummy",std::list<std::string>()));
            // Set the return location; this is now always %o0
            //setReturnLocations(proc->getEpilogue(), 8 /* %o0 */);

            address += inst.numBytes;       // For coverage
            // This is a CTI block that doesn't fall through and so must
            // stop sequentially decoding
            return false;
        }
        else {
            // Else no restore after this call.
            // An outedge may be added to the lexical
            // successor of the call which will be 8 bytes
            // ahead or in the case where the callee returns
            // a struct, 12 bytes head
            // If forceOutEdge is set, set offset to 0 and no out-edge will be
            // added yet
            int offset = inst.forceOutEdge ? 0 :
                (call_stmt->returnsStruct() ? 12 : 8);

            bool ret = true;
            // Check for _exit; probably should check for other "never return"
            // functions
            const char* name = pBF->SymbolByAddress(dest);
            if (name && strcmp(name, "_exit") == 0) {
                // Don't keep decoding after this call
                ret = false;
                // Also don't add an out-edge; setting offset to 0 will do this
                offset = 0;
                // But we have already set the number of out-edges to 1
                callBB->updateType(CALL, 0);
            }

            // Handle the call (register the destination as a proc)
            // and possibly set the outedge.
            handleCall(proc, dest, callBB, cfg, address, offset);

            if (inst.forceOutEdge) {
                // There is no need to force a goto to the new out-edge, since
                // we will continue decoding from there. If other edges exist
                // to the outedge, they will generate the required label
                cfg->addOutEdge(callBB, inst.forceOutEdge);
                address = inst.forceOutEdge;
            }
            else {
                // Continue decoding from the lexical successor
                address += offset;
            }
            BB_rtls = NULL;

            return ret;
        }
    }
}

/*==============================================================================
 * FUNCTION:         case_SD
 * OVERVIEW:         Handles a non-call, static delayed (SD) instruction
 * PARAMETERS:       address - the native address of the SD
 *                   delta - the offset of the above address from the logical
 *                     address at which the procedure starts (i.e. the one
 *                     given by dis)
 *                   inst - the info summaries when decoding the SD
 *                     instruction
 *                   delay_inst - the info summaries when decoding the delay
 *                     instruction
 *                   BB_rtls - the list of RTLs currently built for the BB under
 *                     construction
 *                   cfg - the CFG of the enclosing procedure
 *                   tq: Object managing the target queue
 *                   os - output stream for rtls
 * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set NULL
 * RETURNS:          <nothing>
 *============================================================================*/
void SparcFrontEnd::case_SD(ADDRESS& address, int delta, ADDRESS hiAddress,
    DecodeResult& inst, DecodeResult& delay_inst, std::list<RTL*>*& BB_rtls,
    Cfg* cfg, TargetQueue& tq, std::ofstream &os)
{

    // Aliases for the SD and delay RTLs
    GotoStatement* SD_stmt = static_cast<GotoStatement*>(inst.rtl->getList().back());
    RTL* delay_rtl = delay_inst.rtl;

    // Try the "delay instruction has been copied" optimisation, emitting the
    // delay instruction now if the optimisation won't apply
    if (delay_inst.type != NOP) {
        if (optimise_DelayCopy(address, SD_stmt->getFixedDest(), delta,
          hiAddress))
            SD_stmt->adjustFixedDest(-4);
        else {
            // Move the delay instruction before the SD. Must update the address
            // in case there is a branch to the SD
            delay_rtl->updateAddress(address);
            BB_rtls->push_back(delay_rtl);
            // Display RTL representation if asked
            //if (progOptions.rtl)
if (0)          // SETTINGS!
                delay_rtl->print(os);
        }
    }

    // Update the address (for coverage)
    address += 8;

    // Add the SD
    BB_rtls->push_back(inst.rtl);

    // Add the one-way branch BB
    PBB pBB = cfg->newBB(BB_rtls, ONEWAY, 1);
    if (pBB == 0) { BB_rtls = NULL; return; }

    // Visit the destination, and add the out-edge
    ADDRESS uDest = SD_stmt->getFixedDest();
    handleBranch(uDest, hiAddress, pBB, cfg, tq);
    BB_rtls = NULL;
}


/*==============================================================================
 * FUNCTION:         case_DD
 * OVERVIEW:         Handles all dynamic delayed jumps (jmpl, also dynamic
 *                    calls)
 * PARAMETERS:       address - the native address of the DD
 *                   delta - the offset of the above address from the logical
 *                     address at which the procedure starts (i.e. the one
 *                     given by dis)
 *                   inst - the info summaries when decoding the SD
 *                     instruction
 *                   delay_inst - the info summaries when decoding the delay
 *                     instruction
 *                   BB_rtls - the list of RTLs currently built for the BB under
 *                     construction
 *                   cfg - the CFG of the enclosing procedure
 *                   tq: Object managing the target queue
 *                   proc: pointer to the current Proc object
 *                   callSet - a set of pointers to CallStatements for procs yet to
 *                     be processed
 * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set NULL
 * RETURNS:          true if next instruction is to be fetched sequentially from
 *                      this one
 *============================================================================*/
bool SparcFrontEnd::case_DD(ADDRESS& address, int delta, DecodeResult& inst,
    DecodeResult& delay_inst, std::list<RTL*>*& BB_rtls, Cfg* cfg,
    TargetQueue& tq, UserProc* proc, std::set<CallStatement*>& callSet) {

    if (delay_inst.type != NOP) {
        // Emit the delayed instruction, unless a NOP
        delay_inst.rtl->updateAddress(address);
        BB_rtls->push_back(delay_inst.rtl);
    }

    // Set address past this instruction and delay slot (may be changed later).
    // This in so that we cover the jmp/call and delay slot instruction, in
    // case we return false
    address += 8;

    // Emit the DD and end the current BB
    BB_rtls->push_back(inst.rtl);
    BasicBlock* newBB;
    bool bRet = true;
    switch (inst.rtl->getList().back()->getKind()) {
        case STMT_CALL:
            // Will be a computed call
            newBB = cfg->newBB(BB_rtls, COMPCALL, 1);
            break;
        case STMT_RET:
            newBB = cfg->newBB(BB_rtls, RET, 0);
            bRet = false;
            break;
        case STMT_CASE:
            newBB = cfg->newBB(BB_rtls, COMPJUMP, 0);
            bRet = false;
            break;
        default:
            break;
    }
    if (newBB == NULL) return false;

    Statement* last = inst.rtl->getList().back();
    // Do extra processing for for special types of DD
    if (last->getKind() == STMT_CALL) {

        // Attempt to add a return BB if the delay
        // instruction is a RESTORE
        CallStatement*  call_stmt   =
          (CallStatement*)(inst.rtl->getList().back());
        BasicBlock* returnBB = optimise_CallReturn(call_stmt,
            inst.rtl, delay_inst.rtl, cfg);
        if (returnBB != NULL) {
            cfg->addOutEdge(newBB,returnBB);

            // We have to set the epilogue
            // for the enclosing procedure (all proc's must have an
            // epilogue) and remove the RESTORE in the delay slot that
            // has just been pushed to the list of RTLs
          //proc->setEpilogue(new CalleeEpilogue("__dummy",std::list<std::string>()));
            // Set the return location; this is now always %o0
          //setReturnLocations(proc->getEpilogue(), 8 /* %o0 */);
            newBB->getRTLs()->remove(delay_inst.rtl);

            // Put a label on the return BB; indicate that a jump is reqd
            cfg->setLabel(returnBB);
            newBB->setJumpReqd();

            // Add this call to the list of calls to analyse. We won't be able
            // to analyse it's callee(s), of course.
            callSet.insert(call_stmt);

            return false;
        }
        else {
            // Instead, add the standard out edge to original address+8 (now
            // just address)
            cfg->addOutEdge(newBB, address);
        }
        // Add this call to the list of calls to analyse. We won't be able
        // to analyse its callee(s), of course.
        callSet.insert(call_stmt);
    }
    else if(last->getKind() == STMT_CASE) {

        // Attempt to process this jmpl as a switch statement.
        // NOTE: the isSwitch and processSwitch methods should
        // arguably be merged into one
        CaseStatement*   stmt_jump   = static_cast<CaseStatement*>(
            inst.rtl->getList().back());
        if (isSwitch(newBB, stmt_jump->getDest(), proc, pBF))
            processSwitch(newBB, delta, cfg, tq, pBF);
        else {
            std::cerr << "Warning: COMPUTED JUMP at " << std::hex << address-8 << std::endl;
        }
    }

    // Set the address of the lexical successor of the call
    // that is to be decoded next and create a new list of
    // RTLs for the next basic block.
    BB_rtls = NULL;
    return bRet;
}

/*==============================================================================
 * FUNCTION:         case_SCD
 * OVERVIEW:         Handles all Static Conditional Delayed non-anulled branches
 * PARAMETERS:       address - the native address of the DD
 *                   delta - the offset of the above address from the logical
 *                     address at which the procedure starts (i.e. the one
 *                     given by dis)
                     hiAddress - first address outside this code section
 *                   inst - the info summaries when decoding the SD
 *                     instruction
 *                   delay_inst - the info summaries when decoding the delay
 *                     instruction
 *                   BB_rtls - the list of RTLs currently built for the BB under
 *                     construction
 *                   cfg - the CFG of the enclosing procedure
 *                   tq: Object managing the target queue
 * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set NULL
 * RETURNS:          true if next instruction is to be fetched sequentially from
 *                      this one
 *============================================================================*/
bool SparcFrontEnd::case_SCD(ADDRESS& address, int delta, ADDRESS hiAddress,
    DecodeResult& inst, DecodeResult& delay_inst, std::list<RTL*>*& BB_rtls,
    Cfg* cfg, TargetQueue& tq) {
    GotoStatement*   stmt_jump   = static_cast<GotoStatement*>(
      inst.rtl->getList().back());
    ADDRESS uDest = stmt_jump->getFixedDest();
    
    // Assume that if we find a call in the delay slot, it's actually a pattern
    // such as move/call/move
// MVE: Check this! Only needed for HP PA/RISC
    bool delayPattern = delay_inst.rtl->isCall();

    if (delayPattern) {
        // Just emit the branch, and decode the instruction immediately
        // following next. Assumes the first instruction of the pattern is
        // not used in the true leg
        BB_rtls->push_back(inst.rtl);
        PBB pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
        if (pBB == 0)  return false;
        handleBranch(uDest, hiAddress, pBB, cfg, tq);
        // Add the "false" leg
        cfg->addOutEdge(pBB, address+4);
        address += 4;           // Skip the SCD only
        // Start a new list of RTLs for the next BB
        BB_rtls = NULL;
        std::cerr << "Warning: instruction at " << std::hex << address;
        std::cerr << " not copied to true leg of preceeding branch\n";
        return true;
    }

    if (!delay_inst.rtl->areFlagsAffected()) {
        // SCD; flags not affected. Put delay inst first
        if (delay_inst.type != NOP) {
            // Emit delay instr
            BB_rtls->push_back(delay_inst.rtl);
            // This is in case we have an in-edge to the branch. If the BB
            // is split, we want the split to happen here, so this delay
            // instruction is active on this path
            delay_inst.rtl->updateAddress(address);
        }
        // Now emit the branch
        BB_rtls->push_back(inst.rtl);
        PBB pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
        if (pBB == 0)  return false;
        handleBranch(uDest, hiAddress, pBB, cfg, tq);
        // Add the "false" leg; skips the NCT
        cfg->addOutEdge(pBB, address+8);
        // Skip the NCT/NOP instruction
        address += 8;
    }
    else if (optimise_DelayCopy(address, uDest, delta, hiAddress)) {
        // We can just branch to the instr before uDest.
        // Adjust the destination of the branch
        stmt_jump->adjustFixedDest(-4);
        // Now emit the branch
        BB_rtls->push_back(inst.rtl);
        PBB pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
        if (pBB == 0) return false;
        handleBranch(uDest-4, hiAddress, pBB, cfg, tq);
        // Add the "false" leg: point to the delay inst
        cfg->addOutEdge(pBB, address+4);
        address += 4;           // Skip branch but not delay
    }
    else // The CCs are affected, and we can't use the copy delay slot trick
    {
        // SCD, must copy delay instr to orphan
        // Copy the delay instruction to the dest of the branch, as an orphan
        // First add the branch.
        BB_rtls->push_back(inst.rtl);
        // Make a BB for the current list of RTLs
        // We want to do this first, else ordering can go silly
        PBB pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
        if (pBB == 0) return false;
        // Visit the target of the branch
        tq.visit(cfg, uDest, pBB);
        std::list<RTL*>* pOrphan = new std::list<RTL*>;
        pOrphan->push_back(delay_inst.rtl);
        // Change the address to 0, since this code has no source address
        // (else we may branch to here when we want to branch to the real
        // BB with this instruction).
        // Note that you can't use an address that is a fixed function of the
        // destination addr, because there can be several jumps to the same
        // destination that all require an orphan. The instruction in the
        // orphan will often but not necessarily be the same, so we can't use
        // the same orphan BB. newBB knows to consider BBs with address 0 as
        // being in the map, so several BBs can exist with address 0
        delay_inst.rtl->updateAddress(0);
        // Add a branch from the orphan instruction to the dest of the branch
        // Again, we can't even give the jumps a special address like 1, since
        // then the BB would have this getLowAddr.
        std::list<Statement*>* gl = new std::list<Statement*>;
        gl->push_back(new GotoStatement(uDest));
        pOrphan->push_back(new RTL(0, gl));
        PBB pOrBB = cfg->newBB(pOrphan, ONEWAY, 1);
        // Add an out edge from the orphan as well
        cfg->addOutEdge(pOrBB, uDest, true);
        // Add an out edge from the current RTL to
        // the orphan. Put a label at the orphan
        cfg->addOutEdge(pBB, pOrBB, true);
        // Add the "false" leg to the NCT
        cfg->addOutEdge(pBB, address+4);
        // Don't skip the delay instruction, so it will
        // be decoded next.
        address += 4;
    }
    // Start a new list of RTLs for the next BB
    BB_rtls = NULL;
    return true;
}

/*==============================================================================
 * FUNCTION:         case_SCDAN
 * OVERVIEW:         Handles all static conditional delayed anulled branches
 *                     followed by an NCT (but not NOP) instruction.
 * PARAMETERS:       address - the native address of the DD
 *                   delta - the offset of the above address from the logical
 *                     address at which the procedure starts (i.e. the one
 *                     given by dis)
                     hiAddress - first address outside this code section
 *                   inst - the info summaries when decoding the SD
 *                     instruction
 *                   delay_inst - the info summaries when decoding the delay
 *                     instruction
 *                   BB_rtls - the list of RTLs currently built for the BB under
 *                     construction
 *                   cfg - the CFG of the enclosing procedure
 *                   tq: Object managing the target queue
 * SIDE EFFECTS:     address may change; BB_rtls may be appended to or set NULL
 * RETURNS:          true if next instruction is to be fetched sequentially from
 *                      this one
 *============================================================================*/
bool SparcFrontEnd::case_SCDAN(ADDRESS& address, int delta, ADDRESS hiAddress,
    DecodeResult& inst, DecodeResult& delay_inst, std::list<RTL*>*& BB_rtls,
    Cfg* cfg, TargetQueue& tq) {

    // We may have to move the delay instruction to an orphan
    // BB, which then branches to the target of the jump.
    // Instead of moving the delay instruction to an orphan BB,
    // we may have a duplicate of the delay instruction just
    // before the target; if so, we can branch to that and not
    // need the orphan. We do just a binary comparison; that
    // may fail to make this optimisation if the instr has
    // relative fields.
    GotoStatement*   stmt_jump   = static_cast<GotoStatement*>(
      inst.rtl->getList().back());
    ADDRESS uDest = stmt_jump->getFixedDest();
    PBB pBB;
    if (optimise_DelayCopy(address, uDest, delta, hiAddress)) {
        // Adjust the destination of the branch
        stmt_jump->adjustFixedDest(-4);
        // Now emit the branch
        BB_rtls->push_back(inst.rtl);
        pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
        if (pBB == 0) return false;
        handleBranch(uDest-4, hiAddress, pBB, cfg, tq);
    }
    else    // SCDAN; must move delay instr to orphan. Assume it's not a NOP
            // (though if it is, no harm done)
    {
        // Move the delay instruction to the dest of the branch, as an orphan
        // First add the branch.
        BB_rtls->push_back(inst.rtl);
        // Make a BB for the current list of RTLs
        // We want to do this first, else ordering can go silly
        pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
        if (pBB == 0) return false;
        // Visit the target of the branch
        tq.visit(cfg, uDest, pBB);
        std::list<RTL*>* pOrphan = new std::list<RTL*>;
        pOrphan->push_back(delay_inst.rtl);
        // Change the address to 0, since this code has no source address
        // (else we may branch to here when we want to branch to the real
        // BB with this instruction).
        delay_inst.rtl->updateAddress(0);
        // Add a branch from the orphan instruction to the dest of the branch
        std::list<Statement*>* gl = new std::list<Statement*>;
        gl->push_back(new GotoStatement(uDest));
        pOrphan->push_back(new RTL(0, gl));
        PBB pOrBB = cfg->newBB(pOrphan, ONEWAY, 1);
        // Add an out edge from the orphan as well. Set a label there.
        cfg->addOutEdge(pOrBB, uDest, true);
        // Add an out edge from the current RTL to
        // the orphan. Set a label there.
        cfg->addOutEdge(pBB, pOrBB, true);
    }
    // Both cases (orphan or not)
    // Add the "false" leg: point past delay inst. Set a label there (see below)
    cfg->addOutEdge(pBB, address+8, true);
    // Could need a jump to the following BB, e.g. if uDest is the delay slot
    // instruction itself! e.g. beq,a $+8
    pBB->setJumpReqd();
    address += 8;           // Skip branch and delay
    BB_rtls = NULL;         // Start new BB return true;
    return true;
}


/*==============================================================================
 * FUNCTION:         SparcFrontEnd::processProc
 * OVERVIEW:         Builds the CFG for a procedure out of the RTLs constructed
 *                   during decoding. The semantics of delayed CTIs are
 *                   transformed into CTIs that aren't delayed.
 * NOTE:             This function overrides (and replaces) the function with
 *                     the same name in class FrontEnd. The required actions
 *                     are so different that the base class implementation
 *                     can't be re-used
 * PARAMETERS:       address - the native address at which the procedure starts
 *                   proc - the procedure object
 *                   os - output stream for rtl output
 *                   spec - if true, this is a speculative decode
 *                   helperFunc - This parameter is here only to agree with
 *                    the parameters for the base class (else the virtual
 *                    function mechanism won't work). Do not use.
 * RETURNS:          True if a good decode
 *============================================================================*/
bool SparcFrontEnd::processProc(ADDRESS address, UserProc* proc,
  std::ofstream &os, bool spec /* = false */, PHELPER helperFunc /* = NULL */) {

    // init arguments and return set to be all 31 machine registers
    // Was important: because o registers are save in i registers, and
    // i registers have higher register numbers (e.g. i1=r25, o1=r9)
    // it helps the prover to process higher register numbers first!
    // But do r30 first (%i6, saves %o6, the stack pointer)
    Signature *sig = proc->getSignature();
    sig->addParameter(Unary::regOf(30));
    sig->addReturn   (Unary::regOf(30));
    sig->addParameter(Unary::regOf(31));
    sig->addReturn   (Unary::regOf(31));
    for (int r=29; r>0; r--) {
        sig->addParameter(Unary::regOf(r));
        sig->addReturn   (Unary::regOf(r));
    }

    // Declare an object to manage the queue of targets not yet processed yet.
    // This has to be individual to the procedure! (so not a global)
    TargetQueue targetQueue;

    // Similarly, we have a set of CallStatement pointers. These may be
    // disregarded if this is a speculative decode that fails (i.e. an illegal
    // instruction is found). If not, this set will be used to add to the set
    // of calls to be analysed in the cfg, and also to call prog.visitProc()
    std::set<CallStatement*> callSet;

    // Indicates whether or not the next instruction to be decoded is the
    // lexical successor of the current one. Will be true for all NCTs and for
    // CTIs with a fall through branch.
    bool sequentialDecode = true;

    // The control flow graph of the current procedure
    Cfg* cfg = proc->getCFG();

    // If this is a speculative decode, the second time we decode the same
    // address, we get no cfg. Else an error.
    if (spec && (cfg == 0))
        return false;
    assert(cfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    targetQueue.initial(address);

    // Get the next address from which to continue decoding and go from
    // there. Exit the loop if there are no more addresses or they all
    // correspond to locations that have been decoded.
    while ((address = targetQueue.nextAddress(cfg)) != NO_ADDRESS) {

        // The list of RTLs for the current basic block
        std::list<RTL*>* BB_rtls = new std::list<RTL*>();

        // Keep decoding sequentially until a CTI without a fall through branch
        // is decoded
        //ADDRESS start = address;
        DecodeResult inst;
        while (sequentialDecode) {

            if (Boomerang::get()->traceDecoder)
                std::cout << "*" << std::hex << address << "\t" << std::flush;

            inst = decodeInstruction(address);

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid)
                return false;

            // Check for invalid instructions
            if (!inst.valid) {
                std::cerr << "Invalid instruction at " << std::hex << address
                  << ": ";
                std::cerr << std::setfill('0') << std::setw(2);
                int delta = pBF->getTextDelta();
                for (int j=0; j<inst.numBytes; j++)
                    std::cerr << std::setfill('0') << std::setw(2) <<
                      (unsigned)*(unsigned char*)(address+delta + j) << " ";
                std::cerr << std::setfill(' ') << std::setw(0) << "\n";
                return false;
            }

            // Don't display the RTL here; do it after the switch statement
            // in case the delay slot instruction is moved before this one

            // Need to construct a new list of RTLs if a basic block has just
            // been finished but decoding is continuing from its lexical
            // successor
            if (BB_rtls == NULL)
                BB_rtls = new std::list<RTL*>();

            // Define aliases to the RTLs so that they can be treated as a high
            // level types where appropriate.
            RTL*   rtl        = inst.rtl;
            GotoStatement*   stmt_jump   = static_cast<GotoStatement*>(
              rtl->getList().back());
            Statement* last = rtl->getList().back();

#define BRANCH_DS_ERROR 0   // If set, a branch to the delay slot of a delayed
                            // CTI instruction is flagged as an error
#if BRANCH_DS_ERROR
            if ((last->getKind() == JUMP_RTL) ||
                (last->getKind() == STMT_CALL) ||
                (last->getKind() == JCOND_RTL) ||
                (last->getKind() == STMT_RET)) {
                ADDRESS dest = stmt_jump->getFixedDest();
                if ((dest != NO_ADDRESS) && (dest < hiAddress)){
                    unsigned inst_before_dest = *((unsigned*)(dest-4+pBF->getTextDelta()));

                    unsigned bits31_30 = inst_before_dest >> 30;
                    unsigned bits23_22 = (inst_before_dest >> 22) & 3;
                    unsigned bits24_19 = (inst_before_dest >> 19) & 0x3f;
                    unsigned bits29_25 = (inst_before_dest >> 25) & 0x1f;
                    if ((bits31_30 == 0x01) ||      // Call
                        ((bits31_30 == 0x02) && (bits24_19 == 0x38)) || // Jmpl
                        ((bits31_30 == 0x00) && (bits23_22 == 0x02) &&
                         (bits29_25 != 0x18))) {// Branch, but not (f)ba,a
                            // The above test includes floating point branches
                            std::cerr << "Target of branch at " << std::hex <<
                                rtl->getAddress() <<
                                " is delay slot of CTI at " << dest-4 << std::endl;
                    }
                }
            }
#endif

            switch (inst.type) {
            case NOP:
                // Always put the NOP into the BB. It may be needed if it is the
                // the destinsation of a branch. Even if not the start of a BB,
                // some other branch may be discovered to it later.
                BB_rtls->push_back(rtl);

                // Then increment the native address pointer
                address = address + 4;
                break;

            case NCT:
                // Ordinary instruction. Add it to the list of RTLs this BB
                BB_rtls->push_back(rtl);
                address += inst.numBytes;
                // Ret/restore epilogues are handled as ordinary RTLs now
                if (last->getKind() == STMT_RET)
                    sequentialDecode = false;
                break;

            case SKIP: {
                // We can't simply ignore the skipped delay instruction as there
                // will most likely be a branch to it so we simply set the jump
                // to go to one past the skipped instruction.
                stmt_jump->setDest(address+8);
                BB_rtls->push_back(rtl);

                // Construct the new basic block and save its destination
                // address if it hasn't been visited already
                PBB pBB = cfg->newBB(BB_rtls, ONEWAY, 1);
                handleBranch(address+8, pBF->getLimitTextHigh(), pBB, cfg, targetQueue);

                // There is no fall through branch.
                sequentialDecode = false;
                address += 8;       // Update address for coverage
                break;
            }

            case SU: {   
                // Ordinary, non-delay branch.
                BB_rtls->push_back(inst.rtl);

                PBB pBB = cfg->newBB(BB_rtls, ONEWAY, 1);
                handleBranch(stmt_jump->getFixedDest(), pBF->getLimitTextHigh(), pBB,
                    cfg, targetQueue);

                // There is no fall through branch.
                sequentialDecode = false;
                address += 8;       // Update address for coverage
                break;
            }

            case SD: {
                // This includes "call" and "ba". If a "call", it might
                // be a move_call_move idiom, or a call to .stret4
                DecodeResult delay_inst = decodeInstruction(address+4);
                if (Boomerang::get()->traceDecoder)
                    std::cerr << "*" << std::hex << address+4 << "\t" <<
                      std::flush;
                if (last->getKind() == STMT_CALL) {
                    // Check the delay slot of this call. First case of interest
                    // is when the instruction is a restore, e.g.
                    // 142c8:  40 00 5b 91        call         exit
                    // 142cc:  91 e8 3f ff        restore      %g0, -1, %o0
                    if (((SparcDecoder*)decoder)->
                      isRestore(address+4+pBF->getTextDelta())) {
                        // Give the address of the call; I think that this is
                        // actually important, if faintly annoying
                        delay_inst.rtl->updateAddress(address);
                        BB_rtls->push_back(delay_inst.rtl);
                        // The restore means it is effectively followed by a
                        // return (since the resore semantics chop off one level
                        // of return address)
                        ((CallStatement*)last)->setReturnAfterCall(true);
                        sequentialDecode = false;
                        case_CALL(address, inst, nop_inst, BB_rtls,
                          proc, callSet, os, true);
                        break;
                    }
                    // Next class of interest is if it assigns to %o7
                    // (could be a move, add, and possibly others). E.g.:
                    // 14e4c:  82 10 00 0f        mov          %o7, %g1
                    // 14e50:  7f ff ff 60        call         blah
                    // 14e54:  9e 10 00 01        mov          %g1, %o7
                    // Note there could be an unrelated instruction between the
                    // first move and the call (move/x/call/move in UQBT terms)
                    // In boomerang, we leave the semantics of the moves there
                    // (to be likely removed by dataflow analysis) and merely
                    // insert a return BB after the call
                    int nd = delay_inst.rtl->getNumStmt();
                    // Note that if an add, there may be an assignment to a
                    // temp register first. So look at last RT
                    Statement* a = delay_inst.rtl->elementAt(nd-1); // Look at last
                    if (a && a->isAssign()) {
                        Exp* lhs = a->getLeft();
                        if (lhs->isRegN(15)) {       // %o7 is r[15]
                            // If it's an add, this is special. Example:
                            //   call foo
                            //   add %o7, K, %o7
                            // is equivalent to call foo / ba .+K
                            Exp* rhs = a->getRight();
                            static Unary o7(opRegOf, new Const(15));
                            if ((((Binary*)rhs)->getSubExp2()->getOper() ==
                              opIntConst) && (*((Binary*)rhs)->getSubExp1()
                              == o7)) {
                                // Get the constant
                                int K = ((Const*)
                                  ((Binary*)rhs)->getSubExp2()) ->getInt();
                                case_CALL(address, inst, nop_inst, BB_rtls,
                                  proc, callSet, os, true);
                                // We don't generate a goto; instead, we just
                                // decode from the new address
                                // Note: the call to case_CALL has already
                                //  incremented address by 8, so don't do again
                                address += K;
                                break;
                            } else {
                                // We assume this is some sort of move/x/call/
                                // move pattern. The overall effect is to
                                // pop one return address, we we emit a return
                                // after this call
                                ((CallStatement*)last)->setReturnAfterCall(true);
                                sequentialDecode = false;
                                case_CALL(address, inst, nop_inst, BB_rtls,
                                  proc, callSet, os, true);
                                break;
                            }
                        }
                    }
                }
                RTL* delay_rtl = delay_inst.rtl;

                switch(delay_inst.type) {
                case NOP:
                case NCT: {
                    // Ordinary delayed instruction. Since NCT's can't
                    // affect unconditional jumps, we put the delay
                    // instruction before the jump or call
                    if (last->getKind() == STMT_CALL) {

                        // This is a call followed by an NCT/NOP
                        sequentialDecode = case_CALL(address, inst,
                            delay_inst, BB_rtls, proc, callSet, os);
                    }
                    else {
                        // This is a non-call followed by an NCT/NOP
                        case_SD(address, pBF->getTextDelta(),
                          pBF->getLimitTextHigh(), inst, delay_inst,
                          BB_rtls, cfg, targetQueue, os);

                        // There is no fall through branch.
                        sequentialDecode = false;
                    }
                    if (spec && (inst.valid == false))
                        return false;
                    break;
                }

                case SKIP:
                    case_unhandled_stub(address);
                    address += 8;
                    break;

                case SU: {
                    // SD/SU.
                    // This will be either BA or CALL followed by BA,A. Our
                    // interpretation is that it is as if the SD (i.e. the
                    // BA or CALL) now takes the destination of the SU
                    // (i.e. the BA,A). For example:
                    //     call 1000, ba,a 2000
                    // is really like:
                    //     call 2000.

                    // Just so that we can check that our interpretation is
                    // correct the first time we hit this case...
                    case_unhandled_stub(address);

                    // Adjust the destination of the SD and emit it.
                    GotoStatement* delay_jump = static_cast<GotoStatement*>(
                      delay_rtl->getList().back());
                    int dest = 4+address+delay_jump->getFixedDest();
                    stmt_jump->setDest(dest);
                    BB_rtls->push_back(inst.rtl);

                    // Create the appropriate BB
                    if (last->getKind() == STMT_CALL) {
                        handleCall(proc, dest, cfg->newBB(BB_rtls,CALL, 1), cfg,
                            address, 8);
                        
                        // Set the address of the lexical successor of the
                        // call that is to be decoded next. Set RTLs to
                        // NULL so that a new list of RTLs will be created
                        // for the next BB.
                        BB_rtls = NULL;
                        address = address + 8;

                        // Add this call site to the set of call sites which
                        // need to be analysed later.
                        callSet.insert((CallStatement*)inst.rtl->getList().back());
                    }
                    else {
                        PBB pBB = cfg->newBB(BB_rtls,ONEWAY, 1);
                        handleBranch(dest, pBF->getLimitTextHigh(), pBB, cfg, targetQueue);

                        // There is no fall through branch.
                        sequentialDecode = false;
                    }
                    break;
                }
                default:
                    case_unhandled_stub(address);
                    address += 8;       // Skip the pair
                    break;
                }
                break;
            }

            case DD: {
                DecodeResult delay_inst; 
                if (inst.numBytes == 4) {
                    // Ordinary instruction. Look at the delay slot
                    delay_inst = decodeInstruction(address+4);
                }
                else {
                    // Must be a prologue or epilogue or something.
                    delay_inst = nop_inst;
                    // Should be no need to adjust the coverage; the number of
                    // bytes should take care of it
                }
                    
                RTL* delay_rtl = delay_inst.rtl;

                // Display RTL representation if asked
                if (Boomerang::get()->printRtl && delay_rtl != NULL)
                    delay_rtl->print(os);

                switch(delay_inst.type) {
                case NOP:
                case NCT: {
                    sequentialDecode = case_DD(address, pBF->getTextDelta(), inst,
                        delay_inst, BB_rtls, cfg, targetQueue, proc, callSet);
                    break;
                }
                default:
                    case_unhandled_stub(address);
                    break;
                }
                break;
            }

            case SCD: {
                // Always execute the delay instr, and branch if
                // condition is met.
                // Normally, the delayed instruction moves in front
                // of the branch. But if it affects the condition
                // codes, we may have to duplicate it as an orphan in
                // the true leg of the branch, and fall through to the
                // delay instruction in the "false" leg
                // Instead of moving the delay instruction to an orphan BB, we
                // may have a duplicate of the delay instruction just before the
                // target; if so, we can branch to that and not need the orphan
                // We do just a binary comparison; that may fail to make this
                // optimisation if the instr has relative fields.

                DecodeResult delay_inst = decodeInstruction(address+4);
                RTL* delay_rtl = delay_inst.rtl;

                // Display low level RTL representation if asked
                if (Boomerang::get()->printRtl && delay_rtl != NULL)
                    delay_rtl->print(os);

                switch(delay_inst.type) {
                case NOP:
                case NCT:
                {
                    sequentialDecode = case_SCD(address, pBF->getTextDelta(),
                      pBF->getLimitTextHigh(), inst, delay_inst, BB_rtls, cfg,
                      targetQueue);
                    break;
                }
                default:
                    if (delay_inst.rtl->getList().back()->getKind() ==
                      STMT_CALL) {
                        // Assume it's the move/call/move pattern
                        sequentialDecode = case_SCD(address,
                          pBF->getTextDelta(), pBF->getLimitTextHigh(),
                          inst, delay_inst, BB_rtls, cfg, targetQueue);
                        break;
                    }
                    case_unhandled_stub(address);
                    break;
                }
                break;
            }

            case SCDAN:
            {
                // Execute the delay instruction if the branch is taken;
                // skip (anull) the delay instruction if branch not taken.
                DecodeResult delay_inst = decodeInstruction(address+4);
                RTL* delay_rtl = delay_inst.rtl;

                // Display RTL representation if asked
                if (Boomerang::get()->printRtl && delay_rtl != NULL)
                    delay_rtl->print(os);

                switch(delay_inst.type) {
                case NOP:
                {
                    // This is an ordinary two-way branch.
                    // Add the branch to the list of RTLs for this BB
                    BB_rtls->push_back(rtl);
                    // Create the BB and add it to the CFG
                    PBB pBB = cfg->newBB(BB_rtls, TWOWAY, 2);
                    if (pBB == 0) {
                        sequentialDecode = false; break;
                    }
                    // Visit the destination of the branch; add "true" leg
                    ADDRESS uDest = stmt_jump->getFixedDest();
                    handleBranch(uDest, pBF->getLimitTextHigh(), pBB, cfg, targetQueue);
                    // Add the "false" leg: point past the delay inst
                    cfg->addOutEdge(pBB, address+8);
                    address += 8;           // Skip branch and delay
                    BB_rtls = NULL;         // Start new BB
                    break;
                }

                case NCT:
                {
                    sequentialDecode = case_SCDAN(address, pBF->getTextDelta(),
                      pBF->getLimitTextHigh(), inst, delay_inst, BB_rtls, cfg,
                      targetQueue);
                    break;
                }

                default:
                    case_unhandled_stub(address);
                    address = address + 8;
                    break;
                }
                break;
            }
            default:            // Others are non sparc cases
                break;
            }

            // Display RTL representation if asked
            //if (progOptions.rtl && (inst.rtl != NULL))
if (0)          // SETTINGS
                inst.rtl->print(os);

            // If sequentially decoding, check if the next address happens to
            // be the start of an existing BB. If so, finish off the current BB
            // (if any RTLs) as a fallthrough, and  no need to decode again
            // (unless it's an incomplete BB, then we do decode it).
            // In fact, mustn't decode twice, because it will muck up the
            // coverage, but also will cause subtle problems like add a call
            // to the list of calls to be processed, then delete the call RTL
            // (e.g. Pentium 134.perl benchmark)
            if (sequentialDecode && cfg->isLabel(address)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    PBB pBB = cfg->newBB(BB_rtls, FALL, 1);
                    // Add an out edge to this address
                    if (pBB) {
                        cfg->addOutEdge(pBB, address);
                        BB_rtls = NULL;         // Need new list of RTLs
                    }
                }
                // Pick a new address to decode from, if the BB is complete
                if (!cfg->isIncomplete(address))
                    sequentialDecode = false;
            }

        }       // while (sequentialDecode)

        // Add this range to the coverage
        //proc->addRange(start, address);

        // Must set sequentialDecode back to true
        sequentialDecode = true;
    }


    // Add the callees to the set of CallStatements to proces for parameter recovery,
    // and also to the Prog object
    for (std::set<CallStatement*>::iterator it = callSet.begin(); it != callSet.end();
      it++) {
        ADDRESS dest = (*it)->getFixedDest();
        // Don't speculatively decode procs that are outside of the main text
        // section, apart from dynamically linked ones (in the .plt)
        if (pBF->IsDynamicLinkedProc(dest) || !spec ||
          (dest < pBF->getLimitTextHigh())) {
            cfg->addCall(*it);
            // Don't visit the destination of a register call
            //if (dest != NO_ADDRESS) newProc(proc->getProg(), dest);
            if (dest != NO_ADDRESS) proc->getProg()->setNewProc(dest);
        }
    }

    // MVE: Not 100% sure this is the right place for this
    proc->setEntryBB();

    return true;
}

/*==============================================================================
 * FUNCTION:      emitNop
 * OVERVIEW:      Emit a null RTL with the given address.
 * PARAMETERS:    pRtls - List of RTLs to append this instruction to
 *                uAddr - Native address of this instruction
 * RETURNS:       <nothing>
 *============================================================================*/
void SparcFrontEnd::emitNop(std::list<RTL*>* pRtls, ADDRESS uAddr)
{
    // Emit a null RTL with the given address. Required to cope with
    // SKIP instructions. Yes, they really happen, e.g. /usr/bin/vi 2.5
    RTL* pRtl = new RTL;
    pRtl->updateAddress(uAddr);
    pRtls->push_back(pRtl);
}

/*==============================================================================
 * FUNCTION:      emitCopyPC
 * OVERVIEW:      Emit the RTL for a call $+8 instruction, which is merely
 *                  %o7 = %pc
 * NOTE:          Assumes that the delay slot RTL has already been pushed; we
 *                  must push the semantics BEFORE that RTL, since the delay
 *                  slot instruction may use %o7. Example:
 *                  CALL $+8            ! This code is common in startup code
 *                  ADD  %o7, 20, %o0
 * PARAMETERS:    pRtls - list of RTLs to append to
 *                uAddr - native address for the RTL
 * RETURNS:       <nothing>
 *============================================================================*/
void SparcFrontEnd::emitCopyPC(std::list<RTL*>* pRtls, ADDRESS uAddr)
{
    // Emit %o7 = %pc
    Assign* a = new Assign(32,
        new Unary(opRegOf, new Const(15)),      // %o7 == r[15]
        new Terminal(opPC));
    // Add the Exp to an RTL
    RTL* pRtl = new RTL(uAddr);
    pRtl->appendStmt(a);
    // Add the RTL to the list of RTLs, but to the second last position
    pRtls->insert(--pRtls->end(), pRtl);
}

/*==============================================================================
 * FUNCTION:        helperFunc
 * OVERVIEW:        Checks for sparc specific helper functions like .urem,
 *                      which have specific sematics.
 * NOTE:            This needs to be handled in a resourcable way.
 * PARAMETERS:      dest: destination of the call (native address)
 *                  addr: address of current instruction (native addr)
 *                  lrtl: list of RTL* for current BB
 * RETURNS:         True if a helper function was found and handled; false
 *                      otherwise
 *============================================================================*/
// Append one assignment to a list of RTLs
void SparcFrontEnd::appendAssignment(Exp* lhs, Exp* rhs, int size, ADDRESS addr,
  std::list<RTL*>* lrtl) {
    Assign* a = new Assign(size, lhs, rhs);
    // Create an RTL with this one Statement
    std::list<Statement*>* lrt = new std::list<Statement*>;
    lrt->push_back(a);
    RTL* rtl = new RTL(addr, lrt);
    // Append this RTL to the list of RTLs for this BB
    lrtl->push_back(rtl);
}

/* Small helper function to build an expression with
 * *128* m[m[r[14]+64]] = m[r[8]] OP m[r[9]] */
void SparcFrontEnd::quadOperation(ADDRESS addr, std::list<RTL*>* lrtl, OPER op)
{
    Exp* lhs = new Unary(opMemOf,
        new Unary(opMemOf,
            new Binary(opPlus,
                new Unary(opRegOf, new Const(14)),
                new Const(64))));
    Exp* rhs = new Binary(op,
        new Unary(opMemOf,
            new Unary(opRegOf, new Const(8))),
        new Unary(opMemOf,
            new Unary(opRegOf, new Const(9))));
    appendAssignment(lhs, rhs, 128, addr, lrtl);
}

// Determine if this is a helper function, e.g. .mul. If so, append the
// appropriate RTLs to lrtl, and return true
bool SparcFrontEnd::helperFunc(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl)
{
    if (!pBF->IsDynamicLinkedProc(dest)) return false;
    const char* p = pBF->SymbolByAddress(dest);
    if (p == NULL) {
        std::cerr << "Error: Can't find symbol for PLT address " << std::hex << dest <<
          std::endl;
        return false;
    }
    std::string name(p);
    //if (progOptions.fastInstr == false)
if (0)  // SETTINGS!
        return helperFuncLong(dest, addr, lrtl, name);
    Exp* rhs;
    if (name == ".umul") {
        // %o0 * %o1
        rhs = new Binary(opMult,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".mul") {
        // %o0 *! %o1
        rhs = new Binary(opMults,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".udiv") {
        // %o0 / %o1
        rhs = new Binary(opDiv,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".div") {
        // %o0 /! %o1
        rhs = new Binary(opDivs,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".urem") {
        // %o0 % %o1
        rhs = new Binary(opMod,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".rem") {
        // %o0 %! %o1
        rhs = new Binary(opMods,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
//  } else if (name.substr(0, 6) == ".stret") {
//      // No operation. Just use %o0
//      rhs->push(idRegOf); rhs->push(idIntConst); rhs->push(8);
    } else if (name == "_Q_mul") {
        // Pointers to args are in %o0 and %o1; ptr to result at [%sp+64]
        // So semantics is m[m[r[14]] = m[r[8]] *f m[r[9]]
        quadOperation(addr, lrtl, opFMult);
        return true;
    } else if (name == "_Q_div") {
        quadOperation(addr, lrtl, opFDiv);
        return true;
    } else if (name == "_Q_add") {
        quadOperation(addr, lrtl, opFPlus);
        return true;
    } else if (name == "_Q_sub") {
        quadOperation(addr, lrtl, opFMinus);
        return true;
    } else {
        // Not a (known) helper function
        return false;
    }
    // Need to make an RTAssgn with %o0 = rhs
    Exp* lhs = new Unary(opRegOf, new Const(8));
    Assign* a = new Assign(32, lhs, rhs);
    // Create an RTL with this one Exp
    std::list<Statement*>* lrt = new std::list<Statement*>;
    lrt->push_back(a);
    RTL* rtl = new RTL(addr, lrt);
    // Append this RTL to the list of RTLs for this BB
    lrtl->push_back(rtl);
    return true;
}

/* Another small helper function to generate either (for V9):
    *64* tmp[tmpl] = sgnex(32, 64, r8) op sgnex(32, 64, r9)
    *32* r8 = truncs(64, 32, tmp[tmpl])
    *32* r9 = r[tmpl]@32:63
  or for v8:
    *32* r[tmp] = r8 op r9
    *32* r8 = r[tmp]
    *32* r9 = %Y
*/
void SparcFrontEnd::gen32op32gives64(OPER op, std::list<RTL*>* lrtl, ADDRESS addr) {
    std::list<Statement*>* ls = new std::list<Statement*>;
#ifdef V9_ONLY
    // tmp[tmpl] = sgnex(32, 64, r8) op sgnex(32, 64, r9)
    Statement* a = new Assign(64,
        new Unary(opTemp, new Const("tmpl")),
        new Binary(op,          // opMult or opMults
            new Ternary(opSgnEx, Const(32), Const(64),
                new Unary(opRegOf, new Const(8))),
            new Ternary(opSgnEx, Const(32), Const(64),
                new Unary(opRegOf, new Const(9)))));
    ls->push_back(a);
    // r8 = truncs(64, 32, tmp[tmpl]);
    a = new Assign(32,
        new Unary(opRegOf, new Const(8)),
        new Ternary(opTruncs, new Const(64), new Const(32),
            new Unary(opTemp, new Const("tmpl"))));
    ls->push_back(a);
    // r9 = r[tmpl]@32:63;
    a = new Assign(32,
        new Unary(opRegOf, new Const(9)),
        new Ternary(opAt, new Unary(opTemp, new Const("tmpl")),
            new Const(32), new Const(63)));
    ls->push_back(a);
#else
    // BTL: The .umul and .mul support routines are used in V7 code. We
    //      implsment these using the V8 UMUL and SMUL instructions.
    // BTL: In SPARC V8, UMUL and SMUL perform 32x32 -> 64 bit multiplies.
    //      The 32 high-order bits are written to %Y and the
    //      32 low-order bits are written to r[rd]. This is also true on V9
    //      although the high-order bits are also written into the
    //      32 high-order bits of the 64 bit r[rd].

    // r[tmp] = r8 op r9
    Assign* a = new Assign(32,
        new Unary(opTemp, new Const("tmp")),
        new Binary(op,          // opMult or opMults
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9))));
    ls->push_back(a);
    // r8 = r[tmp];  /* low-order bits */
    a = new Assign(32,
        new Unary(opRegOf, new Const(8)),
        new Unary(opTemp, new Const("tmp")));
    ls->push_back(a);
    // r9 = %Y;      /* high-order bits */
    a = new Assign(32,
        new Unary(opRegOf, new Const(8)),
        new Unary(opMachFtr, new Const("%Y")));
    ls->push_back(a);
#endif /* V9_ONLY */
    RTL* rtl = new RTL(addr, ls);
    lrtl->push_back(rtl);
    delete ls;
}

// This is the long version of helperFunc (i.e. -f not used). This does the
// complete 64 bit semantics
bool SparcFrontEnd::helperFuncLong(ADDRESS dest, ADDRESS addr, std::list<RTL*>* lrtl,
  std::string& name) {
    Exp* rhs;
    Exp* lhs;
    if (name == ".umul") {
        gen32op32gives64(opMult, lrtl, addr);
        return true;
    } else if (name == ".mul") {
        gen32op32gives64(opMults, lrtl, addr);
        return true;
    } else if (name == ".udiv") {
        // %o0 / %o1
        rhs = new Binary(opDiv,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".div") {
        // %o0 /! %o1
        rhs = new Binary(opDivs,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".urem") {
        // %o0 % %o1
        rhs = new Binary(opMod,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
    } else if (name == ".rem") {
        // %o0 %! %o1
        rhs = new Binary(opMods,
            new Unary(opRegOf, new Const(8)),
            new Unary(opRegOf, new Const(9)));
//  } else if (name.substr(0, 6) == ".stret") {
//      // No operation. Just use %o0
//      rhs->push(idRegOf); rhs->push(idIntConst); rhs->push(8);
    } else if (name == "_Q_mul") {
        // Pointers to args are in %o0 and %o1; ptr to result at [%sp+64]
        // So semantics is m[m[r[14]] = m[r[8]] *f m[r[9]]
        quadOperation(addr, lrtl, opFMult);
        return true;
    } else if (name == "_Q_div") {
        quadOperation(addr, lrtl, opFDiv);
        return true;
    } else if (name == "_Q_add") {
        quadOperation(addr, lrtl, opFPlus);
        return true;
    } else if (name == "_Q_sub") {
        quadOperation(addr, lrtl, opFMinus);
        return true;
    } else {
        // Not a (known) helper function
        return false;
    }
    // Need to make an RTAssgn with %o0 = rhs
    lhs = new Unary(opRegOf, new Const(8));
    appendAssignment(lhs, rhs, 32, addr, lrtl);
    return true;
}

#if 0       // Not using CSR
/*==============================================================================
 * FUNCTION:        setReturnLocations
 * OVERVIEW:        Set the return location for the given callee epilogue
 *                      to be the standard set of Sparc locations, using iReg
 *                      (either 8 for %o0 or 24 for %i0) to return integers
 * NOTE:            This is part of a hack that would go away if we could have
 *                  Logues that were both caller-prologues and callee-epilogues
 * PARAMETERS:      epilogue: pointer to a CalleeEpilogue object that is to
 *                      have it's return spec set
 *                  iReg: The register that integers are returned in (either
 *                      8 for %o0, or 24 for %i0)
 * RETURNS:         nothing
 *============================================================================*/
void SparcFrontEnd::setReturnLocations(CalleeEpilogue* epilogue, int iReg)
{
    // This function is somewhat similar to CSRParser::setReturnLocations() in
    // CSR/csrparser.y
    // First need to set up spec, which is a ReturnLocation object with the
    // four Sparc return locations
    typeToExp*Map retMap;
    Exp* ss;
    ss.push(idRegOf); ss.push(idIntConst); ss.push(iReg);
    retMap.insert(pair<Type, Exp*>(Type(::INTEGER), ss));
    retMap.insert(pair<Type, Exp*>(Type(::DATA_ADDRESS), ss));
    // Now we want ss to be %f0, i.e. r[32]
    ss.substIndex(2, 32);
    retMap.insert(pair<Type, Exp*>(Type(::FLOATP, 32), ss));
    // Now we want ss to be %f0to1, i.e. r[64]
    ss.substIndex(2, 64);
    retMap.insert(pair<Type, Exp*>(Type(::FLOATP, 64), ss));
    ReturnLocations spec(retMap);
    epilogue->setRetSpec(spec);
}
#endif

/*==============================================================================
 * FUNCTION:      construct
 * OVERVIEW:      Construct a new instance of SparcFrontEnd
 * PARAMETERS:    Same as the FrontEnd constructor, except decoder is **
 * RETURNS:       <nothing>
 *============================================================================*/
#ifdef DYNAMIC
extern "C" {
    SparcFrontEnd* construct(Prog *prog, NJMCDecoder** decoder) {
        SparcFrontEnd *fe = new SparcFrontEnd(prog);
        *decoder = fe->getDecoder();
        return fe;
    }
}
#endif

/*==============================================================================
 * FUNCTION:      SparcFrontEnd::SparcFrontEnd
 * OVERVIEW:      SparcFrontEnd constructor
 * NOTE:          Seems to be necessary to put this here; forces the vtable
 *                  entries to point to this dynamic linked library
 * PARAMETERS:    Same as the FrontEnd constructor
 * RETURNS:       <N/A>
 *============================================================================*/
SparcFrontEnd::SparcFrontEnd(BinaryFile *pBF)
  : FrontEnd(pBF)
{
    decoder = new SparcDecoder();
    nop_inst.numBytes = 0;          // So won't disturb coverage
    nop_inst.type = NOP;
    nop_inst.valid = true;
    nop_inst.rtl = new RTL();
}

// destructor
SparcFrontEnd::~SparcFrontEnd()
{
}

/*==============================================================================
 * FUNCTION:    GetMainEntryPoint
 * OVERVIEW:    Locate the starting address of "main" in the code section
 * PARAMETERS:  None
 * RETURNS:     Native pointer if found; NO_ADDRESS if not
 *============================================================================*/
ADDRESS SparcFrontEnd::getMainEntryPoint( bool &gotMain ) 
{
    ADDRESS start = pBF->GetMainEntryPoint();
    if( start != NO_ADDRESS ) return start;

    start = pBF->GetEntryPoint();
    if( start == NO_ADDRESS ) return NO_ADDRESS;

    return start;
}
