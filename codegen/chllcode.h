/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */
/*
 * 22 Nov 02 - Mike: Re-ordered initialisations to keep gcc quiet
 */

/*==============================================================================
 * FILE:       chllcode.h
 * OVERVIEW:   Concrete class for the "C" high level language
 *             This class is provides methods which are specific for the C
 *             language binding.  I guess this will be the most popular output
 *             language unless we do C++.
 *============================================================================*/

#ifndef _CHLLCODE_H_
#define _CHLLCODE_H_

#include <string>

class BasicBlock;
class Exp;
class Proc;

class CHLLCode : public HLLCode {
private:
	std::list<char *> lines;

        void indent(char *str, int indLevel);
        void appendExp(char *str, Exp *exp);
        void appendType(char *str, Type *typ);

public:
	// constructor
	CHLLCode();
	CHLLCode(UserProc *p);

	// destructor
	virtual ~CHLLCode();

	// clear this class, calls the base
	virtual void reset();

	/*
	 * Functions to add new code
	 */

	// pretested loops (cond is optional because it is in the bb 
        // [somewhere])
	virtual void AddPretestedLoopHeader(int indLevel, Exp *cond);
	virtual void AddPretestedLoopEnd(int indLevel);

	// endless loops
	virtual void AddEndlessLoopHeader(int indLevel);
	virtual void AddEndlessLoopEnd(int indLevel);

	// posttested loops
	virtual void AddPosttestedLoopHeader(int indLevel);
	virtual void AddPosttestedLoopEnd(int indLevel, Exp *cond);

	// case conditionals "nways"
	virtual void AddCaseCondHeader(int indLevel, Exp *cond);
	virtual void AddCaseCondOption(int indLevel, Exp *opt);
	virtual void AddCaseCondOptionEnd(int indLevel);
	virtual void AddCaseCondElse(int indLevel);
	virtual void AddCaseCondEnd(int indLevel);

	// if conditions
	virtual void AddIfCondHeader(int indLevel, Exp *cond);
	virtual void AddIfCondEnd(int indLevel);

	// if else conditions
	virtual void AddIfElseCondHeader(int indLevel, Exp *cond);
	virtual void AddIfElseCondOption(int indLevel);
	virtual void AddIfElseCondEnd(int indLevel);

	// goto, break, continue, etc
	virtual void AddGoto(int indLevel, int ord);
	virtual void AddContinue(int indLevel);
	virtual void AddBreak(int indLevel);

	// labels
	virtual void AddLabel(int indLevel, int ord);
	virtual void RemoveLabel(int ord);

	// sequential statements
	virtual void AddAssignmentStatement(int indLevel, AssignExp *exp);
	virtual void AddCallStatement(int indLevel, Exp *retloc, Proc *proc, 
            std::vector<Exp*> &args);
	virtual void AddIndCallStatement(int indLevel, Exp *retloc, Exp *exp,
            std::vector<Exp*> &args);
	virtual void AddReturnStatement(int indLevel, Exp *ret);
	virtual void AddProcStart(Signature *signature);
	virtual void AddProcEnd();
	virtual void AddLocal(const char *name, Type *type);

    // comments
    virtual void AddLineComment(char* cmt);

	/*
	 * output functions
	 */
	virtual void print(std::ostream &os);
};

#endif
