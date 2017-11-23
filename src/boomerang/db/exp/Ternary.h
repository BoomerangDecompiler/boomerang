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


#include "boomerang/db/exp/Binary.h"


/**
 * Ternary is a subclass of Binary, holding three subexpressions
 */
class Ternary : public Binary
{
public:
    Ternary(OPER op, SharedExp e1, SharedExp e2, SharedExp e3);
    Ternary(const Ternary& o);
    virtual ~Ternary() override;

    /// \copydoc Binary::clone
    virtual SharedExp clone() const override;

    template<typename Ty, typename Arg1, typename Arg2, typename Arg3>
    static std::shared_ptr<Ternary> get(Ty ty, Arg1 arg1, Arg2 arg2, Arg3 arg3)
    { return std::make_shared<Ternary>(ty, arg1, arg2, arg3); }

    /// \copydoc Binary::operator==
    bool operator==(const Exp& o) const override;

    /// \copydoc Binary::operator<
    bool operator<(const Exp& o) const override;

    /// \copydoc Binary::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Binary::getArity
    int getArity() const override { return 3; }

    /// \copydoc Binary::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Binary::printr
    virtual void printr(QTextStream& os, bool = false) const override;

    /// \copydoc Binary::printx
    virtual void printx(int ind) const override;

    /// \copydoc Binary::appendDotFile
    virtual void appendDotFile(QTextStream& of) override;

    /// \copydoc Exp::setSubExp3
    void setSubExp3(SharedExp e) override;

    /// \copydoc Exp::getSubExp3
    SharedExp getSubExp3() override;

    /// \copydoc Exp::getSubExp3
    SharedConstExp getSubExp3() const override;

    /// \copydoc Exp::refSubExp3
    SharedExp& refSubExp3() override;

    /// \copydoc Binary::doSearchChildren
    void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

    /// \copydoc Binary::polySimplify
    SharedExp polySimplify(bool& bMod) override;

    /// \copydoc Binary::simplifyArith
    SharedExp simplifyArith() override;

    /// \copydoc Binary::simplifyAddr
    SharedExp simplifyAddr() override;

    /// \copydoc Binary::accept
    bool accept(ExpVisitor *v) override;

    /// \copydoc Binary::accept
    SharedExp accept(ExpModifier *v) override;

    /// \copydoc Binary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Binary::descendType
    virtual void descendType(SharedType parentType, bool& ch, Statement *s) override;

private:
    SharedExp subExp3; ///< Third subexpression pointer
};
