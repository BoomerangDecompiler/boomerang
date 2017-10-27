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


#include "boomerang/db/statements/Statement.h"


class JunctionStatement : public Statement
{
public:
    JunctionStatement();
    virtual ~JunctionStatement() override;

    /// \copydoc Statement::clone
    virtual Statement *clone() const override { return new JunctionStatement(); }

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

    /// \copydoc Statement::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Statement::search
    virtual bool search(const Exp& /*search*/, SharedExp& /*result*/) const override { return false; }

    /// \copydoc Statement::searchAll
    virtual bool searchAll(const Exp& /*search*/, std::list<SharedExp>& /*result*/) const override { return false; }

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAndReplace(const Exp& /*search*/, SharedExp /*replace*/, bool /*cc*/ = false)  override { return false; }

    /// \copydoc Statement::generateCode
    virtual void generateCode(ICodeGenerator * /*hll*/, BasicBlock * /*pbb*/) override {}

    /// \copydoc Statement::simplify
    virtual void simplify() override {}

    bool isLoopJunction() const;
};
