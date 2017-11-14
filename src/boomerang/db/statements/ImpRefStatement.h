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


#include "boomerang/db/statements/TypingStatement.h"


/**
 * An implicit reference has only an expression.
 * It holds the type information that results
 * from taking the address of a location.
 * Note that dataflow can't decide which local variable
 * (in the decompiled output) is being taken,
 * if there is more than one local variable
 * sharing the same memory address (separated then by type).
 */
class ImpRefStatement : public TypingStatement
{
public:
    ImpRefStatement(SharedType ty, SharedExp a);

    /// \copydoc Statement::clone
    virtual Statement *clone() const override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtPartModifier *modifier) override;

    /// \copydoc Statement::isDefinition
    virtual bool isDefinition() const override { return false; }

    /// \copydoc Statement::usesExp
    virtual bool usesExp(const Exp&) const override { return false; }

    /// \copydoc Statement::search
    virtual bool search(const Exp&, SharedExp&) const override;

    /// \copydoc Statement::searchAll
    virtual bool searchAll(const Exp&, std::list<SharedExp, std::allocator<SharedExp> >&) const override;

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAndReplace(const Exp&, SharedExp, bool cc = false) override;

    /// \copydoc Statement::generateCode
    virtual void generateCode(ICodeGenerator *, const BasicBlock *)  override {}

    /// \copydoc Statement::simplify
    virtual void simplify() override;

    /// \copydoc Statement::print
    /// \note ImpRefStatement not yet used
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \returns the address expression of the (implicitly) referenced location.
    SharedExp getAddressExp() const { return m_addressExp; }

    /// \returns the type of this expression
    SharedType getType() const { return m_type; }

    /// Meet the internal type with ty. Set ch if a change
    void meetWith(SharedType ty, bool& ch);

private:
    SharedExp m_addressExp; ///< The expression representing the address of the location referenced
};
