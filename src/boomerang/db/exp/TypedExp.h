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

    // Constructor, type, and subexpression.
    // A rare const parameter allows the common case of providing a temporary,
    // e.g. foo = new TypedExp(Type(INTEGER), ...);
    TypedExp(SharedType ty, SharedExp e1);
    TypedExp(TypedExp& o);

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

    /// \copydoc Unary::appendDotFile
    virtual void appendDotFile(QTextStream& of) override;

    /// Get and set the type
    SharedType getType()       { return m_type; }
    const SharedType& getType() const { return m_type; }
    void setType(SharedType ty) { m_type = ty; }

    /// \copydoc Unary::polySimplify
    virtual SharedExp polySimplify(bool& bMod) override;

    /// \copydoc Unary::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Unary::accept
    virtual SharedExp accept(ExpModifier *v) override;

    /// \copydoc Unary::ascendType
    virtual SharedType ascendType() override;

    /// \copydoc Unary::descendType
    virtual void descendType(SharedType, bool&, Statement *) override;

private:
    SharedType m_type;
};
