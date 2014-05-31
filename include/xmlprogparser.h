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
 *============================================================================*/
#ifndef _XMLPROGPARSER_H_
#define _XMLPROGPARSER_H_

#include <string>
#include <list>
#include <map>

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
class Statement;
class XMLProgParser;
class QString;
class QStringList;
class QXmlStreamAttributes;
class QXmlStreamReader;
class QXmlStreamWriter;
class QStringRef;
#define MAX_STACK 1024

typedef struct {
    const char *tag;
    void (XMLProgParser::*start_proc)(const QXmlStreamAttributes &);
    void (XMLProgParser::*end_proc)(Context *c, int e);
} _tag;

class XMLProgParser {
  public:
    XMLProgParser() {}
    Prog *parse(const QString &filename);
    void persistToXML(Prog *prog);
    void handleElementStart(QXmlStreamReader &strm);
    void handleElementEnd(const QXmlStreamReader &el);

protected:
    void parseFile(const QString &filename);
    void parseChildren(Cluster *c);

#define TAGD(x)                                                                                                        \
    void start_##x(const QXmlStreamAttributes &attr);                                                                                 \
    void addToContext_##x(Context *c, int e);

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

    void persistToXML(QXmlStreamWriter &out, Global *g);
    void persistToXML(QXmlStreamWriter &out, Cluster *c);
    void persistToXML(QXmlStreamWriter &out, Proc *proc);
    void persistToXML(QXmlStreamWriter &out, LibProc *proc);
    void persistToXML(QXmlStreamWriter &out, UserProc *proc);
    void persistToXML(QXmlStreamWriter &out, Signature *sig);
    void persistToXML(QXmlStreamWriter &out, const Type *ty);
    void persistToXML(QXmlStreamWriter &out, const Exp *e);
    void persistToXML(QXmlStreamWriter &out, Cfg *cfg);
    void persistToXML(QXmlStreamWriter &out, const BasicBlock *bb);
    void persistToXML(QXmlStreamWriter &out, const RTL *rtl);
    void persistToXML(QXmlStreamWriter &out, const Statement *stmt);

    static _tag tags[];

    int operFromString(const QStringRef &s);
    const char *getAttr(const QXmlStreamAttributes &attr, const char *name);

    std::list<Context *> stack;
    std::map<int, void *> idToX;
    int phase;

    void addId(const QXmlStreamAttributes &attr, void *x);
    void *findId(const QStringRef &id);
    void *findId(const char *id);
};

#endif
