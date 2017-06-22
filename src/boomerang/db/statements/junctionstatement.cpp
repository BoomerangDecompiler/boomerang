#include "junctionstatement.h"

#include "boomerang/db/basicblock.h"
#include "boomerang/db/visitor.h"


JunctionStatement::JunctionStatement()
{
	m_kind = STMT_JUNCTION;
}


JunctionStatement::~JunctionStatement()
{
}


bool JunctionStatement::accept(StmtVisitor *visitor)
{
	return visitor->visit(this);
}


bool JunctionStatement::accept(StmtExpVisitor * /*visitor*/)
{
	return true;
}


bool JunctionStatement::accept(StmtModifier * /*visitor*/)
{
	return true;
}


bool JunctionStatement::accept(StmtPartModifier * /*visitor*/)
{
	return true;
}


void JunctionStatement::print(QTextStream& os, bool html) const
{
	os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

	if (html) {
		os << "</td><td>";
		os << "<a name=\"stmt" << m_number << "\">";
	}

	os << "JUNCTION ";

	for (size_t i = 0; i < m_parent->getNumInEdges(); i++) {
		os << m_parent->getInEdges()[i]->getHiAddr();

		if (m_parent->isBackEdge(i)) {
			os << "*";
		}

		os << " ";
	}

	if (isLoopJunction()) {
		os << "LOOP";
	}

// TODO: PassManager::get("RangeAnalysis",this)->printData(os);
//    os << "\n\t\t\tranges: ";
//    Ranges.print(os);
//    if (html)
//        os << "</a></td>";
}


bool JunctionStatement::isLoopJunction() const
{
	for (size_t i = 0; i < m_parent->getNumInEdges(); i++) {
		if (m_parent->isBackEdge(i)) {
			return true;
		}
	}

	return false;
}
