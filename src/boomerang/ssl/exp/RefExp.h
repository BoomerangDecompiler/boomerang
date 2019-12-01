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


#include "boomerang/ssl/exp/Unary.h"
#include "boomerang/ssl/statements/Statement.h"


/**
 * RefExp statements map uses of expressions back to their definitions.
 * RefExp is a subclass of Unary, holding an ordinary Exp pointer,
 * and a pointer to a defining statement (could be a phi assignment).
 * This is used for subscripting SSA variables. Example:
 *   m[1000]
 * becomes
 *   m[1000]{3}
 * if defined at statement 3.
 *
 * The integer is really a pointer to the defining statement,
 * printed as the statement number for compactness.
 * If the expression is not explicitly defined anywhere,
 * the defining statement is nullptr.
 */
class BOOMERANG_API RefExp : public Unary
{
public:
    /// \param usedExp Expression that is used
    /// \param definition Statment where the expression is defined. May be nullptr.
    RefExp(SharedExp usedExp, const SharedStmt &definition);
    RefExp(const RefExp &other) = default;
    RefExp(RefExp &&other)      = default;

    ~RefExp() override { m_def = nullptr; }

    RefExp &operator=(const RefExp &other) = default;
    RefExp &operator=(RefExp &&other) = default;

public:
    /// \copydoc Unary::clone
    SharedExp clone() const override;

    /// \copydoc Unary::get
    static std::shared_ptr<RefExp> get(SharedExp usedExp, const SharedStmt &definition);

    /// \copydoc Unary::operator==
    bool operator==(const Exp &o) const override;

    /// \copydoc Unary::operator<
    bool operator<(const Exp &o) const override;

    /// \copydoc Unary::equalNoSubscript
    bool equalNoSubscript(const Exp &o) const override;

    const SharedStmt &getDef() const { return m_def; }
    void setDef(const SharedStmt &def);

    SharedExp addSubscript(const SharedStmt &def);

    /// Before type analysis, implicit definitions are nullptr.
    /// During and after TA, they point to an implicit assignment statement.
    bool isImplicitDef() const;

    /// \copydoc Unary::ascendType
    SharedType ascendType() override;

    /// \copydoc Unary::descendType
    bool descendType(SharedType newType) override;

public:
    /// \copydoc Unary::acceptVisitor
    bool acceptVisitor(ExpVisitor *v) override;

private:
    /// \copydoc Unary::acceptPreModifier
    SharedExp acceptPreModifier(ExpModifier *mod, bool &visitChildren) override;

    /// \copydoc Unary::acceptPostModifier
    SharedExp acceptPostModifier(ExpModifier *mod) override;

private:
    SharedStmt m_def; ///< The defining statement
};
