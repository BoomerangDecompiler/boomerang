/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       chllcode.cpp
 * OVERVIEW:   Concrete class for the "C" high level language
 *             This class is provides methods which are specific for the C
 *             language binding.  I guess this will be the most popular output
 *             language unless we do C++.
 *============================================================================*/

/*
 * $Revision$
 * 20 Jun 02 - Trent: Quick and dirty implementation for debugging
 * 28 Jun 02 - Trent: Starting to look better
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "dataflow.h"
#include "cfg.h"
#include "exp.h"
#include "proc.h"
#include "prog.h"
#include "hllcode.h"
#include "chllcode.h"
#include "signature.h"

#include <sstream>

extern char *operStrings[];

CHLLCode::CHLLCode() : HLLCode()
{	
}

CHLLCode::CHLLCode(UserProc *p) : HLLCode(p)
{	
}

CHLLCode::~CHLLCode()
{
}

void CHLLCode::indent(char *str, int indLevel)
{
    memset(str, ' ', indLevel * 4);
    str[indLevel * 4] = 0;
}

void CHLLCode::appendExp(char *str, Exp *exp)
{
    if (exp == NULL) return;

    char s[1024];
    Const   *c = (Const*)exp;
    Unary   *u = (Unary*)exp;
    Binary  *b = (Binary*)exp;
    Ternary *t = (Ternary*)exp;
    
    switch(exp->getOper()) {
        case opIntConst:
            sprintf(s, "%d", c->getInt());
            strcat(str, s);
            break;
        case opFltConst:
            sprintf(s, "%f", c->getFlt());
            strcat(str, s);
            break;
        case opStrConst:
            sprintf(s, "\"%s\"", c->getStr());
            strcat(str, s);
            break;
        case opAddrOf:
            strcat(str, "&");
            appendExp(str, u->getSubExp1());
            break;
        case opParam:
        case opGlobal:
        case opLocal:
            c = dynamic_cast<Const*>(u->getSubExp1());
            assert(c && c->getOper() == opStrConst);
            strcat(str, c->getStr());
            break;
        case opEquals:
            appendExp(str, b->getSubExp1());
            strcat(str, " == ");
            appendExp(str, b->getSubExp2());
            break;
        case opNotEqual:
            appendExp(str, b->getSubExp1());
            strcat(str, " != ");
            appendExp(str, b->getSubExp2());
            break;
        case opLess:
        case opLessUns:
            appendExp(str, b->getSubExp1());
            strcat(str, " < ");
            appendExp(str, b->getSubExp2());
            break;
        case opGtr:
        case opGtrUns:
            appendExp(str, b->getSubExp1());
            strcat(str, " > ");
            appendExp(str, b->getSubExp2());
            break;
        case opLessEq:
        case opLessEqUns:
            appendExp(str, b->getSubExp1());
            strcat(str, " <= ");
            appendExp(str, b->getSubExp2());
            break;
        case opGtrEq:
        case opGtrEqUns:
            appendExp(str, b->getSubExp1());
            strcat(str, " >= ");
            appendExp(str, b->getSubExp2());
            break;
        case opAnd:
            appendExp(str, b->getSubExp1());
            strcat(str, " && ");
            appendExp(str, b->getSubExp2());
            break;
        case opOr:
            appendExp(str, b->getSubExp1());
            strcat(str, " || ");
            appendExp(str, b->getSubExp2());
            break;
        case opBitAnd:
            appendExp(str, b->getSubExp1());
            strcat(str, " & ");
            appendExp(str, b->getSubExp2());
            break;
        case opBitOr:
            appendExp(str, b->getSubExp1());
            strcat(str, " | ");
            appendExp(str, b->getSubExp2());
            break;
        case opBitXor:
            appendExp(str, b->getSubExp1());
            strcat(str, " ^ ");
            appendExp(str, b->getSubExp2());
            break;
        case opNot:
            strcat(str, "~(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opLNot:
            strcat(str, "!(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opNeg:
            strcat(str, "-(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opAt:
        {
            strcat(str, "(((");
            appendExp(str, t->getSubExp1());
            strcat(str, ")");
            c = dynamic_cast<Const*>(t->getSubExp3());
            assert(c && c->getOper() == opIntConst);
            int last = c->getInt();
            sprintf(s, ">>%d)", last);
            strcat(str, s);
            c = dynamic_cast<Const*>(t->getSubExp2());
            assert(c && c->getOper() == opIntConst);
            unsigned int mask = (1 << (c->getInt() - last + 1)) - 1;
            sprintf(s, "&0x%x)", mask);
            strcat(str, s);
            break;
        }
        case opPlus:
            appendExp(str, b->getSubExp1());
            strcat(str, " + ");
            appendExp(str, b->getSubExp2());
            break;
        case opMinus:
            appendExp(str, b->getSubExp1());
            strcat(str, " - ");
            appendExp(str, b->getSubExp2());
            break;
        case opAssignExp:
            appendExp(str, b->getSubExp1());
            strcat(str, " = ");
            appendExp(str, b->getSubExp2());
            break;
        case opMemOf:
            strcat(str, "*(int*)(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opRegOf:
            strcat(str, "r[");
            appendExp(str, u->getSubExp1());
            strcat(str, "]");
            break;
        case opTemp:
            strcat(str, "tmp");
            break;
        case opMult:
        case opDiv:
        case opFPlus:
        case opFMinus:
        case opFMult:
        case opFDiv:
        case opFPlusd:
        case opFMinusd:
        case opFMultd:
        case opFDivd:
        case opFPlusq:
        case opFMinusq:
        case opFMultq:
        case opFDivq:
        case opFMultsd:
        case opFMultdq:
        case opSQRTs:
        case opSQRTd:
        case opSQRTq:
        case opMults:
        case opDivs:
        case opMod:
        case opMods:
        case opSignExt:
        case opShiftL:
        case opShiftR:
        case opShiftRA:
        case opRotateL:
        case opRotateR:
        case opRotateLC:
        case opRotateRC:
        case opTargetInst:
        case opTypedExp:
        case opNamedExp:
        case opGuard:
        case opTern:
        case opVar:
        case opArg:
        case opExpand:
        case opSize:
        case opCastIntStar:
        case opPostVar:
        case opTruncu:
        case opTruncs:
        case opZfill:
        case opSgnEx:
        case opFsize:
        case opItof:
        case opFtoi:
        case opFround:
        case opForceInt:
        case opForceFlt:
        case opFpush:
        case opFpop:
        case opSin:
        case opCos:
        case opTan:
        case opArcTan:
        case opLog2:
        case opLog10:
        case opLoge:
        case opSqrt:
        case opExecute:
        case opCodeAddr:
        case opPC:
        case opAFP:
        case opAGP:
            // not implemented
            std::cerr << "not implemented " << operStrings[exp->getOper()] << 
                std::endl;
            assert(false);
            break;
        default:
            // others
            std::cerr << "not implemented " << operStrings[exp->getOper()] << 
                std::endl;
            assert(false);
    }
}

void CHLLCode::appendType(char *str, Type *typ)
{
    if (typ == NULL) return;
    // TODO: decode types
    strcat(str, typ->getCtype());
}

void CHLLCode::reset()
{
    for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
         it++) delete *it;
    lines.clear();
}

void CHLLCode::AddPretestedLoopHeader(int indLevel, Exp *cond)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "while (");
    appendExp(s, cond);
    strcat(s, ") {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddPretestedLoopEnd(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "}");
    lines.push_back(strdup(s));
}

void CHLLCode::AddEndlessLoopHeader(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "for (;;) {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddEndlessLoopEnd(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "}");
    lines.push_back(strdup(s));
}

void CHLLCode::AddPosttestedLoopHeader(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "do {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddPosttestedLoopEnd(int indLevel, Exp *cond)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "} while (");
    appendExp(s, cond);
    strcat(s, ");");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCaseCondHeader(int indLevel, Exp *cond)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "switch(");
    appendExp(s, cond);
    strcat(s, ") {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCaseCondOption(int indLevel, Exp *opt)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "case ");
    appendExp(s, opt);
    strcat(s, ":");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCaseCondOptionEnd(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "break;");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCaseCondElse(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "default:");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCaseCondEnd(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "}");
    lines.push_back(strdup(s));
}

void CHLLCode::AddIfCondHeader(int indLevel, Exp *cond)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "if (");
    appendExp(s, cond);
    strcat(s, ") {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddIfCondEnd(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "}");
    lines.push_back(strdup(s));
}

void CHLLCode::AddIfElseCondHeader(int indLevel, Exp *cond)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "if (");
    appendExp(s, cond);
    strcat(s, ") {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddIfElseCondOption(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "} else {");
    lines.push_back(strdup(s));
}

void CHLLCode::AddIfElseCondEnd(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "}");
    lines.push_back(strdup(s));
}

void CHLLCode::AddGoto(int indLevel, int ord)
{
    char s[1024];
    indent(s, indLevel);
    sprintf(s + strlen(s), "goto L%d;", ord);
    lines.push_back(strdup(s));
}

void CHLLCode::AddContinue(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "continue;");
    lines.push_back(strdup(s));
}

void CHLLCode::AddBreak(int indLevel)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "break;");
    lines.push_back(strdup(s));
}

void CHLLCode::AddLabel(int indLevel, int ord)
{
    char s[1024];
    sprintf(s, "L%d:", ord);
    lines.push_back(strdup(s));
}

void CHLLCode::RemoveLabel(int ord)
{
    char s[1024];
    sprintf(s, "L%d:", ord);
    for (std::list<char*>::iterator it = lines.begin();
         it != lines.end(); it++)
        if (!strcmp(*it, s)) {
            delete *it;
            lines.erase(it);
            break;
        }
}

void CHLLCode::AddAssignmentStatement(int indLevel, AssignExp *exp)
{
    Exp *match;
    // hack
    if (exp->getSubExp1()->getOper() == opFlags ||
        exp->search(new Terminal(opPC), match) ||
        (exp->getSubExp1()->getOper() == opRegOf &&
        exp->getSubExp1()->getSubExp1()->getOper() == opTemp))
        return;
    char s[1024];
    indent(s, indLevel);
    appendExp(s, exp);
    strcat(s, ";");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCallStatement(int indLevel, Exp *retloc, Proc *proc, 
    std::vector<Exp*> &args)
{
    char s[1024];
    indent(s, indLevel);
    if (retloc) {
        appendExp(s, retloc);
        strcat(s, " = ");
    }
    strcat(s, proc->getName());
    strcat(s, "(");
    for (unsigned int i = 0; i < args.size(); i++) {
        appendExp(s, args[i]);
        if (i < args.size() - 1) strcat(s, ", ");
    }
    strcat(s, ");");
    lines.push_back(strdup(s));
}

void CHLLCode::AddReturnStatement(int indLevel, Exp *ret)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "return ");
    appendExp(s, ret);
    strcat(s, ";");
    lines.push_back(strdup(s));
}

void CHLLCode::AddProcStart(Signature *signature)
{
    char s[1024];
    s[0] = 0;
    appendType(s, signature->getReturnType());
    strcat(s, " ");
    strcat(s, signature->getName());
    strcat(s, "(");
    for (int i = 0; i < signature->getNumParams(); i++) {
        appendType(s, signature->getParamType(i));
        strcat(s, " ");
        strcat(s, signature->getParamName(i));
        if (i != signature->getNumParams() - 1)
            strcat(s, ", ");
    }
    strcat(s, ")");
    lines.push_back(strdup(s));
    lines.push_back(strdup("{"));
}

void CHLLCode::AddProcEnd()
{
    lines.push_back(strdup("}"));
}

void CHLLCode::AddLocal(const char *name, Type *type)
{
    char s[1024];
    s[0] = 0;
    appendType(s, type);
    strcat(s, " ");
    strcat(s, name);
    strcat(s, ";");
    lines.push_back(strdup(s));
}

void CHLLCode::print(std::ostream &os)
{
    for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
         it++) os << *it << std::endl;
}


