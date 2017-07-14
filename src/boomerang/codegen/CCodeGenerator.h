#pragma once

/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

#include "boomerang/codegen/ICodeGenerator.h"

#include <string>
#include <sstream>

class BasicBlock;
class Exp;
class Function;
class Assign;
class LocationSet;

// Operator precedence

/*
 * Operator Name                Associativity    Operators
 * Primary scope resolution     left to right    ::
 * Primary                      left to right    ()    [ ]     .    -> dynamic_cast typeid
 * Unary                        right to left    ++    --    +  -  !     ~    &  *  (type_name)  sizeof new
 * delete
 * C++ Pointer to Member        left to right    .* ->*
 * Multiplicative               left to right    *  /  %
 * Additive                     left to right    +  -
 * Bitwise Shift                left to right    <<    >>
 * Relational                   left to right    <  >  <=  >=
 * Equality                     left to right    ==    !=
 * Bitwise AND                  left to right    &
 * Bitwise Exclusive OR         left to right    ^
 * Bitwise Inclusive OR         left to right    |
 * Logical AND                  left to right    &&
 * Logical OR                   left to right    ||
 * Conditional                  right to left    ? :
 * Assignment                   right to left    =  +=  -=  *=    /=    <<=     >>=  %=   &=    ^=    |=
 * Comma                        left to right    ,
 */

/// Operator precedence
enum PREC
{
	PREC_NONE = 0,  ///< Outer level (no parens required)
	PREC_COMMA,     ///< Comma
	PREC_ASSIGN,    ///< Assignment
	PREC_COND,      ///< Conditional
	PREC_LOG_OR,    ///< Logical OR
	PREC_LOG_AND,   ///< Logical AND
	PREC_BIT_IOR,   ///< Bitwise Inclusive OR
	PREC_BIT_XOR,   ///< Bitwise Exclusive OR
	PREC_BIT_AND,   ///< Bitwise AND
	PREC_EQUAL,     ///< Equality
	PREC_REL,       ///< Relational
	PREC_BIT_SHIFT, ///< Bitwise Shift
	PREC_ADD,       ///< Additive
	PREC_MULT,      ///< Multiplicative
	PREC_PTR_MEM,   ///< C++ Pointer to Member
	PREC_UNARY,     ///< Unary
	PREC_PRIM,      ///< Primary
	PREC_SCOPE      ///< Primary scope resolution
};


/**
 * Concrete class for the "C" high level language
 * This class provides methods which are specific for the C language binding.
 * I guess this will be the most popular output language unless we do C++.
 */
class CCodeGenerator : public ICodeGenerator
{
public:
	// constructor
	CCodeGenerator(UserProc *p);

	// destructor
	virtual ~CCodeGenerator();

	// clear this class, calls the base
	virtual void reset() override;

	/*
	 * Functions to add new code
	 */

	// pretested loops (cond is optional because it is in the bb [somewhere])
	virtual void addPretestedLoopHeader(int indLevel, const SharedExp& cond) override;
	virtual void addPretestedLoopEnd(int indLevel) override;

	// endless loops
	virtual void addEndlessLoopHeader(int indLevel) override;
	virtual void addEndlessLoopEnd(int indLevel) override;

	// posttested loops
	virtual void addPostTestedLoopHeader(int indLevel) override;
	virtual void addPostTestedLoopEnd(int indLevel, const SharedExp& cond) override;

	// case conditionals "nways"
	virtual void addCaseCondHeader(int indLevel, const SharedExp& cond) override;
	virtual void addCaseCondOption(int indLevel, Exp& opt) override;
	virtual void addCaseCondOptionEnd(int indLevel) override;
	virtual void addCaseCondElse(int indLevel) override;
	virtual void addCaseCondEnd(int indLevel) override;

	// if conditions
	virtual void addIfCondHeader(int indLevel, const SharedExp& cond) override;
	virtual void addIfCondEnd(int indLevel) override;

	// if else conditions
	virtual void addIfElseCondHeader(int indLevel, const SharedExp& cond) override;
	virtual void addIfElseCondOption(int indLevel) override;
	virtual void addIfElseCondEnd(int indLevel) override;

	// goto, break, continue, etc
	virtual void addGoto(int indLevel, int ord) override;
	virtual void addContinue(int indLevel) override;
	virtual void addBreak(int indLevel) override;

	// labels
	virtual void addLabel(int indLevel, int ord) override;
	virtual void removeLabel(int ord) override;
	virtual void removeUnusedLabels(int maxOrd) override;

	// sequential statements
	virtual void addAssignmentStatement(int indLevel, Assign *asgn) override;
	virtual void addCallStatement(int indLevel, Function *proc, const QString& name, StatementList& args,
								  StatementList *results) override;
	virtual void addIndCallStatement(int indLevel, const SharedExp& exp, StatementList& args, StatementList *results) override;
	virtual void addReturnStatement(int indLevel, StatementList *rets) override;

	// proc related
	virtual void addProcStart(UserProc *proc) override;
	virtual void addProcEnd() override;
	virtual void addLocal(const QString& name, SharedType type, bool last = false) override;
	virtual void addGlobal(const QString& name, SharedType type, const SharedExp& init = nullptr) override;
	virtual void addPrototype(UserProc *proc) override;

public:
	// comments
	virtual void addLineComment(const QString& cmt) override;

	/*
	 * output functions
	 */
	virtual void print(QTextStream& os) override;

private:
	void addProcDec(UserProc *proc, bool open); // Implement AddProcStart and AddPrototype

private:
	/// The generated code.
	QStringList m_lines;

	/// Output 4 * \a indLevel spaces to \a str
	void indent(QTextStream& str, int indLevel);

	/**
	* Append code for the given expression \a exp to stream \a str.
	*
	* \param str        The stream to output to.
	* \param exp        The expresson to output.
	* \param curPrec     The current operator precedence. Add parens around this expression if necessary.
	* \param uns         If true, cast operands to unsigned if necessary.
	*
	* \todo This function is 800+ lines, and should possibly be split up.
	*/
	void appendExp(QTextStream& str, const Exp& exp, PREC curPrec, bool uns = false);
	void appendType(QTextStream& str, SharedType typ);
	void appendTypeIdent(QTextStream& str, SharedType typ, QString ident);

	/// Adds: (
	void openParen(QTextStream& str, PREC outer, PREC inner);

	/// Adds: )
	void closeParen(QTextStream& str, PREC outer, PREC inner);

	void appendLine(const QString& s);

	/// All locals in a Proc
	std::map<QString, SharedType> locals;

	/// All used goto labels.
	std::set<int> usedLabels;
};
