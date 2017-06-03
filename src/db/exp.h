/*
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 */

/***************************************************************************/ /**
 * \file       exp.h
 * \brief   Provides the definition for the Exp class and its subclasses.
 ******************************************************************************/

#pragma once

/* Main class hierarchy:    Exp (abstract)
 *                    _____/ | \
 *                   /         |    \
 *                Unary       Const Terminal
 *   TypedExp____/    |    \          \
 *    FlagDef___/ Binary Location  TypeVal
 *     RefExp__/    |
 *               Ternary
 */
#include "include/operator.h" // Declares the OPER enum
#include "include/types.h"    // For ADDRESS, etc
#include "include/type.h"     // The Type class for typed expressions
#include "include/util.h"
#include "exphelp.h"

#include <QtCore/QString>

#include <cstdio>  // For sprintf
#include <list>
#include <vector>
#include <set>
#include <cassert>
#include <memory>

class UseSet;
class DefSet;
class RTL; // For class FlagDef
class Instruction;
class BasicBlock;
class LocationSet;
class InstructionSet;
class TypeVal;
class ExpVisitor;
class ExpModifier;
class XMLProgParser;
class Function;
class UserProc;
class Exp;

#define DEBUG_BUFSIZE    5000 // Size of the debug print buffer

typedef std::unique_ptr<Exp>         UniqExp;
typedef std::shared_ptr<Exp>         SharedExp;
typedef std::shared_ptr<const Exp>   SharedConstExp;

typedef std::shared_ptr<RTL>         SharedRTL;

/**
 * \class Exp
 * An expression class, though it will probably be used to hold many other things (e.g. perhaps transformations).
 * It is a standard tree representation. Exp itself is abstract.
 * A special class Const is used for constants.
 * Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions respectively.
 * For efficiency of representation, these have to be separate classes, derived from Exp.
 */
class Exp : public Printable, public std::enable_shared_from_this<Exp>
{
protected:
	OPER m_oper; ///< The operator (e.g. opPlus)
	mutable unsigned m_lexBegin = 0, m_lexEnd = 0;

	// Constructor, with ID
	Exp(OPER op)
		: m_oper(op) {}

public:
	virtual ~Exp() {}

	/// Return the operator. Note: I'd like to make this protected, but then subclasses don't seem to be able to use
	/// it (at least, for subexpressions)
	OPER getOper() const { return m_oper; }
	const char *getOperName() const;

	// A few simplifications use this
	void setOper(OPER x) { m_oper = x; }

	void setLexBegin(unsigned int n) const { m_lexBegin = n; }
	void setLexEnd(unsigned int n) const { m_lexEnd = n; }
	unsigned getLexBegin() const { return m_lexBegin; }
	unsigned getLexEnd() const { return m_lexEnd; }
	QString toString() const override;

	/// Print the expression to the given stream
	virtual void print(QTextStream& os, bool html = false) const = 0;

	/***************************************************************************/ /**
	 * \brief  Print an infix representation of the object to the given file stream,
	 *         with its type in \<angle brackets\>.
	 * \param os Output stream to send the output to
	 ******************************************************************************/
	void printt(QTextStream& os) const;

	/***************************************************************************/ /**
	 * \brief        Print an infix representation of the object to the given file stream, but convert r[10] to r10 and
	 *               v[5] to v5
	 * \note   Never modify this function to emit debugging info; the back ends rely on this being clean to emit
	 *         correct C.  If debugging is desired, use operator<<
	 * \param os Output stream to send the output to
	 ******************************************************************************/
	void printAsHL(QTextStream& os); ///< Print with v[5] as v5
	char *prints();                  ///< Print to string (for debugging and logging)
	void dump();                     ///< Print to standard error (for debugging)

	/// Recursive print: don't want parens at the top level
	virtual void printr(QTextStream& os, bool html = false) const { print(os, html); }

	// But most classes want standard
	// For debugging: print in indented hex. In gdb: "p x->printx(0)"
	virtual void printx(int ind) const = 0;

	/// Display as a dotty graph
	void createDotFile(const char *name);
	virtual void appendDotFile(QTextStream& os) = 0;

	/// Clone (make copy of self that can be deleted without affecting self)
	virtual SharedExp clone() const = 0;

	/// Type sensitive equality
	virtual bool operator==(const Exp& o) const = 0;

	/// Type sensitive less than
	virtual bool operator<(const Exp& o) const = 0;

	/// Type insensitive less than. Class TypedExp overrides
	virtual bool operator<<(const Exp& o) const { return(*this < o); }

	/// Comparison ignoring subscripts
	virtual bool operator*=(const Exp& o) const = 0;

	/// Return the number of subexpressions. This is only needed in rare cases.
	/// Could use polymorphism for all those cases, but this is easier
	virtual int getArity() const { return 0; } // Overridden for Unary, Binary, etc

	//    //    //    //    //    //    //
	//     Enquiry functions    //
	//    //    //    //    //    //    //

	/// True if this is a call to a flag function
	bool isFlagCall() const { return m_oper == opFlagCall; }
	/// True if this represents one of the abstract flags locations, int or float
	bool isFlags() const { return m_oper == opFlags || m_oper == opFflags; }
	/// True if is one of the main 4 flags
	bool isMainFlag() const { return m_oper >= opZF && m_oper <= opOF; }
	/// True if this is a register location
	bool isRegOf() const { return m_oper == opRegOf; }

	/***************************************************************************/ /**
	 * \brief        Returns true if the expression is r[K] where K is int const
	 * \returns      True if matches
	 ******************************************************************************/
	bool isRegOfK();

	/***************************************************************************/ /**
	 * \brief        Returns true if the expression is r[N] where N is the given int const
	 * \param N - the specific register to be tested for
	 * \returns      True if this is a specific numeric register
	 ******************************************************************************/
	bool isRegN(int n) const;

	/// True if this is a memory location (any memory nesting depth)
	bool isMemOf() const { return m_oper == opMemOf; }
	/// True if this is an address of
	bool isAddrOf() const { return m_oper == opAddrOf; }
	/// True if this is an array expression
	bool isArrayIndex() const { return m_oper == opArrayIndex; }
	/// True if this is a struct member access
	bool isMemberOf() const { return m_oper == opMemberAccess; }
	/// True if this is a temporary. Note some old code still has r[tmp]
	bool isTemp() const;

	/// True if this is the anull Terminal (anulls next instruction)
	bool isAnull() const { return m_oper == opAnull; }
	/// True if this is the Nil Terminal (terminates lists; "NOP" expression)
	bool isNil() const { return m_oper == opNil; }
	/// True if this is %pc
	bool isPC() { return m_oper == opPC; }

	/***************************************************************************/ /**
	 * \brief        Returns true if is %afp, %afp+k, %afp-k, or a[m[<any of these]]
	 * \returns            True if found
	 ******************************************************************************/
	bool isAfpTerm();

	/// True if is int const
	bool isIntConst() const { return m_oper == opIntConst; }
	/// True if is string const
	bool isStrConst() const { return m_oper == opStrConst; }
	/// Get string constant even if mangled
	QString getAnyStrConst();

	/// True if is flt point const
	bool isFltConst() const { return m_oper == opFltConst; }
	/// True if inteter or string constant
	bool isConst() const { return m_oper == opIntConst || m_oper == opStrConst; }
	/// True if is a post-var expression (var_op' in SSL file)
	bool isPostVar() const { return m_oper == opPostVar; }
	/// True if this is an opSize (size case; deprecated)
	bool isSizeCast() const { return m_oper == opSize; }
	/// True if this is a subscripted expression (SSA)
	bool isSubscript() const { return m_oper == opSubscript; }
	// True if this is a phi assignmnet (SSA)
	//        bool        isPhi() {return op == opPhi;}
	/// True if this is a local variable
	bool isLocal() const { return m_oper == opLocal; }
	/// True if this is a global variable
	bool isGlobal() const { return m_oper == opGlobal; }
	/// True if this is a typeof
	bool isTypeOf() const { return m_oper == opTypeOf; }

	/***************************************************************************/ /**
	 * \brief        Returns the index for this var, e.g. if v[2], return 2
	 * \returns      The index
	 ******************************************************************************/
	int getVarIndex();

	/// True if this is a terminal
	virtual bool isTerminal() const { return false; }
	/// True if this is the constant "true"
	bool isTrue() const { return m_oper == opTrue; }
	/// True if this is the constant "false"
	bool isFalse() const { return m_oper == opFalse; }
	/// True if this is a disjunction, i.e. x or y
	bool isDisjunction() const { return m_oper == opOr; }
	/// True if this is a conjunction, i.e. x and y
	bool isConjunction() const { return m_oper == opAnd; }
	/// True if this is a boolean constant
	bool isBoolConst() const { return m_oper == opTrue || m_oper == opFalse; }
	/// True if this is an equality (== or !=)
	bool isEquality() const { return m_oper == opEquals /*|| op == opNotEqual*/; }

	/// True if this is a comparison
	bool isComparison() const
	{
		return m_oper == opEquals || m_oper == opNotEqual || m_oper == opGtr || m_oper == opLess || m_oper == opGtrUns || m_oper == opLessUns ||
			   m_oper == opGtrEq || m_oper == opLessEq || m_oper == opGtrEqUns || m_oper == opLessEqUns;
	}

	/// True if this is a TypeVal
	bool isTypeVal() const { return m_oper == opTypeVal; }
	/// True if this is a machine feature
	bool isMachFtr() const { return m_oper == opMachFtr; }
	/// True if this is a parameter. Note: opParam has two meanings: a SSL parameter, or a function parameter
	bool isParam() const { return m_oper == opParam; }

	/// True if this is a location
	bool isLocation() const { return m_oper == opMemOf || m_oper == opRegOf || m_oper == opGlobal || m_oper == opLocal || m_oper == opParam; }
	/// True if this is a typed expression
	bool isTypedExp() const { return m_oper == opTypedExp; }

	// FIXME: are these used?
	// Matches this expression to the pattern, if successful returns a list of variable bindings, otherwise returns
	// nullptr
	virtual SharedExp match(const SharedConstExp& pattern);

	/// match a string pattern
	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings);

	//    //    //    //    //    //    //
	//    Search and Replace    //
	//    //    //    //    //    //    //

	/// Search for Exp *search in this Exp. If found, return true and return a ptr to the matching expression in
	/// result (useful with wildcards).

	/***************************************************************************/ /**
	 * \brief  Search this expression for the given subexpression, and if found, return true and return a pointer
	 *         to the matched expression in result
	 *         useful when there are wildcards, e.g. search pattern is *r[?] result is r[2].
	 * \param   search     ptr to Exp we are searching for
	 * \param   result     ref to ptr to Exp that matched
	 * \returns            True if a match was found
	 ******************************************************************************/
	virtual bool search(const Exp& search, SharedExp& result);

	// Search for Exp search in this Exp. For each found, add a ptr to the matching expression in result (useful
	// with wildcards).      Does NOT clear result on entry

	/***************************************************************************/ /**
	 * \brief        Search this expression for the given subexpression, and for each found, return a pointer to the
	 *                      matched expression in result
	 * \param   search     ptr to Exp we are searching for
	 * \param   result  ref to list of Exp that matched
	 * \returns            True if a match was found
	 ******************************************************************************/
	bool searchAll(const Exp& search, std::list<SharedExp>& result);

	/// Search this Exp for *search; if found, replace with *replace

	/***************************************************************************/ /**
	 * \brief   Search for the given subexpression, and replace if found
	 * \note    If the top level expression matches, return val != this
	 *
	 * \param       search - reference to Exp we are searching for
	 * \param       replace - ptr to Exp to replace it with
	 * \param       change - ref to boolean, set true if a change made (else cleared)
	 * \returns     True if a change made
	 ******************************************************************************/
	SharedExp searchReplace(const Exp& search, const SharedExp& replace, bool& change);

	/***************************************************************************/ /**
	 * \brief   Search for the given subexpression, and replace wherever found
	 * \note    If the top level expression matches, something other than "this" will be returned
	 * \note    It is possible with wildcards that in very unusual circumstances a replacement will be made to
	 *              something that is already deleted.
	 * \note    Replacements are cloned. Caller to delete search and replace
	 * \note    \a change is ALWAYS assigned. No need to clear beforehand.
	 *
	 * \param   search     reference to Exp we are searching for
	 * \param   replace ptr to Exp to replace it with
	 * \param   change set true if a change made; cleared otherwise
	 * \param   once - if set to true only the first possible replacement will be made
	 *
	 * \returns the result (often this, but possibly changed)
	 ******************************************************************************/
	SharedExp searchReplaceAll(const Exp& search, const SharedExp& replace, bool& change, bool once = false);

	/***************************************************************************/ /**
	 * \brief   Search for the given subexpression
	 * \note    Caller must free the list li after use, but not the Exp objects that they point to
	 * \note    If the top level expression matches, li will contain search
	 * \note    Now a static function. Searches pSrc, not this
	 * \note    Mostly not for public use.
	 *
	 * \param   search ptr to Exp we are searching for
	 * \param   pSrc ref to ptr to Exp to search. Reason is that we can then overwrite that pointer
	 *               to effect a replacement. So we need to append &pSrc in the list. Can't append &this!
	 * \param   li   list of Exp** where pointers to the matches are found
	 * \param   once true if not all occurrences to be found, false for all
	 *
	 ******************************************************************************/
	static void doSearch(const Exp& search, SharedExp& pSrc, std::list<SharedExp *>& li, bool once);

	/***************************************************************************/ /**
	 * \brief       Search for the given subexpression in all children
	 * \note        Virtual function; different implementation for each subclass of Exp
	 * \note            Will recurse via doSearch
	 *
	 * \param       search - ptr to Exp we are searching for
	 * \param       li - list of Exp** where pointers to the matches are found
	 * \param       once - true if not all occurrences to be found, false for all
	 ******************************************************************************/
	virtual void doSearchChildren(const Exp&, std::list<SharedExp *>&, bool);

	/// Propagate all possible assignments to components of this expression.
	/// Propagate all possible statements to this expression.
	SharedExp propagateAll();

	/**
	 * As above, but keep propagating until no change.
	 * Propagate all possible statements to this expression,
	 * and repeat until there is no further change.
	 */
	SharedExp propagateAllRpt(bool& changed);

	//    //    //    //    //    //    //
	//      Sub expressions    //
	//    //    //    //    //    //    //

	/**
	 * These are here so we can (optionally) prevent code clutter.
	 * Using a *Exp (that is known to be a Binary* say), you can just directly call getSubExp2
	 * However, you can still choose to cast from Exp* to Binary* etc. and avoid the virtual call
	 */
	template<class T>
	std::shared_ptr<T> access()
	{
		return shared_from_base<T>();
	}

	template<class T>
	std::shared_ptr<const T> access() const { return const_cast<Exp *>(this)->access<T>(); }

	template<class T, int SUB_IDX, int ... Path>
	std::shared_ptr<T> access()
	{
		switch (SUB_IDX)
		{
		case 1:
			return getSubExp1()->access<T, Path ...>();

		case 2:
			return getSubExp2()->access<T, Path ...>();

		case 3:
			return getSubExp3()->access<T, Path ...>();

		default:
			assert(false);
		}
	}

	template<class T, int SUB_IDX, int ... Path>
	std::shared_ptr<const T> access() const
	{
		switch (SUB_IDX)
		{
		case 1:
			return getSubExp1()->access<T, Path ...>();

		case 2:
			return getSubExp2()->access<T, Path ...>();

		case 3:
			return getSubExp3()->access<T, Path ...>();

		default:
			assert(false);
		}
	}

	/***************************************************************************/ /**
	 * \brief    Get subexpression
	 * \returns  Pointer to the requested subexpression
	 ******************************************************************************/
	virtual SharedExp getSubExp1() { return nullptr; }
	virtual SharedConstExp getSubExp1() const { return nullptr; }
	virtual SharedExp getSubExp2() { return nullptr; }
	virtual SharedConstExp getSubExp2() const { return nullptr; }
	virtual SharedExp getSubExp3() { return nullptr; }
	virtual SharedConstExp getSubExp3() const { return nullptr; }
	virtual SharedExp& refSubExp1();
	virtual SharedExp& refSubExp2();
	virtual SharedExp& refSubExp3();

	/***************************************************************************/ /**
	 * \brief  Set requested subexpression; 1 is first
	 * \param  e Pointer to subexpression to set
	 * \note   If an expression already exists, it is ;//deleted
	 ******************************************************************************/
	virtual void setSubExp1(SharedExp /*e*/) { assert(false); }
	virtual void setSubExp2(SharedExp /*e*/) { assert(false); }
	virtual void setSubExp3(SharedExp /*e*/) { assert(false); }

	// Get the complexity depth. Basically, add one for each unary, binary, or ternary
	int getComplexityDepth(UserProc *proc);

	// Get memory depth. Add one for each m[]
	int getMemDepth();

	//    //    //    //    //    //    //
	//    Guarded assignment    //
	//    //    //    //    //    //    //

	/***************************************************************************/ /**
	 * \brief        Returns a ptr to the guard expression, or 0 if none
	 * \returns            Ptr to the guard, or 0
	 ******************************************************************************/
	SharedExp getGuard(); // Get the guard expression, or 0 if not

	//    //    //    //    //    //    //    //    //
	//    Expression Simplification    //
	//    //    //    //    //    //    //    //    //


	// These simplifying functions don't really belong in class Exp, but they know too much about how Exps work
	// They can't go into util.so, since then util.so and db.so would co-depend on each other for testing at least

	/***************************************************************************/ /**
	 * \brief        Takes an expression consisting on only + and - operators and partitions its terms into positive
	 *               non-integer fixed terms, negative non-integer fixed terms and integer terms. For example, given:
	 *                   %sp + 108 + n - %sp - 92
	 *               the resulting partition will be:
	 *                   positives = { %sp, n }
	 *                   negatives = { %sp }
	 *                   integers  = { 108, -92 }
	 * \note         integers is a vector so we can use the accumulate func
	 * \note         Expressions are NOT cloned. Therefore, do not delete the expressions in positives or negatives
	 *
	 * \param positives - the list of positive terms
	 * \param negatives - the list of negative terms
	 * \param integers - the vector of integer terms
	 * \param negate - determines whether or not to negate the whole expression, i.e. we are on the RHS of an opMinus
	 ******************************************************************************/
	void partitionTerms(std::list<SharedExp>& positives, std::list<SharedExp>& negatives, std::vector<int>& integers,
						bool negate);

	/***************************************************************************/ /**
	 * \brief        This method simplifies an expression consisting of + and - at the top level. For example,
	 *               (%sp + 100) - (%sp + 92) will be simplified to 8.
	 * \note         Any expression can be so simplified
	 * \note         User must ;//delete result
	 * \returns      Ptr to the simplified expression
	 ******************************************************************************/
	virtual SharedExp simplifyArith() { return shared_from_this(); }

	/***************************************************************************/ /**
	 * \brief        This method creates an expression that is the sum of all expressions in a list.
	 *               E.g. given the list <4,r[8],m[14]> the resulting expression is 4+r[8]+m[14].
	 * \note         static (non instance) function
	 * \note         Exps ARE cloned
	 * \param        exprs - a list of expressions
	 * \returns      a new Exp with the accumulation
	 ******************************************************************************/
	static SharedExp accumulate(std::list<SharedExp>& exprs);

	/***************************************************************************/ /**
	 * \brief        Apply various simplifications such as constant folding. Also canonicalise by putting iteger
	 *               constants on the right hand side of sums, adding of negative constants changed to subtracting
	 *               positive constants, etc.  Changes << k to a multiply
	 * \note         User must ;//delete result
	 * \note         Address simplification (a[ m[ x ]] == x) is done separately
	 * \returns      Ptr to the simplified expression
	 *
	 * \internal
	 * This code is so big, so weird and so lame it's not funny.  What this boils down to is the process of
	 * unification.
	 * We're trying to do it with a simple iterative algorithm, but the algorithm keeps getting more and more complex.
	 * Eventually I will replace this with a simple theorem prover and we'll have something powerful, but until then,
	 * dont rely on this code to do anything critical. - trent 8/7/2002
	 ******************************************************************************/
	SharedExp simplify();

	virtual SharedExp polySimplify(bool& bMod)
	{
		bMod = false;
		return shared_from_this();
	}

	// Just the address simplification a[ m[ any ]]
	virtual SharedExp simplifyAddr() { return shared_from_this(); }
	virtual SharedExp simplifyConstraint() { return shared_from_this(); }

	/***************************************************************************/ /**
	 * \brief       Replace succ(r[k]) by r[k+1]
	 * \note        Could change top level expression
	 * \returns     Fixed expression
	 ******************************************************************************/
	SharedExp fixSuccessor(); // succ(r2) -> r3

	/// Kill any zero fill, sign extend, or truncates

	/***************************************************************************/ /**
	 * \brief        Remove size operations such as zero fill, sign extend
	 * \note         Could change top level expression
	 * \note         Does not handle truncation at present
	 * \returns      Fixed expression
	 ******************************************************************************/
	SharedExp killFill();

	/// Do the work of finding used locations. If memOnly set, only look inside m[...]
	/// Find the locations used by this expression. Use the UsedLocsFinder visitor class
	/// If memOnly is true, only look inside m[...]
	void addUsedLocs(LocationSet& used, bool memOnly = false);

	/// allZero is set if all subscripts in the whole expression are null or implicit; otherwise cleared
	SharedExp removeSubscripts(bool& allZero);

	/// Get number of definitions (statements this expression depends on)
	virtual int getNumRefs() { return 0; }

	/// Convert from SSA form, where this is not subscripted (but defined at statement d)
	/// Needs the UserProc for the symbol map
	// FIXME: if the wrapped expression does not convert to a location, the result is subscripted, which is probably not
	// what is wanted!
	SharedExp fromSSAleft(UserProc *proc, Instruction *d);

	/// Generate constraints for this Exp.
	///
	/// \note The behaviour is a bit different depending on whether or not
	/// parameter result is a type constant or a type variable.
	/// If the constraint is always satisfied, return true
	/// If the constraint can never be satisfied, return false
	///
	/// Example: this is opMinus and result is <int>, constraints are:
	///     sub1 = <int> and sub2 = <int> or
	///     sub1 = <ptr> and sub2 = <ptr>
	/// Example: this is opMinus and result is Tr (typeOf r), constraints are:
	///     sub1 = <int> and sub2 = <int> and Tr = <int> or
	///     sub1 = <ptr> and sub2 = <ptr> and Tr = <int> or
	///     sub1 = <ptr> and sub2 = <int> and Tr = <ptr>
	virtual SharedExp genConstraints(SharedExp restrictTo);

	/// Visitation
	/// Note: best to have accept() as pure virtual, so you don't forget to implement it for new subclasses of Exp
	virtual bool accept(ExpVisitor *v)       = 0;
	virtual SharedExp accept(ExpModifier *v) = 0;
	void fixLocationProc(UserProc *p);
	UserProc *findProc();

	/// Set or clear the constant subscripts
	void setConscripts(int n, bool bClear);

	/// Strip size casts from an Exp
	SharedExp stripSizes();

	/// Subscript all e in this Exp with statement def
	/// Subscript any occurrences of e with e{def} in this expression
	SharedExp expSubscriptVar(const SharedExp& e, Instruction *def /*, Cfg* cfg */);

	/// Subscript all e in this Exp with 0 (implicit assignments)
	/// Subscript any occurrences of e with e{-} in this expression
	/// \note subscript with nullptr, not implicit assignments as above
	SharedExp expSubscriptValNull(const SharedExp& e /*, Cfg* cfg */);

	/// Subscript all locations in this expression with their implicit assignments
	SharedExp expSubscriptAllNull(/*Cfg* cfg*/);

	/// Perform call bypass and simple (assignment only) propagation to this exp
	/// Note: can change this, so often need to clone before calling
	SharedExp bypass();
	void bypassComp();                  ///< As above, but only the xxx of m[xxx]
	bool containsFlags();               ///< Check if this exp contains any flag calls

	/// Check if this expression contains a bare memof (no subscripts) or one that has no symbol (i.e. is not a local
	/// variable or a parameter)
	bool containsBadMemof(UserProc *p); ///< Check if this Exp contains a bare (non subscripted) memof

	/// \note No longer used?
	bool containsMemof(UserProc *proc); ///< Check of this Exp contains any memof at all. Not used.

	// Data flow based type analysis (implemented in type/dfa.cpp)
	// Pull type information up the expression tree
	virtual SharedType ascendType()
	{
		assert(false);
		return nullptr;
	}

	/// Push type information down the expression tree
	virtual void descendType(SharedType /*parentType*/, bool& /*ch*/, Instruction * /*s*/) { assert(0); }

protected:
	template<typename CHILD>
	std::shared_ptr<CHILD> shared_from_base()
	{
		return std::static_pointer_cast<CHILD>(shared_from_this());
	}
};

// Not part of the Exp class, but logically belongs with it:
QTextStream& operator<<(QTextStream& os, const Exp *p); // Print the Exp poited to by p

inline QTextStream& operator<<(QTextStream& os, const SharedConstExp& p)
{
	os << p.get();
	return os;
}


/***************************************************************************/ /**
 * Const is a subclass of Exp, and holds either an integer, floating point, string, or address constant
 ******************************************************************************/
class Const : public Exp
{
protected:
	friend class XMLProgParser;

private:
	union
	{
		int      i;    ///< Integer
		/// \note although we have i and a as unions, both often use the same operator (opIntConst).
		/// There is no opCodeAddr any more.
		ADDRESS  a;    ///< void* conflated with unsigned int: needs fixing
		QWord    ll;   ///< 64 bit integer
		double   d;    ///< Double precision float
		// const char *p; ///< Pointer to string

		/// Don't store string: function could be renamed
		Function *pp;      ///< Pointer to function
	}

	u;

	QString m_string;
	int m_conscript;   ///< like a subscript for constants
	SharedType m_type; ///< Constants need types during type analysis

public:
	// Special constructors overloaded for the various constants
	Const(uint32_t i);
	Const(int i);
	Const(QWord ll);

	/// \remark This is bad. We need a way of constructing true unsigned constants
	Const(ADDRESS a);
	Const(double d);
	//    Const(const char *p);
	Const(const QString& p);
	Const(Function *p);
	// Copy constructor
	Const(const Const& o);

	template<class T>
	static std::shared_ptr<Const> get(T i) { return std::make_shared<Const>(i); }

	/// Nothing to destruct: Don't deallocate the string passed to constructor
	virtual ~Const() {}

	// Clone
	virtual SharedExp clone() const override;

	// Compare

	/***************************************************************************/ /**
	 * \brief        Virtual function to compare myself for equality with
	 *               another Exp
	 * \param  o     Ref to other Exp
	 * \returns      True if equal
	 ******************************************************************************/
	virtual bool operator==(const Exp& o) const override;

	/***************************************************************************/ /**
	 * \brief      Virtual function to compare myself with another Exp
	 * \note       The test for a wildcard is only with this object, not the other object (o).
	 *             So when searching and there could be wildcards, use search == *this not *this == search
	 * \param      o - Ref to other Exp
	 * \returns    true if equal
	 ******************************************************************************/
	virtual bool operator<(const Exp& o) const override;

	/***************************************************************************/ /**
	 * \brief        Virtual function to compare myself for equality with another Exp, *ignoring subscripts*
	 * \param        o - Ref to other Exp
	 * \returns      True if equal
	 ******************************************************************************/
	virtual bool operator*=(const Exp& o) const override;

	// Get the constant
	int getInt() const { return u.i; }
	QWord getLong() const { return u.ll; }
	double getFlt() const { return u.d; }
	QString getStr() const { return m_string; }
	ADDRESS getAddr() const { return u.a; }
	QString getFuncName() const;

	// Set the constant
	void setInt(int i) { u.i = i; }
	void setLong(QWord ll) { u.ll = ll; }
	void setFlt(double d) { u.d = d; }
	void setStr(const QString& p) { m_string = p; }
	void setAddr(ADDRESS a) { u.a = a; }

	// Get and set the type
	SharedType getType() { return m_type; }
	const SharedType getType() const { return m_type; }
	void setType(SharedType ty) { m_type = ty; }

	/***************************************************************************/ /**
	 * \brief       "Print" in infix notation the expression to a stream.
	 *              Mainly for debugging, or maybe some low level windows
	 * \param       os  - Ref to an output stream
	 ******************************************************************************/
	virtual void print(QTextStream& os, bool = false) const override;

	/// Print "recursive" (extra parens not wanted at outer levels)
	void printNoQuotes(QTextStream& os) const;
	virtual void printx(int ind) const override;

	virtual void appendDotFile(QTextStream& of) override;
	virtual SharedExp genConstraints(SharedExp restrictTo) override;

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;

	/***************************************************************************/ /**
	 * \brief        Matches this expression to the given patten
	 * \param        pattern to match
	 * \returns            list of variable bindings, or nullptr if matching fails
	 ******************************************************************************/
	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

	int getConscript() const { return m_conscript; }
	void setConscript(int cs) { m_conscript = cs; }

	virtual SharedType ascendType() override;
	virtual void descendType(SharedType parentType, bool& ch, Instruction *s) override;
};

/***************************************************************************/ /**
 * Terminal is a subclass of Exp, and holds special zero arity items such as opFlags (abstract flags register)
 ******************************************************************************/
class Terminal : public Exp
{
protected:
	friend class XMLProgParser;

public:
	// Constructors
	Terminal(OPER op);
	Terminal(const Terminal& o); ///< Copy constructor

	static SharedExp get(OPER op) { return std::make_shared<Terminal>(op); }

	// Clone
	SharedExp clone() const override;

	// Compare
	bool operator==(const Exp& o) const override;
	bool operator<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;
	void print(QTextStream& os, bool = false) const override;
	void appendDotFile(QTextStream& of) override;
	void printx(int ind) const override;

	bool isTerminal() const override { return true; }

	/// Visitation
	bool accept(ExpVisitor *v) override;
	SharedExp accept(ExpModifier *v) override;

	SharedType ascendType() override;
	void descendType(SharedType parentType, bool& ch, Instruction *s) override;

	bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;
};

/***************************************************************************/ /**
 * Unary is a subclass of Exp, holding one subexpression
 ******************************************************************************/
class Unary : public Exp
{
protected:
	SharedExp subExp1; ///< One subexpression pointer

	/// Constructor, with just ID
	Unary(OPER op);

public:
	/// Constructor, with ID and subexpression
	Unary(OPER op, SharedExp e);
	/// Copy constructor
	Unary(const Unary& o);
	static SharedExp get(OPER op, SharedExp e1) { return std::make_shared<Unary>(op, e1); }

	// Clone
	virtual SharedExp clone() const override;

	// Compare
	virtual bool operator==(const Exp& o) const override;
	virtual bool operator<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;

	// Destructor
	virtual ~Unary();

	/// Arity
	virtual int getArity() const override { return 1; }

	// Print
	virtual void print(QTextStream& os, bool html = false) const override;
	virtual void appendDotFile(QTextStream& of) override;
	virtual void printx(int ind) const override;

	// Set first subexpression
	void setSubExp1(SharedExp e) override;

	// Get first subexpression
	SharedExp getSubExp1() override;
	SharedConstExp getSubExp1() const override;

	// Get a reference to subexpression 1
	SharedExp& refSubExp1() override;

	virtual SharedExp match(const SharedConstExp& pattern) override;
	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

	// Search children
	void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

	// Do the work of simplifying this expression

	/***************************************************************************/ /**
	 * \brief        Do the work of simplification
	 * \note         User must ;//delete result
	 * \note         Address simplification (a[ m[ x ]] == x) is done separately
	 * \returns      Ptr to the simplified expression
	 ******************************************************************************/
	virtual SharedExp polySimplify(bool& bMod) override;
	SharedExp simplifyArith() override;

	/***************************************************************************/ /**
	 * \brief        Just do addressof simplification: a[ m[ any ]] == any, m[ a[ any ]] = any, and also
	 *               a[ size m[ any ]] == any
	 * \todo         Replace with a visitor some day
	 * \returns      Ptr to the simplified expression
	 ******************************************************************************/
	SharedExp simplifyAddr() override;
	virtual SharedExp simplifyConstraint() override;

	// Type analysis
	SharedExp genConstraints(SharedExp restrictTo) override;

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;

	virtual SharedType ascendType() override;
	virtual void descendType(SharedType parentType, bool& ch, Instruction *s) override;

protected:
	friend class XMLProgParser;
}; // class Unary


/**
 * Binary is a subclass of Unary, holding two subexpressions
 */
class Binary : public Unary
{
protected:
	friend class XMLProgParser;

protected:
	SharedExp subExp2; ///< Second subexpression pointer

	// Constructor, with ID
	Binary(OPER op);

public:
	// Constructor, with ID and subexpressions
	Binary(OPER op, SharedExp e1, SharedExp e2);
	// Copy constructor
	Binary(const Binary& o);

	static std::shared_ptr<Binary> get(OPER op, SharedExp e1, SharedExp e2)
	{ return std::make_shared<Binary>(op, e1, e2); }

	// Clone
	virtual SharedExp clone() const override;

	// Compare
	bool operator==(const Exp& o) const override;
	bool operator<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;

	// Destructor
	virtual ~Binary();

	// Arity
	int getArity() const override { return 2; }

	// Print
	virtual void print(QTextStream& os, bool html = false) const override;
	virtual void printr(QTextStream& os, bool html = false) const override;
	virtual void appendDotFile(QTextStream& of) override;
	virtual void printx(int ind) const override;

	// Set second subexpression
	void setSubExp2(SharedExp e) override;

	// Get second subexpression
	SharedExp getSubExp2() override;
	SharedConstExp getSubExp2() const override;

	/***************************************************************************/ /**
	 * \brief        Swap the two subexpressions
	 ******************************************************************************/
	void commute();                   ///< Commute the two operands
	SharedExp& refSubExp2() override; ///< Get a reference to subexpression 2

	virtual SharedExp match(const SharedConstExp& pattern) override;
	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

	/// Search children
	void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

	/// Do the work of simplifying this expression
	virtual SharedExp polySimplify(bool& bMod) override;
	SharedExp simplifyArith() override;
	SharedExp simplifyAddr() override;
	virtual SharedExp simplifyConstraint() override;

	/// Type analysis
	SharedExp genConstraints(SharedExp restrictTo) override;

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;

	virtual SharedType ascendType() override;
	virtual void descendType(SharedType parentType, bool& ch, Instruction *s) override;

private:
	/// Return a constraint that my subexpressions have to be of type typeval1 and typeval2 respectively
	SharedExp constrainSub(const std::shared_ptr<TypeVal>& typeVal1, const std::shared_ptr<TypeVal>& typeVal2);
};


/***************************************************************************/ /**
 * Ternary is a subclass of Binary, holding three subexpressions
 ******************************************************************************/
class Ternary : public Binary
{
protected:
	friend class XMLProgParser;

private:
	SharedExp subExp3; // Third subexpression pointer

	// Constructor, with operator
	Ternary(OPER op);

public:
	// Constructor, with operator and subexpressions
	Ternary(OPER op, SharedExp e1, SharedExp e2, SharedExp e3);
	// Copy constructor
	Ternary(const Ternary& o);

	/***************************************************************************/ /**
	 * \brief        Virtual function to make a clone of myself, i.e. to create
	 *               a new Exp with the same contents as myself, but not sharing
	 *               any memory. Deleting the clone will not affect this object.
	 *               Pointers to subexpressions are not copied, but also cloned.
	 * \returns      Pointer to cloned object
	 ******************************************************************************/
	virtual SharedExp clone() const override;

	// Compare
	bool operator==(const Exp& o) const override;
	bool operator<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;

	// Destructor
	virtual ~Ternary();

	// Arity
	int getArity() const override { return 3; }

	// Print
	virtual void print(QTextStream& os, bool html = false) const override;
	virtual void printr(QTextStream& os, bool = false) const override;
	virtual void appendDotFile(QTextStream& of) override;
	virtual void printx(int ind) const override;

	// Set third subexpression
	void setSubExp3(SharedExp e) override;

	// Get third subexpression
	SharedExp getSubExp3() override;
	SharedConstExp getSubExp3() const override;

	// Get a reference to subexpression 3
	SharedExp& refSubExp3() override;

	// Search children
	void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once) override;

	SharedExp polySimplify(bool& bMod) override;
	SharedExp simplifyArith() override;
	SharedExp simplifyAddr() override;

	// Type analysis
	SharedExp genConstraints(SharedExp restrictTo) override;

	// Visitation
	bool accept(ExpVisitor *v) override;
	SharedExp accept(ExpModifier *v) override;

	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

	virtual SharedType ascendType() override;

	virtual void descendType(SharedType /*parentType*/, bool& ch, Instruction *s) override;
};


/***************************************************************************/ /**
 * TypedExp is a subclass of Unary, holding one subexpression and a Type
 ******************************************************************************/
class TypedExp : public Unary
{
protected:
	friend class XMLProgParser;

private:
	SharedType type;

public:
	// Constructor
	TypedExp();
	// Constructor, subexpression
	TypedExp(SharedExp e1);
	// Constructor, type, and subexpression.
	// A rare const parameter allows the common case of providing a temporary,
	// e.g. foo = new TypedExp(Type(INTEGER), ...);
	TypedExp(SharedType ty, SharedExp e1);
	// Copy constructor
	TypedExp(TypedExp& o);

	// Clone
	virtual SharedExp clone() const override;

	// Compare
	bool operator==(const Exp& o) const override;
	bool operator<(const Exp& o) const override;
	bool operator<<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;

	virtual void print(QTextStream& os, bool html = false) const override;
	virtual void appendDotFile(QTextStream& of) override;
	virtual void printx(int ind) const override;

	/// Get and set the type
	virtual SharedType getType()       { return type; }
	virtual const SharedType& getType() const { return type; }
	virtual void setType(SharedType ty) { type = ty; }

	// polySimplify
	virtual SharedExp polySimplify(bool& bMod) override;

	// Visitation
	/// All the Unary derived accept functions look the same, but they have to be repeated because the particular visitor
	/// function called each time is different for each class (because "this" is different each time)
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;

	virtual SharedType ascendType() override;

	virtual void descendType(SharedType, bool&, Instruction *) override;
};


/***************************************************************************/ /**
 * FlagDef is a subclass of Unary, and holds a list of parameters (in the subexpression), and a pointer to an RTL
 ******************************************************************************/
class FlagDef : public Unary
{
protected:
	friend class XMLProgParser;

private:
	SharedRTL rtl;

public:
	FlagDef(SharedExp params, SharedRTL rtl);
	virtual ~FlagDef();

	virtual void appendDotFile(QTextStream& of) override;

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;
};


/***************************************************************************/ /**
 * RefExp is a subclass of Unary, holding an ordinary Exp pointer, and a pointer to a defining statement (could be a
 * phi assignment).  This is used for subscripting SSA variables. Example:
 *   m[1000] becomes m[1000]{3} if defined at statement 3
 * The integer is really a pointer to the defining statement, printed as the statement number for compactness.
 ******************************************************************************/
class RefExp : public Unary
{
protected:
	friend class XMLProgParser;

private:
	Instruction *m_def; // The defining statement
	// Constructor with expression (e) and statement defining it (def)

public:
	RefExp(SharedExp e, Instruction *def);
	virtual ~RefExp()
	{
		m_def = nullptr;
	}

	static std::shared_ptr<RefExp> get(SharedExp e, Instruction *def);
	SharedExp clone() const override;
	bool operator==(const Exp& o) const override;
	bool operator<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;

	virtual void print(QTextStream& os, bool html = false) const override;
	virtual void printx(int ind) const override;

	Instruction *getDef() const { return m_def; } // Ugh was called getRef()
	SharedExp addSubscript(Instruction *_def)
	{
		m_def = _def;
		return shared_from_this();
	}

	void setDef(Instruction *_def)   /*assert(_def);*/
	{
		m_def = _def;
	}

	SharedExp genConstraints(SharedExp restrictTo) override;

	bool references(const Instruction *s) const { return m_def == s; }
	virtual SharedExp polySimplify(bool& bMod) override;
	virtual SharedExp match(const SharedConstExp& pattern) override;
	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

	// Before type analysis, implicit definitions are nullptr.  During and after TA, they point to an implicit
	// assignment statement.  Don't implement here, since it would require #including of statement.h
	bool isImplicitDef() const;

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;

	virtual SharedType ascendType() override;
	virtual void descendType(SharedType parentType, bool& ch, Instruction *s) override;

protected:
	RefExp()
		: Unary(opSubscript)
		, m_def(nullptr) {}
};


/***************************************************************************/ /**
 * class TypeVal. Just a Terminal with a Type. Used for type values in constraints
 * ==============================================================================*/
class TypeVal : public Terminal
{
protected:
	friend class XMLProgParser;

private:
	SharedType val;

public:
	TypeVal(SharedType ty);
	~TypeVal();

	static std::shared_ptr<TypeVal> get(const SharedType& st) { return std::make_shared<TypeVal>(st); }

	virtual SharedType getType() { return val; }
	virtual void setType(SharedType t) { val = t; }
	virtual SharedExp clone() const override;

	bool operator==(const Exp& o) const override;
	bool operator<(const Exp& o) const override;
	bool operator*=(const Exp& o) const override;
	void print(QTextStream& os, bool = false) const override;
	void printx(int ind) const override;

	/// Should not be constraining constraints
	SharedExp genConstraints(SharedExp /*restrictTo*/) override
	{
		assert(0);
		return nullptr;
	}

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;
};


class Location : public Unary
{
protected:
	friend class XMLProgParser;

protected:
	UserProc *proc;

public:

	/**
	 * Constructor with ID, subexpression, and UserProc*
	 * Create a new Location expression.
	 * \param op Should be opRegOf, opMemOf, opLocal, opGlobal, opParam or opTemp.
	 * \param exp - child expression
	 * \param p - enclosing procedure, if null this constructor will try to find it.
	 */
	Location(OPER op, SharedExp e, UserProc *proc);

	// Copy constructor
	Location(Location& o);

	// Custom constructor
	static SharedExp get(OPER op, SharedExp e, UserProc *proc) { return std::make_shared<Location>(op, e, proc); }
	static SharedExp regOf(int r) { return get(opRegOf, Const::get(r), nullptr); }
	static SharedExp regOf(SharedExp e) { return get(opRegOf, e, nullptr); }
	static SharedExp memOf(SharedExp e, UserProc *p = nullptr) { return get(opMemOf, e, p); }
	static std::shared_ptr<Location> tempOf(SharedExp e) { return std::make_shared<Location>(opTemp, e, nullptr); }
	static SharedExp global(const char *nam, UserProc *p) { return get(opGlobal, Const::get(nam), p); }
	static SharedExp global(const QString& nam, UserProc *p) { return get(opGlobal, Const::get(nam), p); }
	static std::shared_ptr<Location> local(const QString& nam, UserProc *p);

	static SharedExp param(const char *nam, UserProc *p = nullptr) { return get(opParam, Const::get(nam), p); }
	static SharedExp param(const QString& nam, UserProc *p = nullptr) { return get(opParam, Const::get(nam), p); }
	// Clone
	virtual SharedExp clone() const override;

	void setProc(UserProc *p) { proc = p; }
	UserProc *getProc() { return proc; }

	virtual SharedExp polySimplify(bool& bMod) override;
	virtual void getDefinitions(LocationSet& defs);

	// Visitation
	virtual bool accept(ExpVisitor *v) override;
	virtual SharedExp accept(ExpModifier *v) override;
	virtual bool match(const QString& pattern, std::map<QString, SharedConstExp>& bindings) override;

protected:
	Location(OPER _op)
		: Unary(_op)
		, proc(nullptr) {}
};

typedef std::set<SharedExp, lessExpStar> sExp;
