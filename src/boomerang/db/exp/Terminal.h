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


#include "boomerang/db/exp/Exp.h"


/**
 * Terminal is a subclass of Exp, and holds special zero arity items such as opFlags (abstract flags register)
 */
class Terminal : public Exp
{
public:
    // Constructors
    Terminal(OPER op);
    Terminal(const Terminal& o); ///< Copy constructor

    static SharedExp get(OPER op) { return std::make_shared<Terminal>(op); }

    // Clone
    SharedExp clone() const override;

    // Compare
    bool operator==(const Exp& o) const override;
    bool operator<(const Exp& o) const override;
    bool operator*=(const Exp& o) const override;
    void print(QTextStream& os, bool = false) const override;
    void appendDotFile(QTextStream& of) override;
    void printx(int ind) const override;

    bool isTerminal() const override { return true; }

    /// Visitation
    bool accept(ExpVisitor *v) override;
    SharedExp accept(ExpModifier *v) override;

    SharedType ascendType() override;
    void descendType(SharedType parentType, bool& ch, Statement *s) override;

    bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;
};
