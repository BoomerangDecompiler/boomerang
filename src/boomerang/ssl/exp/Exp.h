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


#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/ssl/exp/Operator.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/OStream.h"

#include <QString>

#include <cassert>
#include <list>
#include <memory>
#include <set>
#include <vector>


class Exp;
class Type;
class ExpVisitor;
class ExpModifier;
class UserProc;
class LocationSet;
class CompoundType;

class OStream;

typedef std::unique_ptr<Exp> UniqExp;
typedef std::shared_ptr<Exp> SharedExp;
typedef std::shared_ptr<const Exp> SharedConstExp;
typedef std::shared_ptr<Type> SharedType;
typedef std::shared_ptr<const Type> SharedConstType;


/**
 * \class Exp
 * An expression class, though it will probably be used to hold many other things (e.g. perhaps
 * transformations). It is a standard tree representation. Exp itself is abstract. A special class
 * Const is used for constants. Unary, Binary, and Ternary hold 1, 2, and 3 subexpressions
 * respectively. For efficiency of representation, these have to be separate classes, derived from
 * Exp.
 *
 * Main class hierarchy:
 *                      Exp (abstract)
 *                    ____/  |     \
 *                   /       |      \
 *                Unary     Const   Terminal
 *   TypedExp____/  |   \
 *    RefExp____/ Binary Location
 *                  |
 *               Ternary
 */
class BOOMERANG_API Exp : public std::enable_shared_from_this<Exp>
{
public:
    Exp(OPER oper);
    Exp(const Exp &other) = default;
    Exp(Exp &&other)      = default;

    virtual ~Exp() = default;

    Exp &operator=(const Exp &) = default;
    Exp &operator=(Exp &&) = default;

public:
    /// Clone (make copy of self that can be deleted without affecting self)
    virtual SharedExp clone() const = 0;

    /// Type sensitive equality
    virtual bool operator==(const Exp &o) const = 0;
    bool operator!=(const Exp &o) { return !(*this == o); }

    /// Type sensitive less than
    virtual bool operator<(const Exp &o) const = 0;

    /// Comparison ignoring subscripts
    virtual bool equalNoSubscript(const Exp &o) const = 0;

public:
    /// Return the operator.
    /// \note I'd like to make this protected, but then subclasses
    /// don't seem to be able to use it (at least, for subexpressions)
    OPER getOper() const { return m_oper; }

    /// A few simplifications use this
    void setOper(OPER oper) { m_oper = oper; }

    /// Return the number of subexpressions. This is only needed in rare cases.
    /// Could use polymorphism for all those cases, but this is easier
    virtual int getArity() const;

    // Checks for broad type of expression operator

    /// \returns true if this is a wildcard expression (used for searching)
    bool isWildcard() const;

    /// \returns true if this is a floating point expression (e.g. a +f b)
    bool isFloatExp() const;

    /// \returns true if this is a logical expression (e.g. comparison, && or ||)
    bool isLogExp() const;

    /// \returns true if this is a comparison
    bool isComparison() const;

    /// \returns true if this is a bitwise operation (e.g. & or |)
    bool isBitwise() const;

    /// \returns True if this is a location
    bool isLocation() const;

    /// True if integer const, float const or string const
    bool isConst() const;

    /// \returns true if the operator is symmetric (i.e. a && b <-> b && a)
    bool isSymmetric() const;

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

    /// True if this is the Nil Terminal (terminates lists; "NOP" expression)
    bool isNil() const { return m_oper == opNil; }
    /// True if this is %pc
    bool isPC() const { return m_oper == opPC; }

    /// True if is int const
    bool isIntConst() const { return m_oper == opIntConst; }
    /// True if is long (address) const
    bool isLongConst() const { return m_oper == opLongConst; }
    /// True if is string const
    bool isStrConst() const { return m_oper == opStrConst; }
    /// True if is flt point const
    bool isFltConst() const { return m_oper == opFltConst; }
    /// True if is function pointer const
    bool isFuncPtrConst() const { return m_oper == opFuncConst; }

    /// True if this is a subscripted expression (SSA)
    bool isSubscript() const { return m_oper == opSubscript; }
    /// True if this is a local variable
    bool isLocal() const { return m_oper == opLocal; }
    /// True if this is a global variable
    bool isGlobal() const { return m_oper == opGlobal; }

    /// \returns true if this is the constant "true"
    bool isTrue() const;
    /// \returns true if this is the constant "false"
    bool isFalse() const;
    /// \returns true if this is a disjunction, i.e. x or y
    bool isOr() const { return m_oper == opOr; }
    /// \returns true if this is a conjunction, i.e. x and y
    bool isAnd() const { return m_oper == opAnd; }
    /// \returns true if this is a boolean constant
    bool isBoolConst() const { return m_oper == opTrue || m_oper == opFalse; }
    /// \returns true if this is an equality comparison using ==
    bool isEquality() const { return m_oper == opEquals; }
    /// \returns true if this is an equality comparison using !=
    bool isNotEquality() const { return m_oper == opNotEqual; }

    /// \returns true if this is a machine feature
    bool isMachFtr() const { return m_oper == opMachFtr; }
    /// \returns true if this is a parameter. Note: opParam has two meanings: a SSL parameter, or a
    /// function parameter
    bool isParam() const { return m_oper == opParam; }

    /// \returns True if this is a typed expression
    bool isTypedExp() const { return m_oper == opTypedExp; }

public:
    // subexpression access / modification

    /**
     * These are here so we can (optionally) prevent code clutter.
     * Using a *Exp (that is known to be a Binary* say), you can just directly call getSubExp2
     * However, you can still choose to cast from Exp* to Binary* etc. and avoid the virtual call
     */
    template<class T>
    inline std::shared_ptr<T> access()
    {
        return shared_from_base<T>();
    }

    template<class T>
    inline std::shared_ptr<const T> access() const
    {
        return shared_from_base<const T>();
    }

    /// Access sub-expressions recursively
    template<class T, int SUB_IDX, int... Path>
    std::shared_ptr<T> access()
    {
        // clang-format off
        switch (SUB_IDX) {
        case 1: return getSubExp1()->access<T, Path...>();
        case 2: return getSubExp2()->access<T, Path...>();
        case 3: return getSubExp3()->access<T, Path...>();
        default: assert(false);
        }
        // clang-format on

        return nullptr;
    }

    template<class T, int SUB_IDX, int... Path>
    std::shared_ptr<const T> access() const
    {
        // clang-format off
        switch (SUB_IDX) {
            case 1: return getSubExp1()->access<T, Path...>();
            case 2: return getSubExp2()->access<T, Path...>();
            case 3: return getSubExp3()->access<T, Path...>();
            default: assert(false);
        }
        // clang-format on

        return nullptr;
    }

    /// \returns  Pointer to the requested subexpression
    virtual SharedExp getSubExp1() { return nullptr; }
    virtual SharedConstExp getSubExp1() const { return nullptr; }
    virtual SharedExp getSubExp2() { return nullptr; }
    virtual SharedConstExp getSubExp2() const { return nullptr; }
    virtual SharedExp getSubExp3() { return nullptr; }
    virtual SharedConstExp getSubExp3() const { return nullptr; }
    virtual SharedExp &refSubExp1();
    virtual SharedExp &refSubExp2();
    virtual SharedExp &refSubExp3();

    /// Update a sub-expression
    virtual void setSubExp1(SharedExp /*e*/) { assert(false); }
    virtual void setSubExp2(SharedExp /*e*/) { assert(false); }
    virtual void setSubExp3(SharedExp /*e*/) { assert(false); }

public:
    /**
     * Search this expression for the given subexpression, and if found,
     * return true and return a pointer to the matched expression in result.
     * Useful when there are wildcards, e.g. search pattern is *r[?] result is r[2].
     *
     * \param   pattern Expression we are searching for
     * \param   result  ref to ptr to Exp that matched. If the expression was not found,
     *                  the value is nullptr.
     * \returns         True if a match was found
     */
    virtual bool search(const Exp &pattern, SharedExp &result);

    /**
     * Search this expression for the given subexpression, and for each found,
     * append the found subexpression to \p results.
     *
     * \param   pattern ptr to Exp we are searching for
     * \param   results ref to list of Exp that matched
     * \returns True if a match was found
     */
    bool searchAll(const Exp &pattern, std::list<SharedExp> &results);

    /**
     * Search for the given subexpression, and replace if found
     * \note    If the top level expression matches, return val != this
     *
     * \param    pattern       reference to Exp we are searching for
     * \param    replacement   ptr to Exp to replace it with
     * \param    change        ref to boolean, set true if a change made (else cleared)
     * \returns  True if a change made
     */
    SharedExp searchReplace(const Exp &pattern, const SharedExp &replacement, bool &change);

    /**
     * Search for the given subexpression, and replace wherever found.
     * \note    If the top level expression matches, something other than "this" will be returned
     * \note    It is possible with wildcards that in very unusual circumstances a replacement will
     * be made to something that is already deleted. \note    Replacements are cloned. Caller to
     * delete search and replace \note    \p change is always assigned. No need to clear beforehand.
     *
     * \param   pattern     reference to Exp we are searching for
     * \param   replacement ptr to Exp to replace it with
     * \param   change  set true if a change made; cleared otherwise
     * \param   once    if set to true only the first possible replacement will be made
     *
     * \returns the result (often this, but possibly changed)
     */
    SharedExp searchReplaceAll(const Exp &pattern, const SharedExp &replacement, bool &change,
                               bool once = false);

    /**
     * Search for the given sub-expression in \p toSearch and all children.
     * \note    Mostly not for public use.
     *
     * \param   pattern  Exp we are searching for
     * \param   toSearch Exp to search for \p pattern.
     * \param   matches  list of Exp** where pointers to the matches are found
     * \param   once     true to return after the first match, false to return all matches in \p
     * toSearch
     */
    static void doSearch(const Exp &pattern, SharedExp &toSearch, std::list<SharedExp *> &matches,
                         bool once);

    /**
     * Search for the given subexpression in all children
     * \param pattern ptr to Exp we are searching for
     * \param matches list of Exp** where pointers to the matches are found
     * \param once    true to return after the first match, false to return all matches in *this
     */
    virtual void doSearchChildren(const Exp &pattern, std::list<SharedExp *> &matches, bool once);

    /// Propagate all possible assignments to components of this expression.
    SharedExp propagateAll();

    /**
     * As above, but keep propagating until no change.
     * Propagate all possible statements to this expression,
     * and repeat until there is no further change.
     */
    SharedExp propagateAllRpt(bool &changed);


    // Get the complexity depth. Basically, add one for each unary, binary, or ternary
    int getComplexityDepth(UserProc *proc);

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
     * \note         Expressions are NOT cloned. Therefore, do not delete the expressions in
     * positives or negatives
     *
     * \param positives the list of positive terms
     * \param negatives the list of negative terms
     * \param integers  the vector of integer terms
     * \param negate    determines whether or not to negate the whole expression, i.e. we are on the
     * RHS of an opMinus
     */
    void partitionTerms(std::list<SharedExp> &positives, std::list<SharedExp> &negatives,
                        std::vector<int> &integers, bool negate);

    /**
     * This method creates an expression that is the sum of all expressions in a list.
     * E.g. given the list <4, r[8], m[14]> the resulting expression is 4+r[8]+m[14].
     *
     * \note         static (non instance) function
     * \note         Exps ARE cloned
     * \param        exprs a list of expressions
     * \returns      a new Exp with the accumulation
     */
    static SharedExp accumulate(std::list<SharedExp> &exprs);

    /**
     * Apply various simplifications such as constant folding.
     * Also canonicalise by putting integer constants on the right hand side of sums,
     * adding of negative constants changed to subtracting positive constants, etc.
     * Changes << k to a multiply.
     *
     * \internal
     * This code is so big, so weird and so lame it's not funny.
     * What this boils down to is the process of unification.
     * We're trying to do it with a simple iterative algorithm, but the algorithm keeps getting more
     * and more complex. Eventually I will replace this with a simple theorem prover and we'll have
     * something powerful, but until then, don't rely on this code to do anything critical. - trent
     * 8/7/2002
     *
     * \returns the simplified expression.
     * \sa ExpSimplifier
     */
    SharedExp simplify();

    /**
     * Just do addressof simplification:
     *     a[ m[ any ]] == any,
     *     m[ a[ any ]] = any,
     * and also
     *     a[ size m[ any ]] == any
     *
     * \returns the simplified expression.
     * \sa ExpAddrSimplifier
     */
    SharedExp simplifyAddr();

    /**
     * This method simplifies an expression consisting of + and - at the top level.
     * For example,
     *     (%sp + 100) - (%sp + 92) will be simplified to 8.
     *
     * \returns the simplified expression.
     * \sa ExpArithSimplifier
     */
    SharedExp simplifyArith();

    /**
     * Replace succ(r[k]) by r[k+1]
     * \note        Could change top level expression
     * \returns     Fixed expression
     */
    SharedExp fixSuccessor(); // succ(r2) -> r3

    /// Do the work of finding used locations. If \p memOnly set, only look inside m[...]
    void addUsedLocs(LocationSet &used, bool memOnly = false);

    /// allZero is set if all subscripts in the whole expression are null or implicit; otherwise
    /// cleared
    SharedExp removeSubscripts(bool &allZero);

    /// Convert from SSA form, where this is not subscripted (but defined at statement d)
    /// Needs the UserProc for the symbol map
    // FIXME: if the wrapped expression does not convert to a location, the result is subscripted,
    // which is probably not what is wanted!
    SharedExp fromSSAleft(UserProc *proc, const SharedStmt &d);

    /// Subscript all e in this Exp with statement def
    /// Subscript any occurrences of e with e{def} in this expression
    SharedExp expSubscriptVar(const SharedExp &e, const SharedStmt &def);

    /// Subscript all e in this Exp with 0 (implicit assignments)
    /// Subscript any occurrences of e with e{-} in this expression
    /// \note subscript with nullptr, not implicit assignments as above
    SharedExp expSubscriptValNull(const SharedExp &e);

    /// Subscript all locations in this expression with their implicit assignments
    SharedExp expSubscriptAllNull();

    /// Perform call bypass and simple (assignment only) propagation to this exp
    /// Note: can change this, so often need to clone before calling
    SharedExp bypass();

    /// Check if this exp contains any flag calls
    bool containsFlags();

    /// Check if this expression contains a bare memof (no subscripts) or one that has no symbol
    /// (i.e. is not a local variable or a parameter)
    bool containsBadMemof(); ///< Check if this Exp contains a bare (non subscripted) memof

    // Data flow based type analysis (implemented in type/dfa.cpp)
    // Pull type information up the expression tree
    virtual SharedType ascendType();

    /// Push type information down the expression tree.
    /// \param newType new type of the expression.
    /// \returns true if any change.
    virtual bool descendType(SharedType newType) = 0;

public:
    /// \returns this expression as a string
    QString toString() const;

    /// Print the expression to the given stream
    void print(OStream &os) const;

public:
    /// Accept an expression visitor to visit this expression.
    /// \returns true to continue visiting parent and sibling expressions.
    virtual bool acceptVisitor(ExpVisitor *v) = 0;

    /// Accept an expression modifier to modify this expression and all subexpressions.
    /// \returns the modified expression.
    SharedExp acceptModifier(ExpModifier *mod);

protected:
    /// Accept an expression modifier to modify this expression before modifying all subexpressions.
    virtual SharedExp acceptPreModifier(ExpModifier *mod, bool &visitChildren) = 0;

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
BOOMERANG_API OStream &operator<<(OStream &os, const SharedConstExp &exp);


// Hard-coded numbers of register indices.

// x86
#define REG_X86_AX RegNum(0)
#define REG_X86_CX RegNum(1)
#define REG_X86_DX RegNum(2)
#define REG_X86_BX RegNum(3)
#define REG_X86_SP RegNum(4)
#define REG_X86_BP RegNum(5)
#define REG_X86_SI RegNum(6)
#define REG_X86_DI RegNum(7)
#define REG_X86_AL RegNum(8)
#define REG_X86_CL RegNum(9)
#define REG_X86_DL RegNum(10)
#define REG_X86_BL RegNum(11)
#define REG_X86_AH RegNum(12)
#define REG_X86_CH RegNum(13)
#define REG_X86_DH RegNum(14)
#define REG_X86_BH RegNum(15)

#define REG_X86_SS RegNum(16)
#define REG_X86_CS RegNum(17)
#define REG_X86_DS RegNum(18)
#define REG_X86_ES RegNum(19)
#define REG_X86_FS RegNum(20)
#define REG_X86_GS RegNum(21)

#define REG_X86_EAX RegNum(24)
#define REG_X86_ECX RegNum(25)
#define REG_X86_EDX RegNum(26)
#define REG_X86_EBX RegNum(27)
#define REG_X86_ESP RegNum(28)
#define REG_X86_EBP RegNum(29)
#define REG_X86_ESI RegNum(30)
#define REG_X86_EDI RegNum(31)

#define REG_X86_ST0 RegNum(32) // FP st register
#define REG_X86_ST1 RegNum(33)
#define REG_X86_ST2 RegNum(34)
#define REG_X86_ST3 RegNum(35)
#define REG_X86_ST4 RegNum(36)
#define REG_X86_ST5 RegNum(37)
#define REG_X86_ST6 RegNum(38)
#define REG_X86_ST7 RegNum(39)
#define REG_X86_FSW RegNum(40)
#define REG_X86_FSTP RegNum(41)
#define REG_X86_FCW RegNum(42)

// PPC
#define REG_PPC_SP RegNum(1)

#define REG_PPC_G0 RegNum(0)
#define REG_PPC_G1 RegNum(1)
#define REG_PPC_G2 RegNum(2)
#define REG_PPC_G3 RegNum(3)
#define REG_PPC_G4 RegNum(4)
#define REG_PPC_G5 RegNum(5)
#define REG_PPC_G6 RegNum(6)
#define REG_PPC_G7 RegNum(7)
#define REG_PPC_G8 RegNum(8)
#define REG_PPC_G9 RegNum(9)
#define REG_PPC_G10 RegNum(10)
#define REG_PPC_G11 RegNum(11)
#define REG_PPC_G12 RegNum(12)
#define REG_PPC_G13 RegNum(13)
#define REG_PPC_G31 RegNum(31)

#define REG_PPC_F0 RegNum(32)

#define REG_PPC_VR0 RegNum(64)

#define REG_PPC_CR0 RegNum(100) // Control register

#define REG_PPC_LR RegNum(300)  // Link register
#define REG_PPC_CTR RegNum(301) // Counter register


// ST20
#define REG_ST20_A RegNum(0)
#define REG_ST20_B RegNum(1)
#define REG_ST20_C RegNum(2)
#define REG_ST20_SP RegNum(3)
