#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "PhiAssign.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/ExpModifier.h"
#include "boomerang/db/visitor/StmtVisitor.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtModifier.h"
#include "boomerang/db/visitor/StmtPartModifier.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/Log.h"


bool BasicBlock::BBComparator::operator()(const BasicBlock* bb1, const BasicBlock* bb2) const
{
    // special case: in test code, we have statements that do not belong to BBs.
    // Thus, bb is nullptr
    if (bb1 && bb2) {
        return bb1->getLowAddr() < bb2->getLowAddr();
    }
    else {
        // compare pointers
        return bb1 < bb2;
    }
}


Statement *PhiAssign::clone() const
{
    PhiAssign *pa = new PhiAssign(m_type, m_lhs);

    PhiDefs::const_iterator dd;

    for (dd = m_defs.begin(); dd != m_defs.end(); dd++) {
        PhiInfo pi;
        pi.setDef((Statement *)dd->second.getDef()); // Don't clone the Statement pointer (never moves)
        pi.e = dd->second.e->clone();                // Do clone the expression pointer
        assert(pi.e);
        pa->m_defs.insert(std::make_pair(dd->first, pi));
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

    for (const auto& v : m_defs) {
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

        for (auto it = m_defs.begin(); it != m_defs.end(); /* no increment */) {
            if (it->second.getDef()) {
                if (html) {
                    os << "<a href=\"#stmt" << it->second.getDef()->getNumber() << "\">";
                }

                os << it->second.getDef()->getNumber();

                if (html) {
                    os << "</a>";
                }
            }
            else {
                os << "-";
            }

            if (++it != m_defs.end()) {
                os << " ";
            }
        }

        os << "}";
    }
    else {
        os << "(";

        for (auto it = m_defs.begin(); it != m_defs.end(); /* no increment */) {
            SharedExp e = it->second.e;

            if (e == nullptr) {
                os << "nullptr{";
            }
            else {
                os << e << "{";
            }

            if (it->second.getDef()) {
                os << it->second.getDef()->getNumber();
            }
            else {
                os << "-";
            }

            os << "}";

            if (++it != m_defs.end()) {
                os << " ";
            }
        }

        os << ")";
    }
}


bool PhiAssign::search(const Exp& pattern, SharedExp& result) const
{
    if (m_lhs->search(pattern, result)) {
        return true;
    }

    for (auto& v : m_defs) {
        assert(v.second.e != nullptr);
        // Note: can't match foo{-} because of this
        RefExp re(v.second.e, const_cast<Statement *>(v.second.getDef())); ///< \todo remove const_cast

        if (re.search(pattern, result)) {
            return true;
        }
    }

    return false;
}


bool PhiAssign::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    // FIXME: is this the right semantics for searching a phi statement,
    // disregarding the RHS?
    return m_lhs->searchAll(pattern, result);
}


bool PhiAssign::searchAndReplace(const Exp& pattern, SharedExp replace, bool /*cc*/)
{
    bool change;

    m_lhs = m_lhs->searchReplaceAll(pattern, replace, change);

    for (auto& v : m_defs) {
        assert(v.second.e != nullptr);
        bool ch;
        // Assume that the definitions will also be replaced
        v.second.e = v.second.e->searchReplaceAll(pattern, replace, ch);
        assert(v.second.e);
        change |= ch;
    }

    return change;
}


PhiInfo& PhiAssign::getAt(BasicBlock *idx)
{
    return m_defs[idx];
}


bool PhiAssign::accept(StmtExpVisitor *visitor)
{
    bool visitChildren = true;
    bool ret = visitor->visit(this, visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_lhs) {
        ret = m_lhs->accept(visitor->ev);
    }

    for (auto& v : m_defs) {
        assert(v.second.e != nullptr);
        // RefExp *re = RefExp::get(v.second.e, v.second.def());
        ret = RefExp::get(v.second.e, v.second.getDef())->accept(visitor->ev);

        if (ret == false) {
            return false;
        }
    }

    return true;
}


bool PhiAssign::accept(StmtModifier *v)
{
    bool visitChildren;
    v->visit(this, visitChildren);

    if (v->m_mod) {
        v->m_mod->clearMod();

        if (visitChildren) {
            m_lhs = m_lhs->accept(v->m_mod);
        }

        if (v->m_mod->isMod()) {
            LOG_VERBOSE("PhiAssign changed: now %1", this);
        }
    }

    return true;
}


bool PhiAssign::accept(StmtPartModifier *v)
{
    bool visitChildren;
    v->visit(this, visitChildren);
    v->mod->clearMod();

    if (visitChildren && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->accept(v->mod));
    }

    if (v->mod->isMod()) {
        LOG_VERBOSE("PhiAssign changed: now %1", this);
    }

    return true;
}


void PhiAssign::convertToAssign(SharedExp rhs)
{
    // I believe we always want to propagate to these ex-phi's; check!:
    rhs = rhs->propagateAll();
    // Thanks to tamlin for this cleaner way of implementing this hack

    int        n     = m_number;     // These items disappear with the destructor below
    BasicBlock *bb   = m_parent;
    UserProc   *p    = m_proc;
    SharedExp  lhs_  = m_lhs;
    SharedExp  rhs_  = rhs;
    SharedType type_ = m_type;

    assert(sizeof(Assign) <= sizeof(PhiAssign));
    this->~PhiAssign();                                   // Explicitly destroy this, but keep the memory allocated.
    Assign *a = new (this) Assign(type_, lhs_, rhs_);     // construct in-place. Note that 'a' == 'this'
    a->setNumber(n);
    a->setProc(p);
    a->setBB(bb);
}


void PhiAssign::simplify()
{
    m_lhs = m_lhs->simplify();

    if (m_defs.empty()) {
        return;
    }

    bool              allSame = true;
    PhiDefs::iterator uu      = m_defs.begin();
    Statement         *first  = m_defs.begin()->second.getDef();
    ++uu;

    for ( ; uu != m_defs.end(); uu++) {
        if (uu->second.getDef() != first) {
            allSame = false;
            break;
        }
    }

    if (allSame) {
        LOG_VERBOSE("all the same in %1", this);
        convertToAssign(RefExp::get(m_lhs, first));
        return;
    }

    bool      onlyOneNotThis = true;
    Statement *notthis       = (Statement *)-1;

    for (auto& v : m_defs) {
        if ((v.second.getDef() == nullptr) || v.second.getDef()->isImplicit() || !v.second.getDef()->isPhi() ||
            (v.second.getDef() != this)) {
            if (notthis != (Statement *)-1) {
                onlyOneNotThis = false;
                break;
            }
            else {
                notthis = v.second.getDef();
            }
        }
    }

    if (onlyOneNotThis && (notthis != (Statement *)-1)) {
        LOG_VERBOSE("All but one not this in %1", this);

        convertToAssign(RefExp::get(m_lhs, notthis));
        return;
    }
}


void PhiAssign::putAt(BasicBlock *i, Statement *def, SharedExp e)
{
    assert(e);     // should be something surely
    // assert(defVec.end()==defVec.find(i));
    m_defs[i].setDef(def);
    m_defs[i].e = e;
}


void PhiAssign::enumerateParams(std::list<SharedExp>& le)
{
    for (auto& v : m_defs) {
        assert(v.second.e != nullptr);
        auto r = RefExp::get(v.second.e, v.second.getDef());
        le.push_back(r);
    }
}


Statement* PhiAssign::getStmtAt(BasicBlock* idx)
{
    if (m_defs.find(idx) == m_defs.end()) {
        return nullptr;
    }

    return m_defs[idx].getDef();
}

