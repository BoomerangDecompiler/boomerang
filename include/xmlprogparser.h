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

class XMLProgParser
{
    public:
	XMLProgParser() { }
	Prog *parse(const char *filename);
	void persistToXML(Prog *prog);
	void handleElementStart(const char *el, const char **attr);
	void handleElementEnd(const char *el);

    protected:

	void start_prog(const char **attr);
	void addToContext_prog(Context *c, int e);
	void start_cluster(const char **attr);
	void addToContext_cluster(Context *c, int e);
	void start_libproc(const char **attr);
	void addToContext_libproc(Context *c, int e);
	void start_userproc(const char **attr);
	void addToContext_userproc(Context *c, int e);
	void start_proven(const char **attr);
	void addToContext_proven(Context *c, int e);
	void start_callee(const char **attr);
	void addToContext_callee(Context *c, int e);
	void start_caller(const char **attr);
	void addToContext_caller(Context *c, int e);
	void start_signature(const char **attr);
	void addToContext_signature(Context *c, int e);
	void start_param(const char **attr);
	void addToContext_param(Context *c, int e);
	void start_implicitParam(const char **attr);
	void addToContext_implicitParam(Context *c, int e);
	void start_return(const char **attr);
	void addToContext_return(Context *c, int e);
	void start_rettype(const char **attr);
	void addToContext_rettype(Context *c, int e);
	void start_cfg(const char **attr);
	void addToContext_cfg(Context *c, int e);
	void start_bb(const char **attr);
	void addToContext_bb(Context *c, int e);
	void start_inedge(const char **attr);
	void addToContext_inedge(Context *c, int e);
	void start_outedge(const char **attr);
	void addToContext_outedge(Context *c, int e);
	void start_livein(const char **attr);
	void addToContext_livein(Context *c, int e);
	void start_rtl(const char **attr);
	void addToContext_rtl(Context *c, int e);
	void start_stmt(const char **attr);
	void addToContext_stmt(Context *c, int e);
	void start_assign(const char **attr);
	void addToContext_assign(Context *c, int e);
	void start_callstmt(const char **attr);
	void addToContext_callstmt(Context *c, int e);
	void start_dest(const char **attr);
	void start_returnstmt(const char **attr);
	void addToContext_returnstmt(Context *c, int e);
	void start_gotostmt(const char **attr);
	void addToContext_gotostmt(Context *c, int e);
	void start_branchstmt(const char **attr);
	void addToContext_branchstmt(Context *c, int e);
	void start_type(const char **attr);
	void addToContext_type(Context *c, int e);
	void start_exp(const char **attr);
	void addToContext_exp(Context *c, int e);
	void start_voidType(const char **attr);
	void addToContext_voidType(Context *c, int e);
	void start_integerType(const char **attr);
	void addToContext_integerType(Context *c, int e);
	void start_pointerType(const char **attr);
	void addToContext_pointerType(Context *c, int e);
	void start_charType(const char **attr);
	void addToContext_charType(Context *c, int e);
	void start_location(const char **attr);
	void addToContext_location(Context *c, int e);
	void start_unary(const char **attr);
	void addToContext_unary(Context *c, int e);
	void start_binary(const char **attr);
	void addToContext_binary(Context *c, int e);
	void start_ternary(const char **attr);
	void addToContext_ternary(Context *c, int e);
	void start_const(const char **attr);
	void addToContext_const(Context *c, int e);
	void start_terminal(const char **attr);
	void addToContext_terminal(Context *c, int e);

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

	static const char *tags[];
	static void (XMLProgParser::*start_procs[])(const char**);
	static void (XMLProgParser::*end_procs[])(Context *c, int e);

	int operFromString(const char *s);
	const char *getAttr(const char **attr, const char *name);

	std::list<Context*> stack;
	std::map<int, void*> idToX;
	int phase;

	void addId(const char **attr, void *x);
	void *findId(const char *id);
};

#endif
