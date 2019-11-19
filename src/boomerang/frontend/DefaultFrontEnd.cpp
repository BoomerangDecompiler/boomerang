#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DefaultFrontEnd.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/decomp/IndirectJumpAnalyzer.h"
#include "boomerang/frontend/DecodeResult.h"
#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/util/CFGDotWriter.h"
#include "boomerang/util/log/Log.h"


DefaultFrontEnd::DefaultFrontEnd(Project *project)
    : IFrontEnd(project)
    , m_binaryFile(project->getLoadedBinaryFile())
    , m_program(project->getProg())
    , m_targetQueue(project->getSettings()->traceDecoder)
{
}


DefaultFrontEnd::~DefaultFrontEnd()
{
}


bool DefaultFrontEnd::initialize(Project *project)
{
    m_program    = project->getProg();
    m_binaryFile = project->getLoadedBinaryFile();

    if (!m_decoder) {
        return false;
    }

    return m_decoder->initialize(project);
}


bool DefaultFrontEnd::decodeEntryPointsRecursive(bool decodeMain)
{
    if (!decodeMain) {
        return true;
    }

    BinaryImage *image = m_program->getBinaryFile()->getImage();

    Interval<Address> extent(image->getLimitTextLow(), image->getLimitTextHigh());

    m_program->getProject()->alertStartDecode(extent.lower(),
                                              (extent.upper() - extent.lower()).value());

    bool gotMain;
    Address a = findMainEntryPoint(gotMain);
    LOG_VERBOSE("start: %1, gotMain: %2", a, (gotMain ? "true" : "false"));

    if (a == Address::INVALID) {
        std::vector<Address> entrypoints = findEntryPoints();

        for (auto &entrypoint : entrypoints) {
            if (!decodeRecursive(entrypoint)) {
                return false;
            }
        }

        return true;
    }

    decodeRecursive(a);
    m_program->addEntryPoint(a);

    if (!gotMain) {
        return true; // Decoded successfully, but patterns don't match a known main() pattern
    }

    static const char *mainName[] = { "main", "WinMain", "DriverEntry" };
    QString name                  = m_program->getSymbolNameByAddr(a);

    if (name == nullptr) {
        name = mainName[0];
    }

    for (auto &elem : mainName) {
        if (name != elem) {
            continue;
        }

        Function *proc = m_program->getFunctionByAddr(a);

        if (proc == nullptr) {
            LOG_WARN("No proc found for address %1", a);
            return false;
        }

        auto fty = std::dynamic_pointer_cast<FuncType>(Type::getNamedType(name));

        if (!fty) {
            LOG_WARN("Unable to find signature for known entrypoint %1", name);
        }
        else {
            proc->setSignature(fty->getSignature()->clone());
            proc->getSignature()->setName(name);
            // proc->getSignature()->setFullSig(true); // Don't add or remove parameters
            proc->getSignature()->setForced(true); // Don't add or remove parameters
        }

        break;
    }

    return true;
}


bool DefaultFrontEnd::decodeRecursive(Address addr)
{
    assert(addr != Address::INVALID);

    Function *newProc = m_program->getOrCreateFunction(addr);

    // Sometimes, we have to adjust the entry address since
    // the instruction at addr is just a jump to another address.
    addr = newProc->getEntryAddress();
    LOG_MSG("Starting decode at address %1", addr);
    UserProc *proc = static_cast<UserProc *>(m_program->getFunctionByAddr(addr));

    if (proc == nullptr) {
        LOG_MSG("No proc found at address %1", addr);
        return false;
    }
    else if (proc->isLib()) {
        LOG_MSG("NOT decoding library proc at address %1", addr);
        return false;
    }

    if (processProc(proc, addr)) {
        proc->setDecoded();
    }

    return m_program->isWellFormed();
}


bool DefaultFrontEnd::decodeUndecoded()
{
    bool change = true;
    LOG_MSG("Looking for undecoded procedures to decode...");

    while (change) {
        change = false;

        for (const auto &m : m_program->getModuleList()) {
            for (Function *function : *m) {
                if (function->isLib()) {
                    continue;
                }

                UserProc *userProc = static_cast<UserProc *>(function);

                if (userProc->isDecoded()) {
                    continue;
                }

                // undecoded userproc.. decode it
                change = true;

                if (!processProc(userProc, userProc->getEntryAddress())) {
                    return false;
                }

                userProc->setDecoded();

                // Break out of the loops if not decoding children
                if (!m_program->getProject()->getSettings()->decodeChildren) {
                    break;
                }
            }
        }

        if (!m_program->getProject()->getSettings()->decodeChildren) {
            break;
        }
    }

    return m_program->isWellFormed();
}


bool DefaultFrontEnd::decodeFragment(UserProc *proc, Address a)
{
    if (m_program->getProject()->getSettings()->traceDecoder) {
        LOG_MSG("Decoding fragment at address %1", a);
    }

    return processProc(proc, a);
}


bool DefaultFrontEnd::processProc(UserProc *proc, Address addr)
{
    LOG_VERBOSE("### Decoding proc '%1' at address %2 ###", proc->getName(), addr);

    ProcCFG *cfg = proc->getCFG();
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
            BasicBlock *existingBB = cfg->getBBStartingAt(addr);
            if (existingBB) {
                if (!bbInsns.empty()) {
                    // if bbInsns is not empty, the previous instruction was not a CTI.
                    // Complete the BB as a fallthrough
                    BasicBlock *newBB = cfg->createBB(BBType::Fall, bbInsns);
                    bbInsns.clear();
                    cfg->addEdge(newBB, existingBB);
                }

                if (!existingBB->getIR()->isIncomplete()) {
                    break; // do not disassemble the BB twice
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

            // alert the watchers that we have decoded an instruction
            numBytesDecoded += insn.m_size;
            m_program->getProject()->alertInstructionDecoded(addr, insn.m_size);

            // classify the current instruction. If it is not a CTI,
            // continue disassembling sequentially
            const bool isCTI = insn.isInGroup(MIGroup::Call) || insn.isInGroup(MIGroup::Jump) ||
                               insn.isInGroup(MIGroup::Ret);

            if (!isCTI) {
                addr += insn.m_size;
                bbInsns.push_back(insn);

                lastAddr = std::max(lastAddr, addr);
                continue;
            }

            // this is a CTI. Lift the instruction to gain access to call/jump semantics
            DecodeResult lifted;
            if (!liftInstruction(insn, lifted)) {
                LOG_ERROR("Cannot lift instruction!");

                // try next insruction in queue
                sequentialDecode = false;
                break;
            }

            bbInsns.push_back(insn);
            const RTL::StmtList &sl = lifted.rtl->getStatements();

            for (auto ss = sl.begin(); ss != sl.end(); ++ss) {
                SharedStmt s = *ss;
                s->setProc(proc); // let's do this really early!

                if (m_refHints.find(lifted.rtl->getAddress()) != m_refHints.end()) {
                    const QString &name(m_refHints[lifted.rtl->getAddress()]);
                    Address globAddr = m_program->getGlobalAddrByName(name);

                    if (globAddr != Address::INVALID) {
                        s->searchAndReplace(Const(globAddr),
                                            Unary::get(opAddrOf, Location::global(name, proc)));
                    }
                }

                s->simplify();
            }

            for (SharedStmt s : sl) {
                switch (s->getKind()) {
                case StmtType::Goto: {
                    std::shared_ptr<GotoStatement> jump = s->as<GotoStatement>();
                    assert(jump != nullptr);
                    const Address jumpDest = jump->getFixedDest();
                    sequentialDecode       = false;

                    // computed unconditional jumps have CaseStatement as last statement
                    // and not Goto
                    if (jumpDest == Address::INVALID) {
                        break;
                    }

                    // Static unconditional jump
                    BasicBlock *currentBB = cfg->createBB(BBType::Oneway, bbInsns);

                    // Exit the switch now if the basic block already existed
                    if (currentBB == nullptr) {
                        break;
                    }

                    // Add the out edge if it is to a destination within the procedure
                    if (jumpDest < m_program->getBinaryFile()->getImage()->getLimitTextHigh()) {
                        m_targetQueue.pushAddress(cfg, jumpDest, currentBB);
                        cfg->addEdge(currentBB, jumpDest);
                    }
                    else {
                        LOG_WARN("Goto instruction at address %1 branches beyond end of "
                                 "section, to %2",
                                 addr, jumpDest);
                    }
                } break;

                case StmtType::Case: {
                    // We create the BB as a COMPJUMP type, then change to an NWAY if it turns out
                    // to be a switch stmt
                    cfg->createBB(BBType::CompJump, bbInsns);
                    sequentialDecode = false;
                } break;

                case StmtType::Branch: {
                    std::shared_ptr<GotoStatement> jump = s->as<GotoStatement>();
                    BasicBlock *currentBB               = cfg->createBB(BBType::Twoway, bbInsns);

                    // Stop decoding sequentially if the basic block already existed otherwise
                    // complete the basic block
                    if (currentBB == nullptr) {
                        sequentialDecode = false;
                        break;
                    }

                    // Add the out edge if it is to a destination within the section
                    const Address jumpDest = jump->getFixedDest();

                    if (jumpDest < m_program->getBinaryFile()->getImage()->getLimitTextHigh()) {
                        m_targetQueue.pushAddress(cfg, jumpDest, currentBB);
                        cfg->addEdge(currentBB, jumpDest);
                    }
                    else {
                        LOG_WARN("Branch instruction at address %1 branches beyond end of "
                                 "section, to %2",
                                 addr, jumpDest);
                        currentBB->setType(BBType::Oneway);
                    }

                    // Add the fall-through outedge
                    cfg->addEdge(currentBB, addr + insn.m_size);
                } break;

                case StmtType::Call: {
                    std::shared_ptr<CallStatement> call = s->as<CallStatement>();

                    // Check for a dynamic linked library function
                    if (refersToImportedFunction(call->getDest())) {
                        // Dynamic linked proc pointers are treated as static.
                        const Address linkedAddr = call->getDest()->access<Const, 1>()->getAddr();
                        const QString name       = m_program->getBinaryFile()
                                                 ->getSymbols()
                                                 ->findSymbolByAddress(linkedAddr)
                                                 ->getName();

                        Function *function = proc->getProg()->getOrCreateLibraryProc(name);
                        call->setDestProc(function);
                        call->setIsComputed(false);

                        if (function->isNoReturn() || isNoReturnCallDest(function->getName())) {
                            sequentialDecode = false;
                        }
                    }

                    Address functionAddr = getAddrOfLibraryThunk(call, proc);
                    if (functionAddr != Address::INVALID) {
                        // Yes, it's a library function. Look up its name.
                        QString name = m_program->getBinaryFile()
                                           ->getSymbols()
                                           ->findSymbolByAddress(functionAddr)
                                           ->getName();

                        // Assign the proc to the call
                        Function *p = proc->getProg()->getOrCreateLibraryProc(name);

                        if (call->getDestProc()) {
                            // prevent unnecessary __imp procs
                            m_program->removeFunction(call->getDestProc()->getName());
                        }

                        call->setDestProc(p);
                        call->setIsComputed(false);
                        call->setDest(Location::memOf(Const::get(functionAddr)));

                        if (p->isNoReturn() || isNoReturnCallDest(p->getName())) {
                            sequentialDecode = false;
                        }
                    }

                    // Treat computed and static calls separately
                    if (call->isComputed()) {
                        BasicBlock *currentBB = cfg->createBB(BBType::CompCall, bbInsns);

                        // Stop decoding sequentially if the basic block already
                        // existed otherwise complete the basic block
                        if (currentBB == nullptr) {
                            sequentialDecode = false;
                        }
                        else {
                            cfg->addEdge(currentBB, addr + insn.m_size);
                            bbInsns.clear(); // start a new BB
                            sequentialDecode = true;
                        }
                    }
                    else {
                        // Static call
                        const Address callAddr = call->getFixedDest();

                        // Calls with 0 offset (i.e. call the next instruction) are simply
                        // pushing the PC to the stack. Treat these as non-control flow
                        // instructions and continue.
                        if (callAddr == addr + insn.m_size) {
                            break;
                        }

                        // Record the called address as the start of a new procedure if it
                        // didn't already exist.
                        if (!callAddr.isZero() && (callAddr != Address::INVALID) &&
                            (proc->getProg()->getFunctionByAddr(callAddr) == nullptr)) {
                            if (m_program->getProject()->getSettings()->traceDecoder) {
                                LOG_MSG("p%1", callAddr);
                            }
                        }

                        // Check if this is the _exit or exit function. May prevent us from
                        // attempting to decode invalid instructions, and getting invalid stack
                        // height errors
                        QString procName = m_program->getSymbolNameByAddr(callAddr);

                        if (procName.isEmpty() && refersToImportedFunction(call->getDest())) {
                            Address a = call->getDest()->access<Const, 1>()->getAddr();
                            procName  = m_program->getBinaryFile()
                                           ->getSymbols()
                                           ->findSymbolByAddress(a)
                                           ->getName();
                        }

                        if (!procName.isEmpty() && isNoReturnCallDest(procName)) {
                            // Make sure it has a return appended (so there is only one exit
                            // from the function)
                            cfg->createBB(BBType::Call, bbInsns);
                            sequentialDecode = false;
                        }
                        else {
                            // Create the new basic block
                            BasicBlock *currentBB = cfg->createBB(BBType::Call, bbInsns);

                            // Add the fall through edge if the block didn't
                            // already exist
                            if (currentBB != nullptr) {
                                cfg->addEdge(currentBB, addr + insn.m_size);
                            }

                            // start a new bb
                            bbInsns.clear();
                            sequentialDecode = true;
                        }
                    }
                } break;

                case StmtType::Ret: {
                    cfg->createBB(BBType::Ret, bbInsns);
                    sequentialDecode = false;
                } break;

                case StmtType::BoolAssign:
                    // This is just an ordinary instruction; no control transfer
                    // Fall through
                    // FIXME: Do we need to do anything here?
                case StmtType::Assign:
                case StmtType::PhiAssign:
                case StmtType::ImpAssign:
                    // Do nothing
                    break;
                case StmtType::INVALID: assert(false); break;
                }

                // This can happen if a high-level statement (e.g. a return)
                // is not the last statement in a rtl. Ignore the following statements.
                if (!sequentialDecode) {
                    break;
                }
            }

            if (lifted.reLift) {
                DecodeResult dummyLifted;
                bool ok;
                do {
                    ok = m_decoder->liftInstruction(insn, dummyLifted);
                } while (ok && dummyLifted.reLift);
            }

            addr += insn.m_size;
            lastAddr = std::max(lastAddr, addr);
        } // while sequentialDecode
    }     // while getNextAddress() != Address::INVALID

    CFGDotWriter().writeCFG({ proc }, "cfg-" + proc->getName() + ".dot");

    m_program->getProject()->alertFunctionDecoded(proc, startAddr, lastAddr, numBytesDecoded);

    LOG_VERBOSE("### Finished decoding proc '%1' ###", proc->getName());

    return true;
}


bool DefaultFrontEnd::liftProc(UserProc *proc)
{
    std::list<std::shared_ptr<CallStatement>> callList;

    ProcCFG *cfg = proc->getCFG();
    DecodeResult lifted;

    for (BasicBlock *currentBB : *cfg) {
        std::unique_ptr<RTLList> bbRTLs(new RTLList);

        for (const MachineInstruction &insn : currentBB->getInsns()) {
            if (!m_decoder->liftInstruction(insn, lifted)) {
                LOG_ERROR("Cannot lift instruction");
                return false;
            }

            if (lifted.reLift) {
                bool ok;

                LOG_ERROR("Cannot re-lift instruction");
                do {
                    ok = m_decoder->liftInstruction(insn, lifted);
                } while (ok && lifted.reLift);

                return false;
            }

            for (auto ss = lifted.rtl->begin(); ss != lifted.rtl->end(); ++ss) {
                SharedStmt s = *ss;
                s->setProc(proc); // let's do this really early!
                s->simplify();

                auto jumpStmt = std::dynamic_pointer_cast<GotoStatement>(s);

                // Check for a call to an already existing procedure (including self recursive
                // jumps), or to the PLT (note that a LibProc entry for the PLT function may not yet
                // exist)
                if (s->getKind() == StmtType::Goto) {
                    preprocessProcGoto(ss, jumpStmt->getFixedDest(), lifted.rtl->getStatements(),
                                       lifted.rtl.get());
                    s = *ss; // *ss can be changed within preprocessProcGoto
                }

                switch (s->getKind()) {
                case StmtType::Case: {
                    SharedExp jumpDest = jumpStmt->getDest();

                    if (jumpDest == nullptr) {
                        break;
                    }

                    // Check for indirect calls to library functions, especially in Win32 programs
                    if (refersToImportedFunction(jumpDest)) {
                        LOG_VERBOSE("Jump to a library function: %1, replacing with a call/ret.",
                                    jumpStmt);

                        // jump to a library function
                        // replace with a call ret
                        const BinarySymbol
                            *sym = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(
                                jumpDest->access<Const, 1>()->getAddr());
                        assert(sym != nullptr);

                        QString func = sym->getName();
                        std::shared_ptr<CallStatement> call(new CallStatement);
                        call->setDest(jumpDest->clone());
                        LibProc *lp = proc->getProg()->getOrCreateLibraryProc(func);

                        if (lp == nullptr) {
                            LOG_FATAL("getLibraryProc() returned nullptr");
                        }

                        call->setDestProc(lp);

                        std::unique_ptr<RTL> rtl(new RTL(lifted.rtl->getAddress(), { call }));
                        bbRTLs->push_back(std::move(rtl));
                        currentBB->setIR(std::move(bbRTLs));

                        appendSyntheticReturn(currentBB, proc, lifted.rtl.get());

                        if (lifted.rtl->getAddress() == proc->getEntryAddress()) {
                            // it's a thunk
                            // Proc *lp = prog->findProc(func.c_str());
                            func = "__imp_" + func;
                            proc->setName(func);
                            // lp->setName(func.c_str());
                            m_program->getProject()->alertSignatureUpdated(proc);
                        }

                        callList.push_back(call);
                        break;
                    }

                    // We create the BB as a COMPJUMP type, then change to an NWAY if it turns out
                    // to be a switch stmt
                    bbRTLs->push_back(std::move(lifted.rtl));
                    currentBB->setIR(std::move(bbRTLs));
                    LOG_VERBOSE2("COMPUTED JUMP at address %1, jumpDest = %2", insn.m_addr,
                                 jumpDest);
                } break;

                case StmtType::Call: {
                    std::shared_ptr<CallStatement> call = s->as<CallStatement>();

                    // Check for a dynamic linked library function
                    if (refersToImportedFunction(call->getDest())) {
                        // Dynamic linked proc pointers are treated as static.
                        Address linkedAddr = call->getDest()->access<Const, 1>()->getAddr();
                        QString name       = m_program->getBinaryFile()
                                           ->getSymbols()
                                           ->findSymbolByAddress(linkedAddr)
                                           ->getName();

                        Function *function = proc->getProg()->getOrCreateLibraryProc(name);
                        call->setDestProc(function);
                        call->setIsComputed(false);
                    }

                    const Address functionAddr = getAddrOfLibraryThunk(call, proc);
                    if (functionAddr != Address::INVALID) {
                        // Yes, it's a library function. Look up its name.
                        QString name = m_program->getBinaryFile()
                                           ->getSymbols()
                                           ->findSymbolByAddress(functionAddr)
                                           ->getName();

                        // Assign the proc to the call
                        Function *p = proc->getProg()->getOrCreateLibraryProc(name);

                        if (call->getDestProc()) {
                            // prevent unnecessary __imp procs
                            m_program->removeFunction(call->getDestProc()->getName());
                        }

                        call->setDestProc(p);
                        call->setIsComputed(false);
                        call->setDest(Location::memOf(Const::get(functionAddr)));
                    }

                    // Treat computed and static calls separately
                    if (call->isComputed()) {
                        bbRTLs->push_back(std::move(lifted.rtl));
                        currentBB->setIR(std::move(bbRTLs));

                        // Add this call to the list of calls to analyse. We won't
                        // be able to analyse it's callee(s), of course.
                        callList.push_back(call);
                    }
                    else {
                        // Static call
                        const Address callAddr = call->getFixedDest();

                        // Call the virtual helper function. If implemented, will check for
                        // machine specific funcion calls
                        if (isHelperFunc(callAddr, insn.m_addr, *bbRTLs)) {
                            // We have already added to BB_rtls
                            lifted.rtl.reset(); // Discard the call semantics
                            break;
                        }

                        RTL *rtl = lifted.rtl.get();
                        bbRTLs->push_back(std::move(lifted.rtl));

                        // Add this non computed call site to the set of call sites which need
                        // to be analysed later.
                        callList.push_back(call);

                        // Record the called address as the start of a new procedure if it
                        // didn't already exist.
                        if (!callAddr.isZero() && (callAddr != Address::INVALID) &&
                            (proc->getProg()->getFunctionByAddr(callAddr) == nullptr)) {
                            callList.push_back(call);

                            if (m_program->getProject()->getSettings()->traceDecoder) {
                                LOG_MSG("p%1", callAddr);
                            }
                        }

                        // Check if this is the _exit or exit function. May prevent us from
                        // attempting to decode invalid instructions, and getting invalid stack
                        // height errors
                        QString procName = m_program->getSymbolNameByAddr(callAddr);

                        if (procName.isEmpty() && refersToImportedFunction(call->getDest())) {
                            Address a = call->getDest()->access<Const, 1>()->getAddr();
                            procName  = m_program->getBinaryFile()
                                           ->getSymbols()
                                           ->findSymbolByAddress(a)
                                           ->getName();
                        }

                        if (!procName.isEmpty() && isNoReturnCallDest(procName)) {
                            // Make sure it has a return appended (so there is only one exit
                            // from the function)
                            currentBB->setIR(std::move(bbRTLs));
                            appendSyntheticReturn(currentBB, proc, rtl);
                        }
                        else {
                            // Create the new basic block
                            currentBB->setIR(std::move(bbRTLs));

                            if (call->isReturnAfterCall()) {
                                assert(false); // FIXME

                                //                                 // Constuct the RTLs for the new
                                //                                 basic block
                                //                                 std::unique_ptr<RTLList> rtls(new
                                //                                 RTLList);
                                //                                 rtls->push_back(std::unique_ptr<RTL>(
                                //                                     new RTL(rtl->getAddress() +
                                //                                     1,
                                //                                             {
                                //                                             std::make_shared<ReturnStatement>()
                                //                                             })));
                                //
                                //                                 BasicBlock *returnBB =
                                //                                 cfg->createBB(BBType::Ret,
                                //                                 std::move(rtls));
                                //
                                //                                 // Add out edge from call to
                                //                                 return cfg->addEdge(currentBB,
                                //                                 returnBB);

                                // Mike: do we need to set return locations?
                                // This ends the function
                            }
                        }
                    }

                    if (currentBB && currentBB->getIR()->getRTLs()) {
                        extraProcessCall(call, *currentBB->getIR()->getRTLs());
                    }
                } break;

                case StmtType::Ret: {
                    // Create the list of RTLs for the next basic block and
                    // continue with the next instruction.
                    createReturnBlock(proc, std::move(bbRTLs), std::move(lifted.rtl));
                } break;

                case StmtType::Goto:
                case StmtType::Branch:
                case StmtType::Assign:
                case StmtType::BoolAssign:
                    //                 case StmtType::PhiAssign:
                    //                 case StmtType::ImpAssign:
                    break;
                case StmtType::INVALID:
                default: assert(false); break;
                }

                if (lifted.rtl == nullptr) {
                    break;
                }
            }

            if (lifted.rtl != nullptr && bbRTLs != nullptr) {
                // we have yet put the RTL into the list -> do it now
                bbRTLs->push_back(std::move(lifted.rtl));
            }
        }

        if (bbRTLs != nullptr) {
            currentBB->setIR(std::move(bbRTLs));
        }
    }

    for (const std::shared_ptr<CallStatement> &callStmt : callList) {
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

    return true;
}


bool DefaultFrontEnd::decodeInstruction(Address pc, MachineInstruction &insn, DecodeResult &result)
{
    return disassembleInstruction(pc, insn) && liftInstruction(insn, result);
}


bool DefaultFrontEnd::disassembleInstruction(Address pc, MachineInstruction &insn)
{
    BinaryImage *image = m_program->getBinaryFile()->getImage();
    if (!image || (image->getSectionByAddr(pc) == nullptr)) {
        LOG_ERROR("Attempted to disassemble outside any known section at address %1", pc);
        return false;
    }

    const BinarySection *section = image->getSectionByAddr(pc);
    if (section->getHostAddr() == HostAddress::INVALID) {
        LOG_ERROR("Attempted to disassemble instruction in unmapped section '%1' at address %2",
                  section->getName(), pc);
        return false;
    }

    const ptrdiff_t host_native_diff = (section->getHostAddr() - section->getSourceAddr()).value();

    try {
        return m_decoder->disassembleInstruction(pc, host_native_diff, insn);
    }
    catch (std::runtime_error &e) {
        LOG_ERROR("%1", e.what());
        return false;
    }
}


bool DefaultFrontEnd::liftInstruction(const MachineInstruction &insn, DecodeResult &lifted)
{
    const bool ok = m_decoder->liftInstruction(insn, lifted);

    if (!ok) {
        LOG_ERROR("Cannot find instruction template '%1' at address %2, "
                  "treating instruction as NOP",
                  insn.m_templateName, insn.m_addr);

        lifted.iclass = IClass::NOP;
        lifted.reLift = false;
        lifted.rtl    = std::make_unique<RTL>(insn.m_addr);
    }

    return true;
}


std::unique_ptr<RTLList> DefaultFrontEnd::liftBB(const std::list<MachineInstruction> &bbInsns)
{
    std::unique_ptr<RTLList> bbRTLs(new RTLList);
    DecodeResult lifted;

    for (const MachineInstruction &bbInsn : bbInsns) {
        if (liftInstruction(bbInsn, lifted)) {
            bbRTLs->push_back(std::move(lifted.rtl));
            assert(!lifted.reLift); // fix: BSF/BSR etc.
        }
    }

    return bbRTLs;
}


void DefaultFrontEnd::extraProcessCall(const std::shared_ptr<CallStatement> &, const RTLList &)
{
}


std::vector<Address> DefaultFrontEnd::findEntryPoints()
{
    std::vector<Address> entrypoints;
    bool gotMain = false;
    // AssemblyLayer
    Address a = findMainEntryPoint(gotMain);

    // TODO: find exported functions and add them too ?
    if (a != Address::INVALID) {
        entrypoints.push_back(a);
    }
    else {             // try some other tricks
        QString fname; // = m_program->getProject()->getSettings()->getFilename();

        // X11 Module
        if (fname.endsWith("_drv.o")) {
            int seploc = fname.lastIndexOf(QDir::separator());
            QString p  = fname.mid(seploc + 1); // part after the last path separator

            if (p != fname) {
                QString name = p.mid(0, p.length() - 6) + "ModuleData";
                const BinarySymbol
                    *p_sym = m_program->getBinaryFile()->getSymbols()->findSymbolByName(name);

                if (p_sym) {
                    Address tmpaddr = p_sym->getLocation();


                    BinaryImage *image = m_program->getBinaryFile()->getImage();
                    DWord vers = 0, setup = 0, teardown = 0;
                    bool ok = true;
                    ok &= image->readNative4(tmpaddr, vers);
                    ok &= image->readNative4(tmpaddr, setup);
                    ok &= image->readNative4(tmpaddr, teardown);

                    // TODO: find use for vers ?
                    const Address setupAddr    = Address(setup);
                    const Address teardownAddr = Address(teardown);

                    if (ok) {
                        if (!setupAddr.isZero()) {
                            if (createFunctionForEntryPoint(setupAddr, "ModuleSetupProc")) {
                                entrypoints.push_back(setupAddr);
                            }
                        }

                        if (!teardownAddr.isZero()) {
                            if (createFunctionForEntryPoint(teardownAddr, "ModuleTearDownProc")) {
                                entrypoints.push_back(teardownAddr);
                            }
                        }
                    }
                }
            }
        }

        // Linux kernel module
        if (fname.endsWith(".ko")) {
            const BinarySymbol *p_sym = m_program->getBinaryFile()->getSymbols()->findSymbolByName(
                "init_module");

            if (p_sym) {
                entrypoints.push_back(p_sym->getLocation());
            }

            p_sym = m_program->getBinaryFile()->getSymbols()->findSymbolByName("cleanup_module");

            if (p_sym) {
                entrypoints.push_back(p_sym->getLocation());
            }
        }
    }

    return entrypoints;
}


bool DefaultFrontEnd::isNoReturnCallDest(const QString &name) const
{
    // clang-format off
    return
        name == "_exit" ||
        name == "exit" ||
        name == "ExitProcess" ||
        name == "abort" ||
        name == "_assert" ||
        name == "__debugbreak";
    // clang-format on
}


void DefaultFrontEnd::addRefHint(Address addr, const QString &name)
{
    m_refHints[addr] = name;
}


BasicBlock *DefaultFrontEnd::createReturnBlock(UserProc *proc, std::unique_ptr<RTLList> BB_rtls,
                                               std::unique_ptr<RTL> returnRTL)
{
    ProcCFG *cfg = proc->getCFG();

    assert(BB_rtls != nullptr);

    RTL *retRTL = returnRTL.get();
    BB_rtls->push_back(std::move(returnRTL));
    Address retAddr   = proc->getRetAddr();
    BasicBlock *newBB = nullptr;

    if (retAddr == Address::INVALID) {
        // Create the basic block
        //         newBB = cfg->createBB(BBType::Ret, std::move(BB_rtls));
        if (newBB) {
            SharedStmt s = retRTL->back(); // The last statement should be the ReturnStatement
            proc->setRetStmt(s->as<ReturnStatement>(), retRTL->getAddress());
        }
    }
    else {
        // We want to replace the *whole* RTL with a branch to THE first return's RTL. There can
        // sometimes be extra semantics associated with a return (e.g. x86 ret instruction adds to
        // the stack pointer before setting %pc and branching). Other semantics (e.g. SPARC
        // returning a value as part of the restore instruction) are assumed to appear in a
        // previous RTL. It is assumed that THE return statement will have the same semantics
        // (NOTE: may not always be valid). To avoid this assumption, we need branches to
        // statements, not just to native addresses (RTLs).
        BasicBlock *retBB = proc->getCFG()->findRetNode();
        assert(retBB);

        if (retBB->getIR()->getFirstStmt()->isReturn()) {
            // ret node has no semantics, clearly we need to keep ours
            assert(!retRTL->empty());
            retRTL->pop_back();
        }
        else {
            retRTL->clear();
        }

        retRTL->append(std::make_shared<GotoStatement>(retAddr));
        //         newBB = cfg->createBB(BBType::Oneway, std::move(BB_rtls));

        if (newBB) {
            cfg->ensureBBExists(retAddr, retBB);
            cfg->addEdge(newBB, retBB);

            // Visit the return instruction. This will be needed in most cases to split the
            // return BB (if it has other instructions before the return instruction).
            m_targetQueue.pushAddress(cfg, retAddr, newBB);
        }
    }

    // make sure the RTLs have been moved into the BB
    assert(BB_rtls == nullptr);
    return newBB;
}


bool DefaultFrontEnd::isHelperFunc(Address, Address, RTLList &)
{
    return false;
}


bool DefaultFrontEnd::refersToImportedFunction(const SharedExp &exp)
{
    if (exp && exp->isMemOf() && exp->access<Exp, 1>()->isIntConst()) {
        const BinarySymbol *symbol = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(
            exp->access<Const, 1>()->getAddr());

        if (symbol && symbol->isImportedFunction()) {
            return true;
        }
    }

    return false;
}


void DefaultFrontEnd::appendSyntheticReturn(BasicBlock *callBB, UserProc *proc, RTL *callRTL)
{
    std::unique_ptr<RTLList> ret_rtls(new RTLList);
    std::unique_ptr<RTL> retRTL(
        new RTL(callRTL->getAddress(), { std::make_shared<ReturnStatement>() }));
    BasicBlock *retBB = createReturnBlock(proc, std::move(ret_rtls), std::move(retRTL));

    assert(callBB->getNumSuccessors() == 0);
    proc->getCFG()->addEdge(callBB, retBB);
}


void DefaultFrontEnd::preprocessProcGoto(RTL::StmtList::iterator ss, Address dest,
                                         const RTL::StmtList &sl, RTL *originalRTL)
{
    Q_UNUSED(sl);
    assert(sl.back() == *ss);

    if (dest == Address::INVALID) {
        return;
    }

    Function *proc = m_program->getFunctionByAddr(dest);

    if (proc == nullptr) {
        const BinarySymbolTable *symbols = m_program->getBinaryFile()->getSymbols();
        const BinarySymbol *symbol       = symbols->findSymbolByAddress(dest);

        if (symbol && symbol->isImportedFunction()) {
            proc = m_program->getOrCreateFunction(dest);
        }
    }

    if (proc != nullptr && proc != reinterpret_cast<Function *>(-1)) {
        std::shared_ptr<CallStatement> call(new CallStatement);
        call->setDest(dest);
        call->setDestProc(proc);
        call->setReturnAfterCall(true);

        // also need to change it in the actual RTL
        assert(std::next(ss) == sl.end());
        assert(!originalRTL->empty());
        originalRTL->back() = call;
        *ss                 = call;
    }
}


UserProc *DefaultFrontEnd::createFunctionForEntryPoint(Address entryAddr,
                                                       const QString &functionType)
{
    SharedType ty = NamedType::getNamedType(functionType);
    if (!ty || !ty->isFunc()) {
        LOG_WARN("Cannot create function for entry point at address %1", entryAddr);
        return nullptr;
    }

    Function *func = m_program->getOrCreateFunction(entryAddr);
    if (!func || func->isLib()) {
        LOG_WARN("Cannot create function for entry point: "
                 "Address %1 is already the entry address of a library function.",
                 entryAddr);
        return nullptr;
    }

    std::shared_ptr<Signature> sig = ty->as<FuncType>()->getSignature()->clone();
    const BinarySymbol *p_sym      = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(
        entryAddr);
    QString sym = p_sym ? p_sym->getName() : QString("");

    if (!sym.isEmpty()) {
        sig->setName(sym);
    }

    sig->setForced(true);
    func->setSignature(sig);
    return static_cast<UserProc *>(func);
}


Address DefaultFrontEnd::getAddrOfLibraryThunk(const std::shared_ptr<CallStatement> &call,
                                               UserProc *proc)
{
    if (!call || call->getFixedDest() == Address::INVALID) {
        return Address::INVALID;
    }

    Address callAddr         = call->getFixedDest();
    const BinaryImage *image = m_program->getBinaryFile()->getImage();
    if (!Util::inRange(callAddr, image->getLimitTextLow(), image->getLimitTextHigh())) {
        return Address::INVALID;
    }

    MachineInstruction insn;
    DecodeResult lifted;

    if (!decodeInstruction(callAddr, insn, lifted)) {
        return Address::INVALID;
    }

    // Make sure to re-decode the instruction as often as necessary, but throw away the results.
    // Otherwise this will cause problems e.g. with functions beginning with BSF/BSR.
    if (lifted.reLift) {
        DecodeResult dummyLifted;
        do {
            if (!m_decoder->liftInstruction(insn, dummyLifted)) {
                return Address::INVALID;
            }
        } while (dummyLifted.reLift);
    }

    if (lifted.rtl->empty()) {
        return Address::INVALID;
    }

    SharedStmt firstStmt = lifted.rtl->front();
    if (!firstStmt) {
        return Address::INVALID;
    }

    firstStmt->setProc(proc);
    firstStmt->simplify();

    std::shared_ptr<GotoStatement> jmpStmt = std::dynamic_pointer_cast<GotoStatement>(firstStmt);
    if (!jmpStmt || !refersToImportedFunction(jmpStmt->getDest())) {
        return Address::INVALID;
    }

    return jmpStmt->getDest()->access<Const, 1>()->getAddr();
}
