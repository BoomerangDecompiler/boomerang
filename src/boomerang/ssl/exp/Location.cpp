#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Location.h"

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


Location::Location(const Location &other)
    : Unary(other.m_oper, other.m_subExp1->clone())
    , m_proc(other.m_proc)
{
}


Location::Location(OPER oper, SharedExp exp, UserProc *proc)
    : Unary(oper, exp)
    , m_proc(proc)
{
    assert(m_oper == opRegOf || m_oper == opMemOf || m_oper == opLocal || m_oper == opGlobal ||
           m_oper == opParam || m_oper == opTemp);

    if (proc == nullptr) {
        // eep.. this almost always causes problems
        SharedExp e = exp;

        if (e) {
            bool giveUp = false;

            while (this->m_proc == nullptr && !giveUp) {
                switch (e->getOper()) {
                case opRegOf:
                case opMemOf:
                case opTemp:
                case opLocal:
                case opGlobal:
                case opParam:
                    this->m_proc = e->access<Location>()->getProc();
                    giveUp       = true;
                    break;

                case opSubscript: e = e->getSubExp1(); break;

                default: giveUp = true; break;
                }
            }
        }
    }
}


SharedExp Location::clone() const
{
    return std::make_shared<Location>(m_oper, m_subExp1->clone(), m_proc);
}


SharedExp Location::get(OPER op, SharedExp childExp, UserProc *proc)
{
    return std::make_shared<Location>(op, childExp, proc);
}


SharedExp Location::regOf(RegNum regNum)
{
    return get(opRegOf, Const::get(regNum), nullptr);
}


SharedExp Location::regOf(SharedExp exp)
{
    return get(opRegOf, exp, nullptr);
}


SharedExp Location::memOf(SharedExp exp, UserProc *proc)
{
    return get(opMemOf, exp, proc);
}


SharedExp Location::tempOf(SharedExp e)
{
    return get(opTemp, e, nullptr);
}


SharedExp Location::global(const char *name, UserProc *proc)
{
    return get(opGlobal, Const::get(name), proc);
}


SharedExp Location::global(const QString &name, UserProc *proc)
{
    return get(opGlobal, Const::get(name), proc);
}


SharedExp Location::param(const char *name, UserProc *proc)
{
    return get(opParam, Const::get(name), proc);
}


SharedExp Location::param(const QString &name, UserProc *proc)
{
    return get(opParam, Const::get(name), proc);
}


bool Location::acceptVisitor(ExpVisitor *v)
{
    bool visitChildren = true;
    if (!v->preVisit(shared_from_base<Location>(), visitChildren)) {
        return false;
    }

    if (visitChildren) {
        if (!m_subExp1->acceptVisitor(v)) {
            return false;
        }
    }

    return v->postVisit(shared_from_base<Location>());
}


SharedExp Location::local(const QString &name, UserProc *p)
{
    return get(opLocal, Const::get(name), p);
}


SharedExp Location::acceptPreModifier(ExpModifier *mod, bool &visitChildren)
{
    return mod->preModify(access<Location>(), visitChildren);
}


SharedExp Location::acceptPostModifier(ExpModifier *mod)
{
    return mod->postModify(access<Location>());
}
