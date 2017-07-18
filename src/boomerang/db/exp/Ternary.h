#pragma once

#include "boomerang/db/exp/Binary.h"


/***************************************************************************/ /**
* Ternary is a subclass of Binary, holding three subexpressions
******************************************************************************/
class Ternary : public Binary
{
private:
    SharedExp subExp3; // Third subexpression pointer

    // Constructor, with operator
    Ternary(OPER op);

public:
    // Constructor, with operator and subexpressions
    Ternary(OPER op, SharedExp e1, SharedExp e2, SharedExp e3);
    // Copy constructor
    Ternary(const Ternary& o);

    /***************************************************************************/ /**
    * \brief        Virtual function to make a clone of myself, i.e. to create
    *               a new Exp with the same contents as myself, but not sharing
    *               any memory. Deleting the clone will not affect this object.
    *               Pointers to subexpressions are not copied, but also cloned.
    * \returns      Pointer to cloned object
    ******************************************************************************/
    virtual SharedExp clone() const override;

    // Compare
    bool operator==(const Exp& o) const override;
    bool operator<(const Exp& o) const override;
    bool operator*=(const Exp& o) const override;

    // Destructor
    virtual ~Ternary();

    // Arity
    int getArity() const override { return 3; }

    // Print
    virtual void print(QTextStream& os, bool html = false) const override;
    virtual void printr(QTextStream& os, bool = false) const override;
    virtual void appendDotFile(QTextStream& of) override;
    virtual void printx(int ind) const override;

    // Set third subexpression
    void setSubExp3(SharedExp e) override;

    // Get third subexpression
    SharedExp getSubExp3() override;
    SharedConstExp getSubExp3() const override;

    // Get a reference to subexpression 3
    SharedExp& refSubExp3() override;

    // Search children
    void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

    SharedExp polySimplify(bool& bMod) override;
    SharedExp simplifyArith() override;
    SharedExp simplifyAddr() override;

    // Type analysis
    SharedExp genConstraints(SharedExp restrictTo) override;

    // Visitation
    bool accept(ExpVisitor *v) override;
    SharedExp accept(ExpModifier *v) override;

    virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

    virtual SharedType ascendType() override;

    virtual void descendType(SharedType /*parentType*/, bool& ch, Instruction *s) override;
};
