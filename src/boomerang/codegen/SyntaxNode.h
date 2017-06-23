#pragma once

#include <vector>
#include <cassert>
#include <memory>

#include "boomerang/util/types.h"
#include "boomerang/type/type.h"
#include "boomerang/include/managed.h"

class BasicBlock;


class SyntaxNode
{
protected:
	BasicBlock *m_pbb;
	int m_nodenum;
	int m_score;
	SyntaxNode *m_correspond; // corresponding node in previous state
	bool m_notGoto;
	int m_depth;

public:
	SyntaxNode();
	virtual ~SyntaxNode();

	virtual bool isBlock() const { return false; }
	virtual bool isGoto() const;
	virtual bool isBranch() const;

	virtual void ignoreGoto() {}

	virtual int getNumber() const { return m_nodenum; }

	BasicBlock *getBB() const { return m_pbb; }
	void setBB(BasicBlock *bb) { m_pbb = bb; }

	virtual size_t getNumOutEdges() const = 0;
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) = 0;
	virtual bool endsWithGoto() const = 0;

	virtual bool startsWith(SyntaxNode *node) const { return this == node; }

	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr) = 0;

	int getScore();

	void addToScore(int n) { m_score = getScore() + n; }
	void setDepth(int n) { m_depth = n; }
	int getDepth() const { return m_depth; }

	virtual SyntaxNode *clone() = 0;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) = 0;

	SyntaxNode *getCorrespond() const { return m_correspond; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb)          = 0;
	virtual void printAST(SyntaxNode *root, QTextStream& os) = 0;
	virtual int evaluate(SyntaxNode *root) = 0;

	virtual void addSuccessors(SyntaxNode * /*root*/, std::vector<SyntaxNode *>& /*successors*/) {}
};


class BlockSyntaxNode : public SyntaxNode
{
private:
	std::vector<SyntaxNode *> statements;

public:
	BlockSyntaxNode();
	virtual ~BlockSyntaxNode();

	virtual bool isBlock() const override { return m_pbb == nullptr; }

	virtual void ignoreGoto() override
	{
		if (m_pbb) {
			m_notGoto = true;
		}
		else if (statements.size() > 0) {
			statements[statements.size() - 1]->ignoreGoto();
		}
	}

	size_t getNumStatements() const { return m_pbb ? 0 : statements.size(); }

	SyntaxNode *getStatement(size_t n)
	{
		assert(m_pbb == nullptr);
		return statements[n];
	}

	void prependStatement(SyntaxNode *n)
	{
		assert(m_pbb == nullptr);
		statements.resize(statements.size() + 1);

		for (size_t i = statements.size() - 1; i > 0; i--) {
			statements[i] = statements[i - 1];
		}

		statements[0] = n;
	}

	void addStatement(SyntaxNode *n)
	{
		assert(m_pbb == nullptr);
		statements.push_back(n);
	}

	void setStatement(size_t i, SyntaxNode *n)
	{
		assert(m_pbb == nullptr);
		statements[i] = n;
	}

	virtual size_t getNumOutEdges() const override;
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

	virtual bool endsWithGoto() const override
	{
		if (m_pbb) {
			return isGoto();
		}

		bool last = false;

		if (statements.size() > 0) {
			last = statements[statements.size() - 1]->endsWithGoto();
		}

		return last;
	}

	virtual bool startsWith(SyntaxNode *node) const override
	{
		return this == node || (statements.size() > 0 && statements[0]->startsWith(node));
	}

	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr) override
	{
		if (this == pFor) {
			return cur;
		}

		for (unsigned i = 0; i < statements.size(); i++) {
			SyntaxNode *n = statements[i]->getEnclosingLoop(pFor, cur);

			if (n) {
				return n;
			}
		}

		return nullptr;
	}

	virtual SyntaxNode *clone() override;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

	virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
	virtual void printAST(SyntaxNode *root, QTextStream& os) override;
	virtual int evaluate(SyntaxNode *root) override;
	virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;
}; // class BlockSyntaxNode


class IfThenSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pThen;
	SharedExp cond;

public:
	IfThenSyntaxNode();
	virtual ~IfThenSyntaxNode();

	virtual bool isGoto() const override { return false; }
	virtual bool isBranch() const override { return false; }

	virtual size_t getNumOutEdges() const override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
	virtual bool endsWithGoto() const override { return false; }

	virtual SyntaxNode *clone() override;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr)  override
	{
		if (this == pFor) {
			return cur;
		}

		return pThen->getEnclosingLoop(pFor, cur);
	}

	void setCond(SharedExp e) { cond = e; }
	SharedExp getCond() { return cond; }
	void setThen(SyntaxNode *n) { pThen = n; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
	virtual void printAST(SyntaxNode *root, QTextStream& os) override;
	virtual int evaluate(SyntaxNode *root) override;
	virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;
};


class IfThenElseSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pThen;
	SyntaxNode *pElse;
	SharedExp cond;

public:
	IfThenElseSyntaxNode();
	virtual ~IfThenElseSyntaxNode();

	virtual bool isGoto() const override { return false; }
	virtual bool isBranch() const override { return false; }

	virtual size_t getNumOutEdges() const override { return 1; }

	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t /*n*/)  override
	{
		SyntaxNode *o = pThen->getOutEdge(root, 0);

		assert(o == pElse->getOutEdge(root, 0));
		return o;
	}

	virtual bool endsWithGoto() const override { return false; }

	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr)  override
	{
		if (this == pFor) {
			return cur;
		}

		SyntaxNode *n = pThen->getEnclosingLoop(pFor, cur);

		if (n) {
			return n;
		}

		return pElse->getEnclosingLoop(pFor, cur);
	}

	virtual SyntaxNode *clone() override;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

	void setCond(SharedExp e) { cond = e; }
	void setThen(SyntaxNode *n) { pThen = n; }
	void setElse(SyntaxNode *n) { pElse = n; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
	virtual void printAST(SyntaxNode *root, QTextStream& os) override;
	virtual int evaluate(SyntaxNode *root) override;
	virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;
};


class PretestedLoopSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pBody;
	SharedExp cond;

public:
	PretestedLoopSyntaxNode();
	virtual ~PretestedLoopSyntaxNode();
	virtual bool isGoto() const override { return false; }
	virtual bool isBranch() const override { return false; }

	virtual size_t getNumOutEdges() const override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

	virtual bool endsWithGoto() const override { return false; }
	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr) override
	{
		if (this == pFor) {
			return cur;
		}

		return pBody->getEnclosingLoop(pFor, this);
	}

	virtual SyntaxNode *clone() override;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

	void setCond(SharedExp e) { cond = e; }
	void setBody(SyntaxNode *n) { pBody = n; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
	virtual void printAST(SyntaxNode *root, QTextStream& os) override;
	virtual int evaluate(SyntaxNode *root) override;
	virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;
};


class PostTestedLoopSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pBody;
	SharedExp cond;

public:
	PostTestedLoopSyntaxNode();
	virtual ~PostTestedLoopSyntaxNode();
	virtual bool isGoto() const override { return false; }
	virtual bool isBranch() const override { return false; }

	virtual size_t getNumOutEdges() const override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
	virtual bool endsWithGoto() const override { return false; }
	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr)  override
	{
		if (this == pFor) {
			return cur;
		}

		return pBody->getEnclosingLoop(pFor, this);
	}

	virtual SyntaxNode *clone() override;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

	void setCond(SharedExp e) { cond = e; }
	void setBody(SyntaxNode *n) { pBody = n; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
	virtual void printAST(SyntaxNode *root, QTextStream& os) override;
	virtual int evaluate(SyntaxNode *root) override;
	virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;
};


class InfiniteLoopSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pBody;

public:
	InfiniteLoopSyntaxNode();
	virtual ~InfiniteLoopSyntaxNode();
	virtual bool isGoto() const override { return false; }
	virtual bool isBranch() const override { return false; }

	virtual size_t getNumOutEdges() const override { return 0; }
	virtual SyntaxNode *getOutEdge(SyntaxNode * /*root*/, size_t /*n*/)  override { return nullptr; }
	virtual bool endsWithGoto() const override { return false; }
	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr)  override
	{
		if (this == pFor) {
			return cur;
		}

		return pBody->getEnclosingLoop(pFor, this);
	}

	virtual SyntaxNode *clone() override;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

	void setBody(SyntaxNode *n) { pBody = n; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
	virtual void printAST(SyntaxNode *root, QTextStream& os) override;
	virtual int evaluate(SyntaxNode *root) override;
	virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;
};
