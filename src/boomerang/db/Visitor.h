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


/***************************************************************************/ /**
 * \file       visitor.h
 * \details   Provides the definition for the various visitor and modifier classes.
 *            These classes sometimes are associated with Statement and Exp classes, so they are here to avoid
 *            \#include problems, to make exp.cpp and statement.cpp a little less huge.
 *            The main advantage is that they are quick and easy to implement (once you get used to them), and it
 *            avoids having to declare methods in every Statement or Exp subclass
 * TOP LEVEL CLASSES:
 *     Class name    |  description
 * ------------------|-----------------------------
 * ExpVisitor        | (visit expressions)
 * StmtVisitor       | (visit statements)
 * StmtExpVisitor    | (visit expressions in statements)
 * ExpModifier       | (modify expressions)
 * SimpExpModifier   | (simplifying expression modifier)
 * StmtModifier      | (modify expressions in statements; not abstract)
 * StmtPartModifier  | (as above with special case for whole of LHS)
 *
 * \note There are separate Visitor and Modifier classes. Visitors are more suited for searching: they have the
 * capability of stopping the recursion, but can't change the class of a top level expression. Visitors can also
 * override (prevent) the usual recursing to child objects. Modifiers always recurse to the end, and the ExpModifiers'
 * visit
 * function returns an Exp* so that the top level expression can change class (e.g. RefExp to Binary).
 * The accept() functions (in the target classes) are always the same for all visitors; they encapsulate where the
 * visitable parts of a Statement or expression are.
 * The visit() functions contain the logic of the search/modify/whatever.  Often only a few visitor functions have to
 * do anything. Unfortunately, the visit functions are members of the Visitor (or Modifier) classes, and so have to
 * use public functions of the target classes.
 */

#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/FlagDef.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/TypeVal.h"

class Statement;
class Assignment;
class Assign;
class ImplicitAssign;
class PhiAssign;
class BoolAssign;
class CaseStatement;
class CallStatement;
class ReturnStatement;
class GotoStatement;
class BranchStatement;
class ImpRefStatement;
class JunctionStatement;

class RTL;
class UserProc;
class Cfg;
class Prog;
class BasicBlock;

class LocationSet;


/**
 * The ExpVisitor class is used to iterate over all subexpressions in an expression.
 */
class ExpVisitor
{
public:
    ExpVisitor() {}
    virtual ~ExpVisitor() {}

    // visitor functions return false to abandon iterating through the expression (terminate the search)
    // Set override true to not do the usual recursion into children
    virtual bool visit(const std::shared_ptr<Unary>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(const std::shared_ptr<Binary>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(const std::shared_ptr<Ternary>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(const std::shared_ptr<TypedExp>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(const std::shared_ptr<FlagDef>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(const std::shared_ptr<RefExp>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(const std::shared_ptr<Location>& /*e*/, bool& override)
    {
        override = false;
        return true;
    }

    // These three have zero arity, so there is nothing to override
    virtual bool visit(const std::shared_ptr<Const>& /*e*/) { return true; }
    virtual bool visit(const std::shared_ptr<Terminal>& /*e*/) { return true; }
    virtual bool visit(const std::shared_ptr<TypeVal>& /*e*/) { return true; }
};


/// This class visits subexpressions, and if a location, sets the UserProc
class FixProcVisitor : public ExpVisitor
{
public:
    void setProc(UserProc *p) { proc = p; }
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

    // All other virtual functions inherit from ExpVisitor, i.e. they just visit their children recursively

private:
    // the enclosing UserProc (if a Location)
    UserProc *proc;
};

/// This class is more or less the opposite of the above.
/// It finds a proc by visiting the whole expression if necessary
class GetProcVisitor : public ExpVisitor
{
public:
    GetProcVisitor() { proc = nullptr; } // Constructor
    UserProc *getProc() const { return proc; }
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

    // All others inherit and visit their children

private:
    UserProc *proc; ///< The result (or nullptr)

};


/// This class visits subexpressions, and if a Const, sets or clears a new conscript
class SetConscripts : public ExpVisitor
{
public:
    SetConscripts(int n, bool clear)
        : m_bInLocalGlobal(false)
        , m_bClear(clear)
    {
        m_curConscript = n;
    }

    int getLast() const { return m_curConscript; }

    virtual bool visit(const std::shared_ptr<Const>& e) override;
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    virtual bool visit(const std::shared_ptr<Binary>& b, bool& override) override;

private:
    int m_curConscript;
    bool m_bInLocalGlobal; // True when inside a local or global
    bool m_bClear;         // True when clearing, not setting

};


/**
 * The ExpModifier class is used to iterate over all subexpressions in an expression. It contains methods for each kind
 * of subexpression found in an and can be used to eliminate switch statements.
 * It is a little more expensive to use than ExpVisitor, but can make changes to the expression
 */
class ExpModifier
{
protected:
    bool m_mod = false; // Set if there is any change. Don't have to implement

public:
    ExpModifier()          = default;
    virtual ~ExpModifier() = default;

    bool isMod() const { return m_mod; }
    void clearMod() { m_mod = false; }

    /// visitor functions
    /// Most times these won't be needed. You only need to override the ones that make a change.
    /// preVisit comes before modifications to the children (if any)
    virtual SharedExp preVisit(const std::shared_ptr<Unary>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<Binary>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<Ternary>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<TypedExp>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<FlagDef>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur)
    {
        recur = true;
        return e;
    }

    virtual SharedExp preVisit(const std::shared_ptr<Const>& e) { return e; }
    virtual SharedExp preVisit(const std::shared_ptr<Terminal>& e) { return e; }
    virtual SharedExp preVisit(const std::shared_ptr<TypeVal>& e) { return e; }

    // postVisit comes after modifications to the children (if any)
    virtual SharedExp postVisit(const std::shared_ptr<Unary>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<Binary>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<Ternary>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<TypedExp>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<FlagDef>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<RefExp>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<Location>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<Const>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<Terminal>& e) { return e; }
    virtual SharedExp postVisit(const std::shared_ptr<TypeVal>& e) { return e; }
};


/**
 * The StmtVisitor class is used for code that has to work with all the Statement classes. One advantage is that you
 * don't need to declare a function in every class derived from Statement: the accept methods already do that for you.
 * It does not automatically visit the expressions in the statement.
 */
class StmtVisitor
{
public:
    StmtVisitor() {}
    virtual ~StmtVisitor() {}

    // visitor functions,
    // returns true to continue iterating the container
    virtual bool visit(RTL *rtl); // By default, visits all statements

    virtual bool visit(Assign * /*stmt*/) { return true; }
    virtual bool visit(PhiAssign * /*stmt*/) { return true; }
    virtual bool visit(ImplicitAssign * /*stmt*/) { return true; }
    virtual bool visit(BoolAssign * /*stmt*/) { return true; }
    virtual bool visit(GotoStatement * /*stmt*/) { return true; }
    virtual bool visit(BranchStatement * /*stmt*/) { return true; }
    virtual bool visit(CaseStatement * /*stmt*/) { return true; }
    virtual bool visit(CallStatement * /*stmt*/) { return true; }
    virtual bool visit(ReturnStatement * /*stmt*/) { return true; }
    virtual bool visit(ImpRefStatement * /*stmt*/) { return true; }
    virtual bool visit(JunctionStatement * /*stmt*/) { return true; }
};


class StmtConscriptSetter : public StmtVisitor
{
public:
    StmtConscriptSetter(int n, bool _bClear)
        : m_curConscript(n)
        , m_clear(_bClear) {}
    int getLast() const { return m_curConscript; }

    virtual bool visit(Assign *stmt) override;
    virtual bool visit(PhiAssign *stmt) override;
    virtual bool visit(ImplicitAssign *stmt) override;
    virtual bool visit(BoolAssign *stmt) override;
    virtual bool visit(CaseStatement *stmt) override;
    virtual bool visit(CallStatement *stmt) override;
    virtual bool visit(ReturnStatement *stmt) override;
    virtual bool visit(BranchStatement *stmt) override;
    virtual bool visit(ImpRefStatement *stmt) override;

private:
    int m_curConscript;
    bool m_clear;
};


/// StmtExpVisitor is a visitor of statements, and of expressions within those expressions. The visiting of expressions
/// (after the current node) is done by an ExpVisitor (i.e. this is a preorder traversal).
class StmtExpVisitor
{
public:
    ExpVisitor *ev;
    StmtExpVisitor(ExpVisitor *v, bool _ignoreCol = true)
        : ev(v)
        , m_ignoreCol(_ignoreCol) {}

    virtual ~StmtExpVisitor() {}

    virtual bool visit(Assign * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(PhiAssign * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(ImplicitAssign * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(BoolAssign * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(GotoStatement * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(BranchStatement * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(CaseStatement * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(CallStatement * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(ReturnStatement * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    virtual bool visit(ImpRefStatement * /*stmt*/, bool& override)
    {
        override = false;
        return true;
    }

    bool isIgnoreCol() const { return m_ignoreCol; }

private:
    bool m_ignoreCol; ///< True if ignoring collectors
};


// StmtModifier is a class that for all expressions in this statement, makes a modification.
// The modification is as a result of an ExpModifier; there is a pointer to such an ExpModifier in a StmtModifier.
// Even the top level of the LHS of assignments are changed. This is useful e.g. when modifiying locations to locals
// as a result of converting from SSA form, e.g. eax := ebx -> local1 := local2
// Classes that derive from StmtModifier inherit the code (in the accept member functions) to modify all the expressions
// in the various types of statement.
// Because there is nothing specialised about a StmtModifier, it is not an abstract class (can be instantiated).
class StmtModifier
{
public:
    ExpModifier *m_mod;  ///< The expression modifier object

    StmtModifier(ExpModifier *em, bool ic = false)
        : m_mod(em)
        , m_ignoreCol(ic) {}

    virtual ~StmtModifier() {}
    bool ignoreCollector() const { return m_ignoreCol; }

    // This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
    virtual void visit(Assign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(PhiAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ImplicitAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(BoolAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(GotoStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(BranchStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(CaseStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(CallStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ReturnStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ImpRefStatement * /*s*/, bool& recur) { recur = true; }

protected:
    bool m_ignoreCol;
};


/// As above, but specialised for propagating to. The top level of the lhs of assignment-like statements (including
/// arguments in calls) is not modified. So for example eax := ebx -> eax := local2, but in m[xxx] := rhs, the rhs and
/// xxx are modified, but not the m[xxx]
class StmtPartModifier
{
    bool m_ignoreCol;

public:
    ExpModifier *mod;                                                              // The expression modifier object
    StmtPartModifier(ExpModifier *em, bool ignoreCol = false)
        : m_ignoreCol(ignoreCol)
        , mod(em) {}

    virtual ~StmtPartModifier() {}
    bool ignoreCollector() const { return m_ignoreCol; }

    // This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
    virtual void visit(Assign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(PhiAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ImplicitAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(BoolAssign * /*s*/, bool& recur) { recur = true; }
    virtual void visit(GotoStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(BranchStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(CaseStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(CallStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ReturnStatement * /*s*/, bool& recur) { recur = true; }
    virtual void visit(ImpRefStatement * /*s*/, bool& recur) { recur = true; }
};


class PhiStripper : public StmtModifier
{
    bool m_del; // Set true if this statment is to be deleted

public:
    PhiStripper(ExpModifier *em)
        : StmtModifier(em)
    {
        m_del = false;
    }

    virtual void visit(PhiAssign *, bool& recur) override;

    bool getDelete() const { return m_del; }
};

/// A simplifying expression modifier. It does a simplification on the parent after a child has been modified
class SimpExpModifier : public ExpModifier
{
public:
    SimpExpModifier()
    {
        m_mask      = 1;
        m_unchanged = (unsigned)-1;
    }

    unsigned getUnchanged() { return m_unchanged; }
    bool isTopChanged() { return !(m_unchanged & m_mask); }
    SharedExp preVisit(const std::shared_ptr<Unary>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<Binary>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<Ternary>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<TypedExp>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<FlagDef>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur) override
    {
        recur    = true;
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<Const>& e)  override
    {
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<Terminal>& e) override
    {
        m_mask <<= 1;
        return e;
    }

    SharedExp preVisit(const std::shared_ptr<TypeVal>& e)  override
    {
        m_mask <<= 1;
        return e;
    }

    SharedExp postVisit(const std::shared_ptr<Unary>& e) override;
    SharedExp postVisit(const std::shared_ptr<Binary>& e) override;
    SharedExp postVisit(const std::shared_ptr<Ternary>& e) override;
    SharedExp postVisit(const std::shared_ptr<TypedExp>& e) override;
    SharedExp postVisit(const std::shared_ptr<FlagDef>& e) override;
    SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;
    SharedExp postVisit(const std::shared_ptr<Location>& e) override;
    SharedExp postVisit(const std::shared_ptr<Const>& e) override;
    SharedExp postVisit(const std::shared_ptr<Terminal>& e) override;
    SharedExp postVisit(const std::shared_ptr<TypeVal>& e) override;

protected:
    // These two provide 31 bits (or sizeof(int)-1) of information about whether the child is unchanged.
    // If the mask overflows, it goes to zero, and from then on the child is reported as always changing.
    // (That's why it's an "unchanged" set of flags, instead of a "changed" set).
    // This is used to avoid calling simplify in most cases where it is not necessary.
    unsigned int m_mask;
    unsigned int m_unchanged;
};


/// A modifying visitor to process all references in an expression, bypassing calls (and phi statements if they have been
/// replaced by copy assignments), and performing simplification on the direct parent of the expression that is modified.
/// NOTE: this is sometimes not enough! Consider changing (r+x)+K2) where x gets changed to K1. Now you have (r+K1)+K2,
/// but simplifying only the parent doesn't simplify the K1+K2.
/// Used to also propagate, but this became unwieldy with -l propagation limiting
class CallBypasser : public SimpExpModifier
{
public:
    CallBypasser(Statement *enclosing)
        : m_enclosingStmt(enclosing) {}
    virtual SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;
    virtual SharedExp postVisit(const std::shared_ptr<Location>& e) override;

private:
    Statement *m_enclosingStmt; // Statement that is being modified at present, for debugging only
};


class UsedLocsFinder : public ExpVisitor
{
public:
    UsedLocsFinder(LocationSet& _used, bool _memOnly)
        : m_used(&_used)
        , m_memOnly(_memOnly) {}
    ~UsedLocsFinder() {}

    LocationSet *getLocSet() { return m_used; }
    void setMemOnly(bool b)
    {
        m_memOnly = b;
    }

    bool isMemOnly() { return m_memOnly; }

    bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;
    // Add used locations finder
    bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    bool visit(const std::shared_ptr<Terminal>& e) override;

private:
    LocationSet *m_used; // Set of Exps
    bool m_memOnly;      // If true, only look inside m[...]
};


/// This class differs from the above in these ways:
///  1) it counts locals implicitly referred to with (cast to pointer)(sp-K)
///  2) it does not recurse inside the memof (thus finding the stack pointer as a local)
///  3) only used after fromSSA, so no RefExps to visit
class UsedLocalFinder : public ExpVisitor
{
public:
    UsedLocalFinder(LocationSet& _used, UserProc *_proc)
        : used(&_used)
        , proc(_proc)
        , all(false) {}
    ~UsedLocalFinder() {}

    LocationSet *getLocSet() { return used; }
    bool wasAllFound() { return all; }

    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    virtual bool visit(const std::shared_ptr<TypedExp>& e, bool& override) override;
    virtual bool visit(const std::shared_ptr<Terminal>& e) override;

private:
    LocationSet *used; // Set of used locals' names
    UserProc *proc;    // Enclosing proc
    bool all;          // True if see opDefineAll
};


class UsedLocsVisitor : public StmtExpVisitor
{
public:
    UsedLocsVisitor(ExpVisitor *v, bool cc)
        : StmtExpVisitor(v)
        , m_countCol(cc) {}
    virtual ~UsedLocsVisitor() {}

    /// Needs special attention because the lhs of an assignment isn't used
    /// (except where it's m[blah], when blah is used)
    virtual bool visit(Assign *stmt, bool& override) override;
    virtual bool visit(PhiAssign *stmt, bool& override) override;
    virtual bool visit(ImplicitAssign *stmt, bool& override) override;

    // A BoolAssign uses its condition expression, but not its destination (unless it's an m[x], in which case x is
    // used and not m[x])
    virtual bool visit(BoolAssign *stmt, bool& override) override;

    // Returns aren't used (again, except where m[blah] where blah is used), and there is special logic for when the
    // pass is final
    virtual bool visit(CallStatement *stmt, bool& override) override;

    // Only consider the first return when final
    virtual bool visit(ReturnStatement *stmt, bool& override) override;

private:
    bool m_countCol; ///< True to count uses in collectors
};


class ExpSubscripter : public ExpModifier
{
public:
    ExpSubscripter(const SharedExp& s, Statement *d)
        : m_search(s)
        , m_def(d) {}

    SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur) override;
    SharedExp preVisit(const std::shared_ptr<Binary>& e, bool& recur) override;
    SharedExp preVisit(const std::shared_ptr<Terminal>& e) override;
    SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur) override;

private:
    SharedExp m_search;
    Statement *m_def;
};


class StmtSubscripter : public StmtModifier
{
public:
    StmtSubscripter(ExpSubscripter *es)
        : StmtModifier(es) {}
    virtual ~StmtSubscripter() {}

    virtual void visit(Assign *s, bool& recur) override;
    virtual void visit(PhiAssign *s, bool& recur) override;
    virtual void visit(ImplicitAssign *s, bool& recur) override;
    virtual void visit(BoolAssign *s, bool& recur) override;
    virtual void visit(CallStatement *s, bool& recur) override;
};


class SizeStripper : public ExpModifier
{
public:
    SizeStripper() {}
    virtual ~SizeStripper() {}

    SharedExp preVisit(const std::shared_ptr<Binary>& b, bool& recur) override;
};


class ExpConstCaster : public ExpModifier
{
public:
    ExpConstCaster(int _num, SharedType _ty)
        : m_num(_num)
        , m_ty(_ty)
        , m_changed(false) {}

    virtual ~ExpConstCaster() {}

    bool isChanged() const { return m_changed; }

    SharedExp preVisit(const std::shared_ptr<Const>& c) override;

private:
    int m_num;
    SharedType m_ty;
    bool m_changed;
};


class ConstFinder : public ExpVisitor
{
public:
    ConstFinder(std::list<std::shared_ptr<Const> >& _lc)
        : m_constList(_lc) {}
    virtual ~ConstFinder() {}

    // This is the code (apart from definitions) to find all constants in a Statement
    virtual bool visit(const std::shared_ptr<Const>& e) override;
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

private:
    std::list<std::shared_ptr<Const> >& m_constList;
};


class StmtConstFinder : public StmtExpVisitor
{
public:
    StmtConstFinder(ConstFinder *v)
        : StmtExpVisitor(v) {}
};


/// This class is an ExpModifier because although most of the time it merely maps expressions to locals, in one case,
/// where sp-K is found, we replace it with a[m[sp-K]] so the back end emits it as &localX.
/// FIXME: this is probably no longer necessary, since the back end no longer maps anything!
class DfaLocalMapper : public ExpModifier
{
public:

    // Map expressions to locals, using the (so far DFA based) type analysis information
    // Basically, descend types, and when you get to m[...] compare with the local high level pattern;
    // when at a sum or difference, check for the address of locals high level pattern that is a pointer
    DfaLocalMapper(UserProc *proc);

    SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur) override; // To process m[X]

//    SharedExp preVisit(const std::shared_ptr<Unary> & e, bool& recur) override;        // To process a[X]
    SharedExp preVisit(const std::shared_ptr<Binary>& e, bool& recur) override;   // To look for sp -+ K
    SharedExp preVisit(const std::shared_ptr<TypedExp>& e, bool& recur) override; // To prevent processing TypedExps more than once

public:
    bool change; // True if changed this statement

private:
    UserProc *m_proc;
    Prog *m_prog;
    std::shared_ptr<Signature> m_sig;      ///< Look up once (from proc) for speed

    // Common processing for the two main cases (visiting a Location or a Binary)
    bool processExp(const SharedExp& e);   ///< Common processing here
};


// Convert any exp{-} (with null definition) so that the definition points instead to an implicit assignment (exp{0})
// Note it is important to process refs in a depth first manner, so that e.g. m[sp{-}-8]{-} -> m[sp{0}-8]{-} first, so
// that there is never an implicit definition for m[sp{-}-8], only ever for m[sp{0}-8]
class ImplicitConverter : public ExpModifier
{
public:
    ImplicitConverter(Cfg *cfg)
        : m_cfg(cfg) {}

    // This is in the POST visit function, because it's important to process any child expressions first.
    // Otherwise, for m[r28{0} - 12]{0}, you could be adding an implicit assignment with a nullptr definition for r28.
    SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;

private:
    Cfg *m_cfg;
};


class StmtImplicitConverter : public StmtModifier
{
public:
    StmtImplicitConverter(ImplicitConverter *ic, Cfg *cfg)
        : StmtModifier(ic, false)
        ,                          // False to not ignore collectors (want to make sure that
        m_cfg(cfg) {}              //  collectors have valid expressions so you can ascendType)
    virtual void visit(PhiAssign *s, bool& recur) override;

private:
    Cfg *m_cfg;
};


// Localiser. Subscript a location with the definitions that reach the call, or with {-} if none
class Localiser : public SimpExpModifier
{
public:
    Localiser(CallStatement *c)
        : call(c) {}

    SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur) override;
    SharedExp preVisit(const std::shared_ptr<Location>& e, bool& recur) override;
    SharedExp postVisit(const std::shared_ptr<Location>& e) override;

    // Want to be able to localise a few terminals, in particular <all>
    SharedExp postVisit(const std::shared_ptr<Terminal>& e) override;

private:
    CallStatement *call; // The call to localise to
};

class ComplexityFinder : public ExpVisitor
{
public:
    ComplexityFinder(UserProc *p)
        : count(0)
        , proc(p) {}
    int getDepth() { return count; }

    virtual bool visit(const std::shared_ptr<Unary>&, bool& override) override;
    virtual bool visit(const std::shared_ptr<Binary>&, bool& override) override;
    virtual bool visit(const std::shared_ptr<Ternary>&, bool& override) override;
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

private:
    int count;
    UserProc *proc;
};


/// Used by range analysis
class MemDepthFinder : public ExpVisitor
{
public:
    MemDepthFinder()
        : depth(0) {}
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

    int getDepth() { return depth; }

private:
    int depth;
};


/// A class to propagate everything, regardless, to this expression. Does not consider memory expressions and whether
/// the address expression is primitive. Use with caution; mostly Statement::propagateTo() should be used.
class ExpPropagator : public SimpExpModifier
{
public:
    ExpPropagator()
        : change(false) {}

    bool isChanged() { return change; }
    void clearChanged() { change = false; }

    // Ugh! This is still a separate propagation mechanism from Statement::propagateTo()
    SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;

private:
    bool change;
};

/// Test an address expression (operand of a memOf) for primitiveness (i.e. if it is possible to SSA rename the memOf
/// without problems). Note that the PrimitiveTester is not used with the memOf expression, only its address expression
class PrimitiveTester : public ExpVisitor
{
public:
    PrimitiveTester()
        : result(true) {}               // Initialise result true: need AND of all components
    bool getResult() { return result; }

    // Return true if e is a primitive expression; basically, an expression you can propagate to without causing
    // memory expression problems. See Mike's thesis for details
    // Algorithm: if find any unsubscripted location, not primitive
    //   Implicit definitions are primitive (but keep searching for non primitives)
    //   References to the results of calls are considered primitive... but only if bypassed?
    //   Other references considered non primitive
    // Start with result=true, must find primitivity in all components
    bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;

private:
    bool result;
};

/// Test if an expression (usually the RHS on an assignment) contains memory expressions. If so, it may not be safe to
/// propagate the assignment. NO LONGER USED.
class ExpHasMemofTester : public ExpVisitor
{
public:
    ExpHasMemofTester(UserProc *)
        : result(false) {}
    bool getResult() { return result; }
    bool visit(const std::shared_ptr<Location>& e, bool& override) override;

    bool result;
};


class TempToLocalMapper : public ExpVisitor
{

public:
    TempToLocalMapper(UserProc *p)
        : proc(p) {}
    bool visit(const std::shared_ptr<Location>& e, bool& override) override;

private:
    UserProc *proc; // Proc object for storing the symbols
};

/// Name registers and temporaries
class ExpRegMapper : public ExpVisitor
{
public:
    ExpRegMapper(UserProc *proc);

    // The idea here is to map the default of a register to a symbol with the type of that first use. If the register is
    // not involved in any conflicts, it will use this name by default
    bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;

private:
    UserProc *m_proc; ///< Proc object for storing the symbols
    Prog *m_prog;
};


class StmtRegMapper : public StmtExpVisitor
{
public:
    StmtRegMapper(ExpRegMapper *erm)
        : StmtExpVisitor(erm) {}

    virtual bool common(Assignment *stmt, bool& override);
    virtual bool visit(Assign *stmt, bool& override) override;
    virtual bool visit(PhiAssign *stmt, bool& override) override;
    virtual bool visit(ImplicitAssign *stmt, bool& override) override;
    virtual bool visit(BoolAssign *stmt, bool& override) override;
};


class ConstGlobalConverter : public ExpModifier
{
public:
    ConstGlobalConverter(Prog *pg)
        : m_prog(pg)
    {}

    /// Constant global converter. Example: m[m[r24{16} + m[0x8048d60]{-}]{-}]{-} -> m[m[r24{16} + 32]{-}]{-}
    /// Allows some complex variations to be matched to standard indirect call forms
    SharedExp preVisit(const std::shared_ptr<RefExp>& e, bool& recur)  override;

private:
    Prog *m_prog; // Pointer to the Prog object, for reading memory
};


/// Count the number of times a reference expression is used. Increments the count multiple times if the same reference
/// expression appears multiple times (so can't use UsedLocsFinder for this)
class ExpDestCounter : public ExpVisitor
{
public:
    ExpDestCounter(std::map<SharedExp, int, lessExpStar>& dc)
        : m_destCounts(dc) {}
    bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;

private:
    std::map<SharedExp, int, lessExpStar>& m_destCounts;
};


/// FIXME: do I need to count collectors?
/// All the visitors and modifiers should be refactored to conditionally visit
/// or modify collectors, or not
class StmtDestCounter : public StmtExpVisitor
{
public:
    StmtDestCounter(ExpDestCounter *edc)
        : StmtExpVisitor(edc) {}
    bool visit(PhiAssign *stmt, bool& override) override;
};


/// Search an expression for flags calls, e.g. SETFFLAGS(...) & 0x45
class FlagsFinder : public ExpVisitor
{
public:
    FlagsFinder()
        : m_found(false) {}
    bool isFound() { return m_found; }

private:
    virtual bool visit(const std::shared_ptr<Binary>& e, bool& override) override;

private:
    bool m_found;
};


/// Search an expression for a bad memof (non subscripted or not linked with a symbol, i.e. local or parameter)
class BadMemofFinder : public ExpVisitor
{
public:
    BadMemofFinder(UserProc *)
        : m_found(false) {}
    bool isFound() { return m_found; }

private:
    // Search for bare memofs (not subscripted) in the expression
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;
    virtual bool visit(const std::shared_ptr<RefExp>& e, bool& override) override;

private:
    bool m_found;
};


class ExpCastInserter : public ExpModifier
{
public:
    ExpCastInserter(UserProc *) {}
    static void checkMemofType(const SharedExp& memof, SharedType memofType);
    SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;
    SharedExp postVisit(const std::shared_ptr<Binary>& e) override;
    SharedExp postVisit(const std::shared_ptr<Const>& e) override;

    SharedExp preVisit(const std::shared_ptr<TypedExp>& e, bool& recur)  override
    {
        recur = false;
        return e;
    } // Don't consider if already cast
};


class StmtCastInserter : public StmtVisitor
{
public:
    StmtCastInserter() {}
    bool common(Assignment *s);
    virtual bool visit(Assign *s) override;
    virtual bool visit(PhiAssign *s) override;
    virtual bool visit(ImplicitAssign *s) override;
    virtual bool visit(BoolAssign *s) override;
};


/// Transform an exp by applying mappings to the subscripts. This used to be done by many Exp::fromSSAform() functions.
/// Note that mappings have to be done depth first, so e.g. m[r28{0}-8]{22} -> m[esp-8]{22} first, otherwise there wil be
/// a second implicit definition for m[esp{0}-8] (original should be b[esp+8] by now)
class ExpSsaXformer : public ExpModifier
{
public:
    ExpSsaXformer(UserProc *p)
        : m_proc(p) {}
    UserProc *getProc() { return m_proc; }

    virtual SharedExp postVisit(const std::shared_ptr<RefExp>& e) override;

private:
    UserProc *m_proc;
};


class StmtSsaXformer : public StmtModifier
{
public:
    StmtSsaXformer(ExpSsaXformer *esx, UserProc *p)
        : StmtModifier(esx)
        , m_proc(p) {}
    // virtual            ~StmtSsaXformer() {}
    void commonLhs(Assignment *s);

    // TODO: find out if recur should, or should not be set ?
    virtual void visit(Assign *s, bool& recur) override;
    virtual void visit(PhiAssign *s, bool& recur) override;
    virtual void visit(ImplicitAssign *s, bool& recur) override;
    virtual void visit(BoolAssign *s, bool& recur) override;
    virtual void visit(CallStatement *s, bool& recur) override;

private:
    UserProc *m_proc;
};
