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
     * \param op Should be opRegOf, opMemOf, opLocal, opGlobal, opParam or opTemp.
     * \param exp - child expression
     * \param proc - enclosing procedure, if null this constructor will try to find it.
     */
    Location(OPER op, SharedExp exp, UserProc *proc);
    Location(const Location& other);
    Location(Location&& other) = default;

    virtual ~Location() override = default;

    Location& operator=(const Location& other) = default;
    Location& operator=(Location&& other) = default;

public:
    virtual SharedExp clone() const override;

    static SharedExp get(OPER op, SharedExp childExp, UserProc *proc) { return std::make_shared<Location>(op, childExp, proc); }

    static SharedExp regOf(int regID) { return get(opRegOf, Const::get(regID), nullptr); }
    static SharedExp regOf(SharedExp exp) { return get(opRegOf, exp, nullptr); }
    static SharedExp memOf(SharedExp exp, UserProc *proc = nullptr) { return get(opMemOf, exp, proc); }

    static std::shared_ptr<Location> tempOf(SharedExp e) { return std::make_shared<Location>(opTemp, e, nullptr); }

    static SharedExp global(const char *name, UserProc *proc) { return get(opGlobal, Const::get(name), proc); }
    static SharedExp global(const QString& name, UserProc *proc) { return get(opGlobal, Const::get(name), proc); }

    static std::shared_ptr<Location> local(const QString& name, UserProc *proc);

    static SharedExp param(const char *name, UserProc *proc = nullptr) { return get(opParam, Const::get(name), proc); }
    static SharedExp param(const QString& name, UserProc *proc = nullptr) { return get(opParam, Const::get(name), proc); }

    void setProc(UserProc *p) { m_proc = p; }
    const UserProc *getProc() const { return m_proc; }
    UserProc *getProc() { return m_proc; }

    /// \copydoc Unary::polySimplify
    virtual SharedExp polySimplify(bool& changed) override;

    void getDefinitions(LocationSet& defs);

public:
    /// \copydoc Unary::acceptVisitor
    virtual bool acceptVisitor(ExpVisitor *v) override;

protected:
    /// \copydoc Exp::acceptPreModifier
    virtual SharedExp acceptPreModifier(ExpModifier *mod, bool& visitChildren) override;

    /// \copydoc Exp::acceptPostModifier
    virtual SharedExp acceptPostModifier(ExpModifier *mod) override;

private:
    UserProc *m_proc;
};
