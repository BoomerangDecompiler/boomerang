/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       front68k.cc
 * OVERVIEW:   This file contains routines to manage the decoding of mc68k
 *             instructions and the instantiation to RTLs. These functions
 *             replace Frontend.cc for decoding mc68k instructions.
 *============================================================================*/

/*
 * $Revision$
 * 14 Feb 00 - Mike: converted from front386.cc
 * 23 Feb 00 - Mike: added logic for isReturnAfterCall
 * 09 Nov 00 - Cristina: Added support to generate rtl code to a file
 * 31 Mar 01 - Mike: getFixedDest() returns Address::INVALID for non fixed addresses
 * 31 Jul 01 - Brian: New class HRTL replaces RTlist. Renamed LRTL to HRTLList.
 */

#include "boomerang/global.h"
#include "boomerang/include/frontend.h"
#include "boomerang/include/decoder.h" // prototype for decodeInstruction()
#include "boomerang/db/RTL.h"
#include "boomerang/db/CFG.h"
#include "boomerang/ss.h"
#include "boomerang/db/Prog.h" // For findProc()
#include "boomerang/db/proc/Proc.h"
#include "boomerang/options.h"
#include "boomerang/csr.h"                    // For class CalleeEpilogue

/***************************************************************************/ /**
 * Forward declarations.
 *============================================================================*/

/***************************************************************************/ /**
 * File globals.
 *============================================================================*/

// Queues used by various functions
queue<ADDRESS> qLabels; // Queue of labels this procedure

/***************************************************************************/ /**
 * Forward declarations.
 *============================================================================*/
void initCti();                                                               // Imp in cti68k.cc

struct SemCmp
{
    bool operator()(const SemStr& ss1, const SemStr& ss2) const;
};

bool SemCmp::operator()(const SemStr& ss1, const SemStr& ss2) const
{
    return ss1 < ss2;
}


// A map from semantic string to integer (for identifying branch statements)
static map<SemStr, int, SemCmp> condMap;

int arrConds[12][7] =
{
    { idZF },                                             // HLJCOND_JE: Z
    { idNot, idZF},                                       // HLJCOND_JNE: ~Z
    { idBitXor, idNF, idOF},                              // HLJCOND_JSL: N ^ O
    { idBitOr, idBitXor, idNF, idOF, idZF},               // HLJCOND_JSLE: (N ^ O) | Z
    { idNot, idBitXor, idNF, idOF},                       // HLJCOND_JGES: ~(N ^ O)
    { idBitAnd, idNot, idBitXor, idNF, idOF, idNot, idZF}, // HLJCOND_JSG: ~(N ^ O) & ~Z
    { idCF },                                             // HLJCOND_JUL: C
    { idBitOr, idCF, idZF},                               // HLJCOND_JULE: C | Z
    { idNot, idCF},                                       // HLJCOND_JUGE: ~C
    { idBitAnd, idNot, idCF, idNot, idZF},                // HLJCOND_JUG: ~C & ~Z
    { idNF },                                             // HLJCOND_JMI: N
    { idNot, idNF},                                       // HLJCOND_JPOS: ~N
};

// Ugly. The lengths of the above arrays.
int condLengths[12] = { 1, 2, 3, 5, 4, 7, 1, 3, 2, 5, 1, 2 };

/***************************************************************************/ /**
 * FUNCTION:      initFront
 * OVERVIEW:      Initialise the front end.
 * PARAMETERS:    <none>
 *============================================================================*/
void initFront()
{
    for (int i = 0; i < 12; i++) {
        SemStr ss;
        ss.pushArr(condLengths[i], arrConds[i]);
        condMap[ss] = i;
    }
}


// Get the condition, given that it's something like %NF ^ %OF
JCOND_TYPE getCond(const SemStr *pCond)
{
    map<SemStr, int, SemCmp>::const_iterator mit;

    mit = condMap.find(*pCond);

    if (mit == condMap.end()) {
        ostrstream os;
        os << "Condition ";
        pCond->print(os);
        os << " not known";
        error(os.str());
        return (JCOND_TYPE)0;
    }

    return (JCOND_TYPE)(*mit).second;
}


/***************************************************************************/ /**
 * FUNCTION:      FrontEndSrc::processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:    address - the address at which the procedure starts
 *                pProc - the procedure object
 *                spec - true if a speculative decode
 *                os - output stream for rtl output
 * \returns        True if successful decode
 *============================================================================*/
bool FrontEndSrc::processProc(ADDRESS uAddr, UserProc *pProc, ofstream& os, bool spec /* = false */)
{
    // Call the base class to do all of the work
    return FrontEnd::processProc(uAddr, pProc, os, spec);
}


#if 0

/***************************************************************************//**
 * FUNCTION:      processProc
 * OVERVIEW:      Process a procedure, given a native (source machine) address.
 * PARAMETERS:    address - the address at which the procedure starts
 *                delta - the offset of the above address from the logical
 *                  address at which the procedure starts (i.e. the one
 *                  given by dis)
 *                uUpper - the highest address of the text segment
 *                pProc - the procedure object
 *                decoder - NJMCDecoder object
 *
 *============================================================================*/
void processProc(ADDRESS uAddr, int delta, ADDRESS uUpper, UserProc *pProc,
                 NJMCDecoder& decoder)
{
    PBB      pBB;               // Pointer to the current basic block
    INSTTYPE type;              // Cfg type of instruction (e.g. IRET)

    // Declare a queue of targets not yet processed yet. This has to be
    // individual to the procedure!
    TARGETS targets;

    // Indicates whether or not the next instruction to be decoded is the
    // lexical successor of the current one. Will be true for all NCTs and for
    // CTIs with a fall through branch.
    bool sequentialDecode = true;

    Cfg *pCfg = pProc->getCFG();

    // Initialise the queue of control flow targets that have yet to be decoded.
    targets.push(uAddr);

    // Clear the pointer used by the caller prologue code to access the last
    // call rtl of this procedure
    // decoder.resetLastCall();

    while ((uAddr = nextAddress(targets, pCfg)) != 0) {
        // The list of RTLs for the current basic block
        list<HRTL *> *BB_rtls = new list<HRTL *>();

        // Keep decoding sequentially until a CTI without a fall through branch
        // is decoded
        ADDRESS      start = uAddr;
        DecodeResult inst;

        while (sequentialDecode) {
            // Decode and classify the current instruction
            if (progOptions.trace) {
                cout << "*" << hex << uAddr << "\t" << flush;
            }

            // Decode the inst at uAddr.
            inst = decoder.decodeInstruction(uAddr, delta, pProc);

            // Need to construct a new list of RTLs if a basic block has just
            // been finished but decoding is continuing from its lexical
            // successor
            if (BB_rtls == nullptr) {
                BB_rtls = new list<HRTL *>();
            }

            HRTL *pRtl = inst.rtl;

            if (inst.numBytes == 0) {
                // An invalid instruction. Most likely because a call did
                // not return (e.g. call _exit()), etc. Best thing is to
                // emit a INVALID BB, and continue with valid instructions
                ostrstream ost;
                ost << "invalid instruction at " << hex << uAddr;
                warning(str(ost));
                // Emit the RTL anyway, so we have the address and maybe
                // some other clues
                BB_rtls->push_back(new RTL(uAddr));
                pBB = pCfg->newBB(BB_rtls, INVALID, 0);
                sequentialDecode = false;
                BB_rtls          = nullptr;
                continue;
            }

            HLJump *rtl_jump = static_cast<HLJump *>(pRtl);

            // Display RTL representation if asked
            if (progOptions.rtl) {
                pRtl->print();
            }

            ADDRESS uDest;

            switch (pRtl->getKind())
            {
            case JUMP_HRTL:
                uDest = rtl_jump->getFixedDest();

                // Handle one way jumps and computed jumps separately
                if (uDest != Address::INVALID) {
                    BB_rtls->push_back(pRtl);
                    sequentialDecode = false;

                    pBB = pCfg->newBB(BB_rtls, ONEWAY, 1);

                    // Exit the switch now and stop decoding sequentially if the
                    // basic block already existed
                    if (pBB == 0) {
                        sequentialDecode = false;
                        BB_rtls          = nullptr;
                        break;
                    }

                    // Add the out edge if it is to a destination within the
                    // procedure
                    if (uDest < uUpper) {
                        visit(pCfg, uDest, targets, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        ostrstream ost;
                        ost << "Error: Instruction at " << hex << uAddr;
                        ost << " branches beyond end of section, to ";
                        ost << uDest;
                        error(str(ost));
                    }
                }

                break;

            case NWAYJUMP_HRTL:
                BB_rtls->push_back(pRtl);
                // We create the BB as a COMPJUMP type, then change
                // to an NWAY if it turns out to be a switch stmt
                pBB = pCfg->newBB(BB_rtls, COMPJUMP, 0);

                if (isSwitch(pBB, rtl_jump->getDest(), pProc, pBF)) {
                    processSwitch(pBB, delta, pCfg, targets, pBF);
                }
                else {  // Computed jump
                        // Not a switch statement
                    ostrstream ost;
                    string     sKind("JUMP");

                    if (type == I_COMPCALL) {
                        sKind = "CALL";
                    }

                    ost << "COMPUTED " << sKind << " at "
                        << hex << uAddr << endl;
                    warning(str(ost));
                    BB_rtls = nullptr;    // New HRTLList for next BB
                }

                sequentialDecode = false;
                break;



            case JCOND_HRTL:
                uDest = rtl_jump->getFixedDest();
                BB_rtls->push_back(pRtl);
                pBB = pCfg->newBB(BB_rtls, TWOWAY, 2);

                // Stop decoding sequentially if the basic block already existed
                // otherwise complete the basic block
                if (pBB == 0) {
                    sequentialDecode = false;
                }
                else {
                    // Add the out edge if it is to a destination within the
                    // procedure
                    if (uDest < uUpper) {
                        visit(pCfg, uDest, targets, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        ostrstream ost;
                        ost << "Error: Instruction at " << hex << uAddr;
                        ost << " branches beyond end of section, to ";
                        ost << uDest;
                        error(str(ost));
                    }

                    // Add the fall-through outedge
                    pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                }

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = nullptr;
                break;

            case CALL_HRTL:
                {
                    HLCall *call = static_cast<HLCall *>(pRtl);

                    // Treat computed and static calls seperately
                    if (call->isComputed()) {
                        BB_rtls->push_back(pRtl);
                        pBB = pCfg->newBB(BB_rtls, COMPCALL, 1);

                        // Stop decoding sequentially if the basic block already
                        // existed otherwise complete the basic block
                        if (pBB == 0) {
                            sequentialDecode = false;
                        }
                        else {
                            pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                        }
                    }
                    else {  // Static call
                        BB_rtls->push_back(pRtl);

                        // Find the address of the callee.
                        ADDRESS uNewAddr = call->getFixedDest();

                        // Add this non computed call site to the set of call
                        // sites which need to be analysed later.
                        pCfg->addCall(call);

                        // Record the called address as the start of a new
                        // procedure if it didn't already exist.
                        if ((uNewAddr != Address::INVALID) &&
                            (prog.findProc(uNewAddr) == nullptr)) {
                            prog.visitProc(uNewAddr);

                            if (progOptions.trace) {
                                cout << "p" << hex << uNewAddr << "\t" << flush;
                            }
                        }

                        // Check if this is the _exit function. May prevent us from
                        // attempting to decode invalid instructions.
                        char *name = prog.pBF->SymbolByAddress(uNewAddr);

                        if (name && (strcmp(name, "_exit") == 0)) {
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
                                list<HRTL *> *rtls = new list<HRTL *>();
                                // The only RTL in the basic block is a high level
                                // return that doesn't have any RTs.
                                rtls->push_back(new HLReturn(0, nullptr));

                                BasicBlock *returnBB = pCfg->newBB(rtls, RET, 0);
                                // Add out edge from call to return
                                pCfg->addOutEdge(pBB, returnBB);
                                // Put a label on the return BB (since it's an
                                // orphan); a jump will be reqd
                                pCfg->setLabel(returnBB);
                                pBB->setJumpReqd();
                                // Give the enclosing proc a dummy callee epilogue
                                pProc->setEpilogue(new CalleeEpilogue("__dummy",
                                                                      list<string>()));
                                // Mike: do we need to set return locations?
                                // This ends the function
                                sequentialDecode = false;
                            }
                            else {
                                // Add the fall through edge if the block didn't
                                // already exist
                                if (pBB != nullptr) {
                                    pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                                }
                            }
                        }
                    }

                    // Create the list of RTLs for the next basic block and continue
                    // with the next instruction.
                    BB_rtls = nullptr;
                    break;
                }

            case RET_HRTL:
                // Stop decoding sequentially
                sequentialDecode = false;

                // Add the RTL to the list
                BB_rtls->push_back(pRtl);
                // Create the basic block
                pBB = pCfg->newBB(BB_rtls, RET, 0);

                // Create the list of RTLs for the next basic block and continue
                // with the next instruction.
                BB_rtls = nullptr;    // New HRTLList for next BB
                break;

            case SCOND_HRTL:
            // This is just an ordinary instruction; no control transfer
            // Fall through
            case LOW_LEVEL_HRTL:
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
            if (sequentialDecode && pCfg->existsBB(uAddr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    PBB pBB = pCfg->newBB(BB_rtls, FALL, 1);

                    // Add an out edge to this address
                    if (pBB) {
                        pCfg->addOutEdge(pBB, uAddr);
                        BB_rtls = nullptr;         // Need new list of RTLs
                    }
                }

                // Pick a new address to decode from, if the BB is complete
                if (!pCfg->isIncomplete(uAddr)) {
                    sequentialDecode = false;
                }
            }
        }   // while sequentialDecode

        // Add this range to the coverage
        pProc->addRange(start, uAddr);

        // Must set sequentialDecode back to true
        sequentialDecode = true;
    }   // while nextAddress()

    // This pass is to remove up to 3 nops between ranges.
    // These will be assumed to be padding for alignments of BBs
    // Possibly removes a lot of ranges that could otherwise be combined
    ADDRESS  a1, a2;
    COV_CIT  ii;
    Coverage temp;

    if (pProc->getFirstGap(a1, a2, ii)) {
        do {
            int gap = a2 - a1;

            if (gap < 8) {
                bool allNops = true;

                for (int i = 0; i < gap; i += 2) {
                    // Beware endianness! getWord will work properly
                    if (getWord(a1 + i + delta) != 0x4e71) {
                        allNops = false;
                        break;
                    }
                }

                if (allNops) {
                    // Remove this gap, by adding a range equal to the gap
                    // Note: it's not safe to add the range now, so we put
                    // the range into a temp Coverage object to be added later
                    temp.addRange(a1, a2);
                }
            }
        } while (pProc->getNextGap(a1, a2, ii));
    }

    // Now add the ranges in temp
    pProc->addRanges(temp);
}


#endif
