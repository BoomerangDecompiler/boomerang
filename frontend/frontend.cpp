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

/***************************************************************************/ /**
  * \file       frontend.cpp
  * \brief   This file contains common code for all front ends. The majority
  *                of frontend logic remains in the source dependent files such as
  *                frontsparc.cpp
  ******************************************************************************/
#include "frontend.h"

#include "types.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "decoder.h"
#include "basicblock.h"
#include "sparc/sparcfrontend.h"
#include "pentium/pentiumfrontend.h"
#include "ppc/ppcfrontend.h"
#include "st20/st20frontend.h"
#include "mips/mipsfrontend.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"
#include "log.h"
#include "ansi-c-parser.h"
#include "IBinaryImage.h"
#include "db/SymTab.h"

#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <cstdarg> // For varargs
#include <sstream>

using namespace std;
/***************************************************************************/ /**
  *
  * \brief      Construct the FrontEnd object
  * \param p_BF pointer to the BinaryFile object (loader)
  * \param prog program being decoded
  * \param bff pointer to a BinaryFileFactory object (so the library can be unloaded)
  ******************************************************************************/
FrontEnd::FrontEnd(QObject *p_BF, Prog *prog, BinaryFileFactory *bff) : pLoader(p_BF), pbff(bff), Program(prog) {
    Image = Boomerang::get()->getImage();
    assert(Image);
    BinarySymbols = (SymTab *)Boomerang::get()->getSymbols();
    ldrIface = qobject_cast<LoaderInterface *>(pLoader);
}

/***************************************************************************/ /**
  *
  * \brief Create from a binary file
  * Static function to instantiate an appropriate concrete front end
  * \param pBF pointer to the BinaryFile object (loader)
  * \param prog program being decoded
  * \param pbff pointer to a BinaryFileFactory object (so the library can be unloaded)
  ******************************************************************************/
FrontEnd *FrontEnd::instantiate(QObject *pBF, Prog *prog, BinaryFileFactory *pbff) {
    LoaderInterface *ldr = qobject_cast<LoaderInterface *>(pBF);
    switch (ldr->getMachine()) {
    case MACHINE_PENTIUM:
        return new PentiumFrontEnd(pBF, prog, pbff);
    case MACHINE_SPARC:
        return new SparcFrontEnd(pBF, prog, pbff);
    case MACHINE_PPC:
        return new PPCFrontEnd(pBF, prog, pbff);
    case MACHINE_MIPS:
        return new MIPSFrontEnd(pBF, prog, pbff);
    case MACHINE_ST20:
        return new ST20FrontEnd(pBF, prog, pbff);
    case MACHINE_HPRISC:
        LOG_STREAM() << "No frontend for Hp Risc\n";
        break;
    case MACHINE_PALM:
        LOG_STREAM() << "No frontend for PALM\n";
        break;
    case MACHINE_68K:
        LOG_STREAM() << "No frontend for M68K\n";
        break;
    default:
        LOG_STREAM() << "Machine architecture not supported!\n";
    }
    return nullptr;
}

/***************************************************************************/ /**
  *
  * \brief Create FrontEnd instance given \a fname and \a prog
  * \param fname string with full path to decoded file
  * \param prog program being decoded
  * \returns Binary-specific frontend.
  ******************************************************************************/
FrontEnd *FrontEnd::Load(const QString &fname, Prog *prog) {
    BinaryFileFactory *pbff = new BinaryFileFactory;
    if (pbff == nullptr)
        return nullptr;
    QObject *pBF = pbff->Load(fname);
    if (pBF == nullptr)
        return nullptr;
    FrontEnd *fe = instantiate(pBF, prog, pbff);
    return fe;
}

void FrontEnd::AddSymbol(ADDRESS addr, const QString &nam) {
    BinarySymbols->create(addr,nam);
}
// destructor
FrontEnd::~FrontEnd() {
    if (pbff)
        pbff->UnLoad(); // Unload the BinaryFile library with dlclose() or FreeLibrary()
}

QString FrontEnd::getRegName(int idx) const {
    static QString nullres;
    for (const std::pair<QString,int> &elem : decoder->getRTLDict().RegMap)
        if (elem.second == idx)
            return elem.first;
    return nullres;
}

int FrontEnd::getRegSize(int idx) {
    if (decoder->getRTLDict().DetRegMap.find(idx) == decoder->getRTLDict().DetRegMap.end())
        return 32;
    return decoder->getRTLDict().DetRegMap[idx].g_size();
}

bool FrontEnd::isWin32() { return ldrIface->GetFormat() == LOADFMT_PE; }

bool FrontEnd::noReturnCallDest(const QString &name) {
    return ((name == "_exit") || (name == "exit") || (name == "ExitProcess") || (name == "abort") ||
            (name == "_assert"));
}

void FrontEnd::readLibraryCatalog(const QString &sPath) {
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    QFile file(sPath);
    if (!file.open(QFile::ReadOnly|QFile::Text)) {
        qCritical() << "can't open `" << sPath << "'\n";
        exit(1); //TODO: this should not exit, just inform the caller about the problem
    }
    QTextStream inf(&file);
    QString sig_path;
    while (!inf.atEnd()) {
        QString sFile;
        inf >> sFile;
        sFile = sFile.mid(0, sFile.indexOf('#')); // cut the line to first '#'
        if (sFile.size() > 0 && sFile.endsWith('\n'))
            sFile = sFile.mid(0, sFile.size() - 1);
        if (sFile.isEmpty())
            continue;
        sig_path = Boomerang::get()->getProgPath() + "signatures/" + sFile;
        callconv cc = CONV_C; // Most APIs are C calling convention
        if (sFile == "windows.h")
            cc = CONV_PASCAL; // One exception
        if (sFile == "mfc.h")
            cc = CONV_THISCALL; // Another exception
        readLibrarySignatures(qPrintable(sig_path), cc);
    }
}

void FrontEnd::readLibraryCatalog() {
    // TODO: this is a work for generic semantics provider plugin : HeaderReader
    LibrarySignatures.clear();
    QDir sig_dir(Boomerang::get()->getProgPath());
    if(!sig_dir.cd("signatures")) {
        qWarning("Signatures directory does not exist.");
        return;
    }
    QString sList = sig_dir.absoluteFilePath("common.hs");

    readLibraryCatalog(sList);
    sList = sig_dir.absoluteFilePath(Signature::platformName(getFrontEndId()) + ".hs");
    readLibraryCatalog(sList);
    if (isWin32()) {
        sList = sig_dir.absoluteFilePath("win32.hs");
        readLibraryCatalog(sList);
    }
    // TODO: change this to BinaryLayer query ("FILE_FORMAT","MACHO")
    if (ldrIface->GetFormat() == LOADFMT_MACHO) {
        sList = sig_dir.absoluteFilePath("objc.hs");
        readLibraryCatalog(sList);
    }
}

void FrontEnd::checkEntryPoint(std::vector<ADDRESS> &entrypoints, ADDRESS addr, const char *type) {
    SharedType ty = NamedType::getNamedType(type);
    assert(ty->isFunc());
    UserProc *proc = (UserProc *)Program->setNewProc(addr);
    assert(proc);
    Signature *sig = ty->asFunc()->getSignature()->clone();
    const IBinarySymbol *p_sym  = BinarySymbols->find(addr);
    QString sym = p_sym ? p_sym->getName() : QString("");
    if (!sym.isEmpty())
        sig->setName(sym);
    sig->setForced(true);
    proc->setSignature(sig);
    entrypoints.push_back(addr);
}

std::vector<ADDRESS> FrontEnd::getEntryPoints() {
    std::vector<ADDRESS> entrypoints;
    bool gotMain = false;
    // AssemblyLayer
    ADDRESS a = getMainEntryPoint(gotMain);
    // TODO: find exported functions and add them too ?
    if (a != NO_ADDRESS)
        entrypoints.push_back(a);
    else { // try some other tricks
        QString fname = ldrIface->getFilename();
        // X11 Module
        if (fname.endsWith("_drv.o")) {
            int seploc = fname.lastIndexOf(QDir::separator());
            QString p = fname.mid(seploc + 1); // part after the last path separator
            if (p != fname) {

                QString name = p.mid(0, p.length() - 6) + "ModuleData";
                const IBinarySymbol *p_sym = BinarySymbols->find(name);
                if (p_sym) {
                    ADDRESS tmpaddr = p_sym->getLocation();
                    ADDRESS setup, teardown;
                    /*uint32_t vers = */ Image->readNative4(tmpaddr); // TODO: find use for vers ?
                    setup = ADDRESS::g(Image->readNative4(tmpaddr + 4));
                    teardown = ADDRESS::g(Image->readNative4(tmpaddr + 8));
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
            const IBinarySymbol *p_sym =  BinarySymbols->find("init_module");
            if (p_sym)
                entrypoints.push_back(p_sym->getLocation());
            p_sym =  BinarySymbols->find("cleanup_module");
            if (p_sym)
                entrypoints.push_back(p_sym->getLocation());
        }
    }
    return entrypoints;
}

void FrontEnd::decode(Prog *prg, bool decodeMain, const char *pname) {
    assert(Program == prg);
    if (pname)
        Program->setName(pname);

    if (!decodeMain)
        return;
    Boomerang::get()->alertStartDecode(Image->getLimitTextLow(),
                                         (Image->getLimitTextHigh() - Image->getLimitTextLow()).m_value);

    bool gotMain;
    ADDRESS a = getMainEntryPoint(gotMain);
    LOG_VERBOSE(1) << "start: " << a << " gotmain: " << (gotMain ? "true" : "false") << "\n";
    if (a == NO_ADDRESS) {
        std::vector<ADDRESS> entrypoints = getEntryPoints();
        for (auto &entrypoint : entrypoints)
            decode(Program, entrypoint);
        return;
    }

    decode(Program, a);
    Program->setEntryPoint(a);

    if (!gotMain)
        return;
    static const char *mainName[] = {"main", "WinMain", "DriverEntry"};
    QString name = Program->symbolByAddress(a);
    if (name == nullptr)
        name = mainName[0];
    for (auto &elem : mainName) {
        if (name!=elem)
            continue;
        Function *proc = Program->findProc(a);
        if (proc == nullptr) {
            LOG_VERBOSE(1) << "no proc found for address " << a << "\n";
            return;
        }
        auto fty = std::dynamic_pointer_cast<FuncType>(Type::getNamedType(name));
        if (!fty)
            LOG << "unable to find signature for known entrypoint " << name << "\n";
        else {
            proc->setSignature(fty->getSignature()->clone());
            proc->getSignature()->setName(name);
            // proc->getSignature()->setFullSig(true);        // Don't add or remove parameters
            proc->getSignature()->setForced(true); // Don't add or remove parameters
        }
        break;
    }
}

// Somehow, a == NO_ADDRESS has come to mean decode anything not already decoded
void FrontEnd::decode(Prog *prg, ADDRESS a) {
    assert(Program == prg);
    if (a != NO_ADDRESS) {
        Program->setNewProc(a);
        LOG_VERBOSE(1) << "starting decode at address " << a << "\n";
        UserProc *p = (UserProc *)Program->findProc(a);
        if (p == nullptr) {
            LOG_VERBOSE(1) << "no proc found at address " << a << "\n";
            return;
        }
        if (p->isLib()) {
            LOG << "NOT decoding library proc at address 0x" << a << "\n";
            return;
        }
        QTextStream os(stderr); // rtl output target
        processProc(a, p, os);
        p->setDecoded();

    } else { // a == NO_ADDRESS
        bool change = true;
        while (change) {
            change = false;

            for ( Module *m : *Program) {
                for (Function *pProc : *m) {
                    if (pProc->isLib())
                        continue;
                    UserProc *p = (UserProc *)pProc;
                    if (p->isDecoded())
                        continue;
                    // undecoded userproc.. decode it
                    change = true;
                    QTextStream os(stderr); // rtl output target
                    int res = processProc(p->getNativeAddress(), p, os);
                    if (res != 1)
                        break;
                    p->setDecoded();
                    // Break out of the loops if not decoding children
                    if (Boomerang::get()->noDecodeChildren)
                        break;
                }
            }
            if (Boomerang::get()->noDecodeChildren)
                break;
        }
    }
    Program->wellForm();
}

//! \a a should be the address of an UserProc
void FrontEnd::decodeOnly(Prog *prg, ADDRESS a) {
    assert(Program == prg);
    UserProc *p = (UserProc *)Program->setNewProc(a);
    assert(!p->isLib());
    QTextStream os(stderr); // rtl output target
    if (processProc(p->getNativeAddress(), p, os))
        p->setDecoded();
    Program->wellForm();
}

void FrontEnd::decodeFragment(UserProc *proc, ADDRESS a) {
    if (Boomerang::get()->traceDecoder)
        LOG << "decoding fragment at 0x" << a << "\n";
    QTextStream os(stderr); // rtl output target
    processProc(a, proc, os, true);
}

DecodeResult &FrontEnd::decodeInstruction(ADDRESS pc) {
    if (!Image || Image->getSectionInfoByAddr(pc) == nullptr) {
        LOG << "ERROR: attempted to decode outside any known section " << pc << "\n";
        static DecodeResult invalid;
        invalid.reset();
        invalid.valid = false;
        return invalid;
    }
    const IBinarySection *pSect = Image->getSectionInfoByAddr(pc);
    ptrdiff_t host_native_diff = (pSect->hostAddr() - pSect->sourceAddr()).m_value;
    return decoder->decodeInstruction(pc, host_native_diff);
}

/***************************************************************************/ /**
  *
  * \brief       Read the library signatures from a file
  * \param       sPath The file to read from
  * \param       cc the calling convention assumed
  */
void FrontEnd::readLibrarySignatures(const char *sPath, callconv cc) {
    std::ifstream ifs;

    ifs.open(sPath);

    if (!ifs.good()) {
        LOG_STREAM() << "can't open `" << sPath << "'\n";
        exit(1);
    }

    AnsiCParser *p = new AnsiCParser(ifs, false);

    platform plat = getFrontEndId();
    p->yyparse(plat, cc);

    for (auto &elem : p->signatures) {
#if 0
        LOG_STREAM() << "readLibrarySignatures from " << sPath << ": " << (*it)->getName() << "\n";
#endif
        LibrarySignatures[(elem)->getName()] = elem;
        (elem)->setSigFile(sPath);
    }

    delete p;
    ifs.close();
}

Signature *FrontEnd::getDefaultSignature(const QString &name) {
    Signature *signature = nullptr;
    // Get a default library signature
    if (isWin32())
        signature = Signature::instantiate(PLAT_PENTIUM, CONV_PASCAL, name);
    else {
        signature = Signature::instantiate(getFrontEndId(), CONV_C, name);
    }
    return signature;
}

// get a library signature by name
Signature *FrontEnd::getLibSignature(const QString &name) {
    Signature *signature;
    // Look up the name in the librarySignatures map
    auto it = LibrarySignatures.find(name);
    if (it == LibrarySignatures.end()) {
        LOG << "Unknown library function " << name << "\n";
        signature = getDefaultSignature(name);
    } else {
        // Don't clone here; cloned in CallStatement::setSigArguments
        signature = *it;
        signature->setUnknown(false);
    }
    return signature;
}
void FrontEnd::preprocessProcGoto(std::list<Instruction *>::iterator ss, ADDRESS dest, const std::list<Instruction *> &sl,
                                  RTL *pRtl) {
    assert(sl.back() == *ss);
    if (dest == NO_ADDRESS)
        return;
    Function *proc = Program->findProc(dest);
    if (proc == nullptr) {
        auto symb = BinarySymbols->find(dest);
        if (symb && symb->isImportedFunction())
            proc = Program->setNewProc(dest);
    }
    if (proc != nullptr && proc != (Function *)-1) {
        CallStatement *call = new CallStatement();
        call->setDest(dest);
        call->setDestProc(proc);
        call->setReturnAfterCall(true);
        // also need to change it in the actual RTL
        std::list<Instruction *>::iterator ss1 = ss;
        ss1++;
        assert(ss1 == sl.end());
        assert(!pRtl->empty());
        pRtl->back() = call;
        *ss = call;
    }
}
bool FrontEnd::refersToImportedFunction(Exp *pDest)
{
    if(pDest && pDest->getOper() == opMemOf && pDest->getSubExp1()->getOper() == opIntConst) {
        auto symbol = BinarySymbols->find(((Const *)pDest->getSubExp1())->getAddr());
        if(symbol && symbol->isImportedFunction())
            return true;
    }
    return false;
}
/***************************************************************************/ /**
  *
  * \brief      Process a procedure, given a native (source machine) address.
  * \param uAddr - the address at which the procedure starts
  * \param pProc - the procedure object
  * \param frag - if true, this is just a fragment of a procedure
  * \param spec - if true, this is a speculative decode
  * \param os - the output stream for .rtl output
  * \note This is a sort of generic front end. For many processors, this will be overridden
  *  in the FrontEnd derived class, sometimes calling this function to do most of the work.
  * \returns          true for a good decode (no illegal instructions)
  ******************************************************************************/
bool FrontEnd::processProc(ADDRESS uAddr, UserProc *pProc, QTextStream &/*os*/, bool /*frag*/ /* = false */,
                           bool spec /* = false */) {
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

    Cfg *pCfg = pProc->getCFG();

    // If this is a speculative decode, the second time we decode the same address, we get no cfg. Else an error.
    if (spec && (pCfg == nullptr))
        return false;
    assert(pCfg);

    // Initialise the queue of control flow targets that have yet to be decoded.
    targetQueue.initial(uAddr);

    // Clear the pointer used by the caller prologue code to access the last call rtl of this procedure
    // decoder.resetLastCall();

    // ADDRESS initAddr = uAddr;
    int nTotalBytes = 0;
    ADDRESS startAddr = uAddr;
    ADDRESS lastAddr = uAddr;
    while ((uAddr = targetQueue.nextAddress(*pCfg)) != NO_ADDRESS) {
        // The list of RTLs for the current basic block
        std::list<RTL *> *BB_rtls = new std::list<RTL *>();

        // Keep decoding sequentially until a CTI without a fall through branch is decoded
        // ADDRESS start = uAddr;
        DecodeResult inst;
        while (sequentialDecode) {

            // Decode and classify the current source instruction
            if (Boomerang::get()->traceDecoder)
                LOG << "*" << uAddr << "\t";

            // Decode the inst at uAddr.
            inst = decodeInstruction(uAddr);
            if(!inst.valid || inst.rtl->empty()) {
                qDebug() << "Valid but undecoded instruction at " << QString::number(uAddr.m_value,16);
            }

            // If invalid and we are speculating, just exit
            if (spec && !inst.valid)
                return false;

            // Need to construct a new list of RTLs if a basic block has just been finished but decoding is
            // continuing from its lexical successor
            if (BB_rtls == nullptr)
                BB_rtls = new std::list<RTL *>();

            RTL *pRtl = inst.rtl;
            if (inst.valid == false) {
                // Alert the watchers to the problem
                Boomerang::get()->alertBadDecode(uAddr);

                // An invalid instruction. Most likely because a call did not return (e.g. call _exit()), etc.
                // Best thing is to emit a INVALID BB, and continue with valid instructions
                if (VERBOSE) {
                    LOG << "Warning: invalid instruction at " << uAddr << ": ";
                    // Emit the next 4 bytes for debugging
                    for (int ii = 0; ii < 4; ii++)
                        LOG << ADDRESS::g(Image->readNative1(uAddr + ii) & 0xFF) << " ";
                    LOG << "\n";
                }
                // Emit the RTL anyway, so we have the address and maybe some other clues
                BB_rtls->push_back(new RTL(uAddr));
                pBB = pCfg->newBB(BB_rtls, BBTYPE::INVALID, 0);
                sequentialDecode = false;
                BB_rtls = nullptr;
                continue;
            }

            // alert the watchers that we have decoded an instruction
            Boomerang::get()->alertDecode(uAddr, inst.numBytes);
            nTotalBytes += inst.numBytes;

            // Check if this is an already decoded jump instruction (from a previous pass with propagation etc)
            // If so, we throw away the just decoded RTL (but we still may have needed to calculate the number
            // of bytes.. ick.)
            std::map<ADDRESS, RTL *>::iterator ff = previouslyDecoded.find(uAddr);
            if (ff != previouslyDecoded.end())
                pRtl = ff->second;

            if (pRtl == nullptr) {
                // This can happen if an instruction is "cancelled", e.g. call to __main in a hppa program
                // Just ignore the whole instruction
                if (inst.numBytes > 0)
                    uAddr += inst.numBytes;
                continue;
            }

            // Display RTL representation if asked
            if (Boomerang::get()->printRtl) {
                QString tgt;
                QTextStream st(&tgt);
                pRtl->print(st);
                LOG << tgt;
            }

            ADDRESS uDest;

            // For each Statement in the RTL
            std::list<Instruction *> sl = *(std::list<Instruction *> *)pRtl;
            // Make a copy (!) of the list. This is needed temporarily to work around the following problem.
            // We are currently iterating an RTL, which could be a return instruction. The RTL is passed to
            // createReturnBlock; if this is not the first return statement, it will get cleared, and this will
            // cause problems with the current iteration. The effects seem to be worse for MSVC/Windows.
            // This problem will likely be easier to cope with when the RTLs are removed, and there are special
            // Statements to mark the start of instructions (and their native address).
            // FIXME: However, this workaround breaks logic below where a GOTO is changed to a CALL followed by a return
            // if it points to the start of a known procedure
            std::list<Instruction *>::iterator ss;
#if 1
            for (ss = sl.begin(); ss != sl.end(); ss++) { // }
#else
            // The counter is introduced because ss != sl.end() does not work as it should
            // FIXME: why? Does this really fix the problem?
            int counter = sl.size();
            for (ss = sl.begin(); counter > 0; ss++, counter--) {
#endif
                Instruction *s = *ss;
                s->setProc(pProc); // let's do this really early!
                if (refHints.find(pRtl->getAddress()) != refHints.end()) {
                    const QString &nam(refHints[pRtl->getAddress()]);
                    ADDRESS gu = Program->getGlobalAddr(nam);
                    if (gu != NO_ADDRESS) {
                        s->searchAndReplace(Const(gu), new Unary(opAddrOf, Location::global(nam, pProc)));
                    }
                }
                s->simplify();
                GotoStatement *stmt_jump = dynamic_cast<GotoStatement *>(s);

                // Check for a call to an already existing procedure (including self recursive jumps), or to the PLT
                // (note that a LibProc entry for the PLT function may not yet exist)
                if (s->getKind() == STMT_GOTO) {
                    preprocessProcGoto(ss, stmt_jump->getFixedDest(), sl, pRtl);
                    s = *ss; // *ss can be changed within processProc
                }

                switch (s->getKind()) {

                case STMT_GOTO: {
                    uDest = stmt_jump->getFixedDest();

                    // Handle one way jumps and computed jumps separately
                    if (uDest != NO_ADDRESS) {

                        BB_rtls->push_back(pRtl);
                        sequentialDecode = false;

                        pBB = pCfg->newBB(BB_rtls, BBTYPE::ONEWAY, 1);
                        BB_rtls = nullptr; // Clear when make new BB

                        // Exit the switch now if the basic block already existed
                        if (pBB == nullptr) {
                            break;
                        }

                        // Add the out edge if it is to a destination within the
                        // procedure
                        if (uDest < Image->getLimitTextHigh()) {
                            targetQueue.visit(pCfg, uDest, pBB);
                            pCfg->addOutEdge(pBB, uDest, true);
                        } else {
                            LOG << "Error: Instruction at " << uAddr << " branches beyond end of section, to " << uDest
                                << "\n";
                        }
                    }
                    break;
                }

                case STMT_CASE: {
                    Exp *pDest = stmt_jump->getDest();
                    if (pDest == nullptr) { // Happens if already analysed (now redecoding)
                        // SWITCH_INFO* psi = ((CaseStatement*)stmt_jump)->getSwitchInfo();
                        BB_rtls->push_back(pRtl);
                        pBB = pCfg->newBB(BB_rtls, BBTYPE::NWAY, 0); // processSwitch will update num outedges
                        pBB->processSwitch(pProc);           // decode arms, set out edges, etc
                        sequentialDecode = false;            // Don't decode after the jump
                        BB_rtls = nullptr;                   // New RTLList for next BB
                        break;                               // Just leave it alone
                    }
                    // Check for indirect calls to library functions, especially in Win32 programs
                    if (refersToImportedFunction(pDest)) {
                        LOG_VERBOSE(1) << "jump to a library function: " << stmt_jump << ", replacing with a call/ret.\n";
                        // jump to a library function
                        // replace with a call ret
                        auto *sym = BinarySymbols->find(((Const *)pDest->getSubExp1())->getAddr());
                        assert(sym==nullptr);
                        QString func = sym->getName();
                        CallStatement *call = new CallStatement;
                        call->setDest(pDest->clone());
                        LibProc *lp = pProc->getProg()->getLibraryProc(func);
                        if (lp == nullptr)
                            LOG << "getLibraryProc returned nullptr, aborting\n";
                        assert(lp);
                        call->setDestProc(lp);
                        std::list<Instruction *> *stmt_list = new std::list<Instruction *>;
                        stmt_list->push_back(call);
                        BB_rtls->push_back(new RTL(pRtl->getAddress(), stmt_list));
                        pBB = pCfg->newBB(BB_rtls, BBTYPE::CALL, 1);
                        appendSyntheticReturn(pBB, pProc, pRtl);
                        sequentialDecode = false;
                        BB_rtls = nullptr;
                        if (pRtl->getAddress() == pProc->getNativeAddress()) {
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
                    pBB = pCfg->newBB(BB_rtls, BBTYPE::COMPJUMP, 0);
                    LOG << "COMPUTED JUMP at " << uAddr << ", pDest = " << pDest << "\n";
                    if (Boomerang::get()->noDecompile) {
                        // try some hacks
                        if (pDest->isMemOf() && pDest->getSubExp1()->getOper() == opPlus &&
                            pDest->getSubExp1()->getSubExp2()->isIntConst()) {
                            // assume subExp2 is a jump table
                            ADDRESS jmptbl = ((Const *)pDest->getSubExp1()->getSubExp2())->getAddr();
                            unsigned int i;
                            for (i = 0;; i++) {
                                ADDRESS uDest = ADDRESS::g(Image->readNative4(jmptbl + i * 4));
                                if (Image->getLimitTextLow() <= uDest && uDest < Image->getLimitTextHigh()) {
                                    LOG << "  guessed uDest " << uDest << "\n";
                                    targetQueue.visit(pCfg, uDest, pBB);
                                    pCfg->addOutEdge(pBB, uDest, true);
                                } else
                                    break;
                            }
                            pBB->updateType(BBTYPE::NWAY, i);
                        }
                    }
                    sequentialDecode = false;
                    BB_rtls = nullptr; // New RTLList for next BB
                    break;
                }

                case STMT_BRANCH: {
                    uDest = stmt_jump->getFixedDest();
                    BB_rtls->push_back(pRtl);
                    pBB = pCfg->newBB(BB_rtls, BBTYPE::TWOWAY, 2);

                    // Stop decoding sequentially if the basic block already existed otherwise complete the basic block
                    if (pBB == nullptr)
                        sequentialDecode = false;
                    else {

                        // Add the out edge if it is to a destination within the procedure
                        if (uDest < Image->getLimitTextHigh()) {
                            targetQueue.visit(pCfg, uDest, pBB);
                            pCfg->addOutEdge(pBB, uDest, true);
                        } else {
                            LOG << "Error: Instruction at " << uAddr << " branches beyond end of section, to " << uDest
                                << "\n";
                        }

                        // Add the fall-through outedge
                        pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                    }

                    // Create the list of RTLs for the next basic block and continue with the next instruction.
                    BB_rtls = nullptr;
                    break;
                }

                case STMT_CALL: {
                    CallStatement *call = static_cast<CallStatement *>(s);

                    // Check for a dynamic linked library function
                    if (refersToImportedFunction(call->getDest())) {
                        // Dynamic linked proc pointers are treated as static.
                        ADDRESS linked_addr = ((Const *)call->getDest()->getSubExp1())->getAddr();
                        QString nam = BinarySymbols->find(linked_addr)->getName();
                        Function *p = pProc->getProg()->getLibraryProc(nam);
                        call->setDestProc(p);
                        call->setIsComputed(false);
                    }

                    // Is the called function a thunk calling a library function?
                    // A "thunk" is a function which only consists of: "GOTO library_function"
                    if (call && call->getFixedDest() != NO_ADDRESS) {
                        // Get the address of the called function.
                        ADDRESS callAddr = call->getFixedDest();
                        // It should not be in the PLT either, but getLimitTextHigh() takes this into account
                        if (callAddr < Image->getLimitTextHigh()) {
                            // Decode it.
                            DecodeResult decoded = decodeInstruction(callAddr);
                            if (decoded.valid && !decoded.rtl->empty()) { // is the instruction decoded succesfully?
                                // Yes, it is. Create a Statement from it.
                                RTL *rtl = decoded.rtl;
                                Instruction *first_statement = rtl->front();
                                if (first_statement) {
                                    first_statement->setProc(pProc);
                                    first_statement->simplify();
                                    CaseStatement *stmt_jump = dynamic_cast<CaseStatement *>(first_statement);
                                    // In fact it's a computed (looked up) jump, so the jump seems to be a case
                                    // statement.
                                    if ( nullptr!=stmt_jump &&
                                        refersToImportedFunction(stmt_jump->getDest())) { // Is it an "DynamicLinkedProcPointer"?
                                        // Yes, it's a library function. Look up it's name.
                                        ADDRESS a = ((Const *)stmt_jump->getDest()->getSubExp1())->getAddr();
                                        QString nam = BinarySymbols->find(a)->getName();
                                        // Assign the proc to the call
                                        Function *p = pProc->getProg()->getLibraryProc(nam);
                                        if (call->getDestProc()) {
                                            // prevent unnecessary __imp procs
                                            Program->removeProc(call->getDestProc()->getName());
                                        }
                                        call->setDestProc(p);
                                        call->setIsComputed(false);
                                        call->setDest(Location::memOf(new Const(a)));
                                    }
                                }
                            }
                        }
                    }

                    // Treat computed and static calls separately
                    if (call->isComputed()) {
                        BB_rtls->push_back(pRtl);
                        pBB = pCfg->newBB(BB_rtls, BBTYPE::COMPCALL, 1);

                        // Stop decoding sequentially if the basic block already
                        // existed otherwise complete the basic block
                        if (pBB == nullptr)
                            sequentialDecode = false;
                        else
                            pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                        // Add this call to the list of calls to analyse. We won't
                        // be able to analyse it's callee(s), of course.
                        callList.push_back(call);
                    } else { // Static call
                        // Find the address of the callee.
                        ADDRESS uNewAddr = call->getFixedDest();

                        // Calls with 0 offset (i.e. call the next instruction) are simply pushing the PC to the
                        // stack. Treat these as non-control flow instructions and continue.
                        if (uNewAddr == uAddr + inst.numBytes)
                            break;

                        // Call the virtual helper function. If implemented, will check for machine specific funcion
                        // calls
                        if (helperFunc(uNewAddr, uAddr, BB_rtls)) {
                            // We have already added to BB_rtls
                            pRtl = nullptr; // Discard the call semantics
                            break;
                        }

                        BB_rtls->push_back(pRtl);

                        // Add this non computed call site to the set of call sites which need to be analysed later.
                        // pCfg->addCall(call);
                        callList.push_back(call);

                        // Record the called address as the start of a new procedure if it didn't already exist.
                        if (!uNewAddr.isZero() && uNewAddr != NO_ADDRESS &&
                            pProc->getProg()->findProc(uNewAddr) == nullptr) {
                            callList.push_back(call);
                            // newProc(pProc->getProg(), uNewAddr);
                            if (Boomerang::get()->traceDecoder)
                                LOG << "p" << uNewAddr << "\t";
                        }

                        // Check if this is the _exit or exit function. May prevent us from attempting to decode
                        // invalid instructions, and getting invalid stack height errors
                        QString name = Program->symbolByAddress(uNewAddr);
                        if (name.isEmpty() && refersToImportedFunction(call->getDest())) {
                            ADDRESS a = ((Const *)call->getDest()->getSubExp1())->getAddr();
                            name = BinarySymbols->find(a)->getName();
                        }
                        if (!name.isEmpty() && noReturnCallDest(name)) {
                            // Make sure it has a return appended (so there is only one exit from the function)
                            // call->setReturnAfterCall(true);        // I think only the Sparc frontend cares
                            // Create the new basic block
                            pBB = pCfg->newBB(BB_rtls, BBTYPE::CALL, 1);
                            appendSyntheticReturn(pBB, pProc, pRtl);

                            // Stop decoding sequentially
                            sequentialDecode = false;
                        } else {
                            // Create the new basic block
                            pBB = pCfg->newBB(BB_rtls, BBTYPE::CALL, 1);

                            if (call->isReturnAfterCall()) {
                                // Constuct the RTLs for the new basic block
                                std::list<RTL *> *rtls = new std::list<RTL *>();
                                // The only RTL in the basic block is one with a ReturnStatement
                                std::list<Instruction *> *sl = new std::list<Instruction *>;
                                sl->push_back(new ReturnStatement());
                                rtls->push_back(new RTL(pRtl->getAddress() + 1, sl));

                                BasicBlock *returnBB = pCfg->newBB(rtls, BBTYPE::RET, 0);
                                // Add out edge from call to return
                                pCfg->addOutEdge(pBB, returnBB);
                                // Put a label on the return BB (since it's an orphan); a jump will be reqd
                                pCfg->setLabel(returnBB);
                                pBB->setJumpReqd();
                                // Mike: do we need to set return locations?
                                // This ends the function
                                sequentialDecode = false;
                            } else {
                                // Add the fall through edge if the block didn't
                                // already exist
                                if (pBB != nullptr)
                                    pCfg->addOutEdge(pBB, uAddr + inst.numBytes);
                            }
                        }
                    }

                    extraProcessCall(call, BB_rtls);

                    // Create the list of RTLs for the next basic block and continue with the next instruction.
                    BB_rtls = nullptr;
                    break;
                }

                case STMT_RET: {
                    // Stop decoding sequentially
                    sequentialDecode = false;

                    pBB = createReturnBlock(pProc, BB_rtls, pRtl);

                    // Create the list of RTLs for the next basic block and
                    // continue with the next instruction.
                    BB_rtls = nullptr; // New RTLList for next BB
                } break;

                case STMT_BOOLASSIGN:
                // This is just an ordinary instruction; no control transfer
                // Fall through
                case STMT_JUNCTION:
                // FIXME: Do we need to do anything here?
                case STMT_ASSIGN:
                case STMT_PHIASSIGN:
                case STMT_IMPASSIGN:
                case STMT_IMPREF:
                    // Do nothing
                    break;

                } // switch (s->getKind())
            }
            if (BB_rtls && pRtl)
                // If non null, we haven't put this RTL into a the current BB as yet
                BB_rtls->push_back(pRtl);

            if (inst.reDecode)
                // Special case: redecode the last instruction, without advancing uAddr by numBytes
                continue;
            uAddr += inst.numBytes;
            if (uAddr > lastAddr)
                lastAddr = uAddr;

            // If sequentially decoding, check if the next address happens to be the start of an existing BB. If so,
            // finish off the current BB (if any RTLs) as a fallthrough, and no need to decode again (unless it's an
            // incomplete BB, then we do decode it).
            // In fact, mustn't decode twice, because it will muck up the coverage, but also will cause subtle problems
            // like add a call to the list of calls to be processed, then delete the call RTL (e.g. Pentium 134.perl
            // benchmark)
            if (sequentialDecode && pCfg->existsBB(uAddr)) {
                // Create the fallthrough BB, if there are any RTLs at all
                if (BB_rtls) {
                    BasicBlock *pBB = pCfg->newBB(BB_rtls, BBTYPE::FALL, 1);
                    // Add an out edge to this address
                    if (pBB) {
                        pCfg->addOutEdge(pBB, uAddr);
                        BB_rtls = nullptr; // Need new list of RTLs
                    }
                }
                // Pick a new address to decode from, if the BB is complete
                if (!pCfg->isIncomplete(uAddr))
                    sequentialDecode = false;
            }
        } // while sequentialDecode

        // Add this range to the coverage
        //          pProc->addRange(start, uAddr);

        // Must set sequentialDecode back to true
        sequentialDecode = true;

    } // while nextAddress() != NO_ADDRESS

    // ProgWatcher *w = prog->getWatcher();
    // if (w)
    //      w->alert_done(pProc, initAddr, lastAddr, nTotalBytes);

    // Add the callees to the set of CallStatements, and also to the Prog object
    std::list<CallStatement *>::iterator it;
    for (it = callList.begin(); it != callList.end(); it++) {
        ADDRESS dest = (*it)->getFixedDest();
        auto symb = BinarySymbols->find(dest);
        // Don't speculatively decode procs that are outside of the main text section, apart from dynamically
        // linked ones (in the .plt)
        if ((symb && symb->isImportedFunction()) || !spec || (dest < Image->getLimitTextHigh())) {
            pCfg->addCall(*it);
            // Don't visit the destination of a register call
            Function *np = (*it)->getDestProc();
            if (np == nullptr && dest != NO_ADDRESS) {
                // np = newProc(pProc->getProg(), dest);
                np = pProc->getProg()->setNewProc(dest);
            }
            if (np != nullptr) {
                np->setFirstCaller(pProc);
                pProc->addCallee(np);
            }
        }
    }

    Boomerang::get()->alertDecode(pProc, startAddr, lastAddr, nTotalBytes);

    if (VERBOSE)
        LOG << "finished processing proc " << pProc->getName() << " at address " << pProc->getNativeAddress() << "\n";

    return true;
}

/***************************************************************************/ /**
  *
  * \brief      Decode the RTL at the given address
  * \param      address - native address of the instruction
  * \param      delta - difference between host and native addresses
  * \param      decoder - decoder object
  * \note       Only called from findCoverage()
  * \returns    a pointer to the decoded RTL
  ******************************************************************************/
RTL *decodeRtl(ADDRESS address, int delta, NJMCDecoder *decoder) {
    DecodeResult inst = decoder->decodeInstruction(address, delta);
    RTL *rtl = inst.rtl;
    return rtl;
}

/***************************************************************************/ /**
  *
  * \brief    Get a Prog object (mainly for testing and not decoding)
  * \returns        Pointer to a Prog object (with pFE and pBF filled in)
  ******************************************************************************/
Prog *FrontEnd::getProg() { return Program; }

/***************************************************************************/ /**
  *
  * \brief    Create a Return or a Oneway BB if a return statement already exists
  * \param    pProc: pointer to enclosing UserProc
  * \param    BB_rtls: list of RTLs for the current BB (not including pRtl)
  * \param    pRtl: pointer to the current RTL with the semantics for the return statement (including a
  *           ReturnStatement as the last statement)
  * \returns  Pointer to the newly created BB
  ******************************************************************************/
BasicBlock *FrontEnd::createReturnBlock(UserProc *pProc, std::list<RTL *> *BB_rtls, RTL *pRtl) {
    Cfg *pCfg = pProc->getCFG();
    BasicBlock *pBB;
    // Add the RTL to the list; this has the semantics for the return instruction as well as the ReturnStatement
    // The last Statement may get replaced with a GotoStatement
    if (BB_rtls == nullptr)
        BB_rtls = new std::list<RTL *>; // In case no other semantics
    BB_rtls->push_back(pRtl);
    ADDRESS retAddr = pProc->getTheReturnAddr();
    // LOG << "retAddr = " << retAddr << " rtl = " << pRtl->getAddress() << "\n";
    if (retAddr == NO_ADDRESS) {
        // Create the basic block
        pBB = pCfg->newBB(BB_rtls, BBTYPE::RET, 0);
        Instruction *s = pRtl->back(); // The last statement should be the ReturnStatement
        pProc->setTheReturnAddr((ReturnStatement *)s, pRtl->getAddress());
    } else {
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
        } else
            pRtl->clear();
        pRtl->appendStmt(new GotoStatement(retAddr));
        try {
            pBB = pCfg->newBB(BB_rtls, BBTYPE::ONEWAY, 1);
            // if BB already exists but is incomplete, exception is thrown
            pCfg->addOutEdge(pBB, retAddr, true);
            // Visit the return instruction. This will be needed in most cases to split the return BB (if it has other
            // instructions before the return instruction).
            targetQueue.visit(pCfg, retAddr, pBB);
        } catch (Cfg::BBAlreadyExistsError &) {
            if (VERBOSE)
                LOG << "not visiting " << retAddr << " due to exception\n";
        }
    }
    return pBB;
}

// Add a synthetic return instruction (or branch to the existing return instruction).
// NOTE: the call BB should be created with one out edge (the return or branch BB)
void FrontEnd::appendSyntheticReturn(BasicBlock *pCallBB, UserProc *pProc, RTL *pRtl) {
    ReturnStatement *ret = new ReturnStatement();
    std::list<RTL *> *ret_rtls = new std::list<RTL *>();
    std::list<Instruction *> *stmt_list = new std::list<Instruction *>;
    stmt_list->push_back(ret);
    BasicBlock *pret = createReturnBlock(pProc, ret_rtls, new RTL(pRtl->getAddress() + 1, stmt_list));
    pret->addInEdge(pCallBB);
    pCallBB->setOutEdge(0, pret);
}
