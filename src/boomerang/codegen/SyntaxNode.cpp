#include "SyntaxNode.h"

#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/prog.h"
#include "boomerang/db/exp.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/basicblock.h"


#include <iomanip> // For setfill etc
#include <cstring>
#include <cstdlib>

static int nodecount = 1000;

void PRINT_BEFORE_AFTER(SyntaxNode *root, SyntaxNode *n)
{
	QFile tgt("before.dot");

	if (!tgt.open(QFile::WriteOnly)) {
		return;
	}

	QTextStream of(&tgt);
	of << "digraph before {" << '\n';
	root->printAST(root, of);
	of << "}" << '\n';
	QFile tgt2("after.dot");

	if (!tgt2.open(QFile::WriteOnly)) {
		return;
	}

	QTextStream of1(&tgt2);
	of1 << "digraph after {" << '\n';
	n->printAST(n, of1);
	of1 << "}" << '\n';
	exit(0);
}


SyntaxNode::SyntaxNode()
	: m_pbb(nullptr)
	, m_score(-1)
	, m_correspond(nullptr)
	, m_notGoto(false)
{
	m_nodenum = nodecount++;
}


SyntaxNode::~SyntaxNode()
{
}


int SyntaxNode::getScore()
{
	if (m_score == -1) {
		m_score = evaluate(this);
	}

	return m_score;
}


bool SyntaxNode::isGoto() const
{
	return m_pbb && m_pbb->getType() == BBTYPE::ONEWAY && !m_notGoto;
}


bool SyntaxNode::isBranch() const
{
	return m_pbb && m_pbb->getType() == BBTYPE::TWOWAY;
}


BlockSyntaxNode::BlockSyntaxNode()
{
}


BlockSyntaxNode::~BlockSyntaxNode()
{
	for (auto& elem : statements) {
		delete elem;
	}
}


size_t BlockSyntaxNode::getNumOutEdges() const
{
	if (m_pbb) {
		return m_pbb->getNumOutEdges();
	}

	if (statements.size() == 0) {
		return 0;
	}

	return statements[statements.size() - 1]->getNumOutEdges();
}


SyntaxNode *BlockSyntaxNode::getOutEdge(SyntaxNode *root, size_t n)
{
	if (m_pbb) {
		return root->findNodeFor(m_pbb->getOutEdge(n));
	}

	if (statements.size() == 0) {
		return nullptr;
	}

	return statements[statements.size() - 1]->getOutEdge(root, n);
}


SyntaxNode *BlockSyntaxNode::findNodeFor(BasicBlock *bb)
{
	if (m_pbb == bb) {
		return this;
	}

	SyntaxNode *n = nullptr;

	for (auto& elem : statements) {
		n = elem->findNodeFor(bb);

		if (n != nullptr) {
			break;
		}
	}

	if (n && (n == statements[0])) {
		return this;
	}

	return n;
}


void BlockSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";

	os << "[label=\"";

	if (m_pbb) {
		switch (m_pbb->getType())
		{
		case BBTYPE::ONEWAY:
			os << "Oneway";

			if (m_notGoto) {
				os << " (ignored)";
			}

			break;

		case BBTYPE::TWOWAY:
			os << "Twoway";
			break;

		case BBTYPE::NWAY:
			os << "Nway";
			break;

		case BBTYPE::CALL:
			os << "Call";
			break;

		case BBTYPE::RET:
			os << "Ret";
			break;

		case BBTYPE::FALL:
			os << "Fall";
			break;

		case BBTYPE::COMPJUMP:
			os << "Computed jump";
			break;

		case BBTYPE::COMPCALL:
			os << "Computed call";
			break;

		case BBTYPE::INVALID:
			os << "Invalid";
			break;
		}

		os << " " << m_pbb->getLowAddr();
	}
	else {
		os << "block";
	}

	os << "\"];" << '\n';

	if (m_pbb) {
		for (size_t i = 0; i < m_pbb->getNumOutEdges(); i++) {
			BasicBlock *out = m_pbb->getOutEdge(i);
			os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";

			SyntaxNode *to = root->findNodeFor(out);
			assert(to);
			os << " -> " << to->getNumber() << " [style=dotted";

			if (m_pbb->getNumOutEdges() > 1) {
				os << ",label=" << i;
			}

			os << "];" << '\n';
		}
	}
	else {
		for (auto& elem : statements) {
			elem->printAST(root, os);
		}

		for (unsigned i = 0; i < statements.size(); i++) {
			os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
			os << " -> " << statements[i]->getNumber() << " [label=\"" << i << "\"];" << '\n';
		}
	}
}


#define DEBUG_EVAL    0

int BlockSyntaxNode::evaluate(SyntaxNode *root)
{
#if DEBUG_EVAL
	if (this == root) {
		LOG_STREAM() << "begin eval =============" << '\n';
	}
#endif

	if (m_pbb) {
		return 1;
	}

	int n = 1;

	if (statements.size() == 1) {
		SyntaxNode *out = statements[0]->getOutEdge(root, 0);

		if ((out->getBB() != nullptr) && (out->getBB()->getNumInEdges() > 1)) {
#if DEBUG_EVAL
			LOG_STREAM() << "add 15" << '\n';
#endif
			n += 15;
		}
		else {
#if DEBUG_EVAL
			LOG_STREAM() << "add 30" << '\n';
#endif
			n += 30;
		}
	}

	for (unsigned i = 0; i < statements.size(); i++) {
		n += statements[i]->evaluate(root);

		if (statements[i]->isGoto()) {
			if (i != statements.size() - 1) {
#if DEBUG_EVAL
				LOG_STREAM() << "add 100" << '\n';
#endif
				n += 100;
			}
			else {
#if DEBUG_EVAL
				LOG_STREAM() << "add 50" << '\n';
#endif
				n += 50;
			}
		}
		else if (statements[i]->isBranch()) {
			SyntaxNode *loop = root->getEnclosingLoop(this);
			LOG_STREAM() << "branch " << statements[i]->getNumber() << " not in loop" << '\n';

			if (loop) {
				LOG_STREAM() << "branch " << statements[i]->getNumber() << " in loop " << loop->getNumber() << '\n';
				// this is a bit C specific
				SyntaxNode *out = loop->getOutEdge(root, 0);

				if (out && (statements[i]->getOutEdge(root, 0) == out)) {
					LOG_STREAM() << "found break" << '\n';
					n += 10;
				}

				if (statements[i]->getOutEdge(root, 0) == loop) {
					LOG_STREAM() << "found continue" << '\n';
					n += 10;
				}
			}
			else {
#if DEBUG_EVAL
				LOG_STREAM() << "add 50" << '\n';
#endif
				n += 50;
			}
		}
		else if ((i < statements.size() - 1) && (statements[i]->getOutEdge(root, 0) != statements[i + 1])) {
#if DEBUG_EVAL
			LOG_STREAM() << "add 25" << '\n';
			LOG_STREAM() << statements[i]->getNumber() << " -> " << statements[i]->getOutEdge(root, 0)->getNumber()
						 << " not " << statements[i + 1]->getNumber() << '\n';
#endif
			n += 25;
		}
	}

#if DEBUG_EVAL
	if (this == root) {
		LOG_STREAM() << "end eval = " << n << " =============" << '\n';
	}
#endif
	return n;
}


void BlockSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
	for (unsigned i = 0; i < statements.size(); i++) {
		if (statements[i]->isBlock()) {
			// BlockSyntaxNode *b = (BlockSyntaxNode*)statements[i];
			// can move previous statements into this block
			if (i > 0) {
				LOG_STREAM() << "successor: move previous statement into block" << '\n';
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				BlockSyntaxNode *b1 = (BlockSyntaxNode *)this->clone();
				BlockSyntaxNode *nb = (BlockSyntaxNode *)b1->getStatement(i);
				b1 = (BlockSyntaxNode *)b1->replace(statements[i - 1], nullptr);
				nb->prependStatement(statements[i - 1]->clone());
				n = n->replace(this, b1);
				successors.push_back(n);
				// PRINT_BEFORE_AFTER
			}
		}
		else {
			if (statements.size() != 1) {
				// can replace statement with a block containing that statement
				LOG_STREAM() << "successor: replace statement with a block "
							 << "containing the statement" << '\n';
				BlockSyntaxNode *b = new BlockSyntaxNode();
				b->addStatement(statements[i]->clone());
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(statements[i], b);
				successors.push_back(n);
				// PRINT_BEFORE_AFTER
			}
		}

		// "jump over" style of if-then
		if ((i < statements.size() - 2) && statements[i]->isBranch()) {
			SyntaxNode *b = statements[i];

			if ((b->getOutEdge(root, 0) == statements[i + 2]) &&
				((statements[i + 1]->getOutEdge(root, 0) == statements[i + 2]) || statements[i + 1]->endsWithGoto())) {
				LOG_STREAM() << "successor: jump over style if then" << '\n';
				BlockSyntaxNode *b1 = (BlockSyntaxNode *)this->clone();
				b1 = (BlockSyntaxNode *)b1->replace(statements[i + 1], nullptr);
				IfThenSyntaxNode *nif = new IfThenSyntaxNode();
				SharedExp        cond = b->getBB()->getCond();
				cond = Unary::get(opLNot, cond->clone());
				cond = cond->simplify();
				nif->setCond(cond);
				nif->setThen(statements[i + 1]->clone());
				nif->setBB(b->getBB());
				b1->setStatement(i, nif);
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(this, b1);
				successors.push_back(n);
				// PRINT_BEFORE_AFTER
			}
		}

		// if then else
		if ((i < statements.size() - 2) && statements[i]->isBranch()) {
			SyntaxNode *tThen = statements[i]->getOutEdge(root, 0);
			SyntaxNode *tElse = statements[i]->getOutEdge(root, 1);

			assert(tThen && tElse);

			if ((((tThen == statements[i + 2]) && (tElse == statements[i + 1])) ||
				 ((tThen == statements[i + 1]) && (tElse == statements[i + 2]))) &&
				(tThen->getNumOutEdges() == 1) && (tElse->getNumOutEdges() == 1)) {
				SyntaxNode *else_out = tElse->getOutEdge(root, 0);
				SyntaxNode *then_out = tThen->getOutEdge(root, 0);

				if (else_out == then_out) {
					LOG_STREAM() << "successor: if then else" << '\n';
					SyntaxNode *n = root->clone();
					n->setDepth(root->getDepth() + 1);
					n = n->replace(tThen, nullptr);
					n = n->replace(tElse, nullptr);
					IfThenElseSyntaxNode *nif = new IfThenElseSyntaxNode();
					nif->setCond(statements[i]->getBB()->getCond()->clone());
					nif->setBB(statements[i]->getBB());
					nif->setThen(tThen->clone());
					nif->setElse(tElse->clone());
					n = n->replace(statements[i], nif);
					successors.push_back(n);
					// PRINT_BEFORE_AFTER
				}
			}
		}

		// pretested loop
		if ((i < statements.size() - 2) && statements[i]->isBranch()) {
			SyntaxNode *tBody   = statements[i]->getOutEdge(root, 0);
			SyntaxNode *tFollow = statements[i]->getOutEdge(root, 1);

			assert(tBody && tFollow);

			if ((tBody == statements[i + 1]) && (tFollow == statements[i + 2]) && (tBody->getNumOutEdges() == 1) &&
				(tBody->getOutEdge(root, 0) == statements[i])) {
				LOG_STREAM() << "successor: pretested loop" << '\n';
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(tBody, nullptr);
				PretestedLoopSyntaxNode *nloop = new PretestedLoopSyntaxNode();
				nloop->setCond(statements[i]->getBB()->getCond()->clone());
				nloop->setBB(statements[i]->getBB());
				nloop->setBody(tBody->clone());
				n = n->replace(statements[i], nloop);
				successors.push_back(n);
				// PRINT_BEFORE_AFTER
			}
		}

		// posttested loop
		if ((i > 0) && (i < statements.size() - 1) && statements[i]->isBranch()) {
			SyntaxNode *tBody   = statements[i]->getOutEdge(root, 0);
			SyntaxNode *tFollow = statements[i]->getOutEdge(root, 1);

			assert(tBody && tFollow);

			if ((tBody == statements[i - 1]) && (tFollow == statements[i + 1]) && (tBody->getNumOutEdges() == 1) &&
				(tBody->getOutEdge(root, 0) == statements[i])) {
				LOG_STREAM() << "successor: posttested loop" << '\n';
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(tBody, nullptr);
				PostTestedLoopSyntaxNode *nloop = new PostTestedLoopSyntaxNode();
				nloop->setCond(statements[i]->getBB()->getCond()->clone());
				nloop->setBB(statements[i]->getBB());
				nloop->setBody(tBody->clone());
				n = n->replace(statements[i], nloop);
				successors.push_back(n);
				// PRINT_BEFORE_AFTER
			}
		}

		// infinite loop
		if ((statements[i]->getNumOutEdges() == 1) && (statements[i]->getOutEdge(root, 0) == statements[i])) {
			LOG_STREAM() << "successor: infinite loop" << '\n';
			SyntaxNode *n = root->clone();
			n->setDepth(root->getDepth() + 1);
			InfiniteLoopSyntaxNode *nloop = new InfiniteLoopSyntaxNode();
			nloop->setBody(statements[i]->clone());
			n = n->replace(statements[i], nloop);
			successors.push_back(n);
			PRINT_BEFORE_AFTER(root, n);
		}

		statements[i]->addSuccessors(root, successors);
	}
}


SyntaxNode *BlockSyntaxNode::clone()
{
	BlockSyntaxNode *b = new BlockSyntaxNode();

	b->m_correspond = this;

	if (m_pbb) {
		b->m_pbb = m_pbb;
	}
	else {
		for (auto& elem : statements) {
			b->addStatement(elem->clone());
		}
	}

	return b;
}


SyntaxNode *BlockSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	if (m_correspond == from) {
		return to;
	}

	if (m_pbb == nullptr) {
		std::vector<SyntaxNode *> news;

		for (auto& elem : statements) {
			SyntaxNode *n = elem;

			if (elem->getCorrespond() == from) {
				n = to;
			}
			else {
				n = elem->replace(from, to);
			}

			if (n) {
				news.push_back(n);
			}
		}

		statements.resize(news.size());

		for (unsigned i = 0; i < news.size(); i++) {
			statements[i] = news[i];
		}
	}

	return this;
}


IfThenSyntaxNode::IfThenSyntaxNode()
	: pThen(nullptr)
	, cond(nullptr)
{
}


IfThenSyntaxNode::~IfThenSyntaxNode()
{
	delete pThen;
}


SyntaxNode *IfThenSyntaxNode::getOutEdge(SyntaxNode *root, size_t /*n*/)
{
	SyntaxNode *n1 = root->findNodeFor(m_pbb->getOutEdge(0));

	assert(n1 != pThen);
	return n1;
}


int IfThenSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;

	n += pThen->evaluate(root);
	return n;
}


void IfThenSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
	pThen->addSuccessors(root, successors);
}


SyntaxNode *IfThenSyntaxNode::clone()
{
	IfThenSyntaxNode *b = new IfThenSyntaxNode();

	b->m_correspond = this;
	b->m_pbb        = m_pbb;
	b->cond         = cond->clone();
	b->pThen        = pThen->clone();
	return b;
}


SyntaxNode *IfThenSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(m_correspond != from);

	if (pThen->getCorrespond() == from) {
		assert(to);
		pThen = to;
	}
	else {
		pThen = pThen->replace(from, to);
	}

	return this;
}


SyntaxNode *IfThenSyntaxNode::findNodeFor(BasicBlock *bb)
{
	if (m_pbb == bb) {
		return this;
	}

	return pThen->findNodeFor(bb);
}


void IfThenSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << "[label=\"if " << cond << " \"];" << '\n';
	pThen->printAST(root, os);
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << pThen->getNumber() << " [label=then];" << '\n';
	SyntaxNode *follows = root->findNodeFor(m_pbb->getOutEdge(0));
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << follows->getNumber() << " [style=dotted];" << '\n';
}


IfThenElseSyntaxNode::IfThenElseSyntaxNode()
	: pThen(nullptr)
	, pElse(nullptr)
	, cond(nullptr)
{
}


IfThenElseSyntaxNode::~IfThenElseSyntaxNode()
{
	if (pThen) {
		delete pThen;
	}

	if (pElse) {
		delete pElse;
	}
}


int IfThenElseSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;

	n += pThen->evaluate(root);
	n += pElse->evaluate(root);
	return n;
}


void IfThenElseSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
	// at the moment we can always ignore gotos at the end of
	// then and else, because we assume they have the same
	// follow
	if ((pThen->getNumOutEdges() == 1) && pThen->endsWithGoto()) {
		LOG_STREAM() << "successor: ignoring goto at end of then of if then else" << '\n';
		SyntaxNode *n = root->clone();
		n->setDepth(root->getDepth() + 1);
		SyntaxNode *nThen = pThen->clone();
		nThen->ignoreGoto();
		n = n->replace(pThen, nThen);
		successors.push_back(n);
	}

	if ((pElse->getNumOutEdges() == 1) && pElse->endsWithGoto()) {
		LOG_STREAM() << "successor: ignoring goto at end of else of if then else" << '\n';
		SyntaxNode *n = root->clone();
		n->setDepth(root->getDepth() + 1);
		SyntaxNode *nElse = pElse->clone();
		nElse->ignoreGoto();
		n = n->replace(pElse, nElse);
		successors.push_back(n);
	}

	pThen->addSuccessors(root, successors);
	pElse->addSuccessors(root, successors);
}


SyntaxNode *IfThenElseSyntaxNode::clone()
{
	IfThenElseSyntaxNode *b = new IfThenElseSyntaxNode();

	b->m_correspond = this;
	b->m_pbb        = m_pbb;
	b->cond         = cond->clone();
	b->pThen        = pThen->clone();
	b->pElse        = pElse->clone();
	return b;
}


SyntaxNode *IfThenElseSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(m_correspond != from);

	if (pThen->getCorrespond() == from) {
		assert(to);
		pThen = to;
	}
	else {
		pThen = pThen->replace(from, to);
	}

	if (pElse->getCorrespond() == from) {
		assert(to);
		pElse = to;
	}
	else {
		pElse = pElse->replace(from, to);
	}

	return this;
}


SyntaxNode *IfThenElseSyntaxNode::findNodeFor(BasicBlock *bb)
{
	if (m_pbb == bb) {
		return this;
	}

	SyntaxNode *n = pThen->findNodeFor(bb);

	if (n == nullptr) {
		n = pElse->findNodeFor(bb);
	}

	return n;
}


void IfThenElseSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << "[label=\"if " << cond << " \"];" << '\n';
	pThen->printAST(root, os);
	pElse->printAST(root, os);
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << pThen->getNumber() << " [label=then];" << '\n';
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << pElse->getNumber() << " [label=else];" << '\n';
}


PretestedLoopSyntaxNode::PretestedLoopSyntaxNode()
	: pBody(nullptr)
	, cond(nullptr)
{
}


PretestedLoopSyntaxNode::~PretestedLoopSyntaxNode()
{
	if (pBody) {
		delete pBody;
	}
}


SyntaxNode *PretestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t /*n*/)
{
	return root->findNodeFor(m_pbb->getOutEdge(1));
}


int PretestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;

	n += pBody->evaluate(root);
	return n;
}


void PretestedLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
	// we can always ignore gotos at the end of the body.
	if ((pBody->getNumOutEdges() == 1) && pBody->endsWithGoto()) {
		LOG_STREAM() << "successor: ignoring goto at end of body of pretested "
					 << "loop" << '\n';
		assert(pBody->getOutEdge(root, 0)->startsWith(this));
		SyntaxNode *n = root->clone();
		n->setDepth(root->getDepth() + 1);
		SyntaxNode *nBody = pBody->clone();
		nBody->ignoreGoto();
		n = n->replace(pBody, nBody);
		successors.push_back(n);
	}

	pBody->addSuccessors(root, successors);
}


SyntaxNode *PretestedLoopSyntaxNode::clone()
{
	PretestedLoopSyntaxNode *b = new PretestedLoopSyntaxNode();

	b->m_correspond = this;
	b->m_pbb        = m_pbb;
	b->cond         = cond->clone();
	b->pBody        = pBody->clone();
	return b;
}


SyntaxNode *PretestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(m_correspond != from);

	if (pBody->getCorrespond() == from) {
		assert(to);
		pBody = to;
	}
	else {
		pBody = pBody->replace(from, to);
	}

	return this;
}


SyntaxNode *PretestedLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
	if (m_pbb == bb) {
		return this;
	}

	return pBody->findNodeFor(bb);
}


void PretestedLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << "[label=\"loop pretested ";
	os << cond << " \"];" << '\n';
	pBody->printAST(root, os);
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << pBody->getNumber() << ";" << '\n';
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << getOutEdge(root, 0)->getNumber() << " [style=dotted];" << '\n';
}


PostTestedLoopSyntaxNode::PostTestedLoopSyntaxNode()
	: pBody(nullptr)
	, cond(nullptr)
{
}


PostTestedLoopSyntaxNode::~PostTestedLoopSyntaxNode()
{
	if (pBody) {
		delete pBody;
	}
}


SyntaxNode *PostTestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t /*n*/)
{
	return root->findNodeFor(m_pbb->getOutEdge(1));
}


int PostTestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;

	n += pBody->evaluate(root);
	return n;
}


void PostTestedLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
	// we can always ignore gotos at the end of the body.
	if ((pBody->getNumOutEdges() == 1) && pBody->endsWithGoto()) {
		LOG_STREAM() << "successor: ignoring goto at end of body of posttested "
					 << "loop" << '\n';
		assert(pBody->getOutEdge(root, 0) == this);
		SyntaxNode *n = root->clone();
		n->setDepth(root->getDepth() + 1);
		SyntaxNode *nBody = pBody->clone();
		nBody->ignoreGoto();
		n = n->replace(pBody, nBody);
		successors.push_back(n);
	}

	pBody->addSuccessors(root, successors);
}


SyntaxNode *PostTestedLoopSyntaxNode::clone()
{
	PostTestedLoopSyntaxNode *b = new PostTestedLoopSyntaxNode();

	b->m_correspond = this;
	b->m_pbb        = m_pbb;
	b->cond         = cond->clone();
	b->pBody        = pBody->clone();
	return b;
}


SyntaxNode *PostTestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(m_correspond != from);

	if (pBody->getCorrespond() == from) {
		assert(to);
		pBody = to;
	}
	else {
		pBody = pBody->replace(from, to);
	}

	return this;
}


SyntaxNode *PostTestedLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
	if (m_pbb == bb) {
		return this;
	}

	SyntaxNode *n = pBody->findNodeFor(bb);

	if (n == pBody) {
		return this;
	}

	return n;
}


void PostTestedLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << "[label=\"loop posttested ";
	os << cond << " \"];" << '\n';
	pBody->printAST(root, os);
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << pBody->getNumber() << ";" << '\n';
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << " -> " << getOutEdge(root, 0)->getNumber() << " [style=dotted];" << '\n';
}


InfiniteLoopSyntaxNode::InfiniteLoopSyntaxNode()
	: pBody(nullptr)
{
}


InfiniteLoopSyntaxNode::~InfiniteLoopSyntaxNode()
{
	if (pBody) {
		delete pBody;
	}
}


int InfiniteLoopSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;

	n += pBody->evaluate(root);
	return n;
}


void InfiniteLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
	// we can always ignore gotos at the end of the body.
	if ((pBody->getNumOutEdges() == 1) && pBody->endsWithGoto()) {
		LOG_STREAM() << "successor: ignoring goto at end of body of infinite "
					 << "loop" << '\n';
		assert(pBody->getOutEdge(root, 0) == this);
		SyntaxNode *n = root->clone();
		n->setDepth(root->getDepth() + 1);
		SyntaxNode *nBody = pBody->clone();
		nBody->ignoreGoto();
		n = n->replace(pBody, nBody);
		successors.push_back(n);
	}

	pBody->addSuccessors(root, successors);
}


SyntaxNode *InfiniteLoopSyntaxNode::clone()
{
	InfiniteLoopSyntaxNode *b = new InfiniteLoopSyntaxNode();

	b->m_correspond = this;
	b->m_pbb        = m_pbb;
	b->pBody        = pBody->clone();
	return b;
}


SyntaxNode *InfiniteLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(m_correspond != from);

	if (pBody->getCorrespond() == from) {
		assert(to);
		pBody = to;
	}
	else {
		pBody = pBody->replace(from, to);
	}

	return this;
}


SyntaxNode *InfiniteLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
	if (m_pbb == bb) {
		return this;
	}

	SyntaxNode *n = pBody->findNodeFor(bb);

	if (n == pBody) {
		return this;
	}

	return n;
}


void InfiniteLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
	os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
	os << "[label=\"loop infinite\"];" << '\n';

	if (pBody) {
		pBody->printAST(root, os);
	}

	if (pBody) {
		os << qSetFieldWidth(4) << m_nodenum << qSetFieldWidth(0) << " ";
		os << " -> " << pBody->getNumber() << ";" << '\n';
	}
}
