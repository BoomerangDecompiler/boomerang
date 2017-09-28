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


/***************************************************************************/ /**
 * TypedExp is a subclass of Unary, holding one subexpression and a Type
 ******************************************************************************/
class TypedExp : public Unary
{
public:
    // Constructor
    TypedExp();
    // Constructor, subexpression
    TypedExp(SharedExp e1);
    // Constructor, type, and subexpression.
    // A rare const parameter allows the common case of providing a temporary,
    // e.g. foo = new TypedExp(Type(INTEGER), ...);
    TypedExp(SharedType ty, SharedExp e1);
    // Copy constructor
    TypedExp(TypedExp& o);

    // Clone
    virtual SharedExp clone() const override;

    // Compare
    bool operator==(const Exp& o) const override;
    bool operator<(const Exp& o) const override;
    bool operator<<(const Exp& o) const override;
    bool operator*=(const Exp& o) const override;

    virtual void print(QTextStream& os, bool html = false) const override;
    virtual void appendDotFile(QTextStream& of) override;
    virtual void printx(int ind) const override;

    /// Get and set the type
    virtual SharedType getType()       { return type; }
    virtual const SharedType& getType() const { return type; }
    virtual void setType(SharedType ty) { type = ty; }

    // polySimplify
    virtual SharedExp polySimplify(bool& bMod) override;

    // Visitation
    /// All the Unary derived accept functions look the same, but they have to be repeated because the particular visitor
    /// function called each time is different for each class (because "this" is different each time)
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;

    virtual void descendType(SharedType, bool&, Statement *) override;

private:
    SharedType type;
};

