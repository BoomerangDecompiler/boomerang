/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002-2003, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file     prog.cpp
 * \brief    Implementation of the program class. Holds information of
 *                interest to the whole program.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/
#include "prog.h"

#include "boomerang/util/Log.h"
#include "boomerang/core/BinaryFileFactory.h"

#include "boomerang/c/ansi-c-parser.h"

#include "boomerang/db/module.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h"
#include "boomerang/db/SymTab.h"
#include "boomerang/db/BinaryImage.h"
#include "boomerang/db/signature.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Location.h"

#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/db/managed.h"

#include "boomerang/loader/IBinaryFile.h"

#include "boomerang/passes/RangeAnalysis.h"

#include "boomerang/type/type.h"

#include "boomerang/util/types.h"
#include "boomerang/util/Util.h" // For lockFileWrite etc

#include "boomerang/frontend/frontend.h"

#include <QtCore/QFileInfo>
#include <QtCore/QSaveFile>
#include <QtCore/QDebug>
#include <QtCore/QXmlStreamWriter>
#include <QtCore/QDir>
#include <QtCore/QString>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>

#ifdef _WIN32
#  include <windows.h>
#  ifndef __MINGW32__
namespace dbghelp
{
#    include <dbghelp.h>
#  endif
}
#endif

#include <sys/types.h>


Prog::Prog(const QString& name)
    : m_defaultFrontend(nullptr)
    , m_name(name)
    , m_iNumberedProc(1)
{
    m_binarySymbols = (SymTab *)Boomerang::get()->getSymbols();
    m_rootCluster   = getOrInsertModule(getNameNoPathNoExt());
    m_path          = m_name;
    m_image         = Boomerang::get()->getImage();
}


Prog::~Prog()
{
    delete m_defaultFrontend;

    for (Module *m : m_moduleList) {
        delete m;
    }
}


Module *Prog::getOrInsertModule(const QString& name, const ModuleFactory& fact, IFrontEnd *frontEnd)
{
    for (Module *m : m_moduleList) {
        if (m->getName() == name) {
            return m;
        }
    }

    Module *m = fact.create(name, this, frontEnd ? frontEnd : m_defaultFrontend);
    m_moduleList.push_back(m);
    connect(this, SIGNAL(rereadLibSignatures()), m, SLOT(onLibrarySignaturesChanged()));
    return m;
}


void Prog::setFrontEnd(IFrontEnd *frontEnd)
{
    m_loaderIface     = frontEnd->getLoader();
    m_defaultFrontend = frontEnd;

    for (Module *m : m_moduleList) {
        delete m;
    }

    m_moduleList.clear();
    m_rootCluster = nullptr;

    if (m_loaderIface && !m_name.isEmpty()) {
        if (m_rootCluster) {
            m_rootCluster->eraseFromParent();
        }

        m_rootCluster = this->getOrInsertModule(getNameNoPathNoExt());
    }
}


void Prog::setName(const char *name)
{
    m_name = name;
    m_rootCluster->setName(name);
}


bool Prog::wellForm() const
{
    bool wellformed = true;

    for (Module *module : m_moduleList) {
        for (Function *func : *module) {
            if (!func->isLib()) {
                UserProc *u = (UserProc *)func;
                wellformed &= u->getCFG()->wellFormCfg();
            }
        }
    }

    return wellformed;
}


void Prog::finishDecode()
{
    for (Module *module : m_moduleList) {
        for (Function *func : *module) {
            if (func->isLib()) {
                continue;
            }

            UserProc *p = (UserProc *)func;

            if (!p->isDecoded()) {
                continue;
            }

            p->assignProcsToCalls();
            p->finalSimplify();
        }
    }
}


void Prog::generateDotFile() const
{
    assert(!Boomerang::get()->dotFile.isEmpty());
    QFile tgt(Boomerang::get()->dotFile);

    if (!tgt.open(QFile::WriteOnly | QFile::Text)) {
        return;
    }

    QTextStream of(&tgt);
    of << "digraph Cfg {\n";

    for (Module *module : m_moduleList) {
        for (Function *func : *module) {
            if (func->isLib()) {
                continue;
            }

            UserProc *p = (UserProc *)func;

            if (!p->isDecoded()) {
                continue;
            }

            // Subgraph for the proc name
            of << "\nsubgraph cluster_" << p->getName() << " {\n"
               << "       color=gray;\n    label=" << p->getName() << ";\n";
            // Generate dotty CFG for this proc
            p->getCFG()->generateDotFile(of);
        }
    }

    of << "}";
}


void Prog::generateDataSectionCode(QString section_name, Address section_start, uint32_t size, ICodeGenerator *code) const
{
    code->addGlobal("start_" + section_name, IntegerType::get(32, -1), Const::get(section_start));
    code->addGlobal(section_name + "_size", IntegerType::get(32, -1), Const::get(size ? size : (unsigned int)-1));
    auto l = Terminal::get(opNil);

    for (unsigned int i = 0; i < size; i++) {
        int n = m_image->readNative1(section_start + size - 1 - i);

        if (n < 0) {
            n = 256 + n;
        }

        l = Binary::get(opList, Const::get(n), l);
    }

    code->addGlobal(section_name, ArrayType::get(IntegerType::get(8, -1), size), l);
}


void Prog::generateCode(Module *cluster, UserProc *proc, bool /*intermixRTL*/) const
{
    // QString basedir = m_rootCluster->makeDirs();
    QTextStream *os = nullptr;

    if (cluster) {
        cluster->openStream("c");
        cluster->closeStreams();
    }

    const bool generate_all   = cluster == nullptr || cluster == m_rootCluster;
    bool all_procedures = proc == nullptr;

    if (generate_all) {
        m_rootCluster->openStream("c");
        os = &m_rootCluster->getStream();

        if (proc == nullptr) {
            ICodeGenerator *code  = Boomerang::get()->getCodeGenerator();
            bool           global = false;

            if (Boomerang::get()->noDecompile) {
                const char *sections[] = { "rodata", "data", "data1", nullptr };

                for (int j = 0; sections[j]; j++) {
                    QString str = ".";
                    str += sections[j];
                    IBinarySection *info = m_image->getSectionInfoByName(str);

                    if (info) {
                        generateDataSectionCode(sections[j], info->getSourceAddr(), info->getSize(), code);
                    }
                    else {
                        generateDataSectionCode(sections[j], Address::INVALID, 0, code);
                    }
                }

                code->addGlobal("source_endianness", IntegerType::get(STD_SIZE),
                                Const::get(getFrontEndId() != Platform::PENTIUM));
                (*os) << "#include \"boomerang.h\"\n\n";
                global = true;
            }

            for (Global *elem : m_globals) {
                // Check for an initial value
                SharedExp e = elem->getInitialValue(this);
                // if (e) {
                code->addGlobal(elem->getName(), elem->getType(), e);
                global = true;
            }

            if (global) {
                code->print(*os); // Avoid blank line if no globals
            }
        }
    }

    // First declare prototypes for all but the first proc
    bool first = true, proto = false;

    for (Module *module : m_moduleList) {
        for (Function *func : *module) {
            if (func->isLib()) {
                continue;
            }

            if (first) {
                first = false;
                continue;
            }

            proto = true;
            UserProc       *up   = (UserProc *)func;
            ICodeGenerator *code = Boomerang::get()->getCodeGenerator(up);
            code->addPrototype(up); // May be the wrong signature if up has ellipsis

            if (generate_all) {
                code->print(*os);
            }

            delete code;
        }
    }

    if (proto && generate_all) {
        *os << "\n"; // Separate prototype(s) from first proc
    }

    for (Module *module : m_moduleList) {
        if (!generate_all && (cluster != module)) {
            continue;
        }

        module->openStream("c");

        for (Function *func : *module) {
            if (func->isLib()) {
                continue;
            }

            UserProc *up = (UserProc *)func;

            if (!up->isDecoded()) {
                continue;
            }

            if (!all_procedures && (up != proc)) {
                continue;
            }

            up->getCFG()->compressCfg();
            up->getCFG()->removeOrphanBBs();

            ICodeGenerator *code = Boomerang::get()->getCodeGenerator(up);
            up->generateCode(code);
            code->print(module->getStream());
            delete code;
        }
    }

    for (Module *module : m_moduleList) {
        module->closeStreams();
    }
}


void Prog::generateRTL(Module *cluster, UserProc *proc) const
{
    bool generate_all   = cluster == nullptr;
    bool all_procedures = proc == nullptr;

    for (Module *module : m_moduleList) {
        if (!generate_all && (module != cluster)) {
            continue;
        }

        cluster->openStream("rtl");

        for (Function *func : *module) {
            if (!all_procedures && (func != proc)) {
                continue;
            }

            if (func->isLib()) {
                continue;
            }

            UserProc *p = (UserProc *)func;

            if (!p->isDecoded()) {
                continue;
            }

            p->print(module->getStream());
        }
    }

    for (Module *module : m_moduleList) {
        module->closeStreams();
    }
}


Instruction *Prog::getStmtAtLex(Module *cluster, unsigned int begin, unsigned int end) const
{
    bool search_all = cluster == nullptr;

    for (Module *m : m_moduleList) {
        if (!search_all && (m != cluster)) {
            continue;
        }

        for (Function *pProc : *m) {
            if (pProc->isLib()) {
                continue;
            }

            UserProc *p = (UserProc *)pProc;

            if (!p->isDecoded()) {
                continue;
            }

            Instruction *s = p->getStmtAtLex(begin, end);

            if (s) {
                return s;
            }
        }
    }

    return nullptr;
}


bool Prog::isModuleUsed(Module *c) const
{
    // TODO: maybe module can have no procedures and still be used ?
    return !c->empty();
}


Module *Prog::getDefaultModule(const QString& name)
{
    QString             cfname;
    const IBinarySymbol *bsym = m_binarySymbols->find(name);

    if (bsym) {
        cfname = bsym->belongsToSourceFile();
    }

    if (cfname.isEmpty() || !cfname.endsWith(".c")) {
        return m_rootCluster;
    }

    LOG << "got filename " << cfname << " for " << name << "\n";
    cfname.chop(2); // remove .c
    Module *c = findModule(cfname);

    if (c == nullptr) {
        c = getOrInsertModule(cfname);
        m_rootCluster->addChild(c);
    }

    return c;
}


void Prog::generateCode(QTextStream& os) const
{
    ICodeGenerator *code = Boomerang::get()->getCodeGenerator();

    for (Global *glob : m_globals) {
        // Check for an initial value
        auto e = glob->getInitialValue(this);

        if (e) {
            code->addGlobal(glob->getName(), glob->getType(), e);
        }
    }

    code->print(os);
    delete code;

    for (Module *module : m_moduleList) {
        for (Function *pProc : *module) {
            if (pProc->isLib()) {
                continue;
            }

            UserProc *p = (UserProc *)pProc;

            if (!p->isDecoded()) {
                continue;
            }

            p->getCFG()->compressCfg();
            code = Boomerang::get()->getCodeGenerator(p);
            p->generateCode(code);
            code->print(os);
            delete code;
        }
    }
}


void Prog::print(QTextStream& out) const
{
    for (Module *module : m_moduleList) {
        for (Function *proc : *module) {
            if (proc->isLib()) {
                continue;
            }

            UserProc *p = (UserProc *)proc;

            if (!p->isDecoded()) {
                continue;
            }

            // decoded userproc.. print it
            p->print(out);
        }
    }
}


void Prog::clear()
{
    m_name = "";

    for (Module *module : m_moduleList) {
        delete module;
    }

    m_moduleList.clear();
    delete m_defaultFrontend;
    m_defaultFrontend = nullptr;
}


Function *Prog::setNewProc(Address uAddr)
{
    // this test fails when decoding sparc, why?  Please investigate - trent
    // Likely because it is in the Procedure Linkage Table (.plt), which for Sparc is in the data section
    // assert(uAddr >= limitTextLow && uAddr < limitTextHigh);
    // Check if we already have this proc
    Function *pProc = findProc(uAddr);

    if (pProc == (Function *)-1) { // Already decoded and deleted?
        return nullptr;            // Yes, exit with 0
    }

    if (pProc) {      // Exists already ?
        return pProc; // Yes, we are done
    }

       Address other = m_loaderIface->getJumpTarget(uAddr);

    if (other != Address::INVALID) {
        uAddr = other;
    }

    pProc = findProc(uAddr);

    if (pProc) {      // Exists already ?
        return pProc; // Yes, we are done
    }

    QString             pName;
    const IBinarySymbol *sym = m_binarySymbols->find(uAddr);
    bool                bLib = false;

    if (sym) {
        bLib  = sym->isImportedFunction() || sym->isStaticFunction();
        pName = sym->getName();
    }

    if (pName.isEmpty()) {
        // No name. Give it a numbered name
        pName = QString("proc%1").arg(m_iNumberedProc++);
        LOG_VERBOSE(1) << "assigning name " << pName << " to addr " << uAddr << "\n";
    }

    pProc = m_rootCluster->getOrInsertFunction(pName, uAddr, bLib);
    return pProc;
}


#if defined(_WIN32) && !defined(__MINGW32__)

SharedType typeFromDebugInfo(int index, DWORD64 ModBase);

SharedType makeUDT(int index, DWORD64 ModBase)
{
    HANDLE hProcess = GetCurrentProcess();
    int    got;
    WCHAR  *name;

    got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_SYMNAME, &name);
    assert(got);

    if (got) {
        char nameA[1024];
        WideCharToMultiByte(CP_ACP, 0, name, -1, nameA, sizeof(nameA), 0, nullptr);
        SharedType ty = Type::getNamedType(nameA);

        if (ty) {
            return NamedType::get(nameA);
        }

        auto  cty   = CompoundType::get();
        DWORD count = 0;
        got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_CHILDRENCOUNT, &count);
        int FindChildrenSize = sizeof(dbghelp::TI_FINDCHILDREN_PARAMS) + count * sizeof(ULONG);
        dbghelp::TI_FINDCHILDREN_PARAMS *pFC = (dbghelp::TI_FINDCHILDREN_PARAMS *)malloc(FindChildrenSize);
        memset(pFC, 0, FindChildrenSize);
        pFC->Count = count;
        got        = SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_FINDCHILDREN, pFC);

        for (unsigned int i = 0; i < count; i++) {
            char fieldName[1024];
            got = dbghelp::SymGetTypeInfo(hProcess, ModBase, pFC->ChildId[i], dbghelp::TI_GET_SYMNAME, &name);
            WideCharToMultiByte(CP_ACP, 0, name, -1, fieldName, sizeof(fieldName), 0, nullptr);
            DWORD mytype;
            got = dbghelp::SymGetTypeInfo(hProcess, ModBase, pFC->ChildId[i], dbghelp::TI_GET_TYPE, &mytype);
            cty->addType(typeFromDebugInfo(mytype, ModBase), fieldName);
        }

        Type::addNamedType(nameA, cty);
        ty = Type::getNamedType(nameA);
        assert(ty);
        return NamedType::get(nameA);
    }

    return nullptr;
}


SharedType typeFromDebugInfo(int index, DWORD64 ModBase)
{
    HANDLE hProcess = GetCurrentProcess();

    int     got;
    DWORD   d;
    ULONG64 lsz = 0;

    got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_SYMTAG, &d);
    assert(got);
    got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_LENGTH, &lsz);
    int sz = (int)lsz * 8; // bits

    switch (d)
    {
    case 11:
        return makeUDT(index, ModBase);

    case 13:
        // TODO: signature
        return FuncType::get();

    case 14:
        got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_TYPE, &d);
        assert(got);
        return PointerType::get(typeFromDebugInfo(d, ModBase));

    case 15:
        got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_TYPE, &d);
        assert(got);
        got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_LENGTH, &lsz);
        assert(got);
        return ArrayType::get(typeFromDebugInfo(d, ModBase), (unsigned)lsz);

        break;

    case 16:
        got = dbghelp::SymGetTypeInfo(hProcess, ModBase, index, dbghelp::TI_GET_BASETYPE, &d);
        assert(got);

        switch (d)
        {
        case 1:
            return VoidType::get();

        case 2:
            return CharType::get();

        case 3:
            return CharType::get();

        case 6:  // int
        case 13: // long
            return IntegerType::get(sz, 1);

        case 7:  // unsigned int
        case 14: // ulong
            return IntegerType::get(sz, -1);

        case 8:
            return FloatType::get(sz);

        case 10:
            return BooleanType::get();

        default:
            LOG_STREAM() << "unhandled base type " << d << "\n";
            assert(false);
        }

        break;

    default:
        LOG_STREAM() << "unhandled symtag " << d << "\n";
        assert(false);
    }

    return nullptr;
}


int debugRegister(int r)
{
    switch (r)
    {
    case 2:
        return 26; // edx

    case 4:
        return 25; // ecx

    case 8:
        return 29; // ebp
    }

    assert(false);
    return -1;
}


BOOL CALLBACK addSymbol(dbghelp::PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext)
{
    Function *proc = (Function *)UserContext;

    if (pSymInfo->Flags & SYMFLAG_PARAMETER) {
        SharedType ty = typeFromDebugInfo(pSymInfo->TypeIndex, pSymInfo->ModBase);

        if (pSymInfo->Flags & SYMFLAG_REGREL) {
            assert(pSymInfo->Register == 8); // ebp
            proc->getSignature()->addParameter(
                ty, pSymInfo->Name,
                Location::memOf(Binary::get(opPlus, Location::regOf(28), Const::get((int)pSymInfo->Address - 4))));
        }
        else if (pSymInfo->Flags & SYMFLAG_REGISTER) {
            proc->getSignature()->addParameter(ty, pSymInfo->Name, Location::regOf(debugRegister(pSymInfo->Register)));
        }
    }
    else if ((pSymInfo->Flags & SYMFLAG_LOCAL) && !proc->isLib()) {
        UserProc *u = (UserProc *)proc;
        assert(pSymInfo->Flags & SYMFLAG_REGREL);
        assert(pSymInfo->Register == 8);
        SharedExp memref =
            Location::memOf(Binary::get(opMinus, Location::regOf(28), Const::get(-((int)pSymInfo->Address - 4))));
        SharedType ty = typeFromDebugInfo(pSymInfo->TypeIndex, pSymInfo->ModBase);
        u->addLocal(ty, pSymInfo->Name, memref);
    }

    return TRUE;
}


#endif


void Prog::removeProc(const QString& name)
{
    Function *f = findProc(name);

    if (f && (f != (Function *)-1)) {
        f->removeFromParent();
        Boomerang::get()->alertRemove(f);
        // FIXME: this function removes the function from module, but it leaks it
    }
}


int Prog::getNumProcs(bool user_only) const
{
    int n = 0;

    if (user_only) {
        for (Module *m : m_moduleList) {
            for (Function *pProc : *m) {
                if (!pProc->isLib()) {
                    n++;
                }
            }
        }
    }
    else {
        for (Module *m : m_moduleList) {
            n += m->size();
        }
    }

    return n;
}


Function *Prog::findProc(Address uAddr) const
{
    for (Module *m : m_moduleList) {
        Function *r = m->getFunction(uAddr);

        if (r != nullptr) {
            return r;
        }
    }

    return nullptr;
}


Function *Prog::findProc(const QString& name) const
{
    for (Module *m : m_moduleList) {
        Function *f = m->getFunction(name);

        if (f) {
            return f;
        }
    }

    return nullptr;
}


LibProc *Prog::getLibraryProc(const QString& nam) const
{
    Function *p = findProc(nam);

    if (p && p->isLib()) {
        return (LibProc *)p;
    }

    return (LibProc *)m_rootCluster->getOrInsertFunction(nam, Address::INVALID, true);
}


Platform Prog::getFrontEndId() const
{
    return m_defaultFrontend->getType();
}


std::shared_ptr<Signature> Prog::getDefaultSignature(const char *name) const
{
    return m_defaultFrontend->getDefaultSignature(name);
}


std::vector<SharedExp>& Prog::getDefaultParams()
{
    return m_defaultFrontend->getDefaultParams();
}


std::vector<SharedExp>& Prog::getDefaultReturns()
{
    return m_defaultFrontend->getDefaultReturns();
}


bool Prog::isWin32() const
{
    return m_defaultFrontend && m_defaultFrontend->isWin32();
}


QString Prog::getGlobalName(Address uaddr) const
{
    // FIXME: inefficient
    for (Global *glob : m_globals) {
        if (glob->containsAddress(uaddr)) {
            return glob->getName();
        }
    }

    return getSymbolByAddress(uaddr);
}


void Prog::dumpGlobals() const
{
    for (Global *glob : m_globals) {
        LOG_STREAM() << glob->toString() << "\n";
    }
}


Address Prog::getGlobalAddr(const QString& nam) const
{
    Global *glob = getGlobal(nam);

    if (glob) {
        return glob->getAddress();
    }

    auto symbol = m_binarySymbols->find(nam);
    return symbol ? symbol->getLocation() : Address::INVALID;
}


Global *Prog::getGlobal(const QString& nam) const
{
    auto iter =
        std::find_if(m_globals.begin(), m_globals.end(), [nam](Global *g) -> bool {
        return g->getName() == nam;
    });

    if (iter == m_globals.end()) {
        return nullptr;
    }

    return *iter;
}


bool Prog::markGlobalUsed(Address uaddr, SharedType knownType)
{
    for (Global *glob : m_globals) {
        if (glob->containsAddress(uaddr)) {
            if (knownType) {
                glob->meetType(knownType);
            }

            return true;
        }
    }

    if (m_image->getSectionInfoByAddr(uaddr) == nullptr) {
        LOG_VERBOSE(1) << "refusing to create a global at address that is in no known section of the binary: " << uaddr
                       << "\n";
        return false;
    }

    QString    nam = newGlobalName(uaddr);
    SharedType ty;

    if (knownType) {
        ty = knownType;

        if (ty->resolvesToArray() && ty->as<ArrayType>()->isUnbounded()) {
            SharedType baseType = ty->as<ArrayType>()->getBaseType();
            int        baseSize = 0;

            if (baseType) {
                baseSize = baseType->getBytes();
            }

            auto symbol = m_binarySymbols->find(nam);
            int  sz     = symbol ? symbol->getSize() : 0;

            if (sz && baseSize) {
                // Note: since ty is a pointer and has not been cloned, this will also set the type for knownType
                ty->as<ArrayType>()->setLength(sz / baseSize);
            }
        }
    }
    else {
        ty = guessGlobalType(nam, uaddr);
    }

    Global *global = new Global(ty, uaddr, nam, this);
    m_globals.insert(global);

    if (VERBOSE) {
        LOG << "globalUsed: name " << nam << ", address " << uaddr;

        if (knownType) {
            LOG << ", known type " << ty->getCtype() << "\n";
        }
        else {
            LOG << ", guessed type " << ty->getCtype() << "\n";
        }
    }

    return true;
}


std::shared_ptr<ArrayType> Prog::makeArrayType(Address u, SharedType t)
{
    QString nam = newGlobalName(u);

    assert(m_loaderIface);
    // TODO: fix the case of missing symbol table interface
    auto symbol = m_binarySymbols->find(nam);

    if (!symbol || (symbol->getSize() == 0)) {
        return ArrayType::get(t); // An "unbounded" array
    }

    unsigned int sz = symbol->getSize();
    int          n  = t->getBytes(); // TODO: use baseType->getBytes()

    if (n == 0) {
        n = 1;
    }

    return ArrayType::get(t, sz / n);
}


SharedType Prog::guessGlobalType(const QString& nam, Address u) const
{
#if defined(_WIN32) && !defined(__MINGW32__)
    HANDLE               hProcess = GetCurrentProcess();
    dbghelp::SYMBOL_INFO *sym     = (dbghelp::SYMBOL_INFO *)malloc(sizeof(dbghelp::SYMBOL_INFO) + 1000);
    sym->SizeOfStruct = sizeof(*sym);
    sym->MaxNameLen   = 1000;
    sym->Name[0]      = 0;
    BOOL got = dbghelp::SymFromAddr(hProcess, u.value(), 0, sym);

    if (got && *sym->Name && sym->TypeIndex) {
        assert(nam == sym->Name);
        return typeFromDebugInfo(sym->TypeIndex, sym->ModBase);
    }
#endif
    auto symbol = m_binarySymbols->find(nam);
    int  sz     = symbol ? symbol->getSize() : 0;

    if (sz == 0) {
        // Check if it might be a string
        const char *str = getStringConstant(u);

        if (str) {
            // return char* and hope it is dealt with properly
            return PointerType::get(CharType::get());
        }
    }

    SharedType ty;

    switch (sz)
    {
    case 1:
    case 2:
    case 4:
    case 8:
        ty = IntegerType::get(sz * 8);
        break;

    default:
        ty = std::make_shared<ArrayType>(std::make_shared<CharType>(), sz);
    }

    return ty;
}


QString Prog::newGlobalName(Address uaddr)
{
    QString nam = getGlobalName(uaddr);

    if (!nam.isEmpty()) {
        return nam;
    }

    nam = QString("global%1_%2").arg(m_globals.size()).arg(uaddr.value(), 0, 16);
    LOG_VERBOSE(1) << "naming new global: " << nam << " at address " << uaddr << "\n";
    return nam;
}


SharedType Prog::getGlobalType(const QString& nam) const
{
    for (Global *gl : m_globals) {
        if (gl->getName() == nam) {
            return gl->getType();
        }
    }

    return nullptr;
}


void Prog::setGlobalType(const QString& nam, SharedType ty)
{
    // FIXME: inefficient
    for (Global *gl : m_globals) {
        if (gl->getName() != nam) {
            continue;
        }

        gl->setType(ty);
        return;
    }
}


const char *Prog::getStringConstant(Address uaddr, bool knownString /* = false */) const
{
    const IBinarySection *si = m_image->getSectionInfoByAddr(uaddr);

    // Too many compilers put constants, including string constants, into read/write sections
    // if (si && si->bReadOnly)
    if (si && !si->isAddressBss(uaddr)) {
        // At this stage, only support ascii, null terminated, non unicode strings.
        // At least 4 of the first 6 chars should be printable ascii
        char *p = (char *)(si->getHostAddr() - si->getSourceAddr() + uaddr).value();

        if (knownString) {
            // No need to guess... this is hopefully a known string
            return p;
        }

        int  printable = 0;
        char last      = 0;

        for (int i = 0; i < 6; i++) {
            char c = p[i];

            if (c == 0) {
                break;
            }

            if ((c >= ' ') && (c < '\x7F')) {
                printable++;
            }

            last = c;
        }

        if (printable >= 4) {
            return p;
        }

        // Just a hack while type propagations are not yet ready
        if ((last == '\n') && (printable >= 2)) {
            return p;
        }
    }

    return nullptr;
}


double Prog::getFloatConstant(Address uaddr, bool& ok, int bits) const
{
    ok = true;
    const IBinarySection *si = m_image->getSectionInfoByAddr(uaddr);

    if (si && si->isReadOnly()) {
        if (bits == 64) { // TODO: handle 80bit floats ?
            return m_image->readNativeFloat8(uaddr);
        }
        else {
            assert(bits == 32);
            return m_image->readNativeFloat4(uaddr);
        }
    }

    ok = false;
    return 0.0;
}


QString Prog::getSymbolByAddress(Address dest) const
{
    auto sym = m_binarySymbols->find(dest);

    return sym ? sym->getName() : "";
}


const IBinarySection *Prog::getSectionInfoByAddr(Address a) const
{
    return m_image->getSectionInfoByAddr(a);
}


Address Prog::getLimitTextLow() const
{
    return Boomerang::get()->getImage()->getLimitTextLow();
}


Address Prog::getLimitTextHigh() const
{
    return Boomerang::get()->getImage()->getLimitTextHigh();
}


bool Prog::isReadOnly(Address a) const
{
    return m_image->isReadOnly(a);
}


int Prog::readNative1(Address a) const
{
    return m_image->readNative1(a);
}


int Prog::readNative2(Address a) const
{
    return m_image->readNative2(a);
}


int Prog::readNative4(Address a) const
{
    return m_image->readNative4(a);
}


Function *Prog::findContainingProc(Address uAddr) const
{
    for (Module *module : m_moduleList) {
        for (Function *p : *module) {
            if (p->getNativeAddress() == uAddr) {
                return p;
            }

            if (p->isLib()) {
                continue;
            }

            UserProc *u = (UserProc *)p;

            if (u->containsAddr(uAddr)) {
                return p;
            }
        }
    }

    return nullptr;
}


bool Prog::isProcLabel(Address addr) const
{
    for (Module *m : m_moduleList) {
        if (m->getFunction(addr)) {
            return true;
        }
    }

    return false;
}


QString Prog::getNameNoPath() const
{
    return QFileInfo(m_name).fileName();
}


QString Prog::getNameNoPathNoExt() const
{
    return QFileInfo(m_name).baseName();
}


const void *Prog::getCodeInfo(Address uAddr, const char *& last, int& delta) const
{
    delta = 0;
    last  = nullptr;
    const IBinarySection *pSect = m_image->getSectionInfoByAddr(uAddr);

    if (!pSect) {
        return nullptr;
    }

    if ((!pSect->isCode()) && (!pSect->isReadOnly())) {
        return nullptr;
    }

    // Search all code and read-only sections
    delta = (pSect->getHostAddr() - pSect->getSourceAddr()).value();
    last  = (const char *)(pSect->getHostAddr() + pSect->getSize()).value();
    const char *p = (const char *)(uAddr + delta).value();
    return p;
}


void Prog::decodeEntryPoint(Address a)
{
    Function *p = (UserProc *)findProc(a);

    if ((p == nullptr) || (!p->isLib() && !((UserProc *)p)->isDecoded())) {
        if ((a < m_image->getLimitTextLow()) || (a >= m_image->getLimitTextHigh())) {
            LOG_STREAM(LL_Warn) << "attempt to decode entrypoint at address outside text area, addr=" << a << "\n";
            return;
        }

        m_defaultFrontend->decode(this, a);
        finishDecode();
    }

    if (p == nullptr) {
        p = findProc(a);
    }

    assert(p);

    if (!p->isLib()) { // -sf procs marked as __nodecode are treated as library procs (?)
        m_entryProcs.push_back((UserProc *)p);
    }
}


void Prog::setEntryPoint(Address a)
{
    Function *p = (UserProc *)findProc(a);

    if ((p != nullptr) && !p->isLib()) {
        m_entryProcs.push_back((UserProc *)p);
    }
}


bool Prog::isDynamicLinkedProcPointer(Address dest) const
{
    auto sym = m_binarySymbols->find(dest);

    return sym && sym->isImportedFunction();
}


const QString& Prog::getDynamicProcName(Address uNative) const
{
    static QString dyn("dynamic");
    auto           sym = m_binarySymbols->find(uNative);

    return sym ? sym->getName() : dyn;
}


void Prog::decodeEverythingUndecoded()
{
    for (Module *module : m_moduleList) {
        for (Function *pp : *module) {
            UserProc *up = (UserProc *)pp;

            if (!pp || pp->isLib()) {
                continue;
            }

            if (up->isDecoded()) {
                continue;
            }

            m_defaultFrontend->decode(this, pp->getNativeAddress());
        }
    }

    finishDecode();
}


void Prog::decompile()
{
    Boomerang *boom = Boomerang::get();

    assert(!m_moduleList.empty());
    getNumProcs();
    LOG_VERBOSE(1) << getNumProcs(false) << " procedures\n";

    // Start decompiling each entry point
    for (UserProc *up : m_entryProcs) {
        ProcList call_path;
        LOG_VERBOSE(1) << "decompiling entry point " << up->getName() << "\n";
        int indent = 0;
        up->decompile(&call_path, indent);
    }

    // Just in case there are any Procs not in the call graph.

    if (boom->decodeMain && !boom->noDecodeChildren) {
        bool foundone = true;

        while (foundone) {
            foundone = false;

            for (Module *module : m_moduleList) {
                for (Function *pp : *module) {
                    if (pp->isLib()) {
                        continue;
                    }

                    UserProc *proc = (UserProc *)pp;

                    if (proc->isDecompiled()) {
                        continue;
                    }

                    int indent = 0;
                    proc->decompile(new ProcList, indent);
                    foundone = true;
                }
            }
        }
    }

    // Type analysis, if requested
    if (Boomerang::get()->conTypeAnalysis && Boomerang::get()->dfaTypeAnalysis) {
        LOG_STREAM() << "can't use two types of type analysis at once!\n";
        Boomerang::get()->conTypeAnalysis = false;
    }

    globalTypeAnalysis();

    if (!boom->noDecompile) {
        if (!boom->noRemoveReturns) {
            // A final pass to remove returns not used by any caller
            LOG_VERBOSE(1) << "prog: global removing unused returns\n";

            // Repeat until no change. Note 100% sure if needed.
            while (removeUnusedReturns()) {
            }
        }

        // print XML after removing returns
        for (Module *m :m_moduleList) {
            for (Function *pp : *m) {
                if (pp->isLib()) {
                    continue;
                }

                UserProc *proc = (UserProc *)pp;
                proc->printXML();
            }
        }
    }

    LOG_VERBOSE(1) << "transforming from SSA\n";

    // Now it is OK to transform out of SSA form
    fromSSAform();

    // removeUnusedLocals(); Note: is now in UserProc::generateCode()
    removeUnusedGlobals();
}


void Prog::removeUnusedGlobals()
{
    LOG_VERBOSE(1) << "removing unused globals\n";

    // seach for used globals
    std::list<SharedExp> usedGlobals;

    for (Module *module : m_moduleList) {
        for (Function *pp : *module) {
            if (pp->isLib()) {
                continue;
            }

            UserProc *u = (UserProc *)pp;
            Location search(opGlobal, Terminal::get(opWild), u);
            // Search each statement in u, excepting implicit assignments (their uses don't count, since they don't really
            // exist in the program representation)
            StatementList           stmts;
            StatementList::iterator ss;
            u->getStatements(stmts);

            for (Instruction *s : stmts) {
                if (s->isImplicit()) {
                    continue; // Ignore the uses in ImplicitAssigns
                }

                bool found = s->searchAll(search, usedGlobals);

                if (found && DEBUG_UNUSED) {
                    LOG << " a global is used by stmt " << s->getNumber() << "\n";
                }
            }
        }
    }

    // make a map to find a global by its name (could be a global var too)
    QMap<QString, Global *> namedGlobals;

    for (Global *g : m_globals) {
        namedGlobals[g->getName()] = g;
    }

    // rebuild the globals vector
    Global *usedGlobal;

    m_globals.clear();

    for (const SharedExp& e : usedGlobals) {
        if (DEBUG_UNUSED) {
            LOG << " " << e << " is used\n";
        }

        QString name(e->access<Const, 1>()->getStr());
        usedGlobal = namedGlobals[name];

        if (usedGlobal) {
            m_globals.insert(usedGlobal);
        }
        else {
            LOG << "warning: an expression refers to a nonexistent global\n";
        }
    }
}


bool Prog::removeUnusedReturns()
{
    // For each UserProc. Each proc may process many others, so this may duplicate some work. Really need a worklist of
    // procedures not yet processed.
    // Define a workset for the procedures who have to have their returns checked
    // This will be all user procs, except those undecoded (-sf says just trust the given signature)
    std::set<UserProc *> removeRetSet;
    bool                 change = false;

    for (Module *module : m_moduleList) {
        for (Function *pp : *module) {
            UserProc *proc = dynamic_cast<UserProc *>(pp);

            if ((nullptr == proc) || !proc->isDecoded()) {
                continue; // e.g. use -sf file to just prototype the proc
            }

            removeRetSet.insert(proc);
        }
    }

    // The workset is processed in arbitrary order. May be able to do better, but note that sometimes changes propagate
    // down the call tree (no caller uses potential returns for child), and sometimes up the call tree (removal of
    // returns and/or dead code removes parameters, which affects all callers).
    while (!removeRetSet.empty()) {
        auto it = removeRetSet.begin(); // Pick the first element of the set
        change |= (*it)->removeRedundantReturns(removeRetSet);
        // Note: removing the currently processed item here should prevent unnecessary reprocessing of self recursive
        // procedures
        removeRetSet.erase(it); // Remove the current element (may no longer be the first)
    }

    return change;
}


void Prog::fromSSAform()
{
    for (Module *module : m_moduleList) {
        for (Function *pp : *module) {
            if (pp->isLib()) {
                continue;
            }

            UserProc *proc = (UserProc *)pp;

            if (VERBOSE) {
                LOG << "===== before transformation from SSA form for " << proc->getName() << " =====\n" << *proc
                    << "===== end before transformation from SSA for " << proc->getName() << " =====\n\n";

                if (!Boomerang::get()->dotFile.isEmpty()) {
                    proc->printDFG();
                }
            }

            proc->fromSSAform();
            LOG_VERBOSE(1) << "===== after transformation from SSA form for " << proc->getName() << " =====\n" << *proc
                           << "===== end after transformation from SSA for " << proc->getName() << " =====\n\n";
        }
    }
}


void Prog::conTypeAnalysis()
{
    if (VERBOSE || DEBUG_TA) {
        LOG << "=== start constraint-based type analysis ===\n";
    }

    // FIXME: This needs to be done bottom of the call-tree first, with repeat until no change for cycles
    // in the call graph
    for (Module *module : m_moduleList) {
        for (Function *pp : *module) {
            UserProc *proc = (UserProc *)pp;

            if (proc->isLib() || !proc->isDecoded()) {
                continue;
            }

            proc->conTypeAnalysis();
        }
    }

    if (VERBOSE || DEBUG_TA) {
        LOG << "=== end type analysis ===\n";
    }
}


void Prog::globalTypeAnalysis()
{
    if (VERBOSE || DEBUG_TA) {
        LOG << "### start global data-flow-based type analysis ###\n";
    }

    for (Module *module : m_moduleList) {
        for (Function *pp : *module) {
            UserProc *proc = dynamic_cast<UserProc *>(pp);

            if ((nullptr == proc) || !proc->isDecoded()) {
                continue;
            }

            // FIXME: this just does local TA again. Need to meet types for all parameter/arguments, and return/results!
            // This will require a repeat until no change loop
            LOG_STREAM() << "global type analysis for " << proc->getName() << "\n";
            proc->typeAnalysis();
        }
    }

    if (VERBOSE || DEBUG_TA) {
        LOG << "### end type analysis ###\n";
    }
}


void Prog::rangeAnalysis()
{
    for (Module *module : m_moduleList) {
        RangeAnalysis ra;

        for (Function *pp : *module) {
            UserProc *proc = (UserProc *)pp;

            if (proc->isLib() || !proc->isDecoded()) {
                continue;
            }

            ra.runOnFunction(*proc);
        }
    }
}


void Prog::printCallGraph() const
{
    QString   fname1 = Boomerang::get()->getOutputDirectory().absoluteFilePath("callgraph.out");
    QString   fname2 = Boomerang::get()->getOutputDirectory().absoluteFilePath("callgraph.dot");
    QSaveFile file1(fname1);
    QSaveFile file2(fname2);

    if (!(file1.open(QFile::WriteOnly) && file2.open(QFile::WriteOnly))) {
        LOG_STREAM() << "Cannot open output files for callgraph output";
        return;
    }

    QTextStream                      f1(&file1);
    QTextStream                      f2(&file2);
    std::set<Function *>             seen;
    std::map<Function *, int>        spaces;
    std::map<Function *, Function *> parent;
    std::list<Function *>            procList;
    f2 << "digraph callgraph {\n";
    std::copy(m_entryProcs.begin(), m_entryProcs.end(), std::back_inserter(procList));

    spaces[procList.front()] = 0;

    while (procList.size()) {
        Function *p = procList.front();
        procList.erase(procList.begin());

        if (HostAddress(p) == HostAddress::INVALID) {
            continue;
        }

        if (seen.find(p) != seen.end()) {
            continue;
        }

        seen.insert(p);
        int n = spaces[p];

        for (int i = 0; i < n; i++) {
            f1 << "     ";
        }

        f1 << p->getName() << " @ " << p->getNativeAddress();

        if (parent.find(p) != parent.end()) {
            f1 << " [parent=" << parent[p]->getName() << "]";
        }

        f1 << '\n';

        if (!p->isLib()) {
            n++;
            UserProc               *u         = (UserProc *)p;
            std::list<Function *>& calleeList = u->getCallees();

            for (auto it1 = calleeList.rbegin(); it1 != calleeList.rend(); it1++) {
                procList.push_front(*it1);
                spaces[*it1] = n;
                parent[*it1] = p;
                f2 << p->getName() << " -> " << (*it1)->getName() << ";\n";
            }
        }
    }

    f2 << "}\n";
    f1.flush();
    f2.flush();
    file1.commit();
    file2.commit();
}


void printProcsRecursive(Function *proc, int indent, QTextStream& f, std::set<Function *>& seen)
{
    bool fisttime = false;

    if (seen.find(proc) == seen.end()) {
        seen.insert(proc);
        fisttime = true;
    }

    for (int i = 0; i < indent; i++) {
        f << "     ";
    }

    if (!proc->isLib() && fisttime) { // seen lib proc
        f << proc->getNativeAddress();
        f << " __nodecode __incomplete void " << proc->getName() << "();\n";

        UserProc *u = (UserProc *)proc;

        for (Function *callee : u->getCallees()) {
            printProcsRecursive(callee, indent + 1, f, seen);
        }

        for (int i = 0; i < indent; i++) {
            f << "     ";
        }

        f << "// End of " << proc->getName() << "\n";
    }
    else {
        f << "// " << proc->getName() << "();\n";
    }
}


Machine Prog::getMachine() const
{
    return m_loaderIface->getMachine();
}


void Prog::printSymbolsToFile() const
{
    LOG_STREAM() << "entering Prog::printSymbolsToFile\n";
    QString   fname = Boomerang::get()->getOutputDirectory().absoluteFilePath("symbols.h");
    QSaveFile tgt(fname);

    if (!tgt.open(QFile::WriteOnly)) {
        LOG_STREAM() << " Cannot open " << fname << " for writing\n";
        return;
    }

    QTextStream f(&tgt);

    /* Print procs */
    f << "/* Functions: */\n";
    std::set<Function *> seen;

    for (UserProc *up : m_entryProcs) {
        printProcsRecursive(up, 0, f, seen);
    }

    f << "/* Leftovers: */\n";

    for (Module *m : m_moduleList) {
        for (Function *pp : *m) {
            if (!pp->isLib() && (seen.find(pp) == seen.end())) {
                printProcsRecursive(pp, 0, f, seen);
            }
        }
    }

    f.flush();
    tgt.commit();
    LOG_STREAM() << "leaving Prog::printSymbolsToFile\n";
}


void Prog::printCallGraphXML() const
{
    if (!Boomerang::get()->dumpXML) {
        return;
    }

    for (Module *m : m_moduleList) {
        for (Function *it : *m) {
            it->clearVisited();
        }
    }

    QString     fname = Boomerang::get()->getOutputDirectory().absoluteFilePath("callgraph.xml");
    QSaveFile   CallGraphFile(fname);
    QTextStream f(&CallGraphFile);
    f << "<prog name=\"" << getName() << "\">\n";
    f << "     <callgraph>\n";

    for (UserProc *up : m_entryProcs) {
        up->printCallGraphXML(f, 2);
    }

    for (Module *m : m_moduleList) {
        for (Function *pp : *m) {
            if (!pp->isVisited() && !pp->isLib()) {
                pp->printCallGraphXML(f, 2);
            }
        }
    }

    f << "     </callgraph>\n";
    f << "</prog>\n";
    f.flush();
    CallGraphFile.commit();
}


Module *Prog::findModule(const QString& name) const
{
    for (Module *m : m_moduleList) {
        if (m->getName() == name) {
            return m;
        }
    }

    return nullptr;
}


void Prog::readSymbolFile(const QString& fname)
{
    std::ifstream ifs;

    ifs.open(fname.toStdString());

    if (!ifs.good()) {
        LOG << "can't open `" << fname << "'\n";
        exit(1);
    }

    AnsiCParser *par = new AnsiCParser(ifs, false);
    Platform    plat = getFrontEndId();
    CallConv    cc   = CallConv::C;

    if (isWin32()) {
        cc = CallConv::Pascal;
    }

    par->yyparse(plat, cc);
    Module *tgt_mod = getRootCluster();

    for (Symbol *sym : par->symbols) {
        if (sym->sig) {
            QString name = sym->sig->getName();
            tgt_mod = getDefaultModule(name);
            auto bin_sym       = m_binarySymbols->find(sym->addr);
            bool do_not_decode = (bin_sym && bin_sym->isImportedFunction()) ||
                                 // NODECODE isn't really the right modifier; perhaps we should have a LIB modifier,
                                 // to specifically specify that this function obeys library calling conventions
                                 sym->mods->noDecode;
            Function *p = tgt_mod->getOrInsertFunction(name, sym->addr, do_not_decode);

            if (!sym->mods->incomplete) {
                p->setSignature(sym->sig->clone());
                p->getSignature()->setForced(true);
            }
        }
        else {
            QString nam = sym->nam;

            if (sym->nam.isEmpty()) {
                nam = newGlobalName(sym->addr);
            }

            SharedType ty = sym->ty;

            if (ty == nullptr) {
                ty = guessGlobalType(nam, sym->addr);
            }

            m_globals.insert(new Global(ty, sym->addr, nam, this));
        }
    }

    for (SymbolRef *ref : par->refs) {
        m_defaultFrontend->addRefHint(ref->addr, ref->nam);
    }

    delete par;
    ifs.close();
}


SharedExp Global::getInitialValue(const Prog *prog) const
{
    const IBinarySection *si = prog->getSectionInfoByAddr(m_addr);

    // TODO: see what happens when we skip Bss check here
    if (si && si->isAddressBss(m_addr)) {
        // This global is in the BSS, so it can't be initialised
        // NOTE: this is not actually correct. at least for typing, BSS data can have a type assigned
        return nullptr;
    }

    if (si == nullptr) {
        return nullptr;
    }

    return prog->readNativeAs(m_addr, m_type);
}


QString Global::toString() const
{
    auto    init = getInitialValue(m_parent);
    QString res  = QString("%1 %2 at %3 initial value %4")
                      .arg(m_type->toString())
                      .arg(m_name)
                      .arg(m_addr.toString())
                      .arg((init ? init->prints() : "<none>"));

    return res;
}


SharedExp Prog::readNativeAs(Address uaddr, SharedType type) const
{
    SharedExp            e   = nullptr;
    const IBinarySection *si = getSectionInfoByAddr(uaddr);

    if (si == nullptr) {
        return nullptr;
    }

    if (type->resolvesToPointer()) {
              Address init = Address(readNative4(uaddr));

        if (init.isZero()) {
            return Const::get(0);
        }

        QString nam = getGlobalName(init);

        if (!nam.isEmpty()) {
            // TODO: typecast?
            return Location::global(nam, nullptr);
        }

        if (type->as<PointerType>()->getPointsTo()->resolvesToChar()) {
            const char *str = getStringConstant(init);

            if (str != nullptr) {
                return Const::get(str);
            }
        }
    }

    if (type->resolvesToCompound()) {
        std::shared_ptr<CompoundType> c = type->as<CompoundType>();
        auto n = e = Terminal::get(opNil);

        for (unsigned int i = 0; i < c->getNumTypes(); i++) {
                     Address    addr = uaddr + c->getOffsetTo(i) / 8;
            SharedType t    = c->getType(i);
            auto       v    = readNativeAs(addr, t);

            if (v == nullptr) {
                LOG << "unable to read native address " << addr << " as type " << t->getCtype() << "\n";
                v = Const::get(-1);
            }

            if (n->isNil()) {
                n = Binary::get(opList, v, n);
                e = n;
            }
            else {
                assert(n->getSubExp2()->getOper() == opNil);
                n->setSubExp2(Binary::get(opList, v, n->getSubExp2()));
                n = n->getSubExp2();
            }
        }

        return e;
    }

    if (type->resolvesToArray() && type->as<ArrayType>()->getBaseType()->resolvesToChar()) {
        const char *str = getStringConstant(uaddr, true);

        if (str) {
            // Make a global string
            return Const::get(str);
        }
    }

    if (type->resolvesToArray()) {
        int     nelems  = -1;
        QString nam     = getGlobalName(uaddr);
        int     base_sz = type->as<ArrayType>()->getBaseType()->getSize() / 8;

        if (!nam.isEmpty()) {
            auto symbol = m_binarySymbols->find(nam);
            nelems = symbol ? symbol->getSize() : 0;
            assert(base_sz);
            nelems /= base_sz;
        }

        auto n = e = Terminal::get(opNil);

        for (int i = 0; nelems == -1 || i < nelems; i++) {
            auto v = readNativeAs(uaddr + i * base_sz, type->as<ArrayType>()->getBaseType());

            if (v == nullptr) {
                break;
            }

            if (n->isNil()) {
                n = Binary::get(opList, v, n);
                e = n;
            }
            else {
                assert(n->getSubExp2()->getOper() == opNil);
                n->setSubExp2(Binary::get(opList, v, n->getSubExp2()));
                n = n->getSubExp2();
            }

            // "null" terminated
            if ((nelems == -1) && v->isConst() && (v->access<Const>()->getInt() == 0)) {
                break;
            }
        }
    }

    if (type->resolvesToInteger() || type->resolvesToSize()) {
        int size;

        if (type->resolvesToInteger()) {
            size = type->as<IntegerType>()->getSize();
        }
        else {
            size = type->as<SizeType>()->getSize();
        }

        switch (size)
        {
        case 8:
            return Const::get(m_image->readNative1(uaddr));

        case 16:
            // Note: must respect endianness
            return Const::get(m_image->readNative2(uaddr));

        case 32:
            return Const::get(m_image->readNative4(uaddr));

        case 64:
            return Const::get(m_image->readNative8(uaddr));
        }
    }

    if (!type->resolvesToFloat()) {
        return e;
    }

    switch (type->as<FloatType>()->getSize())
    {
    case 32:
        return Const::get(m_image->readNativeFloat4(uaddr));

    case 64:
        return Const::get(m_image->readNativeFloat8(uaddr));
    }

    return e;
}


void Global::meetType(SharedType ty)
{
    bool ch = false;

    m_type = m_type->meetWith(ty, ch);
}


void Prog::reDecode(UserProc *proc)
{
    QTextStream os(stderr); // rtl output target

    m_defaultFrontend->processProc(proc->getNativeAddress(), proc, os);
}


void Prog::decodeFragment(UserProc *proc, Address a)
{
    if ((a >= m_image->getLimitTextLow()) && (a < m_image->getLimitTextHigh())) {
        m_defaultFrontend->decodeFragment(proc, a);
    }
    else {
        LOG_STREAM() << "attempt to decode fragment outside text area, addr=" << a << "\n";

        if (VERBOSE) {
            LOG << "attempt to decode fragment outside text area, addr=" << a << "\n";
        }
    }
}


SharedExp Prog::addReloc(SharedExp e, Address lc)
{
    assert(e->isConst());

    if (!m_loaderIface->isRelocationAt(lc)) {
        return e;
    }

    auto c = e->access<Const>();
    // relocations have been applied to the constant, so if there is a
    // relocation for this lc then we should be able to replace the constant
    // with a symbol.
       Address             c_addr   = c->getAddr();
    const IBinarySymbol *bin_sym = m_binarySymbols->find(c_addr);

    if (bin_sym != nullptr) {
        unsigned int sz = bin_sym->getSize(); // TODO: fix the case of missing symbol table interface

        if (getGlobal(bin_sym->getName()) == nullptr) {
            Global *global = new Global(SizeType::get(sz * 8), c_addr, bin_sym->getName(), this);
            m_globals.insert(global);
        }

        return Unary::get(opAddrOf, Location::global(bin_sym->getName(), nullptr));
    }
    else {
        const char *str = getStringConstant(c_addr);

        if (str) {
            e = Const::get(str);
        }
        else {
            // check for accesses into the middle of symbols
            for (IBinarySymbol *it : *m_binarySymbols) {
                unsigned int sz = it->getSize();

                if ((it->getLocation() < c_addr) && ((it->getLocation() + sz) > c_addr)) {
                    int off = (c->getAddr() - it->getLocation()).value();
                    e = Binary::get(opPlus, Unary::get(opAddrOf, Location::global(it->getName(), nullptr)),
                                    Const::get(off));
                    break;
                }
            }
        }
    }

    return e;
}


bool Prog::isStringConstant(Address a) const
{
    const SectionInfo *si = static_cast<const SectionInfo *>(m_image->getSectionInfoByAddr(a));

    if (!si) {
        return false;
    }

    QVariant qv = si->attributeInRange("StringsSection", a, a + 1);
    return !qv.isNull();
}


bool Prog::isCFStringConstant(Address a) const
{
    return isStringConstant(a);
}
