#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpDotWriter.h"

#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/log/Log.h"

#include <QFile>
#include <QString>


bool ExpDotWriter::writeDotFile(const std::shared_ptr<Exp> &exp, const QString &filename)
{
    QFile dotFile(filename);

    if (!dotFile.open(QFile::WriteOnly)) {
        LOG_ERROR("Could not open %1 to write dotty file", filename);
        return false;
    }

    OStream os(&dotFile);
    m_os = &os;

    *m_os << "digraph Exp {\n";
    exp->acceptVisitor(this);
    *m_os << "}";

    dotFile.flush();
    dotFile.close();
    m_os = nullptr;

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Unary> &exp)
{
    // First a node for this Unary object
    *m_os << "e_" << HostAddress(exp.get()).toString() << " [shape=record,label=\"{";
    // The (int) cast is to print the address, not the expression!
    *m_os << operToString(exp->getOper()) << "\\n"
          << " | ";
    *m_os << "<p1>";
    *m_os << " }\"];\n";

    // Finally an edge for the subexpression
    *m_os << "e_" << HostAddress(exp.get()) << "->e_" << HostAddress(exp->getSubExp1().get())
          << ";\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Binary> &exp)
{
    // First a node for this Binary object
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << operToString(exp->getOper()) << "\\n"
          << " | ";
    *m_os << "{<p1> | <p2>}";
    *m_os << " }\"];\n";

    // Now an edge for each subexpression
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get())
          << ";\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p2->e_" << HostAddress(exp->getSubExp2().get())
          << ";\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Ternary> &exp)
{
    // First a node for this Ternary object
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << operToString(exp->getOper()) << "\\n"
          << " | ";
    *m_os << "{<p1> | <p2> | <p3>}";
    *m_os << " }\"];\n";

    // Now an edge for each subexpression
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get())
          << ";\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p2->e_" << HostAddress(exp->getSubExp2().get())
          << ";\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p3->e_" << HostAddress(exp->getSubExp3().get())
          << ";\n";

    return true;
}


bool ExpDotWriter::visit(const std::shared_ptr<Const> &exp)
{
    // We define a unique name for each node as "e_0x123456" if the address of "this" == 0x123456
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << operToString(exp->getOper()) << "\\n"
          << " | ";

    switch (exp->getOper()) {
        // Might want to distinguish this better, e.g. "(func*)myProc"
    case opFuncConst: *m_os << exp->getFuncName() << "()"; break;
    case opIntConst: *m_os << exp->getInt(); break;
    case opFltConst: *m_os << exp->getFlt(); break;
    case opStrConst: *m_os << "\\\"" << exp->getStr() << "\\\""; break;
    default: break;
    }

    *m_os << " }\"];\n";

    return true;
}


bool ExpDotWriter::visit(const std::shared_ptr<Terminal> &exp)
{
    *m_os << "e_" << HostAddress(exp.get()).toString() << " [shape=parallelogram,label=\"";

    if (exp->getOper() == opWild) {
        // Note: value is -1, so can't index array
        *m_os << "WILD";
    }
    else {
        *m_os << operToString(exp->getOper());
    }

    *m_os << "\\n" << HostAddress(exp.get()).toString();
    *m_os << "\"];\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<TypedExp> &exp)
{
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << "opTypedExp\\n" << HostAddress(exp.get()) << " | ";
    // Just display the C type for now
    *m_os << exp->getType()->getCtype() << " | <p1>";
    *m_os << " }\"];\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get())
          << ";\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<RefExp> &exp)
{
    return postVisit(exp->access<Unary>());
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Location> &exp)
{
    return postVisit(exp->access<Unary>());
}
