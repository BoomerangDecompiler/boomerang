/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       hllcode.h
 * OVERVIEW:   Interface for a high level language code base class.
 *             This class is provides methods which are generic of procedural
 *             languages like C, Pascal, Fortran, etc.  Included in the base
 *             class is the follow and goto sets which are used during code 
 *             generation.
 *             Concrete implementations of this class provide specific language
 *             bindings for a single procedure in the program.
 *============================================================================*/

#ifndef _HLLCODE_H_
#define _HLLCODE_H_

class BasicBlock;
class Exp;
class UserProc;
class Proc;

class HLLCode {
protected:
	UserProc *m_proc;
	std::list<BasicBlock*> m_followSet;
	std::set<BasicBlock*> m_gotoSet;

public:
	// constructors
	HLLCode() { }
	HLLCode(UserProc *p) : m_proc(p), showAllLabels(false) { }

	// destructor
	virtual ~HLLCode() { }

	// clear the hllcode object (derived classes should call the base)
	virtual void reset() {
		m_followSet.clear();
		m_gotoSet.clear();
	}

	// should we show all labels?
	bool showAllLabels;

	// access to proc
	UserProc *getProc() { return m_proc; }
	void setProc(UserProc *p) { m_proc = p; }

	// access to followSet
	void addtoFollowSet(BasicBlock *bb) { m_followSet.push_back(bb); }
	bool followSetContains(BasicBlock *bb) { 
		std::list<BasicBlock*>::iterator it;
		for (it = m_followSet.begin(); it != m_followSet.end(); it++)
			if (*it == bb) return true;
		return false; 
	}
	BasicBlock *getEnclFollow() { return m_followSet.back(); }
	void removeLastFollow() { m_followSet.pop_back(); }

	// access to gotoSet
	void addtoGotoSet(BasicBlock *bb) { m_gotoSet.insert(bb); }
	bool gotoSetContains(BasicBlock *bb) { return m_gotoSet.find(bb) != m_gotoSet.end(); }

	/*
	 * Functions to add new code, pure virtual.
	 */

	// pretested loops (cond is optional because it is in the bb [somewhere])
	virtual void AddPretestedLoopHeader(BasicBlock *bb, Exp *cond = NULL) = 0;
	virtual void AddPretestedLoopEnd(BasicBlock *bb) = 0;

	// endless loops
	virtual void AddEndlessLoopHeader(BasicBlock *bb) = 0;
	virtual void AddEndlessLoopEnd(BasicBlock *bb) = 0;

	// posttested loops
	virtual void AddPosttestedLoopHeader(BasicBlock *bb) = 0;
	virtual void AddPosttestedLoopEnd(BasicBlock *bb, Exp *cond = NULL) = 0;

	// case conditionals "nways"
	virtual void AddCaseCondHeader(BasicBlock *bb, Exp *cond = NULL) = 0;
	virtual void AddCaseCondOption(BasicBlock *bb, Exp *opt = NULL) = 0;
	virtual void AddCaseCondOptionEnd(BasicBlock *bb) = 0;
	virtual void AddCaseCondElse(BasicBlock *bb) = 0;
	virtual void AddCaseCondEnd(BasicBlock *bb) = 0;

	// if conditions
	virtual void AddIfCondHeader(BasicBlock *bb, Exp *cond = NULL) = 0;
	virtual void AddIfCondEnd(BasicBlock *bb) = 0;

	// if else conditions
	virtual void AddIfElseCondHeader(BasicBlock *bb, Exp *cond = NULL) = 0;
	virtual void AddIfElseCondOption(BasicBlock *bb) = 0;
	virtual void AddIfElseCondEnd(BasicBlock *bb) = 0;

	// else conditions
	virtual void AddElseCondHeader(BasicBlock *bb, Exp *cond = NULL) = 0;
	virtual void AddElseCondEnd(BasicBlock *bb) = 0;

	// goto, break, continue, etc
	virtual void AddGoto(BasicBlock *bb, BasicBlock *dest = NULL) = 0;

	// labels
	virtual void AddLabel(BasicBlock *bb) = 0;

	// sequential statements
	virtual void AddAssignmentStatement(BasicBlock *bb, Exp *exp) = 0;
	virtual void AddCallStatement(BasicBlock *bb, Exp *retloc, Proc *proc, std::vector<Exp*> &args) = 0;
	virtual void AddCallStatement(BasicBlock *bb, Exp *retloc, Exp *dest, std::vector<Exp*> &args) = 0;
	virtual void AddReturnStatement(BasicBlock *bb, Exp *ret) = 0;

	/*
	 * output functions, pure virtual.
	 */
	virtual void toString(std::string &s) = 0;
	virtual bool isLabelAt(int nCharIndex) = 0;
	virtual BasicBlock *getBlockAt(int nCharIndex) = 0;
	virtual Exp *getExpAt(int nCharIndex) = 0;
	virtual Proc *getCallAt(int nCharIndex) = 0;
	virtual void addFormating(int nCharIndex, const char *str) = 0;
};

#endif

