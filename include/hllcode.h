/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   hllcode.h
 * OVERVIEW:   Interface for a high level language code base class.
 *			   This class is provides methods which are generic of procedural
 *			   languages like C, Pascal, Fortran, etc.	Included in the base class
 *			   is the follow and goto sets which are used during code generation.
 *			   Concrete implementations of this class provide specific language
 *			   bindings for a single procedure in the program.
 *============================================================================*/

#ifndef _HLLCODE_H_
#define _HLLCODE_H_

#include <iostream>
#include <vector>
#include <assert.h>

class BasicBlock;
typedef BasicBlock *PBB;
class Exp;
class UserProc;
class Proc;
class Type;
class Signature;
class Assign;
class LocationSet;
struct ReturnInfo;

class HLLCode {
protected:
		UserProc *m_proc;

public:
		// constructors
				HLLCode() { }
				HLLCode(UserProc *p) : m_proc(p) { }

	// destructor
virtual 		~HLLCode() { }

	// clear the hllcode object (derived classes should call the base)
virtual void	reset() {
		}

		// access to proc
		UserProc *getProc() { return m_proc; }
		void	setProc(UserProc *p) { m_proc = p; }

		/*
		 * Functions to add new code, pure virtual.
	 	 */

		// pretested loops
virtual void	AddPretestedLoopHeader(int indLevel, Exp *cond) = 0;
virtual void	AddPretestedLoopEnd(int indLevel) = 0;

		// endless loops
virtual void	AddEndlessLoopHeader(int indLevel) = 0;
virtual void	AddEndlessLoopEnd(int indLevel) = 0;

		// posttested loops
virtual void	AddPosttestedLoopHeader(int indLevel) = 0;
virtual void	AddPosttestedLoopEnd(int indLevel, Exp *cond) = 0;

		// case conditionals "nways"
virtual void	AddCaseCondHeader(int indLevel, Exp *cond) = 0;
virtual void	AddCaseCondOption(int indLevel, Exp *opt) = 0;
virtual void	AddCaseCondOptionEnd(int indLevel) = 0;
virtual void	AddCaseCondElse(int indLevel) = 0;
virtual void	AddCaseCondEnd(int indLevel) = 0;

		// if conditions
virtual void	AddIfCondHeader(int indLevel, Exp *cond) = 0;
virtual void	AddIfCondEnd(int indLevel) = 0;

		// if else conditions
virtual void	AddIfElseCondHeader(int indLevel, Exp *cond) = 0;
virtual void	AddIfElseCondOption(int indLevel) = 0;
virtual void	AddIfElseCondEnd(int indLevel) = 0;

		// goto, break, continue, etc
virtual void	AddGoto(int indLevel, int ord) = 0;
virtual void	AddBreak(int indLevel) = 0;
virtual void	AddContinue(int indLevel) = 0;

		// labels
virtual void	AddLabel(int indLevel, int ord) = 0;
virtual void	RemoveLabel(int ord) = 0;
virtual void	RemoveUnusedLabels(int maxOrd) = 0;

		// sequential statements
virtual void	AddAssignmentStatement(int indLevel, Assign *s) = 0;
virtual void	AddCallStatement(int indLevel, Proc *proc, const char *name, std::vector<Exp*> &args,
					std::vector<ReturnInfo>& rets) = 0;
virtual void	AddIndCallStatement(int indLevel, Exp *exp, std::vector<Exp*> &args) = 0;
virtual void	AddReturnStatement(int indLevel, std::vector<Exp*> &returns) = 0;

		// procedure related
virtual void	AddProcStart(Signature *signature) = 0;
virtual void	AddProcEnd() = 0;
virtual void	AddLocal(const char *name, Type *type, bool last = false) = 0;
virtual void	AddGlobal(const char *name, Type *type, Exp *init = NULL) = 0;
virtual void	AddPrototype(Signature* signature) = 0;

	// comments
virtual void AddLineComment(char* cmt) = 0;
	
		/*
		 * output functions, pure virtual.
		 */
virtual void	print(std::ostream &os) = 0;
};		// class HLLCode

class SyntaxNode {
protected:
		PBB		pbb;
		int		nodenum;
		int		score;
		SyntaxNode *correspond; // corresponding node in previous state
		bool	notGoto;
		int		depth;

public:
				SyntaxNode();
virtual 		~SyntaxNode();

virtual bool	isBlock() { return false; }
virtual bool	isGoto();
virtual bool	isBranch();

virtual void	ignoreGoto() { };

virtual int		getNumber() { return nodenum; }

		PBB		getBB() { return pbb; }
		void	setBB(PBB bb) { pbb = bb; }

virtual int		getNumOutEdges() = 0;
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n) = 0;
virtual bool	endsWithGoto() = 0;
virtual bool	startsWith(SyntaxNode *node) { return this == node; }

virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, 
										 SyntaxNode *cur = NULL) = 0;

		int		getScore();
		void	addToScore(int n) { score = getScore() + n; }
		void	setDepth(int n) { depth = n; }
		int		getDepth() { return depth; }

virtual SyntaxNode *clone() = 0;
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) = 0;
		SyntaxNode *getCorrespond() { return correspond; }

virtual SyntaxNode *findNodeFor(PBB bb) = 0;
virtual void	printAST(SyntaxNode *root, std::ostream &os) = 0;
virtual int		evaluate(SyntaxNode *root) = 0;
virtual void	addSuccessors(SyntaxNode *root, std::vector<SyntaxNode*> &successors) { }
};		// class SyntaxNode

class BlockSyntaxNode : public SyntaxNode {
private:
	std::vector<SyntaxNode*> statements;

public:
	BlockSyntaxNode();
	virtual ~BlockSyntaxNode();

	virtual bool isBlock() { return pbb == NULL; }

	virtual void ignoreGoto() {
		if (pbb) notGoto = true;
		else if (statements.size() > 0)
			statements[statements.size()-1]->ignoreGoto();
	}

	int getNumStatements() { 
		return pbb ? 0 : statements.size(); 
	}
	SyntaxNode *getStatement(int n) {
		assert(pbb == NULL);
		return statements[n]; 
	}
	void prependStatement(SyntaxNode *n) {
		assert(pbb == NULL);
		statements.resize(statements.size()+1);
		for (int i = statements.size()-1; i > 0;  i--)
			statements[i] = statements[i-1];
		statements[0] = n;
	}
	void addStatement(SyntaxNode *n) { 
		assert(pbb == NULL);
		statements.push_back(n); 
	}
	void setStatement(int i, SyntaxNode *n) { 
		assert(pbb == NULL);
		statements[i] = n; 
	}

virtual int getNumOutEdges();
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n);
virtual bool endsWithGoto() {
			if (pbb) return isGoto();
			bool last = false;
			if (statements.size() > 0)
				last = statements[statements.size()-1]->endsWithGoto();
			return last;
		}
virtual bool	startsWith(SyntaxNode *node) { 
			return this == node || (statements.size() > 0 && statements[0]->startsWith(node)); 
		}
virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = NULL) {
			if (this == pFor) return cur;
			for (unsigned i = 0; i < statements.size(); i++) {
				SyntaxNode *n = statements[i]->getEnclosingLoop(pFor, cur);
				if (n) return n;
			}
			return NULL;
		}

virtual SyntaxNode *clone();
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to);

virtual SyntaxNode *findNodeFor(PBB bb);
virtual void	printAST(SyntaxNode *root, std::ostream &os);
virtual int		evaluate(SyntaxNode *root);
virtual void	addSuccessors(SyntaxNode *root, std::vector<SyntaxNode*> &successors);
};		// class BlockSyntaxNode

class IfThenSyntaxNode : public SyntaxNode {
protected:
		SyntaxNode *pThen;
		Exp		*cond;

public:
				IfThenSyntaxNode();
virtual 		~IfThenSyntaxNode();

virtual bool	isGoto() { return false; }
virtual bool	isBranch() { return false; }

virtual int		getNumOutEdges() {
		return 1;
	}
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n);
virtual bool	endsWithGoto() { return false; }

virtual SyntaxNode *clone();
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to);

virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = NULL) {
			if (this == pFor) return cur;
			return pThen->getEnclosingLoop(pFor, cur);
		}

		void	setCond(Exp *e) { cond = e; }
		Exp		*getCond() { return cond; }
		void	setThen(SyntaxNode *n) { pThen = n; }

virtual SyntaxNode *findNodeFor(PBB bb);
virtual void	printAST(SyntaxNode *root, std::ostream &os);
virtual int		evaluate(SyntaxNode *root);
virtual void	addSuccessors(SyntaxNode *root, std::vector<SyntaxNode*> &successors);
};		// class IfThenSyntaxNode

class IfThenElseSyntaxNode : public SyntaxNode {
protected:
		SyntaxNode *pThen;
		SyntaxNode *pElse;
		Exp		*cond;

public:
				IfThenElseSyntaxNode();
virtual 		~IfThenElseSyntaxNode();
virtual bool	isGoto() { return false; }
virtual bool	isBranch() { return false; }

virtual int		getNumOutEdges() {
			return 1;
		}
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n) {
			SyntaxNode *o = pThen->getOutEdge(root, 0);
			assert(o == pElse->getOutEdge(root, 0));
			return o;
		}
virtual bool	endsWithGoto() { return false; }

virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = NULL) {
			if (this == pFor) return cur;
			SyntaxNode *n = pThen->getEnclosingLoop(pFor, cur);
			if (n) return n;
			return pElse->getEnclosingLoop(pFor, cur);
		}


virtual SyntaxNode *clone();
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to);

		void	setCond(Exp *e) { cond = e; }
		void	setThen(SyntaxNode *n) { pThen = n; }
		void	setElse(SyntaxNode *n) { pElse = n; }

virtual SyntaxNode *findNodeFor(PBB bb);
virtual void printAST(SyntaxNode *root, std::ostream &os);
virtual int evaluate(SyntaxNode *root);
virtual void addSuccessors(SyntaxNode *root,
							   std::vector<SyntaxNode*> &successors);
};		// class IfThenElseSyntaxNode

class PretestedLoopSyntaxNode : public SyntaxNode {
protected:
		SyntaxNode *pBody;
		Exp		*cond;

public:
				PretestedLoopSyntaxNode();
virtual 		~PretestedLoopSyntaxNode();
virtual bool	isGoto() { return false; }
virtual bool	isBranch() { return false; }

virtual int		getNumOutEdges() {
		return 1;
	}
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n);
virtual bool	endsWithGoto() { return false; }
virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = NULL) {
			if (this == pFor) return cur;
			return pBody->getEnclosingLoop(pFor, this);
		}

virtual SyntaxNode *clone();
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to);

		void	setCond(Exp *e) { cond = e; }
		void	setBody(SyntaxNode *n) { pBody = n; }

virtual SyntaxNode *findNodeFor(PBB bb);
virtual void	printAST(SyntaxNode *root, std::ostream &os);
virtual int		evaluate(SyntaxNode *root);
virtual void	addSuccessors(SyntaxNode *root, std::vector<SyntaxNode*> &successors);
};		// class PretestedLoopSyntaxNode

class PostTestedLoopSyntaxNode : public SyntaxNode {
protected:
		SyntaxNode *pBody;
		Exp		*cond;

public:
					PostTestedLoopSyntaxNode();
virtual ~PostTestedLoopSyntaxNode();
virtual bool	isGoto() { return false; }
virtual bool	isBranch() { return false; }

virtual int		getNumOutEdges() {
			return 1;
		}
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n);
virtual bool	endsWithGoto() { return false; }
virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = NULL) {
			if (this == pFor) return cur;
			return pBody->getEnclosingLoop(pFor, this);
		}

virtual SyntaxNode *clone();
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to);

		void	setCond(Exp *e) { cond = e; }
		void	setBody(SyntaxNode *n) { pBody = n; }

virtual SyntaxNode *findNodeFor(PBB bb);
virtual void	printAST(SyntaxNode *root, std::ostream &os);
virtual int		evaluate(SyntaxNode *root);
virtual void 	addSuccessors(SyntaxNode *root, std::vector<SyntaxNode*> &successors);
};		// class PostTestedLoopSyntaxNode

class InfiniteLoopSyntaxNode : public SyntaxNode {
protected:
		SyntaxNode *pBody;

public:
				InfiniteLoopSyntaxNode();
virtual 		~InfiniteLoopSyntaxNode();
virtual bool	isGoto() { return false; }
virtual bool	isBranch() { return false; }

virtual int getNumOutEdges() {
			return 0;
		}
virtual SyntaxNode *getOutEdge(SyntaxNode *root, int n) {
			return NULL;
		}
virtual bool endsWithGoto() { return false; }
virtual SyntaxNode *getEnclosingLoop(SyntaxNode *pFor, SyntaxNode *cur = NULL) {
			if (this == pFor) return cur;
			return pBody->getEnclosingLoop(pFor, this);
		}

virtual SyntaxNode *clone();
virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to);

		void	setBody(SyntaxNode *n) { pBody = n; }

virtual SyntaxNode *findNodeFor(PBB bb);
virtual void	printAST(SyntaxNode *root, std::ostream &os);
virtual int		evaluate(SyntaxNode *root);
virtual void 	addSuccessors(SyntaxNode *root, std::vector<SyntaxNode*> &successors);
};		// class InfiniteLoopSyntaxNode

#endif

