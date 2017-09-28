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
    virtual ~JunctionStatement();

    virtual Statement *clone() const override { return new JunctionStatement(); }

    // Accept a visitor (of various kinds) to this Statement. Return true to continue visiting
    bool accept(StmtVisitor *visitor) override;
    bool accept(StmtExpVisitor *visitor) override;
    bool accept(StmtModifier *visitor) override;
    bool accept(StmtPartModifier *visitor) override;

    // returns true if this statement defines anything
    bool isDefinition() const override { return false; }

    bool usesExp(const Exp&) const override { return false; }

    void print(QTextStream& os, bool html = false) const override;

    // general search
    bool search(const Exp& /*search*/, SharedExp& /*result*/) const override { return false; }
    bool searchAll(const Exp& /*search*/, std::list<SharedExp>& /*result*/) const override { return false; }

    /// general search and replace. Set cc true to change collectors as well. Return true if any change
    bool searchAndReplace(const Exp& /*search*/, SharedExp /*replace*/, bool /*cc*/ = false)  override { return false; }

    void generateCode(ICodeGenerator * /*hll*/, BasicBlock * /*pbb*/)  override {}

    // simpify internal expressions
    void simplify() override {}

    bool isLoopJunction() const;
};
