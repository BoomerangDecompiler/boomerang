/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:        XMLProgParser.h
 * OVERVIEW:    parses persisted XML output and creates a new prog.
 *============================================================================*/
/*
 * $Revision$
 * 13 Jun 04 - Trent: Created.
 */

#ifndef _XMLPROGPARSER_H_
#define _XMLPROGPARSER_H_

class Global;
class Cluster;
class Prog;
class Proc;
class UserProc;
class LibProc;
class Signature;
class StatementSet;
class Cluster;
class Context;
class Type;
class Exp;
class Cfg;
class BasicBlock;
class RTL;

#define MAX_STACK 1024

typedef struct {
    const char *tag;
    void (XMLProgParser::*start_proc)(const char**);
    void (XMLProgParser::*end_proc)(Context *c, int e);
} _tag;

class XMLProgParser
{
    public:
	XMLProgParser() { }
	Prog *parse(const char *filename);
	void persistToXML(Prog *prog);
	void handleElementStart(const char *el, const char **attr);
	void handleElementEnd(const char *el);

    protected:

	void parseFile(const char *filename);
	void parseChildren(Cluster *c);

#define TAGD(x) void start_ ## x (const char **attr); \
	void addToContext_ ## x (Context *c, int e);

	TAGD(prog) 
	TAGD(procs) 
	TAGD(global) 
	TAGD(cluster) 
	TAGD(libproc) 
	TAGD(userproc) 
	TAGD(local) 
	TAGD(symbol) 
	TAGD(secondexp)
	TAGD(proven) 
	TAGD(callee) 
	TAGD(caller) 
	TAGD(defines)
	TAGD(signature) 
	TAGD(param) 
	TAGD(implicitparam) 
	TAGD(return) 
	TAGD(rettype) 
	TAGD(prefparam) 
	TAGD(prefreturn) 
	TAGD(cfg) 
	TAGD(bb) 
	TAGD(inedge) 
	TAGD(outedge) 
	TAGD(livein) 
	TAGD(order) 
	TAGD(revorder)
	TAGD(rtl) 
	TAGD(stmt) 
	TAGD(assign) 
	TAGD(lhs) 
	TAGD(rhs) 
	TAGD(callstmt) 
	TAGD(dest) 
	TAGD(argument) 
	TAGD(implicitarg) 
	TAGD(returnexp)
	TAGD(returnstmt)
	TAGD(gotostmt) 
	TAGD(branchstmt) 
	TAGD(cond)
	TAGD(casestmt)
	TAGD(boolstmt)
	TAGD(type) 
	TAGD(exp) 
	TAGD(voidtype) 
	TAGD(integertype) 
	TAGD(pointertype) 
	TAGD(chartype) 
	TAGD(namedtype) 
	TAGD(arraytype)
	TAGD(basetype)
	TAGD(location) 
	TAGD(unary) 
	TAGD(binary) 
	TAGD(ternary) 
	TAGD(const) 
	TAGD(terminal) 
	TAGD(typedexp) 
	TAGD(refexp)
	TAGD(phiexp)
	TAGD(def)
	TAGD(subexp1) 
	TAGD(subexp2) 
	TAGD(subexp3)	

	void persistToXML(std::ostream &out, Global *g);
	void persistToXML(std::ostream &out, Cluster *c);
	void persistToXML(std::ostream &out, Proc *proc);
	void persistToXML(std::ostream &out, LibProc *proc);
	void persistToXML(std::ostream &out, UserProc *proc);
	void persistToXML(std::ostream &out, Signature *sig);
	void persistToXML(std::ostream &out, Type *ty);
	void persistToXML(std::ostream &out, Exp *e);
	void persistToXML(std::ostream &out, Cfg *cfg);
	void persistToXML(std::ostream &out, BasicBlock *bb);
	void persistToXML(std::ostream &out, RTL *rtl);
	void persistToXML(std::ostream &out, Statement *stmt);

	static _tag tags[];

	int operFromString(const char *s);
	const char *getAttr(const char **attr, const char *name);

	std::list<Context*> stack;
	std::map<int, void*> idToX;
	int phase;

	void addId(const char **attr, void *x);
	void *findId(const char *id);
};

#endif
