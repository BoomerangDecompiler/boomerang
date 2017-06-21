#include "branchstatement.h"

#include "core/Boomerang.h"
#include "util/Log.h"

#include "db/basicblock.h"
#include "db/statements/statementhelper.h"
#include "db/visitor.h"


BranchStatement::BranchStatement()
	: m_jumpType((BranchType)0)
	, m_cond(nullptr)
	, m_isFloat(false)
	, m_size(0)
{
	m_kind = STMT_BRANCH;
}


BranchStatement::~BranchStatement()
{
}


void BranchStatement::setCondType(BranchType cond, bool usesFloat /*= false*/)
{
	m_jumpType = cond;
	m_isFloat  = usesFloat;

	// set pCond to a high level representation of this type
	SharedExp p = nullptr;

	switch (cond)
	{
	case BRANCH_JE:
		p = Binary::get(opEquals, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JNE:
		p = Binary::get(opNotEqual, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JSL:
		p = Binary::get(opLess, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JSLE:
		p = Binary::get(opLessEq, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JSGE:
		p = Binary::get(opGtrEq, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JSG:
		p = Binary::get(opGtr, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JUL:
		p = Binary::get(opLessUns, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JULE:
		p = Binary::get(opLessEqUns, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JUGE:
		p = Binary::get(opGtrEqUns, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JUG:
		p = Binary::get(opGtrUns, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JMI:
		p = Binary::get(opLess, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JPOS:
		p = Binary::get(opGtr, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JOF:
		p = Binary::get(opLessUns, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JNOF:
		p = Binary::get(opGtrUns, Terminal::get(opFlags), Const::get(0));
		break;

	case BRANCH_JPAR:
		// Can't handle this properly here; leave an impossible expression involving %flags so propagation will
		// still happen, and we can recognise this later in condToRelational()
		// Update: these expressions seem to get ignored ???
		p = Binary::get(opEquals, Terminal::get(opFlags), Const::get(999));
		break;
	}

	// this is such a hack.. preferably we should actually recognise SUBFLAGS32(..,..,..) > 0 instead of just
	// SUBFLAGS32(..,..,..) but I'll leave this in here for the moment as it actually works.
	if (!Boomerang::get()->noDecompile) {
		p = Terminal::get(usesFloat ? opFflags : opFlags);
	}

	assert(p);
	setCondExpr(p);
}


SharedExp BranchStatement::getCondExpr() const
{
	return m_cond;
}


void BranchStatement::setCondExpr(SharedExp pe)
{
	if (m_cond) {
		// delete pCond;
	}

	m_cond = pe;
}


BasicBlock *BranchStatement::getFallBB() const
{
	ADDRESS a = getFixedDest();

	if (a == NO_ADDRESS) {
		return nullptr;
	}

	if (m_parent == nullptr) {
		return nullptr;
	}

	if (m_parent->getNumOutEdges() != 2) {
		return nullptr;
	}

	if (m_parent->getOutEdge(0)->getLowAddr() == a) {
		return m_parent->getOutEdge(1);
	}

	return m_parent->getOutEdge(0);
}


void BranchStatement::setFallBB(BasicBlock *bb)
{
	ADDRESS a = getFixedDest();

	if (a == NO_ADDRESS) {
		return;
	}

	if (m_parent == nullptr) {
		return;
	}

	if (m_parent->getNumOutEdges() != 2) {
		return;
	}

	if (m_parent->getOutEdge(0)->getLowAddr() == a) {
		m_parent->getOutEdge(1)->deleteInEdge(m_parent);
		m_parent->setOutEdge(1, bb);
		bb->addInEdge(m_parent);
	}
	else {
		m_parent->getOutEdge(0)->deleteInEdge(m_parent);
		m_parent->setOutEdge(0, bb);
		bb->addInEdge(m_parent);
	}
}


BasicBlock *BranchStatement::getTakenBB() const
{
	ADDRESS a = getFixedDest();

	if (a == NO_ADDRESS) {
		return nullptr;
	}

	if (m_parent == nullptr) {
		return nullptr;
	}

	if (m_parent->getNumOutEdges() != 2) {
		return nullptr;
	}

	if (m_parent->getOutEdge(0)->getLowAddr() == a) {
		return m_parent->getOutEdge(0);
	}

	return m_parent->getOutEdge(1);
}


void BranchStatement::setTakenBB(BasicBlock *bb)
{
	ADDRESS a = getFixedDest();

	if (a == NO_ADDRESS) {
		return;
	}

	if (m_parent == nullptr) {
		return;
	}

	if (m_parent->getNumOutEdges() != 2) {
		return;
	}

	if (m_parent->getOutEdge(0)->getLowAddr() == a) {
		m_parent->getOutEdge(0)->deleteInEdge(m_parent);
		m_parent->setOutEdge(0, bb);
		bb->addInEdge(m_parent);
	}
	else {
		m_parent->getOutEdge(1)->deleteInEdge(m_parent);
		m_parent->setOutEdge(1, bb);
		bb->addInEdge(m_parent);
	}
}


bool BranchStatement::search(const Exp& search, SharedExp& result) const
{
	if (m_cond) {
		return m_cond->search(search, result);
	}

	result = nullptr;
	return false;
}


bool BranchStatement::searchAndReplace(const Exp& search, SharedExp replace, bool cc)
{
	GotoStatement::searchAndReplace(search, replace, cc);
	bool change = false;

	if (m_cond) {
		m_cond = m_cond->searchReplaceAll(search, replace, change);
	}

	return change;
}


bool BranchStatement::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
	if (m_cond) {
		return m_cond->searchAll(search, result);
	}

	return false;
}


void BranchStatement::print(QTextStream& os, bool html) const
{
	os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

	if (html) {
		os << "</td><td>";
		os << "<a name=\"stmt" << m_number << "\">";
	}

	os << "BRANCH ";

	if (m_dest == nullptr) {
		os << "*no dest*";
	}
	else if (!m_dest->isIntConst()) {
		os << m_dest;
	}
	else {
		// Really we'd like to display the destination label here...
		os << "0x" << getFixedDest();
	}

	os << ", condition ";

	switch (m_jumpType)
	{
	case BRANCH_JE:
		os << "equals";
		break;

	case BRANCH_JNE:
		os << "not equals";
		break;

	case BRANCH_JSL:
		os << "signed less";
		break;

	case BRANCH_JSLE:
		os << "signed less or equals";
		break;

	case BRANCH_JSGE:
		os << "signed greater or equals";
		break;

	case BRANCH_JSG:
		os << "signed greater";
		break;

	case BRANCH_JUL:
		os << "unsigned less";
		break;

	case BRANCH_JULE:
		os << "unsigned less or equals";
		break;

	case BRANCH_JUGE:
		os << "unsigned greater or equals";
		break;

	case BRANCH_JUG:
		os << "unsigned greater";
		break;

	case BRANCH_JMI:
		os << "minus";
		break;

	case BRANCH_JPOS:
		os << "plus";
		break;

	case BRANCH_JOF:
		os << "overflow";
		break;

	case BRANCH_JNOF:
		os << "no overflow";
		break;

	case BRANCH_JPAR:
		os << "parity";
		break;
	}

	if (m_isFloat) {
		os << " float";
	}

	os << '\n';

	if (m_cond) {
		if (html) {
			os << "<br>";
		}

		os << "High level: ";
		m_cond->print(os, html);
	}

	if (html) {
		os << "</a></td>";
	}
}


Instruction *BranchStatement::clone() const
{
	BranchStatement *ret = new BranchStatement();

	ret->m_dest       = m_dest->clone();
	ret->m_isComputed = m_isComputed;
	ret->m_jumpType   = m_jumpType;
	ret->m_cond       = m_cond ? m_cond->clone() : nullptr;
	ret->m_isFloat    = m_isFloat;
	// Statement members
	ret->m_parent = m_parent;
	ret->m_proc   = m_proc;
	ret->m_number = m_number;
	return ret;
}


bool BranchStatement::accept(StmtVisitor *visitor)
{
	return visitor->visit(this);
}


void BranchStatement::generateCode(HLLCode * /*hll*/, BasicBlock * /*pbb*/, int /*indLevel*/)
{
	// dont generate any code for jconds, they will be handled by the bb
}


bool BranchStatement::usesExp(const Exp& e) const
{
	SharedExp tmp;

	return m_cond && m_cond->search(e, tmp);
}


void BranchStatement::simplify()
{
	if (m_cond) {
		if (condToRelational(m_cond, m_jumpType)) {
			m_isFloat = true;
		}
	}
}


void BranchStatement::genConstraints(LocationSet& cons)
{
	if (m_cond == nullptr) {
		if (VERBOSE) {
			LOG << "Warning: BranchStatment " << m_number << " has no condition expression!\n";
		}

		return;
	}

	SharedType opsType;

	if (m_isFloat) {
		opsType = FloatType::get(0);
	}
	else {
		opsType = IntegerType::get(0);
	}

	if ((m_jumpType == BRANCH_JUGE) || (m_jumpType == BRANCH_JULE) || (m_jumpType == BRANCH_JUG) || (m_jumpType == BRANCH_JUL)) {
		assert(!m_isFloat);
		opsType->as<IntegerType>()->bumpSigned(-1);
	}
	else if ((m_jumpType == BRANCH_JSGE) || (m_jumpType == BRANCH_JSLE) || (m_jumpType == BRANCH_JSG) || (m_jumpType == BRANCH_JSL)) {
		assert(!m_isFloat);
		opsType->as<IntegerType>()->bumpSigned(+1);
	}

	// Constraints leading from the condition
	assert(m_cond->getArity() == 2);
	SharedExp a = m_cond->getSubExp1();
	SharedExp b = m_cond->getSubExp2();
	// Generate constraints for a and b separately (if any).  Often only need a size, since we get basic type and
	// signedness from the branch condition (e.g. jump if unsigned less)
	SharedExp Ta;
	SharedExp Tb;

	if (a->isSizeCast()) {
		opsType->setSize(a->access<Const, 1>()->getInt());
		Ta = Unary::get(opTypeOf, a->getSubExp2());
	}
	else {
		Ta = Unary::get(opTypeOf, a);
	}

	if (b->isSizeCast()) {
		opsType->setSize(b->access<Const, 1>()->getInt());
		Tb = Unary::get(opTypeOf, b->getSubExp2());
	}
	else {
		Tb = Unary::get(opTypeOf, b);
	}

	// Constrain that Ta == opsType and Tb == opsType
	SharedExp con = Binary::get(opEquals, Ta, TypeVal::get(opsType));
	cons.insert(con);
	con = Binary::get(opEquals, Tb, TypeVal::get(opsType));
	cons.insert(con);
}


bool BranchStatement::accept(StmtExpVisitor *v)
{
	bool override;
	bool ret = v->visit(this, override);

	if (override) {
		return ret;
	}

	// Destination will always be a const for X86, so the below will never be used in practice
	if (ret && m_dest) {
		ret = m_dest->accept(v->ev);
	}

	if (ret && m_cond) {
		ret = m_cond->accept(v->ev);
	}

	return ret;
}


bool BranchStatement::accept(StmtPartModifier *v)
{
	bool recur;

	v->visit(this, recur);

	if (m_dest && recur) {
		m_dest = m_dest->accept(v->mod);
	}

	if (m_cond && recur) {
		m_cond = m_cond->accept(v->mod);
	}

	return true;
}


bool BranchStatement::accept(StmtModifier *v)
{
	bool recur;

	v->visit(this, recur);

	if (m_dest && recur) {
		m_dest = m_dest->accept(v->m_mod);
	}

	if (m_cond && recur) {
		m_cond = m_cond->accept(v->m_mod);
	}

	return true;
}


void BranchStatement::dfaTypeAnalysis(bool& ch)
{
	if (m_cond) {
		m_cond->descendType(BooleanType::get(), ch, this);
	}

	// Not fully implemented yet?
}
