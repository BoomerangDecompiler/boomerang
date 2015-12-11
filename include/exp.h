/*
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 */
/***************************************************************************/ /**
  * \file       exp.h
  * \brief   Provides the definition for the Exp class and its subclasses.
  ******************************************************************************/

#pragma once

/* Main class hierarchy:    Exp (abstract)
                      _____/ | \
                     /         |    \
                  Unary       Const Terminal
     TypedExp____/    |    \          \
      FlagDef___/ Binary Location  TypeVal
       RefExp__/    |
                 Ternary
*/
#include "operator.h" // Declares the OPER enum
#include "types.h"    // For ADDRESS, etc
#include "type.h"     // The Type class for typed expressions
#include "util.h"
//#include "statement.h"    // For StmtSet etc
#include "exphelp.h"
//#include "memo.h"

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
#define DEBUG_BUFSIZE 5000 // Size of the debug print buffer

typedef std::unique_ptr<Exp> UniqExp;
typedef std::shared_ptr<Exp> SharedExp;

typedef std::shared_ptr<RTL> SharedRTL;

/**
  * \class Exp
  * An expression class, though it will probably be used to hold many other things (e.g. perhaps transformations).
  * It is a standard tree representation. Exp itself is abstract. A special class Const is used for constants. Unary,
  * Binary, and Ternary hold 1, 2, and 3 subexpressions respectively. For efficiency of representation, these have to be
  * separate classes, derived from Exp.
  */

//! class Exp is abstract. However, the constructor can be called from the constructors of derived classes, and virtual
//! functions not overridden by derived classes can be called
class Exp : public Printable {
  protected:
    OPER op; // The operator (e.g. opPlus)
    mutable unsigned lexBegin = 0, lexEnd = 0;
    // Constructor, with ID
    constexpr Exp(OPER _op) : op(_op) {}

  public:
    // Virtual destructor
    virtual ~Exp() {}

    //! Return the operator. Note: I'd like to make this protected, but then subclasses don't seem to be able to use
    //! it (at least, for subexpressions)
    OPER getOper() const { return op; }
    const char *getOperName() const;
    void setOper(OPER x) { op = x; } // A few simplifications use this

    void setLexBegin(unsigned int n) const { lexBegin = n; }
    void setLexEnd(unsigned int n) const { lexEnd = n; }
    unsigned getLexBegin() const { return lexBegin; }
    unsigned getLexEnd() const { return lexEnd; }
    QString toString() const override;
    //! Print the expression to the given stream
    virtual void print(QTextStream &os, bool html = false) const = 0;
    void printt(QTextStream &os) const;
    void printAsHL(QTextStream &os); //!< Print with v[5] as v5
    char *prints();                               // Print to string (for debugging and logging)
    void dump();                                  // Print to standard error (for debugging)
    //! Recursive print: don't want parens at the top level
    virtual void printr(QTextStream &os, bool html = false) const { print(os, html); }
    // But most classes want standard
    // For debugging: print in indented hex. In gdb: "p x->printx(0)"
    virtual void printx(int ind) const = 0;

    //! Display as a dotty graph
    void createDotFile(const char *name);
    virtual void appendDotFile(QTextStream &os) = 0;

    //! Clone (make copy of self that can be deleted without affecting self)
    virtual Exp *clone() const = 0;

    // Comparison
    //! Type sensitive equality
    virtual bool operator==(const Exp &o) const = 0;
    //! Type sensitive less than
    virtual bool operator<(const Exp &o) const = 0;
    //! Type insensitive less than. Class TypedExp overrides
    virtual bool operator<<(const Exp &o) const { return (*this < o); }
    //! Comparison ignoring subscripts
    virtual bool operator*=(Exp &o) = 0;

    //! Return the number of subexpressions. This is only needed in rare cases.
    //! Could use polymorphism for all those cases, but this is easier
    virtual int getArity() const { return 0; } // Overridden for Unary, Binary, etc

    //    //    //    //    //    //    //
    //     Enquiry functions    //
    //    //    //    //    //    //    //

    //! True if this is a call to a flag function
    bool isFlagCall() const { return op == opFlagCall; }
    //! True if this represents one of the abstract flags locations, int or float
    bool isFlags() const { return op == opFlags || op == opFflags; }
    //! True if is one of the main 4 flags
    bool isMainFlag() const { return op >= opZF && op <= opOF; }
    //! True if this is a register location
    bool isRegOf() const { return op == opRegOf; }
    //! True if this is a register location with a constant index
    bool isRegOfK();
    //! True if this is a specific numeric register
    bool isRegN(int n) const;
    //! True if this is a memory location (any memory nesting depth)
    bool isMemOf() const { return op == opMemOf; }
    //! True if this is an address of
    bool isAddrOf() { return op == opAddrOf; }
    //! True if this is an array expression
    bool isArrayIndex() { return op == opArrayIndex; }
    //! True if this is a struct member access
    bool isMemberOf() { return op == opMemberAccess; }
    //! True if this is a temporary. Note some old code still has r[tmp]
    bool isTemp();
    //! True if this is the anull Terminal (anulls next instruction)
    bool isAnull() { return op == opAnull; }
    //! True if this is the Nil Terminal (terminates lists; "NOP" expression)
    bool isNil() { return op == opNil; }
    //! True if this is %pc
    bool isPC() { return op == opPC; }
    //! True if is %afp, %afp+k, %afp-k, or a[m[<any of these]]
    bool isAfpTerm();
    //! True if is int const
    bool isIntConst() const { return op == opIntConst; }
    //! True if is string const
    bool isStrConst() const { return op == opStrConst; }
    //! Get string constant even if mangled
    QString getAnyStrConst();
    //! True if is flt point const
    bool isFltConst() const { return op == opFltConst; }
    //! True if inteter or string constant
    bool isConst() const { return op == opIntConst || op == opStrConst; }
    //! True if is a post-var expression (var_op' in SSL file)
    bool isPostVar() const { return op == opPostVar; }
    //! True if this is an opSize (size case; deprecated)
    bool isSizeCast() const { return op == opSize; }
    //! True if this is a subscripted expression (SSA)
    bool isSubscript() const { return op == opSubscript; }
    // True if this is a phi assignmnet (SSA)
    //        bool        isPhi() {return op == opPhi;}
    //! True if this is a local variable
    bool isLocal() const { return op == opLocal; }
    //! True if this is a global variable
    bool isGlobal() const { return op == opGlobal; }
    //! True if this is a typeof
    bool isTypeOf() const { return op == opTypeOf; }
    //! Get the index for this var
    int getVarIndex();
    //! True if this is a terminal
    virtual bool isTerminal() { return false; }
    //! True if this is the constant "true"
    bool isTrue() const { return op == opTrue; }
    //! True if this is the constant "false"
    bool isFalse() const { return op == opFalse; }
    //! True if this is a disjunction, i.e. x or y
    bool isDisjunction() const { return op == opOr; }
    //! True if this is a conjunction, i.e. x and y
    bool isConjunction() const { return op == opAnd; }
    //! True if this is a boolean constant
    bool isBoolConst() { return op == opTrue || op == opFalse; }
    //! True if this is an equality (== or !=)
    bool isEquality() { return op == opEquals /*|| op == opNotEqual*/; }
    //! True if this is a comparison
    bool isComparison() {
        return op == opEquals || op == opNotEqual || op == opGtr || op == opLess || op == opGtrUns || op == opLessUns ||
               op == opGtrEq || op == opLessEq || op == opGtrEqUns || op == opLessEqUns;
    }
    //! True if this is a TypeVal
    bool isTypeVal() { return op == opTypeVal; }
    //! True if this is a machine feature
    bool isMachFtr() { return op == opMachFtr; }
    //! True if this is a parameter. Note: opParam has two meanings: a SSL parameter, or a function parameter
    bool isParam() { return op == opParam; }

    //! True if this is a location
    bool isLocation() { return op == opMemOf || op == opRegOf || op == opGlobal || op == opLocal || op == opParam; }
    //! True if this is a typed expression
    bool isTypedExp() const { return op == opTypedExp; }

    // FIXME: are these used?
    // Matches this expression to the pattern, if successful returns a list of variable bindings, otherwise returns
    // nullptr
    virtual Exp *match(Exp *pattern);

    //! match a string pattern
    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings);

    //    //    //    //    //    //    //
    //    Search and Replace    //
    //    //    //    //    //    //    //

    //! Search for Exp *search in this Exp. If found, return true and return a ptr to the matching expression in
    //! result (useful with wildcards).
    virtual bool search(const Exp &search, Exp *&result);

    // Search for Exp search in this Exp. For each found, add a ptr to the matching expression in result (useful
    // with wildcards).      Does NOT clear result on entry
    bool searchAll(const Exp &search, std::list<Exp *> &result);

    //! Search this Exp for *search; if found, replace with *replace
    Exp *searchReplace(const Exp &search, Exp *replace, bool &change);

    //! Search *pSrc for *search; for all occurrences, replace with *replace
    Exp *searchReplaceAll(const Exp &search, Exp *replace, bool &change, bool once = false);

    // Mostly not for public use. Search for subexpression matches.
    static void doSearch(const Exp &search, Exp *&pSrc, std::list<Exp **> &li, bool once);

    // As above.
    virtual void doSearchChildren(const Exp &, std::list<Exp **> &, bool);

    /// Propagate all possible assignments to components of this expression.
    Exp *propagateAll();
    Exp *propagateAllRpt(bool &changed); // As above, but keep propagating until no change

    //    //    //    //    //    //    //
    //      Sub expressions    //
    //    //    //    //    //    //    //

    // These are here so we can (optionally) prevent code clutter.
    // Using a *Exp (that is known to be a Binary* say), you can just directly call getSubExp2.
    // However, you can still choose to cast from Exp* to Binary* etc. and avoid the virtual call
    virtual Exp *getSubExp1() { return nullptr; }
    virtual const Exp *getSubExp1() const { return nullptr; }
    virtual Exp *getSubExp2() { return nullptr; }
    virtual const Exp *getSubExp2() const { return nullptr; }
    virtual Exp *getSubExp3() { return nullptr; }
    virtual const Exp *getSubExp3() const { return nullptr; }
    virtual Exp *&refSubExp1();
    virtual Exp *&refSubExp2();
    virtual Exp *&refSubExp3();
    virtual void setSubExp1(Exp * /*e*/) { assert(false); }
    virtual void setSubExp2(Exp * /*e*/) { assert(false); }
    virtual void setSubExp3(Exp * /*e*/) { assert(false); }

    // Get the complexity depth. Basically, add one for each unary, binary, or ternary
    int getComplexityDepth(UserProc *proc);
    // Get memory depth. Add one for each m[]
    int getMemDepth();

    //    //    //    //    //    //    //
    //    Guarded assignment    //
    //    //    //    //    //    //    //
    Exp *getGuard(); // Get the guard expression, or 0 if not

    //    //    //    //    //    //    //    //    //
    //    Expression Simplification    //
    //    //    //    //    //    //    //    //    //

    void partitionTerms(std::list<Exp *> &positives, std::list<Exp *> &negatives, std::vector<int> &integers,
                        bool negate);
    virtual Exp *simplifyArith() { return this; }
    static Exp *Accumulate(std::list<Exp *> exprs);
    // Simplify the expression
    Exp *simplify();
    virtual Exp *polySimplify(bool &bMod) {
        bMod = false;
        return this;
    }
    // Just the address simplification a[ m[ any ]]
    virtual Exp *simplifyAddr() { return this; }
    virtual Exp *simplifyConstraint() { return this; }
    Exp *fixSuccessor(); // succ(r2) -> r3
    // Kill any zero fill, sign extend, or truncates
    Exp *killFill();

    // Do the work of finding used locations. If memOnly set, only look inside m[...]
    void addUsedLocs(LocationSet &used, bool memOnly = false);

    Exp *removeSubscripts(bool &allZero);

    // Get number of definitions (statements this expression depends on)
    virtual int getNumRefs() { return 0; }

    // Convert from SSA form, where this is not subscripted (but defined at statement d)
    // Needs the UserProc for the symbol map
    Exp *fromSSAleft(UserProc *proc, Instruction *d);

    // Generate constraints for this Exp. NOTE: The behaviour is a bit different depending on whether or not
    // parameter result is a type constant or a type variable.
    // If the constraint is always satisfied, return true
    // If the constraint can never be satisfied, return false
    // Example: this is opMinus and result is <int>, constraints are:
    //     sub1 = <int> and sub2 = <int> or
    //     sub1 = <ptr> and sub2 = <ptr>
    // Example: this is opMinus and result is Tr (typeOf r), constraints are:
    //     sub1 = <int> and sub2 = <int> and Tr = <int> or
    //     sub1 = <ptr> and sub2 = <ptr> and Tr = <int> or
    //     sub1 = <ptr> and sub2 = <int> and Tr = <ptr>
    virtual Exp *genConstraints(Exp *);

    // Visitation
    // Note: best to have accept() as pure virtual, so you don't forget to implement it for new subclasses of Exp
    virtual bool accept(ExpVisitor *v) = 0;
    virtual Exp *accept(ExpModifier *v) = 0;
    void fixLocationProc(UserProc *p);
    UserProc *findProc();
    // Set or clear the constant subscripts
    void setConscripts(int n, bool bClear);
    Exp *stripSizes(); // Strip all size casts
    // Subscript all e in this Exp with statement def:
    Exp *expSubscriptVar(Exp *e, Instruction *def /*, Cfg* cfg */);
    // Subscript all e in this Exp with 0 (implicit assignments)
    Exp *expSubscriptValNull(Exp *e /*, Cfg* cfg */);
    // Subscript all locations in this expression with their implicit assignments
    Exp *expSubscriptAllNull(/*Cfg* cfg*/);
    // Perform call bypass and simple (assignment only) propagation to this exp
    // Note: can change this, so often need to clone before calling
    Exp *bypass();
    void bypassComp();                  // As above, but only the xxx of m[xxx]
    bool containsFlags();               // Check if this exp contains any flag calls
    bool containsBadMemof(UserProc *p); // Check if this Exp contains a bare (non subscripted) memof
    bool containsMemof(UserProc *proc); // Check of this Exp contains any memof at all. Not used.

    // Data flow based type analysis (implemented in type/dfa.cpp)
    // Pull type information up the expression tree
    virtual SharedType ascendType() {
        assert(0);
        return 0;
    }
    //! Push type information down the expression tree
    virtual void descendType(SharedType  /*parentType*/, bool & /*ch*/, Instruction * /*s*/) { assert(0); }

  protected:
    friend class XMLProgParser;
}; // class Exp

// Not part of the Exp class, but logically belongs with it:
QTextStream &operator<<(QTextStream &os, const Exp *p); // Print the Exp poited to by p

/***************************************************************************/ /**
  * Const is a subclass of Exp, and holds either an integer, floating point, string, or address constant
  ******************************************************************************/
class Const : public Exp {
    union {
        int i;         // Integer
                       // Note: although we have i and a as unions, both often use the same operator (opIntConst).
                       // There is no opCodeAddr any more.
        ADDRESS a;     // void* conflated with unsigned int: needs fixing
        QWord ll;      // 64 bit integer
        double d;      // Double precision float
//        const char *p; // Pointer to string
                       // Don't store string: function could be renamed
        Function *pp;      // Pointer to function
    } u;
    QString strin;
    int conscript; // like a subscript for constants
    SharedType type;    // Constants need types during type analysis
  public:
    // Special constructors overloaded for the various constants
    Const(uint32_t i);
    Const(int i);
    Const(QWord ll);
    Const(ADDRESS a);
    Const(double d);
//    Const(const char *p);
    Const(const QString &p);
    Const(Function *p);
    // Copy constructor
    Const(const Const &o);
    template <class T> static Const *get(T i) { return new Const(i); }

    // Nothing to destruct: Don't deallocate the string passed to constructor

    // Clone
    virtual Exp *clone() const;

    // Compare
    virtual bool operator==(const Exp &o) const;
    virtual bool operator<(const Exp &o) const;
    virtual bool operator*=(Exp &o);

    // Get the constant
    int getInt() const { return u.i; }
    QWord getLong() const { return u.ll; }
    double getFlt() const { return u.d; }
    QString getStr() const { return strin; }
    ADDRESS getAddr() const { return u.a; }
    QString getFuncName() const;

    // Set the constant
    void setInt(int i) { u.i = i; }
    void setLong(QWord ll) { u.ll = ll; }
    void setFlt(double d) { u.d = d; }
    void setStr(const QString &p) { strin = p; }
    void setAddr(ADDRESS a) { u.a = a; }

    // Get and set the type
    SharedType getType() { return type; }
    const SharedType getType() const { return type; }
    void setType(SharedType ty) { type = ty; }

    virtual void print(QTextStream &os, bool = false) const;
    // Print "recursive" (extra parens not wanted at outer levels)
    void printNoQuotes(QTextStream &os);
    virtual void printx(int ind) const;

    virtual void appendDotFile(QTextStream &of);
    virtual Exp *genConstraints(Exp *restrictTo);

    // Visitation
    virtual bool accept(ExpVisitor *v);
    virtual Exp *accept(ExpModifier *v);

    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings);

    int getConscript() { return conscript; }
    void setConscript(int cs) { conscript = cs; }

    virtual SharedType ascendType();
    virtual void descendType(SharedType parentType, bool &ch, Instruction *s);

  protected:
    friend class XMLProgParser;
}; // class Const

/***************************************************************************/ /**
  * Terminal is a subclass of Exp, and holds special zero arity items such as opFlags (abstract flags register)
  ******************************************************************************/
class Terminal : public Exp {
  public:
    // Constructors
    Terminal(OPER op);
    Terminal(const Terminal &o); // Copy constructor
    static Exp *get(OPER op) { return new Terminal(op); }

    // Clone
    virtual Exp *clone() const override;

    // Compare
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;
    virtual void print(QTextStream &os, bool = false) const override;
    virtual void appendDotFile(QTextStream &of) override;
    virtual void printx(int ind) const override;
    virtual bool isTerminal()  override { return true; }

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool &ch, Instruction *s) override;

    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings) override;

  protected:
    friend class XMLProgParser;
}; // class Terminal

/***************************************************************************/ /**
  * Unary is a subclass of Exp, holding one subexpression
  ******************************************************************************/
class Unary : public Exp {
  protected:
    Exp *subExp1; // One subexpression pointer

    // Constructor, with just ID
    Unary(OPER op);

  public:
    // Constructor, with ID and subexpression
    Unary(OPER op, Exp *e);
    // Copy constructor
    Unary(const Unary &o);
    static Exp *get(OPER op, Exp *e1) { return new Unary(op, e1); }

    // Clone
    virtual Exp *clone() const override;

    // Compare
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;

    // Destructor
    virtual ~Unary();

    // Arity
    virtual int getArity() const override { return 1; }

    // Print
    virtual void print(QTextStream &os, bool html = false) const override;
    virtual void appendDotFile(QTextStream &of) override;
    virtual void printx(int ind) const override;

    // Set first subexpression
    void setSubExp1(Exp *e) override;
    void setSubExp1ND(Exp *e) { subExp1 = e; }
    // Get first subexpression
    Exp *getSubExp1() override;
    const Exp *getSubExp1() const override;
    // Get a reference to subexpression 1
    Exp *&refSubExp1() override;

    virtual Exp *match(Exp *pattern) override;
    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings) override;

    // Search children
    void doSearchChildren(const Exp &search, std::list<Exp **> &li, bool once) override;

    // Do the work of simplifying this expression
    virtual Exp *polySimplify(bool &bMod) override;
    Exp *simplifyArith() override;
    Exp *simplifyAddr() override;
    virtual Exp *simplifyConstraint() override;

    // Type analysis
    virtual Exp *genConstraints(Exp *restrictTo) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool &ch, Instruction *s) override;

  protected:
    friend class XMLProgParser;
}; // class Unary

/**
 * Binary is a subclass of Unary, holding two subexpressions
 */
class Binary : public Unary {
  protected:
    Exp *subExp2; // Second subexpression pointer

    // Constructor, with ID
    Binary(OPER op);

  public:
    // Constructor, with ID and subexpressions
    Binary(OPER op, Exp *e1, Exp *e2);
    // Copy constructor
    Binary(const Binary &o);
    static Binary *get(OPER op, Exp *e1, Exp *e2) { return new Binary(op, e1, e2); }

    // Clone
    virtual Exp *clone() const override;

    // Compare
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;

    // Destructor
    virtual ~Binary();

    // Arity
    int getArity() const override { return 2; }

    // Print
    virtual void print(QTextStream &os, bool html = false) const override;
    virtual void printr(QTextStream &os, bool html = false) const override;
    virtual void appendDotFile(QTextStream &of) override;
    virtual void printx(int ind) const override;

    // Set second subexpression
    void setSubExp2(Exp *e) override;
    // Get second subexpression
    Exp *getSubExp2() override;
    const Exp *getSubExp2() const override;
    void commute();     //!< Commute the two operands
    Exp *&refSubExp2() override; //!< Get a reference to subexpression 2

    virtual Exp *match(Exp *pattern) override;
    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings) override;

    // Search children
    void doSearchChildren(const Exp &search, std::list<Exp **> &li, bool once) override;

    // Do the work of simplifying this expression
    virtual Exp *polySimplify(bool &bMod) override;
    Exp *simplifyArith() override;
    Exp *simplifyAddr() override;
    virtual Exp *simplifyConstraint() override;

    // Type analysis
    virtual Exp *genConstraints(Exp *restrictTo) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool &ch, Instruction *s) override;

  private:
    Exp *constrainSub(TypeVal *typeVal1, TypeVal *typeVal2);

  protected:
    friend class XMLProgParser;
}; // class Binary

/***************************************************************************/ /**
  * Ternary is a subclass of Binary, holding three subexpressions
  ******************************************************************************/
class Ternary : public Binary {
    Exp *subExp3; // Third subexpression pointer

    // Constructor, with operator
    Ternary(OPER op);

  public:
    // Constructor, with operator and subexpressions
    Ternary(OPER op, Exp *e1, Exp *e2, Exp *e3);
    // Copy constructor
    Ternary(const Ternary &o);

    // Clone
    virtual Exp *clone() const override;

    // Compare
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;

    // Destructor
    virtual ~Ternary();

    // Arity
    int getArity() const override { return 3; }

    // Print
    virtual void print(QTextStream &os, bool html = false) const override;
    virtual void printr(QTextStream &os, bool = false) const override;
    virtual void appendDotFile(QTextStream &of) override;
    virtual void printx(int ind) const override;

    // Set third subexpression
    void setSubExp3(Exp *e) override;
    // Get third subexpression
    Exp *getSubExp3() override;
    const Exp *getSubExp3() const override;
    // Get a reference to subexpression 3
    Exp *&refSubExp3() override;

    // Search children
    void doSearchChildren(const Exp &search, std::list<Exp **> &li, bool once) override;

    virtual Exp *polySimplify(bool &bMod) override;
    Exp *simplifyArith() override;
    Exp *simplifyAddr() override;

    // Type analysis
    virtual Exp *genConstraints(Exp *restrictTo) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType /*parentType*/, bool &ch, Instruction *s) override;

  protected:
    friend class XMLProgParser;
}; // class Ternary

/***************************************************************************/ /**
  * TypedExp is a subclass of Unary, holding one subexpression and a Type
  ******************************************************************************/
class TypedExp : public Unary {
    SharedType type;

  public:
    // Constructor
    TypedExp();
    // Constructor, subexpression
    TypedExp(Exp *e1);
    // Constructor, type, and subexpression.
    // A rare const parameter allows the common case of providing a temporary,
    // e.g. foo = new TypedExp(Type(INTEGER), ...);
    TypedExp(SharedType ty, Exp *e1);
    // Copy constructor
    TypedExp(TypedExp &o);

    // Clone
    virtual Exp *clone() const override;

    // Compare
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator<<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;

    virtual void print(QTextStream &os, bool html = false) const override;
    virtual void appendDotFile(QTextStream &of) override;
    virtual void printx(int ind) const override;

    // Get and set the type
    virtual SharedType getType() { return type; }
    virtual const SharedType &getType() const { return type; }
    virtual void setType(SharedType ty) { type = ty; }

    // polySimplify
    virtual Exp *polySimplify(bool &bMod) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType , bool &, Instruction *) override;

  protected:
    friend class XMLProgParser;
}; // class TypedExp

/***************************************************************************/ /**
  * FlagDef is a subclass of Unary, and holds a list of parameters (in the subexpression), and a pointer to an RTL
  ******************************************************************************/
class FlagDef : public Unary {
    SharedRTL rtl;

  public:
    FlagDef(Exp *params, SharedRTL rtl); // Constructor
    virtual ~FlagDef();             // Destructor
    virtual void appendDotFile(QTextStream &of);
//    RTL *getRtl() { return rtl; }
//    void setRtl(RTL *r) { rtl = r; }

    // Visitation
    virtual bool accept(ExpVisitor *v);
    virtual Exp *accept(ExpModifier *v);

  protected:
    friend class XMLProgParser;
}; // class FlagDef

/***************************************************************************/ /**
  * RefExp is a subclass of Unary, holding an ordinary Exp pointer, and a pointer to a defining statement (could be a
  * phi assignment).  This is used for subscripting SSA variables. Example:
  *   m[1000] becomes m[1000]{3} if defined at statement 3
  * The integer is really a pointer to the defining statement, printed as the statement number for compactness.
  ******************************************************************************/
class RefExp : public Unary {
    Instruction *def; // The defining statement

  public:
    // Constructor with expression (e) and statement defining it (def)
    RefExp(Exp *e, Instruction *def);
    // virtual ~RefExp()   {
    //                        def = nullptr;
    //                    }
    static RefExp *get(Exp *e, Instruction *def) { return new RefExp(e, def); }
    virtual Exp *clone() const override;
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;

    virtual void print(QTextStream &os, bool html = false) const override;
    virtual void printx(int ind) const override;
    // virtual int        getNumRefs() {return 1;}
    Instruction *getDef() { return def; } // Ugh was called getRef()
    Exp *addSubscript(Instruction *_def) {
        def = _def;
        return this;
    }
    void setDef(Instruction *_def) { /*assert(_def);*/
        def = _def;
    }
    virtual Exp *genConstraints(Exp *restrictTo) override;
    bool references(Instruction *s) { return def == s; }
    virtual Exp *polySimplify(bool &bMod) override;
    virtual Exp *match(Exp *pattern) override;
    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings) override;

    // Before type analysis, implicit definitions are nullptr.  During and after TA, they point to an implicit
    // assignment statement.  Don't implement here, since it would require #including of statement.h
    bool isImplicitDef();

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

    virtual SharedType ascendType() override;
    virtual void descendType(SharedType parentType, bool &ch, Instruction *s) override;

  protected:
    RefExp() : Unary(opSubscript), def(nullptr) {}
    friend class XMLProgParser;
}; // class RefExp

/***************************************************************************/ /**
 class TypeVal. Just a Terminal with a Type. Used for type values in constraints
 ==============================================================================*/
class TypeVal : public Terminal {
    SharedType val;

  public:
    TypeVal(SharedType ty);
    ~TypeVal();

    virtual SharedType getType() { return val; }
    virtual void setType(SharedType t) { val = t; }
    virtual Exp *clone() const override;
    virtual bool operator==(const Exp &o) const override;
    virtual bool operator<(const Exp &o) const override;
    virtual bool operator*=(Exp &o) override;
    virtual void print(QTextStream &os, bool = false) const override;
    virtual void printx(int ind) const override;
    virtual Exp *genConstraints(Exp * /*restrictTo*/)  override {
        assert(0);
        return nullptr;
    } // Should not be constraining constraints
    // virtual Exp        *match(Exp *pattern);

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;

  protected:
    friend class XMLProgParser;
}; // class TypeVal

class Location : public Unary {
  protected:
    UserProc *proc;

  public:
    // Constructor with ID, subexpression, and UserProc*
    Location(OPER op, Exp *e, UserProc *proc);
    // Copy constructor
    Location(Location &o);
    // Custom constructor
    static Exp *get(OPER op, Exp *e, UserProc *proc) { return new Location(op, e, proc); }
    static Exp *regOf(int r) { return get(opRegOf, Const::get(r), nullptr); }
    static Exp *regOf(Exp *e) { return get(opRegOf, e, nullptr); }
    static Exp *memOf(Exp *e, UserProc *p = nullptr) { return get(opMemOf, e, p); }
    static Location *tempOf(Exp *e) { return new Location(opTemp, e, nullptr); }
    static Exp *global(const char *nam, UserProc *p) { return get(opGlobal, Const::get(nam), p); }
    static Exp *global(const QString &nam, UserProc *p) { return get(opGlobal, Const::get(nam), p); }
    static Location *local(const QString &nam, UserProc *p);
    static Exp *param(const char *nam, UserProc *p = nullptr) { return get(opParam, Const::get(nam), p); }
    static Exp *param(const QString &nam, UserProc *p = nullptr) { return get(opParam, Const::get(nam), p); }
    // Clone
    virtual Exp *clone() const override;

    void setProc(UserProc *p) { proc = p; }
    UserProc *getProc() { return proc; }

    virtual Exp *polySimplify(bool &bMod) override;
    virtual void getDefinitions(LocationSet &defs);

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual Exp *accept(ExpModifier *v) override;
    virtual bool match(const QString &pattern, std::map<QString, Exp *> &bindings) override;

  protected:
    friend class XMLProgParser;
    Location(OPER op) : Unary(op), proc(nullptr) {}
}; // class Location

typedef std::set<Exp *, lessExpStar> sExp;
