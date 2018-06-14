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
 * Holds one subexpression and the type of this subexpression.
 */
class TypedExp : public Unary
{
public:
    TypedExp(SharedExp e1);

    /// Constructor, type, and subexpression.
    /// A rare const parameter allows the common case of providing a temporary,
    /// e.g. foo = new TypedExp(Type(INTEGER), ...);
    TypedExp(SharedType ty, SharedExp e1);
    TypedExp(const TypedExp& other);
    TypedExp(TypedExp&& other) = default;

    virtual ~TypedExp() override = default;

    TypedExp& operator=(const TypedExp& other) = default;
    TypedExp& operator=(TypedExp&& other) = default;

public:
    /// \copydoc Unary::clone
    virtual SharedExp clone() const override;

    /// \copydoc Unary::operator==
    bool operator==(const Exp& o) const override;

    /// \copydoc Unary::operator<
    bool operator<(const Exp& o) const override;

    /// \copydoc Exp::operator<<
    bool operator<<(const Exp& o) const override;

    /// \copydoc Unary::operator*=
    bool operator*=(const Exp& o) const override;

    /// \copydoc Unary::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Unary::printx
    virtual void printx(int ind) const override;

    /// Get and set the type
    SharedType getType()       { return m_type; }
    SharedConstType getType() const { return m_type; }

    void setType(SharedType ty) { m_type = ty; }

    /// \copydoc Unary::polySimplify
    virtual SharedExp polySimplify(bool& changed) override;

    /// \copydoc Unary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Unary::descendType
    virtual void descendType(SharedType, bool&, Statement *) override;

public:
    /// \copydoc Unary::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Unary::accept
    virtual SharedExp accept(ExpModifier *mod) override;

    virtual SharedExp preAccept(ExpModifier *mod, bool& visitChildren) override;
    virtual SharedExp postAccept(ExpModifier *mod) override;

private:
    SharedType m_type;
};
