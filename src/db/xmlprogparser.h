#pragma once

/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
* \file        XMLProgParser.h
* OVERVIEW:    parses persisted XML output and creates a new prog.
******************************************************************************/

#include <string>
#include <list>
#include <map>
#include <memory>

class Global;
class Module;
class Prog;
class Function;
class UserProc;
class LibProc;
class Signature;
class InstructionSet;
class Module;
class Context;
class Type;
class Exp;
class Cfg;
class BasicBlock;
class RTL;
class Instruction;
class XMLProgParser;
class QString;
class QStringList;
class QXmlStreamAttributes;
class QXmlStreamReader;
class QXmlStreamWriter;
class QStringRef;
typedef std::shared_ptr<Type> SharedType;
#define MAX_STACK    1024

typedef struct
{
	const char *tag;
	void (XMLProgParser::*start_proc)(const QXmlStreamAttributes&);
	void (XMLProgParser::*end_proc)(Context *c, int e);
} XMLTag;

class XMLProgParser
{
public:
	XMLProgParser() {}
	Prog *parse(const QString& filename);
	void persistToXML(Prog *prog);
	void handleElementStart(QXmlStreamReader& strm);
	void handleElementEnd(const QXmlStreamReader& el);

protected:
	void parseFile(const QString& filename);
	void parseChildren(Module *c);

#define TAGD(x)											\
	void start_ ## x(const QXmlStreamAttributes &attr);	\
	void addToContext_ ## x(Context * c, int e);

	TAGD(prog)
	TAGD(procs)
	TAGD(global)
	TAGD(cluster)
	TAGD(libproc)
	TAGD(userproc)
	TAGD(local)
	TAGD(symbol)
	TAGD(secondexp)
	TAGD(proven_true)
	TAGD(callee)
	TAGD(caller)
	TAGD(defines)
	TAGD(signature)
	TAGD(param)
	TAGD(return )
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
	TAGD(assignment)
	TAGD(phiassign)
	TAGD(lhs)
	TAGD(rhs)
	TAGD(callstmt)
	TAGD(dest)
	TAGD(argument)
	TAGD(returnexp)
	TAGD(returntype)
	TAGD(returnstmt)
	TAGD(returns)
	TAGD(modifieds)
	TAGD(gotostmt)
	TAGD(branchstmt)
	TAGD(cond)
	TAGD(casestmt)
	TAGD(boolasgn)
	TAGD(type)
	TAGD(exp)
	TAGD(voidtype)
	TAGD(integertype)
	TAGD(pointertype)
	TAGD(chartype)
	TAGD(namedtype)
	TAGD(arraytype)
	TAGD(basetype)
	TAGD(sizetype)
	TAGD(location)
	TAGD(unary)
	TAGD(binary)
	TAGD(ternary)
	TAGD(const)
	TAGD(terminal)
	TAGD(typedexp)
	TAGD(refexp)
	TAGD(def)
	TAGD(subexp1)
	TAGD(subexp2)
	TAGD(subexp3)

	void persistToXML(QXmlStreamWriter& out, Global *g);
	void persistToXML(QXmlStreamWriter& out, Module *c);
	void persistToXML(QXmlStreamWriter& out, Function *proc);
	void persistToXML(QXmlStreamWriter& out, LibProc *proc);
	void persistToXML(QXmlStreamWriter& out, UserProc *proc);
	void persistToXML(QXmlStreamWriter& out, Signature *sig);
	void persistToXML(QXmlStreamWriter& out, const SharedType& ty);
	void persistToXML(QXmlStreamWriter& out, const SharedExp& e);
	void persistToXML(QXmlStreamWriter& out, Cfg *cfg);
	void persistToXML(QXmlStreamWriter& out, const BasicBlock *bb);
	void persistToXML(QXmlStreamWriter& out, const RTL *rtl);
	void persistToXML(QXmlStreamWriter& out, const Instruction *stmt);

	static XMLTag tags[];

	int operFromString(const QStringRef& s);
	const char *getAttr(const QXmlStreamAttributes& attr, const char *name);

	std::list<Context *> stack;
	std::map<int, void *> idToX;
	std::map<int, SharedType> idToType;
	int phase;

	void addId(const QXmlStreamAttributes& attr, void *x);
	void addType(const QXmlStreamAttributes& attr, SharedType x);
	void *findId(const QStringRef& id);
	void *findId(const char *id);
	SharedType findType(const QStringRef& id);
};
