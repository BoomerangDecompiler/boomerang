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


/// Unary is a non-terminal expression holding a single subexpression.
class BOOMERANG_API Unary : public Exp
{
public:
    Unary(OPER op, SharedExp subExp1);
    Unary(const Unary &other);
    Unary(Unary &&other) = default;

    ~Unary() override;

    Unary &operator=(const Unary &other) = default;
    Unary &operator=(Unary &&other) = default;

public:
    /// \copydoc Exp::clone
    SharedExp clone() const override;

    /// \copydoc Exp::get
    static SharedExp get(OPER op, SharedExp e1);

    /// \copydoc Exp::operator==
    bool operator==(const Exp &o) const override;

    /// \copydoc Exp::operator<
    bool operator<(const Exp &o) const override;

    /// \copydoc Exp::equalNoSubscript
    bool equalNoSubscript(const Exp &o) const override;

    /// \copydoc Exp::getArity
    int getArity() const override { return 1; }

    /// \copydoc Exp::doSearchChildren
    void doSearchChildren(const Exp &search, std::list<SharedExp *> &li, bool once) override;

    /// \copydoc Exp::getSubExp1
    SharedExp getSubExp1() override;

    /// \copydoc Exp::getSubExp1
    SharedConstExp getSubExp1() const override;

    /// \copydoc Exp::setSubExp1
    void setSubExp1(SharedExp e) override;

    /// \copydoc Exp::refSubExp1
    SharedExp &refSubExp1() override;

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

    /// \copydoc Exp::acceptChildModifier
    SharedExp acceptChildModifier(ExpModifier *mod) override;

    /// \copydoc Exp::acceptPostModifier
    SharedExp acceptPostModifier(ExpModifier *mod) override;

protected:
    SharedExp m_subExp1; ///< One subexpression pointer
};
