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
 * 16 Apr 04 - Mike: char[] replaced by ostringstreams
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
#include "boomerang.h"
#include "type.h"
// For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "rtl.h"
#endif

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

void CHLLCode::indent(std::ostringstream& str, int indLevel)
{
    // Can probably do more efficiently
    for (int i=0; i < indLevel*4; i++)
        str << ' ';
}

void CHLLCode::appendExp(std::ostringstream& str, Exp *exp)
{
    if (exp == NULL) return;

    Const   *c = (Const*)exp;
    Unary   *u = (Unary*)exp;
    Binary  *b = (Binary*)exp;
    Ternary *t = (Ternary*)exp;
    
    switch(exp->getOper()) {
        case opIntConst:
            str << std::dec << c->getInt(); break;
        case opLongConst:
            // sprintf(s, "%lld", c->getLong());
            //strcat(str, s);
// Check that this works!
            str << std::dec << c->getLong(); break;
        case opFltConst:
            str << c->getFlt(); break;
        case opStrConst:
            str << "\"" << c->getStr() << "\""; break;
        case opFuncConst:
            str << c->getFuncName(); break;
        case opAddrOf: {
            Exp* sub = u->getSubExp1();
            if (sub->isGlobal()) {
                Prog* prog = m_proc->getProg();
                Const* con = (Const*)((Unary*)sub)->getSubExp1();
                if (prog->getGlobalType(con->getStr())->isArray())
                    // Special C requirement: don't emit "&" for address of
                    // an array
                    break;
            }
            str << "&";
            appendExp(str, sub);
            break;
        }
        case opParam:
        case opGlobal:
        case opLocal:
            c = dynamic_cast<Const*>(u->getSubExp1());
            assert(c && c->getOper() == opStrConst);
            str << c->getStr();
            break;
        case opEquals:
            appendExp(str, b->getSubExp1());
            str << " == ";
            appendExp(str, b->getSubExp2());
            break;
        case opNotEqual:
            appendExp(str, b->getSubExp1());
            str << " != ";
            appendExp(str, b->getSubExp2());
            break;
        case opLess:
        case opLessUns:
            appendExp(str, b->getSubExp1());
            str << " < ";
            appendExp(str, b->getSubExp2());
            break;
        case opGtr:
        case opGtrUns:
            appendExp(str, b->getSubExp1());
            str << " > ";
            appendExp(str, b->getSubExp2());
            break;
        case opLessEq:
        case opLessEqUns:
            appendExp(str, b->getSubExp1());
            str << " <= ";
            appendExp(str, b->getSubExp2());
            break;
        case opGtrEq:
        case opGtrEqUns:
            appendExp(str, b->getSubExp1());
            str << " >= ";
            appendExp(str, b->getSubExp2());
            break;
        case opAnd:
            appendExp(str, b->getSubExp1());
            str << " && ";
            appendExp(str, b->getSubExp2());
            break;
        case opOr:
            appendExp(str, b->getSubExp1());
            str << " || ";
            appendExp(str, b->getSubExp2());
            break;
        case opBitAnd:
            appendExp(str, b->getSubExp1());
            str << " & ";
            appendExp(str, b->getSubExp2());
            break;
        case opBitOr:
            appendExp(str, b->getSubExp1());
            str << " | ";
            appendExp(str, b->getSubExp2());
            break;
        case opBitXor:
            appendExp(str, b->getSubExp1());
            str << " ^ ";
            appendExp(str, b->getSubExp2());
            break;
        case opNot:
            str << "~(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opLNot:
            str << "!(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opNeg:
        case opFNeg:
            str << "-(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opAt:
        {
            str << "(((";
            appendExp(str, t->getSubExp1());
            str << ")";
            c = dynamic_cast<Const*>(t->getSubExp3());
            assert(c && c->getOper() == opIntConst);
            int last = c->getInt();
            str << ">>" << std::dec << last; 
            c = dynamic_cast<Const*>(t->getSubExp2());
            assert(c && c->getOper() == opIntConst);
            unsigned int mask = (1 << (c->getInt() - last + 1)) - 1;
            str << "&0x" << std::hex << mask;
            break;
        }
        case opPlus:
            appendExp(str, b->getSubExp1());
            str << " + ";
            appendExp(str, b->getSubExp2());
            break;
        case opMinus:
            appendExp(str, b->getSubExp1());
            str << " - ";
            appendExp(str, b->getSubExp2());
            break;
        case opMemOf:
            if (u->getSubExp1()->getType()) {
                Exp *l = u->getSubExp1();
                Type *ty = l->getType();
                if (ty->isPointer()) {
                    str << "*";
                    appendExp(str, l);
                    break;
                }
                str << "*(";
                appendType(str, ty);
                str << "*)(";
                appendExp(str, l);
                str << ")";
                break;
            }
            str << "*(int*)(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opRegOf:
            {
                if (u->getSubExp1()->getOper() == opTemp) {
                    // The great debate: r[tmpb] vs tmpb
                    str << "tmp";
                    break;
                }
                assert(u->getSubExp1()->getOper() == opIntConst);
                const char *n = m_proc->getProg()->getRegName(
                                    ((Const*)u->getSubExp1())->getInt());
                if (n)
                    str << n;
                else {
// What is this doing in the back end???
                    str << "r[";
                    appendExp(str, u->getSubExp1());
                    str << "]";
                }
            }
            break;
        case opTemp:
            str << "tmp";
// Doesn't this need a name as well?
            break;
        case opItof:
            str << "(float)(";
            appendExp(str, t->getSubExp3());
            str << ")";
            break;
        case opFsize:
   // needs work!
            appendExp(str, t->getSubExp3());
            break;
        case opMult:
        case opMults:       // FIXME: check types
            appendExp(str, b->getSubExp1());
            str << " * ";
            appendExp(str, b->getSubExp2());
            break;
        case opDiv:
        case opDivs:        // FIXME: check types
            appendExp(str, b->getSubExp1());
            str << " / ";
            appendExp(str, b->getSubExp2());
            break;
        case opMod:
        case opMods:        // Fixme: check types
            appendExp(str, b->getSubExp1());
            str << " % ";
            appendExp(str, b->getSubExp2());
            break;
        case opShiftL:
            appendExp(str, b->getSubExp1());
            str << " << ";
            appendExp(str, b->getSubExp2());
            break;
        case opShiftR:
        case opShiftRA:
            appendExp(str, b->getSubExp1());
            str << " >> ";
            appendExp(str, b->getSubExp2());
            break;
        case opTern:
            appendExp(str, t->getSubExp1());
            str << " ? ";
            appendExp(str, t->getSubExp2());
            str << " : ";
            appendExp(str, t->getSubExp3());
            break;
        case opFPlus:
        case opFPlusd:
        case opFPlusq:
            appendExp(str, b->getSubExp1());
            str << " + ";
            appendExp(str, b->getSubExp2());
            break;
        case opFMinus:
        case opFMinusd:
        case opFMinusq:
            appendExp(str, b->getSubExp1());
            str << " - ";
            appendExp(str, b->getSubExp2());
            break;
        case opFMult:
        case opFMultd:
        case opFMultq:
            appendExp(str, b->getSubExp1());
            str << " * ";
            appendExp(str, b->getSubExp2());
            break;
        case opFDiv:
        case opFDivd:
        case opFDivq:
            appendExp(str, b->getSubExp1());
            str << " / ";
            appendExp(str, b->getSubExp2());
            break;
        case opFround:
            str << "fround(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opFtrunc:
            str << "ftrunc(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opFabs:
            str << "fabs(";
            appendExp(str, u->getSubExp1());
            str << ")";
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
        case opLoge:
        case opSqrt:
        case opExecute:
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
                str << ((Const*)b->getSubExp1())->getStr();
                str << "(";
                Binary *l = (Binary*)b->getSubExp2();
                for (; l && l->getOper() == opList; 
                     l = (Binary*)l->getSubExp2()) {
                    appendExp(str, l->getSubExp1());
                    if (l->getSubExp2()->getOper() == opList)
                        str << ", ";
                }
                str << ")";
            } 
            break;
        case opFlags:
            str << "%flags"; break;
        case opPC:
            str << "%pc"; break;
            break;
        case opZfill:
            // MVE: this is a temporary hack... needs cast?
            //sprintf(s, "/* zfill %d->%d */ ",
            //  ((Const*)t->getSubExp1())->getInt(),
            //  ((Const*)t->getSubExp2())->getInt());
            //strcat(str, s); */
            str << "(";
            appendExp(str, t->getSubExp3());
            str << ")";
            break;
        case opTypedExp:
            if (u->getSubExp1()->getOper() == opTypedExp &&
                *((TypedExp*)u)->getType() ==
                *((TypedExp*)u->getSubExp1())->getType()) {
                appendExp(str, u->getSubExp1());
            } else if (u->getSubExp1()->getOper() == opMemOf) {
                str << "*(";
                appendType(str, ((TypedExp*)u)->getType());
                str << "*)(";
                appendExp(str, u->getSubExp1()->getSubExp1());
                str << ")";
            } else {
                str << "(";
                appendType(str, ((TypedExp*)u)->getType());
                str << ")(";
                appendExp(str, u->getSubExp1());
                str << ")";
            }
            break;
        case opSgnEx: {
            str << "/* opSgnEx */ (int) ";
            Exp* s = t->getSubExp3();
            appendExp(str, s);
            break;
        }
        case opTruncu:
        case opTruncs: {
            str << "/* opTruncs/u */ (int) ";
            Exp* s = t->getSubExp3();
            appendExp(str, s);
            break;
        }
        case opMachFtr: {
            str << "/* machine specific */ (int) ";
            Exp* sub = u->getSubExp1();
            assert(sub->isStrConst());
            char* s = ((Const*)sub)->getStr();
            if (s[0] == '%')        // e.g. %Y
                str << s+1;         // Just use Y
            else
                str << s;
            break;
        }
        case opFflags:
            str << "/* Fflags() */ "; break;
        case opPow:
            str << "pow(";
            appendExp(str, b->getSubExp1());
            str << ", ";
            appendExp(str, b->getSubExp2());
            str << ")";
            break;
        case opLog2:
            str << "log2(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opLog10:
            str << "log10(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opSin:
            str << "sin(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opCos:
            str << "cos(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opTan:
            str << "tan(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opArcTan:
            str << "atan(";
            appendExp(str, u->getSubExp1());
            str << ")";
            break;
        case opSubscript:
            appendExp(str, u->getSubExp1());
            std::cerr << "subscript in code generation of proc " <<
              m_proc->getName() << " exp (without subscript): " << str.str().c_str()
                << "\n";
            assert(false);
            break;
        case opMemberAccess:
            {
                Type *ty = b->getSubExp1()->getType();
                if (ty == NULL) {
                    LOG << "no type for subexp1 of " << b << "\n";
                    str << "/* type failure */ ";
                    break;
                }
                // Trent: what were you thinking here? Fails for things like
                // local11.lhHeight (where local11 is a register)
                //assert(ty->resolvesToCompound());
                if (b->getSubExp1()->getOper() == opMemOf) {
                    appendExp(str, b->getSubExp1()->getSubExp1());
                    str << "->";
                } else {
                    appendExp(str, b->getSubExp1());
                    str << ".";
                }
                str << ((Const*)b->getSubExp2())->getStr();
            }
            break;
        case opArraySubscript:
            if (b->getSubExp1()->getOper() == opMemOf)
                str << "(";
            appendExp(str, b->getSubExp1());
            if (b->getSubExp1()->getOper() == opMemOf)
                str << ")";
            str << "[";
            appendExp(str, b->getSubExp2());
            str << "]";
            break;
        default:
            // others
            OPER op = exp->getOper();
            if (op >= opZF) {
                // Machine flags; can occasionally be manipulated individually
                // Chop off the "op" part
                str << operStrings[op]+2;
                break;
            }
            std::cerr << "not implemented " << operStrings[exp->getOper()] << 
                std::endl;
            assert(false);
    }
}

void CHLLCode::appendType(std::ostringstream& str, Type *typ)
{
    if (typ == NULL) return;
    // TODO: decode types
    str << typ->getCtype();
}

void CHLLCode::reset()
{
    for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
         it++) delete *it;
    lines.clear();
}

void CHLLCode::AddPretestedLoopHeader(int indLevel, Exp *cond)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "while (";
    appendExp(s, cond);
    s << ") {";
    // Note: removing the strdup() causes weird problems.
    // Looks to me that it should work (with no real operator delete(),
    // and garbage collecting...
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddPretestedLoopEnd(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "}";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddEndlessLoopHeader(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "for(;;) {";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddEndlessLoopEnd(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "}";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddPosttestedLoopHeader(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "do {";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddPosttestedLoopEnd(int indLevel, Exp *cond)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "} while (";
    appendExp(s, cond);
    s << ");";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondHeader(int indLevel, Exp *cond)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "switch(";
    appendExp(s, cond);
    s << ") {";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondOption(int indLevel, Exp *opt)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "case ";
    appendExp(s, opt);
    s << ":";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondOptionEnd(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "break;";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondElse(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "default:";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondEnd(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "}";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfCondHeader(int indLevel, Exp *cond)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "if (";
    appendExp(s, cond);
    s << ") {";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfCondEnd(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "}";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfElseCondHeader(int indLevel, Exp *cond)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "if (";
    appendExp(s, cond);
    s << ") {";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfElseCondOption(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "} else {";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfElseCondEnd(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "}";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddGoto(int indLevel, int ord)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "goto L" << std::dec << ord;
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddContinue(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "continue;";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddBreak(int indLevel)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "break;";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddLabel(int indLevel, int ord)
{
    std::ostringstream s;
    s << "L" << std::dec << ord << ":";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::RemoveLabel(int ord)
{
    std::ostringstream s;
    s << "L" << std::dec << ord << ":";
    for (std::list<char*>::iterator it = lines.begin();
      it != lines.end(); it++) {
        if (!strcmp(*it, s.str().c_str())) {
            lines.erase(it);
            break;
        }
    }
}

void CHLLCode::AddAssignmentStatement(int indLevel, Assign *asgn)
{
    std::ostringstream s;
    indent(s, indLevel);
    if (asgn->getLeft()->getOper() == opMemOf && asgn->getSize() != 32) 
        appendExp(s, new TypedExp(new IntegerType(asgn->getSize()), asgn->getLeft()));
    else if (asgn->getLeft()->getOper() == opGlobal &&
             ((Location*)asgn->getLeft())->getType()->isArray())
        appendExp(s, new Binary(opArraySubscript, asgn->getLeft(), new Const(0)));
    else
        appendExp(s, asgn->getLeft());
    s << " = ";
    appendExp(s, asgn->getRight());
    s << ";";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCallStatement(int indLevel, Proc *proc, 
    const char *name, std::vector<Exp*> &args, LocationSet &defs)
{
    std::ostringstream s;
    indent(s, indLevel);
    if (defs.size() >= 1) {
        LocationSet::iterator it = defs.begin();
        appendExp(s, (Exp*)*it);
        s << " = ";
        defs.remove((Exp*)*it);
    }
    s << name << "(";
    for (unsigned int i = 0; i < args.size(); i++) {
        Type *t = proc->getSignature()->getParamType(i);
        bool ok = true;
        if (t && t->isPointer() && ((PointerType*)t)->getPointsTo()->isFunc() 
              && args[i]->isIntConst()) {
            Proc *p = proc->getProg()->findProc(((Const*)args[i])->getInt());
            if (p) {
                s << p->getName();
                ok = false;
            }
        }
        if (ok)
            appendExp(s, args[i]);
        if (i < args.size() - 1) s << ", ";
    }
    s << ");";
    LocationSet::iterator it = defs.begin();
    if (it != defs.end()) {
        s << " // OUT: ";
    }
    for (; it != defs.end(); it++) {
        appendExp(s, *it);
        s << ", ";
    }
    std::string str = s.str();  // Copy the whole string
    int n = str.length();
    if (str.substr(n-2, 2) == ", ")
        str = str.substr(0, n-2);
    lines.push_back(strdup(str.c_str()));
}

// Ugh - almost the same as the above, but it needs to take an expression,
// not a Proc*
void CHLLCode::AddIndCallStatement(int indLevel, Exp *exp,
    std::vector<Exp*> &args)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "(*";
    appendExp(s, exp);
    s << ")(";
    for (unsigned int i = 0; i < args.size(); i++) {
        appendExp(s, args[i]);
        if (i < args.size() - 1) s << ", ";
    }
    s << ");";
    lines.push_back(strdup(s.str().c_str()));
}


void CHLLCode::AddReturnStatement(int indLevel, std::vector<Exp*> &returns)
{
    std::ostringstream s;
    indent(s, indLevel);
    s << "return";
    if (returns.size() >= 1) {
        s << " ";
        appendExp(s, returns[0]);
    }
    s << ";";
    if (returns.size() > 1) {
        s << "/* ";
    }
    for (unsigned i = 1; i < returns.size(); i++) {
        if (i != 1)
            s << ", ";
        appendExp(s, returns[i]);
    }
    if (returns.size() > 1) {
        s << "*/";
    }
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddProcStart(Signature *signature)
{
    std::ostringstream s;
    if (signature->getNumReturns() == 0) {
        s << "void";
    }  else 
        appendType(s, signature->getReturnType(0));
    s << " " << signature->getName() << "(";
    for (int i = 0; i < signature->getNumParams(); i++) {
        Type *ty = signature->getParamType(i); 
        if (ty->isPointer() && ((PointerType*)ty)->getPointsTo()->isArray()) {
            // C does this by default when you pass an array
            ty = ((PointerType*)ty)->getPointsTo();
            Exp *foo = new Const("foo123412341234");
            m_proc->searchAndReplace(Location::memOf(
                                        Location::param(
                                          signature->getParamName(i)), NULL), 
                                     foo);
            m_proc->searchAndReplace(Location::param(
                                       signature->getParamName(i)), 
                                     foo);
            m_proc->searchAndReplace(foo, 
                                     Location::param(
                                       signature->getParamName(i)));
        }
        appendType(s, ty);
        s << " " <<  signature->getParamName(i);
        if (i != signature->getNumParams() - 1)
            s << ", ";
    }
    s << ")";
    lines.push_back(strdup(s.str().c_str()));
    lines.push_back("{");
}

void CHLLCode::AddProcEnd()
{
    lines.push_back(strdup("}"));
    lines.push_back("");
}

void CHLLCode::AddLocal(const char *name, Type *type)
{
    std::ostringstream s;
    appendType(s, type);
    s << " " <<  name;
    Exp *e = m_proc->getLocalExp(name);
    if (e) {
        if (e->getOper() == opSubscript && ((RefExp*)e)->getRef() == NULL &&
            (e->getSubExp1()->getOper() == opParam ||
             e->getSubExp1()->getOper() == opGlobal)) {
            s << " = ";
            appendExp(s, e->getSubExp1());
            s << ";";
        } else {
            s << "; // ";
            e->print(s, true);
        }
    }
    lines.push_back(strdup(s.str().c_str()));
    locals[name] = type->clone();
}

void CHLLCode::AddGlobal(const char *name, Type *type, Exp *init)
{
    std::ostringstream s;
    // Check for array types. These are declared differently in C than
    // they are printed
    if (type->isArray()) {
        // Get the component type
        Type* base = ((ArrayType*)type)->getBaseType();
        appendType(s, base);
        s << " " << name << "[" << std::dec << ((ArrayType*)type)->getLength() << "]";
    } else if (type->isPointer() &&
      ((PointerType*)type)->getPointsTo()->resolvesToFunc()) {
        // These are even more different to declare than to print. Example:
        // void (void)* global0 = foo__1B;   ->
        // void (*global0)(void) = foo__1B;
        PointerType* pt = (PointerType*)type;
        FuncType* ft = (FuncType*)pt->getPointsTo();
        const char *ret, *param;
        ft->getReturnAndParam(ret, param);
        s << ret << "(*" << name << ")" << param;
    } else {
        appendType(s, type);
        s << " " << name;
    }
    // Don't attempt to initialise arrays yet; complex syntax required
    if (init && !type->isArray()) {
        s << " = ";
        appendExp(s, init);
    }
    s << ";";
    lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::print(std::ostream &os)
{
    for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
         it++) os << *it << std::endl;
    if (m_proc == NULL)
        os << std::endl;
}

void CHLLCode::AddLineComment(char* cmt) {
    std::ostringstream s;
    s << "/* " << cmt << "*/";
    lines.push_back(strdup(s.str().c_str()));
}


