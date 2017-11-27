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
 * Unary is a subclass of Exp, holding one subexpression
 */
class Unary : public Exp
{
public:
    Unary(OPER op, SharedExp subExp1);
    Unary(const Unary& o);
    virtual ~Unary() override;

    /// \copydoc Exp::clone
    virtual SharedExp clone() const override;

    /// \copydoc Exp::get
    static SharedExp get(OPER op, SharedExp e1) { return std::make_shared<Unary>(op, e1); }

    /// \copydoc Exp::operator==
    virtual bool operator==(const Exp& o) const override;

    /// \copydoc Exp::operator<
    virtual bool operator<(const Exp& o) const override;

    /// \copydoc Exp::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Exp::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Exp::printx
    virtual void printx(int ind) const override;

    /// \copydoc Exp::appendDotFile
    virtual void appendDotFile(QTextStream& of) override;

    /// \copydoc Exp::getArity
    virtual int getArity() const override { return 1; }

    /// \copydoc Exp::doSearchChildren
    void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

    /// \copydoc Exp::getSubExp1
    SharedExp getSubExp1() override;

    /// \copydoc Exp::getSubExp1
    SharedConstExp getSubExp1() const override;

    /// \copydoc Exp::setSubExp1
    void setSubExp1(SharedExp e) override;

    /// \copydoc Exp::refSubExp1
    SharedExp& refSubExp1() override;

    /// \copydoc Exp::polySimplify
    virtual SharedExp polySimplify(bool& bMod) override;

    /// \copydoc Exp::simplifyArith
    virtual SharedExp simplifyArith() override;

    /// \copydoc Exp::simplifyAddr
    virtual SharedExp simplifyAddr() override;

    /// \copydoc Exp::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Exp::accept
    virtual SharedExp accept(ExpModifier *v) override;

    /// \copydoc Exp::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Exp::descendType
    virtual void descendType(SharedType parentType, bool& ch, Statement *s) override;

protected:
    SharedExp subExp1; ///< One subexpression pointer
};
