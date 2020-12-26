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


#include "boomerang/ssl/statements/Assignment.h"


/**
 * An implicit assignment has only a left hand side. It is a placeholder
 * for storing the types of parameters and globals.
 * That way, you can always find the type of a subscripted variable
 * by looking in its defining Assignment.
 */
class BOOMERANG_API ImplicitAssign : public Assignment
{
public:
    ImplicitAssign(SharedExp lhs);
    ImplicitAssign(SharedType ty, SharedExp lhs);
    ImplicitAssign(const ImplicitAssign &other);
    ImplicitAssign(ImplicitAssign &&other) = default;

    ~ImplicitAssign() override = default;

    ImplicitAssign &operator=(const ImplicitAssign &other) = default;
    ImplicitAssign &operator=(ImplicitAssign &&other) = default;

public:
    /// \copydoc Statement::clone
    SharedStmt clone() const override;

    /// \copydoc Statement::search
    bool search(const Exp &search, SharedExp &result) const override;

    /// \copydoc Statement::searchAll
    bool searchAll(const Exp &search, std::list<SharedExp> &result) const override;

    /// \copydoc Statement::searchAndReplace
    bool searchAndReplace(const Exp &search, SharedExp replace, bool cc = false) override;

    /// \copydoc Statement::printCompact
    void printCompact(OStream &os) const override;

    /// \copydoc Assignment::getRight
    SharedExp getRight() const override { return nullptr; }

    /// \copydoc Statement::simplify
    void simplify() override { }

    /// \copydoc Statement::accept
    bool accept(StmtVisitor *visitor) const override;

    /// \copydoc Statement::accept
    bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    bool accept(StmtPartModifier *modifier) override;
};
