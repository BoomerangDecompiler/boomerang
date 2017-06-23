/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file  basicblock.cpp
 * \brief Implementation of the BasicBlock class.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/
#include "basicblock.h"

#include "boomerang/util/Log.h"

#include "boomerang/db/exp.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/statements/callstatement.h"
#include "boomerang/db/statements/branchstatement.h"
#include "boomerang/db/statements/casestatement.h"
#include "boomerang/db/statements/phiassign.h"
#include "boomerang/db/statements/assign.h"
#include "boomerang/db/visitor.h"

#include "boomerang/codegen/ICodeGenerator.h"

#include "boomerang/type/type.h"

#include "boomerang/util/types.h"
#include "boomerang/util/Util.h"

#include <QtCore/QDebug>
#include <cassert>
#include <algorithm>
#include <cstring>
#include <inttypes.h>


/**********************************
* BasicBlock methods
**********************************/

BasicBlock::BasicBlock(Function *parent)
	: m_targetOutEdges(0)
	, m_inEdgesVisited(0) // From Doug's code
	, m_numForwardInEdges(-1)
	, m_traversed(UNTRAVERSED)
	, m_emitHLLLabel(false)
	, m_indentLevel(0)
	, m_immPDom(nullptr)
	, m_loopHead(nullptr)
	, m_caseHead(nullptr)
	, m_condFollow(nullptr)
	, m_loopFollow(nullptr)
	, m_latchNode(nullptr)
	, m_structuringType(Seq)
	, m_unstructuredType(Structured)
	, m_overlappedRegProcessingDone(false) // others
{
	m_parent = parent;
}


BasicBlock::~BasicBlock()
{
	if (m_listOfRTLs) {
		// Delete the RTLs
		for (RTL *it : *m_listOfRTLs) {
			delete it;
		}

		// and delete the list
		delete m_listOfRTLs;
		m_listOfRTLs = nullptr;
	}
}


BasicBlock::BasicBlock(const BasicBlock& bb)
	: m_nodeType(bb.m_nodeType)
	, m_labelNum(bb.m_labelNum)
	, m_incomplete(bb.m_incomplete) // m_labelNeeded is initialized to false, not copied
	, m_jumpReqd(bb.m_jumpReqd)
	, m_inEdges(bb.m_inEdges)
	, m_outEdges(bb.m_outEdges)
	, m_targetOutEdges(bb.m_targetOutEdges)
	,// From Doug's code
	m_ord(bb.m_ord)
	, m_revOrd(bb.m_revOrd)
	, m_inEdgesVisited(bb.m_inEdgesVisited)
	, m_numForwardInEdges(bb.m_numForwardInEdges)
	, m_traversed(bb.m_traversed)
	, m_emitHLLLabel(bb.m_emitHLLLabel)
	, m_indentLevel(bb.m_indentLevel)
	, m_loopCondType(bb.m_loopCondType)
	, m_structType(bb.m_structType)
	, m_immPDom(bb.m_immPDom)
	, m_loopHead(bb.m_loopHead)
	, m_caseHead(bb.m_caseHead)
	, m_condFollow(bb.m_condFollow)
	, m_loopFollow(bb.m_loopFollow)
	, m_latchNode(bb.m_latchNode)
	, m_structuringType(bb.m_structuringType)
	, m_unstructuredType(bb.m_unstructuredType)
{
	setRTLs(bb.m_listOfRTLs);
	m_parent = bb.m_parent;
}


BasicBlock::BasicBlock(Function *parent, std::list<RTL *> *pRtls, BBTYPE bbType, uint32_t iNumOutEdges)
	: m_nodeType(bbType)
	, m_incomplete(false)
	, m_inEdgesVisited(0) // From Doug's code
	, m_numForwardInEdges(-1)
	, m_traversed(UNTRAVERSED)
	, m_emitHLLLabel(false)
	, m_indentLevel(0)
	, m_immPDom(nullptr)
	, m_loopHead(nullptr)
	, m_caseHead(nullptr)
	, m_condFollow(nullptr)
	, m_loopFollow(nullptr)
	, m_latchNode(nullptr)
	, m_structuringType(Seq)
	, m_unstructuredType(Structured)
	, m_overlappedRegProcessingDone(false) // Others
{
	if (bbType == BBTYPE::TWOWAY) {
		assert(iNumOutEdges >= 2);
	}

	m_outEdges.reserve(iNumOutEdges);    // Reserve the space; values added with AddOutEdge()
	m_parent         = parent;
	m_targetOutEdges = iNumOutEdges;
	// Set the RTLs
	setRTLs(pRtls);
}


bool BasicBlock::isCaseOption()
{
	if (m_caseHead) {
		for (unsigned int i = 0; i < m_caseHead->getNumOutEdges() - 1; i++) {
			if (m_caseHead->getOutEdge(i) == this) {
				return true;
			}
		}
	}

	return false;
}


bool BasicBlock::isTraversed()
{
	return m_traversedMarker;
}


void BasicBlock::setTraversed(bool bTraversed)
{
	m_traversedMarker = bTraversed;
}


void BasicBlock::setRTLs(std::list<RTL *> *rtls)
{
	// should we delete old ones here? breaks some things - trent
	m_listOfRTLs = rtls;

	// Used to set the link between the last instruction (a call) and this BB if this is a call BB
}


BBTYPE BasicBlock::getType()
{
	return m_nodeType;
}


void BasicBlock::updateType(BBTYPE bbType, uint32_t iNumOutEdges)
{
	m_nodeType       = bbType;
	m_targetOutEdges = iNumOutEdges;
	// m_OutEdges.resize(iNumOutEdges);
}


void BasicBlock::setJumpReqd()
{
	m_jumpReqd = true;
}


bool BasicBlock::isJumpReqd()
{
	return m_jumpReqd;
}


char debug_buffer[DEBUG_BUFSIZE];

const char *BasicBlock::prints()
{
	QString     tgt;
	QTextStream ost(&tgt);

	print(ost);
	strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
	debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
	return debug_buffer;
}


void BasicBlock::dump()
{
	QTextStream ost(stderr);

	print(ost);
}


void BasicBlock::print(QTextStream& os, bool html)
{
	if (html) {
		os << "<br>";
	}

	if (m_labelNum) {
		os << "L" << m_labelNum << ": ";
	}

	switch (m_nodeType)
	{
	case BBTYPE::ONEWAY:
		os << "Oneway BB";
		break;

	case BBTYPE::TWOWAY:
		os << "Twoway BB";
		break;

	case BBTYPE::NWAY:
		os << "Nway BB";
		break;

	case BBTYPE::CALL:
		os << "Call BB";
		break;

	case BBTYPE::RET:
		os << "Ret BB";
		break;

	case BBTYPE::FALL:
		os << "Fall BB";
		break;

	case BBTYPE::COMPJUMP:
		os << "Computed jump BB";
		break;

	case BBTYPE::COMPCALL:
		os << "Computed call BB";
		break;

	case BBTYPE::INVALID:
		os << "Invalid BB";
		break;
	}

	os << ":\n";
	os << "in edges: ";

	for (BasicBlock *bb : m_inEdges) {
		os << bb->getHiAddr() << "(" << bb->getLowAddr() << ") ";
	}

	os << "\n";
	os << "out edges: ";

	for (BasicBlock *bb : m_outEdges) {
		os << bb->getLowAddr() << " ";
	}

	os << "\n";

	if (m_listOfRTLs) { // Can be zero if e.g. INVALID
		if (html) {
			os << "<table>\n";
		}

		for (RTL *r : *m_listOfRTLs) {
			r->print(os, html);
		}

		if (html) {
			os << "</table>\n";
		}
	}

	if (m_jumpReqd) {
		if (html) {
			os << "<br>";
		}

		os << "Synthetic out edge(s) to ";

		// assert(TargetOutEdges == OutEdges.size());
		for (BasicBlock *outEdge : m_outEdges) {
			if (outEdge && outEdge->m_labelNum) {
				os << "L" << outEdge->m_labelNum << " ";
			}
		}

		os << '\n';
	}
}


void BasicBlock::printToLog()
{
	QString     tgt;
	QTextStream ost(&tgt);

	print(ost);
	LOG << tgt;
}


bool BasicBlock::isBackEdge(size_t inEdge) const
{
	const BasicBlock *in = m_inEdges[inEdge];

	return this == in || (m_DFTfirst < in->m_DFTfirst && m_DFTlast > in->m_DFTlast);
}


ADDRESS BasicBlock::getLowAddr() const
{
	if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
		return ADDRESS::g(0L);
	}

	ADDRESS a = m_listOfRTLs->front()->getAddress();

	if (a.isZero() && (m_listOfRTLs->size() > 1)) {
		std::list<RTL *>::iterator it = m_listOfRTLs->begin();
		ADDRESS add2 = (*++it)->getAddress();

		// This is a bit of a hack for 286 programs, whose main actually starts at offset 0. A better solution would be
		// to change orphan BBs' addresses to NO_ADDRESS, but I suspect that this will cause many problems. MVE
		if (add2 < ADDRESS::g(0x10)) {
			// Assume that 0 is the real address
			return ADDRESS::g(0L);
		}

		return add2;
	}

	return a;
}


ADDRESS BasicBlock::getHiAddr()
{
	assert(m_listOfRTLs != nullptr);
	return m_listOfRTLs->back()->getAddress();
}


std::list<RTL *> *BasicBlock::getRTLs()
{
	return m_listOfRTLs;
}


const std::list<RTL *> *BasicBlock::getRTLs() const
{
	return m_listOfRTLs;
}


RTL *BasicBlock::getRTLWithStatement(Instruction *stmt)
{
	if (m_listOfRTLs == nullptr) {
		return nullptr;
	}

	for (RTL *rtl : *m_listOfRTLs) {
		for (Instruction *it1 : *rtl) {
			if (it1 == stmt) {
				return rtl;
			}
		}
	}

	return nullptr;
}


std::vector<BasicBlock *>& BasicBlock::getInEdges()
{
	return m_inEdges;
}


const std::vector<BasicBlock *>& BasicBlock::getOutEdges()
{
	return m_outEdges;
}


void BasicBlock::setInEdge(size_t i, BasicBlock *pNewInEdge)
{
	m_inEdges[i] = pNewInEdge;
}


void BasicBlock::setOutEdge(size_t i, BasicBlock *pNewOutEdge)
{
	if (m_outEdges.empty()) {
		assert(i == 0);
		m_outEdges.push_back(pNewOutEdge);       // TODO: why is it allowed to set new edge in empty m_OutEdges array ?
	}
	else {
		assert(i < m_outEdges.size());
		m_outEdges[i] = pNewOutEdge;
	}
}


BasicBlock *BasicBlock::getOutEdge(size_t i)
{
	if (i < m_outEdges.size()) {
		return m_outEdges[i];
	}
	else {
		return nullptr;
	}
}


BasicBlock *BasicBlock::getCorrectOutEdge(ADDRESS a)
{
	for (BasicBlock *it : m_outEdges) {
		if (it->getLowAddr() == a) {
			return it;
		}
	}

	return nullptr;
}


void BasicBlock::addInEdge(BasicBlock *pNewInEdge)
{
	m_inEdges.push_back(pNewInEdge);
}


void BasicBlock::deleteInEdge(std::vector<BasicBlock *>::iterator& it)
{
	it = m_inEdges.erase(it);
}


void BasicBlock::deleteInEdge(BasicBlock *edge)
{
	for (auto it = m_inEdges.begin(); it != m_inEdges.end(); it++) {
		if (*it == edge) {
			deleteInEdge(it);
			break;
		}
	}
}


void BasicBlock::deleteEdge(BasicBlock *edge)
{
	edge->deleteInEdge(this);

	for (auto it = m_outEdges.begin(); it != m_outEdges.end(); it++) {
		if (*it == edge) {
			m_outEdges.erase(it);
			break;
		}
	}
}


unsigned BasicBlock::getDFTOrder(int& first, int& last)
{
	first++;
	m_DFTfirst = first;

	unsigned numTraversed = 1;
	m_traversedMarker = true;

	for (BasicBlock *child : m_outEdges) {
		if (child->m_traversedMarker == false) {
			numTraversed = numTraversed + child->getDFTOrder(first, last);
		}
	}

	last++;
	m_DFTlast = last;

	return numTraversed;
}


unsigned BasicBlock::getRevDFTOrder(int& first, int& last)
{
	first++;
	m_DFTrevfirst = first;

	unsigned numTraversed = 1;
	m_traversedMarker = true;

	for (BasicBlock *parent : m_inEdges) {
		if (parent->m_traversedMarker == false) {
			numTraversed = numTraversed + parent->getRevDFTOrder(first, last);
		}
	}

	last++;
	m_DFTrevlast = last;

	return numTraversed;
}


bool BasicBlock::lessAddress(BasicBlock *bb1, BasicBlock *bb2)
{
	return bb1->getLowAddr() < bb2->getLowAddr();
}


bool BasicBlock::lessFirstDFT(BasicBlock *bb1, BasicBlock *bb2)
{
	return bb1->m_DFTfirst < bb2->m_DFTfirst;
}


bool BasicBlock::lessLastDFT(BasicBlock *bb1, BasicBlock *bb2)
{
	return bb1->m_DFTlast < bb2->m_DFTlast;
}


ADDRESS BasicBlock::getCallDest()
{
	if (m_nodeType != BBTYPE::CALL) {
		return NO_ADDRESS;
	}

	if (m_listOfRTLs->empty()) {
		return NO_ADDRESS;
	}

	RTL *lastRtl = m_listOfRTLs->back();

	for (auto rit = lastRtl->rbegin(); rit != lastRtl->rend(); rit++) {
		if ((*rit)->getKind() == STMT_CALL) {
			return ((CallStatement *)(*rit))->getFixedDest();
		}
	}

	return NO_ADDRESS;
}


Function *BasicBlock::getCallDestProc()
{
	if (m_nodeType != BBTYPE::CALL) {
		return nullptr;
	}

	if (m_listOfRTLs->size() == 0) {
		return nullptr;
	}

	RTL *lastRtl = m_listOfRTLs->back();

	for (auto it = lastRtl->rbegin(); it != lastRtl->rend(); it++) {
		if ((*it)->getKind() == STMT_CALL) {
			return ((CallStatement *)(*it))->getDestProc();
		}
	}

	return nullptr;
}


Instruction *BasicBlock::getFirstStmt(rtlit& rit, StatementList::iterator& sit)
{
	if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
		return nullptr;
	}

	rit = m_listOfRTLs->begin();

	while (rit != m_listOfRTLs->end()) {
		RTL *rtl = *rit;
		sit = rtl->begin();

		if (sit != rtl->end()) {
			return *sit;
		}

		rit++;
	}

	return nullptr;
}


Instruction *BasicBlock::getNextStmt(rtlit& rit, StatementList::iterator& sit)
{
	if (++sit != (*rit)->end()) {
		return *sit; // End of current RTL not reached, so return next
	}

	// Else, find next non-empty RTL & return its first statement
	do {
		if (++rit == m_listOfRTLs->end()) {
			return nullptr;    // End of all RTLs reached, return null Statement
		}
	} while ((*rit)->empty()); // Ignore all RTLs with no statements

	sit = (*rit)->begin();     // Point to 1st statement at start of next RTL
	return *sit;               // Return first statement
}


Instruction *BasicBlock::getPrevStmt(rtlrit& rit, StatementList::reverse_iterator& sit)
{
	if (++sit != (*rit)->rend()) {
		return *sit; // Beginning of current RTL not reached, so return next
	}

	// Else, find prev non-empty RTL & return its last statement
	do {
		if (++rit == m_listOfRTLs->rend()) {
			return nullptr;    // End of all RTLs reached, return null Statement
		}
	} while ((*rit)->empty()); // Ignore all RTLs with no statements

	sit = (*rit)->rbegin();    // Point to last statement at end of prev RTL
	return *sit;               // Return last statement
}


Instruction *BasicBlock::getLastStmt(rtlrit& rit, StatementList::reverse_iterator& sit)
{
	if (m_listOfRTLs == nullptr) {
		return nullptr;
	}

	rit = m_listOfRTLs->rbegin();

	while (rit != m_listOfRTLs->rend()) {
		RTL *rtl = *rit;
		sit = rtl->rbegin();

		if (sit != rtl->rend()) {
			return *sit;
		}

		rit++;
	}

	return nullptr;
}


Instruction *BasicBlock::getFirstStmt()
{
	if (m_listOfRTLs == nullptr) {
		return nullptr;
	}

	for (RTL *rtl : *m_listOfRTLs) {
		if (!rtl->empty()) {
			return rtl->front();
		}
	}

	return nullptr;
}


Instruction *BasicBlock::getLastStmt()
{
	if (m_listOfRTLs == nullptr) {
		return nullptr;
	}

	rtlrit rit = m_listOfRTLs->rbegin();

	while (rit != m_listOfRTLs->rend()) {
		RTL *rtl = *rit;

		if (!rtl->empty()) {
			return rtl->back();
		}

		rit++;
	}

	return nullptr;
}


void BasicBlock::getStatements(StatementList& stmts) const
{
	const std::list<RTL *> *rtls = getRTLs();

	if (!rtls) {
		return;
	}

	for (const RTL *rtl : *rtls) {
		for (Instruction *st : *rtl) {
			if (st->getBB() == nullptr) { // TODO: why statement would have nullptr BB here ?
				st->setBB(const_cast<BasicBlock *>(this));
			}

			stmts.append(st);
		}
	}
}


/*
 * Structuring and code generation.
 *
 * This code is whole heartly based on AST by Doug Simon. Portions may be copyright to him and are available under a BSD
 * style license.
 *
 * Adapted for Boomerang by Trent Waddington, 20 June 2002.
 */
SharedExp BasicBlock::getCond()
{
	// the condition will be in the last rtl
	assert(m_listOfRTLs);
	RTL *last = m_listOfRTLs->back();
	// it should contain a BranchStatement
	BranchStatement *bs = (BranchStatement *)last->getHlStmt();

	if (bs && (bs->getKind() == STMT_BRANCH)) {
		return bs->getCondExpr();
	}

	LOG_VERBOSE(1) << "throwing LastStatementNotABranchError\n";
	throw LastStatementNotABranchError(last->getHlStmt());
}


SharedExp BasicBlock::getDest() noexcept(false)
{
	// The destianation will be in the last rtl
	assert(m_listOfRTLs);
	RTL *lastRtl = m_listOfRTLs->back();
	// It should contain a GotoStatement or derived class
	Instruction   *lastStmt = lastRtl->getHlStmt();
	CaseStatement *cs       = dynamic_cast<CaseStatement *>(lastStmt);

	if (cs) {
		// Get the expression from the switch info
		SWITCH_INFO *si = cs->getSwitchInfo();

		if (si) {
			return si->pSwitchVar;
		}
	}
	else {
		GotoStatement *gs = dynamic_cast<GotoStatement *>(lastStmt);

		if (gs) {
			return gs->getDest();
		}
	}

	LOG_VERBOSE(1) << "throwing LastStatementNotAGotoError\n";
	throw LastStatementNotAGotoError(lastStmt);
}


void BasicBlock::setCond(SharedExp e) noexcept(false)
{
	// the condition will be in the last rtl
	assert(m_listOfRTLs);
	RTL *last = m_listOfRTLs->back();
	// it should contain a BranchStatement
	assert(not last->empty());

	for (auto it = last->rbegin(); it != last->rend(); it++) {
		if ((*it)->getKind() == STMT_BRANCH) {
			((BranchStatement *)(*it))->setCondExpr(e);
			return;
		}
	}

	throw LastStatementNotABranchError(nullptr);
}


bool BasicBlock::isJmpZ(BasicBlock *dest)
{
	// The condition will be in the last rtl
	assert(m_listOfRTLs);
	RTL *last = m_listOfRTLs->back();
	// it should contain a BranchStatement
	assert(not last->empty());

	for (auto it = last->rbegin(); it != last->rend(); it++) {
		if ((*it)->getKind() == STMT_BRANCH) {
			BranchType jt = ((BranchStatement *)(*it))->getCond();

			if ((jt != BRANCH_JE) && (jt != BRANCH_JNE)) {
				return false;
			}

			BasicBlock *trueEdge = m_outEdges[0];

			if (jt == BRANCH_JE) {
				return dest == trueEdge;
			}
			else {
				BasicBlock *falseEdge = m_outEdges[1];
				return dest == falseEdge;
			}
		}
	}

	assert(0);
	return false;
}


BasicBlock *BasicBlock::getLoopBody()
{
	assert(m_structType == PRETESTLOOP || m_structType == POSTTESTLOOP || m_structType == ENDLESSLOOP);
	assert(m_outEdges.size() == 2);

	if (m_outEdges[0] != m_loopFollow) {
		return m_outEdges[0];
	}

	return m_outEdges[1];
}


bool BasicBlock::isAncestorOf(BasicBlock *other)
{
	return((m_loopStamps[0] < other->m_loopStamps[0] && m_loopStamps[1] > other->m_loopStamps[1]) ||
		   (m_revLoopStamps[0] < other->m_revLoopStamps[0] && m_revLoopStamps[1] > other->m_revLoopStamps[1]));

	/*    return (m_DFTfirst < other->m_DFTfirst && m_DFTlast > other->m_DFTlast) ||
	 *  (m_DFTrevlast < other->m_DFTrevlast &&
	 *   m_DFTrevfirst > other->m_DFTrevfirst);*/
}


void BasicBlock::simplify()
{
	if (m_listOfRTLs) {
		for (RTL *elem : *m_listOfRTLs) {
			elem->simplify();
		}
	}

	if (m_nodeType == BBTYPE::TWOWAY) {
		assert(m_outEdges.size() > 1);

		if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
			m_nodeType = BBTYPE::FALL;
		}
		else {
			RTL *last = m_listOfRTLs->back();

			if (last->size() == 0) {
				m_nodeType = BBTYPE::FALL;
			}
			else if (last->back()->isGoto()) {
				m_nodeType = BBTYPE::ONEWAY;
			}
			else if (!last->back()->isBranch()) {
				m_nodeType = BBTYPE::FALL;
			}
		}

		if (m_nodeType == BBTYPE::FALL) {
			// set out edges to be the second one
			if (VERBOSE) {
				LOG << "turning TWOWAY into FALL: " << m_outEdges[0]->getLowAddr() << " " << m_outEdges[1]->getLowAddr()
					<< "\n";
			}

			BasicBlock *redundant = m_outEdges[0];
			m_outEdges[0] = m_outEdges[1];
			m_outEdges.resize(1);
			m_targetOutEdges = 1;
			LOG_VERBOSE(1) << "redundant edge to " << redundant->getLowAddr() << " inedges: ";
			std::vector<BasicBlock *> rinedges = redundant->m_inEdges;
			redundant->m_inEdges.clear();

			for (BasicBlock *redundant_edge : rinedges) {
				LOG_VERBOSE(1) << redundant_edge->getLowAddr() << " ";

				if (redundant_edge != this) {
					redundant->m_inEdges.push_back(redundant_edge);
				}
				else {
					LOG_VERBOSE(1) << "(ignored) ";
				}
			}

			LOG_VERBOSE(1) << "\n";
			// redundant->m_iNumInEdges = redundant->m_InEdges.size();
			LOG_VERBOSE(1) << "   after: " << m_outEdges[0]->getLowAddr() << "\n";
		}

		if (m_nodeType == BBTYPE::ONEWAY) {
			// set out edges to be the first one
			LOG_VERBOSE(1) << "turning TWOWAY into ONEWAY: " << m_outEdges[0]->getLowAddr() << " "
						   << m_outEdges[1]->getLowAddr() << "\n";
			BasicBlock *redundant = m_outEdges[1];
			m_outEdges.resize(1);
			m_targetOutEdges = 1;
			LOG_VERBOSE(1) << "redundant edge to " << redundant->getLowAddr() << " inedges: ";
			std::vector<BasicBlock *> rinedges = redundant->m_inEdges;
			redundant->m_inEdges.clear();

			for (BasicBlock *redundant_edge : rinedges) {
				if (VERBOSE) {
					LOG << redundant_edge->getLowAddr() << " ";
				}

				if (redundant_edge != this) {
					redundant->m_inEdges.push_back(redundant_edge);
				}
				else {
					LOG_VERBOSE(1) << "(ignored) ";
				}
			}

			LOG_VERBOSE(1) << "\n";
			// redundant->m_iNumInEdges = redundant->m_InEdges.size();
			LOG_VERBOSE(1) << "   after: " << m_outEdges[0]->getLowAddr() << "\n";
		}
	}
}


bool BasicBlock::hasBackEdgeTo(BasicBlock *dest)
{
	//    assert(HasEdgeTo(dest) || dest == this);
	return dest == this || dest->isAncestorOf(this);
}


bool BasicBlock::allParentsGenerated()
{
	for (BasicBlock *in : m_inEdges) {
		if (!in->hasBackEdgeTo(this) && (in->m_traversed != DFS_CODEGEN)) {
			return false;
		}
	}

	return true;
}


void BasicBlock::emitGotoAndLabel(ICodeGenerator *hll, int indLevel, BasicBlock *dest)
{
	if (m_loopHead && ((m_loopHead == dest) || (m_loopHead->m_loopFollow == dest))) {
		if (m_loopHead == dest) {
			hll->addContinue(indLevel);
		}
		else {
			hll->addBreak(indLevel);
		}
	}
	else {
		hll->addGoto(indLevel, dest->m_ord);
		dest->m_emitHLLLabel = true;
	}
}


void BasicBlock::WriteBB(ICodeGenerator *hll, int indLevel)
{
	if (DEBUG_GEN) {
		LOG << "Generating code for BB at " << getLowAddr() << "\n";
	}

	// Allocate space for a label to be generated for this node and add this to the generated code. The actual label can
	// then be generated now or back patched later
	hll->addLabel(indLevel, m_ord);

	if (m_listOfRTLs) {
		for (RTL *rtl : *m_listOfRTLs) {
			if (DEBUG_GEN) {
				LOG << rtl->getAddress() << "\t";
			}

			for (Instruction *st : *rtl) {
				st->generateCode(hll, this, indLevel);
			}
		}

		if (DEBUG_GEN) {
			LOG << "\n";
		}
	}

	// save the indentation level that this node was written at
	m_indentLevel = indLevel;
}


void BasicBlock::generateCode_Loop(ICodeGenerator *hll, std::list<BasicBlock *>& gotoSet, int indLevel, UserProc *proc,
								   BasicBlock *latch, std::list<BasicBlock *>& followSet)
{
	// add the follow of the loop (if it exists) to the follow set

	if (m_loopFollow) {
		followSet.push_back(m_loopFollow);
	}

	if (m_loopHeaderType == PreTested) {
		assert(m_latchNode->m_outEdges.size() == 1);

		// write the body of the block (excluding the predicate)
		WriteBB(hll, indLevel);

		// write the 'while' predicate
		SharedExp cond = getCond();

		if (m_outEdges[BTHEN] == m_loopFollow) {
			cond = Unary::get(opNot, cond);
			cond = cond->simplify();
		}

		hll->addPretestedLoopHeader(indLevel, cond);

		// write the code for the body of the loop
		BasicBlock *loopBody = (m_outEdges[BELSE] == m_loopFollow) ? m_outEdges[BTHEN] : m_outEdges[BELSE];
		loopBody->generateCode(hll, indLevel + 1, m_latchNode, followSet, gotoSet, proc);

		// if code has not been generated for the latch node, generate it now
		if (m_latchNode->m_traversed != DFS_CODEGEN) {
			m_latchNode->m_traversed = DFS_CODEGEN;
			m_latchNode->WriteBB(hll, indLevel + 1);
		}

		// rewrite the body of the block (excluding the predicate) at the next nesting level after making sure
		// another label won't be generated
		m_emitHLLLabel = false;
		WriteBB(hll, indLevel + 1);

		// write the loop tail
		hll->addPretestedLoopEnd(indLevel);
	}
	else {
		// write the loop header
		if (m_loopHeaderType == Endless) {
			hll->addEndlessLoopHeader(indLevel);
		}
		else {
			hll->addPostTestedLoopHeader(indLevel);
		}

		// if this is also a conditional header, then generate code for the conditional. Otherwise generate code
		// for the loop body.
		if (m_structuringType == LoopCond) {
			// set the necessary flags so that generateCode can successfully be called again on this node
			m_structuringType = Cond;
			m_traversed       = UNTRAVERSED;
			generateCode(hll, indLevel + 1, m_latchNode, followSet, gotoSet, proc);
		}
		else {
			WriteBB(hll, indLevel + 1);

			// write the code for the body of the loop
			m_outEdges[0]->generateCode(hll, indLevel + 1, m_latchNode, followSet, gotoSet, proc);
		}

		if (m_loopHeaderType == PostTested) {
			// if code has not been generated for the latch node, generate it now
			if (m_latchNode->m_traversed != DFS_CODEGEN) {
				m_latchNode->m_traversed = DFS_CODEGEN;
				m_latchNode->WriteBB(hll, indLevel + 1);
			}

			// hll->AddPosttestedLoopEnd(indLevel, getCond());
			// MVE: the above seems to fail when there is a call in the middle of the loop (so loop is 2 BBs)
			// Just a wild stab:
			hll->addPostTestedLoopEnd(indLevel, m_latchNode->getCond());
		}
		else {
			assert(m_loopHeaderType == Endless);

			// if code has not been generated for the latch node, generate it now
			if (m_latchNode->m_traversed != DFS_CODEGEN) {
				m_latchNode->m_traversed = DFS_CODEGEN;
				m_latchNode->WriteBB(hll, indLevel + 1);
			}

			// write the closing bracket for an endless loop
			hll->addEndlessLoopEnd(indLevel);
		}
	}

	// write the code for the follow of the loop (if it exists)
	if (m_loopFollow) {
		// remove the follow from the follow set
		followSet.resize(followSet.size() - 1);

		if (m_loopFollow->m_traversed != DFS_CODEGEN) {
			m_loopFollow->generateCode(hll, indLevel, latch, followSet, gotoSet, proc);
		}
		else {
			emitGotoAndLabel(hll, indLevel, m_loopFollow);
		}
	}
}


void BasicBlock::generateCode(ICodeGenerator *hll, int indLevel, BasicBlock *latch, std::list<BasicBlock *>& followSet,
							  std::list<BasicBlock *>& gotoSet, UserProc *proc)
{
	// If this is the follow for the most nested enclosing conditional, then don't generate anything. Otherwise if it is
	// in the follow set generate a goto to the follow
	BasicBlock *enclFollow = followSet.size() == 0 ? nullptr : followSet.back();

	if (isIn(gotoSet, this) && !isLatchNode() &&
		((latch && latch->m_loopHead && (this == latch->m_loopHead->m_loopFollow)) || !allParentsGenerated())) {
		emitGotoAndLabel(hll, indLevel, this);
		return;
	}
	else if (isIn(followSet, this)) {
		if (this != enclFollow) {
			emitGotoAndLabel(hll, indLevel, this);
			return;
		}
		else {
			return;
		}
	}

	// Has this node already been generated?
	if (m_traversed == DFS_CODEGEN) {
		// this should only occur for a loop over a single block
		// FIXME: is this true? Perl_list (0x8068028) in the SPEC 2000 perlbmk seems to have a case with sType = Cond,
		// lType == PreTested, and latchNod == 0
		// assert(sType == Loop && lType == PostTested && latchNode == this);
		return;
	}
	else {
		m_traversed = DFS_CODEGEN;
	}

	// if this is a latchNode and the current indentation level is the same as the first node in the loop, then this
	// write out its body and return otherwise generate a goto
	if (isLatchNode()) {
		if (latch && latch->m_loopHead &&
			(indLevel == latch->m_loopHead->m_indentLevel + ((latch->m_loopHead->m_loopHeaderType == PreTested) ? 1 : 0))) {
			WriteBB(hll, indLevel);
			return;
		}
		else {
			// unset its traversed flag
			m_traversed = UNTRAVERSED;

			emitGotoAndLabel(hll, indLevel, this);
			return;
		}
	}

	BasicBlock *child = nullptr;

	switch (m_structuringType)
	{
	case Loop:
	case LoopCond:
		generateCode_Loop(hll, gotoSet, indLevel, proc, latch, followSet);
		break;

	case Cond:
		{
			// reset this back to LoopCond if it was originally of this type
			if (m_latchNode) {
				m_structuringType = LoopCond;
			}

			// for 2 way conditional headers that are effectively jumps into
			// or out of a loop or case body, we will need a new follow node
			BasicBlock *tmpCondFollow = nullptr;

			// keep track of how many nodes were added to the goto set so that
			// the correct number are removed
			int gotoTotal = 0;

			// add the follow to the follow set if this is a case header
			if (m_conditionHeaderType == Case) {
				followSet.push_back(m_condFollow);
			}
			else if ((m_conditionHeaderType != Case) && m_condFollow) {
				// For a structured two conditional header, its follow is
				// added to the follow set
				// myLoopHead = (sType == LoopCond ? this : loopHead);

				if (m_unstructuredType == Structured) {
					followSet.push_back(m_condFollow);
				}

				// Otherwise, for a jump into/outof a loop body, the follow is added to the goto set.
				// The temporary follow is set for any unstructured conditional header branch that is within the
				// same loop and case.
				else {
					if (m_unstructuredType == JumpInOutLoop) {
						// define the loop header to be compared against
						BasicBlock *myLoopHead = (m_structuringType == LoopCond ? this : m_loopHead);
						gotoSet.push_back(m_condFollow);
						gotoTotal++;

						// also add the current latch node, and the loop header of the follow if they exist
						if (latch) {
							gotoSet.push_back(latch);
							gotoTotal++;
						}

						if (m_condFollow->m_loopHead && (m_condFollow->m_loopHead != myLoopHead)) {
							gotoSet.push_back(m_condFollow->m_loopHead);
							gotoTotal++;
						}
					}

					if (m_conditionHeaderType == IfThen) {
						tmpCondFollow = m_outEdges[BELSE];
					}
					else {
						tmpCondFollow = m_outEdges[BTHEN];
					}

					// for a jump into a case, the temp follow is added to the follow set
					if (m_unstructuredType == JumpIntoCase) {
						followSet.push_back(tmpCondFollow);
					}
				}
			}

			// write the body of the block (excluding the predicate)
			WriteBB(hll, indLevel);

			// write the conditional header
			SWITCH_INFO *psi = nullptr; // Init to nullptr to suppress a warning

			if (m_conditionHeaderType == Case) {
				// The CaseStatement will be in the last RTL this BB
				RTL           *last = m_listOfRTLs->back();
				CaseStatement *cs   = (CaseStatement *)last->getHlStmt();
				psi = cs->getSwitchInfo();
				// Write the switch header (i.e. "switch(var) {")
				hll->addCaseCondHeader(indLevel, psi->pSwitchVar);
			}
			else {
				SharedExp cond = getCond();

				if (!cond) {
					cond = Const::get(ADDRESS::g(0xfeedface)); // hack, but better than a crash
				}

				if (m_conditionHeaderType == IfElse) {
					cond = Unary::get(opNot, cond->clone());
					cond = cond->simplify();
				}

				if (m_conditionHeaderType == IfThenElse) {
					hll->addIfElseCondHeader(indLevel, cond);
				}
				else {
					hll->addIfCondHeader(indLevel, cond);
				}
			}

			// write code for the body of the conditional
			if (m_conditionHeaderType != Case) {
				BasicBlock *succ = (m_conditionHeaderType == IfElse ? m_outEdges[BELSE] : m_outEdges[BTHEN]);

				// emit a goto statement if the first clause has already been
				// generated or it is the follow of this node's enclosing loop
				if ((succ->m_traversed == DFS_CODEGEN) || (m_loopHead && (succ == m_loopHead->m_loopFollow))) {
					emitGotoAndLabel(hll, indLevel + 1, succ);
				}
				else {
					succ->generateCode(hll, indLevel + 1, latch, followSet, gotoSet, proc);
				}

				// generate the else clause if necessary
				if (m_conditionHeaderType == IfThenElse) {
					// generate the 'else' keyword and matching brackets
					hll->addIfElseCondOption(indLevel);

					succ = m_outEdges[BELSE];

					// emit a goto statement if the second clause has already
					// been generated
					if (succ->m_traversed == DFS_CODEGEN) {
						emitGotoAndLabel(hll, indLevel + 1, succ);
					}
					else {
						succ->generateCode(hll, indLevel + 1, latch, followSet, gotoSet, proc);
					}

					// generate the closing bracket
					hll->addIfElseCondEnd(indLevel);
				}
				else {
					// generate the closing bracket
					hll->addIfCondEnd(indLevel);
				}
			}
			else { // case header
				   // TODO: linearly emitting each branch of the switch does not result in optimal fall-through.
				   // generate code for each out branch
				for (unsigned int i = 0; i < m_outEdges.size(); i++) {
					// emit a case label
					// FIXME: Not valid for all switch types
					Const caseVal(0);

					if (psi->chForm == 'F') {                            // "Fortran" style?
						caseVal.setInt(((int *)psi->uTable.m_value)[i]); // Yes, use the table value itself
					}
					// Note that uTable has the address of an int array
					else {
						caseVal.setInt((int)(psi->iLower + i));
					}

					hll->addCaseCondOption(indLevel, caseVal);

					// generate code for the current out-edge
					BasicBlock *succ = m_outEdges[i];

					// assert(succ->caseHead == this || succ == condFollow || HasBackEdgeTo(succ));
					if (succ->m_traversed == DFS_CODEGEN) {
						emitGotoAndLabel(hll, indLevel + 1, succ);
					}
					else {
						succ->generateCode(hll, indLevel + 1, latch, followSet, gotoSet, proc);
					}
				}

				// generate the closing bracket
				hll->addCaseCondEnd(indLevel);
			}

			// do all the follow stuff if this conditional had one
			if (m_condFollow) {
				// remove the original follow from the follow set if it was
				// added by this header
				if ((m_unstructuredType == Structured) || (m_unstructuredType == JumpIntoCase)) {
					assert(gotoTotal == 0);
					followSet.resize(followSet.size() - 1);
				}
				else { // remove all the nodes added to the goto set
					for (int i = 0; i < gotoTotal; i++) {
						gotoSet.resize(gotoSet.size() - 1);
					}
				}

				// do the code generation (or goto emitting) for the new conditional follow if it exists, otherwise do
				// it for the original follow
				if (!tmpCondFollow) {
					tmpCondFollow = m_condFollow;
				}

				if (tmpCondFollow->m_traversed == DFS_CODEGEN) {
					emitGotoAndLabel(hll, indLevel, tmpCondFollow);
				}
				else {
					tmpCondFollow->generateCode(hll, indLevel, latch, followSet, gotoSet, proc);
				}
			}

			break;
		}

	case Seq:
		// generate code for the body of this block
		WriteBB(hll, indLevel);

		// return if this is the 'return' block (i.e. has no out edges) after emmitting a 'return' statement
		if (getType() == BBTYPE::RET) {
			// This should be emited now, like a normal statement
			// hll->AddReturnStatement(indLevel, getReturnVal());
			return;
		}

		// return if this doesn't have any out edges (emit a warning)
		if (m_outEdges.empty()) {
			QTextStream q_cerr(stderr);
			q_cerr << "WARNING: no out edge for this BB in " << proc->getName() << ":\n";
			this->print(q_cerr);
			q_cerr << '\n';

			if (m_nodeType == BBTYPE::COMPJUMP) {
				QString     dat;
				QTextStream ost(&dat);
				assert(m_listOfRTLs->size());
				RTL *lastRTL = m_listOfRTLs->back();
				assert(!lastRTL->empty());
				GotoStatement *gs = (GotoStatement *)lastRTL->back();
				ost << "goto " << gs->getDest();
				hll->addLineComment(dat);
			}

			return;
		}

		child = m_outEdges[0];

		if (m_outEdges.size() != 1) {
			BasicBlock *other = m_outEdges[1];
			LOG << "found seq with more than one outedge!\n";
			auto const_dest = std::static_pointer_cast<Const>(getDest());

			if (const_dest->isIntConst() && (const_dest->getAddr() == child->getLowAddr())) {
				other = child;
				child = m_outEdges[1];
				LOG << "taken branch is first out edge\n";
			}

			try {
				hll->addIfCondHeader(indLevel, getCond());

				if (other->m_traversed == DFS_CODEGEN) {
					emitGotoAndLabel(hll, indLevel + 1, other);
				}
				else {
					other->generateCode(hll, indLevel + 1, latch, followSet, gotoSet, proc);
				}

				hll->addIfCondEnd(indLevel);
			}
			catch (LastStatementNotABranchError&) {
				LOG << "last statement is not a cond, don't know what to do with this.\n";
			}
		}

		// generate code for its successor if it hasn't already been visited and is in the same loop/case and is not
		// the latch for the current most enclosing loop.     The only exception for generating it when it is not in
		// the same loop is when it is only reached from this node
		if ((child->m_traversed == DFS_CODEGEN) ||
			((child->m_loopHead != m_loopHead) && (!child->allParentsGenerated() || isIn(followSet, child))) ||
			(latch && latch->m_loopHead && (latch->m_loopHead->m_loopFollow == child)) ||
			!((m_caseHead == child->m_caseHead) || (m_caseHead && (child == m_caseHead->m_condFollow)))) {
			emitGotoAndLabel(hll, indLevel, child);
		}
		else {
			if (m_caseHead && (child == m_caseHead->m_condFollow)) {
				// generate the 'break' statement
				hll->addCaseCondOptionEnd(indLevel);
			}
			else if ((m_caseHead == nullptr) || (m_caseHead != child->m_caseHead) || !child->isCaseOption()) {
				child->generateCode(hll, indLevel, latch, followSet, gotoSet, proc);
			}
		}

		break;

	default:
		LOG_STREAM() << "unhandled sType " << (int)m_structuringType << "\n";
	}
}


Function *BasicBlock::getDestProc()
{
	// The last Statement of the last RTL should be a CallStatement
	CallStatement *call = (CallStatement *)(m_listOfRTLs->back()->getHlStmt());

	assert(call->getKind() == STMT_CALL);
	Function *proc = call->getDestProc();

	if (proc == nullptr) {
		LOG_STREAM() << "Indirect calls not handled yet\n";
		assert(false);
	}

	return proc;
}


void BasicBlock::setLoopStamps(int& time, std::vector<BasicBlock *>& order)
{
	// timestamp the current node with the current time and set its traversed
	// flag
	m_traversed     = DFS_LNUM;
	m_loopStamps[0] = time;

	// recurse on unvisited children and set inedges for all children
	for (BasicBlock *out : m_outEdges) {
		// set the in edge from this child to its parent (the current node)
		// (not done here, might be a problem)
		// outEdges[i]->inEdges.Add(this);

		// recurse on this child if it hasn't already been visited
		if (out->m_traversed != DFS_LNUM) {
			out->setLoopStamps(++time, order);
		}
	}

	// set the the second loopStamp value
	m_loopStamps[1] = ++time;

	// add this node to the ordering structure as well as recording its position within the ordering
	m_ord = (int)order.size();
	order.push_back(this);
}


void BasicBlock::setRevLoopStamps(int& time)
{
	// timestamp the current node with the current time and set its traversed flag
	m_traversed        = DFS_RNUM;
	m_revLoopStamps[0] = time;

	// recurse on the unvisited children in reverse order
	for (int i = (int)m_outEdges.size() - 1; i >= 0; i--) {
		// recurse on this child if it hasn't already been visited
		if (m_outEdges[i]->m_traversed != DFS_RNUM) {
			m_outEdges[i]->setRevLoopStamps(++time);
		}
	}

	// set the the second loopStamp value
	m_revLoopStamps[1] = ++time;
}


void BasicBlock::setRevOrder(std::vector<BasicBlock *>& order)
{
	// Set this node as having been traversed during the post domimator DFS ordering traversal
	m_traversed = DFS_PDOM;

	// recurse on unvisited children
	for (BasicBlock *in : m_inEdges) {
		if (in->m_traversed != DFS_PDOM) {
			in->setRevOrder(order);
		}
	}

	// add this node to the ordering structure and record the post dom. order of this node as its index within this
	// ordering structure
	m_revOrd = (int)order.size();
	order.push_back(this);
}


void BasicBlock::setCaseHead(BasicBlock *head, BasicBlock *follow)
{
	assert(!m_caseHead);

	m_traversed = DFS_CASE;

	// don't tag this node if it is the case header under investigation
	if (this != head) {
		m_caseHead = head;
	}

	// if this is a nested case header, then it's member nodes will already have been tagged so skip straight to its
	// follow
	if ((getType() == BBTYPE::NWAY) && (this != head)) {
		if (m_condFollow && (m_condFollow->m_traversed != DFS_CASE) && (m_condFollow != follow)) {
			m_condFollow->setCaseHead(head, follow);
		}
	}
	else {
		// traverse each child of this node that:
		//   i) isn't on a back-edge,
		//  ii) hasn't already been traversed in a case tagging traversal and,
		// iii) isn't the follow node.
		for (BasicBlock *out : m_outEdges) {
			if (!hasBackEdgeTo(out) && (out->m_traversed != DFS_CASE) && (out != follow)) {
				out->setCaseHead(head, follow);
			}
		}
	}
}


void BasicBlock::setStructType(structType s)
{
	// if this is a conditional header, determine exactly which type of conditional header it is (i.e. switch, if-then,
	// if-then-else etc.)
	if (s == Cond) {
		if (getType() == BBTYPE::NWAY) {
			m_conditionHeaderType = Case;
		}
		else if (m_outEdges[BELSE] == m_condFollow) {
			m_conditionHeaderType = IfThen;
		}
		else if (m_outEdges[BTHEN] == m_condFollow) {
			m_conditionHeaderType = IfElse;
		}
		else {
			m_conditionHeaderType = IfThenElse;
		}
	}

	m_structuringType = s;
}


void BasicBlock::setUnstructType(unstructType us)
{
	assert((m_structuringType == Cond || m_structuringType == LoopCond) && m_conditionHeaderType != Case);
	m_unstructuredType = us;
}


unstructType BasicBlock::getUnstructType()
{
	assert((m_structuringType == Cond || m_structuringType == LoopCond) && m_conditionHeaderType != Case);
	return m_unstructuredType;
}


void BasicBlock::setLoopType(LoopType l)
{
	assert(m_structuringType == Loop || m_structuringType == LoopCond);
	m_loopHeaderType = l;

	// set the structured class (back to) just Loop if the loop type is PreTested OR it's PostTested and is a single
	// block loop
	if ((m_loopHeaderType == PreTested) || ((m_loopHeaderType == PostTested) && (this == m_latchNode))) {
		m_structuringType = Loop;
	}
}


LoopType BasicBlock::getLoopType()
{
	assert(m_structuringType == Loop || m_structuringType == LoopCond);
	return m_loopHeaderType;
}


void BasicBlock::setCondType(CondType c)
{
	assert(m_structuringType == Cond || m_structuringType == LoopCond);
	m_conditionHeaderType = c;
}


CondType BasicBlock::getCondType()
{
	assert(m_structuringType == Cond || m_structuringType == LoopCond);
	return m_conditionHeaderType;
}


bool BasicBlock::inLoop(BasicBlock *header, BasicBlock *latch)
{
	assert(header->m_latchNode == latch);
	assert(header == latch ||
		   ((header->m_loopStamps[0] > latch->m_loopStamps[0] && latch->m_loopStamps[1] > header->m_loopStamps[1]) ||
			(header->m_loopStamps[0] < latch->m_loopStamps[0] && latch->m_loopStamps[1] < header->m_loopStamps[1])));
	// this node is in the loop if it is the latch node OR
	// this node is within the header and the latch is within this when using the forward loop stamps OR
	// this node is within the header and the latch is within this when using the reverse loop stamps
	return this == latch || (header->m_loopStamps[0] < m_loopStamps[0] && m_loopStamps[1] < header->m_loopStamps[1] &&
							 m_loopStamps[0] < latch->m_loopStamps[0] && latch->m_loopStamps[1] < m_loopStamps[1]) ||
		   (header->m_revLoopStamps[0] < m_revLoopStamps[0] && m_revLoopStamps[1] < header->m_revLoopStamps[1] &&
			m_revLoopStamps[0] < latch->m_revLoopStamps[0] && latch->m_revLoopStamps[1] < m_revLoopStamps[1]);
}


char *BasicBlock::getStmtNumber()
{
	static char ret[12];
	Instruction *first = getFirstStmt();

	if (first) {
		sprintf(ret, "%d", first->getNumber());
	}
	else {
		sprintf(ret, "bb%" PRIxPTR, ADDRESS::value_type(this));
	}

	return ret;
}


void BasicBlock::prependStmt(Instruction *s, UserProc *proc)
{
	assert(m_parent == proc);
	// Check the first RTL (if any)
	assert(m_listOfRTLs);
	s->setBB(this);
	s->setProc(proc);

	if (!m_listOfRTLs->empty()) {
		RTL *rtl = m_listOfRTLs->front();

		if (rtl->getAddress().isZero()) {
			// Append to this RTL
			rtl->appendStmt(s);
			return;
		}
	}

	// Otherwise, prepend a new RTL
	std::list<Instruction *> listStmt = { s };
	RTL *rtl = new RTL(ADDRESS::g(0L), &listStmt);
	m_listOfRTLs->push_front(rtl);
}


////////////////////////////////////////////////////

// Check for overlap of liveness between the currently live locations (liveLocs) and the set of locations in ls
// Also check for type conflicts if DFA_TYPE_ANALYSIS
// This is a helper function that is not directly declared in the BasicBlock class
void checkForOverlap(LocationSet& liveLocs, LocationSet& ls, ConnectionGraph& ig, UserProc * /*proc*/)
{
	// For each location to be considered
	for (SharedExp u : ls) {
		if (!u->isSubscript()) {
			continue; // Only interested in subscripted vars
		}

		auto r = std::static_pointer_cast<RefExp>(u);
		// Interference if we can find a live variable which differs only in the reference
		SharedExp dr;

		if (liveLocs.findDifferentRef(r, dr)) {
			assert(dr->access<RefExp>()->getDef() != nullptr);
			assert(u->access<RefExp>()->getDef() != nullptr);
			// We have an interference between r and dr. Record it
			ig.connect(r, dr);

			if (VERBOSE || DEBUG_LIVENESS) {
				LOG << "interference of " << dr << " with " << r << "\n";
			}
		}

		// Add the uses one at a time. Note: don't use makeUnion, because then we don't discover interferences
		// from the same statement, e.g.  blah := r24{2} + r24{3}
		liveLocs.insert(u);
	}
}


bool BasicBlock::calcLiveness(ConnectionGraph& ig, UserProc *myProc)
{
	// Start with the liveness at the bottom of the BB
	LocationSet liveLocs, phiLocs;

	getLiveOut(liveLocs, phiLocs);
	// Do the livensses that result from phi statements at successors first.
	// FIXME: document why this is necessary
	checkForOverlap(liveLocs, phiLocs, ig, myProc);
	// For each RTL in this BB
	std::list<RTL *>::reverse_iterator rit;

	if (m_listOfRTLs) { // this can be nullptr
		for (rit = m_listOfRTLs->rbegin(); rit != m_listOfRTLs->rend(); ++rit) {
			std::list<Instruction *>::reverse_iterator sit;

			// For each statement this RTL
			for (sit = (*rit)->rbegin(); sit != (*rit)->rend(); ++sit) {
				Instruction *s = *sit;
				LocationSet defs;
				s->getDefinitions(defs);
				// The definitions don't have refs yet
				defs.addSubscript(s /* , myProc->getCFG() */);
#if 0
				// I used to think it necessary to consider definitions as a special case. However, I now believe that
				// this was either an error of implementation (e.g. it didn't seem to correctly consider the livenesses
				// causesd by phis) or something to do with renaming but not propagating certain memory locations.
				// The idea is now to clearly divide locations into those that can be renamed and propagated, and those
				// which are not renamed or propagated. (Check this.)

				// Also consider it an interference if we define a location that is the same base variable. This can happen
				// when there is a definition that is unused but for whatever reason not eliminated
				// This check is done at the "bottom" of the statement, i.e. before we add s's uses and remove s's
				// definitions to liveLocs
				// Note that phi assignments don't count
				if (!s->isPhi()) {
					checkForOverlap(liveLocs, defs, ig, myProc, false);
				}
#endif
				// Definitions kill uses. Now we are moving to the "top" of statement s
				liveLocs.makeDiff(defs);

				// Phi functions are a special case. The operands of phi functions are uses, but they don't interfere
				// with each other (since they come via different BBs). However, we don't want to put these uses into
				// liveLocs, because then the livenesses will flow to all predecessors. Only the appropriate livenesses
				// from the appropriate phi parameter should flow to the predecessor. This is done in getLiveOut()
				if (s->isPhi()) {
					continue;
				}

				// Check for livenesses that overlap
				LocationSet uses;
				s->addUsedLocs(uses);
				checkForOverlap(liveLocs, uses, ig, myProc);

				if (DEBUG_LIVENESS) {
					LOG << " ## liveness: at top of " << s << ", liveLocs is " << liveLocs.prints() << "\n";
				}
			}
		}
	}

	// liveIn is what we calculated last time
	if (!(liveLocs == m_liveIn)) {
		m_liveIn = liveLocs;
		return true; // A change
	}

	// No change
	return false;
}


void BasicBlock::getLiveOut(LocationSet& liveout, LocationSet& phiLocs)
{
	Cfg *cfg(((UserProc *)m_parent)->getCFG());

	liveout.clear();

	for (BasicBlock *currBB : m_outEdges) {
		// First add the non-phi liveness
		liveout.makeUnion(currBB->m_liveIn); // add successor liveIn to this liveout set.

		// The first RTL will have the phi functions, if any
		if ((currBB->m_listOfRTLs == nullptr) || (currBB->m_listOfRTLs->size() == 0)) {
			continue;
		}

		RTL *phiRtl = currBB->m_listOfRTLs->front();

		for (Instruction *st : *phiRtl) {
			// Only interested in phi assignments. Note that it is possible that some phi assignments have been
			// converted to ordinary assignments. So the below is a continue, not a break.
			if (!st->isPhi()) {
				continue;
			}

			PhiAssign *pa = (PhiAssign *)st;

			for (std::pair<const BasicBlock *, PhiInfo> v : pa->getDefs()) {
				if (-1 != cfg->pbbToIndex(v.first)) {
					qDebug() << "Someone removed BB that defined the PHI! Need to update PhiAssign defs";
				}
			}

			// Get the jth operand to the phi function; it has a use from BB *this
			// assert(j>=0);
			Instruction *def = pa->getStmtAt(this);

			if (!def) {
				std::deque<BasicBlock *> to_visit(m_inEdges.begin(), m_inEdges.end());
				std::set<BasicBlock *> tried{
					this
				};

				// TODO: this looks like a hack ?  but sometimes PhiAssign has value which is defined in parent of
				// 'this'
				//  BB1 1  - defines r20
				//  BB2 33 - transfers control to BB3
				//  BB3 40 - r10 = phi { 1 }
				while (!to_visit.empty()) {
					BasicBlock *pbb = to_visit.back();

					if (tried.find(pbb) != tried.end()) {
						to_visit.pop_back();
						continue;
					}

					def = pa->getStmtAt(pbb);

					if (def) {
						break;
					}

					tried.insert(pbb);
					to_visit.pop_back();

					for (BasicBlock *bb : pbb->m_inEdges) {
						if (tried.find(bb) != tried.end()) { // already tried
							continue;
						}

						to_visit.push_back(bb);
					}
				}
			}

			SharedExp r = RefExp::get(pa->getLeft()->clone(), def);
			assert(def);
			liveout.insert(r);
			phiLocs.insert(r);

			if (DEBUG_LIVENESS) {
				LOG << " ## Liveness: adding " << r << " due to ref to phi " << st << " in BB at " << getLowAddr()
					<< "\n";
			}
		}
	}
}


int BasicBlock::whichPred(BasicBlock *pred)
{
	int n = m_inEdges.size();

	for (int i = 0; i < n; i++) {
		if (m_inEdges[i] == pred) {
			return i;
		}
	}

	assert(false);
	return -1;
}


//    //    //    //    //    //    //    //    //    //    //    //    //
//                                                //
//         Indirect jump and call analyses        //
//                                                //
//    //    //    //    //    //    //    //    //    //    //    //    //

// Switch High Level patterns

// With array processing, we get a new form, call it form 'a' (don't confuse with form 'A'):
// Pattern: <base>{}[<index>]{} where <index> could be <var> - <Kmin>
// TODO: use initializer lists
static SharedExp forma =
	RefExp::get(Binary::get(opArrayIndex, RefExp::get(Terminal::get(opWild), (Instruction *)-1), Terminal::get(opWild)),
				(Instruction *)-1);

// Pattern: m[<expr> * 4 + T ]
static SharedExp formA = Location::memOf(
	Binary::get(opPlus, Binary::get(opMult, Terminal::get(opWild), Const::get(4)), Terminal::get(opWildIntConst)));

// With array processing, we get a new form, call it form 'o' (don't confuse with form 'O'):
// Pattern: <base>{}[<index>]{} where <index> could be <var> - <Kmin>
// NOT COMPLETED YET!
static SharedExp formo =
	RefExp::get(Binary::get(opArrayIndex, RefExp::get(Terminal::get(opWild), (Instruction *)-1), Terminal::get(opWild)),
				(Instruction *)-1);

// Pattern: m[<expr> * 4 + T ] + T
static SharedExp formO =
	Binary::get(opPlus, Location::memOf(Binary::get(opPlus, Binary::get(opMult, Terminal::get(opWild), Const::get(4)),
													Terminal::get(opWildIntConst))),
				Terminal::get(opWildIntConst));

// Pattern: %pc + m[%pc     + (<expr> * 4) + k]
// where k is a small constant, typically 28 or 20
static SharedExp formR = Binary::get(
	opPlus, Terminal::get(opPC),
	Location::memOf(Binary::get(
						opPlus, Terminal::get(opPC),
						Binary::get(opPlus, Binary::get(opMult, Terminal::get(opWild), Const::get(4)), Const::get(opWildIntConst)))));

// Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
// where k is a smallish constant, e.g. 288 (/usr/bin/vi 2.6, 0c4233c).
static SharedExp formr = Binary::get(
	opPlus, Terminal::get(opPC),
	Location::memOf(Binary::get(opPlus, Terminal::get(opPC),
								Binary::get(opMinus, Binary::get(opMult, Terminal::get(opWild), Const::get(4)),
											Terminal::get(opWildIntConst)))));

struct SwitchForm
{
	SharedConstExp pattern;
	char           type;
};

SwitchForm hlForms[] =
{
	{ forma, 'a' },
	{ formA, 'A' },
	{ formo, 'o' },
	{ formO, 'O' },
	{ formR, 'R' },
	{ formr, 'r' }
};
// Vcall high level patterns
// Pattern 0: global<wild>[0]
static SharedExp vfc_funcptr =
	Binary::get(opArrayIndex, Location::get(opGlobal, Terminal::get(opWildStrConst), nullptr), Const::get(0));

// Pattern 1: m[ m[ <expr> + K1 ] + K2 ]
// K1 is vtable offset, K2 is virtual function offset (could come from m[A2], if A2 is in read-only memory
static SharedExp vfc_both = Location::memOf(
	Binary::get(opPlus, Location::memOf(Binary::get(opPlus, Terminal::get(opWild), Terminal::get(opWildIntConst))),
				Terminal::get(opWildIntConst)));

// Pattern 2: m[ m[ <expr> ] + K2]
static SharedExp vfc_vto =
	Location::memOf(Binary::get(opPlus, Location::memOf(Terminal::get(opWild)), Terminal::get(opWildIntConst)));

// Pattern 3: m[ m[ <expr> + K1] ]
static SharedExp vfc_vfo =
	Location::memOf(Location::memOf(Binary::get(opPlus, Terminal::get(opWild), Terminal::get(opWildIntConst))));

// Pattern 4: m[ m[ <expr> ] ]
static SharedExp vfc_none = Location::memOf(Location::memOf(Terminal::get(opWild)));

static SharedExp hlVfc[] = { vfc_funcptr, vfc_both, vfc_vto, vfc_vfo, vfc_none };

void findSwParams(char form, SharedExp e, SharedExp& expr, ADDRESS& T)
{
	switch (form)
	{
	case 'a':
		{
			// Pattern: <base>{}[<index>]{}
			e = e->getSubExp1();
			SharedExp base = e->getSubExp1();

			if (base->isSubscript()) {
				base = base->getSubExp1();
			}

			auto     con     = base->access<Const, 1>();
			QString  gloName = con->getStr();
			UserProc *p      = std::static_pointer_cast<Location>(base)->getProc();
			Prog     *prog   = p->getProg();
			T    = prog->getGlobalAddr(gloName);
			expr = e->getSubExp2();
			break;
		}

	case 'A':
		{
			// Pattern: m[<expr> * 4 + T ]
			if (e->isSubscript()) {
				e = e->getSubExp1();
			}

			// b will be (<expr> * 4) + T
			SharedExp b = e->getSubExp1();
			T    = b->access<Const, 2>()->getAddr();
			b    = b->getSubExp1(); // b is now <expr> * 4
			expr = b->getSubExp1();
			break;
		}

	case 'O':
		{       // Form O
			// Pattern: m[<expr> * 4 + T ] + T
			T = e->access<Const, 2>()->getAddr();
			// l = m[<expr> * 4 + T ]:
			SharedExp l = e->getSubExp1();

			if (l->isSubscript()) {
				l = l->getSubExp1();
			}

			// b = <expr> * 4 + T:
			SharedExp b = l->getSubExp1();
			// b = <expr> * 4:
			b = b->getSubExp1();
			// expr = <expr>:
			expr = b->getSubExp1();
			break;
		}

	case 'R':
		{
			// Pattern: %pc + m[%pc     + (<expr> * 4) + k]
			T = ADDRESS::g(0L); // ?
			// l = m[%pc  + (<expr> * 4) + k]:
			SharedExp l = e->getSubExp2();

			if (l->isSubscript()) {
				l = l->getSubExp1();
			}

			// b = %pc    + (<expr> * 4) + k:
			SharedExp b = l->getSubExp1();
			// b = (<expr> * 4) + k:
			b = b->getSubExp2();
			// b = <expr> * 4:
			b = b->getSubExp1();
			// expr = <expr>:
			expr = b->getSubExp1();
			break;
		}

	case 'r':
		{
			// Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
			T = ADDRESS::g(0L); // ?
			// b = %pc + m[%pc + ((<expr> * 4) - k)]:
			SharedExp b = e->getSubExp1();
			// l = m[%pc + ((<expr> * 4) - k)]:
			SharedExp l = b->getSubExp2();

			if (l->isSubscript()) {
				l = l->getSubExp1();
			}

			// b = %pc + ((<expr> * 4) - k)
			b = l->getSubExp1();
			// b = ((<expr> * 4) - k):
			b = b->getSubExp2();
			// b = <expr> * 4:
			b = b->getSubExp1();
			// expr = <expr>
			expr = b->getSubExp1();
			break;
		}

	default:
		expr = nullptr;
		T    = NO_ADDRESS;
	}

	// normalize address to native
	T = T.native();
}


int BasicBlock::findNumCases()
{
	// should actually search from the statement to i
	for (BasicBlock *in : m_inEdges) {          // For each in-edge
		if (in->m_nodeType != BBTYPE::TWOWAY) { // look for a two-way BB
			continue;                           // Ignore all others
		}

		assert(in->m_listOfRTLs && in->m_listOfRTLs->size());
		RTL *lastRtl = in->m_listOfRTLs->back();
		assert(not lastRtl->empty());
		BranchStatement *lastStmt = (BranchStatement *)lastRtl->back();
		SharedExp       pCond     = lastStmt->getCondExpr();

		if (pCond->getArity() != 2) {
			continue;
		}

		SharedExp rhs = pCond->getSubExp2();

		if (!rhs->isIntConst()) {
			continue;
		}

		int  k  = std::static_pointer_cast<Const>(rhs)->getInt();
		OPER op = pCond->getOper();

		if ((op == opGtr) || (op == opGtrUns)) {
			return k + 1;
		}

		if ((op == opGtrEq) || (op == opGtrEqUns)) {
			return k;
		}

		if ((op == opLess) || (op == opLessUns)) {
			return k;
		}

		if ((op == opLessEq) || (op == opLessEqUns)) {
			return k + 1;
		}
	}

	LOG << "Could not find number of cases for n-way at address " << getLowAddr() << "\n";
	return 3; // Bald faced guess if all else fails
}


/// Find all the possible constant values that the location defined by s could be assigned with
static void findConstantValues(const Instruction *s, std::list<int>& dests)
{
	if (s == nullptr) {
		return;
	}

	if (s->isPhi()) {
		// For each definition, recurse
		for (const auto& it : *((PhiAssign *)s)) {
			findConstantValues(it.second.def(), dests);
		}
	}
	else if (s->isAssign()) {
		SharedExp rhs = ((Assign *)s)->getRight();

		if (rhs->isIntConst()) {
			dests.push_back(rhs->access<Const>()->getInt());
		}
	}
}


bool BasicBlock::decodeIndirectJmp(UserProc *proc)
{
#define CHECK_REAL_PHI_LOOPS    0
#if CHECK_REAL_PHI_LOOPS
	rtlit rit;
	StatementList::iterator sit;
	Statement               *s = getFirstStmt(rit, sit);

	for (s = getFirstStmt(rit, sit); s; s = getNextStmt(rit, sit)) {
		if (!s->isPhi()) {
			continue;
		}

		Statement      *originalPhi = s;
		InstructionSet workSet, seenSet;
		workSet.insert(s);
		seenSet.insert(s);

		do {
			PhiAssign *pi = (PhiAssign *)*workSet.begin();
			workSet.remove(pi);
			PhiAssign::Definitions::iterator it;

			for (it = pi->begin(); it != pi->end(); it++) {
				if (it->def == nullptr) {
					continue;
				}

				if (!it->def->isPhi()) {
					continue;
				}

				if (seenSet.exists(it->def)) {
					LOG_STREAM() << "Real phi loop involving statements " << originalPhi->getNumber() << " and "
								 << pi->getNumber() << "\n";
					break;
				}
				else {
					workSet.insert(it->def);
					seenSet.insert(it->def);
				}
			}
		} while (workSet.size());
	}
#endif

	if (m_nodeType == BBTYPE::COMPJUMP) {
		assert(m_listOfRTLs->size());
		RTL *lastRtl = m_listOfRTLs->back();

		if (DEBUG_SWITCH) {
			LOG << "decodeIndirectJmp: " << lastRtl->prints();
		}

		assert(not lastRtl->empty());
		CaseStatement *lastStmt = (CaseStatement *)lastRtl->back();
		// Note: some programs might not have the case expression propagated to, because of the -l switch (?)
		// We used to use ordinary propagation here to get the memory expression, but now it refuses to propagate memofs
		// because of the alias safety issue. Eventually, we should use an alias-safe incremental propagation, but for
		// now we'll assume no alias problems and force the propagation
		bool convert; // FIXME: uninitialized value passed to propagateTo
		lastStmt->propagateTo(convert, nullptr, nullptr, true /* force */);
		SharedExp e = lastStmt->getDest();

		char form = 0;

		for (auto& val : hlForms) {
			if (*e *= *val.pattern) { // *= compare ignores subscripts
				form = val.type;

				if (DEBUG_SWITCH) {
					LOG << "indirect jump matches form " << form << "\n";
				}

				break;
			}
		}

		if (form) {
			SWITCH_INFO *swi = new SWITCH_INFO;
			swi->chForm = form;
			ADDRESS   T;
			SharedExp expr;
			findSwParams(form, e, expr, T);

			if (expr) {
				swi->uTable    = T;
				swi->iNumTable = findNumCases();
#if 1           // TMN: Added actual control of the array members, to possibly truncate what findNumCases()
				// thinks is the number of cases, when finding the first array element not pointing to code.
				if (form == 'A') {
					Prog *prog = proc->getProg();

					for (int iPtr = 0; iPtr < swi->iNumTable; ++iPtr) {
						ADDRESS uSwitch = ADDRESS::g(prog->readNative4(swi->uTable + iPtr * 4));

						if ((uSwitch >= prog->getLimitTextHigh()) || (uSwitch < prog->getLimitTextLow())) {
							if (DEBUG_SWITCH) {
								LOG << "Truncating type A indirect jump array to " << iPtr
									<< " entries "
									"due to finding an array entry pointing outside valid code " << uSwitch
									<< " isn't in " << prog->getLimitTextLow() << " .. " << prog->getLimitTextHigh()
									<< "\n";
							}

							// Found an array that isn't a pointer-to-code. Assume array has ended.
							swi->iNumTable = iPtr;
							break;
						}
					}
				}

				if (swi->iNumTable <= 0) {
					LOG << "Switch analysis failure at: " << this->getLowAddr();
					return false;
				}
#endif
				// TODO: missing form = 'R' iOffset is not being set
				swi->iUpper = swi->iNumTable - 1;
				swi->iLower = 0;

				if ((expr->getOper() == opMinus) && expr->getSubExp2()->isIntConst()) {
					swi->iLower  = std::static_pointer_cast<Const>(expr->getSubExp2())->getInt();
					swi->iUpper += swi->iLower;
					expr         = expr->getSubExp1();
				}

				swi->pSwitchVar = expr;
				lastStmt->setDest((SharedExp)nullptr);
				lastStmt->setSwitchInfo(swi);
				return swi->iNumTable != 0;
			}
		}
		else {
			// Did not match a switch pattern. Perhaps it is a Fortran style goto with constants at the leaves of the
			// phi tree. Basically, a location with a reference, e.g. m[r28{-} - 16]{87}
			if (e->isSubscript()) {
				SharedExp sub = e->getSubExp1();

				if (sub->isLocation()) {
					// Yes, we have <location>{ref}. Follow the tree and store the constant values that <location>
					// could be assigned to in dests
					std::list<int> dests;
					findConstantValues(std::static_pointer_cast<RefExp>(e)->getDef(), dests);
					// The switch info wants an array of native addresses
					size_t num_dests = dests.size();

					if (num_dests) {
						int *destArray = new int[num_dests];
						std::copy(dests.begin(), dests.end(), destArray);
						SWITCH_INFO *swi = new SWITCH_INFO;
						swi->chForm     = 'F';                          // The "Fortran" form
						swi->pSwitchVar = e;
						swi->uTable     = ADDRESS::host_ptr(destArray); // WARN: Abuse the uTable member as a pointer
						swi->iNumTable  = num_dests;
						swi->iLower     = 1;                            // Not used, except to compute
						swi->iUpper     = num_dests;                    // the number of options
						lastStmt->setDest((SharedExp)nullptr);
						lastStmt->setSwitchInfo(swi);
						return true;
					}
				}
			}
		}

		return false;
	}
	else if (m_nodeType == BBTYPE::COMPCALL) {
		assert(m_listOfRTLs->size());
		RTL *lastRtl = m_listOfRTLs->back();

		if (DEBUG_SWITCH) {
			LOG << "decodeIndirectJmp: COMPCALL:\n" << lastRtl->prints() << "\n";
		}

		assert(not lastRtl->empty());
		CallStatement *lastStmt = (CallStatement *)lastRtl->back();
		SharedExp     e         = lastStmt->getDest();
		// Indirect calls may sometimes not be propagated to, because of limited propagation (-l switch).
		// Propagate to e, but only keep the changes if the expression matches (don't want excessive propagation to
		// a genuine function pointer expression, even though it's hard to imagine).
		e = e->propagateAll();
		// We also want to replace any m[K]{-} with the actual constant from the (presumably) read-only data section
		ConstGlobalConverter cgc(proc->getProg());
		e = e->accept(&cgc);
		// Simplify the result, e.g. for m[m[(r24{16} + m[0x8048d74]{-}) + 12]{-}]{-} get
		// m[m[(r24{16} + 20) + 12]{-}]{-}, want m[m[r24{16} + 32]{-}]{-}. Note also that making the
		// ConstGlobalConverter a simplifying expression modifier won't work in this case, since the simplifying
		// converter will only simplify the direct parent of the changed expression (which is r24{16} + 20).
		e = e->simplify();

		if (DEBUG_SWITCH) {
			LOG << "decodeIndirect: propagated and const global converted call expression is " << e << "\n";
		}

		int  n          = sizeof(hlVfc) / sizeof(SharedExp);
		bool recognised = false;
		int  i;

		for (i = 0; i < n; i++) {
			if (*e *= *hlVfc[i]) { // *= compare ignores subscripts
				recognised = true;

				if (DEBUG_SWITCH) {
					LOG << "indirect call matches form " << i << "\n";
				}

				break;
			}
		}

		if (!recognised) {
			return false;
		}

		lastStmt->setDest(e); // Keep the changes to the indirect call expression
		int       K1, K2;
		SharedExp vtExp, t1;
		Prog      *prog = proc->getProg();

		switch (i)
		{
		case 0:
			{
				// This is basically an indirection on a global function pointer.  If it is initialised, we have a
				// decodable entry point.  Note: it could also be a library function (e.g. Windows)
				// Pattern 0: global<name>{0}[0]{0}
				K2 = 0;

				if (e->isSubscript()) {
					e = e->getSubExp1();
				}

				e  = e->getSubExp1(); // e is global<name>{0}[0]
				t1 = e->getSubExp2();
				auto t1_const = std::static_pointer_cast<Const>(t1);

				if (e->isArrayIndex() && (t1->isIntConst()) && (t1_const->getInt() == 0)) {
					e = e->getSubExp1(); // e is global<name>{0}
				}

				if (e->isSubscript()) {
					e = e->getSubExp1();                                                        // e is global<name>
				}

				std::shared_ptr<Const> con  = std::static_pointer_cast<Const>(e->getSubExp1()); // e is <name>
				Global                 *glo = prog->getGlobal(con->getStr());
				assert(glo);
				// Set the type to pointer to function, if not already
				SharedType ty = glo->getType();

				if (!ty->isPointer() && !std::static_pointer_cast<PointerType>(ty)->getPointsTo()->isFunc()) {
					glo->setType(PointerType::get(FuncType::get()));
				}

				ADDRESS addr = glo->getAddress();
				// FIXME: not sure how to find K1 from here. I think we need to find the earliest(?) entry in the data
				// map that overlaps with addr
				// For now, let K1 = 0:
				K1    = 0;
				vtExp = Const::get(addr);
				break;
			}

		case 1:
			{
				// Example pattern: e = m[m[r27{25} + 8]{-} + 8]{-}
				if (e->isSubscript()) {
					e = e->getSubExp1();
				}

				e = e->getSubExp1();             // e = m[r27{25} + 8]{-} + 8
				SharedExp rhs = e->getSubExp2(); // rhs = 8
				K2 = std::static_pointer_cast<Const>(rhs)->getInt();
				SharedExp lhs = e->getSubExp1(); // lhs = m[r27{25} + 8]{-}

				if (lhs->isSubscript()) {
					lhs = lhs->getSubExp1(); // lhs = m[r27{25} + 8]
				}

				vtExp = lhs;
				lhs   = lhs->getSubExp1(); // lhs =   r27{25} + 8
				SharedExp CK1 = lhs->getSubExp2();
				K1 = std::static_pointer_cast<Const>(CK1)->getInt();
				break;
			}

		case 2:
			{
				// Example pattern: e = m[m[r27{25}]{-} + 8]{-}
				if (e->isSubscript()) {
					e = e->getSubExp1();
				}

				e = e->getSubExp1();             // e = m[r27{25}]{-} + 8
				SharedExp rhs = e->getSubExp2(); // rhs = 8
				K2 = std::static_pointer_cast<Const>(rhs)->getInt();
				SharedExp lhs = e->getSubExp1(); // lhs = m[r27{25}]{-}

				if (lhs->isSubscript()) {
					lhs = lhs->getSubExp1(); // lhs = m[r27{25}]
				}

				vtExp = lhs;
				K1    = 0;
				break;
			}

		case 3:
			{
				// Example pattern: e = m[m[r27{25} + 8]{-}]{-}
				if (e->isSubscript()) {
					e = e->getSubExp1();
				}

				e  = e->getSubExp1(); // e = m[r27{25} + 8]{-}
				K2 = 0;

				if (e->isSubscript()) {
					e = e->getSubExp1(); // e = m[r27{25} + 8]
				}

				vtExp = e;
				SharedExp lhs = e->getSubExp1(); // lhs =   r27{25} + 8
				// Exp* object = ((Binary*)lhs)->getSubExp1();
				SharedExp CK1 = lhs->getSubExp2();
				K1 = std::static_pointer_cast<Const>(CK1)->getInt();
				break;
			}

		case 4:

			// Example pattern: e = m[m[r27{25}]{-}]{-}
			if (e->isSubscript()) {
				e = e->getSubExp1();
			}

			e  = e->getSubExp1(); // e = m[r27{25}]{-}
			K2 = 0;

			if (e->isSubscript()) {
				e = e->getSubExp1(); // e = m[r27{25}]
			}

			vtExp = e;
			K1    = 0;
			// Exp* object = ((Unary*)e)->getSubExp1();
			break;

		default:
			K1    = K2 = -1; // Suppress warnings
			vtExp = nullptr;
		}

		if (DEBUG_SWITCH) {
			LOG << "form " << i << ": from statement " << lastStmt->getNumber() << " get e = " << lastStmt->getDest()
				<< ", K1 = " << K1 << ", K2 = " << K2 << ", vtExp = " << vtExp << "\n";
		}

		// The vt expression might not be a constant yet, because of expressions not fully propagated, or because of
		// m[K] in the expression (fixed with the ConstGlobalConverter).  If so, look it up in the defCollector in the
		// call
		vtExp = lastStmt->findDefFor(vtExp);

		if (vtExp && DEBUG_SWITCH) {
			LOG << "VT expression boils down to this: " << vtExp << "\n";
		}

		// Danger. For now, only do if -ic given
		bool decodeThru = Boomerang::get()->decodeThruIndCall;

		if (decodeThru && vtExp && vtExp->isIntConst()) {
			ADDRESS addr  = std::static_pointer_cast<Const>(vtExp)->getAddr();
			ADDRESS pfunc = ADDRESS::g(prog->readNative4(addr));

			if (prog->findProc(pfunc) == nullptr) {
				// A new, undecoded procedure
				if (Boomerang::get()->noDecodeChildren) {
					return false;
				}

				prog->decodeEntryPoint(pfunc);
				// Since this was not decoded, this is a significant change, and we want to redecode the current
				// function now that the callee has been decoded
				return true;
			}
		}
	}

	return false;
}


void BasicBlock::processSwitch(UserProc *proc)
{
	RTL           *last(m_listOfRTLs->back());
	CaseStatement *lastStmt((CaseStatement *)last->getHlStmt());
	SWITCH_INFO   *si(lastStmt->getSwitchInfo());

	Boomerang::get()->debugSwitch = true;

	if (Boomerang::get()->debugSwitch) {
		LOG << "processing switch statement type " << si->chForm << " with table at 0x" << si->uTable << ", ";

		if (si->iNumTable) {
			LOG << si->iNumTable << " entries, ";
		}

		LOG << "lo= " << si->iLower << ", hi= " << si->iUpper << "\n";
	}

	ADDRESS uSwitch;
	int     iNumOut, iNum;
	iNumOut = si->iUpper - si->iLower + 1;
	iNum    = iNumOut;
	// Emit an NWAY BB instead of the COMPJUMP. Also update the number of out edges.
	updateType(BBTYPE::NWAY, iNumOut);

	Prog *prog(proc->getProg());
	Cfg  *cfg(proc->getCFG());
	// Where there are repeated switch cases, we have repeated out-edges from the BB. Example:
	// switch (x) {
	//   case 3: case 5:
	//        do something;
	//        break;
	//     case 4: case 10:
	//        do something else
	// ... }
	// The switch statement is emitted assuming one out-edge for each switch value, which is assumed to be iLower+i
	// for the ith zero-based case. It may be that the code for case 5 above will be a goto to the code for case 3,
	// but a smarter back end could group them
	std::list<ADDRESS> dests;

	for (int i = 0; i < iNum; i++) {
		// Get the destination address from the switch table.
		if (si->chForm == 'H') {
			int iValue = prog->readNative4(si->uTable + i * 2);

			if (iValue == -1) {
				continue;
			}

			uSwitch = ADDRESS::g(prog->readNative4(si->uTable + i * 8 + 4));
		}
		else if (si->chForm == 'F') {
			uSwitch = ADDRESS::g(((int *)si->uTable.m_value)[i]);
		}
		else {
			uSwitch = ADDRESS::g(prog->readNative4(si->uTable + i * 4));
		}

		if ((si->chForm == 'O') || (si->chForm == 'R') || (si->chForm == 'r')) {
			// Offset: add table address to make a real pointer to code.  For type R, the table is relative to the
			// branch, so take iOffset. For others, iOffset is 0, so no harm
			if (si->chForm != 'R') {
				assert(si->iOffset == 0);
			}

			uSwitch += si->uTable - si->iOffset;
		}

		if (uSwitch < prog->getLimitTextHigh()) {
			// tq.visit(cfg, uSwitch, this);
			cfg->addOutEdge(this, uSwitch, true);
			// Remember to decode the newly discovered switch code arms, if necessary
			// Don't do it right now, in case there are recursive switch statements (e.g. app7win.exe from
			// hackthissite.org)
			dests.push_back(uSwitch);
		}
		else {
			LOG << "switch table entry branches to past end of text section " << uSwitch << "\n";
#if 1       // TMN: If we reached an array entry pointing outside the program text, we can be quite confident the array
			// has ended. Don't try to pull any more data from it.
			LOG << "Assuming the end of the pointer-array has been reached at index " << i << "\n";
			// TODO: Elevate this logic to the code calculating iNumTable, but still leave this code as a safeguard.
			// Q: Should iNumOut and m_iNumOutEdges really be adjusted (iNum - i) ?
			//            assert(iNumOut        >= (iNum - i));
			assert(int(m_outEdges.size()) >= (iNum - i));
			size_t remove_from_this = m_outEdges.size() - (iNum - i);
			// remove last (iNum - i) out edges
			m_outEdges.erase(m_outEdges.begin() + remove_from_this, m_outEdges.end());
			//            iNumOut        -= (iNum - i);
			m_targetOutEdges -= (iNum - i);
			break;
#else
			iNumOut--;
			m_iNumOutEdges--; // FIXME: where is this set?
#endif
		}
	}

	// Decode the newly discovered switch code arms, if any, and if not already decoded
	int count = 0;

	for (ADDRESS addr : dests) {
		char tmp[1024];
		count++;
		sprintf(tmp, "before decoding fragment %i of %zu (%" PRIxPTR ")", count, dests.size(), addr.m_value);
		Boomerang::get()->alertDecompileDebugPoint(proc, tmp);
		prog->decodeFragment(proc, addr);
	}
}


bool BasicBlock::undoComputedBB(Instruction *stmt)
{
	RTL *last = m_listOfRTLs->back();

	for (auto rr = last->rbegin(); rr != last->rend(); rr++) {
		if (*rr == stmt) {
			m_nodeType = BBTYPE::CALL;
			LOG << "undoComputedBB for statement " << stmt << "\n";
			return true;
		}
	}

	return false;
}


bool BasicBlock::searchAll(const Exp& search_for, std::list<SharedExp>& results)
{
	bool ch = false;

	for (RTL *rtl_it : *m_listOfRTLs) {
		for (Instruction *e : *rtl_it) {
			SharedExp res; // searchAll can be used here too, would it change anything ?

			if (e->search(search_for, res)) {
				ch = true;
				results.push_back(res);
			}
		}
	}

	return ch;
}


bool BasicBlock::searchAndReplace(const Exp& search, SharedExp replace)
{
	bool ch = false;

	for (RTL *rtl_it : *m_listOfRTLs) {
		for (auto& elem : *rtl_it) {
			ch |= (elem)->searchAndReplace(search, replace);
		}
	}

	return ch;
}
