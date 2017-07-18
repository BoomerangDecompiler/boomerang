#include "Location.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/visitor.h"

Location::Location(Location& o)
    : Unary(o.m_oper, o.subExp1->clone())
    , proc(o.proc)
{
}


Location::Location(OPER _op, SharedExp exp, UserProc *_p)
    : Unary(_op, exp)
    , proc(_p)
{
    assert(m_oper == opRegOf || m_oper == opMemOf || m_oper == opLocal || m_oper == opGlobal || m_oper == opParam || m_oper == opTemp);

    if (_p == nullptr) {
        // eep.. this almost always causes problems
        SharedExp e = exp;

        if (e) {
            bool giveUp = false;

            while (this->proc == nullptr && !giveUp) {
                switch (e->getOper())
                {
                case opRegOf:
                case opMemOf:
                case opTemp:
                case opLocal:
                case opGlobal:
                case opParam:
                    this->proc = std::static_pointer_cast<Location>(e)->getProc();
                    giveUp     = true;
                    break;

                case opSubscript:
                    e = e->getSubExp1();
                    break;

                default:
                    giveUp = true;
                    break;
                }
            }
        }
    }
}


SharedExp Location::clone() const
{
    return std::make_shared<Location>(m_oper, subExp1->clone(), proc);
}


bool Location::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
    if (Exp::match(pattern, bindings)) {
        return true;
    }

#ifdef DEBUG_MATCH
    LOG << "location::match " << this << " to " << pattern << ".\n";
#endif

    if ((m_oper == opMemOf) || (m_oper == opRegOf)) {
        if ((m_oper == opRegOf) && !pattern.startsWith("r[")) {
            return false;
        }

        if ((m_oper == opMemOf) && !pattern.startsWith("m[")) {
            return false;
        }

        if (!pattern.endsWith(']')) {
            return false;
        }

        return subExp1->match(pattern.mid(2), bindings); // shouldn't this cut the last ']' ??
    }

    return false;
}



SharedExp Location::polySimplify(bool& bMod)
{
    SharedExp res = Unary::polySimplify(bMod);

    if ((res->getOper() == opMemOf) && (res->getSubExp1()->getOper() == opAddrOf)) {
        if (VERBOSE) {
            LOG << "polySimplify " << res << "\n";
        }

        res  = res->getSubExp1()->getSubExp1();
        bMod = true;
        return res;
    }

    // check for m[a[loc.x]] becomes loc.x
    if ((res->getOper() == opMemOf) && (res->getSubExp1()->getOper() == opAddrOf) &&
        (res->getSubExp1()->getSubExp1()->getOper() == opMemberAccess)) {
        res  = subExp1->getSubExp1();
        bMod = true;
        return res;
    }

    return res;
}


void Location::getDefinitions(LocationSet& defs)
{
    // This is a hack to fix aliasing (replace with something general)
    // FIXME! This is x86 specific too. Use -O for overlapped registers!
    if ((m_oper == opRegOf) && (std::static_pointer_cast<const Const>(subExp1)->getInt() == 24)) {
        defs.insert(Location::regOf(0));
    }
}


bool Location::accept(ExpVisitor *v)
{
    bool override = false, ret = v->visit(shared_from_base<Location>(), override);

    if (override) {
        return ret;
    }

    if (ret) {
        ret &= subExp1->accept(v);
    }

    return ret;
}



SharedExp Location::accept(ExpModifier *v)
{
    // This looks to be the same source code as Unary::accept, but the type of "this" is different, which is all
    // important here!  (it makes a call to a different visitor member function).
    bool      recur;
    SharedExp ret = v->preVisit(shared_from_base<Location>(), recur);

    if (recur) {
        subExp1 = subExp1->accept(v);
    }

    auto loc_ret = std::dynamic_pointer_cast<Location>(ret);

    if (loc_ret) {
        return v->postVisit(loc_ret);
    }

    auto ref_ret = std::dynamic_pointer_cast<RefExp>(ret);

    if (ref_ret) {
        return v->postVisit(ref_ret);
    }

    assert(false);
    return nullptr;
}



std::shared_ptr<Location> Location::local(const QString& nam, UserProc *p)
{
    return std::make_shared<Location>(opLocal, Const::get(nam), p);
}
