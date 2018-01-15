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
#include "boomerang/c/ansi-c-parser.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/IndirectJumpAnalyzer.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/SymTab.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/loader/IFileLoader.h"
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


IFrontEnd::IFrontEnd(IFileLoader *p_BF, Prog *prog)
    : m_fileLoader(p_BF)
    , m_program(prog)
{
    m_image = Boomerang::get()->getImage();
    assert(m_image);
    m_binarySymbols = (SymTab *)Boomerang::get()->getSymbols();
}


IFrontEnd *IFrontEnd::instantiate(IFileLoader *pBF, Prog *prog)
{
    switch (pBF->getMachine())
    {
    case Machine::PENTIUM:
        return new PentiumFrontEnd(pBF, prog);

    case Machine::SPARC:
        return new SparcFrontEnd(pBF, prog);

    case Machine::PPC:
        return new PPCFrontEnd(pBF, prog);

    case Machine::MIPS:
        return new MIPSFrontEnd(pBF, prog);

    case Machine::ST20:
        return new ST20FrontEnd(pBF, prog);

    case Machine::HPRISC:
        LOG_VERBOSE("No frontend for HP RISC");
        break;

    case Machine::PALM:
        LOG_VERBOSE("No frontend for PALM");
        break;

    case Machine::M68K:
        LOG_VERBOSE("No frontend for M68K");
        break;

    default:
        LOG_ERROR("Machine architecture not supported!");
    }

    return nullptr;
}


IFrontEnd *IFrontEnd::create(const QString& fname, Prog *prog, IProject *project)
{
    IFileLoader *loader = project->getBestLoader(fname);

    if (loader == nullptr) {
        return nullptr;
    }

    project->loadBinaryFile(fname);

    return instantiate(loader, prog);
}


void IFrontEnd::addSymbol(Address addr, const QString& nam)
{
    m_binarySymbols->create(addr, nam);
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
    return m_fileLoader && m_fileLoader->getFormat() == LoadFmt::PE;
}


bool IFrontEnd::isNoReturnCallDest(const QString& name)
{
    if ((name == "_exit") ||
        (name == "exit") ||
        (name == "ExitProcess") ||
        (name == "abort") ||
        (name == "_assert")) {
        return true;
    }

    return false;
}


void IFrontEnd::readLibraryCatalog(const QString& sPath)
{
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    QFile file(sPath);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        LOG_ERROR("Cannot open library signature catalog `%1'", sPath);
        return;
    }

    QTextStream inf(&file);
    QString     sig_path;

    while (!inf.atEnd()) {
        QString sFile;
        inf >> sFile;
        sFile = sFile.mid(0, sFile.indexOf('#')); // cut the line to first '#'

        if ((sFile.size() > 0) && sFile.endsWith('\n')) {
            sFile = sFile.mid(0, sFile.size() - 1);
        }

        if (sFile.isEmpty()) {
            continue;
        }

        CallConv cc = CallConv::C; // Most APIs are C calling convention

        if (sFile == "windows.h") {
            cc = CallConv::Pascal; // One exception
        }

        if (sFile == "mfc.h") {
            cc = CallConv::ThisCall; // Another exception
        }

        sig_path = Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("signatures/" + sFile);
        readLibrarySignatures(qPrintable(sig_path), cc);
    }
}


void IFrontEnd::readLibraryCatalog()
{
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    m_librarySignatures.clear();
    QDir sig_dir(Boomerang::get()->getSettings()->getDataDirectory());

    if (!sig_dir.cd("signatures")) {
        LOG_WARN("Signatures directory does not exist.");
        return;
    }

    readLibraryCatalog(sig_dir.absoluteFilePath("common.hs"));
    readLibraryCatalog(sig_dir.absoluteFilePath(Signature::getPlatformName(getType()) + ".hs"));

    if (isWin32()) {
        readLibraryCatalog(sig_dir.absoluteFilePath("win32.hs"));
    }

    // TODO: change this to BinaryLayer query ("FILE_FORMAT","MACHO")
    if (m_fileLoader->getFormat() == LoadFmt::MACHO) {
        readLibraryCatalog(sig_dir.absoluteFilePath("objc.hs"));
    }
}


void IFrontEnd::checkEntryPoint(std::vector<Address>& entrypoints, Address addr, const char *type)
{
    SharedType ty = NamedType::getNamedType(type);

    assert(ty->isFunc());
    UserProc *proc = (UserProc *)m_program->createFunction(addr);
    assert(proc);
    auto                sig    = ty->as<FuncType>()->getSignature()->clone();
    const IBinarySymbol *p_sym = m_binarySymbols->find(addr);
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
        QString fname = QString::null; // Boomerang::get()->getSettings()->getFilename();

        // X11 Module
        if (fname.endsWith("_drv.o")) {
            int     seploc = fname.lastIndexOf(QDir::separator());
            QString p      = fname.mid(seploc + 1); // part after the last path separator

            if (p != fname) {
                QString             name   = p.mid(0, p.length() - 6) + "ModuleData";
                const IBinarySymbol *p_sym = m_binarySymbols->find(name);

                if (p_sym) {
                    Address tmpaddr = p_sym->getLocation();
                    Address setup, teardown;

                    /*uint32_t vers = */ m_image->readNative4(tmpaddr); // TODO: find use for vers ?
                    setup    = Address(m_image->readNative4(tmpaddr + 4));
                    teardown = Address(m_image->readNative4(tmpaddr + 8));

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
            const IBinarySymbol *p_sym = m_binarySymbols->find("init_module");

            if (p_sym) {
                entrypoints.push_back(p_sym->getLocation());
            }

            p_sym = m_binarySymbols->find("cleanup_module");

            if (p_sym) {
                entrypoints.push_back(p_sym->getLocation());
            }
        }
    }

    return entrypoints;
}


void IFrontEnd::decode(Prog *prg, bool decodeMain, const char *pname)
{
    Q_UNUSED(prg);
    assert(m_program == prg);

    if (pname) {
        m_program->setName(pname);
    }

    if (!decodeMain) {
        return;
    }

    Boomerang::get()->alertStartDecode(m_image->getLimitTextLow(),
                                       (m_image->getLimitTextHigh() - m_image->getLimitTextLow()).value());

    bool    gotMain;
    Address a = getMainEntryPoint(gotMain);
    LOG_VERBOSE("start: %1, gotMain: %2", a, (gotMain ? "true" : "false"));

    if (a == Address::INVALID) {
        std::vector<Address> entrypoints = getEntryPoints();

        for (auto& entrypoint : entrypoints) {
            decode(m_program, entrypoint);
        }

        return;
    }

    decode(m_program, a);
    m_program->addEntryPoint(a);

    if (!gotMain) {
        return;
    }

    static const char *mainName[] = { "main", "WinMain", "DriverEntry" };
    QString           name        = m_program->getSymbolByAddress(a);

    if (name == nullptr) {
        name = mainName[0];
    }

    for (auto& elem : mainName) {
        if (name != elem) {
            continue;
        }

        Function *proc = m_program->findFunction(a);

        if (proc == nullptr) {
            LOG_WARN("No proc found for address %1", a);
            return;
        }

        auto fty = std::dynamic_pointer_cast<FuncType>(Type::getNamedType(name));

        if (!fty) {
            LOG_WARN("Unable to find signature for known entrypoint %1", name);
        }
        else {
            proc->setSignature(fty->getSignature()->clone());
            proc->getSignature()->setName(name);
            // proc->getSignature()->setFullSig(true);        // Don't add or remove parameters
            proc->getSignature()->setForced(true); // Don't add or remove parameters
        }

        break;
    }
}


void IFrontEnd::decode(Prog *prg, Address addr)
{
    Q_UNUSED(prg);
    assert(m_program == prg);

    if (addr != Address::INVALID) {
        Function *newProc = m_program->createFunction(addr);

        // Sometimes, we have to adjust the entry address since
        // the instruction at addr is just a jump to another address.
        addr = newProc->getEntryAddress();
        LOG_MSG("Starting decode at address %1", addr);
        UserProc *proc = (UserProc *)m_program->findFunction(addr);

        if (proc == nullptr) {
            LOG_MSG("No proc found at address %1", addr);
            return;
        }

        if (proc->isLib()) {
            LOG_MSG("NOT decoding library proc at address %1", addr);
            return;
        }

        QTextStream os(stderr); // rtl output target

        if (processProc(addr, proc, os)) {
            proc->setDecoded();
        }
    }
    else {   // a == Address::INVALID
        bool change = true;
        LOG_MSG("Looking for undecoded procedures to decode...");

        while (change) {
            change = false;

            for (const auto& m : m_program->getModuleList()) {
                for (Function *function : *m) {
                    if (function->isLib()) {
                        continue;
                    }

                    UserProc *userProc = (UserProc *)function;

                    if (userProc->isDecoded()) {
                        continue;
                    }

                    // undecoded userproc.. decode it
                    change = true;
                    QTextStream os(stderr); // rtl output target
                    int         res = processProc(userProc->getEntryAddress(), userProc, os);

                    if (res != 1) {
                        break;
                    }

                    userProc->setDecoded();

                    // Break out of the loops if not decoding children
                    if (SETTING(noDecodeChildren)) {
                        break;
                    }
                }
            }

            if (SETTING(noDecodeChildren)) {
                break;
            }
        }
    }

    m_program->isWellFormed();
}


void IFrontEnd::decodeOnly(Prog *prg, Address addr)
{
    Q_UNUSED(prg);
    assert(m_program == prg);

    UserProc *p = (UserProc *)m_program->createFunction(addr);
    assert(!p->isLib());
    QTextStream os(stderr); // rtl output target

    if (processProc(p->getEntryAddress(), p, os)) {
        p->setDecoded();
    }

    m_program->isWellFormed();
}


void IFrontEnd::decodeFragment(UserProc *proc, Address a)
{
    if (SETTING(traceDecoder)) {
        LOG_MSG("Decoding fragment at address %1", a);
    }

    QTextStream os(stderr); // rtl output target
    processProc(a, proc, os, true);
}


bool IFrontEnd::decodeInstruction(Address pc, DecodeResult& result)
{
    if (!m_image || (m_image->getSectionByAddr(pc) == nullptr)) {
        LOG_ERROR("attempted to decode outside any known section at address %1");
        result.valid = false;
        return false;
    }

    const IBinarySection *pSect           = m_image->getSectionByAddr(pc);
    ptrdiff_t            host_native_diff = (pSect->getHostAddr() - pSect->getSourceAddr()).value();
    return m_decoder->decodeInstruction(pc, host_native_diff, result);
}


void IFrontEnd::readLibrarySignatures(const char *signatureFile, CallConv cc)
{
    std::unique_ptr<AnsiCParser> p;

    try {
        p.reset(new AnsiCParser(signatureFile, false));
    }
    catch (const char *) {
        LOG_ERROR("Cannot read library signature file '%1'", signatureFile);
        return;
    }

    Platform plat = getType();
    p->yyparse(plat, cc);

    for (auto& elem : p->signatures) {
        m_librarySignatures[elem->getName()] = elem;
        elem->setSigFile(signatureFile);
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
                                   RTL *pRtl)
{
    Q_UNUSED(sl);
    assert(sl.back() == *ss);

    if (dest == Address::INVALID) {
        return;
    }

    Function *proc = m_program->findFunction(dest);

    if (proc == nullptr) {
        auto symb = m_binarySymbols->find(dest);

        if (symb && symb->isImportedFunction()) {
            proc = m_program->createFunction(dest);
        }
    }

    if ((proc != nullptr) && (proc != (Function *)-1)) {
        CallStatement *call = new CallStatement();
        call->setDest(dest);
        call->setDestProc(proc);
        call->setReturnAfterCall(true);
        // also need to change it in the actual RTL
        std::list<Statement *>::iterator ss1 = ss;
        ss1++;
        assert(ss1 == sl.end());
        assert(!pRtl->empty());
        pRtl->back() = call;
        *ss          = call;
    }
}


bool IFrontEnd::refersToImportedFunction(const SharedExp& pDest)
{
    if (pDest && (pDest->getOper() == opMemOf) && (pDest->access<Exp, 1>()->getOper() == opIntConst)) {
        const IBinarySymbol *symbol = m_binarySymbols->find(pDest->access<Const, 1>()->getAddr());

        if (symbol && symbol->isImportedFunction()) {
            return true;
        }
    }

    return false;
}


bool IFrontEnd::processProc(Address uAddr, UserProc *pProc, QTextStream& /*os*/, bool /*frag*/ /* = false */,
                            bool spec /* = false */)
{
    BasicBlock *pBB; // Pointer to the current basic block

    // just in case you missed it
    Boomerang::get()->alertNew(pProc);

    // We have a set of CallStatement pointers. These may be disregarded if this is a speculative decode
    // that fails (i.e. an illegal instruction is found). If not, this set will be used to add to the set of calls
    // to be analysed in the cfg, and also to call newProc()
    std::list<CallStatement *> callList;

    // Indicates whether or not the next instruction to be decoded is the lexical successor of the current one.
    // Will be true for all NCTs and for CTIs with a fall through branch.
    bool sequentialDecode = true;

    Cfg *cfg = pProc->getCFG();

    // If this is a speculative decode, the second time we decode the same address, we get no cfg. Else an error.
    if (spec && (cfg == nullptr)) {
        return false;
    }

    assert(cfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    m_targetQueue.initial(uAddr);

    // Clear the pointer used by the caller prologue code to access the last call rtl of this procedure
    // decoder.resetLastCall();

    // ADDRESS initAddr = uAddr;
    int     nTotalBytes = 0;
    Address startAddr   = uAddr;
    Address lastAddr    = uAddr;

    while ((uAddr = m_targetQueue.getNextAddress(*cfg)) != Address::INVALID) {
        // The list of RTLs for the current basic block
        std::unique_ptr<RTLList> BB_rtls(new RTLList);

        // Keep decoding sequentially until a CTI without a fall through branch is decoded
        DecodeResult inst;

        while (sequentialDecode) {
            // Decode and classify the current source instruction
            if (SETTING(traceDecoder)) {
                LOG_MSG("*%1", uAddr);
            }

            // Decode the inst at uAddr.
            if (!decodeInstruction(uAddr, inst)) {
                LOG_ERROR("Invalid instruction at %1: 0x%2 0x%3 0x%4 0x%5", uAddr,
                    QString::number(m_image->readNative1(uAddr + 0), 16),
                    QString::number(m_image->readNative1(uAddr + 1), 16),
                    QString::number(m_image->readNative1(uAddr + 2), 16),
                    QString::number(m_image->readNative1(uAddr + 3), 16));
            }
            else if (inst.rtl->empty()) {
                LOG_VERBOSE("Instruction at address %1 is a no-op!", uAddr);
            }

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid) {
                delete inst.rtl;
                return false;
            }

            // Need to construct a new list of RTLs if a basic block has just been finished but decoding is
            // continuing from its lexical successor
            if (BB_rtls == nullptr) {
                BB_rtls.reset(new RTLList);
            }

            RTL *pRtl = inst.rtl;

            if (!inst.valid) {
                // Alert the watchers to the problem
                Boomerang::get()->alertBadDecode(uAddr);

                // An invalid instruction. Most likely because a call did not return (e.g. call _exit()), etc.
                // Best thing is to emit an INVALID BB, and continue with valid instructions
                // Emit the RTL anyway, so we have the address and maybe some other clues
                BB_rtls->push_back(new RTL(uAddr));
                pBB = cfg->createBB(BBType::Invalid, std::move(BB_rtls));
                sequentialDecode = false;
                BB_rtls          = nullptr;
                break; // try the next instruction in the queue
            }

            // alert the watchers that we have decoded an instruction
            Boomerang::get()->alertDecode(uAddr, inst.numBytes);
            nTotalBytes += inst.numBytes;

            // Check if this is an already decoded jump instruction (from a previous pass with propagation etc)
            // If so, we throw away the just decoded RTL (but we still may have needed to calculate the number
            // of bytes.. ick.)
            std::map<Address, RTL *>::iterator ff = m_previouslyDecoded.find(uAddr);

            if (ff != m_previouslyDecoded.end()) {
                delete pRtl;
                pRtl = ff->second;
                inst.rtl = pRtl; // don't leave the inst.rtl pointer dangling
            }

            if (pRtl == nullptr) {
                // This can happen if an instruction is "cancelled", e.g. call to __main in a hppa program
                // Just ignore the whole instruction
                if (inst.numBytes > 0) {
                    uAddr += inst.numBytes;
                }

                continue;
            }

            // Display RTL representation if asked
            if (SETTING(printRtl)) {
                QString     tgt;
                QTextStream st(&tgt);
                pRtl->print(st);
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
            std::list<Statement *> sl(pRtl->getStatements());

            for (auto ss = sl.begin(); ss != sl.end(); ss++) {
                Statement *s = *ss;
                s->setProc(pProc); // let's do this really early!

                if (m_refHints.find(pRtl->getAddress()) != m_refHints.end()) {
                    const QString& name(m_refHints[pRtl->getAddress()]);
                    Address        globAddr = m_program->getGlobalAddr(name);

                    if (globAddr != Address::INVALID) {
                        s->searchAndReplace(Const(globAddr), Unary::get(opAddrOf, Location::global(name, pProc)));
                    }
                }

                s->simplify();
                GotoStatement *stmt_jump = dynamic_cast<GotoStatement *>(s);

                // Check for a call to an already existing procedure (including self recursive jumps), or to the PLT
                // (note that a LibProc entry for the PLT function may not yet exist)
                if (s->getKind() == StmtType::Goto) {
                    preprocessProcGoto(ss, stmt_jump->getFixedDest(), sl, pRtl);
                    s = *ss; // *ss can be changed within processProc
                }

                switch (s->getKind())
                {
                case StmtType::Goto:
                    {
                        Address uDest = stmt_jump->getFixedDest();

                        // Handle one way jumps and computed jumps separately
                        if (uDest != Address::INVALID) {
                            BB_rtls->push_back(pRtl);
                            sequentialDecode = false;

                            pBB     = cfg->createBB(BBType::Oneway, std::move(BB_rtls));
                            BB_rtls = nullptr; // Clear when make new BB

                            // Exit the switch now if the basic block already existed
                            if (pBB == nullptr) {
                                break;
                            }

                            // Add the out edge if it is to a destination within the
                            // procedure
                            if (uDest < m_image->getLimitTextHigh()) {
                                m_targetQueue.visit(cfg, uDest, pBB);
                                cfg->addEdge(pBB, uDest);
                            }
                            else {
                                LOG_WARN("Goto instruction at address %1 branches beyond end of section, to %2", uAddr, uDest);
                            }
                        }
                    }
                    break;

                case StmtType::Case:
                    {
                        SharedExp pDest = stmt_jump->getDest();

                        if (pDest == nullptr) {                      // Happens if already analysed (now redecoding)
                            BB_rtls->push_back(pRtl);
                            pBB = cfg->createBB(BBType::Nway, std::move(BB_rtls)); // processSwitch will update num outedges
                            BB_rtls          = nullptr;              // New RTLList for next BB
                            IndirectJumpAnalyzer().processSwitch(pBB, pProc); // decode arms, set out edges, etc
                            sequentialDecode = false;                // Don't decode after the jump
                            break;                                   // Just leave it alone
                        }

                        // Check for indirect calls to library functions, especially in Win32 programs
                        if (refersToImportedFunction(pDest)) {
                            LOG_VERBOSE("Jump to a library function: %1, replacing with a call/ret.", stmt_jump);

                            // jump to a library function
                            // replace with a call ret
                            const IBinarySymbol *sym = m_binarySymbols->find(pDest->access<Const, 1>()->getAddr());
                            assert(sym != nullptr);
                            QString       func  = sym->getName();
                            CallStatement *call = new CallStatement;
                            call->setDest(pDest->clone());
                            LibProc *lp = pProc->getProg()->getLibraryProc(func);

                            if (lp == nullptr) {
                                LOG_FATAL("getLibraryProc() returned nullptr");
                            }

                            call->setDestProc(lp);
                            std::list<Statement *> *stmt_list = new std::list<Statement *>;
                            stmt_list->push_back(call);
                            BB_rtls->push_back(new RTL(pRtl->getAddress(), stmt_list));
                            pBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                            BB_rtls          = nullptr;
                            appendSyntheticReturn(pBB, pProc, pRtl);
                            sequentialDecode = false;

                            if (pRtl->getAddress() == pProc->getEntryAddress()) {
                                // it's a thunk
                                // Proc *lp = prog->findProc(func.c_str());
                                func = "__imp_" + func;
                                pProc->setName(func);
                                // lp->setName(func.c_str());
                                Boomerang::get()->alertUpdateSignature(pProc);
                            }

                            callList.push_back(call);
                            ss = sl.end();
                            ss--; // get out of the loop
                            break;
                        }

                        BB_rtls->push_back(pRtl);
                        // We create the BB as a COMPJUMP type, then change to an NWAY if it turns out to be a switch stmt
                        pBB = cfg->createBB(BBType::CompJump, std::move(BB_rtls));
                        BB_rtls          = nullptr; // New RTLList for next BB
                        LOG_VERBOSE("COMPUTED JUMP at address %1, pDest = %2", uAddr, pDest);

                        if (SETTING(noDecompile)) {
                            // try some hacks
                            if (pDest->isMemOf() && (pDest->getSubExp1()->getOper() == opPlus) &&
                                pDest->getSubExp1()->getSubExp2()->isIntConst()) {
                                // assume subExp2 is a jump table
                                Address jmptbl = pDest->access<Const, 1, 2>()->getAddr();

                                for (unsigned int i = 0; ; i++) {
                                    Address destAddr = Address(m_image->readNative4(jmptbl + 4 * i));

                                    if ((destAddr < m_image->getLimitTextLow()) || (destAddr >= m_image->getLimitTextHigh())) {
                                        break;
                                    }

                                    LOG_MSG("  guessed uDest %1", destAddr);

                                    m_targetQueue.visit(cfg, destAddr, pBB);
                                    cfg->addEdge(pBB, destAddr);
                                }

                                pBB->setType(BBType::Nway);
                            }
                        }

                        sequentialDecode = false;
                        break;
                    }

                case StmtType::Branch:
                    {
                        Address jumpDest = stmt_jump->getFixedDest();
                        BB_rtls->push_back(pRtl);

                        pBB = cfg->createBB(BBType::Twoway, std::move(BB_rtls));
                        // Create the list of RTLs for the next basic block and continue with the next instruction.
                        BB_rtls = nullptr;

                        // Stop decoding sequentially if the basic block already existed otherwise complete the basic block
                        if (pBB == nullptr) {
                            sequentialDecode = false;
                        }
                        else {
                            // Add the out edge if it is to a destination within the section
                            if (jumpDest < m_image->getLimitTextHigh()) {
                                m_targetQueue.visit(cfg, jumpDest, pBB);
                                cfg->addEdge(pBB, jumpDest);
                            }
                            else {
                                LOG_WARN("Branch instruction at address %1 branches beyond end of section, to %2", uAddr, jumpDest);
                                cfg->addEdge(pBB, Address::INVALID);
                            }

                            // Add the fall-through outedge
                            cfg->addEdge(pBB, uAddr + inst.numBytes);
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
                            QString  name       = m_binarySymbols->find(linkedAddr)->getName();
                            Function *function  = pProc->getProg()->getLibraryProc(name);
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
                            if (Util::inRange(callAddr, m_image->getLimitTextLow(), m_image->getLimitTextHigh())) {
                                DecodeResult decoded;

                                // Decode it.
                                if (decodeInstruction(callAddr, decoded) && !decoded.rtl->empty()) {
                                    // Decoded successfully. Create a Statement from it.
                                    Statement *first_statement = decoded.rtl->front();

                                    if (first_statement) {
                                        first_statement->setProc(pProc);
                                        first_statement->simplify();

                                        GotoStatement *jmpStatement = dynamic_cast<GotoStatement *>(first_statement);

                                        // This is a direct jump (x86 opcode FF 25)
                                        // The imported function is at the jump destination.
                                        if (jmpStatement && refersToImportedFunction(jmpStatement->getDest())) {
                                            // Yes, it's a library function. Look up it's name.
                                            Address functionAddr = jmpStatement->getDest()->access<Const, 1>()->getAddr();

                                            QString name = m_binarySymbols->find(functionAddr)->getName();
                                            // Assign the proc to the call
                                            Function *p = pProc->getProg()->getLibraryProc(name);

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

                                delete decoded.rtl;
                            }
                        }

                        // Treat computed and static calls separately
                        if (call->isComputed()) {
                            BB_rtls->push_back(pRtl);
                            pBB = cfg->createBB(BBType::CompCall, std::move(BB_rtls));

                            // Stop decoding sequentially if the basic block already
                            // existed otherwise complete the basic block
                            if (pBB == nullptr) {
                                sequentialDecode = false;
                            }
                            else {
                                cfg->addEdge(pBB, uAddr + inst.numBytes);
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
                            if (callAddr == uAddr + inst.numBytes) {
                                break;
                            }

                            // Call the virtual helper function. If implemented, will check for machine specific funcion
                            // calls
                            if (isHelperFunc(callAddr, uAddr, BB_rtls.get())) {
                                // We have already added to BB_rtls
                                delete pRtl;
                                pRtl = nullptr; // Discard the call semantics
                                break;
                            }

                            BB_rtls->push_back(pRtl);

                            // Add this non computed call site to the set of call sites which need to be analysed later.
                            // pCfg->addCall(call);
                            callList.push_back(call);

                            // Record the called address as the start of a new procedure if it didn't already exist.
                            if (!callAddr.isZero() && (callAddr != Address::INVALID) &&
                                (pProc->getProg()->findFunction(callAddr) == nullptr)) {
                                callList.push_back(call);

                                // newProc(pProc->getProg(), uNewAddr);
                                if (SETTING(traceDecoder)) {
                                    LOG_MSG("p%1", callAddr);
                                }
                            }

                            // Check if this is the _exit or exit function. May prevent us from attempting to decode
                            // invalid instructions, and getting invalid stack height errors
                            QString name = m_program->getSymbolByAddress(callAddr);

                            if (name.isEmpty() && refersToImportedFunction(call->getDest())) {
                                Address a = call->getDest()->access<Const, 1>()->getAddr();
                                name = m_binarySymbols->find(a)->getName();
                            }

                            if (!name.isEmpty() && IFrontEnd::isNoReturnCallDest(name)) {
                                // Make sure it has a return appended (so there is only one exit from the function)
                                // call->setReturnAfterCall(true);        // I think only the Sparc frontend cares
                                // Create the new basic block
                                pBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                                BB_rtls = nullptr;
                                appendSyntheticReturn(pBB, pProc, pRtl);

                                // Stop decoding sequentially
                                sequentialDecode = false;
                            }
                            else {
                                // Create the new basic block
                                pBB = cfg->createBB(BBType::Call, std::move(BB_rtls));
                                BB_rtls = nullptr;

                                if (call->isReturnAfterCall()) {
                                    // The only RTL in the basic block is one with a ReturnStatement
                                    std::list<Statement *> *instrList = new std::list<Statement *>;
                                    instrList->push_back(new ReturnStatement());

                                    // Constuct the RTLs for the new basic block
                                    std::unique_ptr<RTLList> rtls(new RTLList);
                                    rtls->push_back(new RTL(pRtl->getAddress() + 1, instrList));
                                    BasicBlock *returnBB = cfg->createBB(BBType::Ret, std::move(rtls));
                                    rtls = nullptr;

                                    // Add out edge from call to return
                                    cfg->addEdge(pBB, returnBB);

                                    // Mike: do we need to set return locations?
                                    // This ends the function
                                    sequentialDecode = false;
                                }
                                else {
                                    // Add the fall through edge if the block didn't
                                    // already exist
                                    if (pBB != nullptr) {
                                        cfg->addEdge(pBB, uAddr + inst.numBytes);
                                    }
                                }
                            }
                        }

                        extraProcessCall(call, *pBB->getRTLs());

                        // make sure we already moved the created RTL into a BB
                        assert(BB_rtls == nullptr);
                        break;
                    }

                case StmtType::Ret:
                    // Stop decoding sequentially
                    sequentialDecode = false;

                    // Create the list of RTLs for the next basic block and
                    // continue with the next instruction.
                    pBB = createReturnBlock(pProc, std::move(BB_rtls), pRtl);
                    BB_rtls = nullptr; // New RTLList for next BB
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

            if (BB_rtls && pRtl) {
                // If non null, we haven't put this RTL into a the current BB as yet
                BB_rtls->push_back(pRtl);
            }

            if (inst.reDecode) {
                // Special case: redecode the last instruction, without advancing uAddr by numBytes
                continue;
            }

            uAddr += inst.numBytes;

            if (uAddr > lastAddr) {
                lastAddr = uAddr;
            }

            // If sequentially decoding, check if the next address happens to be the start of an existing BB. If so,
            // finish off the current BB (if any RTLs) as a fallthrough, and no need to decode again (unless it's an
            // incomplete BB, then we do decode it).
            // In fact, mustn't decode twice, because it will muck up the coverage, but also will cause subtle problems
            // like add a call to the list of calls to be processed, then delete the call RTL (e.g. Pentium 134.perl
            // benchmark)
            if (sequentialDecode && cfg->isStartOfBB(uAddr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    BasicBlock *bb = cfg->createBB(BBType::Fall, std::move(BB_rtls));
                    BB_rtls = nullptr; // Need new list of RTLs

                    // Add an out edge to this address
                    if (bb) {
                        cfg->addEdge(bb, uAddr);
                    }
                }

                // Pick a new address to decode from, if the BB is complete
                if (!cfg->isStartOfIncompleteBB(uAddr)) {
                    sequentialDecode = false;
                }
            }
        } // while sequentialDecode

        // Add this range to the coverage
        //          pProc->addRange(start, uAddr);

        // Must set sequentialDecode back to true
        sequentialDecode = true;
    } // while nextAddress() != Address::INVALID


    for (CallStatement *callStmt : callList) {
        Address dest = callStmt->getFixedDest();
        auto    symb = m_binarySymbols->find(dest);

        // Don't speculatively decode procs that are outside of the main text section, apart from dynamically
        // linked ones (in the .plt)
        if ((symb && symb->isImportedFunction()) || !spec || (dest < m_image->getLimitTextHigh())) {
            // Don't visit the destination of a register call
            Function *np = callStmt->getDestProc();

            if ((np == nullptr) && (dest != Address::INVALID)) {
                // np = newProc(pProc->getProg(), dest);
                np = pProc->getProg()->createFunction(dest);
            }

            if (np != nullptr) {
                np->setFirstCaller(pProc);
                pProc->addCallee(np);
            }
        }
    }

    Boomerang::get()->alertDecode(pProc, startAddr, lastAddr, nTotalBytes);

    LOG_VERBOSE("Finished processing proc %1 at address %2", pProc->getName(), pProc->getEntryAddress());

    return true;
}


BasicBlock *IFrontEnd::createReturnBlock(UserProc *pProc, std::unique_ptr<RTLList> BB_rtls, RTL *pRtl)
{
    Cfg        *pCfg = pProc->getCFG();
    BasicBlock *pBB = nullptr;

    // Add the RTL to the list; this has the semantics for the return instruction as well as the ReturnStatement
    // The last Statement may get replaced with a GotoStatement
    if (BB_rtls == nullptr) {
        BB_rtls.reset(new RTLList); // In case no other semantics
    }

    BB_rtls->push_back(pRtl);
    Address retAddr = pProc->getTheReturnAddr();

    // LOG << "retAddr = " << retAddr << " rtl = " << pRtl->getAddress() << "\n";
    if (retAddr == Address::INVALID) {
        // Create the basic block
        pBB = pCfg->createBB(BBType::Ret, std::move(BB_rtls));
        BB_rtls = nullptr;
        Statement *s = pRtl->back(); // The last statement should be the ReturnStatement
        pProc->setTheReturnAddr((ReturnStatement *)s, pRtl->getAddress());
    }
    else {
        // We want to replace the *whole* RTL with a branch to THE first return's RTL. There can sometimes be extra
        // semantics associated with a return (e.g. Pentium return adds to the stack pointer before setting %pc and
        // branching). Other semantics (e.g. SPARC returning a value as part of the restore instruction) are assumed to
        // appear in a previous RTL. It is assumed that THE return statement will have the same semantics (NOTE: may
        // not always be valid). To avoid this assumption, we need branches to statements, not just to native addresses
        // (RTLs).
        BasicBlock *retBB = pProc->getCFG()->findRetNode();
        assert(retBB);

        if (retBB->getFirstStmt()->isReturn()) {
            // ret node has no semantics, clearly we need to keep ours
            assert(!pRtl->empty());
            pRtl->pop_back();
        }
        else {
            pRtl->clear();
        }

        pRtl->append(new GotoStatement(retAddr));
        pBB = pCfg->createBB(BBType::Oneway, std::move(BB_rtls));

        if (pBB) {
            // Visit the return instruction. This will be needed in most cases to split the return BB (if it has other
            // instructions before the return instruction).
            m_targetQueue.visit(pCfg, retAddr, pBB);
        }
    }

    // make sure the RTLs have been moved into the BB
    assert(BB_rtls == nullptr);
    return pBB;
}


void IFrontEnd::appendSyntheticReturn(BasicBlock *pCallBB, UserProc *pProc, RTL *pRtl)
{
    ReturnStatement *ret = new ReturnStatement();

    std::unique_ptr<RTLList> ret_rtls(new RTLList);
    std::list<Statement *> *stmt_list = new std::list<Statement *>;
    stmt_list->push_back(ret);
    BasicBlock *pret = createReturnBlock(pProc, std::move(ret_rtls), new RTL(pRtl->getAddress() + 1, stmt_list));

    pret->addPredecessor(pCallBB);
    assert(pCallBB->getNumSuccessors() == 0);
    pCallBB->addPredecessor(pret);
}
