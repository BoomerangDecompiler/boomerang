#include "DecompilerThread.h"

#include "boomerang/util/Log.h"
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/Module.h"
#include "boomerang/db/IBinarySection.h"

#include "boomerang/frontend/Frontend.h"

#include <QtWidgets>
#include <QtCore>
#include <QThread>
#include <QString>
#include <QTableWidget>
#include <sstream>


Qt::HANDLE threadToCollect = 0;


void DecompilerThread::run()
{
    threadToCollect = QThread::currentThreadId();

    Boomerang::get()->setDataDirectory(qApp->applicationDirPath() + "/../lib/boomerang/");
    Boomerang::get()->setOutputDirectory("output");
    // Boomerang::get()->vFlag = true;
    // Boomerang::get()->traceDecoder = true;

    m_parent = new Decompiler();
    m_parent->moveToThread(this);

    Boomerang::get()->addWatcher(m_parent);

    this->setPriority(QThread::LowPriority);
    this->exec();
}


Decompiler *DecompilerThread::getDecompiler()
{
    while (m_parent == nullptr) {
        msleep(10);
    }

    return m_parent;
}


void Decompiler::setUseDFTA(bool d)
{
    Boomerang::get()->dfaTypeAnalysis = d;
}


void Decompiler::setNoDecodeChildren(bool d)
{
    Boomerang::get()->noDecodeChildren = d;
}


void Decompiler::addEntryPoint(Address a, const char *nam)
{
    m_userEntrypoints.push_back(a);
    m_fe->addSymbol(a, nam);
}


void Decompiler::removeEntryPoint(Address a)
{
    for (std::vector<Address>::iterator it = m_userEntrypoints.begin(); it != m_userEntrypoints.end(); it++) {
        if (*it == a) {
            m_userEntrypoints.erase(it);
            break;
        }
    }
}


void Decompiler::changeInputFile(const QString& f)
{
    m_filename = f;
}


void Decompiler::changeOutputPath(const QString& path)
{
    Boomerang::get()->setOutputDirectory(path);
}


void Decompiler::load()
{
    emit loading();

    m_image = Boomerang::get()->getImage();
    m_prog  = new Prog(m_filename);
    m_fe    = IFrontEnd::create(m_filename, m_prog);

    if (m_fe == NULL) {
        emit machineType(QString("Unavailable: Load Failed!"));
        return;
    }

    m_prog->setFrontEnd(m_fe);
    m_fe->readLibraryCatalog();

    switch (m_prog->getMachine())
    {
    case Machine::PENTIUM:
        emit machineType(QString("pentium"));
        break;

    case Machine::SPARC:
        emit machineType(QString("sparc"));
        break;

    case Machine::HPRISC:
        emit machineType(QString("hprisc"));
        break;

    case Machine::PALM:
        emit machineType(QString("palm"));
        break;

    case Machine::PPC:
        emit machineType(QString("ppc"));
        break;

    case Machine::ST20:
        emit machineType(QString("st20"));
        break;

    case Machine::MIPS:
        emit machineType(QString("mips"));
        break;

    case Machine::M68K:
        emit machineType(QString("m68k"));
        break;

    case Machine::UNKNOWN:
        emit machineType(QString("UNKNOWN"));
        break;
    }

    QStringList          entrypointStrings;
    std::vector<Address> entrypoints = m_fe->getEntryPoints();

    for (Address entryPoint : entrypoints) {
        m_userEntrypoints.push_back(entryPoint);
        emit newEntrypoint(entryPoint, m_prog->getSymbolByAddress(entryPoint));
    }

    for (const IBinarySection *section : *m_image) {
        emit newSection(section->getName(), section->getSourceAddr(),
                        section->getSourceAddr() + section->getSize());
    }

    emit loadCompleted();
}


void Decompiler::decode()
{
    emit decoding();


    LOG_MSG("Decoding program %1...", m_prog->getName());

    bool    gotMain;
    Address a = m_fe->getMainEntryPoint(gotMain);

    for (unsigned int i = 0; i < m_userEntrypoints.size(); i++) {
        if (m_userEntrypoints[i] == a) {
            m_fe->decode(m_prog, true, NULL);
            break;
        }
    }

    for (unsigned int i = 0; i < m_userEntrypoints.size(); i++) {
        m_prog->decodeEntryPoint(m_userEntrypoints[i]);
    }

    if (!Boomerang::get()->noDecodeChildren) {
        // decode anything undecoded
        m_fe->decode(m_prog, Address::INVALID);
    }

    LOG_MSG("Decoding finished!");
    m_prog->finishDecode();

    emit decodeCompleted();
}


void Decompiler::decompile()
{
    emit decompiling();

    LOG_MSG("Starting decompile...");
    m_prog->decompile();
    LOG_MSG("Decompile finished!");

    emit decompileCompleted();
}


void Decompiler::emitClusterAndChildren(Module *root)
{
    emit newCluster(root->getName());

    for (size_t i = 0; i < root->getNumChildren(); i++) {
        emitClusterAndChildren(root->getChild(i));
    }
}


void Decompiler::generateCode()
{
    emit generatingCode();

    LOG_MSG("Generating code...");
    m_prog->generateCode();

    Module *root = m_prog->getRootCluster();

    if (root) {
        emitClusterAndChildren(root);
    }

    std::list<Function *>::iterator it;

    for (Module *module : m_prog->getModuleList()) {
        for (Function *p : *module) {
            if (p->isLib()) {
                continue;
            }

            emit newProcInCluster(p->getName(), module->getName());
        }
    }

    LOG_MSG("Generating code completed!");
    emit generateCodeCompleted();
}


const char *Decompiler::getProcStatus(UserProc *p)
{
    switch (p->getStatus())
    {
    case PROC_UNDECODED:
        return "undecoded";

    case PROC_DECODED:
        return "decoded";

    case PROC_SORTED:
        return "sorted";

    case PROC_VISITED:
        return "visited";

    case PROC_INCYCLE:
        return "in cycle";

    case PROC_PRESERVEDS:
        return "preserveds";

    case PROC_EARLYDONE:
        return "early done";

    case PROC_FINAL:
        return "final";

    case PROC_CODE_GENERATED:
        return "code generated";
    }

    return "unknown";
}


void Decompiler::alertConsidering(Function *parent, Function *p)
{
    emit consideringProc(parent ? parent->getName() : "", p->getName());
}


void Decompiler::alertDecompiling(UserProc *p)
{
    emit decompilingProc(p->getName());
}


void Decompiler::alertNew(Function *p)
{
    if (p->isLib()) {
        QString params;

        if ((p->getSignature() == NULL) || p->getSignature()->isUnknown()) {
            params = "<unknown>";
        }
        else {
            for (size_t i = 0; i < p->getSignature()->getNumParams(); i++) {
                auto ty = p->getSignature()->getParamType(i);
                params.append(ty->getCtype());
                params.append(" ");
                params.append(p->getSignature()->getParamName(i));

                if (i != p->getSignature()->getNumParams() - 1) {
                    params.append(", ");
                }
            }
        }

        emit newLibProc(p->getName(), params);
    }
    else {
        emit newUserProc(p->getName(), p->getNativeAddress());
    }
}


void Decompiler::alertRemove(Function *p)
{
    if (p->isLib()) {
        emit removeLibProc(p->getName());
    }
    else {
        emit removeUserProc(p->getName(), p->getNativeAddress());
    }
}


void Decompiler::alertUpdateSignature(Function *p)
{
    alertNew(p);
}


bool Decompiler::getRtlForProc(const QString& name, QString& rtl)
{
    Function *p = m_prog->findProc(name);

    if (p->isLib()) {
        return false;
    }

    UserProc    *up = (UserProc *)p;
    QTextStream os(&rtl);
    up->print(os, true);
    return true;
}


void Decompiler::alertDecompileDebugPoint(UserProc *p, const char *description)
{
    LOG_VERBOSE("%1: %2", p->getName(), description);

    if (m_debugging) {
        m_waiting = true;
        emit debuggingPoint(p->getName(), description);

        while (m_waiting) {
            thread()->wait(10);
        }
    }
}


void Decompiler::stopWaiting()
{
    m_waiting = false;
}


QString Decompiler::getSigFile(const QString& name)
{
    Function *p = m_prog->findProc(name);

    if ((p == nullptr) || !p->isLib() || (p->getSignature() == nullptr)) {
        return "";
    }

    return p->getSignature()->getSigFile();
}


QString Decompiler::getClusterFile(const QString& name)
{
    Module *c = m_prog->findModule(name);

    if (c == NULL) {
        return "";
    }

    return c->getOutPath("c");
}


void Decompiler::rereadLibSignatures()
{
    m_prog->rereadLibSignatures();
}


void Decompiler::renameProc(const QString& oldName, const QString& newName)
{
    Function *p = m_prog->findProc(oldName);

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
        tbl->setItem(tbl->rowCount() - 1, 3, new QTableWidgetItem(tr("%1").arg(c->getType(i)->getSize())));
    }
}
