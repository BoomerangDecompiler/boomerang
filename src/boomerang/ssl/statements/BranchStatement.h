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


#include "boomerang/ssl/statements/GotoStatement.h"


/**
 * BranchStatement has a condition Exp in addition to the destination of the jump.
 */
class BOOMERANG_API BranchStatement : public GotoStatement
{
public:
    BranchStatement();
    BranchStatement(const BranchStatement &other) = default;
    BranchStatement(BranchStatement &&other)      = default;

    virtual ~BranchStatement() override;

    BranchStatement &operator=(const BranchStatement &other) = default;
    BranchStatement &operator=(BranchStatement &&other) = default;

public:
    /// \copydoc GotoStatement::clone
    virtual SharedStmt clone() const override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtVisitor *visitor) const override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtPartModifier *modifier) override;

    // Set and return the BRANCH_TYPE of this jcond as well as whether the
    // floating point condition codes are used.

    /**
     * Sets the type of conditional jump.
     * \param cond      The type of conditional jump
     * \param usesFloat true if this condional jump checks the floating point condition codes
     */
    void setCondType(BranchType cond, bool usesFloat = false);

    /// Return the SemStr expression containing the HL condition.
    /// \returns ptr to an expression
    SharedExp getCondExpr() const;

    /// Set the SemStr expression containing the HL condition.
    /// \param pe Pointer to Exp to set
    void setCondExpr(SharedExp pe);

    /// \returns the destination BB of a taken conditional jump
    BasicBlock *getTakenBB() const;

    /// \returns the destination BB of the fallthrough branch of a conditional jump
    BasicBlock *getFallBB() const;

    void setTakenBB(BasicBlock *bb);
    void setFallBB(BasicBlock *bb);

    /// \copydoc GotoStatement::print
    virtual void print(OStream &os) const override;

    /// \copydoc GotoStatement::search
    virtual bool search(const Exp &search, SharedExp &result) const override;

    /// \copydoc GotoStatement::searchAndReplace
    virtual bool searchAndReplace(const Exp &search, SharedExp replace, bool cc = false) override;

    /// \copydoc GotoStatement::searchAll
    virtual bool searchAll(const Exp &search, std::list<SharedExp> &result) const override;

    /// \copydoc GotoStatement::simplify
    virtual void simplify() override;

private:
    BranchType m_jumpType; ///< The condition for jumping
    SharedExp m_cond;      ///< The Exp representation of the high level condition: e.g., r[8] == 5
    bool m_isFloat;        ///< True if uses floating point CC
    // jtCond seems to be mainly needed for the Pentium weirdness.
    // Perhaps m_isFloat, m_jumpType, and m_size could one day be merged into a type
};
