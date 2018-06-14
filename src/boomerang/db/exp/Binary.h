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
 * Binary is a subclass of Unary, holding two subexpressions
 */
class Binary : public Unary
{
public:
    Binary(OPER op, SharedExp e1, SharedExp e2);
    Binary(const Binary& other);
    Binary(Binary&& other) = default;

    virtual ~Binary() override;

    Binary& operator=(const Binary& other) = default;
    Binary& operator=(Binary&& other) = default;

public:
    /// \copydoc Unary::clone
    virtual SharedExp clone() const override;

    static std::shared_ptr<Binary> get(OPER op, SharedExp e1, SharedExp e2)
    { return std::make_shared<Binary>(op, e1, e2); }

    /// \copydoc Unary::operator==
    bool operator==(const Exp& o) const override;

    /// \copydoc Unary::operator<
    bool operator<(const Exp& o) const override;

    /// \copydoc Unary::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Unary::getArity
    int getArity() const override { return 2; }

    /// \copydoc Unary::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Unary::printr
    virtual void printr(QTextStream& os, bool html = false) const override;

    /// \copydoc Unary::printx
    virtual void printx(int ind) const override;

    /// \copydoc Exp::getSubExp2
    SharedExp getSubExp2() override;
    SharedConstExp getSubExp2() const override;

    /// \copydoc Exp::getSubExp2
    SharedExp& refSubExp2() override;

    /// \copydoc Exp::getSubExp2
    void setSubExp2(SharedExp e) override;

    /// Swap the two subexpressions
    /// \note Changes the meaning for non-commutative operations
    void commute();

    /// \copydoc Unary::doSearchChildren
    void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

    /// \copydoc Unary::simplify
    virtual SharedExp polySimplify(bool& changed) override;

    /// \copydoc Unary::simplifyArith
    SharedExp simplifyArith() override;

    /// \copydoc Unary::simplifyAddr
    SharedExp simplifyAddr() override;

    /// \copydoc Unary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Unary::ascendType
    virtual void descendType(SharedType parentType, bool& changed, Statement *s) override;

public:
    /// \copydoc Unary::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Unary::accept
    virtual SharedExp accept(ExpModifier *v) override;

protected:
    virtual SharedExp preAccept(ExpModifier *mod, bool& visitChildren) override;
    virtual SharedExp postAccept(ExpModifier *) override;

protected:
    SharedExp subExp2; ///< Second subexpression pointer
};
