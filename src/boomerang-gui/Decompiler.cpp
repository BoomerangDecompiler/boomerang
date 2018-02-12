#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Decompiler.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Settings.h"
#include "boomerang/frontend/Frontend.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/codegen/ICodeGenerator.h"

#include <QThread>


Decompiler::Decompiler()
    : QObject()
{
    // create empty project to initialize all relevant data
    Boomerang::get()->getOrCreateProject();
    Boomerang::get()->addWatcher(this);
}


void Decompiler::addEntryPoint(Address entryAddr, const QString& name)
{
    m_userEntrypoints.push_back(entryAddr);
    m_fe->addSymbol(entryAddr, name);
}


void Decompiler::removeEntryPoint(Address entryAddr)
{
    for (std::vector<Address>::iterator it = m_userEntrypoints.begin(); it != m_userEntrypoints.end(); ++it) {
        if (*it == entryAddr) {
            m_userEntrypoints.erase(it);
            break;
        }
    }
}


void Decompiler::loadInputFile(const QString& inputFile, const QString& outputPath)
{
    Boomerang::get()->getSettings()->setOutputDirectory(outputPath);
    emit loadingStarted();

    m_image = Boomerang::get()->getImage();
    m_prog  = new Prog(QFileInfo(inputFile).baseName());
    m_fe    = IFrontEnd::create(inputFile, m_prog, Boomerang::get()->getOrCreateProject());

    if (m_fe == nullptr) {
        emit machineTypeChanged(QString("Unavailable: Load Failed!"));
        return;
    }

    m_prog->setFrontEnd(m_fe);
    m_fe->readLibraryCatalog();

    switch (m_prog->getMachine())
    {
    case Machine::PENTIUM:
        emit machineTypeChanged("pentium");
        break;

    case Machine::SPARC:
        emit machineTypeChanged("sparc");
        break;

    case Machine::HPRISC:
        emit machineTypeChanged("hprisc");
        break;

    case Machine::PALM:
        emit machineTypeChanged("palm");
        break;

    case Machine::PPC:
        emit machineTypeChanged("ppc");
        break;

    case Machine::ST20:
        emit machineTypeChanged("st20");
        break;

    case Machine::MIPS:
        emit machineTypeChanged("mips");
        break;

    case Machine::M68K:
        emit machineTypeChanged("m68k");
        break;

    case Machine::UNKNOWN:
    case Machine::INVALID:
        emit machineTypeChanged("UNKNOWN");
        break;
    }

    std::vector<Address> entrypoints = m_fe->getEntryPoints();

    for (Address entryPoint : entrypoints) {
        m_userEntrypoints.push_back(entryPoint);
        emit entryPointAdded(entryPoint, m_prog->getSymbolByAddress(entryPoint));
    }

    for (const IBinarySection *section : *m_image) {
        emit sectionAdded(section->getName(), section->getSourceAddr(),
                        section->getSourceAddr() + section->getSize());
    }

    emit loadCompleted();
}


void Decompiler::decode()
{
    emit decodingStarted();


    LOG_MSG("Decoding program %1...", m_prog->getName());

    bool    gotMain;
    Address mainAddr = m_fe->getMainEntryPoint(gotMain);

    for (Address entryAddr : m_userEntrypoints) {
        if (entryAddr == mainAddr) {
            m_fe->decode(m_prog, true, nullptr);
            break;
        }
    }

    for (Address entryAddr : m_userEntrypoints) {
        m_prog->decodeEntryPoint(entryAddr);
    }

    if (!SETTING(noDecodeChildren)) {
        // decode anything undecoded
        m_fe->decode(m_prog, Address::INVALID);
    }

    LOG_MSG("Decoding finished!");
    m_prog->finishDecode();

    emit decodeCompleted();
}


void Decompiler::decompile()
{
    emit decompilingStarted();

    LOG_MSG("Starting decompile...");
    m_prog->decompile();
    LOG_MSG("Decompile finished!");

    emit decompileCompleted();
}


void Decompiler::moduleAndChildrenUpdated(Module *root)
{
    emit moduleCreated(root->getName());

    for (size_t i = 0; i < root->getNumChildren(); i++) {
        moduleAndChildrenUpdated(root->getChild(i));
    }
}


void Decompiler::generateCode()
{
    emit generatingCodeStarted();

    LOG_MSG("Generating code...");
    Boomerang::get()->getCodeGenerator()->generateCode(m_prog);

    Module *root = m_prog->getRootModule();

    if (root) {
        moduleAndChildrenUpdated(root);
    }

    for (const auto& module : m_prog->getModuleList()) {
        for (Function *p : *module) {
            if (p->isLib()) {
                continue;
            }

            emit functionAddedToModule(p->getName(), module->getName());
        }
    }

    LOG_MSG("Generating code completed!");
    emit generateCodeCompleted();
}


void Decompiler::alertDiscovered(Function *caller, Function *proc)
{
    emit procDiscovered(caller ? caller->getName() : "", proc->getName());
}


void Decompiler::alertDecompiling(UserProc *p)
{
    emit procDecompileStarted(p->getName());
}


void Decompiler::alertNew(Function *function)
{
    if (function->isLib()) {
        QString params;

        if ((function->getSignature() == nullptr) || function->getSignature()->isUnknown()) {
            params = "<unknown>";
        }
        else {
            for (size_t i = 0; i < function->getSignature()->getNumParams(); i++) {
                auto ty = function->getSignature()->getParamType(i);
                params.append(ty->getCtype());
                params.append(" ");
                params.append(function->getSignature()->getParamName(i));

                if (i != function->getSignature()->getNumParams() - 1) {
                    params.append(", ");
                }
            }
        }

        emit libProcCreated(function->getName(), params);
    }
    else {
        emit userProcCreated(function->getName(), function->getEntryAddress());
    }
}


void Decompiler::alertRemove(Function *function)
{
    if (function->isLib()) {
        emit libProcRemoved(function->getName());
    }
    else {
        emit userProcRemoved(function->getName(), function->getEntryAddress());
    }
}


void Decompiler::alertUpdateSignature(Function *p)
{
    alertNew(p);
}


bool Decompiler::getRTLForProc(const QString& name, QString& rtl)
{
    Function *p = m_prog->findFunction(name);

    if (p->isLib()) {
        return false;
    }

    UserProc    *up = static_cast<UserProc *>(p);
    QTextStream os(&rtl);
    up->print(os, true);
    return true;
}


void Decompiler::alertDecompileDebugPoint(UserProc *proc, const char *description)
{
    LOG_VERBOSE("%1: %2", proc->getName(), description);

    if (m_debugging) {
        m_waiting = true;
        emit debugPointHit(proc->getName(), description);

        while (m_waiting) {
            QThread::yieldCurrentThread();
        }
    }
}


void Decompiler::stopWaiting()
{
    m_waiting = false;
}


QString Decompiler::getSigFilePath(const QString& name)
{
    Function *function = m_prog->findFunction(name);

    if (!function || !function->isLib() || !function->getSignature()) {
        return "";
    }

    return function->getSignature()->getSigFilePath();
}


QString Decompiler::getClusterFile(const QString& name)
{
    Module *module = m_prog->findModule(name);
    return module ? module->getOutPath("c") : "";
}


void Decompiler::rereadLibSignatures()
{
    m_prog->updateLibrarySignatures();
}


void Decompiler::renameProc(const QString& oldName, const QString& newName)
{
    Function *p = m_prog->findFunction(oldName);

    if (p) {
        p->setName(newName);
    }
}


void Decompiler::getCompoundMembers(const QString& name, QTableWidget *tbl)
{
    auto ty = NamedType::getNamedType(name);

    tbl->setRowCount(0);

    if ((ty == nullptr) || !ty->resolvesToCompound()) {
        return;
    }

    std::shared_ptr<CompoundType> c = ty->as<CompoundType>();

    for (size_t i = 0; i < c->getNumTypes(); i++) {
        tbl->setRowCount(tbl->rowCount() + 1);
        tbl->setItem(tbl->rowCount() - 1, 0, new QTableWidgetItem(tr("%1").arg(c->getOffsetTo(i))));
        tbl->setItem(tbl->rowCount() - 1, 1, new QTableWidgetItem(tr("%1").arg(c->getOffsetTo(i) / 8)));
        tbl->setItem(tbl->rowCount() - 1, 2, new QTableWidgetItem(c->getName(i)));
        tbl->setItem(tbl->rowCount() - 1, 3, new QTableWidgetItem(tr("%1").arg(c->getTypeAtIdx(i)->getSize())));
    }
}
