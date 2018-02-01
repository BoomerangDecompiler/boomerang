#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Proc.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/codegen/syntax/SyntaxNode.h"
#include "boomerang/db/Module.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/ImpRefStatement.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Util.h"

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <sstream>
#include <algorithm> // For find()
#include <cstring>


#ifdef _WIN32
#  include <windows.h>
#  ifndef __MINGW32__
namespace dbghelp
{
#    include <dbghelp.h>
}
#  endif
#endif

typedef std::map<Statement *, int> RefCounter;


Function::Function(Address entryAddr, Signature *sig, Module *module)
    : m_signature(sig)
    , m_entryAddress(entryAddr)
    , m_firstCaller(nullptr)
    , m_module(module)
{
    if (module) {
        m_prog = module->getProg();
    }
}


Function::~Function()
{
}


void Function::eraseFromParent()
{
    // Replace the entry in the procedure map with -1 as a warning not to decode that address ever again
    m_module->setLocationMap(getEntryAddress(), reinterpret_cast<Function *>(-1));

    // Delete the cfg etc.
    m_module->getFunctionList().remove(this);

    deleteCFG();
    delete this;
}


QString Function::getName() const
{
    assert(m_signature);
    return m_signature->getName();
}


void Function::setName(const QString& nam)
{
    assert(m_signature);
    m_signature->setName(nam);
}


Address Function::getEntryAddress() const
{
    return m_entryAddress;
}


void Function::setEntryAddress(Address a)
{
    m_entryAddress = a;
}


void Function::renameParam(const char *oldName, const char *newName)
{
    m_signature->renameParam(oldName, newName);
}


void Function::matchParams(std::list<SharedExp>& /*actuals*/, UserProc& /*caller*/)
{
    // TODO: not implemented, not used, but large amount of docs :)
}


std::list<Type> *Function::getParamTypeList(const std::list<SharedExp>& /*actuals*/)
{
    // TODO: not implemented, not used
    return nullptr;
}


void Function::printCallGraphXML(QTextStream& os, int depth, bool /*recurse*/)
{
    if (!DUMP_XML) {
        return;
    }

    m_visited = true;

    for (int i = 0; i < depth; i++) {
        os << "      ";
    }

    os << "<proc name=\"" << getName() << "\"/>\n";
}


void Function::printDetailsXML()
{
    if (!DUMP_XML) {
        return;
    }

    QFile file(Boomerang::get()->getSettings()->getOutputDirectory().absoluteFilePath(getName() + "-details.xml"));

    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "Can't write to file:" << file.fileName();
        return;
    }

    QTextStream out(&file);
    out << "<proc name=\"" << getName() << "\">\n";
    unsigned i;

    for (i = 0; i < m_signature->getNumParams(); i++) {
        out << "   <param name=\"" << m_signature->getParamName(i) << "\" "
            << "exp=\"" << m_signature->getParamExp(i) << "\" "
            << "type=\"" << m_signature->getParamType(i)->getCtype() << "\"\n";
    }

    for (i = 0; i < m_signature->getNumReturns(); i++) {
        out << "   <return exp=\"" << m_signature->getReturnExp(i) << "\" "
            << "type=\"" << m_signature->getReturnType(i)->getCtype() << "\"/>\n";
    }

    out << "</proc>\n";
}


void Function::removeFromModule()
{
    assert(m_module);
    m_module->getFunctionList().remove(this);
    m_module->setLocationMap(m_entryAddress, nullptr);
}


void Function::setParent(Module *c)
{
    if (c == m_module) {
        return;
    }

    removeFromModule();
    m_module = c;
    c->getFunctionList().push_back(this);
    c->setLocationMap(m_entryAddress, this);
}


Function *Function::getFirstCaller()
{
    if ((m_firstCaller == nullptr) && (m_firstCallerAddr != Address::INVALID)) {
        m_firstCaller     = m_prog->findFunction(m_firstCallerAddr);
        m_firstCallerAddr = Address::INVALID;
    }

    return m_firstCaller;
}


void Function::removeParameter(SharedExp e)
{
    int n = m_signature->findParam(e);

    if (n != -1) {
        m_signature->removeParameter(n);

        for (auto const& elem : m_callerSet) {
            if (DEBUG_UNUSED) {
                LOG_MSG("Removing argument %1 in pos %2 from %3", e, n, elem);
            }

            (elem)->removeArgument(n);
        }
    }
}


void Function::addCallers(std::set<UserProc *>& callers)
{
    std::set<CallStatement *>::iterator it;

    for (it = m_callerSet.begin(); it != m_callerSet.end(); it++) {
        UserProc *callerProc = (*it)->getProc();
        callers.insert(callerProc);
    }
}


void Function::setProvenTrue(SharedExp fact)
{
    assert(fact->isEquality());
    SharedExp lhs = fact->getSubExp1();
    SharedExp rhs = fact->getSubExp2();
    m_provenTrue[lhs] = rhs;
}
