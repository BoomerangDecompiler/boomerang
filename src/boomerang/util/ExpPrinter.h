#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/ssl/exp/Operator.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class Exp;
class OStream;


class ExpPrinter : private ExpVisitor
{
    friend OStream& operator<<(OStream& lhs, ExpPrinter&& rhs);

public:
    explicit ExpPrinter(Exp& exp, bool html = false);

private:
    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Unary>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Binary>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Ternary>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<TypedExp>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<RefExp>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<RefExp>& exp) override;

    /// \copydoc ExpVisitor::visit
    bool visit(const std::shared_ptr<Const>& exp) override;

    /// \copydoc ExpVisitor::visit
    bool visit(const std::shared_ptr<Terminal>& exp) override;

private:
    /**
     * Given an expression, determine if child expressions
     * need to be parenthesized or not.
     */
    bool childrenNeedParentheses(const std::shared_ptr<Exp>& exp) const;

    bool printr(const std::shared_ptr<Exp>& exp);

private:
    OStream *m_os;
    Exp& m_exp;
    bool m_html;
};

OStream& operator<<(OStream& lhs, ExpPrinter&& rhs);
