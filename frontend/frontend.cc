/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       frontend.cc
 * OVERVIEW:   This file contains common code for all front ends. The majority
 *              of frontend logic remains in the source dependent files such as
 *              frontsparc.cc
 *============================================================================*/

/*
 * $Revision$
 * 08 Apr 02 - Mike: Mods to adapt UQBT code to boomerang
 * 16 May 02 - Mike: Moved getMainEntry point here from prog
 */

#include <queue>
#include <stdarg.h>         // For varargs
#include <dlfcn.h>          // dlopen, dlsym

#include "frontend.h"
#include "exp.h"
#include "prog.h"
#include "decoder.h"
#include "BinaryFile.h"
extern Prog prog;

// Check that BOOMDIR is set
#ifndef BOOMDIR
#error BOOMDIR must be set!
#endif
#define LIBDIR BOOMDIR "/lib"

/*==============================================================================
 * FUNCTION:      FrontEnd::FrontEnd
 * OVERVIEW:      Construct the FrontEnd object
 * PARAMETERS:    delta: host - native address difference
 *                uUpper: highest+1 address in the text segment
 *                decoder: Ptr to the NJMCDecoder object
 * RETURNS:       <N/a>
 *============================================================================*/
FrontEnd::FrontEnd(int delta, ADDRESS uUpper, NJMCDecoder* decoder)
 : delta(delta), uUpper(uUpper), decoder(decoder)
{}

/*==============================================================================
 * FUNCTION:      FrontEnd::processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:    address - the address at which the procedure starts
 *                pProc - the procedure object
 *                spec - if true, this is a speculative decode
 *                helperFunc - pointer to a function to call (if not null) to
 *                  do a processor specific test for helper functions
 *                os - the output stream for .rtl output
 * NOTE:          This is a sort of generic front end. For many processors,
 *                  this will be overridden in the FrontEndSrc derived class,
 *                  sometimes calling this function to do most of the work
 * RETURNS:       true for a good decode (no illegal instructions)
 *============================================================================*/
bool FrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os,
    bool spec /* = false */, PHELPER helperFunc)
{
    PBB pBB;                    // Pointer to the current basic block
    INSTTYPE type;              // Cfg type of instruction (e.g. IRET)

    // Declare a queue of targets not yet processed yet. This has to be
    // individual to the procedure!
    TargetQueue targetQueue;

    // Similarly, we have a set of HLCall pointers. These may be disregarded
    // if this is a speculative decode that fails (i.e. an illegal instruction
    // is found). If not, this set will be used to add to the set of calls to
    // be analysed in the cfg, and also to call prog.visitProc()
    std::set<HLCall*> callSet;

    // Indicates whether or not the next instruction to be decoded is the
    // lexical successor of the current one. Will be true for all NCTs and for
    // CTIs with a fall through branch.
    bool sequentialDecode = true;

    Cfg* pCfg = pProc->getCFG();

    // If this is a speculative decode, the second time we decode the same
    // address, we get no cfg. Else an error.
    if (spec && (pCfg == 0))
        return false;
    assert(pCfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    targetQueue.initial(uAddr);

    // Clear the pointer used by the caller prologue code to access the last
    // call rtl of this procedure
    //decoder.resetLastCall();

    while ((uAddr = targetQueue.nextAddress(pCfg)) != NO_ADDRESS) {

        // The list of RTLs for the current basic block
        std::list<RTL*>* BB_rtls = new std::list<RTL*>();

        // Keep decoding sequentially until a CTI without a fall through branch
        // is decoded
        //ADDRESS start = uAddr;
        DecodeResult inst;
        while (sequentialDecode) {

            // Decode and classify the current instruction
            // FIXME: We need a settings system.
            // For now, uncomment and recompile...
//            if (progOptions.trace)
//              std::cout << "*" << std::hex << uAddr << "\t" << std::flush;

            // Decode the inst at uAddr.
            inst = decoder->decodeInstruction(uAddr, delta);

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid)
                return false;

            // Need to construct a new list of RTLs if a basic block has just
            // been finished but decoding is continuing from its lexical
            // successor
            if (BB_rtls == NULL)
                BB_rtls = new std::list<RTL*>();

            RTL* pRtl = inst.rtl;
            if (inst.valid == false)
            {
                // An invalid instruction. Most likely because a call did
                // not return (e.g. call _exit()), etc. Best thing is to
                // emit a INVALID BB, and continue with valid instructions
                std::cerr << "Warning: invalid instruction at " << std::hex << uAddr;
                std::cerr << std::endl;
                // Emit the RTL anyway, so we have the address and maybe
                // some other clues
                BB_rtls->push_back(new RTL(uAddr));  
                pBB = pCfg->newBB(BB_rtls, INVALID, 0);
                sequentialDecode = false; BB_rtls = NULL; continue;
            }
    
            if (pRtl == 0) {
                // This can happen if an instruction is "cancelled", e.g.
                // call to __main in a hppa program
                // Just ignore the whole instruction
                uAddr += inst.numBytes;
                continue;
            }

            HLJump* rtl_jump = static_cast<HLJump*>(pRtl);

            // Display RTL representation if asked
            // FIXME: Settings
//            if (progOptions.rtl) {
if (0) {
                pRtl->print(os);
                os << std::flush;            // Handy when the translator crashes
            }
    
            ADDRESS uDest;

            switch (pRtl->getKind())
            {

            case JUMP_RTL:
            {
                uDest = rtl_jump->getFixedDest();
    
                // Handle one way jumps and computed jumps separately
                if (uDest != NO_ADDRESS) {
                    BB_rtls->push_back(pRtl);
                    sequentialDecode = false;

                    pBB = pCfg->newBB(BB_rtls,ONEWAY,1);

                    // Exit the switch now and stop decoding sequentially if the
                    // basic block already existed
                    if (pBB == 0) {
                        sequentialDecode = false;
                        BB_rtls = NULL;
                        break;
                    }

                    // Add the out edge if it is to a destination within the
                    // procedure
                    if (uDest < uUpper) {
                        targetQueue.visit(pCfg, uDest, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        std::cerr << "Error: Instruction at " << std::hex << uAddr;
                        std::cerr << " branches beyond end of section, to ";
                        std::cerr << uDest << std::endl;
                    }
                }
                break;
            }

            case NWAYJUMP_RTL:
            {
                BB_rtls->push_back(pRtl);
                // We create the BB as a COMPJUMP type, then change
                // to an NWAY if it turns out to be a switch stmt
                pBB = pCfg->newBB(BB_rtls, COMPJUMP, 0);
                // FIXME: This needs to call a new register branch processing
                // function
                if (isSwitch(pBB, rtl_jump->getDest(), pProc)) {
                    processSwitch(pBB, delta, pCfg, targetQueue, pProc);
                }
                else { // Computed jump
                    // Not a switch statement
                    std::string sKind("JUMP");
                    if (type == I_COMPCALL) sKind = "CALL";
                    std::cerr << "Warning: COMPUTED " << sKind << " at "
                      << std::hex << uAddr << std::endl;
                    BB_rtls = NULL;    // New RTLList for next BB
                }
                sequentialDecode = false;
                break;     
            }



            case JCOND_RTL:
            {
                uDest = rtl_jump->getFixedDest();
                BB_rtls->push_back(pRtl);
                pBB = pCfg->newBB(BB_rtls, TWOWAY, 2);

                // Stop decoding sequentially if the basic block already existed
                // otherwise complete the basic block
                if (pBB == 0)
                    sequentialDecode = false;
                else {

                    // Add the out edge if it is to a destination within the
                    // procedure
                    if (uDest < uUpper) {
                        targetQueue.visit(pCfg, uDest, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        std::cerr << "Error: Instruction at " << std::hex << uAddr;
                        std::cerr << " branches beyond end of section, to ";
                        std::cerr << uDest << std::endl;
                    }

                    // Add the fall-through outedge
                    pCfg->addOutEdge(pBB, uAddr + inst.numBytes); 
                }

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = NULL;
                break;
            }

            case CALL_RTL:
            {
                HLCall* call = static_cast<HLCall*>(pRtl);

                // Treat computed and static calls seperately
                if (call->isComputed()) {
                    BB_rtls->push_back(pRtl);
                    pBB = pCfg->newBB(BB_rtls, COMPCALL, 1);

                    // Stop decoding sequentially if the basic block already
                    // existed otherwise complete the basic block
                    if (pBB == 0)
                        sequentialDecode = false;
                    else
                        pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                    // Add this call to the list of calls to analyse. We won't
                    // be able to analyse it's callee(s), of course.
                    callSet.insert(call);
                }
                else {      // Static call
                    // Find the address of the callee.
                    ADDRESS uNewAddr = call->getFixedDest();

                    // Calls with 0 offset (i.e. call the next instruction) are
                    // simply pushing the PC to the stack. Treat these as
                    // non-control flow instructions and continue.
                    if (uNewAddr == uAddr + inst.numBytes)
                        break;

                    // Check for a helper function, if the caller provided one)
                    if (helperFunc != NULL) {
                        if ((*helperFunc)(uNewAddr, uAddr, BB_rtls)) {
                            // We have already added to BB_rtls
                            break;
                        }
                    }

                    BB_rtls->push_back(pRtl);

                    // Add this non computed call site to the set of call
                    // sites which need to be analysed later.
                    //pCfg->addCall(call);
                    callSet.insert(call);

                    // Record the called address as the start of a new
                    // procedure if it didn't already exist.
                    if (uNewAddr && prog.findProc(uNewAddr) == NULL) {
                        callSet.insert(call);
                        //prog.visitProc(uNewAddr);
                        // FIXME: settings
//                        if (progOptions.trace)
//                            std::cout << "p" << std::hex << uNewAddr << "\t" << std::flush; 
                    }

                    // Check if this is the _exit or exit function. May prevent
                    // us from attempting to decode invalid instructions, and
                    // getting invalid stack height errors
                    const char* name = prog.pBF->SymbolByAddress(uNewAddr);
                    if (name && ((strcmp(name, "_exit") == 0) ||
                                 (strcmp(name,  "exit") == 0))) {
                        // Create the new basic block
                        pBB = pCfg->newBB(BB_rtls, CALL, 0);

                        // Stop decoding sequentially
                        sequentialDecode = false;
                    }
                    else {
                        // Create the new basic block
                        pBB = pCfg->newBB(BB_rtls, CALL, 1);

                        if (call->isReturnAfterCall()) {
                            // Constuct the RTLs for the new basic block
                            std::list<RTL*>* rtls = new std::list<RTL*>();
                            // The only RTL in the basic block is a high level
                            // return that doesn't have any RTs.
                            rtls->push_back(new HLReturn(0, NULL));
        
                            BasicBlock* returnBB = pCfg->newBB(rtls, RET, 0);
                            // Add out edge from call to return
                            pCfg->addOutEdge(pBB, returnBB);
                            // Put a label on the return BB (since it's an
                            // orphan); a jump will be reqd
                            pCfg->setLabel(returnBB);
                            pBB->setJumpReqd();
                            // Mike: do we need to set return locations?
                            // This ends the function
                            sequentialDecode = false;
                        }
                        else
                        {
                            // Add the fall through edge if the block didn't
                            // already exist
                            if (pBB != NULL)
                                pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                        }
                    }
                }

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = NULL;
                break;  
            }

            case RET_RTL:
                // Stop decoding sequentially
                sequentialDecode = false;

                // Add the RTL to the list
                BB_rtls->push_back(pRtl);
                // Create the basic block
                pBB = pCfg->newBB(BB_rtls, RET, 0);

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = NULL;    // New RTLList for next BB
                break;

            case SCOND_RTL:
                // This is just an ordinary instruction; no control transfer
                // Fall through
            case HL_NONE:
                // We must emit empty RTLs for NOPs, because they could be the
                // destinations of jumps (and splitBB won't work)
                // Just emit the current instr to the current BB
                BB_rtls->push_back(pRtl);
                break;
        
            } // switch (pRtl->getKind())

            uAddr += inst.numBytes;
            // Update the RTL's number of bytes for coverage analysis (only)
            inst.rtl->updateNumBytes(inst.numBytes);

            // If sequentially decoding, check if the next address happens to
            // be the start of an existing BB. If so, finish off the current BB
            // (if any RTLs) as a fallthrough, and  no need to decode again
            // (unless it's an incomplete BB, then we do decode it).
            // In fact, mustn't decode twice, because it will muck up the
            // coverage, but also will cause subtle problems like add a call
            // to the list of calls to be processed, then delete the call RTL
            // (e.g. Pentium 134.perl benchmark)
            if (sequentialDecode && pCfg->isLabel(uAddr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    PBB pBB = pCfg->newBB(BB_rtls, FALL, 1);
                    // Add an out edge to this address
                    if (pBB) {
                        pCfg->addOutEdge(pBB, uAddr);
                        BB_rtls = NULL;         // Need new list of RTLs
                    }
                }
                // Pick a new address to decode from, if the BB is complete
                if (!pCfg->isIncomplete(uAddr))
                    sequentialDecode = false;
            }

        }   // while sequentialDecode

        // Add this range to the coverage
//        pProc->addRange(start, uAddr);

        // Must set sequentialDecode back to true
        sequentialDecode = true;

    }   // while nextAddress() != NO_ADDRESS

#if 0
    // This pass is to remove up to 7 nops in a row between ranges.
    // These will be assumed to be padding for alignments of BBs
    // Just removes a lot of ranges that could otherwise be combined
    ADDRESS a1, a2;
    COV_CIT ii;
    Coverage temp;
    if (pProc->getFirstGap(a1, a2, ii)) {
        do {
            int gap = a2 - a1;
            if (gap < 8) {
                bool allNops = true;
                for (int i=0; i < gap; i+= NOP_SIZE) {
                    if (getInst(a1+i+delta) != NOP_INST) {
                        allNops = false;
                        break;
                    }
                }
                if (allNops)
                    // Remove this gap, by adding a range equal to the gap
                    // Note: it's not safe to add the range now, so we put
                    // the range into a temp Coverage object to be added later
                    temp.addRange(a1, a2);
            }
        } while (pProc->getNextGap(a1, a2, ii));
    }
#endif
    // Now add the ranges in temp
//    pProc->addRanges(temp);

    // Add the resultant coverage to the program's coverage
//    pProc->addProcCoverage();

    // Add the callees to the set of HLCalls to proces for CSR, and also
    // to the Prog object
    std::set<HLCall*>::iterator it;
    for (it = callSet.begin(); it != callSet.end(); it++) {
        ADDRESS dest = (*it)->getFixedDest();
        // Don't speculatively decode procs that are outside of the main text
        // section, apart from dynamically linked ones (in the .plt)
        if (prog.pBF->IsDynamicLinkedProc(dest) || !spec || (dest < uUpper)) {
            pCfg->addCall(*it);
            // Don't visit the destination of a register call
            if (dest != NO_ADDRESS) prog.visitProc(dest);
        }
    }

    return true;
}

/*==============================================================================
 * FUNCTION:    FrontEnd::getInst
 * OVERVIEW:    Fetch the smallest (nop-sized) instruction, in an endianness
 *                independent manner
 * NOTE:        Frequently overridden
 * PARAMETERS:  addr - host address to getch from
 * RETURNS:     An integer with the instruction in it
 *============================================================================*/
int FrontEnd::getInst(int addr)
{
    return (int)(*(unsigned char*)addr);
}


/*==============================================================================
 * FUNCTION:    TargetQueue::visit
 * OVERVIEW:    Visit a destination as a label, i.e. check whether we need to
 *              queue it as a new BB to create later.
 *              Note: at present, it is important to visit an address BEFORE
 *              an out edge is added to that address. This is because adding
 *              an out edge enters the address into the Cfg's BB map, and it
 *              looks like the BB has already been visited, and it gets
 *              overlooked. It would be better to have a scheme whereby the
 *              order of calling these functions (i.e. visit() and
 *              AddOutEdge()) did not matter.
 * PARAMETERS:  pCfg - the enclosing CFG
 *              uNewAddr - the address to be checked
 *              pNewBB - set to the lower part of the BB if the address
 *              already exists as a non explicit label (BB has to be split)
 * RETURNS:     <nothing>
 *============================================================================*/
void TargetQueue::visit(Cfg* pCfg, ADDRESS uNewAddr, PBB& pNewBB)
{
    // Find out if we've already parsed the destination
    bool bParsed = pCfg->label(uNewAddr, pNewBB);
    // Add this address to the back of the local queue,
    // if not already processed
    if (!bParsed) {
        targets.push(uNewAddr);
        // FIXME: settings
//        if (progOptions.trace)
//            std::cout << ">" << std::hex << uNewAddr << "\t" << dec << std::flush;
    }
}

/*==============================================================================
 * FUNCTION:    TargetQueue::initial
 * OVERVIEW:    Seed the queue with an initial address
 * PARAMETERS:  uAddr: Native address to seed the queue with
 * RETURNS:     <nothing>
 *============================================================================*/
void TargetQueue::initial(ADDRESS uAddr)
{
    targets.push(uAddr);
}

/*==============================================================================
 * FUNCTION:          TergetQueue::nextAddress
 * OVERVIEW:          Return the next target from the queue of non-processed
 *                    targets.
 * PARAMETERS:        cfg - the enclosing CFG
 * RETURNS:           The next address to process, or NO_ADDRESS if none
 *                      (targets is empty)
 *============================================================================*/
ADDRESS TargetQueue::nextAddress(Cfg* cfg)
{
    while (!targets.empty())
    {
        ADDRESS address = targets.front();
        targets.pop();
        // FIXME: settings
//        if (progOptions.trace)
//            std::cout << "<" << std::hex << address << "\t" << std::flush;

        // If no label there at all, or if there is a BB, it's incomplete,
        // then we can parse this address next
        if (!cfg->isLabel(address) || cfg->isIncomplete(address))
            return address;
    }
    return NO_ADDRESS;
}

/*==============================================================================
 * FUNCTION:      decodeRtl
 * OVERVIEW:      Decode the RTL at the given address
 * PARAMETERS:    address - native address of the instruction
 *                delta - difference between host and native addresses
 *                decoder - decoder object
 * NOTE:          Only called from findCoverage()
 * RETURNS:       a pointer to the decoded RTL
 *============================================================================*/
RTL* decodeRtl(ADDRESS address, int delta, NJMCDecoder* decoder)
{
    DecodeResult inst = 
        decoder->decodeInstruction(address, delta);

    // Define aliases to the RTLs so that they can be treated as a high
    // level types where appropriate.
    RTL*   rtl        = inst.rtl;

    // Set the size (a few things still need this)
    rtl->updateNumBytes(inst.numBytes);
    return rtl;
}

#if 0           // ? This must have been some weird parser idea; no longer used
/*==============================================================================
 * FUNCTION:       makeMachineDep
 * OVERVIEW:       Construct a special assignment expression, of this form:
 *                  MI(k) = a, b, ... (n constants)
 *                  where MI is the special opMachineInst unary, that takes
 *                  an argument that indicates which special instruction it
 *                  is.
 * EXAMPLE:        Sparc save might be k=1, n=1, a=the argument to the save
 *                  restore might be k=2, n=0 (so the list on the right will
 *                  just be the Terminal Null)
 * PARAMETERS:     k: instruction identifier
 * RETURNS:        Nothing
 *============================================================================*/
Exp* makeMachDep(int k, int n, ...) {
    Binary* ret = new Binary(opAssign);
    ret->setSubExp1(new Unary(opTargetInst, new Const(k)));
    if (n == 0) {
        ret->setSubExp2(new Terminal(opNil));
        return ret;
    }
    Exp* rhs;
    va_list(args);
    va_start(args, n);
    if (n == 1)
        rhs = new Const(va_arg(args, int));
    else {
        /* We have a string of opCommas; the last one gets a Const
        // E.g.    ,
        //        / \
        //       l1  ,
        //          / \
        //         l2  ,
        //            / \
        //           l3  l4     */
        Binary* cur = new Binary(opComma);
        cur->setSubExp1(new Const(va_arg(args, int)));
        rhs = cur;
        for (int i=1; i < n; i++) {
            if (i == n-1) {          // If last argument
                cur->setSubExp2(new Const(va_arg(args, int)));
            } else {
                Binary* tmp;
                cur->setSubExp2((tmp = new Binary(opComma)));
                cur = tmp;
                cur->setSubExp1(new Const(va_arg(args, int)));
            }
        }
    }
    ret->setSubExp2(rhs);
    return ret;
}
#endif

#if 0
/*==============================================================================
 * FUNCTION:    is286Push
 * OVERVIEW:    Return true if this RTL is likely to represent a 286 push
 *              instruction. Base this on "%sp = %sp -2" in the first rt
 *              (where for 286, %sp is r[4])
 * PARAMETERS:  pRTL - pointer to an RTL to be checked
 * RETURNS:     True if matched
 *============================================================================*/
static int spMinus2[6] = {idMinus, idRegOf, idIntConst, 4, idIntConst, 2};
static int regSp[3] = {idRegOf, idIntConst, 4};

bool is286Push(const HRTL* pRTL)
{
    int n = pRTL->getNumRT();
    if (n < 2) return false;
    RTAssgn* rt = (RTAssgn*)pRTL->elementAt(0);
    if (rt->getKind() != RTASSGN) return false;
    SemStr* ss = rt->getRHS();
    if (!ss->isArrayEqual(6, spMinus2)) return false;
    ss = rt->getLHS();
    if (!ss->isArrayEqual(3, regSp)) return false;
    return true;
}
#endif

/*==============================================================================
 * FUNCTION:    GetMainEntryPoint
 * OVERVIEW:    Locate the starting address of "main" in the code section
 * PARAMETERS:  None
 * RETURNS:     Native pointer if found; NO_ADDRESS if not
 *============================================================================*/
ADDRESS FrontEnd::getMainEntryPoint( void ) {
    ADDRESS start = prog.pBF->GetMainEntryPoint();
    if( start != NO_ADDRESS ) return start;

    start = prog.pBF->GetEntryPoint();
    if( start == NO_ADDRESS ) return NO_ADDRESS;

    if ((prog.pBF->GetFormat() == LOADFMT_PE ) ||
      (prog.pBF->GetFormat() == LOADFMT_EXE)) {
        int instCount = 100;
        int conseq = 0;
        ADDRESS addr = start;
        
        // Look for 3 calls in a row in the first 100 instructions, with
        // no other instructions between them. This is the "windows" pattern
        do {
            DecodeResult inst =
              decoder->decodeInstruction(addr, prog.textDelta);
            if ((inst.rtl->getKind() == CALL_RTL) &&
                ((HLCall*)inst.rtl)->getFixedDest() != NO_ADDRESS) {
                if (++conseq == 3)
                    // Success. Return the target of the last call
                    return ((HLCall*)inst.rtl)->getFixedDest();
            }
            else 
                conseq = 0;         // Must be consequitive
            addr += inst.numBytes;
        } while (--instCount);
#if 0       // Was for finding main in DOS 286 programs
        // Try another pattern; this one is for DOS programs. In the first
        // 120 instructions, look for 3 or more pushes, then a call. These
        // will be setting up envp, argv, and argc
        instCount = 120; addr = start; conseq = 0;
        do {
            DecodeResult inst = decoder.decodeInstruction(addr, prog.textDelta);
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

        // Not ideal; we must return start
        std::cerr << "main function not found\n";
        return start;
    }
    return NO_ADDRESS;
}

// Dummy for now
bool isSwitch(PBB pbb, Exp* e, UserProc* p) {return false;}
void processSwitch(PBB pbb, int delta, Cfg* cfg, TargetQueue& tq, UserProc* p)
{};

/*==============================================================================
 * FUNCTION:    getInstanceFor
 * OVERVIEW:    Guess the machine required to decode this binary file;
 *                load the library and return an instance of FrontEndSrc
 * PARAMETERS:  sName: name of the binary file
 *              dlHandle: ref to a void* needed for closeInstance
 *              delta: difference between host and native addresses
 *              ADDRESS uUpper: one past end of code segment (native addr)
 *              decoder: ref to ptr to decoder object
 * RETURNS:     Pointer to a FrontEndSrc* object, or 0 if not successful
 *============================================================================*/
typedef FrontEndSrc* (*constructFcn)(int, ADDRESS, NJMCDecoder**);
#define TESTMAGIC2(buf,off,a,b)     (buf[off] == a && buf[off+1] == b)
#define TESTMAGIC4(buf,off,a,b,c,d) (buf[off] == a && buf[off+1] == b && \
                                     buf[off+2] == c && buf[off+3] == d)
FrontEndSrc* FrontEnd::getInstanceFor( const char *sName, void*& dlHandle,
  int delta, ADDRESS uUpper, NJMCDecoder*& decoder) {
    FILE *f;
    char buf[64];
    std::string libName, machName;

    f = fopen (sName, "ro");
    if( f == NULL ) {
        std::cerr << "Unable to open binary file: " << sName << std::endl;
        fclose(f);
        return NULL;
    }
    fread (buf, sizeof(buf), 1, f);
    fclose(f);

    if( TESTMAGIC4(buf,0, '\x7F','E','L','F') ) {
        // ELF Binary; they have an enum for the machine!
        switch (buf[5]) {
            case 2: machName = "sparc"; break;
            case 1: machName = "pentium"; break;   // The book says otherwise...
            default:
                std::cerr << "Unknown ELF machine type " << (int)buf[5] << std::endl;
                return NULL;
        }
    } else if( TESTMAGIC2( buf,0, 'M','Z' ) ) { /* DOS-based file */
        // This test could be strengthened a bit!
        machName = "pentium";
    } else if( TESTMAGIC4( buf,0x3C, 'a','p','p','l' ) ||
               TESTMAGIC4( buf,0x3C, 'p','a','n','l' ) ) {
        /* PRC Palm-pilot binary */
        machName = "mc68k";
    } else if( buf[0] == 0x02 && buf[2] == 0x01 &&
               (buf[1] == 0x10 || buf[1] == 0x0B) &&
               (buf[3] == 0x07 || buf[3] == 0x08 || buf[4] == 0x0B) ) {
        /* HP Som binary (last as it's not really particularly good magic) */
        libName = "hppa";
    } else {
        std::cerr << "FrontEnd::getInstanceFor: unrecognised binary file" <<
            sName << std::endl;
        return NULL;
    }
    
    // Load the specific decoder library
    libName = std::string(LIBDIR) + "/libfront" + machName + ".so";
    dlHandle = dlopen(libName.c_str(), RTLD_LAZY);
    if (dlHandle == NULL) {
        std::cerr << "Could not open dynamic loader library " << libName << std::endl;
        std::cerr << "dlerror is " << dlerror() << std::endl;
        return NULL;
    }
    // Use the handle to find the "construct" function
    constructFcn pFcn = (constructFcn) dlsym(dlHandle, "construct");
    if (pFcn == NULL) {
        std::cerr << "Front end library " << libName << " does not have a construct "
            "function\n";
        return NULL;
    }

    // Call the construct function
    return (*pFcn)(delta, uUpper, &decoder);
}

/*==============================================================================
 * FUNCTION:    FrontEnd::closeInstance
 * OVERVIEW:    Close the library opened by getInstanceFor
 * PARAMETERS:  fe: ptr to the FrontEndSrc object returned by getInstanceFor
 *              dlHandle: a void* from getInstanceFor
 * RETURNS:     Nothing
 *============================================================================*/
void FrontEnd::closeInstance(FrontEnd* fe, void* dlHandle) {
    delete fe;
    dlclose(dlHandle);
}
