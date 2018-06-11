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


#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/FlagDef.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/RTL.h"

#include <QFile>
#include <QString>
#include <QTextStream>


bool ExpDotWriter::writeDotFile(const std::shared_ptr<Exp>& exp, const QString& filename)
{
    QFile dotFile(filename);

    if (!dotFile.open(QFile::WriteOnly)) {
        LOG_ERROR("Could not open %1 to write dotty file", filename);
        return false;
    }

    QTextStream os(&dotFile);
    m_os = &os;

    *m_os << "digraph Exp {\n";
    exp->accept(this);
    *m_os << "}";

    dotFile.flush();
    dotFile.close();
    m_os = nullptr;

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Unary>& exp)
{
    // First a node for this Unary object
    *m_os << "e_" << HostAddress(exp.get()).toString() << " [shape=record,label=\"{";
    // The (int) cast is to print the address, not the expression!
    *m_os << operToString(exp->getOper()) << "\\n" << " | ";
    *m_os << "<p1>";
    *m_os << " }\"];\n";

    // Finally an edge for the subexpression
    *m_os << "e_" << HostAddress(exp.get()) << "->e_" << HostAddress(exp->getSubExp1().get()) << ";\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Binary>& exp)
{
    // First a node for this Binary object
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << operToString(exp->getOper()) << "\\n" << " | ";
    *m_os << "{<p1> | <p2>}";
    *m_os << " }\"];\n";

    // Now an edge for each subexpression
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get()) << ";\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p2->e_" << HostAddress(exp->getSubExp2().get()) << ";\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Ternary>& exp)
{
    // First a node for this Ternary object
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << operToString(exp->getOper()) << "\\n" << " | ";
    *m_os << "{<p1> | <p2> | <p3>}";
    *m_os << " }\"];\n";

    // Now an edge for each subexpression
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get()) << ";\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p2->e_" << HostAddress(exp->getSubExp2().get()) << ";\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p3->e_" << HostAddress(exp->getSubExp3().get()) << ";\n";

    return true;
}


bool ExpDotWriter::visit(const std::shared_ptr<Const>& exp)
{
    // We define a unique name for each node as "e_0x123456" if the address of "this" == 0x123456
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << operToString(exp->getOper()) << "\\n" << " | ";

    switch (exp->getOper())
    {
            // Might want to distinguish this better, e.g. "(func*)myProc"
        case opFuncConst:   *m_os << exp->getFuncName() << "()";        break;
        case opIntConst:    *m_os << exp->getInt();                     break;
        case opFltConst:    *m_os << exp->getFlt();                     break;
        case opStrConst:    *m_os << "\\\"" << exp->getStr() << "\\\""; break;
        default:            break;
    }

    *m_os << " }\"];\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<FlagDef>& exp)
{
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << "opFlagDef \\n" << HostAddress(exp.get()) << "| ";
    // Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
    *m_os << "{ RTL ";

    const size_t n = exp->getRTL()->size();
    for (size_t i = 0; i < n; i++) {
        *m_os << "| <r" << i << "> ";
    }

    *m_os << "} | <p1> }\"];\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get()) << ";\n";

    return true;
}


bool ExpDotWriter::visit(const std::shared_ptr<Terminal>& exp)
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


bool ExpDotWriter::postVisit(const std::shared_ptr<TypedExp>& exp)
{
    *m_os << "e_" << HostAddress(exp.get()) << " [shape=record,label=\"{";
    *m_os << "opTypedExp\\n" << HostAddress(exp.get()) << " | ";
    // Just display the C type for now
    *m_os << exp->getType()->getCtype() << " | <p1>";
    *m_os << " }\"];\n";
    *m_os << "e_" << HostAddress(exp.get()) << ":p1->e_" << HostAddress(exp->getSubExp1().get()) << ";\n";

    return true;
}


bool ExpDotWriter::postVisit(const std::shared_ptr<RefExp>& exp)
{
    return postVisit(std::static_pointer_cast<Unary>(exp));
}


bool ExpDotWriter::postVisit(const std::shared_ptr<Location>& exp)
{
    return postVisit(std::static_pointer_cast<Unary>(exp));
}
