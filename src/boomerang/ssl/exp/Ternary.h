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


#include "boomerang/ssl/exp/Binary.h"


/// Ternary is a non-terminal expression holding three subexpressions.
class BOOMERANG_API Ternary : public Binary
{
public:
    Ternary(OPER op, SharedExp e1, SharedExp e2, SharedExp e3);
    Ternary(const Ternary &other);
    Ternary(Ternary &&other) = default;

    virtual ~Ternary() override;

    Ternary &operator=(const Ternary &other) = default;
    Ternary &operator=(Ternary &&other) = default;

public:
    /// \copydoc Binary::clone
    virtual SharedExp clone() const override;

    static std::shared_ptr<Ternary> get(OPER op, SharedExp e1, SharedExp e2, SharedExp e3);

    /// \copydoc Binary::operator==
    bool operator==(const Exp &o) const override;

    /// \copydoc Binary::operator<
    bool operator<(const Exp &o) const override;

    /// \copydoc Binary::equalNoSubscript
    bool equalNoSubscript(const Exp &o) const override;

    /// \copydoc Binary::getArity
    int getArity() const override { return 3; }

    /// \copydoc Exp::setSubExp3
    void setSubExp3(SharedExp e) override;

    /// \copydoc Exp::getSubExp3
    SharedExp getSubExp3() override;

    /// \copydoc Exp::getSubExp3
    SharedConstExp getSubExp3() const override;

    /// \copydoc Exp::refSubExp3
    SharedExp &refSubExp3() override;

    /// \copydoc Binary::doSearchChildren
    void doSearchChildren(const Exp &search, std::list<SharedExp *> &li, bool once) override;

    /// \copydoc Binary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Binary::descendType
    virtual bool descendType(SharedType newType) override;

public:
    /// \copydoc Binary::acceptVisitor
    bool acceptVisitor(ExpVisitor *v) override;

protected:
    /// \copydoc Binary::acceptPreModifier
    virtual SharedExp acceptPreModifier(ExpModifier *mod, bool &visitChildren) override;

    /// \copydoc Binary::acceptChildModifier
    virtual SharedExp acceptChildModifier(ExpModifier *mod) override;

    /// \copydoc Binary::acceptPostModifier
    virtual SharedExp acceptPostModifier(ExpModifier *mod) override;

private:
    SharedExp m_subExp3; ///< Third subexpression pointer
};
