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


/// Binary is an expression holding two subexpressions.
class BOOMERANG_API Binary : public Unary
{
public:
    Binary(OPER op, SharedExp e1, SharedExp e2);
    Binary(const Binary &other);
    Binary(Binary &&other) = default;

    virtual ~Binary() override;

    Binary &operator=(const Binary &other) = default;
    Binary &operator=(Binary &&other) = default;

public:
    /// \copydoc Unary::clone
    virtual SharedExp clone() const override;

    static std::shared_ptr<Binary> get(OPER op, SharedExp e1, SharedExp e2);

    /// \copydoc Unary::operator==
    bool operator==(const Exp &o) const override;

    /// \copydoc Unary::operator<
    bool operator<(const Exp &o) const override;

    /// \copydoc Unary::equalNoSubscript
    bool equalNoSubscript(const Exp &o) const override;

    /// \copydoc Unary::getArity
    int getArity() const override { return 2; }

    /// \copydoc Exp::getSubExp2
    SharedExp getSubExp2() override;
    SharedConstExp getSubExp2() const override;

    /// \copydoc Exp::getSubExp2
    SharedExp &refSubExp2() override;

    /// \copydoc Exp::getSubExp2
    void setSubExp2(SharedExp e) override;

    /// Swap the two subexpressions
    /// \note Changes the meaning for non-commutative operations
    void commute();

    /// \copydoc Unary::doSearchChildren
    void doSearchChildren(const Exp &search, std::list<SharedExp *> &li, bool once) override;

    /// \copydoc Unary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Unary::ascendType
    virtual bool descendType(SharedType newType) override;

public:
    /// \copydoc Unary::acceptVisitor
    virtual bool acceptVisitor(ExpVisitor *v) override;

protected:
    /// \copydoc Unary::acceptPreModifier
    virtual SharedExp acceptPreModifier(ExpModifier *mod, bool &visitChildren) override;

    /// \copydoc Unary::acceptChildModifier
    virtual SharedExp acceptChildModifier(ExpModifier *mod) override;

    /// \copydoc Unary::acceptPostModifier
    virtual SharedExp acceptPostModifier(ExpModifier *mod) override;

protected:
    SharedExp m_subExp2; ///< Second subexpression pointer
};
