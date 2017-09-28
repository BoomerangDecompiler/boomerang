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
#include "boomerang/db/exp/Const.h"

class Location : public Unary
{
public:
    /**
     * Constructor with ID, subexpression, and UserProc*
     * Create a new Location expression.
     * \param op Should be opRegOf, opMemOf, opLocal, opGlobal, opParam or opTemp.
     * \param exp - child expression
     * \param proc - enclosing procedure, if null this constructor will try to find it.
     */
    Location(OPER op, SharedExp exp, UserProc *proc);

    // Copy constructor
    Location(Location& o);

    // Custom constructor
    static SharedExp get(OPER op, SharedExp e, UserProc *proc) { return std::make_shared<Location>(op, e, proc); }
    static SharedExp regOf(int r) { return get(opRegOf, Const::get(r), nullptr); }
    static SharedExp regOf(SharedExp e) { return get(opRegOf, e, nullptr); }
    static SharedExp memOf(SharedExp e, UserProc *p = nullptr) { return get(opMemOf, e, p); }
    static std::shared_ptr<Location> tempOf(SharedExp e) { return std::make_shared<Location>(opTemp, e, nullptr); }
    static SharedExp global(const char *nam, UserProc *p) { return get(opGlobal, Const::get(nam), p); }
    static SharedExp global(const QString& nam, UserProc *p) { return get(opGlobal, Const::get(nam), p); }
    static std::shared_ptr<Location> local(const QString& nam, UserProc *p);

    static SharedExp param(const char *nam, UserProc *p = nullptr) { return get(opParam, Const::get(nam), p); }
    static SharedExp param(const QString& nam, UserProc *p = nullptr) { return get(opParam, Const::get(nam), p); }
    // Clone
    virtual SharedExp clone() const override;

    void setProc(UserProc *p) { proc = p; }
    UserProc *getProc() { return proc; }

    virtual SharedExp polySimplify(bool& bMod) override;
    virtual void getDefinitions(LocationSet& defs);

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;
    virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

protected:
    Location(OPER _op)
        : Unary(_op)
        , proc(nullptr) {}

private:
    UserProc *proc;
};
