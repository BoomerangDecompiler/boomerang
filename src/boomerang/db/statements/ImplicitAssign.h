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
 * An implicit assignment has only a left hand side. It is a placeholder
 * for storing the types of parameters and globals.
 * That way, you can always find the type of a subscripted variable
 * by looking in its defining Assignment.
 */
class ImplicitAssign : public Assignment
{
public:
    /// Constructor and subexpression
    ImplicitAssign(SharedExp lhs);

    /// Constructor, type, and subexpression
    ImplicitAssign(SharedType ty, SharedExp lhs);
    ImplicitAssign(ImplicitAssign& o);

    // The first virtual function (here the destructor) can't be in statement.h file for gcc
    virtual ~ImplicitAssign();

    virtual Statement *clone() const override;

    /// Data flow based type analysis
    void dfaTypeAnalysis(bool& ch) override;

    // general search
    virtual bool search(const Exp& search, SharedExp& result) const override;
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    // general search and replace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    virtual void printCompact(QTextStream& os, bool html = false) const override;

    // Statement and Assignment functions
    virtual SharedExp getRight() const override { return nullptr; }
    virtual void simplify() override {}

    // Visitation
    // visit this Statement
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;
};
