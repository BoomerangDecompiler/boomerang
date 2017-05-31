#pragma once

/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       hllcode.h
 *   Interface for a high level language code base class.
 * This class is provides methods which are generic of procedural
 * languages like C, Pascal, Fortran, etc.    Included in the base class
 * is the follow and goto sets which are used during code generation.
 * Concrete implementations of this class provide specific language
 * bindings for a single procedure in the program.
 ******************************************************************************/

#include <vector>
#include <cassert>
#include "include/statement.h" // For CallStatement::RetLocs
#include <memory>

class BasicBlock;
class Exp;
using SharedExp = std::shared_ptr<Exp>;
class UserProc;
class Function;
class Type;
class Signature;
class Assign;
class LocationSet;
class CallStatement;
class QTextStream;
// class CallStatement::RetLocs;
class ReturnStatement;

class HLLCode
{
protected:
	UserProc *m_proc; // Pointer to the enclosing UserProc

public:
	// constructors
	HLLCode() {}
	HLLCode(UserProc *p)
		: m_proc(p) {}

	// destructor
	virtual ~HLLCode() {}

	// clear the hllcode object (derived classes should call the base)
	virtual void reset() {}

	// access to proc
	UserProc *getProc() { return m_proc; }
	void setProc(UserProc *p) { m_proc = p; }

	/*
	 * Functions to add new code, pure virtual.
	 */

	// pretested loops
	virtual void AddPretestedLoopHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void AddPretestedLoopEnd(int indLevel) = 0;

	// endless loops
	virtual void AddEndlessLoopHeader(int indLevel) = 0;
	virtual void AddEndlessLoopEnd(int indLevel)    = 0;

	// posttested loops
	virtual void AddPosttestedLoopHeader(int indLevel) = 0;
	virtual void AddPosttestedLoopEnd(int indLevel, const SharedExp& cond) = 0;

	// case conditionals "nways"
	virtual void AddCaseCondHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void AddCaseCondOption(int indLevel, Exp& opt) = 0;
	virtual void AddCaseCondOptionEnd(int indLevel)        = 0;
	virtual void AddCaseCondElse(int indLevel)             = 0;
	virtual void AddCaseCondEnd(int indLevel) = 0;

	// if conditions
	virtual void AddIfCondHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void AddIfCondEnd(int indLevel) = 0;

	// if else conditions
	virtual void AddIfElseCondHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void AddIfElseCondOption(int indLevel) = 0;
	virtual void AddIfElseCondEnd(int indLevel)    = 0;

	// goto, break, continue, etc
	virtual void AddGoto(int indLevel, int ord) = 0;
	virtual void AddBreak(int indLevel)         = 0;
	virtual void AddContinue(int indLevel)      = 0;

	// labels
	virtual void AddLabel(int indLevel, int ord) = 0;
	virtual void RemoveLabel(int ord)            = 0;
	virtual void RemoveUnusedLabels(int maxOrd)  = 0;

	// sequential statements
	virtual void AddAssignmentStatement(int indLevel, Assign *s) = 0;
	virtual void AddCallStatement(int indLevel, Function *proc, const QString& name, StatementList& args,
								  StatementList *results) = 0;
	virtual void AddIndCallStatement(int indLevel, const SharedExp& exp, StatementList& args, StatementList *results) = 0;
	virtual void AddReturnStatement(int indLevel, StatementList *rets) = 0;

	// procedure related
	virtual void AddProcStart(UserProc *proc) = 0;
	virtual void AddProcEnd() = 0;
	virtual void AddLocal(const QString& name, SharedType type, bool last = false) = 0;
	virtual void AddGlobal(const QString& name, SharedType type, const SharedExp& init = nullptr) = 0;
	virtual void AddPrototype(UserProc *proc) = 0;

	// comments
	virtual void AddLineComment(const QString& cmt) = 0;

	/*
	 * output functions, pure virtual.
	 */
	virtual void print(QTextStream& os) = 0;
}; // class HLLCode

class SyntaxNode
{
protected:
	BasicBlock *pbb;
	int nodenum;
	int score;
	SyntaxNode *correspond; // corresponding node in previous state
	bool notGoto;
	int depth;

public:
	SyntaxNode();
	virtual ~SyntaxNode();

	virtual bool isBlock() { return false; }
	virtual bool isGoto();
	virtual bool isBranch();

	virtual void ignoreGoto() {}

	virtual int getNumber() { return nodenum; }

	BasicBlock *getBB() { return pbb; }
	void setBB(BasicBlock *bb) { pbb = bb; }

	virtual size_t getNumOutEdges() = 0;
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) = 0;
	virtual bool endsWithGoto() = 0;

	virtual bool startsWith(SyntaxNode *node) { return this == node; }

	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr) = 0;

	int getScore();

	void addToScore(int n) { score = getScore() + n; }
	void setDepth(int n) { depth = n; }
	int getDepth() { return depth; }

	virtual SyntaxNode *clone() = 0;
	virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) = 0;

	SyntaxNode *getCorrespond() { return correspond; }

	virtual SyntaxNode *findNodeFor(BasicBlock *bb)          = 0;
	virtual void printAST(SyntaxNode *root, QTextStream& os) = 0;
	virtual int evaluate(SyntaxNode *root) = 0;

	virtual void addSuccessors(SyntaxNode * /*root*/, std::vector<SyntaxNode *>& /*successors*/) {}
}; // class SyntaxNode

class BlockSyntaxNode : public SyntaxNode
{
private:
	std::vector<SyntaxNode *> statements;

public:
	BlockSyntaxNode();
	virtual ~BlockSyntaxNode();

	virtual bool isBlock()  override { return pbb == nullptr; }

	virtual void ignoreGoto()  override
	{
		if (pbb) {
			notGoto = true;
		}
		else if (statements.size() > 0) {
			statements[statements.size() - 1]->ignoreGoto();
		}
	}

	size_t getNumStatements() { return pbb ? 0 : statements.size(); }
	SyntaxNode *getStatement(size_t n)
	{
		assert(pbb == nullptr);
		return statements[n];
	}

	void prependStatement(SyntaxNode *n)
	{
		assert(pbb == nullptr);
		statements.resize(statements.size() + 1);

		for (size_t i = statements.size() - 1; i > 0; i--) {
			statements[i] = statements[i - 1];
		}

		statements[0] = n;
	}

	void addStatement(SyntaxNode *n)
	{
		assert(pbb == nullptr);
		statements.push_back(n);
	}

	void setStatement(size_t i, SyntaxNode *n)
	{
		assert(pbb == nullptr);
		statements[i] = n;
	}

	virtual size_t getNumOutEdges() override;
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

	virtual bool endsWithGoto()  override
	{
		if (pbb) {
			return isGoto();
		}

		bool last = false;

		if (statements.size() > 0) {
			last = statements[statements.size() - 1]->endsWithGoto();
		}

		return last;
	}

	virtual bool startsWith(SyntaxNode *node)  override
	{
		return this == node || (statements.size() > 0 && statements[0]->startsWith(node));
	}

	virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = nullptr)  override
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

	virtual bool isGoto()  override { return false; }
	virtual bool isBranch()  override { return false; }

	virtual size_t getNumOutEdges() override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
	virtual bool endsWithGoto()  override { return false; }

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
}; // class IfThenSyntaxNode

class IfThenElseSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pThen;
	SyntaxNode *pElse;
	SharedExp cond;

public:
	IfThenElseSyntaxNode();
	virtual ~IfThenElseSyntaxNode();
	virtual bool isGoto()  override { return false; }
	virtual bool isBranch()  override { return false; }

	virtual size_t getNumOutEdges() override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t /*n*/)  override
	{
		SyntaxNode *o = pThen->getOutEdge(root, 0);

		assert(o == pElse->getOutEdge(root, 0));
		return o;
	}

	virtual bool endsWithGoto()  override { return false; }

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
}; // class IfThenElseSyntaxNode

class PretestedLoopSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pBody;
	SharedExp cond;

public:
	PretestedLoopSyntaxNode();
	virtual ~PretestedLoopSyntaxNode();
	virtual bool isGoto()  override { return false; }
	virtual bool isBranch()  override { return false; }

	virtual size_t getNumOutEdges() override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

	virtual bool endsWithGoto()  override { return false; }
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
}; // class PretestedLoopSyntaxNode

class PostTestedLoopSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pBody;
	SharedExp cond;

public:
	PostTestedLoopSyntaxNode();
	virtual ~PostTestedLoopSyntaxNode();
	virtual bool isGoto()  override { return false; }
	virtual bool isBranch()  override { return false; }

	virtual size_t getNumOutEdges() override { return 1; }
	virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
	virtual bool endsWithGoto()  override { return false; }
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
}; // class PostTestedLoopSyntaxNode

class InfiniteLoopSyntaxNode : public SyntaxNode
{
protected:
	SyntaxNode *pBody;

public:
	InfiniteLoopSyntaxNode();
	virtual ~InfiniteLoopSyntaxNode();
	virtual bool isGoto()  override { return false; }
	virtual bool isBranch()  override { return false; }

	virtual size_t getNumOutEdges() override { return 0; }
	virtual SyntaxNode *getOutEdge(SyntaxNode * /*root*/, size_t /*n*/)  override { return nullptr; }
	virtual bool endsWithGoto()  override { return false; }
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
}; // class InfiniteLoopSyntaxNode
