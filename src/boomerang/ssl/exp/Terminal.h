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


#include "boomerang/ssl/exp/Exp.h"


/// Terminal holds special zero arity items
/// such as opFlags (abstract flags register)
/// These are always terminal expressions.
class BOOMERANG_API Terminal : public Exp
{
public:
    Terminal(OPER op);
    Terminal(const Terminal &other);
    Terminal(Terminal &&other) = default;

    ~Terminal() override = default;

    Terminal &operator=(const Terminal &) = default;
    Terminal &operator=(Terminal &&) = default;

public:
    /// \copydoc Exp::clone
    SharedExp clone() const override;

    /// \copydoc Exp::get
    static SharedExp get(OPER op);

    /// \copydoc Exp::operator==
    bool operator==(const Exp &o) const override;

    /// \copydoc Exp::operator<
    bool operator<(const Exp &o) const override;

    /// \copydoc Exp::equalNoSubscript
    bool equalNoSubscript(const Exp &o) const override;

    /// \copydoc Exp::ascendType
    SharedType ascendType() override;

    /// \copydoc Exp::descendType
    bool descendType(SharedType newType) override;

public:
    /// \copydoc Exp::acceptVisitor
    bool acceptVisitor(ExpVisitor *v) override;

protected:
    /// \copydoc Exp::acceptPreModifier
    SharedExp acceptPreModifier(ExpModifier *mod, bool &visitChildren) override;

    /// \copydoc Exp::acceptPostModifier
    SharedExp acceptPostModifier(ExpModifier *mod) override;
};
