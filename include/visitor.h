/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/*==============================================================================
 * FILE:	   visitor.h
 * OVERVIEW:   Provides the definition for the various visitor and modifier
 *			   classes. These classes sometimes are associated with Statement
 *			   and Exp classes, so they are here to avoid #include problems,
 *			   and also to make exp.cpp and statement.cpp a little less huge
 *============================================================================*/
/*
 * $Revision$
 *
 * We have Visitor and Modifier classes separate. Visitors are more suited
 *	 for searching: they have the capability of stopping the recursion,
 *	 but can't change the class of a top level expression. Modifiers always
 *	 recurse to the end, and the ExpModifiers' visit function returns an Exp*
 *	 so that the top level expression can change class (e.g. RefExp to Binary).
 * The accept() functions (in the target classes) are always the same for all
 *	 visitors; they encapsulate where the visitable parts of a Statement or
 *	 expression are.
 * The visit() functions contain the logic of the search/modify/whatever.
 *	 Often only a few visitor functions have to do anything. Unfortunately,
 *	 the visit functions are members of the Visitor (or Modifier) classes, and
 *	 so have to use public functions of the target classes.
 *
 * 14 Jun 04 - Mike: Created, from work started by Trent in 2003
 */

/*
 * The ExpVisitor class is used to iterate over all subexpressions in
 * an expression. It contains methods for each kind of subexpression found
 * and can be used to eliminate switch statements.
 */

#ifndef __VISITOR_H__
#define __VISITOR_H__

#ifndef NULL
#define NULL 0		// Often defined in stdio.h
#endif

#include "exp.h"

class Statement;
class Assignment;
class Assign;
class ImplicitAssign;
class BoolAssign;
class CaseStatement;
class CallStatement;
class ReturnStatement;
class GotoStatement;
class BranchStatement;

class RTL;
class UserProc;
class BasicBlock;
typedef BasicBlock* PBB;

class LocationSet;

class ExpVisitor {

public:
	ExpVisitor() { }
	virtual ~ExpVisitor() { }

	// visitor functions,
	// return false to abandon iterating through the expression (terminate the search)
	// Set override true to not do the usual recursion into children
virtual bool visit(Unary *e,	bool& override) {override = false; return true;}
virtual bool visit(Binary *e,	bool& override) {override = false; return true;}
virtual bool visit(Ternary *e,	bool& override) {override = false; return true;}
virtual bool visit(TypedExp *e, bool& override) {override = false; return true;}
virtual bool visit(FlagDef *e,	bool& override) {override = false; return true;}
virtual bool visit(RefExp *e,	bool& override) {override = false; return true;}
virtual bool visit(Location *e, bool& override) {override = false; return true;}
// These three have zero arity, so there is nothing to override
virtual bool visit(Const *e	  ) {return true;}
virtual bool visit(Terminal *e) {return true;}
virtual bool visit(TypeVal *e ) {return true;}
};

// This class visits subexpressions, and if a location, sets the UserProc
class FixProcVisitor : public ExpVisitor {
	// the enclosing UserProc (if a Location)
	UserProc* proc;

public:
		void setProc(UserProc* p) { proc = p; }
virtual bool visit(Location *e, bool& override);
	// All other virtual functions inherit from ExpVisitor, i.e. they just
	// visit their children recursively
};

// This class is more or less the opposite of the above. It finds a proc by
// visiting the whole expression if necessary
class GetProcVisitor : public ExpVisitor {
		UserProc* proc;			// The result (or NULL)

public:
				GetProcVisitor() {proc = NULL;}	// Constructor
	UserProc*	getProc() {return proc;}
virtual bool	visit(Location *e, bool& override);
	// All others inherit and visit their children
};

// This class visits subexpressions, and if a Const, sets or clears a new conscript
class SetConscripts : public ExpVisitor {
		int		curConscript;
		bool	bInLocalGlobal;		// True when inside a local or global
		bool	bClear;				// True when clearing, not setting
public:
				SetConscripts(int n, bool bClear) : bInLocalGlobal(false), bClear(bClear) {curConscript = n;}
		int		getLast() {return curConscript;}
virtual bool	visit(Const* e);
virtual bool	visit(Location* e, bool& override);
virtual bool	visit(Binary* b,	bool& override);
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
protected:
		bool	mod;		// Set if there is any change. Don't have to implement
public:
				ExpModifier() {mod = false;}
virtual 		~ExpModifier() { }
		bool	isMod() {return mod;}
		void	clearMod() {mod = false;}

		// visitor functions
		// Most times these won't be needed
		// Note: you only need to override the ones that make a cange.
		// preVisit comes before modifications to the children (if any)
virtual Exp*	preVisit(Unary		*e,	bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(Binary 	*e, bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(Ternary	*e, bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(TypedExp	*e, bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(FlagDef	*e, bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(RefExp		*e, bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(Location	*e, bool& recur) {recur = true; return e;}
virtual Exp*	preVisit(Const		*e			   ) {				return e;}
virtual Exp*	preVisit(Terminal	*e			   ) {				return e;}
virtual Exp*	preVisit(TypeVal	*e			   ) {				return e;}

		// postVisit comes after modifications to the children (if any)
virtual Exp*	postVisit(Unary		*e)	{return e;}
virtual Exp*	postVisit(Binary	*e)	{return e;}
virtual Exp*	postVisit(Ternary	*e)	{return e;}
virtual Exp*	postVisit(TypedExp	*e) {return e;}
virtual Exp*	postVisit(FlagDef	*e)	{return e;}
virtual Exp*	postVisit(RefExp	*e)	{return e;}
virtual Exp*	postVisit(Location	*e) {return e;}
virtual Exp*	postVisit(Const		*e)	{return e;}
virtual Exp*	postVisit(Terminal	*e) {return e;}
virtual Exp*	postVisit(TypeVal	*e)	{return e;}
};

/* 
 * The StmtVisitor class is used for code that has to work with all the Statement classes.
 * One advantage is that you don't need to declare a function in every class derived from
 * Statement: the accept methods already do that for you.
 * It does not automatically visit the expressions in the statement.
 */
class StmtVisitor {
public:
	StmtVisitor() { }
	virtual ~StmtVisitor() { }

	// visitor functions, 
	// returns true to continue iterating the container
virtual bool	visit(RTL				*rtl);	// By default, visits all statements
virtual bool	visit(Assign			*stmt)	{ return true;}
virtual bool	visit(PhiAssign			*stmt)	{ return true;}
virtual bool	visit(ImplicitAssign	*stmt)	{ return true;}
virtual bool	visit(BoolAssign		*stmt)	{ return true;}
virtual bool	visit(GotoStatement		*stmt)	{ return true;}
virtual bool	visit(BranchStatement	*stmt)	{ return true;}
virtual bool	visit(CaseStatement		*stmt)	{ return true;}
virtual bool	visit(CallStatement		*stmt)	{ return true;}
virtual bool	visit(ReturnStatement	*stmt)	{ return true;}
};

class StmtConscriptSetter : public StmtVisitor {
		int		curConscript;
		bool	bClear;
public:
				 StmtConscriptSetter(int n, bool bClear) : curConscript(n), bClear(bClear) {}
	int			 getLast() {return curConscript;}

virtual	bool	visit(Assign *stmt);
virtual	bool	visit(PhiAssign *stmt);
virtual bool	visit(ImplicitAssign *stmt);
virtual bool	visit(BoolAssign *stmt);
virtual bool	visit(CaseStatement *stmt);
virtual bool	visit(CallStatement *stmt);
virtual bool	visit(ReturnStatement *stmt);
virtual bool	visit(BranchStatement *stmt);
};

// StmtExpVisitor is a visitor of statements, and of expressions within
// those expressions. The visiting of expressions is done by an ExpVisitor.
class StmtExpVisitor {
public:
	ExpVisitor*	 ev;
				 StmtExpVisitor(ExpVisitor* v) {
					ev = v;}
virtual			~StmtExpVisitor() {}
virtual bool	visit(         Assign *stmt, bool& override) {override = false; return true;}
virtual bool	visit(      PhiAssign *stmt, bool& override) {override = false; return true;}
virtual bool	visit( ImplicitAssign *stmt, bool& override) {override = false; return true;}
virtual bool	visit(     BoolAssign *stmt, bool& override) {override = false; return true;}
virtual bool	visit(  GotoStatement *stmt, bool& override) {override = false; return true;}
virtual bool	visit(BranchStatement *stmt, bool& override) {override = false; return true;}
virtual bool	visit(  CaseStatement *stmt, bool& override) {override = false; return true;}
virtual bool	visit(  CallStatement *stmt, bool& override) {override = false; return true;}
virtual bool	visit(ReturnStatement *stmt, bool& override) {override = false; return true;}
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
	ExpModifier* mod;			// The expression modifier object
				StmtModifier(ExpModifier* em) {mod = em;}	// Constructor
	virtual		~StmtModifier() {}
	// This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
virtual void visit(Assign *s,			bool& recur) {recur = true;}
virtual void visit(PhiAssign *s,		bool& recur) {recur = true;}
virtual void visit(ImplicitAssign *s,	bool& recur) {recur = true;}
virtual void visit(BoolAssign *s,		bool& recur) {recur = true;}
virtual void visit(GotoStatement *s,	bool& recur) {recur = true;}
virtual void visit(BranchStatement *s,	bool& recur) {recur = true;}
virtual void visit(CaseStatement *s,	bool& recur) {recur = true;}
virtual void visit(CallStatement *s,	bool& recur) {recur = true;}
virtual void visit(ReturnStatement *s,	bool& recur) {recur = true;}
};

class PhiStripper : public StmtModifier {
		bool	del;			// Set true if this statment is to be deleted
public:
				PhiStripper(ExpModifier* em) : StmtModifier(em) {del = false;} 
virtual void	visit(PhiAssign* stmt, bool& recur);
		bool	getDelete() {return del;}
};

// This class visits subexpressions, strips references
class RefStripper : public ExpModifier {
public:
				RefStripper() {}
virtual Exp*	preVisit(RefExp* ei, bool& recur);
		// All other virtual functions inherit and do nothing
};

class CallRefsFixer : public ExpModifier {
		// These two provide 31 bits (or sizeof(int)-1) of information about whether
		// the child is unchanged. If the mask overflows, it goes to zero, and
		// from then on the child is reported as always changing.
		// This is used to avoid calling simplify in most cases where it is not
		// necessary.
	unsigned	mask;
	unsigned	unchanged;
public:
			 	CallRefsFixer() { mask = 1; unchanged = (unsigned)-1;}
virtual Exp*	preVisit(Unary		*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(Binary		*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(Ternary	*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(TypedExp	*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(FlagDef	*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(RefExp		*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(Location	*e, bool& recur) { recur = true; mask <<= 1; return e;}
virtual Exp*	preVisit(Const		*e)	{ mask <<= 1; return e;}
virtual Exp*	preVisit(Terminal	*e)	{ mask <<= 1; return e;}
virtual Exp*	preVisit(TypeVal	*e)	{ mask <<= 1; return e;}

virtual Exp*	postVisit(Unary *e);
virtual Exp*	postVisit(Binary *e);
virtual Exp*	postVisit(Ternary *e);
virtual Exp*	postVisit(TypedExp *e);
virtual Exp*	postVisit(FlagDef *e);
virtual Exp*	postVisit(RefExp *e);
virtual Exp*	postVisit(Location *e);
virtual Exp*	postVisit(Const *e);
virtual Exp*	postVisit(Terminal *e);
virtual Exp*	postVisit(TypeVal *e);
};

class UsedLocsFinder : public ExpVisitor {
	LocationSet* used;
public:
				UsedLocsFinder(LocationSet& used) {this->used = &used;}
				~UsedLocsFinder() {}

		LocationSet* getLocSet() {return used;}
virtual bool	visit(RefExp *e,	bool& override);
virtual bool	visit(Location *e, bool& override);
virtual bool	visit(Terminal* e);
};

class UsedLocsVisitor : public StmtExpVisitor {
public:
		// The bool final is for ignoring those parts of expressions that are
		// not to be considered for the final pass just before code generation.
		// For example, implicit parameters and returns in CallStatements are
		// ignored (not considered used).
		bool	final;
				UsedLocsVisitor(ExpVisitor* v, bool f = false) : StmtExpVisitor(v) {final = f;}
virtual			~UsedLocsVisitor() {}
		// Needs special attention because the lhs of an assignment isn't used
		// (except where it's m[blah], when blah is used)
virtual bool visit(		   Assign *stmt, bool& override);
virtual bool visit(		PhiAssign *stmt, bool& override);
virtual bool visit(ImplicitAssign *stmt, bool& override);
		// A BoolAssign uses its condition expression, but not its destination
		// (unless it's an m[x], in which case x is used and not m[x])
virtual bool visit(BoolAssign *stmt, bool& override);
		// Returns aren't used (again, except where m[blah] where blah is used),
		// and there is special logic for when the pass is final
virtual bool visit(CallStatement *stmt, bool& override);
		// Only consider the first return when final
virtual bool visit(ReturnStatement *stmt, bool& override);
};

class ExpSubscripter : public ExpModifier {
		Exp*	search;
		Statement* def;
public:
				ExpSubscripter(Exp* s, Statement* d) : search(s), def(d) { }
virtual Exp*	preVisit(Location *e, bool& recur);
virtual Exp*	preVisit(Terminal *e);
virtual Exp*	preVisit(RefExp *e,   bool& recur);
};

class StmtSubscripter : public StmtModifier {
public:
				StmtSubscripter(ExpModifier* em) : StmtModifier(em) {}
virtual			~StmtSubscripter() {}

virtual void	visit(		  Assign *s, bool& recur);
virtual void	visit(	   PhiAssign *s, bool& recur);
virtual void	visit(ImplicitAssign *s, bool& recur);
virtual void	visit(    BoolAssign *s, bool& recur);
virtual void	visit( CallStatement *s, bool& recur);
};

class SizeStripper : public ExpModifier {
public:
				SizeStripper() {}
virtual			~SizeStripper() {}

virtual Exp*	preVisit(Binary *b,   bool& recur);
};

class ExpConstCaster: public ExpModifier {
		int		num;
		Type*	ty;
		bool	changed;
public:
				ExpConstCaster(int num, Type* ty)
				  : num(num), ty(ty), changed(false) {}
virtual			~ExpConstCaster() {}
		bool	isChanged() {return changed;}

virtual Exp* 	preVisit(Const *c);
};

class ConstFinder : public ExpVisitor {
		std::list<Const*>& lc;
public:
				ConstFinder(std::list<Const*>& lc) : lc(lc) {}
virtual			~ConstFinder() {}

virtual bool	visit(Const *e);
virtual bool	visit(Location *e, bool& override);
};

class StmtConstFinder : public StmtExpVisitor {
public:
				StmtConstFinder(ConstFinder* v) : StmtExpVisitor(v) {}
};

class DfaLocalConverter : public ExpModifier {
		Type*	parentType;
		UserProc* proc;
		Prog*	prog;
		Signature* sig;		// Look up once (from proc) for speed
		int		sp;			// Look up the stack pointer register once
public:
				DfaLocalConverter(UserProc* proc);
		void	setType(Type* ty) {parentType = ty;}
		//Type*	getType() {return parentType;}

		Exp*	preVisit(Location* e, bool& recur);
		Exp*	postVisit(Location* e);
		Exp*	preVisit(Binary* e, bool& recur);
};

class StmtDfaLocalConverter : public StmtModifier {
public:
				StmtDfaLocalConverter(ExpModifier* em) : StmtModifier(em) {}

virtual void	visit(		   Assign *s, bool& recur);
virtual void	visit(	    PhiAssign *s, bool& recur);
virtual void	visit( ImplicitAssign *s, bool& recur);
virtual void	visit(     BoolAssign *s, bool& recur);
virtual void	visit(  CallStatement *s, bool& recur);
virtual void	visit(BranchStatement *s, bool& recur);
virtual void	visit(ReturnStatement *s, bool& recur);
};

// Convert any exp{0} with null definition so that the definition points instead to an implicit assignment
class ImplicitConverter : public ExpModifier {
		Cfg* cfg;
public:
				ImplicitConverter(Cfg* cfg) : cfg(cfg) { }
		Exp*	postVisit(RefExp* e);
};

#endif	// #ifndef __VISITOR_H__
