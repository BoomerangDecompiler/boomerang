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
 * An ordinary assignment with left and right hand sides.
 * Example: *i32* r25 := 5
 */
class BOOMERANG_API Assign : public Assignment
{
public:
    Assign();
    Assign(SharedExp lhs, SharedExp rhs, SharedExp guard = nullptr);
    Assign(SharedType ty, SharedExp lhs, SharedExp rhs, SharedExp guard = nullptr);

    Assign(const Assign &other);
    Assign(Assign &&other) = default;

    ~Assign() override = default;

    Assign &operator=(const Assign &other) = default;
    Assign &operator=(Assign &&other) = default;

public:
    /// \copydoc Statement::clone
    SharedStmt clone() const override;

    /// \copydoc Statement::accept
    bool accept(StmtVisitor *visitor) const override;

    /// \copydoc Statement::accept
    bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    bool accept(StmtPartModifier *modifier) override;

    /// \copydoc Assignment::printCompact
    void printCompact(OStream &os) const override;

    /// \copydoc Assignment::search
    bool search(const Exp &search, SharedExp &result) const override;

    /// \copydoc Assignment::searchAll
    bool searchAll(const Exp &search, std::list<SharedExp> &result) const override;

    /// \copydoc Assignment::searchAndReplace
    bool searchAndReplace(const Exp &search, SharedExp replace, bool cc = false) override;

    /// \copydoc Assignment::simplify
    void simplify() override;

    /// \copydoc Assignment::simplifyAddr
    void simplifyAddr() override;

    /// \copydoc Assignment::getRight
    SharedExp getRight() const override { return m_rhs; }

    SharedExp &getRightRef() { return m_rhs; }
    const SharedExp &getRightRef() const { return m_rhs; }

    /// set the rhs to something new
    void setRight(SharedExp e) { m_rhs = e; }

public:
    /// Guard
    void setGuard(SharedExp g) { m_guard = g; }
    SharedExp getGuard() const { return m_guard; }
    inline bool isGuarded() const { return m_guard != nullptr; }

private:
    SharedExp m_rhs;
    SharedExp m_guard;
};
