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


#include "boomerang/db/statements/Assignment.h"


/**
 * An implicit assignment has only a left hand side. It is a placeholder
 * for storing the types of parameters and globals.
 * That way, you can always find the type of a subscripted variable
 * by looking in its defining Assignment.
 */
class ImplicitAssign : public Assignment
{
public:
    ImplicitAssign(SharedExp lhs);
    ImplicitAssign(SharedType ty, SharedExp lhs);
    ImplicitAssign(ImplicitAssign& o);

    virtual ~ImplicitAssign() override;

    /// \copydoc Statement::clone
    virtual Statement *clone() const override;

    /// \copydoc Statement::search
    virtual bool search(const Exp& search, SharedExp& result) const override;

    /// \copydoc Statement::searchAll
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    /// \copydoc Statement::printCompact
    virtual void printCompact(QTextStream& os, bool html = false) const override;

    /// \copydoc Assignment::getRight
    virtual SharedExp getRight() const override { return nullptr; }

    /// \copydoc Statement::simplify
    virtual void simplify() override {}

    /// \copydoc Statement::accept
    virtual bool accept(StmtVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtPartModifier *modifier) override;
};
