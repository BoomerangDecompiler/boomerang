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

#include <iostream>

class BasicBlock;
class Exp;
class UserProc;
class Proc;
class Type;
class Signature;

class HLLCode {
protected:
	UserProc *m_proc;

public:
	// constructors
	HLLCode() { }
	HLLCode(UserProc *p) : m_proc(p) { }

	// destructor
	virtual ~HLLCode() { }

	// clear the hllcode object (derived classes should call the base)
	virtual void reset() {
	}

	// access to proc
	UserProc *getProc() { return m_proc; }
	void setProc(UserProc *p) { m_proc = p; }

	/*
	 * Functions to add new code, pure virtual.
	 */

	// pretested loops
	virtual void AddPretestedLoopHeader(int indLevel, Exp *cond) = 0;
	virtual void AddPretestedLoopEnd(int indLevel) = 0;

	// endless loops
	virtual void AddEndlessLoopHeader(int indLevel) = 0;
	virtual void AddEndlessLoopEnd(int indLevel) = 0;

	// posttested loops
	virtual void AddPosttestedLoopHeader(int indLevel) = 0;
	virtual void AddPosttestedLoopEnd(int indLevel, Exp *cond) = 0;

	// case conditionals "nways"
	virtual void AddCaseCondHeader(int indLevel, Exp *cond) = 0;
	virtual void AddCaseCondOption(int indLevel, Exp *opt) = 0;
	virtual void AddCaseCondOptionEnd(int indLevel) = 0;
	virtual void AddCaseCondElse(int indLevel) = 0;
	virtual void AddCaseCondEnd(int indLevel) = 0;

	// if conditions
	virtual void AddIfCondHeader(int indLevel, Exp *cond) = 0;
	virtual void AddIfCondEnd(int indLevel) = 0;

	// if else conditions
	virtual void AddIfElseCondHeader(int indLevel, Exp *cond) = 0;
	virtual void AddIfElseCondOption(int indLevel) = 0;
	virtual void AddIfElseCondEnd(int indLevel) = 0;

	// goto, break, continue, etc
	virtual void AddGoto(int indLevel, int ord) = 0;
        virtual void AddBreak(int indLevel) = 0;
        virtual void AddContinue(int indLevel) = 0;

	// labels
	virtual void AddLabel(int indLevel, int ord) = 0;
        virtual void RemoveLabel(int ord) = 0;

	// sequential statements
	virtual void AddAssignmentStatement(int indLevel, AssignExp *exp) = 0;
	virtual void AddCallStatement(int indLevel, Exp *retloc, Proc *proc, 
            std::vector<Exp*> &args) = 0;
	virtual void AddReturnStatement(int indLevel, Exp *ret) = 0;
	virtual void AddProcStart(Signature *signature) = 0;
	virtual void AddProcEnd() = 0;
	virtual void AddLocal(const char *name, Type *type) = 0;

	/*
	 * output functions, pure virtual.
	 */
	virtual void print(std::ostream &os) = 0;
};

#endif

