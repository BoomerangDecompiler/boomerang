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
 * 22 May 03 - Mike: delete -> free() to keep valgrind happy
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "statement.h"
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
        case opLongConst:
            sprintf(s, "%lld", c->getLong());
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
        case opMemOf:
            strcat(str, "*(int*)(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opRegOf:
            {
                if (u->getSubExp1()->getOper() == opTemp) {
                    // The great debate: r[tmpb] vs tmpb
                    strcat(str, "tmp");
                    break;
                }
                assert(u->getSubExp1()->getOper() == opIntConst);
                const char *n = m_proc->getProg()->getRegName(
                                    ((Const*)u->getSubExp1())->getInt());
                if (n)
                    strcat(str, n);
                else {
                    strcat(str, "r[");
                    appendExp(str, u->getSubExp1());
                    strcat(str, "]");
                }
            }
            break;
        case opTemp:
            strcat(str, "tmp");
            break;
        case opItof:
            strcat(str, "(float)(");
            appendExp(str, t->getSubExp3());
            strcat(str, ")");
            break;
        case opFsize:
            // needs work
            appendExp(str, t->getSubExp3());
            break;
        case opMult:
        case opMults:       // FIXME: check types
            appendExp(str, b->getSubExp1());
            strcat(str, " * ");
            appendExp(str, b->getSubExp2());
            break;
        case opDiv:
        case opDivs:        // FIXME: check types
            appendExp(str, b->getSubExp1());
            strcat(str, " / ");
            appendExp(str, b->getSubExp2());
            break;
        case opMod:
        case opMods:        // Fixme: check types
            appendExp(str, b->getSubExp1());
            strcat(str, " % ");
            appendExp(str, b->getSubExp2());
            break;
        case opShiftL:
            appendExp(str, b->getSubExp1());
            strcat(str, " << ");
            appendExp(str, b->getSubExp2());
            break;
        case opShiftR:
        case opShiftRA:
            appendExp(str, b->getSubExp1());
            strcat(str, " >> ");
            appendExp(str, b->getSubExp2());
            break;
        case opTern:
            appendExp(str, t->getSubExp1());
            strcat(str, " ? ");
            appendExp(str, t->getSubExp2());
            strcat(str, " : ");
            appendExp(str, t->getSubExp3());
            break;
        case opFPlus:
        case opFPlusd:
        case opFPlusq:
            appendExp(str, b->getSubExp1());
            strcat(str, " + ");
            appendExp(str, b->getSubExp2());
            break;
        case opFMinus:
        case opFMinusd:
        case opFMinusq:
            appendExp(str, b->getSubExp1());
            strcat(str, " - ");
            appendExp(str, b->getSubExp2());
            break;
        case opFMult:
        case opFMultd:
        case opFMultq:
            appendExp(str, b->getSubExp1());
            strcat(str, " * ");
            appendExp(str, b->getSubExp2());
            break;
        case opFDiv:
        case opFDivd:
        case opFDivq:
            appendExp(str, b->getSubExp1());
            strcat(str, " / ");
            appendExp(str, b->getSubExp2());
            break;
        case opFround:
            strcat(str, "fround(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opFMultsd:
        case opFMultdq:
        case opSQRTs:
        case opSQRTd:
        case opSQRTq:
        case opSignExt:
        case opRotateL:
        case opRotateR:
        case opRotateLC:
        case opRotateRC:
        case opTargetInst:
        case opNamedExp:
        case opGuard:
        case opVar:
        case opArg:
        case opExpand:
        case opSize:
        case opCastIntStar:
        case opPostVar:
        case opFtoi:
        case opForceInt:
        case opForceFlt:
        case opFpush:
        case opFpop:
        case opSin:
        case opCos:
        case opTan:
        case opArcTan:
        case opLoge:
        case opSqrt:
        case opExecute:
        case opCodeAddr:
        case opAFP:
        case opAGP:
            // not implemented
            std::cerr << "not implemented " << operStrings[exp->getOper()] << 
                std::endl;
            assert(false);
            break;
        case opFlagCall:
            {
                assert(b->getSubExp1()->getOper() == opStrConst);
                strcat(str, ((Const*)b->getSubExp1())->getStr());
                strcat(str, "(");
                Binary *l = (Binary*)b->getSubExp2();
                for (; l && l->getOper() == opList; 
                     l = (Binary*)l->getSubExp2()) {
                    appendExp(str, l->getSubExp1());
                    if (l->getSubExp2()->getOper() == opList)
                        strcat(str, ", ");
                }
                strcat(str, ")");
            } 
            break;
        case opFlags:
            strcat(str, "%flags");    
            break;
        case opPC:
            strcat(str, "%pc");
            break;
        case opZfill:
            // MVE: this is a temporary hack... needs cast?
            //sprintf(s, "/* zfill %d->%d */ ",
            //  ((Const*)t->getSubExp1())->getInt(),
            //  ((Const*)t->getSubExp2())->getInt());
            //strcat(str, s); */
            appendExp(str, t->getSubExp3());
            break;
        case opTypedExp:
            if (u->getSubExp1()->getOper() == opTypedExp &&
                *((TypedExp*)u)->getType() ==
                *((TypedExp*)u->getSubExp1())->getType()) {
                appendExp(str, u->getSubExp1());
            } else if (u->getSubExp1()->getOper() == opMemOf) {
                strcat(str, "*(");
                appendType(str, ((TypedExp*)u)->getType());
                strcat(str, "*)(");
                appendExp(str, u->getSubExp1()->getSubExp1());
                strcat(str, ")");
            } else {
                strcat(str, "(");
                appendType(str, ((TypedExp*)u)->getType());
                strcat(str, ")(");
                appendExp(str, u->getSubExp1());
                strcat(str, ")");
            }
            break;
        case opSgnEx: {
            strcat(str, "/* opSgnEx */ (int) ");
            Exp* s = t->getSubExp3();
            appendExp(str, s);
            break;
        }
        case opTruncu:
        case opTruncs: {
            strcat(str, "/* opTruncs/u */ (int) ");
            Exp* s = t->getSubExp3();
            appendExp(str, s);
            break;
        }
        case opMachFtr: {
            strcat(str, "/* machine specific */ (int) ");
            Exp* sub = u->getSubExp1();
            assert(sub->isStrConst());
            char* s = ((Const*)sub)->getStr();
            if (s[0] == '%')        // e.g. %Y
                strcat(str, s+1);   // Just use Y
            else
                strcat(str, s);
            break;
        }
        case opPow:
            strcat(str, "pow(");
            appendExp(str, b->getSubExp1());
            strcat(str, ", ");
            appendExp(str, b->getSubExp2());
            strcat(str, ")");
            break;
        case opLog2:
            strcat(str, "log2(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opLog10:
            strcat(str, "log10(");
            appendExp(str, u->getSubExp1());
            strcat(str, ")");
            break;
        case opSubscript:
            appendExp(str, u->getSubExp1());
            std::cerr << "subscript in code generation of proc " << m_proc->getName() << " exp (without subscript): " << str << std::endl;
            assert(false);
            break;
        case opMemberAccess:
            if (b->getSubExp1()->getOper() == opGlobal) {
                const char *nam = ((Const*)b->getSubExp1()->getSubExp1())->
                                              getStr();
                Type *ty = m_proc->getProg()->getGlobalType((char*)nam);
                if (ty) {
                    if (ty->isNamed())
                        ty = ((NamedType*)ty)->resolvesTo();
                    if (ty->isPointer()) {
                        appendExp(str, b->getSubExp1());
                        strcat(str, "->");
                        strcat(str, ((Const*)b->getSubExp2())->getStr());
                    } else {
                        assert(ty->isCompound());
                        appendExp(str, b->getSubExp1());
                        strcat(str, ".");
                        strcat(str, ((Const*)b->getSubExp2())->getStr());
                    }
                }
            } else {
                appendExp(str, b->getSubExp1());
                strcat(str, ".");
                strcat(str, ((Const*)b->getSubExp2())->getStr());
            }
            break;
        case opArraySubscript:
            appendExp(str, b->getSubExp1());
            strcat(str, "[");
            appendExp(str, b->getSubExp2());
            strcat(str, "]");
            break;
        default:
            // others
            OPER op = exp->getOper();
            if (op >= opZF) {
                // Machine flags; can occasionally be manipulated individually
                // Chop off the "op" part
                strcat(str, operStrings[op]+2);
                break;
            }
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
    lines.push_back(strdup(s));     // See below
}

void CHLLCode::RemoveLabel(int ord)
{
    char s[1024];
    sprintf(s, "L%d:", ord);
    for (std::list<char*>::iterator it = lines.begin();
         it != lines.end(); it++)
        if (!strcmp(*it, s)) {
            free (*it);             // Note: allocated in strdup (above)
            lines.erase(it);
            break;
        }
}

void CHLLCode::AddAssignmentStatement(int indLevel, Assign *asgn)
{
    char s[1024];
    indent(s, indLevel);
    if (asgn->getLeft()->getOper() == opMemOf && asgn->getSize() != 32) 
        appendExp(s, new TypedExp(new IntegerType(asgn->getSize()), asgn->getLeft()));
    else
        appendExp(s, asgn->getLeft());
    strcat(s, " = ");
    appendExp(s, asgn->getRight());
    strcat(s, ";");
    lines.push_back(strdup(s));
}

void CHLLCode::AddCallStatement(int indLevel, Proc *proc, 
    std::vector<Exp*> &args, LocationSet &defs)
{
    char s[1024];
    indent(s, indLevel);
    if (defs.size() >= 1) {
        LocationSet::iterator it = defs.begin();
        appendExp(s, (Exp*)*it);
        strcat(s, " = ");
        defs.remove((Exp*)*it);
    }
    strcat(s, proc->getName());
    strcat(s, "(");
    for (unsigned int i = 0; i < args.size(); i++) {
        Type *t = proc->getSignature()->getParamType(i);
        bool ok = true;
        if (t && t->isPointer() && ((PointerType*)t)->getPointsTo()->isFunc() 
              && args[i]->isIntConst()) {
            Proc *p = proc->getProg()->findProc(((Const*)args[i])->getAddr());
            if (p) {
                strcat(s, p->getName());
                ok = false;
            }
        }
        if (ok)
            appendExp(s, args[i]);
        if (i < args.size() - 1) strcat(s, ", ");
    }
    strcat(s, ");");
    LocationSet::iterator it = defs.begin();
    if (it != defs.end()) {
        strcat(s, " // OUT: ");
    }
    for (; it != defs.end(); it++) {
        appendExp(s, *it);
        strcat(s, ", ");
    }
    if (s[strlen(s)-1] == ' ' && s[strlen(s)-2] == ',')
        s[strlen(s)-2] = 0;
    lines.push_back(strdup(s));
}

// Ugh - almost the same as the above, but it needs to take an expression,
// not a Proc*
void CHLLCode::AddIndCallStatement(int indLevel, Exp *exp,
    std::vector<Exp*> &args)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "(*");
    appendExp(s, exp);
    strcat(s, ")(");
    for (unsigned int i = 0; i < args.size(); i++) {
        appendExp(s, args[i]);
        if (i < args.size() - 1) strcat(s, ", ");
    }
    strcat(s, ");");
    lines.push_back(strdup(s));
}


void CHLLCode::AddReturnStatement(int indLevel, std::vector<Exp*> &returns)
{
    char s[1024];
    indent(s, indLevel);
    strcat(s, "return");
    if (returns.size() >= 1) {
        strcat(s, " ");
        appendExp(s, returns[0]);
    }
    strcat(s, ";");
    if (returns.size() > 1) {
        strcat(s, "/* ");
    }
    for (unsigned i = 1; i < returns.size(); i++) {
        if (i != 1)
            strcat(s, ", ");
        appendExp(s, returns[i]);
    }
    if (returns.size() > 1) {
        strcat(s, "*/");
    }
    lines.push_back(strdup(s));
}

void CHLLCode::AddProcStart(Signature *signature)
{
    char s[1024];
    s[0] = 0;
    if (signature->getNumReturns() == 0) {
        strcat(s, "void");
    }  else 
        appendType(s, signature->getReturnType(0));
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
    lines.push_back("");
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
    locals[name] = type->clone();
}

void CHLLCode::AddGlobal(const char *name, Type *type, Exp *init)
{
    char s[1024];
    s[0] = 0;
    appendType(s, type);
    strcat(s, " ");
    strcat(s, name);
    if (init) {
        strcat(s, " = ");
        appendExp(s, init);
    }
    strcat(s, ";");
    lines.push_back(strdup(s));
}

void CHLLCode::print(std::ostream &os)
{
    for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
         it++) os << *it << std::endl;
    if (m_proc == NULL)
        os << std::endl;
}

void CHLLCode::AddLineComment(char* cmt) {
    char s[1024];
    s[0] = '/'; s[1] = '*';
    strcat(&s[2], cmt);
    strcat(s, "*/");
    lines.push_back(strdup(s));
}


