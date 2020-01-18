#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "GotoStatement.h"

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"

#include <QTextStreamManipulator>


GotoStatement::GotoStatement(Address jumpDest)
    : Statement(StmtType::Goto)
    , m_isComputed(false)
{
    m_dest = Const::get(jumpDest);
}


GotoStatement::GotoStatement(SharedExp dest)
    : Statement(StmtType::Goto)
    , m_dest(dest)
    , m_isComputed(!dest->isConst())
{
    assert(m_dest != nullptr);
}


GotoStatement::~GotoStatement()
{
}


Address GotoStatement::getFixedDest() const
{
    return m_dest->isIntConst() ? m_dest->access<Const>()->getAddr() : Address::INVALID;
}


void GotoStatement::setDest(SharedExp pd)
{
    m_dest = pd;
    assert(m_dest != nullptr);
}


void GotoStatement::setDest(Address addr)
{
    m_dest = Const::get(addr);
}


SharedExp GotoStatement::getDest()
{
    return m_dest;
}


const SharedExp GotoStatement::getDest() const
{
    return m_dest;
}


bool GotoStatement::search(const Exp &pattern, SharedExp &result) const
{
    return m_dest->search(pattern, result);
}


bool GotoStatement::searchAndReplace(const Exp &pattern, SharedExp replace, bool /*cc*/)
{
    bool change = false;
    m_dest      = m_dest->searchReplaceAll(pattern, replace, change);
    assert(m_dest != nullptr);
    return change;
}


bool GotoStatement::searchAll(const Exp &pattern, std::list<SharedExp> &result) const
{
    return m_dest->searchAll(pattern, result);
}


void GotoStatement::print(OStream &os) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";
    os << "GOTO ";

    if (!m_dest->isIntConst()) {
        m_dest->print(os);
    }
    else {
        os << getFixedDest();
    }
}


void GotoStatement::setIsComputed(bool b)
{
    m_isComputed = b;
}


bool GotoStatement::isComputed() const
{
    return m_isComputed;
}


SharedStmt GotoStatement::clone() const
{
    std::shared_ptr<GotoStatement> ret(new GotoStatement(*this));

    ret->m_dest = m_dest->clone();

    return ret;
}


bool GotoStatement::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
}


void GotoStatement::simplify()
{
    if (isComputed()) {
        m_dest = m_dest->simplifyArith();
        m_dest = m_dest->simplify();
        assert(m_dest != nullptr);
    }
}


bool GotoStatement::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret           = v->visit(shared_from_this()->as<GotoStatement>(), visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret) {
        ret = m_dest->acceptVisitor(v->ev);
    }

    return ret;
}


bool GotoStatement::accept(StmtModifier *v)
{
    bool visitChildren = true;
    v->visit(shared_from_this()->as<GotoStatement>(), visitChildren);

    if (v->m_mod && visitChildren) {
        m_dest = m_dest->acceptModifier(v->m_mod);
        assert(m_dest != nullptr);
    }

    return true;
}


bool GotoStatement::accept(StmtPartModifier *v)
{
    bool visitChildren = true;
    v->visit(shared_from_this()->as<GotoStatement>(), visitChildren);

    if (visitChildren) {
        m_dest = m_dest->acceptModifier(v->mod);
        assert(m_dest != nullptr);
    }

    return true;
}
