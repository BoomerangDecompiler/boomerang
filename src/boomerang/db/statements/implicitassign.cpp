#include "implicitassign.h"

#include "boomerang/util/Log.h"

#include "boomerang/db/exp.h"
#include "boomerang/db/visitor.h"

#include "boomerang/type/type.h"


ImplicitAssign::ImplicitAssign(SharedExp _lhs)
	: Assignment(_lhs)
{
	m_kind = STMT_IMPASSIGN;
}


ImplicitAssign::ImplicitAssign(SharedType ty, SharedExp _lhs)
	: Assignment(ty, _lhs)
{
	m_kind = STMT_IMPASSIGN;
}


ImplicitAssign::ImplicitAssign(ImplicitAssign& o)
	: Assignment(o.m_type ? o.m_type->clone() : nullptr, o.m_lhs->clone())
{
	m_kind = STMT_IMPASSIGN;
}


ImplicitAssign::~ImplicitAssign()
{
}


Instruction *ImplicitAssign::clone() const
{
	ImplicitAssign *ia = new ImplicitAssign(m_type, m_lhs);

	return ia;
}


bool ImplicitAssign::accept(StmtVisitor *visitor)
{
	return visitor->visit(this);
}


void ImplicitAssign::printCompact(QTextStream& os, bool html) const
{
	os << "*" << m_type << "* ";

	if (m_lhs) {
		m_lhs->print(os, html);
	}

	os << " := -";
}


bool ImplicitAssign::search(const Exp& search, SharedExp& result) const
{
	return m_lhs->search(search, result);
}


bool ImplicitAssign::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
	return m_lhs->searchAll(search, result);
}


bool ImplicitAssign::searchAndReplace(const Exp& search, SharedExp replace, bool cc)
{
	Q_UNUSED(cc);
	bool change;
	m_lhs = m_lhs->searchReplaceAll(search, replace, change);
	return change;
}


bool ImplicitAssign::accept(StmtExpVisitor *v)
{
	bool override;
	bool ret = v->visit(this, override);

	if (override) {
		return ret;
	}

	if (ret && m_lhs) {
		ret = m_lhs->accept(v->ev);
	}

	return ret;
}


bool ImplicitAssign::accept(StmtModifier *v)
{
	bool recur;

	v->visit(this, recur);
	v->m_mod->clearMod();

	if (recur) {
		m_lhs = m_lhs->accept(v->m_mod);
	}

	if (VERBOSE && v->m_mod->isMod()) {
		LOG << "ImplicitAssign changed: now " << this << "\n";
	}

	return true;
}


bool ImplicitAssign::accept(StmtPartModifier *v)
{
	bool recur;

	v->visit(this, recur);
	v->mod->clearMod();

	if (recur && m_lhs->isMemOf()) {
		m_lhs->setSubExp1(m_lhs->getSubExp1()->accept(v->mod));
	}

	if (VERBOSE && v->mod->isMod()) {
		LOG << "ImplicitAssign changed: now " << this << "\n";
	}

	return true;
}


/// Data flow based type analysis
void ImplicitAssign::dfaTypeAnalysis(bool& ch)
{
	Assignment::dfaTypeAnalysis(ch);
}
