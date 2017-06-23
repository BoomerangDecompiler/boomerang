#pragma once

/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

#include <vector>
#include <cassert>
#include <memory>

#include "include/types.h"
#include "type/type.h"
#include "include/managed.h"


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
class QString;
class ReturnStatement;


/**
 * Base class for generating high-level code from statements.
 *
 * This class is provides methods which are generic of procedural
 * languages like C, Pascal, Fortran, etc. Included in the base class
 * is the follow and goto sets which are used during code generation.
 * Concrete implementations of this class provide specific language
 * bindings for a single procedure in the program.
 */
class ICodeGenerator
{
public:
	ICodeGenerator(UserProc *p)
		: m_proc(p) {}

	// destructor
	virtual ~ICodeGenerator() {}

	/// clear the code generator object (derived classes should call the base)
	virtual void reset() {}

	// access to proc
	UserProc *getProc() const { return m_proc; }

	/*
	 * Functions to add new code, pure virtual.
	 */

	// pretested loops
	virtual void addPretestedLoopHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void addPretestedLoopEnd(int indLevel) = 0;

	// endless loops
	virtual void addEndlessLoopHeader(int indLevel) = 0;
	virtual void addEndlessLoopEnd(int indLevel)    = 0;

	// post-tested loops
	virtual void addPostTestedLoopHeader(int indLevel) = 0;
	virtual void addPostTestedLoopEnd(int indLevel, const SharedExp& cond) = 0;

	// case conditionals "nways"
	virtual void addCaseCondHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void addCaseCondOption(int indLevel, Exp& opt) = 0;
	virtual void addCaseCondOptionEnd(int indLevel)        = 0;
	virtual void addCaseCondElse(int indLevel)             = 0;
	virtual void addCaseCondEnd(int indLevel) = 0;

	// if conditions
	virtual void addIfCondHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void addIfCondEnd(int indLevel) = 0;

	// if else conditions
	virtual void addIfElseCondHeader(int indLevel, const SharedExp& cond) = 0;
	virtual void addIfElseCondOption(int indLevel) = 0;
	virtual void addIfElseCondEnd(int indLevel)    = 0;

	// goto, break, continue, etc
	virtual void addGoto(int indLevel, int ord) = 0;
	virtual void addBreak(int indLevel)         = 0;
	virtual void addContinue(int indLevel)      = 0;

	// labels
	virtual void addLabel(int indLevel, int ord) = 0;
	virtual void removeLabel(int ord)            = 0;
	virtual void removeUnusedLabels(int maxOrd)  = 0;

	// sequential statements
	virtual void addAssignmentStatement(int indLevel, Assign *s) = 0;
	virtual void addCallStatement(int indLevel, Function *proc, const QString& name, StatementList& args,
								  StatementList *results) = 0;
	virtual void addIndCallStatement(int indLevel, const SharedExp& exp, StatementList& args, StatementList *results) = 0;
	virtual void addReturnStatement(int indLevel, StatementList *rets) = 0;

	// procedure related
	virtual void addProcStart(UserProc *proc) = 0;
	virtual void addProcEnd() = 0;
	virtual void addLocal(const QString& name, SharedType type, bool last = false) = 0;
	virtual void addGlobal(const QString& name, SharedType type, const SharedExp& init = nullptr) = 0;
	virtual void addPrototype(UserProc *proc) = 0;

	// comments
	virtual void addLineComment(const QString& cmt) = 0;

	/*
	 * output functions, pure virtual.
	 */
	virtual void print(QTextStream& os) = 0;

protected:
	UserProc *m_proc; ///< Pointer to the enclosing UserProc
};
