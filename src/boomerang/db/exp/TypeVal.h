#pragma once

#include "boomerang/db/exp/Terminal.h"


/***************************************************************************/ /**
* class TypeVal. Just a Terminal with a Type. Used for type values in constraints
* ==============================================================================*/
class TypeVal : public Terminal
{
public:
    TypeVal(SharedType ty);
    ~TypeVal();

    static std::shared_ptr<TypeVal> get(const SharedType& st) { return std::make_shared<TypeVal>(st); }

    virtual SharedType getType() { return val; }
    virtual void setType(SharedType t) { val = t; }
    virtual SharedExp clone() const override;

    bool operator==(const Exp& o) const override;
    bool operator<(const Exp& o) const override;
    bool operator*=(const Exp& o) const override;
    void print(QTextStream& os, bool = false) const override;
    void printx(int ind) const override;

    /// Should not be constraining constraints
    SharedExp genConstraints(SharedExp /*restrictTo*/) override
    {
        assert(0);
        return nullptr;
    }

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

private:
    SharedType val;
};
