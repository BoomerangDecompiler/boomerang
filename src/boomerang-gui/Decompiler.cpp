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

#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/binary/BinaryFile.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/module/Module.h"
#include "boomerang/db/proc/LibProc.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ifc/ICodeGenerator.h"
#include "boomerang/ifc/IFrontEnd.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/util/log/Log.h"

#include <QThread>


Decompiler::Decompiler()
    : QObject()
{
    Log::getOrCreateLog().addDefaultLogSinks(
        m_project.getSettings()->getOutputDirectory().absolutePath());

    m_project.addWatcher(this);
    m_project.loadPlugins();
}


Decompiler::~Decompiler()
{
}


void Decompiler::addEntryPoint(Address entryAddr, const QString &name)
{
    m_userEntrypoints.push_back(entryAddr);
    m_project.getLoadedBinaryFile()->getSymbols()->createSymbol(entryAddr, name);
}


void Decompiler::removeEntryPoint(Address entryAddr)
{
    for (std::vector<Address>::iterator it = m_userEntrypoints.begin();
         it != m_userEntrypoints.end(); ++it) {
        if (*it == entryAddr) {
            m_userEntrypoints.erase(it);
            break;
        }
    }
}


void Decompiler::loadInputFile(const QString &inputFile, const QString &outputPath)
{
    m_project.getSettings()->setOutputDirectory(outputPath);
    emit loadingStarted();

    bool ok = m_project.loadBinaryFile(inputFile);
    if (!ok) {
        emit machineTypeChanged(QString("Unavailable: Load Failed!"));
        return;
    }

    switch (m_project.getLoadedBinaryFile()->getMachine()) {
    case Machine::X86: emit machineTypeChanged("pentium"); break;
    case Machine::SPARC: emit machineTypeChanged("sparc"); break;
    case Machine::PPC: emit machineTypeChanged("ppc"); break;
    case Machine::ST20: emit machineTypeChanged("st20"); break;
    case Machine::UNKNOWN:
    case Machine::INVALID: emit machineTypeChanged("UNKNOWN"); break;
    }

    IFrontEnd *fe = m_project.getProg()->getFrontEnd();
    assert(fe != nullptr);
    std::vector<Address> entrypoints = fe->findEntryPoints();

    for (Address entryPoint : entrypoints) {
        m_userEntrypoints.push_back(entryPoint);
        emit entryPointAdded(entryPoint, m_project.getProg()->getSymbolNameByAddr(entryPoint));
    }

    for (const BinarySection *section : *m_project.getLoadedBinaryFile()->getImage()) {
        emit sectionAdded(section->getName(), section->getSourceAddr(),
                          section->getSourceAddr() + section->getSize());
    }

    emit loadCompleted();
}


void Decompiler::decode()
{
    emit decodingStarted();

    LOG_MSG("Decoding program %1...", m_project.getProg()->getName());

    bool ok = m_project.decodeBinaryFile();
    if (!ok) {
        emit machineTypeChanged(QString("Unavailable: Decode Failed!"));
        return;
    }

    LOG_MSG("Decoding finished!");
    emit decodeCompleted();
}


void Decompiler::decompile()
{
    emit decompilingStarted();

    LOG_MSG("Starting decompile...");
    m_project.decompileBinaryFile();
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
    m_project.generateCode();

    Module *root = m_project.getProg()->getRootModule();

    if (root) {
        moduleAndChildrenUpdated(root);
    }

    for (const auto &module : m_project.getProg()->getModuleList()) {
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


void Decompiler::onFunctionDiscovered(Function *proc)
{
    emit procDiscovered("", proc->getName());
}


void Decompiler::onDecompileInProgress(UserProc *p)
{
    emit procDecompileStarted(p->getName());
}


void Decompiler::onFunctionCreated(Function *function)
{
    if (function->isLib()) {
        QString params;

        if ((function->getSignature() == nullptr) || function->getSignature()->isUnknown()) {
            params = "<unknown>";
        }
        else {
            for (int i = 0; i < function->getSignature()->getNumParams(); i++) {
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


void Decompiler::onFunctionRemoved(Function *function)
{
    if (function->isLib()) {
        emit libProcRemoved(function->getName());
    }
    else {
        emit userProcRemoved(function->getName(), function->getEntryAddress());
    }
}


void Decompiler::onSignatureUpdated(Function *p)
{
    onFunctionCreated(p);
}


bool Decompiler::getRTLForProc(const QString &name, QString &rtl)
{
    Function *p = m_project.getProg()->getFunctionByName(name);

    if (!p || p->isLib()) {
        return false;
    }

    assert(dynamic_cast<UserProc *>(p) != nullptr);
    UserProc *up = static_cast<UserProc *>(p);
    OStream os(&rtl);
    up->print(os);
    return true;
}


void Decompiler::onDecompileDebugPoint(UserProc *proc, const char *description)
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


QString Decompiler::getSigFilePath(const QString &name)
{
    Function *function = m_project.getProg()->getFunctionByName(name);

    if (!function || !function->isLib() || !function->getSignature()) {
        return "";
    }

    return function->getSignature()->getSigFilePath();
}


QString Decompiler::getClusterFile(const QString &name)
{
    Module *module = m_project.getProg()->findModule(name);
    return module ? module->getOutPath("c") : "";
}


void Decompiler::rereadLibSignatures()
{
    m_project.getProg()->updateLibrarySignatures();
}


void Decompiler::renameProc(const QString &oldName, const QString &newName)
{
    Function *proc = m_project.getProg()->getFunctionByName(oldName);

    if (proc) {
        proc->setName(newName);
    }
}


void Decompiler::getCompoundMembers(const QString &name, QTableWidget *tbl)
{
    auto ty = NamedType::getNamedType(name);

    tbl->setRowCount(0);

    if ((ty == nullptr) || !ty->resolvesToCompound()) {
        return;
    }

    std::shared_ptr<CompoundType> c = ty->as<CompoundType>();

    for (int i = 0; i < c->getNumMembers(); i++) {
        tbl->setRowCount(tbl->rowCount() + 1);
        tbl->setItem(tbl->rowCount() - 1, 0,
                     new QTableWidgetItem(tr("%1").arg(c->getMemberOffsetByIdx(i))));
        tbl->setItem(tbl->rowCount() - 1, 1,
                     new QTableWidgetItem(tr("%1").arg(c->getMemberOffsetByIdx(i) / 8)));
        tbl->setItem(tbl->rowCount() - 1, 2, new QTableWidgetItem(c->getMemberNameByIdx(i)));
        tbl->setItem(tbl->rowCount() - 1, 3,
                     new QTableWidgetItem(tr("%1").arg(c->getMemberTypeByIdx(i)->getSize())));
    }
}
