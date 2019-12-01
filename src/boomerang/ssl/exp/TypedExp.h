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


/// Holds one subexpression and the type of this subexpression.
class BOOMERANG_API TypedExp : public Unary
{
public:
    TypedExp(SharedExp e1);
    TypedExp(SharedType ty, SharedExp e1);
    TypedExp(const TypedExp &other);
    TypedExp(TypedExp &&other) = default;

    ~TypedExp() override = default;

    TypedExp &operator=(const TypedExp &other) = default;
    TypedExp &operator=(TypedExp &&other) = default;

public:
    static std::shared_ptr<TypedExp> get(SharedExp exp);
    static std::shared_ptr<TypedExp> get(SharedType ty, SharedExp exp);

    /// \copydoc Unary::clone
    SharedExp clone() const override;

    /// \copydoc Unary::operator==
    bool operator==(const Exp &o) const override;

    /// \copydoc Unary::operator<
    bool operator<(const Exp &o) const override;

    /// \copydoc Unary::equalNoSubscript
    bool equalNoSubscript(const Exp &o) const override;

    /// Get and set the type
    SharedType getType() { return m_type; }
    SharedConstType getType() const { return m_type; }

    void setType(SharedType ty) { m_type = ty; }

    /// \copydoc Unary::ascendType
    SharedType ascendType() override;

    /// \copydoc Unary::descendType
    bool descendType(SharedType newType) override;

public:
    /// \copydoc Unary::acceptVisitor
    bool acceptVisitor(ExpVisitor *v) override;

protected:
    /// \copydoc Unary::acceptPreModifier
    SharedExp acceptPreModifier(ExpModifier *mod, bool &visitChildren) override;

    /// \copydoc Unary::acceptPostModifier
    SharedExp acceptPostModifier(ExpModifier *mod) override;

private:
    SharedType m_type;
};
