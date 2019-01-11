#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BoolAssign.h"

#include "boomerang/ifc/ICodeGenerator.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/StatementHelper.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"


BoolAssign::BoolAssign(int size)
    : Assignment(nullptr)
    , m_jumpType(BranchType::JE)
    , m_cond(nullptr)
    , m_isFloat(false)
    , m_size(size)
{
    m_kind = StmtType::BoolAssign;
}


BoolAssign::~BoolAssign()
{
}


void BoolAssign::setCondType(BranchType cond, bool usesFloat /*= false*/)
{
    m_jumpType = cond;
    m_isFloat  = usesFloat;
    setCondExpr(Terminal::get(opFlags));
}


void BoolAssign::makeSigned()
{
    // Make this into a signed branch
    switch (m_jumpType) {
    case BranchType::JUL: m_jumpType = BranchType::JSL; break;
    case BranchType::JULE: m_jumpType = BranchType::JSLE; break;
    case BranchType::JUGE: m_jumpType = BranchType::JSGE; break;
    case BranchType::JUG: m_jumpType = BranchType::JSG; break;

    default:
        // Do nothing for other cases
        break;
    }
}


SharedExp BoolAssign::getCondExpr() const
{
    return m_cond;
}


void BoolAssign::setCondExpr(SharedExp pss)
{
    m_cond = pss;
}


void BoolAssign::printCompact(OStream &os) const
{
    os << "BOOL ";
    m_lhs->print(os);
    os << " := CC(";

    switch (m_jumpType) {
    case BranchType::JE: os << "equals"; break;
    case BranchType::JNE: os << "not equals"; break;
    case BranchType::JSL: os << "signed less"; break;
    case BranchType::JSLE: os << "signed less or equals"; break;
    case BranchType::JSGE: os << "signed greater or equals"; break;
    case BranchType::JSG: os << "signed greater"; break;
    case BranchType::JUL: os << "unsigned less"; break;
    case BranchType::JULE: os << "unsigned less or equals"; break;
    case BranchType::JUGE: os << "unsigned greater or equals"; break;
    case BranchType::JUG: os << "unsigned greater"; break;
    case BranchType::JMI: os << "minus"; break;
    case BranchType::JPOS: os << "plus"; break;
    case BranchType::JOF: os << "overflow"; break;
    case BranchType::JNOF: os << "no overflow"; break;
    case BranchType::JPAR: os << "ev parity"; break;
    case BranchType::JNPAR: os << "odd parity"; break;
    case BranchType::INVALID: assert(false); break;
    }

    os << ")";

    if (m_isFloat) {
        os << ", float";
    }

    os << '\n';

    if (m_cond) {
        os << "High level: ";
        m_cond->print(os);

        os << "\n";
    }
}


Statement *BoolAssign::clone() const
{
    BoolAssign *ret = new BoolAssign(m_size);

    ret->m_jumpType = m_jumpType;
    ret->m_cond     = (m_cond) ? m_cond->clone() : nullptr;
    ret->m_isFloat  = m_isFloat;
    ret->m_size     = m_size;
    // Statement members
    ret->m_bb     = m_bb;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


bool BoolAssign::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
}


void BoolAssign::simplify()
{
    if (m_cond) {
        condToRelational(m_cond, m_jumpType);
    }
}


void BoolAssign::getDefinitions(LocationSet &defs, bool) const
{
    defs.insert(getLeft());
}


bool BoolAssign::search(const Exp &pattern, SharedExp &result) const
{
    assert(m_lhs);

    if (m_lhs->search(pattern, result)) {
        return true;
    }

    assert(m_cond);
    return m_cond->search(pattern, result);
}


bool BoolAssign::searchAll(const Exp &pattern, std::list<SharedExp> &result) const
{
    bool ch = false;

    assert(m_lhs);

    if (m_lhs->searchAll(pattern, result)) {
        ch = true;
    }

    assert(m_cond);
    return m_cond->searchAll(pattern, result) || ch;
}


bool BoolAssign::searchAndReplace(const Exp &pattern, SharedExp replace, bool cc)
{
    Q_UNUSED(cc);

    assert(m_cond);
    assert(m_lhs);

    bool chl = false, chr = false;
    m_cond = m_cond->searchReplaceAll(pattern, replace, chl);
    m_lhs  = m_lhs->searchReplaceAll(pattern, replace, chr);

    return chl || chr;
}


void BoolAssign::setLeftFromList(const std::list<Statement *> &stmts)
{
    assert(stmts.size() == 1);
    Assign *first = static_cast<Assign *>(stmts.front());
    assert(first->getKind() == StmtType::Assign);

    m_lhs = first->getLeft();
}


bool BoolAssign::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret           = v->visit(this, visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_cond) {
        ret = m_cond->acceptVisitor(v->ev);
    }

    return ret;
}


bool BoolAssign::accept(StmtModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    if (v->m_mod) {
        if (m_cond && visitChildren) {
            m_cond = m_cond->acceptModifier(v->m_mod);
        }

        if (visitChildren && m_lhs->isMemOf()) {
            m_lhs->setSubExp1(m_lhs->getSubExp1()->acceptModifier(v->m_mod));
        }
    }

    return true;
}


bool BoolAssign::accept(StmtPartModifier *v)
{
    bool visitChildren;

    v->visit(this, visitChildren);

    if (m_cond && visitChildren) {
        m_cond = m_cond->acceptModifier(v->mod);
    }

    if (m_lhs && visitChildren) {
        m_lhs = m_lhs->acceptModifier(v->mod);
    }

    return true;
}
