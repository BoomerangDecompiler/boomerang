/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:       visitor.h
 * OVERVIEW:   Provides the definition for the various visitor and modifier
 *             classes. These classes sometimes are associated with Statement
 *             and Exp classes, so they are here to avoid #include problems,
 *             and also to make exp.cpp and statement.cpp a little less huge
 *============================================================================*/
/*
 * $Revision$
 *
 * We have Visitor and Modifier classes separate. Visitors are more suited
 *   for searching: they have the capability of stopping the recursion,
 *   but can't change the class of a top level expression. Modifiers always
 *   recurse to the end, and the ExpModifiers' visit function returns an Exp*
 *   so that the top level expression can change class (e.g. RefExp to Binary).
 * The accept() functions (in the target classes) are always the same for all
 *   visitors; they encapsulate where the visitable parts of a Statement or
 *   expression are.
 * The visit() functions contain the logic of the search/modify/whatever.
 *   Often only a few visitor functions have to do anything. Unfortunately,
 *   the visit functions are members of the Visitor (or Modifier) classes, and
 *   so have to use public functions of the target classes.
 *
 * 14 Jun 04 - Mike: Created, from work started by Trent in 2003
 */

/*
 * The ExpVisitor class is used to iterate over all subexpressions in
 * an expression. It contains methods for each kind of subexpression found
 * in an and can be used to eliminate switch statements.
 */

#ifndef __VISITOR_H__
#define __VISITOR_H__

#ifndef NULL
#define NULL 0      // Often defined in stdio.h
#endif

#if 1
#include "exp.h"
#else
class Exp;
class Unary;
class Binary;
class Ternary;
class RefExp;
class PhiExp;
class Terminal;
class Const;
class TypedExp;
class FlagDef;
class Location;
class TypeVal;
#endif

class Statement;
class Assign;
class CaseStatement;
class CallStatement;
class ReturnStatement;
class GotoStatement;
class BoolStatement;
class BranchStatement;

class RTL;
class UserProc;
class BasicBlock;
typedef BasicBlock* PBB;


class ExpVisitor {

public:
    ExpVisitor() { }
    virtual ~ExpVisitor() { }

    // visitor functions,
    // return true to continue iterating through the expression
    // Note: you only need to override the ones that "do something"
    virtual bool visit(Unary *e)    {return true;}
    virtual bool visit(Binary *e)   {return true;}
    virtual bool visit(Ternary *e)  {return true;}
    virtual bool visit(TypedExp *e) {return true;}
    virtual bool visit(FlagDef *e)  {return true;}
    virtual bool visit(RefExp *e)   {return true;}
    virtual bool visit(PhiExp *e)   {return true;}
    virtual bool visit(Location *e) {return true;}
    virtual bool visit(Const *e)    {return true;}
    virtual bool visit(Terminal *e) {return true;}
    virtual bool visit(TypeVal *e)  {return true;}
};

// This class visits subexpressions, and if a location, sets the UserProc
class FixProcVisitor : public ExpVisitor {
    // the enclosing UserProc (if a Location)
    UserProc* proc;

public:
    void setProc(UserProc* p) { proc = p; }
    virtual bool visit(Location *e);
    // All other virtual functions inherit from ExpVisitor, i.e. they just
    // visit their children recursively
};

// This class is more or less the opposite of the above. It finds a proc by
// visiting the whole expression if necessary
class GetProcVisitor : public ExpVisitor {
    UserProc* proc;         // The result (or NULL)

public:
                 GetProcVisitor() {proc = NULL;}    // Constructor
    UserProc*    getProc() {return proc;}
    virtual bool visit(Location *e);
    // All others inherit and visit their children
};

// This class visits subexpressions, and if a Const, sets a new conscript
class SetConscripts : public ExpVisitor {
    int     curConscript;
    bool    bInLocalGlobal;     // True when inside a local or global
public:
            SetConscripts(int n) : bInLocalGlobal(false)
                {curConscript = n;}
    int     getLast() {return curConscript;}
    virtual bool visit(Const* e);
    virtual bool visit(Location* e);
    // All other virtual functions inherit from ExpVisitor: return true
};

/*
 * The ExpModifier class is used to iterate over all subexpressions in
 * an expression. It contains methods for each kind of subexpression found
 * in an and can be used to eliminate switch statements.
 * It is a little more expensive to use than ExpVisitor, but can make changes
 * to the expression
 */
class ExpModifier {

public:
    ExpModifier() { }
    virtual ~ExpModifier() { }

    // visitor functions
    // Most times these won't be needed
    // Note: you only need to override the ones that make a cange.
    virtual Exp* visit(Unary *e)    {return e;}
    virtual Exp* visit(Binary *e)   {return e;}
    virtual Exp* visit(Ternary *e)  {return e;}
    virtual Exp* visit(TypedExp *e) {return e;}
    virtual Exp* visit(FlagDef *e)  {return e;}
    virtual Exp* visit(RefExp *e)   {return e;}
    virtual Exp* visit(PhiExp *e)   {return e;}
    virtual Exp* visit(Location *e) {return e;}
    virtual Exp* visit(Const *e)    {return e;}
    virtual Exp* visit(Terminal *e) {return e;}
    virtual Exp* visit(TypeVal *e)  {return e;}
};

/* 
 * The StmtVisitor class is used to iterate over all stmts in a basic 
 * block. It contains methods for each kind of Statement found in an
 * RTL and can be used to eliminate switch statements.
 */
class StmtVisitor {
private:
    // the enclosing basic block
    PBB pBB;

public:
    StmtVisitor() { pBB = NULL; }
    virtual ~StmtVisitor() { }

    // allows the container being iteratorated over to identify itself
    PBB getBasicBlock() { return pBB; }
    void setBasicBlock(PBB bb) { pBB = bb; }

    // visitor functions, 
    // returns true to continue iterating the container
    virtual bool visit(RTL *rtl);   // By default, visits all statements
    virtual bool visit(Assign *stmt) = 0;
    virtual bool visit(GotoStatement *stmt) = 0;
    virtual bool visit(BranchStatement *stmt) = 0;
    virtual bool visit(CaseStatement *stmt) = 0;
    virtual bool visit(CallStatement *stmt) = 0;
    virtual bool visit(ReturnStatement *stmt) = 0;
    virtual bool visit(BoolStatement *stmt) = 0;
};

class StmtSetConscripts : public StmtVisitor {
    int     curConscript;
public:
                 StmtSetConscripts(int n) {curConscript = n;}
    int          getLast() {return curConscript;}

    virtual bool visit(Assign *stmt);
    virtual bool visit(GotoStatement *stmt) {return true;}
    virtual bool visit(BranchStatement *stmt) {return true;}
    virtual bool visit(CaseStatement *stmt);
    virtual bool visit(CallStatement *stmt);
    virtual bool visit(ReturnStatement *stmt);
    virtual bool visit(BoolStatement *stmt);
};

// StmtModifier is a class that visits all statements in an RTL, and for
// all expressions in the various types of statements, makes a modification.
// The modification is as a result of an ExpModifier; there is a pointer to
// such an ExpModifier in a StmtModifier
// Classes that derive from StmtModifier inherit the code (in the accept member
// functions) to modify all the expressions in the various types of statement.
// Because there is nothing specialised about a StmtModifier, it is not an
// abstract class (can be instantiated).
class StmtModifier {
public:
    ExpModifier* mod;           // The expression modifier object
                StmtModifier(ExpModifier* em) {mod = em;}   // Constructor
    virtual     ~StmtModifier() {}
    // This class' visitor functions don't return anything. Maybe we'll need
    // return values at a later stage.
    virtual void visit(Assign *stmt)         {};
    virtual void visit(GotoStatement *stmt)  {};
    virtual void visit(BranchStatement *stmt){};
    virtual void visit(CaseStatement *stmt)  {};
    virtual void visit(CallStatement *stmt)  {};
    virtual void visit(ReturnStatement *stmt){};
    virtual void visit(BoolStatement *stmt)  {};
};

class StripPhis : public StmtModifier {
    bool    del;            // Set true if this statment is to be deleted
public:
                 StripPhis(ExpModifier* em) : StmtModifier(em) {del = false;} 
    virtual void visit(Assign* stmt);
    bool    getDelete() {return del;}
};

// This class visits subexpressions, strips references
class StripRefs : public ExpModifier {
public:
            StripRefs() {}
    virtual Exp* visit(RefExp* e);
    // All other virtual functions inherit and do nothing
};

#endif  // #ifndef __VISIOR_H__
