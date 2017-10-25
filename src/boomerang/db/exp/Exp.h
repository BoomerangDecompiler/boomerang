#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


/**
 * \file       exp.h
 * \brief   Provides the definition for the Exp class and its subclasses.
 */

#include <QString>

#include <list>
#include <vector>
#include <set>
#include <cassert>
#include <memory>

#include "boomerang/db/exp/Operator.h"
#include "boomerang/db/exp/ExpHelp.h"
#include "boomerang/util/Util.h"


class Exp;
class Type;
class ExpVisitor;
class ExpModifier;
class UserProc;
class LocationSet;
class Statement;
class CompoundType;


typedef std::unique_ptr<Exp>         UniqExp;
typedef std::shared_ptr<Exp>         SharedExp;
typedef std::shared_ptr<const Exp>   SharedConstExp;
typedef std::shared_ptr<Type>        SharedType;


/**
 * \class Exp
 * An expression class, though it will probably be used to hold many other things (e.g. perhaps transformations).
 * It is a standard tree representation. Exp itself is abstract.
 * A special class Const is used for constants.
 * Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions respectively.
 * For efficiency of representation, these have to be separate classes, derived from Exp.
 *
 * Main class hierarchy:    Exp (abstract)
 *                    _____/ | \
 *                   /         |    \
 *                Unary       Const Terminal
 *   TypedExp____/  |     \          \
 *    FlagDef___/ Binary Location  TypeVal
 *     RefExp__/    |
 *               Ternary
 */
class Exp : public Printable, public std::enable_shared_from_this<Exp>
{
protected:
    // Constructor, with ID
    Exp(OPER op)
        : m_oper(op) {}

public:
    virtual ~Exp() override = default;

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

    /**
     * \brief  Print an infix representation of the object to the given file stream,
     *         with its type in \<angle brackets\>.
     * \param os Output stream to send the output to
     */
    void printt(QTextStream& os) const;

    /**
     * \brief        Print an infix representation of the object to the given file stream, but convert r[10] to r10 and
     *               v[5] to v5
     * \note   Never modify this function to emit debugging info; the back ends rely on this being clean to emit
     *         correct C.  If debugging is desired, use operator<<
     * \param os Output stream to send the output to
     */
    void printAsHL(QTextStream& os); ///< Print with v[5] as v5

    /**
     * \brief        Print to a static string (for debugging)
     * \returns            Address of the static buffer
     */
    char *prints();                  ///< Print to string (for debugging and logging)
    void dump();                     ///< Print to standard error (for debugging)

    /// Recursive print: don't want parens at the top level
    virtual void printr(QTextStream& os, bool html = false) const { print(os, html); }

    // But most classes want standard
    // For debugging: print in indented hex. In gdb: "p x->printx(0)"
    virtual void printx(int ind) const = 0;

    /// Display as a dotty graph

    /**
     * \brief Create a dotty file (use dotty to display the file; search the web for "graphviz").
     *        Mainly for debugging
     * \param name - Name of the file to create
     */
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

    /**
     * \brief        Returns true if the expression is r[K] where K is int const
     * \returns      True if matches
     */
    bool isRegOfK();

    /**
     * \brief        Returns true if the expression is r[N] where N is the given int const
     * \param n      the specific register to be tested for
     * \returns      True if this is a specific numeric register
     */
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

    /**
     * \brief        Returns true if is %afp, %afp+k, %afp-k, or a[m[<any of these]]
     * \returns            True if found
     */
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

    /**
     * \brief        Returns the index for this var, e.g. if v[2], return 2
     * \returns      The index
     */
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

    /**
     * \brief  Search this expression for the given subexpression, and if found, return true and return a pointer
     *         to the matched expression in result
     *         useful when there are wildcards, e.g. search pattern is *r[?] result is r[2].
     * \param   search     ptr to Exp we are searching for
     * \param   result     ref to ptr to Exp that matched
     * \returns            True if a match was found
     */
    virtual bool search(const Exp& search, SharedExp& result);

    // Search for Exp search in this Exp. For each found, add a ptr to the matching expression in result (useful
    // with wildcards).      Does NOT clear result on entry

    /**
     * \brief        Search this expression for the given subexpression, and for each found, return a pointer to the
     *                      matched expression in result
     * \param   search     ptr to Exp we are searching for
     * \param   result  ref to list of Exp that matched
     * \returns            True if a match was found
     */
    bool searchAll(const Exp& search, std::list<SharedExp>& result);

    /// Search this Exp for *search; if found, replace with *replace

    /**
     * \brief   Search for the given subexpression, and replace if found
     * \note    If the top level expression matches, return val != this
     *
     * \param       search - reference to Exp we are searching for
     * \param       replace - ptr to Exp to replace it with
     * \param       change - ref to boolean, set true if a change made (else cleared)
     * \returns     True if a change made
     */
    SharedExp searchReplace(const Exp& search, const SharedExp& replace, bool& change);

    /**
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
     */
    SharedExp searchReplaceAll(const Exp& search, const SharedExp& replace, bool& change, bool once = false);

    /**
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
     */
    static void doSearch(const Exp& search, SharedExp& pSrc, std::list<SharedExp *>& li, bool once);

    /**
     * \brief       Search for the given subexpression in all children
     * \note        Virtual function; different implementation for each subclass of Exp
     * \note            Will recurse via doSearch
     *
     * \param       search - ptr to Exp we are searching for
     * \param       li - list of Exp** where pointers to the matches are found
     * \param       once - true if not all occurrences to be found, false for all
     */
    virtual void doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once);

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
    std::shared_ptr<T> access() { return shared_from_base<T>(); }

    template<class T>
    std::shared_ptr<const T> access() const { return const_cast<Exp *>(this)->access<T>(); }

    /// Access sub-expressions recursively
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

        return nullptr;
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

        return nullptr;
    }

    /**
     * \brief    Get subexpression
     * \returns  Pointer to the requested subexpression
     */
    virtual SharedExp getSubExp1() { return nullptr; }
    virtual SharedConstExp getSubExp1() const { return nullptr; }
    virtual SharedExp getSubExp2() { return nullptr; }
    virtual SharedConstExp getSubExp2() const { return nullptr; }
    virtual SharedExp getSubExp3() { return nullptr; }
    virtual SharedConstExp getSubExp3() const { return nullptr; }
    virtual SharedExp& refSubExp1();
    virtual SharedExp& refSubExp2();
    virtual SharedExp& refSubExp3();

    /**
     * \brief  Set requested subexpression; 1 is first
     * param  e Pointer to subexpression to set
     * \note   If an expression already exists, it is ;//deleted
     */
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

    /**
     * \brief        Returns a ptr to the guard expression, or 0 if none
     * \returns            Ptr to the guard, or 0
     */
    SharedExp getGuard(); // Get the guard expression, or 0 if not

    //    //    //    //    //    //    //    //    //
    //    Expression Simplification    //
    //    //    //    //    //    //    //    //    //


    // These simplifying functions don't really belong in class Exp, but they know too much about how Exps work
    // They can't go into util.so, since then util.so and db.so would co-depend on each other for testing at least

    /**
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
     */
    void partitionTerms(std::list<SharedExp>& positives, std::list<SharedExp>& negatives, std::vector<int>& integers,
                        bool negate);

    /**
     * \brief        This method simplifies an expression consisting of + and - at the top level. For example,
     *               (%sp + 100) - (%sp + 92) will be simplified to 8.
     * \note         Any expression can be so simplified
     * \note         User must ;//delete result
     * \returns      Ptr to the simplified expression
     */
    virtual SharedExp simplifyArith() { return shared_from_this(); }

    /**
     * \brief        This method creates an expression that is the sum of all expressions in a list.
     *               E.g. given the list <4,r[8],m[14]> the resulting expression is 4+r[8]+m[14].
     * \note         static (non instance) function
     * \note         Exps ARE cloned
     * \param        exprs - a list of expressions
     * \returns      a new Exp with the accumulation
     */
    static SharedExp accumulate(std::list<SharedExp>& exprs);

    /**
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
     */
    SharedExp simplify();

    virtual SharedExp polySimplify(bool& bMod)
    {
        bMod = false;
        return shared_from_this();
    }

    // Just the address simplification a[ m[ any ]]
    virtual SharedExp simplifyAddr() { return shared_from_this(); }
    virtual SharedExp simplifyConstraint() { return shared_from_this(); }

    /**
     * \brief       Replace succ(r[k]) by r[k+1]
     * \note        Could change top level expression
     * \returns     Fixed expression
     */
    SharedExp fixSuccessor(); // succ(r2) -> r3

    /// Kill any zero fill, sign extend, or truncates

    /**
     * \brief        Remove size operations such as zero fill, sign extend
     * \note         Could change top level expression
     * \note         Does not handle truncation at present
     * \returns      Fixed expression
     */
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
    SharedExp fromSSAleft(UserProc *proc, Statement *d);

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
    SharedExp expSubscriptVar(const SharedExp& e, Statement *def /*, Cfg* cfg */);

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
    virtual void descendType(SharedType /*parentType*/, bool& /*ch*/, Statement * /*s*/) { assert(0); }


    static SharedExp convertFromOffsetToCompound(SharedExp parent, std::shared_ptr<CompoundType>& c, unsigned n);

protected:
    template<typename CHILD>
    std::shared_ptr<CHILD> shared_from_base()
    {
        return std::static_pointer_cast<CHILD>(shared_from_this());
    }

protected:
    OPER m_oper; ///< The operator (e.g. opPlus)
    mutable unsigned int m_lexBegin = 0, m_lexEnd = 0;
};

// Not part of the Exp class, but logically belongs with it:

/**
 * \brief Output operator for Exp*
 * Prints the Exp pointed to by p
 * \param os output stream to send to
 * \param p ptr to Exp to print to the stream
 * \returns copy of os (for concatenation)
 */
QTextStream& operator<<(QTextStream& os, const Exp *p);


inline QTextStream& operator<<(QTextStream& os, const SharedConstExp& p)
{
    os << p.get();
    return os;
}


typedef std::set<SharedExp, lessExpStar> ExpSet;
