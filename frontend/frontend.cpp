/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
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
 * 09 Jul 02 - Mike: Fixed machine check for elf files (was checking endianness
 *                    rather than machine type)
 * 22 Nov 02 - Mike: Quelched warnings
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <queue>
#include <stdarg.h>         // For varargs
#include <sstream>
#ifndef WIN32
#include <dlfcn.h>          // dlopen, dlsym
#endif

#include "types.h"
#include "exp.h"
#include "proc.h"
#include "cfg.h"
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "decoder.h"
#include "sparcfrontend.h"
#include "pentiumfrontend.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"
#include "ansi-c-parser.h"

// Check that BOOMDIR is set
#ifndef WIN32
#ifndef BOOMDIR
#error BOOMDIR must be set!
#endif
#define LIBDIR BOOMDIR "/lib"
#endif

/*==============================================================================
 * FUNCTION:      FrontEnd::FrontEnd
 * OVERVIEW:      Construct the FrontEnd object
 * PARAMETERS:    prog: program being decoded
 * RETURNS:       <N/a>
 *============================================================================*/
FrontEnd::FrontEnd(BinaryFile *pBF)
 : pBF(pBF)
{}

FrontEnd* FrontEnd::instantiate(BinaryFile *pBF) {
    switch(pBF->GetMachine()) {
    case MACHINE_PENTIUM:
        return new PentiumFrontEnd(pBF);
    case MACHINE_SPARC:
        return new SparcFrontEnd(pBF);
    case MACHINE_HPRISC:
    case MACHINE_PALM:
        std::cerr << "Machine not supported\n";
    }
    return NULL;
}

FrontEnd* FrontEnd::Load(const char *fname) {
    BinaryFile *pBF = BinaryFile::Load(fname);
    if (pBF == NULL) return NULL;
    return instantiate(pBF);
}

// destructor
FrontEnd::~FrontEnd() {
}

bool FrontEnd::isWin32() {
    return pBF->GetFormat() == LOADFMT_PE;
}

FrontEnd *FrontEnd::createById(std::string &str, BinaryFile *pBF) {
    if (str == "pentium")
        return new PentiumFrontEnd(pBF);
    if (str == "sparc")
        return new SparcFrontEnd(pBF);
    return NULL;
}

void FrontEnd::readLibraryCatalog(const char *sPath, bool win32) {
    std::ifstream inf(sPath);
    if (!inf.good()) {
        std::cerr << "can't open `" << sPath << "'" << std::endl;
        exit(1);
    }

    while (!inf.eof()) {
        std::string sFile;
        inf >> sFile;
        size_t j = sFile.find('#');
        if (j != (size_t)-1)
            sFile = sFile.substr(0, j);
	if (sFile.size() > 0 && sFile[sFile.size()-1] == '\n')
	    sFile = sFile.substr(0, sFile.size()-1);
        if (sFile == "") continue;
        std::string sPath = Boomerang::get()->getProgPath() + "signatures/"
          + sFile;
        readLibrarySignatures(sPath.c_str(), win32);
    }
    inf.close();
}

void FrontEnd::readLibraryCatalog() {
    std::string sList = Boomerang::get()->getProgPath() +
      "signatures/common.hs";
    readLibraryCatalog(sList.c_str());
    sList = Boomerang::get()->getProgPath() + "signatures/" + getFrontEndId()
      + ".hs";
    readLibraryCatalog(sList.c_str());
    if (isWin32()) {
        sList = Boomerang::get()->getProgPath() + "signatures/win32.hs";
        readLibraryCatalog(sList.c_str(), true);
    }
}

Prog *FrontEnd::decode() 
{
    Prog *prog = new Prog(pBF, this);
    readLibraryCatalog();

    bool gotMain;
    ADDRESS a = getMainEntryPoint(gotMain);
    if (a == NO_ADDRESS) return false;

    decode(prog, a);
    return prog;
}

void FrontEnd::decode(Prog *prog, ADDRESS a) 
{
    prog->setNewProc(a);

    bool change = true;
    while (change) {
        change = false;
        PROGMAP::const_iterator it;
        for (Proc *pProc = prog->getFirstProc(it); pProc != NULL; 
          pProc = prog->getNextProc(it)) {
            if (pProc->isLib()) continue;
            UserProc *p = (UserProc*)pProc;
            if (p->isDecoded()) continue;

            // undecoded userproc.. decode it           
            change = true;
            std::ofstream os;
            int res = processProc(p->getNativeAddress(), p, os);
            if (res == 1)
                p->setDecoded();
            else
                break;
        }
    }
    prog->wellForm();
}

DecodeResult& FrontEnd::decodeInstruction(ADDRESS pc) {
    return decoder->decodeInstruction(pc, pBF->getTextDelta());
}

/*==============================================================================
 * FUNCTION:       FrontEnd::readLibrarySignatures
 * OVERVIEW:       Read the library signatures from a file
 * PARAMETERS:     The file to read from
 * RETURNS:        <nothing>
 *============================================================================*/
void FrontEnd::readLibrarySignatures(const char *sPath, bool win32) {
    std::ifstream ifs;

    ifs.open(sPath);

    if (!ifs.good()) {
        std::cerr << "can't open `" << sPath << "'" << std::endl;
        exit(1);
    }

    AnsiCParser *p = new AnsiCParser(ifs, false);
    std::string s = "-stdc-";
    if (win32) s = "-win32-";
    s += getFrontEndId();
    p->yyparse(s.c_str());

    for (std::list<Signature*>::iterator it = p->signatures.begin();
     it != p->signatures.end(); it++)
        librarySignatures[(*it)->getName()] = *it;

    delete p;
    ifs.close();
}

// get a library signature by name
Signature *FrontEnd::getLibSignature(const char *name) {
    Signature *signature;
    // Look up the name in the librarySignatures map
    std::map<std::string, Signature*>::iterator it;
    it = librarySignatures.find(name);
    if (it == librarySignatures.end()) {
        std::cerr << "unknown library function " << name << std::endl;
        // Get a default library signature
        if (isWin32())
            signature = Signature::instantiate("-win32-pentium", name);
        else {
            std::string s = "-stdc-";
            s += getFrontEndId();
            signature = Signature::instantiate(s.c_str(), name);
        } 
    }
    else {
        signature = (*it).second->clone();
    }
    return signature;
}

#if 0       // Note: moved to Prog::setNewProc
/*==============================================================================
 * FUNCTION:    FrontEnd::newProc
 * OVERVIEW:    Call this function when a procedure is discovered (usually by
 *                decoding a call instruction). That way, it is given a name
 *                that can be displayed in the dot file, etc. If we assign it
 *                a number now, then it will retain this number always
 * PARAMETERS:  prog  - program to add the new procedure to
 *              uAddr - Native address of the procedure entry point
 * RETURNS:     Pointer to the Proc object, or 0 if this is a deleted (not to
 *                be decoded) address
 *============================================================================*/
Proc* FrontEnd::newProc(Prog *prog, ADDRESS uAddr) {
    // this test fails when decoding sparc, why?  Please investigate - trent
    //assert(uAddr >= limitTextLow && uAddr < limitTextHigh);
    // Check if we already have this proc
    Proc* pProc = prog->findProc(uAddr);
    if (pProc == (Proc*)-1)         // Already decoded and deleted?
        return 0;                   // Yes, exit with 0
    if (pProc)
        // Yes, we are done
        return pProc;
    char* pName = pBF->SymbolByAddress(uAddr);
    bool bLib = pBF->IsDynamicLinkedProc(uAddr);
    if (pName == 0) {
        // No name. Give it a numbered name
        std::ostringstream ost;
        ost << "proc" << m_iNumberedProc++;
        pName = strdup(ost.str().c_str());
    }
    pProc = prog->newProc(pName, uAddr, bLib);
    return pProc;
}
#endif


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
 *                  this will be overridden in the FrontEnd derived class,
 *                  sometimes calling this function to do most of the work
 * RETURNS:       true for a good decode (no illegal instructions)
 *============================================================================*/
bool FrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os,
  bool spec /* = false */, PHELPER helperFunc) {
    PBB pBB;                    // Pointer to the current basic block
    INSTTYPE type;              // Cfg type of instruction (e.g. IRET)

    // Declare a queue of targets not yet processed yet. This has to be
    // individual to the procedure!
    TargetQueue targetQueue;

    // Similarly, we have a set of HLCall pointers. These may be disregarded
    // if this is a speculative decode that fails (i.e. an illegal instruction
    // is found). If not, this set will be used to add to the set of calls to
    // be analysed in the cfg, and also to call newProc()
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

    // ADDRESS initAddr = uAddr;
    int nTotalBytes = 0;
    ADDRESS lastAddr = uAddr;

    while ((uAddr = targetQueue.nextAddress(pCfg)) != NO_ADDRESS) {

        // The list of RTLs for the current basic block
        std::list<RTL*>* BB_rtls = new std::list<RTL*>();

        // Keep decoding sequentially until a CTI without a fall through branch
        // is decoded
        //ADDRESS start = uAddr;
        DecodeResult inst;
        while (sequentialDecode) {

            // Decode and classify the current instruction
            if (Boomerang::get()->traceDecoder)
                std::cout << "*" << std::hex << uAddr << "\t" << std::flush;

            // procedure out edge
            if (processed[uAddr] != NULL) {
                // Alert the watcher to the problem
                //ProgWatcher *w = prog->getWatcher();
                //if (w)
                //    w->alert_baddecode(uAddr);

                Proc *p = processed[uAddr];

                if (p == pProc) {
                    std::cerr << "Warning: attempt to redecode already decoded "
                      "outedge at " << std::hex << uAddr;
                    std::cerr << std::endl;
                } else {
                    std::cerr << "Warning: interprocedural outedge at " <<
                      std::hex << uAddr;
                    std::cerr << std::endl;
                }

                BB_rtls->push_back(new RTL(uAddr));  
                pBB = pCfg->newBB(BB_rtls, INVALID, 0);
                sequentialDecode = false; BB_rtls = NULL; continue;
            }

            processed[uAddr] = pProc;
            // Decode the inst at uAddr.
            inst = decodeInstruction(uAddr);

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid)
                return false;

            // Need to construct a new list of RTLs if a basic block has just
            // been finished but decoding is continuing from its lexical
            // successor
            if (BB_rtls == NULL)
                BB_rtls = new std::list<RTL*>();

            RTL* pRtl = inst.rtl;
            if (inst.valid == false) {
                // Alert the watcher to the problem
                //ProgWatcher *w = prog->getWatcher();
                //if (w)
                //    w->alert_baddecode(uAddr);

                // An invalid instruction. Most likely because a call did
                // not return (e.g. call _exit()), etc. Best thing is to
                // emit a INVALID BB, and continue with valid instructions
                std::cerr << "Warning: invalid instruction at " << std::hex
                  << uAddr;
                std::cerr << std::endl;
                // Emit the RTL anyway, so we have the address and maybe
                // some other clues
                BB_rtls->push_back(new RTL(uAddr));  
                pBB = pCfg->newBB(BB_rtls, INVALID, 0);
                sequentialDecode = false; BB_rtls = NULL; continue;
            }

            // alert the watcher that we have decoded an instruction
            //ProgWatcher *w = prog->getWatcher();
            //if (w)
            //    w->alert_decode(uAddr, inst.numBytes);
            nTotalBytes += inst.numBytes;           
    
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
                os << std::flush;        // Handy when the translator crashes
            }
    
            ADDRESS uDest;

            switch (pRtl->getKind())
            {

            case JUMP_RTL:
            {
                uDest = rtl_jump->getFixedDest();
    
                // Handle one way jumps and computed jumps separately
                if (uDest != NO_ADDRESS) {

                    if (processed[uDest] != NULL && processed[uDest] != pProc) {
                        // PROBLEM: interprocedural jump.  This is an
                        // optimization done by some compilers when a call
                        // is immediately followed by a return
                        if (processed[uDest]->getNativeAddress() == uDest) {
                            // replace with a call ret
                            HLCall *call = new HLCall(rtl_jump->getAddress());
                            call->setDest(uDest);
                            BB_rtls->push_back(call);
                            pBB = pCfg->newBB(BB_rtls, CALL, 1);
                            HLReturn *ret = new HLReturn(rtl_jump->getAddress()
                              +1);
                            std::list<RTL*> *ret_rtls = new std::list<RTL*>();
                            ret_rtls->push_back(ret);
                            PBB pret = pCfg->newBB(ret_rtls, RET, 0);
                            pret->addInEdge(pBB);
                            pBB->setOutEdge(0, pret);
                            sequentialDecode = false;
                            continue;
                        } else {
                            assert(false);
                        }
                    }

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
                    if (uDest < pBF->getLimitTextHigh()) {
                        targetQueue.visit(pCfg, uDest, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        std::cerr << "Error: Instruction at " << std::hex <<
                          uAddr << " branches beyond end of section, to "
                          << uDest << std::endl;
                    }
                }
                break;
            }

            case NWAYJUMP_RTL: {
                if (rtl_jump->getDest()->getOper() == opMemOf &&
                    rtl_jump->getDest()->getSubExp1()->getOper() == opIntConst && 
                    pBF->IsDynamicLinkedProcPointer(((Const*)rtl_jump->
                      getDest()->getSubExp1())->getAddr())) {
                    // jump to a library function
                    // replace with a call ret
                    std::string func = pBF->GetDynamicProcName(
                      ((Const*)rtl_jump->getDest()->getSubExp1())->getAddr());
                    HLCall *call = new HLCall(rtl_jump->getAddress());
                    call->setDest(rtl_jump->getDest()->clone());
                    LibProc *lp = pProc->getProg()->getLibraryProc(
                      func.c_str());
                    assert(lp);
                    call->setDestProc(lp);
                    BB_rtls->push_back(call);
                    pBB = pCfg->newBB(BB_rtls, CALL, 1);
                    HLReturn *ret = new HLReturn(rtl_jump->getAddress()+1);
                    std::list<RTL*> *ret_rtls = new std::list<RTL*>();
                    ret_rtls->push_back(ret);
                    PBB pret = pCfg->newBB(ret_rtls, RET, 0);
                    pret->addInEdge(pBB);
                    pBB->setOutEdge(0, pret);
                    sequentialDecode = false;
                    if (rtl_jump->getAddress() == pProc->getNativeAddress()) {
                        // its a thunk
                        //Proc *lp = prog->findProc(func.c_str());
                        pProc->setName(func.c_str());
                        func = std::string("__imp_") + func;

                        lp->setName(func.c_str());
                    }
                    callSet.insert(call);
                    continue;                   
                }
                BB_rtls->push_back(pRtl);
                // We create the BB as a COMPJUMP type, then change
                // to an NWAY if it turns out to be a switch stmt
                pBB = pCfg->newBB(BB_rtls, COMPJUMP, 0);
                // FIXME: This needs to call a new register branch processing
                // function
                if (isSwitch(pBB, rtl_jump->getDest(), pProc, pBF)) {
                    processSwitch(pBB, pBF->getTextDelta(), pCfg, targetQueue,
                      pBF);
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
                    if (uDest < pBF->getLimitTextHigh()) {
                        targetQueue.visit(pCfg, uDest, pBB);
                        pCfg->addOutEdge(pBB, uDest, true);
                    }
                    else {
                        std::cerr << "Error: Instruction at " << std::hex <<
                          uAddr << " branches beyond end of section, to "
                          << uDest << std::endl;
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

                if (call->getDest()->getOper() == opMemOf &&
                  call->getDest()->getSubExp1()->getOper() == opIntConst &&
                  pBF->IsDynamicLinkedProcPointer(((Const*)call->getDest()
                  ->getSubExp1())->getAddr())) {
                    // dynamic linked proc pointers are assumed to be static.
                    const char *nam = pBF->GetDynamicProcName(
                      ((Const*)call->getDest()->getSubExp1())->getAddr());
                    Proc *p = pProc->getProg()->getLibraryProc(nam);
                    call->setDestProc(p);
                }

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
                    if (uNewAddr && pProc->getProg()->findProc(uNewAddr) == NULL) {
                        callSet.insert(call);
                        if (processed[uNewAddr] != NULL) {
                            // PROBLEM: A procedure for this address has not
                            // been found but instructions at the destination
                            // have been decoded.  I imagine this could happen
                            // if a compiler was to optimize the pattern CALL
                            // followed by RET with a JUMP, and we came across
                            // this pattern before we came across a real call
                            // to the same procedure.
                            UserProc *p = (UserProc*)processed[uNewAddr];
                            assert(p->isLib() == false);
                            // remove this procedure's instructions from the map
                            std::list<ADDRESS> l;
                            for (std::map<ADDRESS, Proc*>::iterator it =
                              processed.begin(); it != processed.end(); it++)
                                if ((*it).second == p)
                                    l.push_back((*it).first);
                            for (std::list<ADDRESS>::iterator itl = l.begin();
                              itl != l.end(); itl++)
                                processed[*itl] = NULL;
                            // create a new procedure in the same program
                            //newProc(pProc->getProg(), uNewAddr);
                            pProc->getProg()->setNewProc(uNewAddr);
                            // undecode the old procedure
                            p->unDecode();
                            if (p == pProc) {
                                // DIFFERENT PROBLEM: The procedure is calling
                                // itself, but at a different entrypoint.
                                // The current procedure has been undecoded,
                                // therefore we have to get out of this
                                // function.  The callSet will be processed when
                                // this procedure is redecoded.
                                return false;
                            }
                        }
                        //newProc(pProc->getProg(), uNewAddr);
                        if (Boomerang::get()->traceDecoder)
                            std::cout << "p" << std::hex << uNewAddr << "\t"
                              << std::flush; 
                    }

                    // Check if this is the _exit or exit function. May prevent
                    // us from attempting to decode invalid instructions, and
                    // getting invalid stack height errors
                    const char* name = pBF->SymbolByAddress(uNewAddr);
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
                {
                    // Stop decoding sequentially
                    sequentialDecode = false;

                    // Add the RTL to the list
                    BB_rtls->push_back(pRtl);
                    // Create the basic block
                    pBB = pCfg->newBB(BB_rtls, RET, 0);

                    // If this ret pops anything other than the return address,
                    // this information can be useful in the proc
                    int popped = ((HLReturn*)pRtl)->getNumBytesPopped(); 
                    if (popped != 0) {
                        // this also gives us information about the calling
                        // convention
                        pProc->setBytesPopped(popped);
                    }

                    // Create the list of RTLs for the next basic block and
                    // continue with the next instruction.
                    BB_rtls = NULL;    // New RTLList for next BB
                }
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
            if (uAddr > lastAddr)
                lastAddr = uAddr;
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
                    if (getInst(a1+i+pBF->getTextDelta()) != NOP_INST) {
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

    //ProgWatcher *w = prog->getWatcher();
    //if (w)
    //    w->alert_done(pProc, initAddr, lastAddr, nTotalBytes);

    // Add the callees to the set of HLCalls to proces for CSR, and also
    // to the Prog object
    std::set<HLCall*>::iterator it;
    for (it = callSet.begin(); it != callSet.end(); it++) {
        ADDRESS dest = (*it)->getFixedDest();
        // Don't speculatively decode procs that are outside of the main text
        // section, apart from dynamically linked ones (in the .plt)
        if (pBF->IsDynamicLinkedProc(dest) || !spec ||
          (dest < pBF->getLimitTextHigh())) {
            pCfg->addCall(*it);
            // Don't visit the destination of a register call
            Proc *np = (*it)->getDestProc();
            if (np == NULL && dest != NO_ADDRESS) {
                //np = newProc(pProc->getProg(), dest);
                np = pProc->getProg()->setNewProc(dest);
            }
            if (np != NULL) {
                np->setFirstCaller(pProc);
                pProc->setCallee(np);
            }           
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
void TargetQueue::visit(Cfg* pCfg, ADDRESS uNewAddr, PBB& pNewBB) {
    // Find out if we've already parsed the destination
    bool bParsed = pCfg->label(uNewAddr, pNewBB);
    // Add this address to the back of the local queue,
    // if not already processed
    if (!bParsed) {
        targets.push(uNewAddr);
        if (Boomerang::get()->traceDecoder)
            std::cout << ">" << std::hex << uNewAddr << "\t" << std::flush;
    }
}

/*==============================================================================
 * FUNCTION:    TargetQueue::initial
 * OVERVIEW:    Seed the queue with an initial address
 * PARAMETERS:  uAddr: Native address to seed the queue with
 * RETURNS:     <nothing>
 *============================================================================*/
void TargetQueue::initial(ADDRESS uAddr) {
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
ADDRESS TargetQueue::nextAddress(Cfg* cfg) {
    while (!targets.empty())
    {
        ADDRESS address = targets.front();
        targets.pop();
        if (Boomerang::get()->traceDecoder)
            std::cout << "<" << std::hex << address << "\t" << std::flush;

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
RTL* decodeRtl(ADDRESS address, int delta, NJMCDecoder* decoder) {
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

bool isSwitch(PBB pbb, Exp* e, UserProc* p);
void processSwitch(PBB pbb, int delta, Cfg* cfg, TargetQueue& tq,
  BinaryFile* pBF);

/*==============================================================================
 * FUNCTION:    getInstanceFor
 * OVERVIEW:    Guess the machine required to decode this binary file;
 *                load the library and return an instance of FrontEnd
 * PARAMETERS:  sName: name of the binary file
 *              dlHandle: ref to a void* needed for closeInstance
 *              prog: the program to decode
 *              decoder: ref to ptr to decoder object
 * RETURNS:     Pointer to a FrontEnd* object, or 0 if not successful
 *============================================================================*/
typedef FrontEnd* (*constructFcn)(int, ADDRESS, NJMCDecoder**);
#define TESTMAGIC2(buf,off,a,b)     (buf[off] == a && buf[off+1] == b)
#define TESTMAGIC4(buf,off,a,b,c,d) (buf[off] == a && buf[off+1] == b && \
                                     buf[off+2] == c && buf[off+3] == d)
FrontEnd* FrontEnd::getInstanceFor( const char *sName, void*& dlHandle,
  BinaryFile *pBF, NJMCDecoder*& decoder) {
    FILE *f;
    char buf[64];
    std::string libName, machName;
    dlHandle = 0;           // Only used with DYNAMIC code

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
        if (buf[0x13] == 2) {       // Sparc, big endian
            machName = "sparc"; 
#ifndef DYNAMIC
            {
                SparcFrontEnd *fe = new SparcFrontEnd(pBF);
                decoder = fe->getDecoder();
                return fe;
            }
#endif
        }
        else if (buf[0x12] == 3) {
            machName = "pentium"; 
#ifndef DYNAMIC
            {
                PentiumFrontEnd *fe = new PentiumFrontEnd(pBF);
                decoder = fe->getDecoder();
                return fe;
            }
#endif
        } else {
            std::cerr << "Unknown ELF machine type " << std::hex <<
              (int)buf[0x12] << (int)buf[0x13] << std::endl;
            return NULL;
        }
    } else if( TESTMAGIC2( buf,0, 'M','Z' ) ) { /* DOS-based file */
        // This test could be strengthened a bit!
        machName = "pentium";
#ifndef DYNAMIC
        PentiumFrontEnd *fe = new PentiumFrontEnd(pBF);
        decoder = fe->getDecoder();
        return fe;
#endif
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

#ifdef DYNAMIC
    // Load the specific decoder library
    libName = std::string(LIBDIR) + "/libfront" + machName + ".so";
    dlHandle = dlopen(libName.c_str(), RTLD_LAZY);
    if (dlHandle == NULL) {
        std::cerr << "Could not open dynamic loader library " << libName <<
          std::endl;
        std::cerr << "dlerror is " << dlerror() << std::endl;
        return NULL;
    }
    // Use the handle to find the "construct" function
    constructFcn pFcn = (constructFcn) dlsym(dlHandle, "construct");
    if (pFcn == NULL) {
        std::cerr << "Front end library " << libName <<
            " does not have a construct function\n";
        return NULL;
    }

    // Call the construct function
    return (*pFcn)(pBF, &decoder);
#endif

    return 0;
}

/*==============================================================================
 * FUNCTION:    FrontEnd::closeInstance
 * OVERVIEW:    Close the library opened by getInstanceFor
 * PARAMETERS:  dlHandle: a void* from getInstanceFor
 * RETURNS:     Nothing
 *============================================================================*/
void FrontEnd::closeInstance(void* dlHandle) {
#ifdef DYNAMIC
    if (dlHandle) dlclose(dlHandle);
#endif
}

/*==============================================================================
 * FUNCTION:    FrontEnd::getProg
 * OVERVIEW:    Get a Prog object (mainly for testing and not decoding)
 * NOTE:        Caller to destroy
 * PARAMETERS:  None
 * RETURNS:     Pointer to a Prog object (with pFE and pBF filled in)
 *============================================================================*/
Prog* FrontEnd::getProg() {
    Prog *prog = new Prog(pBF, this);
    return prog;
}

