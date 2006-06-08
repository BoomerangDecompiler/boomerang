
#include <fstream>
#include <iomanip>			// For setfill etc
#include "prog.h"
#include "exp.h"
#include "hllcode.h"
#include "cfg.h"
#include "statement.h"

static int nodecount = 1000;


#define PRINT_BEFORE_AFTER std::ofstream of("before.dot"); \
				of << "digraph before {" << std::endl; \
				root->printAST(root, of); \
				of << "}" << std::endl; \
				of.close(); \
				std::ofstream of1("after.dot"); \
				of1 << "digraph after {" << std::endl; \
				n->printAST(n, of1); \
				of1 << "}" << std::endl; \
				of1.close(); \
				exit(0);

SyntaxNode::SyntaxNode() : pbb(NULL), score(-1), correspond(NULL), 
						   notGoto(false)
{
	nodenum = nodecount++;
}

SyntaxNode::~SyntaxNode()
{
}

int SyntaxNode::getScore()
{
	if (score == -1)
		score = evaluate(this);
	return score;
}

bool SyntaxNode::isGoto()
{
	return pbb && pbb->getType() == ONEWAY && !notGoto;
}

bool SyntaxNode::isBranch()
{
	return pbb && pbb->getType() == TWOWAY;
}

BlockSyntaxNode::BlockSyntaxNode()
{
}

BlockSyntaxNode::~BlockSyntaxNode()
{
	for (unsigned i = 0; i < statements.size(); i++)
		delete statements[i];
}

int BlockSyntaxNode::getNumOutEdges() {
	if (pbb)
		return pbb->getNumOutEdges();
	if (statements.size() == 0)
		return 0;
	return statements[statements.size()-1]->getNumOutEdges();
}

SyntaxNode *BlockSyntaxNode::getOutEdge(SyntaxNode *root, int n) {
	if (pbb)
		return root->findNodeFor(pbb->getOutEdge(n));
	if (statements.size() == 0)
		return NULL;
	return statements[statements.size()-1]->getOutEdge(root, n);
}

SyntaxNode *BlockSyntaxNode::findNodeFor(PBB bb)
{
	if (pbb == bb)
		return this;
	SyntaxNode *n = NULL;
	for (unsigned i = 0; i < statements.size(); i++) {
		n = statements[i]->findNodeFor(bb);
		if (n != NULL)
			break;
	}
	if (n && n == statements[0])
		return this;
	return n;
}

void BlockSyntaxNode::printAST(SyntaxNode *root, std::ostream &os)
{
	os << std::setw(4) << std::dec << nodenum << " ";
	os << "[label=\"";
	if (pbb) {
		switch(pbb->getType()) {
			case ONEWAY:	os << "Oneway"; 
							if (notGoto) os << " (ignored)"; break;
			case TWOWAY:	os << "Twoway"; break;
			case NWAY:		os << "Nway"; break;
			case CALL:		os << "Call"; break;
			case RET:		os << "Ret"; break;
			case FALL:		os << "Fall"; break;
			case COMPJUMP:	os << "Computed jump"; break;
			case COMPCALL:	os << "Computed call"; break;
			case INVALID:	os << "Invalid"; break;
		}
		os << " " << std::hex << pbb->getLowAddr();
	} else
		os << "block";
	os << "\"];" << std::endl;
	if (pbb) {
		for (int i = 0; i < pbb->getNumOutEdges(); i++) {
			PBB out = pbb->getOutEdge(i);
			os << std::setw(4) << std::dec << nodenum << " ";
			SyntaxNode *to = root->findNodeFor(out);
			assert(to);
			os << " -> " << to->getNumber() << " [style=dotted";
			if (pbb->getNumOutEdges() > 1)
				os << ",label=" << i;
			os << "];" << std::endl;
		}
	} else	{
		for (unsigned i = 0; i < statements.size(); i++)
			statements[i]->printAST(root, os);
		for (unsigned i = 0; i < statements.size(); i++) {
			os << std::setw(4) << std::dec << nodenum << " ";
			os << " -> " << statements[i]->getNumber() 
						 << " [label=\"" << i << "\"];" << std::endl;
		}
	}
}

#define DEBUG_EVAL 0

int BlockSyntaxNode::evaluate(SyntaxNode *root)
{
#if DEBUG_EVAL
	if (this == root) 
		std::cerr << "begin eval =============" << std::endl;
#endif
	if (pbb)
		return 1;
	int n = 1;
	if (statements.size() == 1) {
		SyntaxNode *out = statements[0]->getOutEdge(root, 0);
		if (out->getBB() != NULL && out->getBB()->getNumInEdges() > 1) {
#if DEBUG_EVAL
			std::cerr << "add 15" << std::endl;
#endif
			n += 15;
		} else {
#if DEBUG_EVAL
			std::cerr << "add 30" << std::endl;
#endif
			n += 30;
		}
	}
	for (unsigned i = 0; i < statements.size(); i++) {
		n += statements[i]->evaluate(root); 
		if (statements[i]->isGoto()) {
			if (i != statements.size()-1) {
#if DEBUG_EVAL
				std::cerr << "add 100" << std::endl;
#endif
				n += 100;
			} else {
#if DEBUG_EVAL
				std::cerr << "add 50" << std::endl;
#endif
				n += 50;
			}
		} else if (statements[i]->isBranch()) {
			SyntaxNode *loop = root->getEnclosingLoop(this);
			std::cerr << "branch " << statements[i]->getNumber() 
					  << " not in loop" << std::endl;
			if (loop) {
				std::cerr << "branch " << statements[i]->getNumber() 
						  << " in loop " << loop->getNumber() << std::endl;
				// this is a bit C specific
				SyntaxNode *out = loop->getOutEdge(root, 0);
				if (out && statements[i]->getOutEdge(root, 0) == out) {
					std::cerr << "found break" << std::endl;
					n += 10;
				}
				if (statements[i]->getOutEdge(root, 0) == loop) {
					std::cerr << "found continue" << std::endl;
					n += 10;
				}
			} else {
#if DEBUG_EVAL
				std::cerr << "add 50" << std::endl;
#endif
				n += 50;
			}
		} else if (i < statements.size()-1 && 
				 statements[i]->getOutEdge(root, 0) != statements[i+1]) {
#if DEBUG_EVAL
			std::cerr << "add 25" << std::endl;
			std::cerr << statements[i]->getNumber() << " -> "
					  << statements[i]->getOutEdge(root, 0)->getNumber() 
					  << " not " << statements[i+1]->getNumber() << std::endl;
#endif
			n += 25;
		}
	}
#if DEBUG_EVAL
	if (this == root) 
		std::cerr << "end eval = " << n << " =============" << std::endl;
#endif
	return n;
}

void BlockSyntaxNode::addSuccessors(SyntaxNode *root,
									std::vector<SyntaxNode*> &successors)
{
	for (unsigned i = 0; i < statements.size(); i++) {
		if (statements[i]->isBlock()) {
			//BlockSyntaxNode *b = (BlockSyntaxNode*)statements[i];
			// can move previous statements into this block
			if (i > 0) {
				std::cerr << "successor: move previous statement into block" 
						  << std::endl;
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				BlockSyntaxNode *b1 = (BlockSyntaxNode*)this->clone();
				BlockSyntaxNode *nb = (BlockSyntaxNode*)b1->getStatement(i);
				b1 = (BlockSyntaxNode*)b1->replace(statements[i-1], NULL);
				nb->prependStatement(statements[i-1]->clone());
				n = n->replace(this, b1);
				successors.push_back(n);
				//PRINT_BEFORE_AFTER
			}
		} else {
			if (statements.size() != 1) {
				// can replace statement with a block containing that statement
				std::cerr << "successor: replace statement with a block "
						  << "containing the statement" << std::endl;
				BlockSyntaxNode *b = new BlockSyntaxNode();
				b->addStatement(statements[i]->clone());
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(statements[i], b);
				successors.push_back(n);
				//PRINT_BEFORE_AFTER
			}
		}
		// "jump over" style of if-then
		if (i < statements.size() - 2 && statements[i]->isBranch()) {
			SyntaxNode *b = statements[i];
			if (b->getOutEdge(root, 0) == statements[i+2] &&
				(statements[i+1]->getOutEdge(root, 0) == statements[i+2] ||
				 statements[i+1]->endsWithGoto())) {
					std::cerr << "successor: jump over style if then" 
							  << std::endl;
					BlockSyntaxNode *b1 = 
						(BlockSyntaxNode*)this->clone();
					b1 = (BlockSyntaxNode*)b1->replace(statements[i+1], NULL);
					IfThenSyntaxNode *nif = new IfThenSyntaxNode();
					Exp *cond = b->getBB()->getCond();
					cond = new Unary(opLNot, cond->clone());
					cond = cond->simplify();
					nif->setCond(cond);
					nif->setThen(statements[i+1]->clone());
					nif->setBB(b->getBB());
					b1->setStatement(i, nif);
					SyntaxNode *n = root->clone();
					n->setDepth(root->getDepth() + 1);
					n = n->replace(this, b1);
					successors.push_back(n);
					//PRINT_BEFORE_AFTER
			}
		}
		// if then else
		if (i < statements.size() - 2 && statements[i]->isBranch()) {
			SyntaxNode *tThen = statements[i]->getOutEdge(root, 0);
			SyntaxNode *tElse = statements[i]->getOutEdge(root, 1);

			assert(tThen && tElse);
			if (((tThen == statements[i+2] && tElse == statements[i+1]) ||
				 (tThen == statements[i+1] && tElse == statements[i+2])) &&
				tThen->getNumOutEdges() == 1 && tElse->getNumOutEdges() == 1) {
				SyntaxNode *else_out = tElse->getOutEdge(root, 0);
				SyntaxNode *then_out = tThen->getOutEdge(root, 0);

				if (else_out == then_out) {
					std::cerr << "successor: if then else" << std::endl;
					SyntaxNode *n = root->clone();
					n->setDepth(root->getDepth() + 1);
					n = n->replace(tThen, NULL);
					n = n->replace(tElse, NULL);
					IfThenElseSyntaxNode *nif = new IfThenElseSyntaxNode();
					nif->setCond(statements[i]->getBB()->getCond()->clone());
					nif->setBB(statements[i]->getBB());
					nif->setThen(tThen->clone());
					nif->setElse(tElse->clone());
					n = n->replace(statements[i], nif);
					successors.push_back(n);
					//PRINT_BEFORE_AFTER
				}
			}
		}

		// pretested loop
		if (i < statements.size() - 2 && statements[i]->isBranch()) {
			SyntaxNode *tBody = statements[i]->getOutEdge(root, 0);
			SyntaxNode *tFollow =  statements[i]->getOutEdge(root, 1);

			assert(tBody && tFollow);
			if (tBody == statements[i+1] && tFollow == statements[i+2] &&
				tBody->getNumOutEdges() == 1 &&
				tBody->getOutEdge(root, 0) == statements[i]) {
				std::cerr << "successor: pretested loop" << std::endl;
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(tBody, NULL);
				PretestedLoopSyntaxNode *nloop = new PretestedLoopSyntaxNode();
				nloop->setCond(statements[i]->getBB()->getCond()->clone());
				nloop->setBB(statements[i]->getBB());
				nloop->setBody(tBody->clone());
				n = n->replace(statements[i], nloop);
				successors.push_back(n);
				//PRINT_BEFORE_AFTER
			}
		}

		// posttested loop
		if (i > 0 && i < statements.size()-1 && statements[i]->isBranch()) {
			SyntaxNode *tBody = statements[i]->getOutEdge(root, 0);
			SyntaxNode *tFollow =  statements[i]->getOutEdge(root, 1);

			assert(tBody && tFollow);
			if (tBody == statements[i-1] && tFollow == statements[i+1] &&
				tBody->getNumOutEdges() == 1 &&
				tBody->getOutEdge(root, 0) == statements[i]) {
				std::cerr << "successor: posttested loop" << std::endl;
				SyntaxNode *n = root->clone();
				n->setDepth(root->getDepth() + 1);
				n = n->replace(tBody, NULL);
				PostTestedLoopSyntaxNode *nloop = 
												new PostTestedLoopSyntaxNode();
				nloop->setCond(statements[i]->getBB()->getCond()->clone());
				nloop->setBB(statements[i]->getBB());
				nloop->setBody(tBody->clone());
				n = n->replace(statements[i], nloop);
				successors.push_back(n);
				//PRINT_BEFORE_AFTER
			}
		}

		// infinite loop
		if (statements[i]->getNumOutEdges() == 1 &&
			statements[i]->getOutEdge(root, 0) == statements[i]) {
			std::cerr << "successor: infinite loop" << std::endl;
			SyntaxNode *n = root->clone();
			n->setDepth(root->getDepth() + 1);
			InfiniteLoopSyntaxNode *nloop = new InfiniteLoopSyntaxNode();
			nloop->setBody(statements[i]->clone());
			n = n->replace(statements[i], nloop);
			successors.push_back(n);
			PRINT_BEFORE_AFTER
		}

		statements[i]->addSuccessors(root, successors);
	}
}

SyntaxNode *BlockSyntaxNode::clone()
{
	BlockSyntaxNode *b = new BlockSyntaxNode();
	b->correspond = this;
	if (pbb)
		b->pbb = pbb;
	else
		for (unsigned i = 0; i < statements.size(); i++)
			b->addStatement(statements[i]->clone());
	return b;
}

SyntaxNode *BlockSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	if (correspond == from)
		return to;

	if (pbb == NULL) {
		std::vector<SyntaxNode*> news;
		for (unsigned i = 0; i < statements.size(); i++) {
			SyntaxNode *n = statements[i];
			if (statements[i]->getCorrespond() == from)
				n = to;
			else
				n = statements[i]->replace(from, to);
			if (n)
				news.push_back(n);
		}
		statements.resize(news.size());
		for (unsigned i = 0; i < news.size(); i++)
			statements[i] = news[i];
	}
	return this;
}

IfThenSyntaxNode::IfThenSyntaxNode() : pThen(NULL), cond(NULL)
{
}


IfThenSyntaxNode::~IfThenSyntaxNode()
{
	if (pThen)
		delete pThen;
}

SyntaxNode *IfThenSyntaxNode::getOutEdge(SyntaxNode *root, int n) {
	SyntaxNode *n1 = root->findNodeFor(pbb->getOutEdge(0));
	assert(n1 != pThen);
	return n1;
}

int IfThenSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;
	n += pThen->evaluate(root);
	return n;
}

void IfThenSyntaxNode::addSuccessors(SyntaxNode *root,
									 std::vector<SyntaxNode*> &successors)
{
	pThen->addSuccessors(root, successors);
}

SyntaxNode *IfThenSyntaxNode::clone()
{
	IfThenSyntaxNode *b = new IfThenSyntaxNode();
	b->correspond = this;
	b->pbb = pbb;
	b->cond = cond->clone();
	b->pThen = pThen->clone();
	return b;
}

SyntaxNode *IfThenSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(correspond != from);
	if (pThen->getCorrespond() == from) {
		assert(to);
		pThen = to;
	} else 
		pThen = pThen->replace(from, to);
	return this;
}

SyntaxNode *IfThenSyntaxNode::findNodeFor(PBB bb)
{
	if (pbb == bb)
		return this;
	return pThen->findNodeFor(bb);
}

void IfThenSyntaxNode::printAST(SyntaxNode *root, std::ostream &os)
{
	os << std::setw(4) << std::dec << nodenum << " ";
	os << "[label=\"if " << cond << " \"];" << std::endl;
	pThen->printAST(root, os);
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << pThen->getNumber() << " [label=then];" << std::endl;
	SyntaxNode *follows = root->findNodeFor(pbb->getOutEdge(0));
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << follows->getNumber() << " [style=dotted];" << std::endl;
}

IfThenElseSyntaxNode::IfThenElseSyntaxNode() : pThen(NULL), pElse(NULL), 
	cond(NULL)
{
}

IfThenElseSyntaxNode::~IfThenElseSyntaxNode()
{
	if (pThen)
		delete pThen;
	if (pElse)
		delete pElse;
}

int IfThenElseSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;
	n += pThen->evaluate(root);
	n += pElse->evaluate(root);
	return n;
}

void IfThenElseSyntaxNode::addSuccessors(SyntaxNode *root,
										 std::vector<SyntaxNode*> &successors)
{
	// at the moment we can always ignore gotos at the end of 
	// then and else, because we assume they have the same
	// follow
	if (pThen->getNumOutEdges() == 1 && pThen->endsWithGoto()) {
		std::cerr << "successor: ignoring goto at end of then of if then else" 
				  << std::endl;
		SyntaxNode *n = root->clone();
		n->setDepth(root->getDepth() + 1);
		SyntaxNode *nThen = pThen->clone();
		nThen->ignoreGoto();
		n = n->replace(pThen, nThen);
		successors.push_back(n);
	}

	if (pElse->getNumOutEdges() == 1 && pElse->endsWithGoto()) {
		std::cerr << "successor: ignoring goto at end of else of if then else" 
				  << std::endl;
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
	b->correspond = this;
	b->pbb = pbb;
	b->cond = cond->clone();
	b->pThen = pThen->clone();
	b->pElse = pElse->clone();
	return b;
}

SyntaxNode *IfThenElseSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(correspond != from);
	if (pThen->getCorrespond() == from) {
		assert(to);
		pThen = to;
	} else 
		pThen = pThen->replace(from, to);
	if (pElse->getCorrespond() == from) {
		assert(to);
		pElse = to;
	} else 
		pElse = pElse->replace(from, to);
	return this;
}

SyntaxNode *IfThenElseSyntaxNode::findNodeFor(PBB bb)
{
	if (pbb == bb)
		return this;
	SyntaxNode *n = pThen->findNodeFor(bb);
	if (n == NULL)
		n = pElse->findNodeFor(bb);
	return n;
}

void IfThenElseSyntaxNode::printAST(SyntaxNode *root, std::ostream &os)
{
	os << std::setw(4) << std::dec << nodenum << " ";
	os << "[label=\"if " << cond << " \"];" << std::endl;
	pThen->printAST(root, os);
	pElse->printAST(root, os);
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << pThen->getNumber() << " [label=then];" << std::endl;
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << pElse->getNumber() << " [label=else];" << std::endl;
}


PretestedLoopSyntaxNode::PretestedLoopSyntaxNode() : pBody(NULL), cond(NULL)
{
}

PretestedLoopSyntaxNode::~PretestedLoopSyntaxNode()
{
	if (pBody)
		delete pBody;
	if (cond)
		delete cond;
}

SyntaxNode *PretestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, int n) {
	return root->findNodeFor(pbb->getOutEdge(1));
}

int PretestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;
	n += pBody->evaluate(root);
	return n;
}

void PretestedLoopSyntaxNode::addSuccessors(SyntaxNode *root,
										std::vector<SyntaxNode*> &successors)
{
	// we can always ignore gotos at the end of the body.
	if (pBody->getNumOutEdges() == 1 && pBody->endsWithGoto()) {
		std::cerr << "successor: ignoring goto at end of body of pretested "
				  << "loop" << std::endl;
		SyntaxNode *out = pBody->getOutEdge(root, 0);
		assert(out->startsWith(this));
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
	b->correspond = this;
	b->pbb = pbb;
	b->cond = cond->clone();
	b->pBody = pBody->clone();
	return b;
}

SyntaxNode *PretestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(correspond != from);
	if (pBody->getCorrespond() == from) {
		assert(to);
		pBody = to;
	} else 
		pBody = pBody->replace(from, to);
	return this;
}

SyntaxNode *PretestedLoopSyntaxNode::findNodeFor(PBB bb)
{
	if (pbb == bb)
		return this;
	return pBody->findNodeFor(bb);
}

void PretestedLoopSyntaxNode::printAST(SyntaxNode *root, std::ostream &os)
{
	os << std::setw(4) << std::dec << nodenum << " ";
	os << "[label=\"loop pretested ";
	os	<< cond << " \"];" << std::endl;
	pBody->printAST(root, os);
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << pBody->getNumber() << ";" << std::endl;
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << getOutEdge(root, 0)->getNumber() 
				 << " [style=dotted];" << std::endl;
}

PostTestedLoopSyntaxNode::PostTestedLoopSyntaxNode() : pBody(NULL), cond(NULL)
{
}

PostTestedLoopSyntaxNode::~PostTestedLoopSyntaxNode()
{
	if (pBody)
		delete pBody;
	if (cond)
		delete cond;
}

SyntaxNode *PostTestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, int n) {
	return root->findNodeFor(pbb->getOutEdge(1));
}

int PostTestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;
	n += pBody->evaluate(root);
	return n;
}

void PostTestedLoopSyntaxNode::addSuccessors(SyntaxNode *root,
										std::vector<SyntaxNode*> &successors)
{
	// we can always ignore gotos at the end of the body.
	if (pBody->getNumOutEdges() == 1 && pBody->endsWithGoto()) {
		std::cerr << "successor: ignoring goto at end of body of posttested "
				  << "loop" << std::endl;
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
	b->correspond = this;
	b->pbb = pbb;
	b->cond = cond->clone();
	b->pBody = pBody->clone();
	return b;
}

SyntaxNode *PostTestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(correspond != from);
	if (pBody->getCorrespond() == from) {
		assert(to);
		pBody = to;
	} else 
		pBody = pBody->replace(from, to);
	return this;
}

SyntaxNode *PostTestedLoopSyntaxNode::findNodeFor(PBB bb)
{
	if (pbb == bb)
		return this;
	SyntaxNode *n = pBody->findNodeFor(bb);
	if (n == pBody)
		return this;
	return n;
}

void PostTestedLoopSyntaxNode::printAST(SyntaxNode *root, std::ostream &os)
{
	os << std::setw(4) << std::dec << nodenum << " ";
	os << "[label=\"loop posttested ";
	os	<< cond << " \"];" << std::endl;
	pBody->printAST(root, os);
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << pBody->getNumber() << ";" << std::endl;
	os << std::setw(4) << std::dec << nodenum << " ";
	os << " -> " << getOutEdge(root, 0)->getNumber() 
				 << " [style=dotted];" << std::endl;
}

InfiniteLoopSyntaxNode::InfiniteLoopSyntaxNode() : pBody(NULL)
{
}

InfiniteLoopSyntaxNode::~InfiniteLoopSyntaxNode()
{
	if (pBody)
		delete pBody;
}

int InfiniteLoopSyntaxNode::evaluate(SyntaxNode *root)
{
	int n = 1;
	n += pBody->evaluate(root);
	return n;
}

void InfiniteLoopSyntaxNode::addSuccessors(SyntaxNode *root,
										std::vector<SyntaxNode*> &successors)
{
	// we can always ignore gotos at the end of the body.
	if (pBody->getNumOutEdges() == 1 && pBody->endsWithGoto()) {
		std::cerr << "successor: ignoring goto at end of body of infinite "
				  << "loop" << std::endl;
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
	b->correspond = this;
	b->pbb = pbb;
	b->pBody = pBody->clone();
	return b;
}

SyntaxNode *InfiniteLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
	assert(correspond != from);
	if (pBody->getCorrespond() == from) {
		assert(to);
		pBody = to;
	} else 
		pBody = pBody->replace(from, to);
	return this;
}

SyntaxNode *InfiniteLoopSyntaxNode::findNodeFor(PBB bb)
{
	if (pbb == bb)
		return this;
	SyntaxNode *n = pBody->findNodeFor(bb);
	if (n == pBody)
		return this;
	return n;
}

void InfiniteLoopSyntaxNode::printAST(SyntaxNode *root, std::ostream &os)
{
	os << std::setw(4) << std::dec << nodenum << " ";
	os << "[label=\"loop infinite\"];" << std::endl;
	if (pBody)
		pBody->printAST(root, os);
	if (pBody) {
		os << std::setw(4) << std::dec << nodenum << " ";
		os << " -> " << pBody->getNumber() << ";" << std::endl;
	}
}

