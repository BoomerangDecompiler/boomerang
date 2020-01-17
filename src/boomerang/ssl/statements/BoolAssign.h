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
 * BoolAssign represents "setCC" type instructions, where some destination is set
 * (to 1 or 0) depending on the condition codes.
 * It has a condition Exp, similar to the BranchStatement class.
 */
class BOOMERANG_API BoolAssign : public Assignment
{
public:
    BoolAssign(SharedExp lhs, BranchType bt, SharedExp cond);
    BoolAssign(const BoolAssign &other);
    BoolAssign(BoolAssign &&other) = default;

    ~BoolAssign() override;

    BoolAssign &operator=(const BoolAssign &other);
    BoolAssign &operator=(BoolAssign &&other) = default;

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

    /**
     * Sets the BranchType of this jcond as well as the flag
     * indicating whether or not the floating point condition codes
     * are used.
     * \param cond      the type of branch
     * \param usesFloat - this condional jump checks the floating point condition codes
     */
    void setCondType(BranchType cond, bool usesFloat = false);

    BranchType getCond() const { return m_jumpType; }
    bool isFloat() const { return m_isFloat; }
    void setFloat(bool b) { m_isFloat = b; }

    // Set and return the Exp representing the HL condition

    /// \returns the Exp expression containing the HL condition.
    SharedExp getCondExpr() const;

    /**
     * Set the Exp expression containing the HL condition.
     * \param pss Pointer to semantic string to set
     */
    void setCondExpr(SharedExp pss);

    /// \copydoc Assignment::printCompact
    void printCompact(OStream &os) const override;

    /// \copydoc Statement::simplify
    void simplify() override;

    /// \copydoc Statement::getDefinitions
    void getDefinitions(LocationSet &def, bool assumeABICompliance) const override;

    /// \copydoc Assignment::getRight
    SharedExp getRight() const override;

    /// \copydoc Statement::search
    bool search(const Exp &search, SharedExp &result) const override;

    /// \copydoc Statement::searchAll
    bool searchAll(const Exp &search, std::list<SharedExp> &result) const override;

    /// \copydoc Statement::searchAndReplace
    bool searchAndReplace(const Exp &search, SharedExp replace, bool cc = false) override;

private:
    BranchType m_jumpType = BranchType::INVALID; ///< the condition for setting true
    SharedExp m_cond; ///< Exp representation of the high level condition: e.g. r[8] == 5
    bool m_isFloat;   ///< True if condition uses floating point CC
};
