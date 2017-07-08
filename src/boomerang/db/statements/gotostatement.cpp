#include "gotostatement.h"

#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/exp.h"
#include "boomerang/db/visitor.h"


GotoStatement::GotoStatement()
	: m_dest(nullptr)
	, m_isComputed(false)
{
	m_kind = STMT_GOTO;
}


GotoStatement::GotoStatement(Address uDest)
	: m_isComputed(false)
{
	m_kind = STMT_GOTO;
	m_dest = Const::get(uDest);
}


GotoStatement::~GotoStatement()
{
	if (m_dest) {
		// delete pDest;
	}
}


Address GotoStatement::getFixedDest() const
{
	if (m_dest->getOper() != opIntConst) {
		return NO_ADDRESS;
	}

	return constDest()->getAddr();
}


void GotoStatement::setDest(SharedExp pd)
{
	m_dest = pd;
}


void GotoStatement::setDest(Address addr)
{
	// This fails in FrontSparcTest, do you really want it to Mike? -trent
	//    assert(addr >= prog.limitTextLow && addr < prog.limitTextHigh);

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


void GotoStatement::adjustFixedDest(int delta)
{
	// Ensure that the destination is fixed.
	if ((m_dest == nullptr) || (m_dest->getOper() != opIntConst)) {
		LOG << "Can't adjust destination of non-static CTI\n";
	}

	   Address dest = constDest()->getAddr();
	constDest()->setAddr(dest + delta);
}


bool GotoStatement::search(const Exp& search, SharedExp& result) const
{
	result = nullptr;

	if (m_dest) {
		return m_dest->search(search, result);
	}

	return false;
}


bool GotoStatement::searchAndReplace(const Exp& search, SharedExp replace, bool /*cc*/)
{
	bool change = false;

	if (m_dest) {
		m_dest = m_dest->searchReplaceAll(search, replace, change);
	}

	return change;
}


bool GotoStatement::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
	if (m_dest) {
		return m_dest->searchAll(search, result);
	}

	return false;
}


void GotoStatement::print(QTextStream& os, bool html) const
{
	os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

	if (html) {
		os << "</td><td>";
		os << "<a name=\"stmt" << m_number << "\">";
	}

	os << "GOTO ";

	if (m_dest == nullptr) {
		os << "*no dest*";
	}
	else if (m_dest->getOper() != opIntConst) {
		m_dest->print(os);
	}
	else {
		os << "0x" << getFixedDest();
	}

	if (html) {
		os << "</a></td>";
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


Instruction *GotoStatement::clone() const
{
	GotoStatement *ret = new GotoStatement();

	ret->m_dest       = m_dest->clone();
	ret->m_isComputed = m_isComputed;
	// Statement members
	ret->m_parent = m_parent;
	ret->m_proc   = m_proc;
	ret->m_number = m_number;
	return ret;
}


bool GotoStatement::accept(StmtVisitor *visitor)
{
	return visitor->visit(this);
}


void GotoStatement::generateCode(ICodeGenerator * /*hll*/, BasicBlock * /*pbb*/, int /*indLevel*/)
{
	// don't generate any code for jumps, they will be handled by the BB
}


void GotoStatement::simplify()
{
	if (isComputed()) {
		m_dest = m_dest->simplifyArith();
		m_dest = m_dest->simplify();
	}
}


bool GotoStatement::usesExp(const Exp& e) const
{
	SharedExp where;

	return(m_dest->search(e, where));
}


bool GotoStatement::accept(StmtExpVisitor *v)
{
	bool override;
	bool ret = v->visit(this, override);

	if (override) {
		return ret;
	}

	if (ret && m_dest) {
		ret = m_dest->accept(v->ev);
	}

	return ret;
}


bool GotoStatement::accept(StmtModifier *v)
{
	bool recur;

	v->visit(this, recur);

	if (m_dest && recur) {
		m_dest = m_dest->accept(v->m_mod);
	}

	return true;
}


bool GotoStatement::accept(StmtPartModifier *v)
{
	bool recur;

	v->visit(this, recur);

	if (m_dest && recur) {
		m_dest = m_dest->accept(v->mod);
	}

	return true;
}
