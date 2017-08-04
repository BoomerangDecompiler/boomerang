#pragma once

#include "boomerang/db/exp/Unary.h"

/**
 * Binary is a subclass of Unary, holding two subexpressions
 */
class Binary : public Unary
{
protected:
    /// Constructor, with ID
    Binary(OPER op);

public:
    /// Constructor, with ID and subexpressions
    Binary(OPER op, SharedExp e1, SharedExp e2);

    /// Copy constructor
    Binary(const Binary& o);

    static std::shared_ptr<Binary> get(OPER op, SharedExp e1, SharedExp e2)
    { return std::make_shared<Binary>(op, e1, e2); }

    // Clone
    virtual SharedExp clone() const override;

    // Compare
    bool operator==(const Exp& o) const override;
    bool operator<(const Exp& o) const override;
    bool operator*=(const Exp& o) const override;

    /// Destructor
    virtual ~Binary();

    /// Arity
    int getArity() const override { return 2; }

    // Print
    virtual void print(QTextStream& os, bool html = false) const override;
    virtual void printr(QTextStream& os, bool html = false) const override;
    virtual void appendDotFile(QTextStream& of) override;
    virtual void printx(int ind) const override;

    // Set second subexpression
    void setSubExp2(SharedExp e) override;

    // Get second subexpression
    SharedExp getSubExp2() override;
    SharedConstExp getSubExp2() const override;

    /***************************************************************************/ /**
    * \brief        Swap the two subexpressions
    ******************************************************************************/
    void commute();                   ///< Commute the two operands
    SharedExp& refSubExp2() override; ///< Get a reference to subexpression 2

    virtual SharedExp match(const SharedConstExp& pattern) override;
    virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

    /// Search children
    void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

    /// Do the work of simplifying this expression
    virtual SharedExp polySimplify(bool& bMod) override;
    SharedExp simplifyArith() override;
    SharedExp simplifyAddr() override;
    virtual SharedExp simplifyConstraint() override;

    /// Type analysis
    SharedExp genConstraints(SharedExp restrictTo) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool& ch, Statement *s) override;

private:
    /// Return a constraint that my subexpressions have to be of type typeval1 and typeval2 respectively
    SharedExp constrainSub(const std::shared_ptr<TypeVal>& typeVal1, const std::shared_ptr<TypeVal>& typeVal2);

protected:
    SharedExp subExp2; ///< Second subexpression pointer
};
