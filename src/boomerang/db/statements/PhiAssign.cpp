#include "PhiAssign.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/Visitor.h"
#include "boomerang/util/Log.h"


Instruction *PhiAssign::clone() const
{
    PhiAssign *pa = new PhiAssign(m_type, m_lhs);

    Definitions::const_iterator dd;

    for (dd = DefVec.begin(); dd != DefVec.end(); dd++) {
        PhiInfo pi;
        pi.def((Instruction *)dd->second.def());     // Don't clone the Statement pointer (never moves)
        pi.e = dd->second.e->clone();                // Do clone the expression pointer
        assert(pi.e);
        pa->DefVec.insert(std::make_pair(dd->first, pi));
    }

    return pa;
}


bool PhiAssign::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


void PhiAssign::printCompact(QTextStream& os, bool html) const
{
    os << "*" << m_type << "* ";

    if (m_lhs) {
        m_lhs->print(os, html);
    }

    os << " := phi";
    // Print as lhs := phi{9 17} for the common case where the lhs is the same location as all the referenced
    // locations. When not, print as local4 := phi(r24{9} argc{17})
    bool simple = true;

    for (const auto& v : DefVec) {
        assert(v.second.e != nullptr);

        // If e is nullptr assume it is meant to match lhs
        if (!(*v.second.e == *m_lhs)) {
            // One of the phi parameters has a different base expression to lhs. Use non-simple print.
            simple = false;
            break;
        }
    }

    if (simple) {
        os << "{";

        for (auto it = DefVec.begin(); it != DefVec.end(); /* no increment */) {
            if (it->second.def()) {
                if (html) {
                    os << "<a href=\"#stmt" << it->second.def()->getNumber() << "\">";
                }

                os << it->second.def()->getNumber();

                if (html) {
                    os << "</a>";
                }
            }
            else {
                os << "-";
            }

            if (++it != DefVec.end()) {
                os << " ";
            }
        }

        os << "}";
    }
    else {
        os << "(";

        for (auto it = DefVec.begin(); it != DefVec.end(); /* no increment */) {
            SharedExp e = it->second.e;

            if (e == nullptr) {
                os << "nullptr{";
            }
            else {
                os << e << "{";
            }

            if (it->second.def()) {
                os << it->second.def()->getNumber();
            }
            else {
                os << "-";
            }

            os << "}";

            if (++it != DefVec.end()) {
                os << " ";
            }
        }

        os << ")";
    }
}


bool PhiAssign::search(const Exp& search, SharedExp& result) const
{
    if (m_lhs->search(search, result)) {
        return true;
    }

    for (auto& v : DefVec) {
        assert(v.second.e != nullptr);
        // Note: can't match foo{-} because of this
        RefExp re(v.second.e, const_cast<Instruction *>(v.second.def())); ///< \todo remove const_cast

        if (re.search(search, result)) {
            return true;
        }
    }

    return false;
}


bool PhiAssign::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
    return m_lhs->searchAll(search, result);
}


bool PhiAssign::searchAndReplace(const Exp& search, SharedExp replace, bool /*cc*/)
{
    bool change;

    m_lhs = m_lhs->searchReplaceAll(search, replace, change);

    for (auto& v : DefVec) {
        assert(v.second.e != nullptr);
        bool ch;
        // Assume that the definitions will also be replaced
        v.second.e = v.second.e->searchReplaceAll(search, replace, ch);
        assert(v.second.e);
        change |= ch;
    }

    return change;
}


void PhiAssign::genConstraints(LocationSet& cons)
{
    // Generate a constraints st that all the phi's have to be the same type as
    // result
    SharedExp result = Unary::get(opTypeOf, RefExp::get(m_lhs, this));

    for (auto& v : DefVec) {
        assert(v.second.e != nullptr);
        SharedExp conjunct = Binary::get(opEquals, result, Unary::get(opTypeOf, RefExp::get(v.second.e, v.second.def())));
        cons.insert(conjunct);
    }
}


PhiInfo& PhiAssign::getAt(BasicBlock *idx)
{
    return DefVec[idx];
}


bool PhiAssign::accept(StmtExpVisitor *visitor)
{
    bool override;
    bool ret = visitor->visit(this, override);

    if (override) {
        return ret;
    }

    if (ret && m_lhs) {
        ret = m_lhs->accept(visitor->ev);
    }

    for (auto& v : DefVec) {
        assert(v.second.e != nullptr);
        // RefExp *re = RefExp::get(v.second.e, v.second.def());
        ret = RefExp::get(v.second.e, v.second.def())->accept(visitor->ev);

        if (ret == false) {
            return false;
        }
    }

    return true;
}


bool PhiAssign::accept(StmtModifier *v)
{
    bool recur;

    v->visit(this, recur);
    v->m_mod->clearMod();

    if (recur) {
        m_lhs = m_lhs->accept(v->m_mod);
    }

    if (VERBOSE && v->m_mod->isMod()) {
        LOG << "PhiAssign changed: now " << this << "\n";
    }

    return true;
}


bool PhiAssign::accept(StmtPartModifier *v)
{
    bool recur;

    v->visit(this, recur);
    v->mod->clearMod();

    if (recur && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->accept(v->mod));
    }

    if (VERBOSE && v->mod->isMod()) {
        LOG << "PhiAssign changed: now " << this << "\n";
    }

    return true;
}


void PhiAssign::convertToAssign(SharedExp rhs)
{
    // I believe we always want to propagate to these ex-phi's; check!:
    rhs = rhs->propagateAll();
    // Thanks to tamlin for this cleaner way of implementing this hack
    assert(sizeof(Assign) <= sizeof(PhiAssign));
    int        n     = m_number;     // These items disappear with the destructor below
    BasicBlock *bb   = m_parent;
    UserProc   *p    = m_proc;
    SharedExp  lhs_  = m_lhs;
    SharedExp  rhs_  = rhs;
    SharedType type_ = m_type;

    this->~PhiAssign();                                   // Explicitly destroy this, but keep the memory allocated.
    Assign *a = new (this) Assign(type_, lhs_, rhs_);     // construct in-place. Note that 'a' == 'this'
    a->setNumber(n);
    a->setProc(p);
    a->setBB(bb);
}


void PhiAssign::simplify()
{
    m_lhs = m_lhs->simplify();

    if (DefVec.empty()) {
        return;
    }

    bool allSame                 = true;
    Definitions::iterator uu     = DefVec.begin();
    Instruction           *first = DefVec.begin()->second.def();
    ++uu;

    for ( ; uu != DefVec.end(); uu++) {
        if (uu->second.def() != first) {
            allSame = false;
            break;
        }
    }

    if (allSame) {
        LOG_VERBOSE(1) << "all the same in " << this << "\n";
        convertToAssign(RefExp::get(m_lhs, first));
        return;
    }

    bool        onlyOneNotThis = true;
    Instruction *notthis       = (Instruction *)-1;

    for (auto& v : DefVec) {
        if ((v.second.def() == nullptr) || v.second.def()->isImplicit() || !v.second.def()->isPhi() ||
            (v.second.def() != this)) {
            if (notthis != (Instruction *)-1) {
                onlyOneNotThis = false;
                break;
            }
            else {
                notthis = v.second.def();
            }
        }
    }

    if (onlyOneNotThis && (notthis != (Instruction *)-1)) {
        if (VERBOSE) {
            LOG << "all but one not this in " << this << "\n";
        }

        convertToAssign(RefExp::get(m_lhs, notthis));
        return;
    }
}


void PhiAssign::putAt(BasicBlock *i, Instruction *def, SharedExp e)
{
    assert(e);     // should be something surely
    // assert(defVec.end()==defVec.find(i));
    DefVec[i].def(def);
    DefVec[i].e = e;
}


void PhiAssign::enumerateParams(std::list<SharedExp>& le)
{
    for (auto& v : DefVec) {
        assert(v.second.e != nullptr);
        auto r = RefExp::get(v.second.e, v.second.def());
        le.push_back(r);
    }
}


void PhiAssign::dfaTypeAnalysis(bool& ch)
{
    iterator it = DefVec.begin();

    while (it->second.e == nullptr && it != DefVec.end()) {
        ++it;
    }

    assert(it != DefVec.end());
    SharedType meetOfArgs = it->second.def()->getTypeFor(m_lhs);

    for (++it; it != DefVec.end(); ++it) {
        PhiInfo& phinf(it->second);

        if (phinf.e == nullptr) {
            continue;
        }

        assert(phinf.def());
        SharedType typeOfDef = phinf.def()->getTypeFor(phinf.e);
        meetOfArgs = meetOfArgs->meetWith(typeOfDef, ch);
    }

    m_type = m_type->meetWith(meetOfArgs, ch);

    for (it = DefVec.begin(); it != DefVec.end(); ++it) {
        if (it->second.e == nullptr) {
            continue;
        }

        it->second.def()->meetWithFor(m_type, it->second.e, ch);
    }

    Assignment::dfaTypeAnalysis(ch); // Handle the LHS
}
