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


#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class QString;
class OStream;


class ExpDotWriter : private ExpVisitor
{
public:
    bool writeDotFile(const std::shared_ptr<class Exp> &exp, const QString &filename);

private:
    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<Unary> &exp) override;

    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<Binary> &exp) override;

    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<Ternary> &exp) override;

    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<TypedExp> &exp) override;

    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<RefExp> &exp) override;

    /// \copydoc ExpVisitor::postVisit
    bool postVisit(const std::shared_ptr<Location> &exp) override;

    /// \copydoc ExpVisitor::preVisit
    bool visit(const std::shared_ptr<Const> &exp) override;

    /// \copydoc ExpVisitor::visit
    bool visit(const std::shared_ptr<Terminal> &exp) override;

private:
    OStream *m_os;
};
