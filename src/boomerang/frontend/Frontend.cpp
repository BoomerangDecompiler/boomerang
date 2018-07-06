#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Frontend.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"
#include "boomerang/c/ansi-c-parser.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/IndirectJumpAnalyzer.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/binary/BinaryFile.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"

#include "boomerang/frontend/sparc/sparcfrontend.h"
#include "boomerang/frontend/pentium/pentiumfrontend.h"
#include "boomerang/frontend/ppc/ppcfrontend.h"
#include "boomerang/frontend/st20/st20frontend.h"
#include "boomerang/frontend/mips/mipsfrontend.h"

#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FuncType.h"

#include <QDir>

#include <cassert>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <cstdarg> // For varargs
#include <sstream>


IFrontEnd::IFrontEnd(BinaryFile *binaryFile, Prog *prog)
    : m_binaryFile(binaryFile)
    , m_program(prog)
    , m_targetQueue(prog->getProject()->getSettings()->traceDecoder)
{
}


IFrontEnd::~IFrontEnd()
{
}


IFrontEnd *IFrontEnd::instantiate(BinaryFile *binaryFile, Prog *prog)
{
    switch (binaryFile->getMachine())
    {
    case Machine::PENTIUM:
        return new PentiumFrontEnd(binaryFile, prog);

    case Machine::SPARC:
        return new SparcFrontEnd(binaryFile, prog);

    case Machine::PPC:
        return new PPCFrontEnd(binaryFile, prog);

    case Machine::MIPS:
        return new MIPSFrontEnd(binaryFile, prog);

    case Machine::ST20:
        return new ST20FrontEnd(binaryFile, prog);

    case Machine::HPRISC:
        LOG_WARN("No frontend for HP RISC");
        break;

    case Machine::PALM:
        LOG_WARN("No frontend for PALM");
        break;

    case Machine::M68K:
        LOG_WARN("No frontend for M68K");
        break;

    default:
        LOG_ERROR("Machine architecture not supported!");
    }

    return nullptr;
}


QString IFrontEnd::getRegName(int idx) const
{
    return m_decoder->getRegName(idx);
}


int IFrontEnd::getRegSize(int idx)
{
    return m_decoder->getRegSize(idx);
}


bool IFrontEnd::isWin32() const
{
    return m_binaryFile && m_binaryFile->getFormat() == LoadFmt::PE;
}


bool IFrontEnd::isNoReturnCallDest(const QString& name)
{
    return
        (name == "_exit") ||
        (name == "exit") ||
        (name == "ExitProcess") ||
        (name == "abort") ||
        (name == "_assert") ||
        (name == "__debugbreak");
}


void IFrontEnd::readLibraryCatalog(const QString& filePath)
{
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    QFile file(filePath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        LOG_ERROR("Cannot open library signature catalog `%1'", filePath);
        return;
    }

    QTextStream inf(&file);
    QString     sig_path;

    while (!inf.atEnd()) {
        QString sigFilePath;
        inf >> sigFilePath;
        sigFilePath = sigFilePath.mid(0, sigFilePath.indexOf('#')); // cut the line to first '#'

        if ((sigFilePath.size() > 0) && sigFilePath.endsWith('\n')) {
            sigFilePath = sigFilePath.mid(0, sigFilePath.size() - 1);
        }

        if (sigFilePath.isEmpty()) {
            continue;
        }

        CallConv cc = CallConv::C; // Most APIs are C calling convention

        if (sigFilePath == "windows.h") {
            cc = CallConv::Pascal; // One exception
        }

        if (sigFilePath == "mfc.h") {
            cc = CallConv::ThisCall; // Another exception
        }

        sig_path = m_program->getProject()->getSettings()->getDataDirectory().absoluteFilePath("signatures/" + sigFilePath);
        readLibrarySignatures(qPrintable(sig_path), cc);
    }
}


void IFrontEnd::readLibraryCatalog()
{
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    m_librarySignatures.clear();
    QDir sig_dir(m_program->getProject()->getSettings()->getDataDirectory());

    if (!sig_dir.cd("signatures")) {
        LOG_WARN("Signatures directory does not exist.");
        return;
    }

    readLibraryCatalog(sig_dir.absoluteFilePath("common.hs"));
    readLibraryCatalog(sig_dir.absoluteFilePath(Util::getPlatformName(getType()) + ".hs"));

    if (isWin32()) {
        readLibraryCatalog(sig_dir.absoluteFilePath("win32.hs"));
    }

    // TODO: change this to BinaryLayer query ("FILE_FORMAT","MACHO")
    if (m_binaryFile->getFormat() == LoadFmt::MACHO) {
        readLibraryCatalog(sig_dir.absoluteFilePath("objc.hs"));
    }
}


void IFrontEnd::checkEntryPoint(std::vector<Address>& entrypoints, Address addr, const char *type)
{
    SharedType ty = NamedType::getNamedType(type);

    assert(ty->isFunc());
    UserProc *proc = static_cast<UserProc *>(m_program->getOrCreateFunction(addr));
    assert(proc);

    auto                sig    = ty->as<FuncType>()->getSignature()->clone();
    const BinarySymbol *p_sym = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(addr);
    QString             sym    = p_sym ? p_sym->getName() : QString("");

    if (!sym.isEmpty()) {
        sig->setName(sym);
    }

    sig->setForced(true);
    proc->setSignature(sig);
    entrypoints.push_back(addr);
}


std::vector<Address> IFrontEnd::getEntryPoints()
{
    std::vector<Address> entrypoints;
    bool                 gotMain = false;
    // AssemblyLayer
    Address a = getMainEntryPoint(gotMain);

    // TODO: find exported functions and add them too ?
    if (a != Address::INVALID) {
        entrypoints.push_back(a);
    }
    else { // try some other tricks
        QString fname = QString::null; // m_program->getProject()->getSettings()->getFilename();

        // X11 Module
        if (fname.endsWith("_drv.o")) {
            int     seploc = fname.lastIndexOf(QDir::separator());
            QString p      = fname.mid(seploc + 1); // part after the last path separator

            if (p != fname) {
                QString             name   = p.mid(0, p.length() - 6) + "ModuleData";
                const BinarySymbol *p_sym = m_program->getBinaryFile()->getSymbols()->findSymbolByName(name);

                if (p_sym) {
                    Address tmpaddr = p_sym->getLocation();
                    Address setup, teardown;

                    BinaryImage *image = m_program->getBinaryFile()->getImage();
                    /*uint32_t vers = */ image->readNative4(tmpaddr); // TODO: find use for vers ?
                    setup    = Address(image->readNative4(tmpaddr + 4));
                    teardown = Address(image->readNative4(tmpaddr + 8));

                    if (!setup.isZero()) {
                        checkEntryPoint(entrypoints, setup, "ModuleSetupProc");
                    }

                    if (!teardown.isZero()) {
                        checkEntryPoint(entrypoints, teardown, "ModuleTearDownProc");
                    }
                }
            }
        }

        // Linux kernel module
        if (fname.endsWith(".ko")) {
            const BinarySymbol *p_sym = m_program->getBinaryFile()->getSymbols()->findSymbolByName("init_module");

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


bool IFrontEnd::decodeEntryPointsRecursive(bool decodeMain)
{
    if (!decodeMain) {
        return true;
    }

    BinaryImage *image = m_program->getBinaryFile()->getImage();

    Interval<Address> extent(image->getLimitTextLow(), image->getLimitTextHigh());

    m_program->getProject()->alertStartDecode(extent.lower(), (extent.upper() - extent.lower()).value());

    bool    gotMain;
    Address a = getMainEntryPoint(gotMain);
    LOG_VERBOSE("start: %1, gotMain: %2", a, (gotMain ? "true" : "false"));

    if (a == Address::INVALID) {
        std::vector<Address> entrypoints = getEntryPoints();

        for (auto& entrypoint : entrypoints) {
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
    QString           name        = m_program->getSymbolNameByAddr(a);

    if (name == nullptr) {
        name = mainName[0];
    }

    for (auto& elem : mainName) {
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
            proc->getSignature()->setForced(true);     // Don't add or remove parameters
        }

        break;
    }

    return true;
}


bool IFrontEnd::decodeRecursive(Address addr)
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

    QTextStream os(stderr); // rtl output target

    if (processProc(addr, proc, os)) {
        proc->setDecoded();
    }

    return m_program->isWellFormed();
}


bool IFrontEnd::decodeUndecoded()
{
    bool change = true;
    LOG_MSG("Looking for undecoded procedures to decode...");

    while (change) {
        change = false;

        for (const auto& m : m_program->getModuleList()) {
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
                QTextStream os(stderr); // rtl output target

                if (!processProc(userProc->getEntryAddress(), userProc, os)) {
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


bool IFrontEnd::decodeOnly(Address addr)
{
    UserProc *p = static_cast<UserProc *>(m_program->getOrCreateFunction(addr));
    assert(!p->isLib());
    QTextStream os(stderr); // rtl output target

    bool ok = processProc(p->getEntryAddress(), p, os);
    if (ok) {
        p->setDecoded();
    }

    return ok && m_program->isWellFormed();
}


bool IFrontEnd::decodeFragment(UserProc *proc, Address a)
{
    if (m_program->getProject()->getSettings()->traceDecoder) {
        LOG_MSG("Decoding fragment at address %1", a);
    }

    QTextStream os(stderr); // rtl output target
    return processProc(a, proc, os, true);
}


bool IFrontEnd::decodeInstruction(Address pc, DecodeResult& result)
{
    BinaryImage *image = m_program->getBinaryFile()->getImage();
    if (!image || (image->getSectionByAddr(pc) == nullptr)) {
        LOG_ERROR("Attempted to decode outside any known section at address %1", pc);
        result.valid = false;
        return false;
    }

    const BinarySection *section = image->getSectionByAddr(pc);
    if (section->getHostAddr() == HostAddress::INVALID) {
        LOG_ERROR("Attempted to decode instruction in unmapped section '%1' at address %2",
            section->getName(), pc);
        return false;
    }

    ptrdiff_t host_native_diff = (section->getHostAddr() - section->getSourceAddr()).value();

    try {
        return m_decoder->decodeInstruction(pc, host_native_diff, result);
    }
    catch (std::invalid_argument& e) {
        LOG_ERROR("%1", e.what());
        return false;
    }
}


void IFrontEnd::readLibrarySignatures(const char *signatureFile, CallConv cc)
{
    std::unique_ptr<AnsiCParser> p;

    try {
        p.reset(new AnsiCParser(signatureFile, false));
    }
    catch (const char *err) {
        LOG_ERROR("Cannot read library signature file '%1': %2", signatureFile, err);
        return;
    }

    Platform plat = getType();
    p->yyparse(plat, cc);

    for (auto& signature : p->signatures) {
        m_librarySignatures[signature->getName()] = signature;
        signature->setSigFilePath(signatureFile);
    }
}


std::shared_ptr<Signature> IFrontEnd::getDefaultSignature(const QString& name)
{
    // Get a default library signature
    if (isWin32()) {
        return Signature::instantiate(Platform::PENTIUM, CallConv::Pascal, name);
    }

    return Signature::instantiate(getType(), CallConv::C, name);
}


std::shared_ptr<Signature> IFrontEnd::getLibSignature(const QString& name)
{
    std::shared_ptr<Signature> signature;
    // Look up the name in the librarySignatures map
    auto it = m_librarySignatures.find(name);

    if (it == m_librarySignatures.end()) {
        LOG_WARN("Unknown library function '%1', please update signatures!", name);
        signature = getDefaultSignature(name);
    }
    else {
        // Don't clone here; cloned in CallStatement::setSigArguments
        signature = *it;
        signature->setUnknown(false);
    }

    return signature;
}


void IFrontEnd::preprocessProcGoto(std::list<Statement *>::iterator ss,
                                   Address dest, const std::list<Statement *>& sl,
                                   RTL *originalRTL)
{
    Q_UNUSED(sl);
    assert(sl.back() == *ss);

    if (dest == Address::INVALID) {
        return;
    }

    Function *proc = m_program->getFunctionByAddr(dest);

    if (proc == nullptr) {
        auto symb = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(dest);

        if (symb && symb->isImportedFunction()) {
            proc = m_program->getOrCreateFunction(dest);
        }
    }

    if ((proc != nullptr) && (proc != reinterpret_cast<Function *>(-1))) {
        CallStatement *call = new CallStatement();
        call->setDest(dest);
        call->setDestProc(proc);
        call->setReturnAfterCall(true);
        // also need to change it in the actual RTL
        assert(std::next(ss) == sl.end());
        assert(!originalRTL->empty());
        originalRTL->back() = call;
        *ss          = call;
    }
}


bool IFrontEnd::refersToImportedFunction(const SharedExp& exp)
{
    if (exp && (exp->getOper() == opMemOf) && (exp->access<Exp, 1>()->getOper() == opIntConst)) {
        const BinarySymbol *symbol = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(exp->access<Const, 1>()->getAddr());

        if (symbol && symbol->isImportedFunction()) {
            return true;
        }
    }

    return false;
}


bool IFrontEnd::processProc(Address addr, UserProc *proc, QTextStream& /*os*/,
                            bool /*frag*/ /* = false */, bool spec /* = false */)
{
    BasicBlock *currentBB;

    LOG_VERBOSE("### Decoding proc '%1' at address %2 ###", proc->getName(), addr);

    // We have a set of CallStatement pointers. These may be disregarded if this is a speculative decode
    // that fails (i.e. an illegal instruction is found). If not, this set will be used to add to the set of calls
    // to be analysed in the cfg, and also to call newProc()
    std::list<CallStatement *> callList;

    // Indicates whether or not the next instruction to be decoded is the lexical successor of the current one.
    // Will be true for all NCTs and for CTIs with a fall through branch.
    bool sequentialDecode = true;

    Cfg *cfg = proc->getCFG();

    // If this is a speculative decode, the second time we decode the same address, we get no cfg. Else an error.
    if (spec && (cfg == nullptr)) {
        return false;
    }

    assert(cfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    m_targetQueue.initial(addr);

    // Clear the pointer used by the caller prologue code to access the last call rtl of this procedure
    // decoder.resetLastCall();

    int numBytesDecoded = 0;
    Address startAddr   = addr;
    Address lastAddr    = addr;

    while ((addr = m_targetQueue.getNextAddress(*cfg)) != Address::INVALID) {
        // The list of RTLs for the current basic block
        std::unique_ptr<RTLList> BB_rtls(new RTLList);

        // Keep decoding sequentially until a CTI without a fall through branch is decoded
        DecodeResult inst;

        while (sequentialDecode) {
            // Decode and classify the current source instruction
            if (m_program->getProject()->getSettings()->traceDecoder) {
                LOG_MSG("*%1", addr);
            }

            if (!decodeInstruction(addr, inst)) {
                QString message;
                BinaryImage *image = m_program->getBinaryFile()->getImage();
                message.sprintf("Encountered invalid or unrecognized instruction at address %s: 0x%02X 0x%02X 0x%02X 0x%02X", qPrintable(addr.toString()),
                                image->readNative1(addr + 0),
                                image->readNative1(addr + 1),
                                image->readNative1(addr + 2),
                                image->readNative1(addr + 3));
                LOG_WARN(message);
                break;
            }
            else if (inst.rtl->empty()) {
                LOG_VERBOSE("Instruction at address %1 is a no-op!", addr);
            }

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid) {
                inst.rtl.reset();
                return false;
            }

            // Need to construct a new list of RTLs if a basic block has just been finished but decoding is
            // continuing from its lexical successor
            if (BB_rtls == nullptr) {
                BB_rtls.reset(new std::list<std::unique_ptr<RTL>>());
            }

            if (!inst.valid) {
                // Alert the watchers to the problem
                m_program->getProject()->alertBadDecode(addr);

                // An invalid instruction. Most likely because a call did not return (e.g. call _exit()), etc.
                // Best thing is to emit an INVALID BB, and continue with valid instructions
                // Emit the RTL anyway, so we have the address and maybe some other clues
                BB_rtls->push_back(Util::makeUnique<RTL>(addr));
                currentBB = cfg->createBB(BBType::Invalid, std::move(BB_rtls));
                sequentialDecode = false;
                BB_rtls          = nullptr;
                break; // try the next instruction in the queue
            }

            // alert the watchers that we have decoded an instruction
            m_program->getProject()->alertInstructionDecoded(addr, inst.numBytes);
            numBytesDecoded += inst.numBytes;

            // Check if this is an already decoded jump instruction (from a previous pass with propagation etc)
            // If so, we throw away the just decoded RTL (but we still may have needed to calculate the number
            // of bytes.. ick.)
            std::map<Address, RTL *>::iterator ff = m_previouslyDecoded.find(addr);

            if (ff != m_previouslyDecoded.end()) {
                inst.rtl.reset(ff->second);
            }

            if (!inst.rtl) {
                // This can happen if an instruction is "cancelled", e.g. call to __main in a hppa program
                // Just ignore the whole instruction
                if (inst.numBytes > 0) {
                    addr += inst.numBytes;
                }

                continue;
            }

            // Display RTL representation if asked
            if (m_program->getProject()->getSettings()->printRTLs) {
                QString     tgt;
                QTextStream st(&tgt);
                inst.rtl->print(st);
                LOG_MSG(tgt);
            }

            // Make a copy (!) of the list. This is needed temporarily to work around the following problem.
            // We are currently iterating an RTL, which could be a return instruction. The RTL is passed to
            // createReturnBlock; if this is not the first return statement, it will get cleared, and this will
            // cause problems with the current iteration. The effects seem to be worse for MSVC/Windows.
            // This problem will likely be easier to cope with when the RTLs are removed, and there are special
            // Statements to mark the start of instructions (and their native address).
            // FIXME: However, this workaround breaks logic below where a GOTO is changed to a CALL followed by a return
            // if it points to the start of a known procedure
            std::list<Statement *> sl(inst.rtl->getStatements());

            for (auto ss = sl.begin(); ss != sl.end(); ++ss) {
                Statement *s = *ss;
                s->setProc(proc); // let's do this really early!

                if (m_refHints.find(inst.rtl->getAddress()) != m_refHints.end()) {
                    const QString& name(m_refHints[inst.rtl->getAddress()]);
                    Address        globAddr = m_program->getGlobalAddrByName(name);

                    if (globAddr != Address::INVALID) {
                        s->searchAndReplace(Const(globAddr), Unary::get(opAddrOf, Location::global(name, proc)));
                    }
                }

                s->simplify();
                GotoStatement *jumpStmt = dynamic_cast<GotoStatement *>(s);

                // Check for a call to an already existing procedure (including self recursive jumps), or to the PLT
                // (note that a LibProc entry for the PLT function may not yet exist)
                if (s->getKind() == StmtType::Goto) {
                    preprocessProcGoto(ss, jumpStmt->getFixedDest(), sl, inst.rtl.get());
                    s = *ss; // *ss can be changed within processProc
                }

                switch (s->getKind())
                {
                case StmtType::Goto:
                    {
                        Address jumpDest = jumpStmt->getFixedDest();

                        // Handle one way jumps and computed jumps separately
                        if (jumpDest != Address::INVALID) {
                            BB_rtls->push_back(std::move(inst.rtl));
                            sequentialDecode = false;

                            currentBB = cfg->createBB(BBType::Oneway, std::move(BB_rtls));

                            // Exit the switch now if the basic block already existed
                            if (currentBB == nullptr) {
                                break;
                            }

                            // Add the out edge if it is to a destination within the
                            // procedure
                            if (jumpDest < m_program->getBinaryFile()->getImage()->getLimitTextHigh()) {
                                m_targetQueue.visit(cfg, jumpDest, currentBB);
                                cfg->addEdge(currentBB, jumpDest);
                            }
                            else {
                                LOG_WARN("Goto instruction at address %1 branches beyond end of section, to %2", addr, jumpDest);
                            }
                        }
                    }
                    break;

                case StmtType::Case:
                    {
                        SharedExp jumpDest = jumpStmt->getDest();

                        if (jumpDest == nullptr) {                      // Happens if already analysed (now redecoding)
                            BB_rtls->push_back(std::move(inst.rtl));
                            currentBB = cfg->createBB(BBType::Nway, std::move(BB_rtls)); // processSwitch will update num outedges
                            BB_rtls          = nullptr;              // New RTLList for next BB
                            IndirectJumpAnalyzer().processSwitch(currentBB, proc); // decode arms, set out edges, etc
                            sequentialDecode = false;                // Don't decode after the jump
                            break;                                   // Just leave it alone
                        }

                        // Check for indirect calls to library functions, especially in Win32 programs
                        if (refersToImportedFunction(jumpDest)) {
                            LOG_VERBOSE("Jump to a library function: %1, replacing with a call/ret.", jumpStmt);

                            // jump to a library function
                            // replace with a call ret
                            const BinarySymbol *sym = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(jumpDest->access<Const, 1>()->getAddr());
                            assert(sym != nullptr);
                            QString       func  = sym->getName();
                            CallStatement *call = new CallStatement;
                            call->setDest(jumpDest->clone());
                            LibProc *lp = proc->getProg()->getOrCreateLibraryProc(func);

                            if (lp == nullptr) {
                                LOG_FATAL("getLibraryProc() returned nullptr");
                            }

                            call->setDestProc(lp);
                            BB_rtls->push_back(std::unique_ptr<RTL>(new RTL(inst.rtl->getAddress(), { call })));

                            currentBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                            appendSyntheticReturn(currentBB, proc, inst.rtl.get());
                            sequentialDecode = false;

                            if (inst.rtl->getAddress() == proc->getEntryAddress()) {
                                // it's a thunk
                                // Proc *lp = prog->findProc(func.c_str());
                                func = "__imp_" + func;
                                proc->setName(func);
                                // lp->setName(func.c_str());
                                m_program->getProject()->alertSignatureUpdated(proc);
                            }

                            callList.push_back(call);
                            ss = sl.end();
                            ss--; // get out of the loop
                            break;
                        }

                        BB_rtls->push_back(std::move(inst.rtl));

                        // We create the BB as a COMPJUMP type, then change to an NWAY if it turns out to be a switch stmt
                        currentBB = cfg->createBB(BBType::CompJump, std::move(BB_rtls));

                        LOG_VERBOSE2("COMPUTED JUMP at address %1, jumpDest = %2", addr, jumpDest);

                        sequentialDecode = false;
                        break;
                    }

                case StmtType::Branch:
                    {
                        Address jumpDest = jumpStmt->getFixedDest();
                        BB_rtls->push_back(std::move(inst.rtl));

                        currentBB = cfg->createBB(BBType::Twoway, std::move(BB_rtls));
                        // Create the list of RTLs for the next basic block and continue with the next instruction.
                        BB_rtls = nullptr;

                        // Stop decoding sequentially if the basic block already existed otherwise complete the basic block
                        if (currentBB == nullptr) {
                            sequentialDecode = false;
                        }
                        else {
                            // Add the out edge if it is to a destination within the section
                            if (jumpDest < m_program->getBinaryFile()->getImage()->getLimitTextHigh()) {
                                m_targetQueue.visit(cfg, jumpDest, currentBB);
                                cfg->addEdge(currentBB, jumpDest);
                            }
                            else {
                                LOG_WARN("Branch instruction at address %1 branches beyond end of section, to %2", addr, jumpDest);
                                cfg->addEdge(currentBB, Address::INVALID);
                            }

                            // Add the fall-through outedge
                            cfg->addEdge(currentBB, addr + inst.numBytes);
                        }
                    }
                    break;

                case StmtType::Call:
                    {
                        CallStatement *call = static_cast<CallStatement *>(s);

                        // Check for a dynamic linked library function
                        if (refersToImportedFunction(call->getDest())) {
                            // Dynamic linked proc pointers are treated as static.
                            Address  linkedAddr = call->getDest()->access<Const, 1>()->getAddr();
                            QString  name       = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(linkedAddr)->getName();
                            Function *function  = proc->getProg()->getOrCreateLibraryProc(name);
                            call->setDestProc(function);
                            call->setIsComputed(false);

                            if (function->isNoReturn() || IFrontEnd::isNoReturnCallDest(function->getName())) {
                                sequentialDecode = false;
                            }
                        }

                        // Is the called function a thunk calling a library function?
                        // A "thunk" is a function which only consists of: "GOTO library_function"
                        if (call && (call->getFixedDest() != Address::INVALID)) {
                            // Get the address of the called function.
                            Address callAddr = call->getFixedDest();

                            // It should not be in the PLT either, but getLimitTextHigh() takes this into account
                            if (Util::inRange(callAddr, m_program->getBinaryFile()->getImage()->getLimitTextLow(), m_program->getBinaryFile()->getImage()->getLimitTextHigh())) {
                                DecodeResult decoded;

                                // Decode it.
                                if (decodeInstruction(callAddr, decoded) && !decoded.rtl->empty()) {
                                    // Decoded successfully. Create a Statement from it.
                                    Statement *firstStmt = decoded.rtl->front();

                                    if (firstStmt) {
                                        firstStmt->setProc(proc);
                                        firstStmt->simplify();

                                        GotoStatement *jmpStatement = dynamic_cast<GotoStatement *>(firstStmt);

                                        // This is a direct jump (x86 opcode FF 25)
                                        // The imported function is at the jump destination.
                                        if (jmpStatement && refersToImportedFunction(jmpStatement->getDest())) {
                                            // Yes, it's a library function. Look up it's name.
                                            Address functionAddr = jmpStatement->getDest()->access<Const, 1>()->getAddr();

                                            QString name = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(functionAddr)->getName();
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
                                    }
                                }

                                decoded.rtl.reset();
                            }
                        }

                        // Treat computed and static calls separately
                        if (call->isComputed()) {
                            BB_rtls->push_back(std::move(inst.rtl));
                            currentBB = cfg->createBB(BBType::CompCall, std::move(BB_rtls));

                            // Stop decoding sequentially if the basic block already
                            // existed otherwise complete the basic block
                            if (currentBB == nullptr) {
                                sequentialDecode = false;
                            }
                            else {
                                cfg->addEdge(currentBB, addr + inst.numBytes);
                            }

                            // Add this call to the list of calls to analyse. We won't
                            // be able to analyse it's callee(s), of course.
                            callList.push_back(call);
                        }
                        else { // Static call
                               // Find the address of the callee.
                            Address callAddr = call->getFixedDest();

                            // Calls with 0 offset (i.e. call the next instruction) are simply pushing the PC to the
                            // stack. Treat these as non-control flow instructions and continue.
                            if (callAddr == addr + inst.numBytes) {
                                break;
                            }

                            // Call the virtual helper function. If implemented, will check for machine specific funcion
                            // calls
                            if (isHelperFunc(callAddr, addr, *BB_rtls)) {
                                // We have already added to BB_rtls
                                inst.rtl.reset(); // Discard the call semantics
                                break;
                            }

                            RTL *rtl = inst.rtl.get();
                            BB_rtls->push_back(std::move(inst.rtl));

                            // Add this non computed call site to the set of call sites which need to be analysed later.
                            callList.push_back(call);

                            // Record the called address as the start of a new procedure if it didn't already exist.
                            if (!callAddr.isZero() && (callAddr != Address::INVALID) &&
                                (proc->getProg()->getFunctionByAddr(callAddr) == nullptr)) {
                                callList.push_back(call);

                                if (m_program->getProject()->getSettings()->traceDecoder) {
                                    LOG_MSG("p%1", callAddr);
                                }
                            }

                            // Check if this is the _exit or exit function. May prevent us from attempting to decode
                            // invalid instructions, and getting invalid stack height errors
                            QString name = m_program->getSymbolNameByAddr(callAddr);

                            if (name.isEmpty() && refersToImportedFunction(call->getDest())) {
                                Address a = call->getDest()->access<Const, 1>()->getAddr();
                                name = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(a)->getName();
                            }

                            if (!name.isEmpty() && IFrontEnd::isNoReturnCallDest(name)) {
                                // Make sure it has a return appended (so there is only one exit from the function)
                                // call->setReturnAfterCall(true);        // I think only the Sparc frontend cares
                                // Create the new basic block
                                currentBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                                appendSyntheticReturn(currentBB, proc, rtl);

                                // Stop decoding sequentially
                                sequentialDecode = false;
                            }
                            else {
                                // Create the new basic block
                                currentBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                                BB_rtls = nullptr;

                                if (call->isReturnAfterCall()) {
                                    // Constuct the RTLs for the new basic block
                                    std::unique_ptr<RTLList> rtls(new RTLList);
                                    rtls->push_back(std::unique_ptr<RTL>(new RTL(rtl->getAddress() + 1, { new ReturnStatement() })));
                                    BasicBlock *returnBB = cfg->createBB(BBType::Ret, std::move(rtls));

                                    // Add out edge from call to return
                                    cfg->addEdge(currentBB, returnBB);

                                    // Mike: do we need to set return locations?
                                    // This ends the function
                                    sequentialDecode = false;
                                }
                                else {
                                    // Add the fall through edge if the block didn't
                                    // already exist
                                    if (currentBB != nullptr) {
                                        cfg->addEdge(currentBB, addr + inst.numBytes);
                                    }
                                }
                            }
                        }

                        if (currentBB && currentBB->getRTLs()) {
                            extraProcessCall(call, *currentBB->getRTLs());
                        }

                        // make sure we already moved the created RTL into a BB
                        assert(BB_rtls == nullptr);
                        break;
                    }

                case StmtType::Ret:
                    // Stop decoding sequentially
                    sequentialDecode = false;

                    // Create the list of RTLs for the next basic block and
                    // continue with the next instruction.
                    currentBB = createReturnBlock(proc, std::move(BB_rtls), std::move(inst.rtl));
                    break;

                case StmtType::BoolAssign:
                // This is just an ordinary instruction; no control transfer
                // Fall through
                // FIXME: Do we need to do anything here?
                case StmtType::Assign:
                case StmtType::PhiAssign:
                case StmtType::ImpAssign:
                case StmtType::ImpRef:
                    // Do nothing
                    break;
                case StmtType::INVALID:
                    assert(false);
                    break;
                }
            }

            if (BB_rtls && inst.rtl) {
                // If non null, we haven't put this RTL into a the current BB as yet
                BB_rtls->push_back(std::move(inst.rtl));
            }

            if (inst.reDecode) {
                // Special case: redecode the last instruction, without advancing addr by numBytes
                continue;
            }

            addr += inst.numBytes;

            if (addr > lastAddr) {
                lastAddr = addr;
            }

            // If sequentially decoding, check if the next address happens to be the start of an existing BB. If so,
            // finish off the current BB (if any RTLs) as a fallthrough, and no need to decode again (unless it's an
            // incomplete BB, then we do decode it).
            // In fact, mustn't decode twice, because it will muck up the coverage, but also will cause subtle problems
            // like add a call to the list of calls to be processed, then delete the call RTL (e.g. Pentium 134.perl
            // benchmark)
            if (sequentialDecode && cfg->isStartOfBB(addr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    BasicBlock *bb = cfg->createBB(BBType::Fall, std::move(BB_rtls));
                    BB_rtls = nullptr; // Need new list of RTLs

                    // Add an out edge to this address
                    if (bb) {
                        cfg->addEdge(bb, addr);
                    }
                }

                // Pick a new address to decode from, if the BB is complete
                if (!cfg->isStartOfIncompleteBB(addr)) {
                    sequentialDecode = false;
                }
            }
        } // while sequentialDecode

        sequentialDecode = true;
    } // while getNextAddress() != Address::INVALID


    for (CallStatement *callStmt : callList) {
        Address dest = callStmt->getFixedDest();
        auto    symb = m_program->getBinaryFile()->getSymbols()->findSymbolByAddress(dest);

        // Don't speculatively decode procs that are outside of the main text section, apart from dynamically
        // linked ones (in the .plt)
        if ((symb && symb->isImportedFunction()) || !spec || (dest < m_program->getBinaryFile()->getImage()->getLimitTextHigh())) {
            // Don't visit the destination of a register call
            Function *np = callStmt->getDestProc();

            if ((np == nullptr) && (dest != Address::INVALID)) {
                // np = newProc(proc->getProg(), dest);
                np = proc->getProg()->getOrCreateFunction(dest);
            }

            if (np != nullptr) {
                np->setFirstCaller(proc);
                proc->addCallee(np);
            }
        }
    }

    m_program->getProject()->alertFunctionDecoded(proc, startAddr, lastAddr, numBytesDecoded);

    LOG_VERBOSE("### Finished decoding proc '%1' ###", proc->getName());

    return true;
}


BasicBlock *IFrontEnd::createReturnBlock(UserProc *proc, std::unique_ptr<RTLList> BB_rtls, std::unique_ptr<RTL> returnRTL)
{
    Cfg *cfg = proc->getCFG();

    // Add the RTL to the list; this has the semantics for the return instruction as well as the ReturnStatement
    // The last Statement may get replaced with a GotoStatement
    if (BB_rtls == nullptr) {
        BB_rtls.reset(new RTLList); // In case no other semantics
    }

    RTL *retRTL = returnRTL.get();
    BB_rtls->push_back(std::move(returnRTL));
    Address retAddr = proc->getTheReturnAddr();
    BasicBlock *newBB = nullptr;

    if (retAddr == Address::INVALID) {
        // Create the basic block
        newBB = cfg->createBB(BBType::Ret, std::move(BB_rtls));
        Statement *s = retRTL->back(); // The last statement should be the ReturnStatement
        proc->setTheReturnAddr(static_cast<ReturnStatement *>(s), retRTL->getAddress());
    }
    else {
        // We want to replace the *whole* RTL with a branch to THE first return's RTL. There can sometimes be extra
        // semantics associated with a return (e.g. Pentium return adds to the stack pointer before setting %pc and
        // branching). Other semantics (e.g. SPARC returning a value as part of the restore instruction) are assumed to
        // appear in a previous RTL. It is assumed that THE return statement will have the same semantics (NOTE: may
        // not always be valid). To avoid this assumption, we need branches to statements, not just to native addresses
        // (RTLs).
        BasicBlock *retBB = proc->getCFG()->findRetNode();
        assert(retBB);

        if (retBB->getFirstStmt()->isReturn()) {
            // ret node has no semantics, clearly we need to keep ours
            assert(!retRTL->empty());
            retRTL->pop_back();
        }
        else {
            retRTL->clear();
        }

        retRTL->append(new GotoStatement(retAddr));
        newBB = cfg->createBB(BBType::Oneway, std::move(BB_rtls));

        if (newBB) {
            cfg->addEdge(newBB, retBB);

            // Visit the return instruction. This will be needed in most cases to split the return BB (if it has other
            // instructions before the return instruction).
            m_targetQueue.visit(cfg, retAddr, newBB);
        }
    }

    // make sure the RTLs have been moved into the BB
    assert(BB_rtls == nullptr);
    return newBB;
}


void IFrontEnd::appendSyntheticReturn(BasicBlock *callBB, UserProc *proc, RTL *callRTL)
{
    std::unique_ptr<RTLList> ret_rtls(new RTLList);
    std::unique_ptr<RTL> retRTL(new RTL(callRTL->getAddress() + 1, { new ReturnStatement }));
    BasicBlock *retBB = createReturnBlock(proc, std::move(ret_rtls), std::move(retRTL));

    assert(callBB->getNumSuccessors() == 0);
    proc->getCFG()->addEdge(callBB, retBB);
}
