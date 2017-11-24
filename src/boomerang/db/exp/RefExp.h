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


#include "boomerang/db/exp/Unary.h"


/**
 * RefExp statements map uses of expressions back to their definitions.
 * RefExp is a subclass of Unary, holding an ordinary Exp pointer,
 * and a pointer to a defining statement (could be a phi assignment).
 * This is used for subscripting SSA variables. Example:
 *   m[1000] becomes m[1000]{3} if defined at statement 3
 * The integer is really a pointer to the defining statement,
 * printed as the statement number for compactness.
 * If the expression is not explicitly defined anywhere,
 * the defining statement is nullptr.
 */
class RefExp : public Unary
{
public:
    /// \param usedExp Expression that is used
    /// \param definition Pointer to the statment where the expression is defined
    RefExp(SharedExp usedExp, Statement *definition);
    RefExp(const RefExp& other) = default;
    RefExp(RefExp&& other) = default;

    virtual ~RefExp() override { m_def = nullptr; }

    RefExp& operator=(const RefExp& other) = default;
    RefExp& operator=(RefExp&& other) = default;

public:
    /// \copydoc Unary::clone
    SharedExp clone() const override;

    /// \copydoc Unary::get
    static std::shared_ptr<RefExp> get(SharedExp usedExp, Statement *definition);

    /// \copydoc Unary::operator==
    bool operator==(const Exp& o) const override;

    /// \copydoc Unary::operator<
    bool operator<(const Exp& o) const override;

    /// \copydoc Unary::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Unary::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Unary::printx
    virtual void printx(int ind) const override;

    Statement *getDef() const { return m_def; } // Ugh was called getRef()
    void setDef(Statement *_def);

    SharedExp addSubscript(Statement *_def);

    bool references(const Statement *s) const { return m_def == s; }

    virtual SharedExp polySimplify(bool& bMod) override;

    /**
     * Before type analysis, implicit definitions are nullptr.
     * During and after TA, they point to an implicit assignment statement.
     */
    bool isImplicitDef() const;

    /// \copydoc Unary::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Unary::accept
    virtual SharedExp accept(ExpModifier *v) override;

    /// \copydoc Unary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Unary::descendType
    virtual void descendType(SharedType parentType, bool& ch, Statement *s) override;

private:
    Statement *m_def; ///< The defining statement
};
