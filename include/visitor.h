/*
 * Copyright (C) 2004-2006, Mike Van Emmerik and Trent Waddington
 */
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
  *visit
  * function returns an Exp* so that the top level expression can change class (e.g. RefExp to Binary).
  * The accept() functions (in the target classes) are always the same for all visitors; they encapsulate where the
  * visitable parts of a Statement or expression are.
  * The visit() functions contain the logic of the search/modify/whatever.  Often only a few visitor functions have to
  * do anything. Unfortunately, the visit functions are members of the Visitor (or Modifier) classes, and so have to
  * use public functions of the target classes.
  */

#ifndef __VISITOR_H__
#define __VISITOR_H__

#include "exp.h" // Needs to know class hierarchy, e.g. so that can convert Unary* to Exp* in return of
                 // ExpModifier::preVisit()

class Instruction;
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

class RTL;
class UserProc;
class Cfg;
class Prog;
class BasicBlock;

class LocationSet;

/*
 * The ExpVisitor class is used to iterate over all subexpressions in an expression.
 */

class ExpVisitor {

  public:
    ExpVisitor() {}
    virtual ~ExpVisitor() {}

    // visitor functions return false to abandon iterating through the expression (terminate the search)
    // Set override true to not do the usual recursion into children
    virtual bool visit(Unary * /*e*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(Binary * /*e*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(Ternary * /*e*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(TypedExp * /*e*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(FlagDef * /*e*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(RefExp * /*e*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(Location * /*e*/, bool &override) {
        override = false;
        return true;
    }
    // These three have zero arity, so there is nothing to override
    virtual bool visit(Const * /*e*/) { return true; }
    virtual bool visit(Terminal * /*e*/) { return true; }
    virtual bool visit(TypeVal * /*e*/) { return true; }
};

// This class visits subexpressions, and if a location, sets the UserProc
class FixProcVisitor : public ExpVisitor {
    // the enclosing UserProc (if a Location)
    UserProc *proc;

  public:
    void setProc(UserProc *p) { proc = p; }
    virtual bool visit(Location *e, bool &override);
    // All other virtual functions inherit from ExpVisitor, i.e. they just visit their children recursively
};

// This class is more or less the opposite of the above. It finds a proc by visiting the whole expression if necessary
class GetProcVisitor : public ExpVisitor {
    UserProc *proc; // The result (or nullptr)

  public:
    GetProcVisitor() { proc = nullptr; } // Constructor
    UserProc *getProc() { return proc; }
    virtual bool visit(Location *e, bool &override);
    // All others inherit and visit their children
};

// This class visits subexpressions, and if a Const, sets or clears a new conscript
class SetConscripts : public ExpVisitor {
    int curConscript;
    bool bInLocalGlobal; // True when inside a local or global
    bool bClear;         // True when clearing, not setting
  public:
    SetConscripts(int n, bool _bClear) : bInLocalGlobal(false), bClear(_bClear) { curConscript = n; }
    int getLast() { return curConscript; }
    virtual bool visit(Const *e);
    virtual bool visit(Location *e, bool &override);
    virtual bool visit(Binary *b, bool &override);
    // All other virtual functions inherit from ExpVisitor: return true
};

/*
 * The ExpModifier class is used to iterate over all subexpressions in an expression. It contains methods for each kind
 * of subexpression found in an and can be used to eliminate switch statements.
 * It is a little more expensive to use than ExpVisitor, but can make changes to the expression
 */
class ExpModifier {
  protected:
    bool mod; // Set if there is any change. Don't have to implement
  public:
    ExpModifier() { mod = false; }
    virtual ~ExpModifier() {}
    bool isMod() { return mod; }
    void clearMod() { mod = false; }

    // visitor functions
    // Most times these won't be needed. You only need to override the ones that make a cange.
    // preVisit comes before modifications to the children (if any)
    virtual Exp *preVisit(Unary *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(Binary *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(Ternary *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(TypedExp *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(FlagDef *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(RefExp *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(Location *e, bool &recur) {
        recur = true;
        return e;
    }
    virtual Exp *preVisit(Const *e) { return e; }
    virtual Exp *preVisit(Terminal *e) { return e; }
    virtual Exp *preVisit(TypeVal *e) { return e; }

    // postVisit comes after modifications to the children (if any)
    virtual Exp *postVisit(Unary *e) { return e; }
    virtual Exp *postVisit(Binary *e) { return e; }
    virtual Exp *postVisit(Ternary *e) { return e; }
    virtual Exp *postVisit(TypedExp *e) { return e; }
    virtual Exp *postVisit(FlagDef *e) { return e; }
    virtual Exp *postVisit(RefExp *e) { return e; }
    virtual Exp *postVisit(Location *e) { return e; }
    virtual Exp *postVisit(Const *e) { return e; }
    virtual Exp *postVisit(Terminal *e) { return e; }
    virtual Exp *postVisit(TypeVal *e) { return e; }
};

/*
 * The StmtVisitor class is used for code that has to work with all the Statement classes. One advantage is that you
 * don't need to declare a function in every class derived from Statement: the accept methods already do that for you.
 * It does not automatically visit the expressions in the statement.
 */
class StmtVisitor {
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
};

class StmtConscriptSetter : public StmtVisitor {
    int curConscript;
    bool bClear;

  public:
    StmtConscriptSetter(int n, bool _bClear) : curConscript(n), bClear(_bClear) {}
    int getLast() { return curConscript; }

    virtual bool visit(Assign *stmt);
    virtual bool visit(PhiAssign *stmt);
    virtual bool visit(ImplicitAssign *stmt);
    virtual bool visit(BoolAssign *stmt);
    virtual bool visit(CaseStatement *stmt);
    virtual bool visit(CallStatement *stmt);
    virtual bool visit(ReturnStatement *stmt);
    virtual bool visit(BranchStatement *stmt);
    virtual bool visit(ImpRefStatement *stmt);
};

// StmtExpVisitor is a visitor of statements, and of expressions within those expressions. The visiting of expressions
// (after the current node) is done by an ExpVisitor (i.e. this is a preorder traversal).
class StmtExpVisitor {
    bool ignoreCol; // True if ignoring collectors
  public:
    ExpVisitor *ev;
    StmtExpVisitor(ExpVisitor *v, bool _ignoreCol = true) : ignoreCol(_ignoreCol), ev(v) {}
    virtual ~StmtExpVisitor() {}
    virtual bool visit(Assign * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(PhiAssign * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(ImplicitAssign * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(BoolAssign * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(GotoStatement * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(BranchStatement * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(CaseStatement * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(CallStatement * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(ReturnStatement * /*stmt*/, bool &override) {
        override = false;
        return true;
    }
    virtual bool visit(ImpRefStatement * /*stmt*/, bool &override) {
        override = false;
        return true;
    }

    bool isIgnoreCol() { return ignoreCol; }
};

// StmtModifier is a class that for all expressions in this statement, makes a modification.
// The modification is as a result of an ExpModifier; there is a pointer to such an ExpModifier in a StmtModifier.
// Even the top level of the LHS of assignments are changed. This is useful e.g. when modifiying locations to locals
// as a result of converting from SSA form, e.g. eax := ebx -> local1 := local2
// Classes that derive from StmtModifier inherit the code (in the accept member functions) to modify all the expressions
// in the various types of statement.
// Because there is nothing specialised about a StmtModifier, it is not an abstract class (can be instantiated).
class StmtModifier {
  protected:
    bool ignoreCol;

  public:
    ExpModifier *mod;                                                          // The expression modifier object
    StmtModifier(ExpModifier *em, bool ic = false) : ignoreCol(ic), mod(em) {} // Constructor
    virtual ~StmtModifier() {}
    bool ignoreCollector() { return ignoreCol; }
    // This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
    virtual void visit(Assign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(PhiAssign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(ImplicitAssign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(BoolAssign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(GotoStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(BranchStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(CaseStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(CallStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(ReturnStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(ImpRefStatement * /*s*/, bool &recur) { recur = true; }
};

// As above, but specialised for propagating to. The top level of the lhs of assignment-like statements (including
// arguments in calls) is not modified. So for example eax := ebx -> eax := local2, but in m[xxx] := rhs, the rhs and
// xxx are modified, but not the m[xxx]
class StmtPartModifier {
    bool ignoreCol;

  public:
    ExpModifier *mod;                                                              // The expression modifier object
    StmtPartModifier(ExpModifier *em, bool ic = false) : ignoreCol(ic), mod(em) {} // Constructor
    virtual ~StmtPartModifier() {}
    bool ignoreCollector() { return ignoreCol; }
    // This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
    virtual void visit(Assign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(PhiAssign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(ImplicitAssign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(BoolAssign * /*s*/, bool &recur) { recur = true; }
    virtual void visit(GotoStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(BranchStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(CaseStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(CallStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(ReturnStatement * /*s*/, bool &recur) { recur = true; }
    virtual void visit(ImpRefStatement * /*s*/, bool &recur) { recur = true; }
};

class PhiStripper : public StmtModifier {
    bool del; // Set true if this statment is to be deleted
  public:
    PhiStripper(ExpModifier *em) : StmtModifier(em) { del = false; }
    virtual void visit(PhiAssign *, bool &recur);
    bool getDelete() { return del; }
};

// A simplifying expression modifier. It does a simplification on the parent after a child has been modified
class SimpExpModifier : public ExpModifier {
  protected:
    // These two provide 31 bits (or sizeof(int)-1) of information about whether the child is unchanged.
    // If the mask overflows, it goes to zero, and from then on the child is reported as always changing.
    // (That's why it's an "unchanged" set of flags, instead of a "changed" set).
    // This is used to avoid calling simplify in most cases where it is not necessary.
    unsigned mask;
    unsigned unchanged;

  public:
    SimpExpModifier() {
        mask = 1;
        unchanged = (unsigned)-1;
    }
    unsigned getUnchanged() { return unchanged; }
    bool isTopChanged() { return !(unchanged & mask); }
    virtual Exp *preVisit(Unary *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(Binary *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(Ternary *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(TypedExp *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(FlagDef *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(RefExp *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(Location *e, bool &recur) {
        recur = true;
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(Const *e) {
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(Terminal *e) {
        mask <<= 1;
        return e;
    }
    virtual Exp *preVisit(TypeVal *e) {
        mask <<= 1;
        return e;
    }

    virtual Exp *postVisit(Unary *e);
    virtual Exp *postVisit(Binary *e);
    virtual Exp *postVisit(Ternary *e);
    virtual Exp *postVisit(TypedExp *e);
    virtual Exp *postVisit(FlagDef *e);
    virtual Exp *postVisit(RefExp *e);
    virtual Exp *postVisit(Location *e);
    virtual Exp *postVisit(Const *e);
    virtual Exp *postVisit(Terminal *e);
    virtual Exp *postVisit(TypeVal *e);
};

// A modifying visitor to process all references in an expression, bypassing calls (and phi statements if they have been
// replaced by copy assignments), and performing simplification on the direct parent of the expression that is modified.
// NOTE: this is sometimes not enough! Consider changing (r+x)+K2) where x gets changed to K1. Now you have (r+K1)+K2,
// but simplifying only the parent doesn't simplify the K1+K2.
// Used to also propagate, but this became unwieldy with -l propagation limiting
class CallBypasser : public SimpExpModifier {
    Instruction *enclosingStmt; // Statement that is being modified at present, for debugging only
  public:
    CallBypasser(Instruction *enclosing) : enclosingStmt(enclosing) {}
    virtual Exp *postVisit(RefExp *e);
    virtual Exp *postVisit(Location *e);
};

class UsedLocsFinder : public ExpVisitor {
    LocationSet *used; // Set of Exps
    bool memOnly;      // If true, only look inside m[...]
  public:
    UsedLocsFinder(LocationSet &_used, bool _memOnly) : used(&_used), memOnly(_memOnly) {}
    ~UsedLocsFinder() {}

    LocationSet *getLocSet() { return used; }
    void setMemOnly(bool b) { memOnly = b; }
    bool isMemOnly() { return memOnly; }

    virtual bool visit(RefExp *e, bool &override);
    virtual bool visit(Location *e, bool &override);
    virtual bool visit(Terminal *e);
};

// This class differs from the above in these ways:
//  1) it counts locals implicitly referred to with (cast to pointer)(sp-K)
//  2) it does not recurse inside the memof (thus finding the stack pointer as a local)
//  3) only used after fromSSA, so no RefExps to visit
class UsedLocalFinder : public ExpVisitor {
    LocationSet *used; // Set of used locals' names
    UserProc *proc;    // Enclosing proc
    bool all;          // True if see opDefineAll
  public:
    UsedLocalFinder(LocationSet &_used, UserProc *_proc) : used(&_used), proc(_proc), all(false) {}
    ~UsedLocalFinder() {}

    LocationSet *getLocSet() { return used; }
    bool wasAllFound() { return all; }

    virtual bool visit(Location *e, bool &override);
    virtual bool visit(TypedExp *e, bool &override);
    virtual bool visit(Terminal *e);
};

class UsedLocsVisitor : public StmtExpVisitor {
    bool countCol; // True to count uses in collectors
  public:
    UsedLocsVisitor(ExpVisitor *v, bool cc) : StmtExpVisitor(v), countCol(cc) {}
    virtual ~UsedLocsVisitor() {}
    // Needs special attention because the lhs of an assignment isn't used (except where it's m[blah], when blah is
    // used)
    virtual bool visit(Assign *stmt, bool &override);
    virtual bool visit(PhiAssign *stmt, bool &override);
    virtual bool visit(ImplicitAssign *stmt, bool &override);
    // A BoolAssign uses its condition expression, but not its destination (unless it's an m[x], in which case x is
    // used and not m[x])
    virtual bool visit(BoolAssign *stmt, bool &override);
    // Returns aren't used (again, except where m[blah] where blah is used), and there is special logic for when the
    // pass is final
    virtual bool visit(CallStatement *stmt, bool &override);
    // Only consider the first return when final
    virtual bool visit(ReturnStatement *stmt, bool &override);
};

class ExpSubscripter : public ExpModifier {
    Exp *search;
    Instruction *def;

  public:
    ExpSubscripter(Exp *s, Instruction *d) : search(s), def(d) {}
    virtual Exp *preVisit(Location *e, bool &recur);
    virtual Exp *preVisit(Binary *e, bool &recur);
    virtual Exp *preVisit(Terminal *e);
    virtual Exp *preVisit(RefExp *e, bool &recur);
};

class StmtSubscripter : public StmtModifier {
  public:
    StmtSubscripter(ExpSubscripter *es) : StmtModifier(es) {}
    virtual ~StmtSubscripter() {}

    virtual void visit(Assign *s, bool &recur);
    virtual void visit(PhiAssign *s, bool &recur);
    virtual void visit(ImplicitAssign *s, bool &recur);
    virtual void visit(BoolAssign *s, bool &recur);
    virtual void visit(CallStatement *s, bool &recur);
};

class SizeStripper : public ExpModifier {
  public:
    SizeStripper() {}
    virtual ~SizeStripper() {}

    virtual Exp *preVisit(Binary *b, bool &recur);
};

class ExpConstCaster : public ExpModifier {
    int num;
    SharedType ty;
    bool changed;

  public:
    ExpConstCaster(int _num, SharedType _ty) : num(_num), ty(_ty), changed(false) {}
    virtual ~ExpConstCaster() {}
    bool isChanged() { return changed; }

    virtual Exp *preVisit(Const *c);
};

class ConstFinder : public ExpVisitor {
    std::list<Const *> &lc;

  public:
    ConstFinder(std::list<Const *> &_lc) : lc(_lc) {}
    virtual ~ConstFinder() {}

    virtual bool visit(Const *e);
    virtual bool visit(Location *e, bool &override);
};

class StmtConstFinder : public StmtExpVisitor {
  public:
    StmtConstFinder(ConstFinder *v) : StmtExpVisitor(v) {}
};

// This class is an ExpModifier because although most of the time it merely maps expressions to locals, in one case,
// where sp-K is found, we replace it with a[m[sp-K]] so the back end emits it as &localX.
// FIXME: this is probably no longer necessary, since the back end no longer maps anything!
class DfaLocalMapper : public ExpModifier {
    UserProc *proc;
    Prog *prog;
    Signature *sig;          // Look up once (from proc) for speed
    bool processExp(Exp *e); // Common processing here
  public:
    bool change; // True if changed this statement

    DfaLocalMapper(UserProc *proc);

    Exp *preVisit(Location *e, bool &recur); // To process m[X]
    //        Exp*        preVisit(Unary*    e, bool& recur);        // To process a[X]
    Exp *preVisit(Binary *e, bool &recur);   // To look for sp -+ K
    Exp *preVisit(TypedExp *e, bool &recur); // To prevent processing TypedExps more than once
};

#if 0 // FIXME: deleteme
class StmtDfaLocalMapper : public StmtModifier {
public:
                    StmtDfaLocalMapper(ExpModifier* em, bool ic = false) : StmtModifier(em, ic) {}

virtual void        visit(           Assign *s, bool& recur);
virtual void        visit(        PhiAssign *s, bool& recur);
virtual void        visit( ImplicitAssign *s, bool& recur);
virtual void        visit(     BoolAssign *s, bool& recur);
virtual void        visit(  CallStatement *s, bool& recur);
virtual void        visit(BranchStatement *s, bool& recur);
virtual void        visit(ReturnStatement *s, bool& recur);
virtual void        visit(ImpRefStatement *s, bool& recur);
virtual void        visit(  CaseStatement *s, bool& recur);
};
#endif

// Convert any exp{-} (with null definition) so that the definition points instead to an implicit assignment (exp{0})
// Note it is important to process refs in a depth first manner, so that e.g. m[sp{-}-8]{-} -> m[sp{0}-8]{-} first, so
// that there is never an implicit definition for m[sp{-}-8], only ever for m[sp{0}-8]
class ImplicitConverter : public ExpModifier {
    Cfg *m_cfg;

  public:
    ImplicitConverter(Cfg *cfg) : m_cfg(cfg) {}
    Exp *postVisit(RefExp *e);
};

class StmtImplicitConverter : public StmtModifier {
    Cfg *m_cfg;

  public:
    StmtImplicitConverter(ImplicitConverter *ic, Cfg *cfg)
        : StmtModifier(ic, false), // False to not ignore collectors (want to make sure that
          m_cfg(cfg) {}            //  collectors have valid expressions so you can ascendType)
    virtual void visit(PhiAssign *s, bool &recur);
};

class Localiser : public SimpExpModifier {
    CallStatement *call; // The call to localise to
  public:
    Localiser(CallStatement *c) : call(c) {}
    Exp *preVisit(RefExp *e, bool &recur);
    Exp *preVisit(Location *e, bool &recur);
    Exp *postVisit(Location *e);
    Exp *postVisit(Terminal *e);
};

class ComplexityFinder : public ExpVisitor {
    int count;
    UserProc *proc;

  public:
    ComplexityFinder(UserProc *p) : count(0), proc(p) {}
    int getDepth() { return count; }

    virtual bool visit(Unary *, bool &override);
    virtual bool visit(Binary *, bool &override);
    virtual bool visit(Ternary *, bool &override);
    virtual bool visit(Location *e, bool &override);
};

// Used by range analysis
class MemDepthFinder : public ExpVisitor {
    int depth;

  public:
    MemDepthFinder() : depth(0) {}
    virtual bool visit(Location *e, bool &override);
    int getDepth() { return depth; }
};

// A class to propagate everything, regardless, to this expression. Does not consider memory expressions and whether
// the address expression is primitive. Use with caution; mostly Statement::propagateTo() should be used.
class ExpPropagator : public SimpExpModifier {
    bool change;

  public:
    ExpPropagator() : change(false) {}
    bool isChanged() { return change; }
    void clearChanged() { change = false; }
    Exp *postVisit(RefExp *e);
};

// Test an address expression (operand of a memOf) for primitiveness (i.e. if it is possible to SSA rename the memOf
// without problems). Note that the PrimitiveTester is not used with the memOf expression, only its address expression
class PrimitiveTester : public ExpVisitor {
    bool result;

  public:
    PrimitiveTester() : result(true) {} // Initialise result true: need AND of all components
    bool getResult() { return result; }
    bool visit(Location *, bool &override);
    bool visit(RefExp *e, bool &override);
};

// Test if an expression (usually the RHS on an assignment) contains memory expressions. If so, it may not be safe to
// propagate the assignment. NO LONGER USED.
class ExpHasMemofTester : public ExpVisitor {
    bool result;
    UserProc *proc;

  public:
    ExpHasMemofTester(UserProc *p) : result(false), proc(p) {}
    bool getResult() { return result; }
    bool visit(Location *e, bool &override);
};

class TempToLocalMapper : public ExpVisitor {
    UserProc *proc; // Proc object for storing the symbols
  public:
    TempToLocalMapper(UserProc *p) : proc(p) {}
    bool visit(Location *e, bool &override);
};

// Name registers and temporaries
class ExpRegMapper : public ExpVisitor {
    UserProc *proc; // Proc object for storing the symbols
    Prog *prog;

  public:
    ExpRegMapper(UserProc *proc);
    bool visit(RefExp *e, bool &override);
};

class StmtRegMapper : public StmtExpVisitor {
  public:
    StmtRegMapper(ExpRegMapper *erm) : StmtExpVisitor(erm) {}
    virtual bool common(Assignment *stmt, bool &override);
    virtual bool visit(Assign *stmt, bool &override);
    virtual bool visit(PhiAssign *stmt, bool &override);
    virtual bool visit(ImplicitAssign *stmt, bool &override);
    virtual bool visit(BoolAssign *stmt, bool &override);
};

class ConstGlobalConverter : public ExpModifier {
    Prog *prog; // Pointer to the Prog object, for reading memory
  public:
    ConstGlobalConverter(Prog *pg) : prog(pg) {}
    virtual Exp *preVisit(RefExp *e, bool &recur);
};

// Count the number of times a reference expression is used. Increments the count multiple times if the same reference
// expression appears multiple times (so can't use UsedLocsFinder for this)
class ExpDestCounter : public ExpVisitor {
    std::map<Exp *, int, lessExpStar> &destCounts;

  public:
    ExpDestCounter(std::map<Exp *, int, lessExpStar> &dc) : destCounts(dc) {}
    bool visit(RefExp *e, bool &override);
};

// FIXME: do I need to count collectors? All the visitors and modifiers should be refactored to conditionally visit
// or modify collectors, or not
class StmtDestCounter : public StmtExpVisitor {
  public:
    StmtDestCounter(ExpDestCounter *edc) : StmtExpVisitor(edc) {}
    bool visit(PhiAssign *stmt, bool &override);
};

// Search an expression for flags calls, e.g. SETFFLAGS(...) & 0x45
class FlagsFinder : public ExpVisitor {
    bool found;

  public:
    FlagsFinder() : found(false) {}
    bool isFound() { return found; }

  private:
    virtual bool visit(Binary *e, bool &override);
};

// Search an expression for a bad memof (non subscripted or not linked with a symbol, i.e. local or parameter)
class BadMemofFinder : public ExpVisitor {
    bool found;
    UserProc *proc;

  public:
    BadMemofFinder(UserProc *p) : found(false), proc(p) {}
    bool isFound() { return found; }

  private:
    virtual bool visit(Location *e, bool &override);
    virtual bool visit(RefExp *e, bool &override);
};

class ExpCastInserter : public ExpModifier {
    UserProc *proc; // The enclising UserProc
  public:
    ExpCastInserter(UserProc *p) : proc(p) {}
    static void checkMemofType(Exp *memof, SharedType memofType);
    virtual Exp *postVisit(RefExp *e);
    virtual Exp *postVisit(Binary *e);
    virtual Exp *postVisit(Const *e);
    virtual Exp *preVisit(TypedExp *e, bool &recur) {
        recur = false;
        return e;
    } // Don't consider if already cast
};

class StmtCastInserter : public StmtVisitor {
    //      ExpCastInserter* ema;
  public:
    StmtCastInserter() {}
    bool common(Assignment *s);
    virtual bool visit(Assign *s);
    virtual bool visit(PhiAssign *s);
    virtual bool visit(ImplicitAssign *s);
    virtual bool visit(BoolAssign *s);
};

// Transform an exp by applying mappings to the subscripts. This used to be done by many Exp::fromSSAform() functions.
// Note that mappings have to be done depth first, so e.g. m[r28{0}-8]{22} -> m[esp-8]{22} first, otherwise there wil be
// a second implicit definition for m[esp{0}-8] (original should be b[esp+8] by now)
class ExpSsaXformer : public ExpModifier {
    UserProc *proc;

  public:
    ExpSsaXformer(UserProc *p) : proc(p) {}
    UserProc *getProc() { return proc; }

    virtual Exp *postVisit(RefExp *e);
};

class StmtSsaXformer : public StmtModifier {
    UserProc *proc;

  public:
    StmtSsaXformer(ExpSsaXformer *esx, UserProc *p) : StmtModifier(esx), proc(p) {}
    // virtual            ~StmtSsaXformer() {}
    void commonLhs(Assignment *s);
    // TODO: find out if recur should, or should not be set ?
    virtual void visit(Assign *s, bool &recur);
    virtual void visit(PhiAssign *s, bool &recur);
    virtual void visit(ImplicitAssign *s, bool &recur);
    virtual void visit(BoolAssign *s, bool &recur);
    virtual void visit(CallStatement *s, bool &recur);
};

#endif // #ifndef __VISITOR_H__
