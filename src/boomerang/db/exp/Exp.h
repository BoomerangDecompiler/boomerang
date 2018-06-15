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
typedef std::shared_ptr<const Type>   SharedConstType;


/**
 * \class Exp
 * An expression class, though it will probably be used to hold many other things (e.g. perhaps transformations).
 * It is a standard tree representation. Exp itself is abstract.
 * A special class Const is used for constants.
 * Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions respectively.
 * For efficiency of representation, these have to be separate classes, derived from Exp.
 *
 * Main class hierarchy:
 *                      Exp (abstract)
 *                    ____/  |     \
 *                   /       |      \
 *                Unary     Const   Terminal
 *   TypedExp____/  |   \
 *    FlagDef___/ Binary Location
 *     RefExp__/    |
 *               Ternary
 */
class Exp : public Printable, public std::enable_shared_from_this<Exp>
{
public:
    Exp(OPER oper) : m_oper(oper) {}

    Exp(const Exp& other) = default;
    Exp(Exp&& other) = default;

    virtual ~Exp() override = default;

    Exp& operator=(const Exp&) = default;
    Exp& operator=(Exp&&) = default;

public:
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

    /// Return the operator.
    /// \note I'd like to make this protected, but then subclasses
    /// don't seem to be able to use it (at least, for subexpressions)
    OPER getOper() const { return m_oper; }
    const char *getOperName() const;

    /// A few simplifications use this
    void setOper(OPER x) { m_oper = x; }

    /// \returns this expression as a string
    QString toString() const override;

    /// Print the expression to the given stream
    virtual void print(QTextStream& os, bool html = false) const = 0;

    /// Recursive print: don't want parens at the top level
    virtual void printr(QTextStream& os, bool html = false) const { print(os, html); }

    /// Print an infix representation of the object to \p os,
    /// with its type in \<angle brackets\>.
    void printt(QTextStream& os) const;

    /// Print to a static buffer (for debugging)
    char *prints();

    /// For debugging: print in indented hex. In gdb: "p x->printx(0)"
    virtual void printx(int ind) const = 0;

    /// Print to standard error (for debugging)
    void dump();

    /// Return the number of subexpressions. This is only needed in rare cases.
    /// Could use polymorphism for all those cases, but this is easier
    virtual int getArity() const { return 0; } // Overridden for Unary, Binary, etc

    /// True if this is a call to a flag function
    bool isFlagCall() const { return m_oper == opFlagCall; }
    /// True if this represents one of the abstract flags locations, int or float
    bool isFlags() const { return m_oper == opFlags || m_oper == opFflags; }
    /// True if is one of the main 4 flags
    bool isMainFlag() const { return m_oper >= opZF && m_oper <= opOF; }
    /// True if this is a register location
    bool isRegOf() const { return m_oper == opRegOf; }

    /// \returns true if the expression is r[K] where K is an integer Const
    bool isRegOfConst() const;

    /// \returns true if the expression is r[n] with register number \p n
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
    bool isPC() const { return m_oper == opPC; }

    /// Returns true if is %afp, %afp+k, %afp-k, or a[m[...]]
    bool isAfpTerm();

    /// True if is int const
    bool isIntConst() const { return m_oper == opIntConst; }
    /// True if is string const
    bool isStrConst() const { return m_oper == opStrConst; }
    /// Get string constant even if mangled
    QString getAnyStrConst();

    /// True if is flt point const
    bool isFltConst() const { return m_oper == opFltConst; }
    /// True if integer const, float const or string const
    bool isConst() const { return m_oper == opIntConst || m_oper == opFltConst || m_oper == opStrConst; }
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

    /// \returns the index for this var, e.g. if this is v[2], return 2
    /// \note this must be opVar!
    int getVarIndex();

    /// \returns true if this is a terminal
    virtual bool isTerminal() const { return false; }
    /// \returns true if this is the constant "true"
    bool isTrue() const { return m_oper == opTrue; }
    /// \returns true if this is the constant "false"
    bool isFalse() const { return m_oper == opFalse; }
    /// \returns true if this is a disjunction, i.e. x or y
    bool isDisjunction() const { return m_oper == opOr; }
    /// \returns true if this is a conjunction, i.e. x and y
    bool isConjunction() const { return m_oper == opAnd; }
    /// \returns true if this is a boolean constant
    bool isBoolConst() const { return m_oper == opTrue || m_oper == opFalse; }
    /// \returns true if this is an equality (== or !=)
    bool isEquality() const { return m_oper == opEquals /*|| op == opNotEqual*/; }

    /// \returns true if this is a comparison
    bool isComparison() const
    {
        return m_oper == opEquals   || m_oper == opNotEqual
            || m_oper == opGtr      || m_oper == opLess
            || m_oper == opGtrUns   || m_oper == opLessUns
            || m_oper == opGtrEq    || m_oper == opLessEq
            || m_oper == opGtrEqUns || m_oper == opLessEqUns;
    }

    /// \returns true if this is a machine feature
    bool isMachFtr() const { return m_oper == opMachFtr; }
    /// \returns true if this is a parameter. Note: opParam has two meanings: a SSL parameter, or a function parameter
    bool isParam() const { return m_oper == opParam; }

    /// \returns True if this is a location
    bool isLocation() const { return m_oper == opMemOf || m_oper == opRegOf || m_oper == opGlobal || m_oper == opLocal || m_oper == opParam; }
    /// \returns True if this is a typed expression
    bool isTypedExp() const { return m_oper == opTypedExp; }

    /**
     * Search this expression for the given subexpression, and if found, return true and return a pointer
     * to the matched expression in result
     * useful when there are wildcards, e.g. search pattern is *r[?] result is r[2].
     * \param   pattern ptr to Exp we are searching for
     * \param   result  ref to ptr to Exp that matched
     * \returns         True if a match was found
     */
    virtual bool search(const Exp& pattern, SharedExp& result);

    /**
     * Search this expression for the given subexpression, and for each found,
     * append the found subexpression to \p results.
     *
     * \param   pattern ptr to Exp we are searching for
     * \param   results ref to list of Exp that matched
     * \returns True if a match was found
     */
    bool searchAll(const Exp& pattern, std::list<SharedExp>& results);

    /**
     * Search for the given subexpression, and replace if found
     * \note    If the top level expression matches, return val != this
     *
     * \param    pattern       reference to Exp we are searching for
     * \param    replacement   ptr to Exp to replace it with
     * \param    change        ref to boolean, set true if a change made (else cleared)
     * \returns  True if a change made
     */
    SharedExp searchReplace(const Exp& pattern, const SharedExp& replacement, bool& change);

    /**
     * Search for the given subexpression, and replace wherever found.
     * \note    If the top level expression matches, something other than "this" will be returned
     * \note    It is possible with wildcards that in very unusual circumstances a replacement will be made to
     *          something that is already deleted.
     * \note    Replacements are cloned. Caller to delete search and replace
     * \note    \p change is always assigned. No need to clear beforehand.
     *
     * \param   pattern     reference to Exp we are searching for
     * \param   replacement ptr to Exp to replace it with
     * \param   change  set true if a change made; cleared otherwise
     * \param   once    if set to true only the first possible replacement will be made
     *
     * \returns the result (often this, but possibly changed)
     */
    SharedExp searchReplaceAll(const Exp& pattern, const SharedExp& replacement, bool& change, bool once = false);

    /**
     * Search for the given sub-expression in \p toSearch and all children.
     * \note    Mostly not for public use.
     *
     * \param   pattern  Exp we are searching for
     * \param   toSearch Exp to search for \p pattern.
     * \param   matches  list of Exp** where pointers to the matches are found
     * \param   once     true to return after the first match, false to return all matches in \p toSearch
     */
    static void doSearch(const Exp& pattern, SharedExp& toSearch, std::list<SharedExp *>& matches, bool once);

    /**
     * Search for the given subexpression in all children
     * \param pattern ptr to Exp we are searching for
     * \param matches list of Exp** where pointers to the matches are found
     * \param once    true to return after the first match, false to return all matches in *this
     */
    virtual void doSearchChildren(const Exp& pattern, std::list<SharedExp *>& matches, bool once);

    /// Propagate all possible assignments to components of this expression.
    SharedExp propagateAll();

    /**
     * As above, but keep propagating until no change.
     * Propagate all possible statements to this expression,
     * and repeat until there is no further change.
     */
    SharedExp propagateAllRpt(bool& changed);


    /**
     * These are here so we can (optionally) prevent code clutter.
     * Using a *Exp (that is known to be a Binary* say), you can just directly call getSubExp2
     * However, you can still choose to cast from Exp* to Binary* etc. and avoid the virtual call
     */
    template<class T>
    inline std::shared_ptr<T> access() { return shared_from_base<T>(); }

    template<class T>
    inline std::shared_ptr<const T> access() const { return shared_from_base<const T>(); }

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

    /// \returns  Pointer to the requested subexpression
    virtual SharedExp getSubExp1() { return nullptr; }
    virtual SharedConstExp getSubExp1() const { return nullptr; }
    virtual SharedExp getSubExp2() { return nullptr; }
    virtual SharedConstExp getSubExp2() const { return nullptr; }
    virtual SharedExp getSubExp3() { return nullptr; }
    virtual SharedConstExp getSubExp3() const { return nullptr; }
    virtual SharedExp& refSubExp1();
    virtual SharedExp& refSubExp2();
    virtual SharedExp& refSubExp3();

    /// Update a sub-expression
    virtual void setSubExp1(SharedExp /*e*/) { assert(false); }
    virtual void setSubExp2(SharedExp /*e*/) { assert(false); }
    virtual void setSubExp3(SharedExp /*e*/) { assert(false); }

    // Get the complexity depth. Basically, add one for each unary, binary, or ternary
    int getComplexityDepth(UserProc *proc);

    // Get memory depth. Add one for each m[]
    int getMemDepth();

    /// \returns a ptr to the guard expression, or 0 if none
    SharedExp getGuard();


    // These simplifying functions don't really belong in class Exp,
    // but they know too much about how Exps work

    /**
     * Takes an expression consisting on only + and - operators and partitions
     * its terms into positive non-integer fixed terms, negative non-integer
     * fixed terms and integer terms. For example, given:
     *     %sp + 108 + n - %sp - 92
     * the resulting partition will be:
     *     positives = { %sp, n }
     *     negatives = { %sp }
     *     integers  = { 108, -92 }
     *
     * \note         integers is a vector so we can use the accumulate func
     * \note         Expressions are NOT cloned. Therefore, do not delete the expressions in positives or negatives
     *
     * \param positives the list of positive terms
     * \param negatives the list of negative terms
     * \param integers  the vector of integer terms
     * \param negate    determines whether or not to negate the whole expression, i.e. we are on the RHS of an opMinus
     */
    void partitionTerms(std::list<SharedExp>& positives, std::list<SharedExp>& negatives, std::vector<int>& integers,
                        bool negate);

    /**
     * This method simplifies an expression consisting of + and - at the top level.
     * For example,
     *     (%sp + 100) - (%sp + 92) will be simplified to 8.
     *
     * \note         Any expression can be so simplified
     * \note         Overridden in subclasses
     * \returns      Ptr to the simplified expression
     */
    virtual SharedExp simplifyArith() { return shared_from_this(); }

    /**
     * This method creates an expression that is the sum of all expressions in a list.
     * E.g. given the list <4, r[8], m[14]> the resulting expression is 4+r[8]+m[14].
     *
     * \note         static (non instance) function
     * \note         Exps ARE cloned
     * \param        exprs a list of expressions
     * \returns      a new Exp with the accumulation
     */
    static SharedExp accumulate(std::list<SharedExp>& exprs);

    /**
     * Apply various simplifications such as constant folding.
     * Also canonicalise by putting integer constants on the right hand side of sums,
     * adding of negative constants changed to subtracting positive constants, etc.
     * Changes << k to a multiply
     *
     * \note         Address simplification (a[ m[ x ]] == x) is done separately
     * \returns      Ptr to the simplified expression
     *
     * \internal
     * This code is so big, so weird and so lame it's not funny.
     * What this boils down to is the process of unification.
     * We're trying to do it with a simple iterative algorithm, but the algorithm keeps getting more and more complex.
     * Eventually I will replace this with a simple theorem prover and we'll have something powerful, but until then,
     * don't rely on this code to do anything critical. - trent 8/7/2002
     */
    SharedExp simplify();

    /**
     * Do the work of simplification
     * \note         Address simplification (a[ m[ x ]] == x) is done separately
     * \returns      Ptr to the simplified expression
     */
    virtual SharedExp polySimplify(bool& changed)
    {
        changed = false;
        return shared_from_this();
    }

    /**
     * Just do addressof simplification:
     *     a[ m[ any ]] == any,
     *     m[ a[ any ]] = any,
     * and also
     *     a[ size m[ any ]] == any
     *
     * \todo         Replace with a visitor some day
     * \returns      Ptr to the simplified expression
     */
    virtual SharedExp simplifyAddr() { return shared_from_this(); }

    /**
     * Replace succ(r[k]) by r[k+1]
     * \note        Could change top level expression
     * \returns     Fixed expression
     */
    SharedExp fixSuccessor(); // succ(r2) -> r3

    /**
     * Remove size operations such as zero fill, sign extend
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

    /// Convert from SSA form, where this is not subscripted (but defined at statement d)
    /// Needs the UserProc for the symbol map
    // FIXME: if the wrapped expression does not convert to a location, the result is subscripted, which is probably not
    // what is wanted!
    SharedExp fromSSAleft(UserProc *proc, Statement *d);

    /// Set or clear the constant subscripts
    void setConscripts(int n, bool clear);

    /// Strip size casts from an Exp
    SharedExp stripSizes();

    /// Subscript all e in this Exp with statement def
    /// Subscript any occurrences of e with e{def} in this expression
    SharedExp expSubscriptVar(const SharedExp& e, Statement *def);

    /// Subscript all e in this Exp with 0 (implicit assignments)
    /// Subscript any occurrences of e with e{-} in this expression
    /// \note subscript with nullptr, not implicit assignments as above
    SharedExp expSubscriptValNull(const SharedExp& e);

    /// Subscript all locations in this expression with their implicit assignments
    SharedExp expSubscriptAllNull();

    /// Perform call bypass and simple (assignment only) propagation to this exp
    /// Note: can change this, so often need to clone before calling
    SharedExp bypass();

    /// Check if this exp contains any flag calls
    bool containsFlags();

    /// Check if this expression contains a bare memof (no subscripts) or one that has no symbol (i.e. is not a local
    /// variable or a parameter)
    bool containsBadMemof(); ///< Check if this Exp contains a bare (non subscripted) memof

    // Data flow based type analysis (implemented in type/dfa.cpp)
    // Pull type information up the expression tree
    virtual SharedType ascendType();

    /// Push type information down the expression tree
    virtual void descendType(SharedType /*parentType*/, bool& /*ch*/, Statement * /*s*/);

    static SharedExp convertFromOffsetToCompound(SharedExp parent, std::shared_ptr<CompoundType>& c, unsigned n);


public:
    /// Accept an expression visitor to visit this expression.
    /// \returns true to continue visiting parent and sibling expressions.
    virtual bool acceptVisitor(ExpVisitor *v)         = 0;

    /// Accept an expression modifier to modify this expression and all subexpressions.
    /// \returns the modified expression.
    SharedExp acceptModifier(ExpModifier *mod);

protected:
    /// Accept an expression modifier to modify this expression before modifying all subexpressions.
    virtual SharedExp acceptPreModifier(ExpModifier *mod, bool& visitChildren) = 0;

    /// Accept an expression modifier to modify all subexpressions (children) of this expression.
    virtual SharedExp acceptChildModifier(ExpModifier *) { return shared_from_this(); }

    /// Accept an exppression modifier to modify this expression after modifying all subexpressions.
    virtual SharedExp acceptPostModifier(ExpModifier *mod) = 0;

protected:
    template<typename CHILD>
    std::shared_ptr<CHILD> shared_from_base()
    {
        return std::static_pointer_cast<CHILD>(shared_from_this());
    }

    template<typename CHILD>
    std::shared_ptr<const CHILD> shared_from_base() const
    {
        return std::static_pointer_cast<CHILD>(shared_from_this());
    }

protected:
    OPER m_oper; ///< The operator (e.g. opPlus)
};


/// Prints the Exp pointed to by \p p to \p os
QTextStream& operator<<(QTextStream& os, const Exp *p);


inline QTextStream& operator<<(QTextStream& os, const SharedConstExp& p)
{
    os << p.get();
    return os;
}


// Hard-coded numbers of register indices.

// pentium
#define PENT_REG_AX      (0)
#define PENT_REG_CX      (1)
#define PENT_REG_DX      (2)
#define PENT_REG_BX      (3)
#define PENT_REG_SP      (4)
#define PENT_REG_BP      (5)
#define PENT_REG_SI      (6)
#define PENT_REG_DI      (7)
#define PENT_REG_AL      (8)
#define PENT_REG_CL      (9)
#define PENT_REG_DL     (10)
#define PENT_REG_BL     (11)
#define PENT_REG_AH     (12)
#define PENT_REG_CH     (13)
#define PENT_REG_DH     (14)
#define PENT_REG_BH     (15)
#define PENT_REG_ES     (16)
#define PENT_REG_CS     (17)
#define PENT_REG_SS     (18)
#define PENT_REG_DS     (19)

#define PENT_REG_EAX    (24)
#define PENT_REG_ECX    (25)
#define PENT_REG_EDX    (26)
#define PENT_REG_EBX    (27)
#define PENT_REG_ESP    (28)
#define PENT_REG_EBP    (29)
#define PENT_REG_ESI    (30)
#define PENT_REG_EDI    (31)

#define PENT_REG_ST0    (32) // FP st register
#define PENT_REG_ST1    (33)
#define PENT_REG_ST2    (34)
#define PENT_REG_ST3    (35)
#define PENT_REG_ST4    (36)
#define PENT_REG_ST5    (37)
#define PENT_REG_ST6    (38)
#define PENT_REG_ST7    (39)
#define PENT_REG_FSW    (40)
#define PENT_REG_FSTP   (41)
#define PENT_REG_FCW    (42)


// sparc
#define SPARC_REG_G0  (0)
#define SPARC_REG_G1  (1)
#define SPARC_REG_G2  (2)
#define SPARC_REG_G3  (3)
#define SPARC_REG_G4  (4)
#define SPARC_REG_G5  (5)
#define SPARC_REG_G6  (6)
#define SPARC_REG_G7  (7)
#define SPARC_REG_O0  (8)
#define SPARC_REG_O1  (9)
#define SPARC_REG_O2 (10)
#define SPARC_REG_O3 (11)
#define SPARC_REG_O4 (12)
#define SPARC_REG_O5 (13)
#define SPARC_REG_O6 (14)
#define SPARC_REG_O7 (15)
#define SPARC_REG_L0 (16)
#define SPARC_REG_L1 (17)
#define SPARC_REG_L2 (18)
#define SPARC_REG_L3 (19)
#define SPARC_REG_L4 (20)
#define SPARC_REG_L5 (21)
#define SPARC_REG_L6 (22)
#define SPARC_REG_L7 (23)
#define SPARC_REG_I0 (24)
#define SPARC_REG_I1 (25)
#define SPARC_REG_I2 (26)
#define SPARC_REG_I3 (27)
#define SPARC_REG_I4 (28)
#define SPARC_REG_I5 (29)
#define SPARC_REG_I6 (30)
#define SPARC_REG_I7 (31)


#define SPARC_REG_SP (14) // stack pointer
#define SPARC_REG_FP (30) // frame pointer

#define SPARC_REG_F0    (32)
#define SPARC_REG_F31   (63)
#define SPARC_REG_F0TO1 (64)
#define SPARC_REG_F28TO31 (87)

// mips
#define MIPS_REG_ZERO   (0)
#define MIPS_REG_AT     (1)
#define MIPS_REG_V0     (2)
#define MIPS_REG_V1     (3)

#define MIPS_REG_T0     (8)

#define MIPS_REG_SP     (29)
#define MIPS_REG_FP     (30)
#define MIPS_REG_RA     (31)
#define MIPS_REG_F0     (32)


// ppc
#define PPC_REG_SP      (1)

#define PPC_REG_G0      (0)
#define PPC_REG_G1      (1)
#define PPC_REG_G2      (2)
#define PPC_REG_G3      (3)
#define PPC_REG_G4      (4)
#define PPC_REG_G5      (5)
#define PPC_REG_G6      (6)
#define PPC_REG_G7      (7)
#define PPC_REG_G8      (8)
#define PPC_REG_G9      (9)
#define PPC_REG_G10     (10)
#define PPC_REG_G11     (11)
#define PPC_REG_G12     (12)
#define PPC_REG_G13     (13)
#define PPC_REG_G31     (31)
#define PPC_REG_CR0     (64) // Control register


// ST20
#define ST20_REG_A      (0)
#define ST20_REG_B      (1)
#define ST20_REG_C      (2)
#define ST20_REG_SP     (3)
