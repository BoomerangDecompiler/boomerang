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
 * Terminal holds special zero arity items
 * such as opFlags (abstract flags register)
 */
class Terminal : public Exp
{
public:
    Terminal(OPER op);
    Terminal(const Terminal& other);
    Terminal(Terminal&& other) = default;

    virtual ~Terminal() override = default;

    Terminal& operator=(const Terminal&) = default;
    Terminal& operator=(Terminal&&) = default;

public:
    /// \copydoc Exp::clone
    virtual SharedExp clone() const override;

    /// \copydoc Exp::get
    static SharedExp get(OPER op) { return std::make_shared<Terminal>(op); }

    /// \copydoc Exp::operator==
    bool operator==(const Exp& o) const override;

    /// \copydoc Exp::operator<
    bool operator<(const Exp& o) const override;

    /// \copydoc Exp::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Exp::print
    void print(QTextStream& os, bool = false) const override;

    /// \copydoc Exp::printx
    void printx(int ind) const override;

    /// \copydoc Exp::appendDotFile
    void appendDotFile(QTextStream& of) override;

    /// \copydoc Exp::isTerminal
    bool isTerminal() const override { return true; }

    /// \copydoc Exp::accept
    bool accept(ExpVisitor *v) override;

    /// \copydoc Exp::accept
    SharedExp accept(ExpModifier *v) override;

    /// \copydoc Exp::ascendType
    SharedType ascendType() override;

    /// \copydoc Exp::descendType
    void descendType(SharedType parentType, bool& changed, Statement *s) override;
};
