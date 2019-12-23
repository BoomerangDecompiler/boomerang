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

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"


SharedStmt PhiAssign::clone() const
{
    std::shared_ptr<PhiAssign> pa(new PhiAssign(m_type, m_lhs));

    for (const auto &[frag, ref] : m_defs) {
        assert(ref->getSubExp1());

        // Clone the expression pointer, but not the fragment pointer (never moves)
        pa->m_defs.insert({ frag, RefExp::get(ref->getSubExp1()->clone(), ref->getDef()) });
    }

    return pa;
}


bool PhiAssign::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
}


void PhiAssign::printCompact(OStream &os) const
{
    os << "*" << m_type << "* ";

    if (m_lhs) {
        m_lhs->print(os);
    }

    os << " := phi";
    for (const auto &[frag, ref] : m_defs) {
        Q_UNUSED(frag);
        Q_UNUSED(ref);

        assert(ref->getSubExp1() != nullptr);
        assert(*ref->getSubExp1() == *m_lhs);
    }

    os << "{";

    for (auto it = m_defs.begin(); it != m_defs.end(); /* no increment */) {
        if (it->second->getDef()) {
            os << it->second->getDef()->getNumber();
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


bool PhiAssign::search(const Exp &pattern, SharedExp &result) const
{
    if (m_lhs->search(pattern, result)) {
        return true;
    }

    for (const std::shared_ptr<RefExp> &ref : *this) {
        assert(ref->getSubExp1() != nullptr);
        // Note: can't match foo{-} because of this
        if (ref->search(pattern, result)) {
            return true;
        }
    }

    return false;
}


bool PhiAssign::searchAll(const Exp &pattern, std::list<SharedExp> &result) const
{
    // FIXME: is this the right semantics for searching a phi statement,
    // disregarding the RHS?
    return m_lhs->searchAll(pattern, result);
}


bool PhiAssign::searchAndReplace(const Exp &pattern, SharedExp replace, bool /*cc*/)
{
    bool change = false;

    m_lhs = m_lhs->searchReplaceAll(pattern, replace, change);

    for (const std::shared_ptr<RefExp> &refExp : *this) {
        assert(refExp->getSubExp1() != nullptr);
        bool ch;

        // Assume that the definitions will also be replaced
        refExp->setSubExp1(refExp->getSubExp1()->searchReplaceAll(pattern, replace, ch));
        assert(refExp->getSubExp1());
        change |= ch;
    }

    return change;
}


bool PhiAssign::accept(StmtExpVisitor *visitor)
{
    bool visitChildren = true;
    if (!visitor->visit(shared_from_this()->as<PhiAssign>(), visitChildren)) {
        return false;
    }
    else if (!visitChildren) {
        return true;
    }
    else if (m_lhs && !m_lhs->acceptVisitor(visitor->ev)) {
        return false;
    }

    for (const std::shared_ptr<RefExp> &ref : *this) {
        assert(ref->getSubExp1() != nullptr);
        if (!ref->acceptVisitor(visitor->ev)) {
            return false;
        }
    }

    return true;
}


bool PhiAssign::accept(StmtModifier *v)
{
    bool visitChildren;
    v->visit(shared_from_this()->as<PhiAssign>(), visitChildren);

    if (v->m_mod) {
        v->m_mod->clearModified();

        if (visitChildren) {
            m_lhs = m_lhs->acceptModifier(v->m_mod);
        }

        if (v->m_mod->isModified()) {
            LOG_VERBOSE("PhiAssign changed: now %1", shared_from_this());
        }
    }

    return true;
}


bool PhiAssign::accept(StmtPartModifier *v)
{
    bool visitChildren;
    v->visit(shared_from_this()->as<PhiAssign>(), visitChildren);
    v->mod->clearModified();

    if (visitChildren && m_lhs->isMemOf()) {
        m_lhs->setSubExp1(m_lhs->getSubExp1()->acceptModifier(v->mod));
    }

    if (v->mod->isModified()) {
        LOG_VERBOSE("PhiAssign changed: now %1", shared_from_this());
    }

    return true;
}


void PhiAssign::simplify()
{
    m_lhs = m_lhs->simplify();
}


void PhiAssign::putAt(IRFragment *frag, const SharedStmt &def, SharedExp e)
{
    assert(e); // should be something surely

    // Can't use operator[] here since PhiInfo is not default-constructible
    PhiDefs::iterator it = m_defs.find(frag);
    if (it == m_defs.end()) {
        m_defs.insert({ frag, RefExp::get(e, def) });
    }
    else {
        it->second->setDef(def);
        it->second->setSubExp1(e);
    }
}


SharedConstStmt PhiAssign::getStmtAt(IRFragment *idx) const
{
    PhiDefs::const_iterator it = m_defs.find(idx);
    return (it != m_defs.end()) ? it->second->getDef() : nullptr;
}


SharedStmt PhiAssign::getStmtAt(IRFragment *idx)
{
    PhiDefs::iterator it = m_defs.find(idx);
    return (it != m_defs.end()) ? it->second->getDef() : nullptr;
}


void PhiAssign::removeAllReferences(const std::shared_ptr<RefExp> &refExp)
{
    for (PhiDefs::iterator pi = m_defs.begin(); pi != m_defs.end();) {
        std::shared_ptr<RefExp> &p = pi->second;
        assert(p->getSubExp1());

        if (*p == *refExp) {       // Will we ever see this?
            pi = m_defs.erase(pi); // Erase this phi parameter
            continue;
        }

        // Chase the definition
        SharedStmt def = p->getDef();
        if (def && def->isAssign()) {
            SharedExp rhs = def->as<Assign>()->getRight();

            if (*rhs == *refExp) {     // Check if RHS is a single reference to this
                pi = m_defs.erase(pi); // Yes, erase this phi parameter
                continue;
            }
        }

        ++pi; // keep it
    }
}
