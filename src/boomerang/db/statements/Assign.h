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
 * Assign an ordinary assignment with left and right sides.
 */
class Assign : public Assignment
{
public:
    Assign();
    Assign(SharedExp lhs, SharedExp rhs, SharedExp guard = nullptr);
    Assign(SharedType ty, SharedExp lhs, SharedExp rhs, SharedExp guard = nullptr);
    Assign(Assign& o);
    virtual ~Assign() override = default;

    /// \copydoc Statement::clone
    virtual Statement *clone() const override;

    /// \copydoc Assignment::getRight
    virtual SharedExp getRight() const override { return m_rhs; }

    SharedExp& getRightRef() { return m_rhs; }
    const SharedExp& getRightRef() const { return m_rhs; }

    /// set the rhs to something new
    void setRight(SharedExp e) { m_rhs = e; }

    /// \copydoc Statement::accept
    virtual bool accept(StmtVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtPartModifier *modifier) override;

    /// \copydoc Assignment::printCompact
    virtual void printCompact(QTextStream& os, bool html = false) const override;

    /// Guard
    void setGuard(SharedExp g) { m_guard = g; }
    SharedExp getGuard() const { return m_guard; }
    inline bool isGuarded() const { return m_guard != nullptr; }

    /// \copydoc Assignment::usesExp
    virtual bool usesExp(const Exp& e) const override;

    /// \copydoc Assignment::isDefinition
    virtual bool isDefinition() const override { return true; }

    /// \copydoc Assignment::search
    virtual bool search(const Exp& search, SharedExp& result) const override;

    /// \copydoc Assignment::searchAll
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    /// \copydoc Assignment::searchAndReplace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    /// Get memory depth
    virtual int getMemDepth() const;

    /// \copydoc Assignment::generateCode
    virtual void generateCode(ICodeGenerator *gen, const BasicBlock *parentBB) override;

    /// \copydoc Assignment::simplify
    virtual void simplify() override;

    /// \copydoc Assignment::simplifyAddr
    virtual void simplifyAddr() override;

    /// \copydoc Statement::fixSuccessor
    virtual void fixSuccessor() override;

    /// \copydoc Assignment::genConstraints
    virtual void genConstraints(LocationSet& cons) override;

    /// \copydoc Assignment::dfaTypeAnalysis
    void dfaTypeAnalysis(bool& ch) override;

private:
    SharedExp m_rhs;
    SharedExp m_guard;
};
