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

	/// Remove all generated code.
	virtual void reset() override;

	/*
	 * Functions to add new code
	 */

	// pretested loops (cond is optional because it is in the bb [somewhere])
	/// Adds: while (\p cond) {
	virtual void addPretestedLoopHeader(int indLevel, const SharedExp& cond) override;
	/// Adds: }
	virtual void addPretestedLoopEnd(int indLevel) override;

	// endless loops
	/// Adds: for(;;) {
	virtual void addEndlessLoopHeader(int indLevel) override;
	/// Adds: }
	virtual void addEndlessLoopEnd(int indLevel) override;

	// posttested loops
	/// Adds: do {
	virtual void addPostTestedLoopHeader(int indLevel) override;
	/// Adds: } while (\a cond);
	virtual void addPostTestedLoopEnd(int indLevel, const SharedExp& cond) override;

	// case conditionals "nways"
	/// Adds: switch(\a cond) {
	virtual void addCaseCondHeader(int indLevel, const SharedExp& cond) override;
	/// Adds: case \a opt :
	virtual void addCaseCondOption(int indLevel, Exp& opt) override;
	/// Adds: break;
	virtual void addCaseCondOptionEnd(int indLevel) override;
	/// Adds: default:
	virtual void addCaseCondElse(int indLevel) override;
	/// Adds: }
	virtual void addCaseCondEnd(int indLevel) override;

	// if conditions
	/// Adds: if(\a cond) {
	virtual void addIfCondHeader(int indLevel, const SharedExp& cond) override;
	/// Adds: }
	virtual void addIfCondEnd(int indLevel) override;

	// if else conditions
	/// Adds: if(\a cond) {
	virtual void addIfElseCondHeader(int indLevel, const SharedExp& cond) override;
	/// Adds: } else {
	virtual void addIfElseCondOption(int indLevel) override;
	/// Adds: }
	virtual void addIfElseCondEnd(int indLevel) override;

	// goto, break, continue, etc
	/// Adds: goto L \em ord
	virtual void addGoto(int indLevel, int ord) override;
	/// Adds: continue;
	virtual void addContinue(int indLevel) override;
	/// Adds: break;
	virtual void addBreak(int indLevel) override;

	// labels
	/// Adds: L \a ord :
	virtual void addLabel(int indLevel, int ord) override;
	/// Search for the label L \a ord and remove it from the generated code.
	virtual void removeLabel(int ord) override;

	/**
	* Removes labels from the code which are not in usedLabels.
	* \param maxOrd UNUSED
	*/
	virtual void removeUnusedLabels(int maxOrd) override;

	// sequential statements
	/// Prints an assignment expression.
	virtual void addAssignmentStatement(int indLevel, Assign *asgn) override;

	/**
	* Adds a call to \a proc.
	*
	* \param indLevel        A string containing spaces to the indentation level.
	* \param proc            The Proc the call is to.
	* \param name            The name the Proc has.
	* \param args            The arguments to the call.
	* \param results        The variable that will receive the return value of the function.
	*
	* \todo                Remove the \a name parameter and use Proc::getName()
	* \todo                Add assingment for when the function returns a struct.
	*/
	virtual void addCallStatement(int indLevel, Function *proc, const QString& name, StatementList& args,
								  StatementList *results) override;

	/**
	* Adds an indirect call to \a exp.
	* \see AddCallStatement
	* \param results UNUSED
	* \todo Add the use of \a results like AddCallStatement.
	*/
	virtual void addIndCallStatement(int indLevel, const SharedExp& exp, StatementList& args, StatementList *results) override;

	/**
	* Adds a return statement and returns the first expression in \a rets.
	* \todo This should be returning a struct if more than one real return value.
	*/
	virtual void addReturnStatement(int indLevel, StatementList *rets) override;

	// proc related
	/**
	* Print the start of a function, and also as a comment its address.
	*/
	virtual void addProcStart(UserProc *proc) override;

	/// Adds: }
	virtual void addProcEnd() override;

	/**
	 * Declare a local variable.
	 * \param name given to the new local
	 * \param type of this local variable
	 * \param last true if an empty line should be added.
	 */
	virtual void addLocal(const QString& name, SharedType type, bool last = false) override;

	/**
	* Add the declaration for a global.
	* \param name given name for the global
	* \param type The type of the global
	* \param init The initial value of the global.
	*/
	virtual void addGlobal(const QString& name, SharedType type, const SharedExp& init = nullptr) override;
	/// Add a prototype (for forward declaration)
	virtual void addPrototype(UserProc *proc) override;

public:
	/// Adds one line of comment to the code.
	virtual void addLineComment(const QString& cmt) override;

	/// Dump all generated code to \a os.
	virtual void print(QTextStream& os) override;

private:
	/**
	* Print the declaration of a function.
	* \param proc to print
	* \param open False if this is just a prototype and ";" should be printed instead of "{"
	*/
	void addProcDec(UserProc *proc, bool open); // Implement AddProcStart and AddPrototype

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

	/// Print the type represented by \a typ to \a str.
	void appendType(QTextStream& str, SharedType typ);

	/**
	* Print the indented type to \a str.
	*/
	void appendTypeIdent(QTextStream& str, SharedType typ, QString ident);

	/// Adds: (
	void openParen(QTextStream& str, PREC outer, PREC inner);

	/// Adds: )
	void closeParen(QTextStream& str, PREC outer, PREC inner);

	/// Private helper functions, to reduce redundant code, and
	/// have a single place to put a breakpoint on.
	void appendLine(const QString& s);

	/// All locals in a Proc
	std::map<QString, SharedType> locals;

	/// All used goto labels.
	std::set<int> usedLabels;

private:
	/// The generated code.
	QStringList m_lines;
};
