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
 * FILE:	   chllcode.h
 * OVERVIEW:   Concrete class for the "C" high level language
 *			   This class is provides methods which are specific for the C
 *			   language binding.  I guess this will be the most popular output
 *			   language unless we do C++.
 *============================================================================*/

#ifndef _CHLLCODE_H_
#define _CHLLCODE_H_

#include <string>
#include <sstream>

class BasicBlock;
class Exp;
class Proc;
class Assign;
class LocationSet;

// Operator precedence
/*
Operator Name				Associativity	Operators
Primary scope resolution	left to right	::
Primary						left to right	()	[ ]	 .	-> dynamic_cast typeid
Unary						right to left	++	--	+  -  !	 ~	&  *
											(type_name)	 sizeof new delete
C++ Pointer to Member		left to right	.* ->*
Multiplicative				left to right	*  /  %
Additive					left to right	+  -
Bitwise Shift				left to right	<<	>>
Relational					left to right	<  >  <=  >=
Equality					left to right	==	!=
Bitwise AND					left to right	&
Bitwise Exclusive OR		left to right	^
Bitwise Inclusive OR		left to right	|
Logical AND					left to right	&&
Logical OR					left to right	||
Conditional					right to left	? :
Assignment					right to left	=  +=  -=  *=	/=	<<=	 >>=  %=
											&=	^=	|=
Comma						left to right	,
*/

enum PREC {
	PREC_NONE=0,			// Outer level (no parens required)
	PREC_COMMA,				// Comma
	PREC_ASSIGN,			// Assignment
	PREC_COND,				// Conditional
	PREC_LOG_OR,			// Logical OR
	PREC_LOG_AND,			// Logical AND
	PREC_BIT_IOR,			// Bitwise Inclusive OR
	PREC_BIT_XOR,			// Bitwise Exclusive OR
	PREC_BIT_AND,			// Bitwise AND
	PREC_EQUAL,				// Equality
	PREC_REL,				// Relational
	PREC_BIT_SHIFT,			// Bitwise Shift
	PREC_ADD,				// Additive
	PREC_MULT,				// Multiplicative
	PREC_PTR_MEM,			// C++ Pointer to Member
	PREC_UNARY,				// Unary
	PREC_PRIM,				// Primary
	PREC_SCOPE				// Primary scope resolution
};




class CHLLCode : public HLLCode {
private:
		std::list<char *> lines;

		void indent(std::ostringstream& str, int indLevel);
		void appendExp(std::ostringstream& str, Exp *exp, PREC curPrec, bool uns = false);
		void appendType(std::ostringstream& str, Type *typ);
		void appendTypeIdent(std::ostringstream& str, Type *typ, const char *ident);
		void openParen(std::ostringstream& str, PREC outer, PREC inner) {
			if (inner < outer) str << "("; }
		void closeParen(std::ostringstream& str, PREC outer, PREC inner) {
			if (inner < outer) str << ")"; }

		std::map<std::string, Type*> locals;

		std::set<int> usedLabels;

public:
		// constructor
				CHLLCode();
				CHLLCode(UserProc *p);

		// destructor
virtual			~CHLLCode();

		// clear this class, calls the base
virtual void	reset();

		/*
		 * Functions to add new code
		 */

		// pretested loops (cond is optional because it is in the bb [somewhere])
virtual void	AddPretestedLoopHeader(int indLevel, Exp *cond);
virtual void	AddPretestedLoopEnd(int indLevel);

		// endless loops
virtual void	AddEndlessLoopHeader(int indLevel);
virtual void	AddEndlessLoopEnd(int indLevel);

		// posttested loops
virtual void	AddPosttestedLoopHeader(int indLevel);
virtual void	AddPosttestedLoopEnd(int indLevel, Exp *cond);

		// case conditionals "nways"
virtual void	AddCaseCondHeader(int indLevel, Exp *cond);
virtual void	AddCaseCondOption(int indLevel, Exp *opt);
virtual void	AddCaseCondOptionEnd(int indLevel);
virtual void	AddCaseCondElse(int indLevel);
virtual void	AddCaseCondEnd(int indLevel);

		// if conditions
virtual void	AddIfCondHeader(int indLevel, Exp *cond);
virtual void	AddIfCondEnd(int indLevel);

		// if else conditions
virtual void	AddIfElseCondHeader(int indLevel, Exp *cond);
virtual void	AddIfElseCondOption(int indLevel);
virtual void	AddIfElseCondEnd(int indLevel);

		// goto, break, continue, etc
virtual void	AddGoto(int indLevel, int ord);
virtual void	AddContinue(int indLevel);
virtual void	AddBreak(int indLevel);

		// labels
virtual void	AddLabel(int indLevel, int ord);
virtual void	RemoveLabel(int ord);
virtual void	RemoveUnusedLabels(int maxOrd);

		// sequential statements
virtual void	AddAssignmentStatement(int indLevel, Assign *asgn);
virtual void	AddCallStatement(int indLevel, Proc *proc, const char *name, std::vector<Exp*> &args,
		std::vector<ReturnInfo>& rets);
virtual void	AddIndCallStatement(int indLevel, Exp *exp, std::vector<Exp*> &args);
virtual void	AddReturnStatement(int indLevel, std::vector<Exp*> &returns);

		// proc related
virtual void	AddProcStart(Signature *signature);
virtual void	AddProcEnd();
virtual void	AddLocal(const char *name, Type *type, bool last = false);
virtual void	AddGlobal(const char *name, Type *type, Exp *init = NULL);
virtual void	AddPrototype(Signature *signature);
private:
		void	AddProcDec(Signature *signature, bool open);	// Implement AddProcStart and AddPrototype
public:

		// comments
virtual void	AddLineComment(char* cmt);

		/*
		 * output functions
		 */
virtual void	print(std::ostream &os);
};

#endif
