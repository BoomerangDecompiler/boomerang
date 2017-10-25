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
protected:
    /// Constructor, with just ID
    Unary(OPER op);

public:
    /// Constructor, with ID and subexpression
    Unary(OPER op, SharedExp e);
    /// Copy constructor
    Unary(const Unary& o);

    static SharedExp get(OPER op, SharedExp e1) { return std::make_shared<Unary>(op, e1); }

    // Clone
    virtual SharedExp clone() const override;

    // Compare
    virtual bool operator==(const Exp& o) const override;
    virtual bool operator<(const Exp& o) const override;
    bool operator*=(const Exp& o) const override;

    // Destructor
    virtual ~Unary() override;

    /// Arity
    virtual int getArity() const override { return 1; }

    // Print
    virtual void print(QTextStream& os, bool html = false) const override;
    virtual void appendDotFile(QTextStream& of) override;
    virtual void printx(int ind) const override;

    // Set first subexpression
    void setSubExp1(SharedExp e) override;

    // Get first subexpression
    SharedExp getSubExp1() override;
    SharedConstExp getSubExp1() const override;

    // Get a reference to subexpression 1
    SharedExp& refSubExp1() override;

    virtual SharedExp match(const SharedConstExp& pattern) override;
    virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

    // Search children
    void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

    // Do the work of simplifying this expression

    /**
     * \brief        Do the work of simplification
     * \note         User must ;//delete result
     * \note         Address simplification (a[ m[ x ]] == x) is done separately
     * \returns      Ptr to the simplified expression
     */
    virtual SharedExp polySimplify(bool& bMod) override;
    SharedExp simplifyArith() override;

    /**
     * \brief        Just do addressof simplification: a[ m[ any ]] == any, m[ a[ any ]] = any, and also
     *               a[ size m[ any ]] == any
     * \todo         Replace with a visitor some day
     * \returns      Ptr to the simplified expression
     */
    SharedExp simplifyAddr() override;
    virtual SharedExp simplifyConstraint() override;

    // Type analysis
    SharedExp genConstraints(SharedExp restrictTo) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool& ch, Statement *s) override;

protected:
    SharedExp subExp1; ///< One subexpression pointer
};
