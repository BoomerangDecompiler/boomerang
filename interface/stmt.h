/*==============================================================================
 * FILE:       stmt.h
 * OVERVIEW:   Provides the definition for the Stmt class and its 
 *             subclasses.
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Trent: Created
 */

#ifndef __STMT_H_
#define __STMT_H_

#include <fstream>      // For ostream, cout etc
#include <stdio.h>      // For sprintf
#include <list>
#include "scope.h"

class Exp;
class Func;

/*============================================================================== 
 * Stmt is a statement class, it hold all information about a statement.  
 * There are many types of statements and they all derive from this base 
 * class, however this base class is not abstract, you can instantiate it
 * to insert a "null" statement.
 *============================================================================*/
class Stmt {
   Func *func;
public:
   Stmt() { }
   Stmt(Func *f) : func(f) { }
   Func *getFunc() { return func; }
   void setFunc(Func *f) { func = f; }
   void    print(ostream& os = cout);
};

/*============================================================================== 
 * ExpStmt is the most common kind of statement.  It evaluates to something
 * and the result is thrown away.  Hopefully some side effect has occured, like
 * assigning to an identifier or calling a function.
 *============================================================================*/
class ExpStmt : public Stmt {
   Exp *exp;
public:
   ExpStmt(Func *f, Exp *e) : Stmt(f), exp(e) { }
   Exp *getExp() { return exp; }
   void setExp(Exp *e) { exp = e; }
};

/*============================================================================== 
 * BlockStmt is a list of statements with a scope.  It contains all the other
 * statements and may be a part of other statements.
 *============================================================================*/
class BlockStmt : public Stmt, public Scope {
   list<Stmt *> *stmts;
public:
   // About a million constructors cause optional parameters just dont work, and 
   // no, I dont wanna hear about any << patterns ok?
   BlockStmt() : Stmt(), Scope(), stmts(NULL) { }
   BlockStmt(list<Stmt *> *ls) : Stmt(), Scope(), stmts(ls) { }
   BlockStmt(list<Decl *> *ld) : Stmt(), Scope(ld), stmts(NULL) { }
   BlockStmt(list<Decl *> *ld, list<Stmt *> *ls) : Stmt(), Scope(ld), stmts(ls) { }
   BlockStmt(Func *f) : Stmt(f) { }
   BlockStmt(Func *f, list<Stmt *> *ls) : Stmt(f), Scope(), stmts(ls) { }
   BlockStmt(Func *f, list<Decl *> *ld) : Stmt(f), Scope(ld), stmts(NULL) { }
   BlockStmt(Func *f, list<Decl *> *ld, list<Stmt *> *ls) : Stmt(f), Scope(ld), stmts(ls) { }

   list<Stmt *> *getStmts() { return stmts; }
   void addStmt(Stmt *s) { stmts->push_back(s); }
};

/*============================================================================== 
 * IfStmt is a statement that attaches a condition to a statement.  If the
 * condition evaluates true the statement receives control.
 *============================================================================*/
class IfStmt : public Stmt {
   Stmt *body;
public:
   IfStmt(Func *f, Stmt *b) : Stmt(f), body(b) { }
};

/*============================================================================== 
 * IfElseStmt is a statement that attaches a condition to two statements.  If 
 * the condition evaluates true the first statement receives control otherwise
 * the second statement receives control.
 *============================================================================*/
class IfElseStmt : public Stmt {
   Stmt *trueBody, *falseBody;
public:
   IfElseStmt(Func *f, Stmt *bt, Stmt *bf) : Stmt(f), trueBody(bt), falseBody(bf) { }
};

/*============================================================================== 
 * LoopStmt is a class of statements that transfer control repeatably in a 
 * program. They can be conditional, nonconditional, pretested, post-tested or
 * have more complex semantics like the for statement in C.
 *============================================================================*/
class LoopStmt : public Stmt {
   Stmt *body;
public:
   LoopStmt(Func *f, Stmt *b) : Stmt(f), body(b) { }
   Stmt *getPreHeader();
   Stmt *getHeader();
   Stmt *getExit();
};

class CondLoopStmt : public LoopStmt {
   Exp *condition;
   bool pretested;
public:
   CondLoopStmt(Func *f, Stmt *b, Exp *cond, bool pre) : LoopStmt(f, b), condition(cond), pretested(pre) { }
   Exp *getCondition() { return condition; }
   void setCondition(Exp *cond) { condition = cond; }
   bool isPreTested() { return pretested; }
   bool isPostTested() { return !pretested; }
   void setPreTested(bool pre) { pretested = pre; }
};

class ForStmt : public LoopStmt {
   Exp *init;
   Exp *condition;
   Exp *iteration;
public:
   ForStmt(Func *f, Stmt *b, Exp *i, Exp *c, Exp *it) : LoopStmt(f, b), init(i), condition(c), iteration(it) { }
   Exp *getInit() { return init; }
   Exp *getCondition() { return condition; }
   Exp *getIteration() { return iteration; }
   void setInit(Exp *i) { init = i; }
   void setCondiiton(Exp *c) { condition = c; }
   void setIteration(Exp *i) { iteration = i; }
};

/*============================================================================== 
 * JumpStmt is a class of statements that transfer control in a program to 
 * some specified destination.  They include break, continue, return and goto
 * statements in C.
 *============================================================================*/
class JumpStmt : public Stmt {
   Stmt *dest;
public:
   JumpStmt(Func *f, Stmt *d) : Stmt(f), dest(d) { }
   Stmt *getDest() { return dest; }
   void setDest(Stmt *d) { dest = d; }
};

class BreakStmt : public JumpStmt {
   LoopStmt *loop;
public:
   BreakStmt(Func *f, LoopStmt *l) : JumpStmt(f, l->getExit()), loop(l) { }
   LoopStmt *getLoop() { return loop; }
   void setLoop(LoopStmt *l) { setDest(l->getExit()); loop = l; }
};

class ContinueStmt : public JumpStmt {
   LoopStmt *loop;
public:
   ContinueStmt(Func *f, LoopStmt *l) : JumpStmt(f, l->getHeader()), loop(l) { }
   LoopStmt *getLoop() { return loop; }
   void setLoop(LoopStmt *l) { setDest(l->getHeader()); loop = l; }
};

class ReturnStmt : public JumpStmt {
   Exp *exp;
public:
   ReturnStmt(Func *f, Exp *e) : JumpStmt(f), exp(e) { }
   Exp *getExp() { return exp; }
   void setExp(Exp *e) { exp = e; }
};

class GotoStmt : public JumpStmt {
   Exp *ident;
public:
   GotoStmt(Func *f, Exp *e) : JumpStmt(f), ident(e) { }
   Exp *getIdent() { return exp; }
   void setIdent(Exp *e) { ident = e; }
};
