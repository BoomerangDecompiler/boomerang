/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   chllcode.cpp
 * OVERVIEW:   Concrete class for the "C" high level language
 *			   This class is provides methods which are specific for the C
 *			   language binding.  I guess this will be the most popular output
 *			   language unless we do C++.
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

void CHLLCode::indent(std::ostringstream& str, int indLevel) {
	// Can probably do more efficiently
	for (int i=0; i < indLevel; i++)
		str << "    ";
}

// Append code for the given expression exp to stream str
// The current operator precedence is curPrec; add parens around this
// expression if necessary
// uns = cast operands to unsigned if necessary

void CHLLCode::appendExp(std::ostringstream& str, Exp *exp, PREC curPrec, bool uns /* = false */ ) {

	if (exp == NULL) return;

	Const	*c = (Const*)exp;
	Unary	*u = (Unary*)exp;
	Binary	*b = (Binary*)exp;
	Ternary *t = (Ternary*)exp;
	
	OPER op = exp->getOper();
	// First, a crude cast if unsigned
	if (uns && op != opIntConst /* && !DFA_TYPE_ANALYSIS */) {
		str << "(unsigned)";
		curPrec = PREC_UNARY;
	}

	switch(op) {
		case opIntConst: {
			int K = c->getInt();
			if (uns && K < 0) {
				// An unsigned constant. Use some heuristics
				unsigned rem = (unsigned)K % 100;
				if (rem == 0 || rem == 99) {
					// A multiple of 100, or one less; use 4000000000U style
					char num[16];
					sprintf(num, "%u", K);
					str << num << "U";
				} else {
					// Output it in 0xF0000000 style
					str << "0x" << std::hex << K;
				}
			} else {
				if (c->getType() && c->getType()->isChar()) {
					if (c->getInt() == '\a')
						str << "'\\a'";
					else if (c->getInt() == '\b')
						str << "'\\b'";
					else if (c->getInt() == '\f')
						str << "'\\f'";
					else if (c->getInt() == '\n')
						str << "'\\n'";
					else if (c->getInt() == '\r')
						str << "'\\r'";
					else if (c->getInt() == '\t')
						str << "'\\t'";
					else if (c->getInt() == '\v')
						str << "'\\v'";
					else if (c->getInt() == '\\')
						str << "'\\\\'";
					else if (c->getInt() == '\?')
						str << "'\\?'";
					else if (c->getInt() == '\'')
						str << "'\\''";
					else if (c->getInt() == '\"')
						str << "'\\\"'";
					else
						str << "'" << (char)c->getInt() << "'";
				} else {
					// More heuristics
					int K = c->getInt();
					if (-2048 <= K && K <= 2048)
						str << std::dec << K; 			// Just a plain vanilla int
					else
						str << "0x" << std::hex << K;	// 0x2000 style
				}
			}
			break;
		}
		case opLongConst:
			// sprintf(s, "%lld", c->getLong());
			//strcat(str, s);
			str << std::dec << c->getLong(); break;
		case opFltConst:
			// What to do with precision here?
			str << c->getFlt(); break;
		case opStrConst:
			str << "\"" << c->getStr() << "\""; break;
		case opFuncConst:
			str << c->getFuncName(); break;
		case opAddrOf: {
			Exp* sub = u->getSubExp1();
			if (sub->getType() && sub->getType()->isArray()) {
				appendExp(str, sub, curPrec);
				break;
			}
			if (sub->isGlobal()) {
				Prog* prog = m_proc->getProg();
				Const* con = (Const*)((Unary*)sub)->getSubExp1();
				Type* gt = prog->getGlobalType(con->getStr());
				if (gt && (gt->isArray() || (gt->isPointer() && gt->asPointer()->getPointsTo()->isChar()))) {
					// Special C requirement: don't emit "&" for address of
					// an array or char*
					appendExp(str, sub, curPrec);
					break;
				}
			}
			openParen(str, curPrec, PREC_UNARY);
			str << "&";
			appendExp(str, sub, PREC_UNARY);
			closeParen(str, curPrec, PREC_UNARY);
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
			{
				openParen(str, curPrec, PREC_EQUAL);
				appendExp(str, b->getSubExp1(), PREC_EQUAL);
				str << " == ";
				Type *ty = b->getSubExp1()->getType();
				if (ty && ty->isPointer() && b->getSubExp2()->isIntConst() && ((Const*)b->getSubExp2())->getInt() == 0)
					str << "NULL";
				else
					appendExp(str, b->getSubExp2(), PREC_EQUAL);				
				closeParen(str, curPrec, PREC_EQUAL);
			}
			break;
		case opNotEqual:
			{
				openParen(str, curPrec, PREC_EQUAL);
				appendExp(str, b->getSubExp1(), PREC_EQUAL);
				str << " != ";
				Type *ty = b->getSubExp1()->getType();
				if (ty && ty->isPointer() && b->getSubExp2()->isIntConst() && ((Const*)b->getSubExp2())->getInt() == 0)
					str << "NULL";
				else
					appendExp(str, b->getSubExp2(), PREC_EQUAL);
				closeParen(str, curPrec, PREC_EQUAL);
			}
			break;
		case opLess:
		case opLessUns:
			openParen(str, curPrec, PREC_REL);
			appendExp(str, b->getSubExp1(), PREC_REL, op == opLessUns);
			str << " < ";
			appendExp(str, b->getSubExp2(), PREC_REL, op == opLessUns);
			closeParen(str, curPrec, PREC_REL);
			break;
		case opGtr:
		case opGtrUns:
			openParen(str, curPrec, PREC_REL);
			appendExp(str, b->getSubExp1(), PREC_REL, op == opGtrUns);
			str << " > ";
			appendExp(str, b->getSubExp2(), PREC_REL, op == opGtrUns);
			closeParen(str, curPrec, PREC_REL);
			break;
		case opLessEq:
		case opLessEqUns:
			openParen(str, curPrec, PREC_REL);
			appendExp(str, b->getSubExp1(), PREC_REL, op == opLessEqUns);
			str << " <= ";
			appendExp(str, b->getSubExp2(), PREC_REL, op == opLessEqUns);
			closeParen(str, curPrec, PREC_REL);
			break;
		case opGtrEq:
		case opGtrEqUns:
			openParen(str, curPrec, PREC_REL);
			appendExp(str, b->getSubExp1(), PREC_REL, op == opGtrEqUns);
			str << " >= ";
			appendExp(str, b->getSubExp2(), PREC_REL, op == opGtrEqUns);
			closeParen(str, curPrec, PREC_REL);
			break;
		case opAnd:
			openParen(str, curPrec, PREC_LOG_AND);
			appendExp(str, b->getSubExp1(), PREC_LOG_AND);
			str << " && ";
			appendExp(str, b->getSubExp2(), PREC_LOG_AND);
			closeParen(str, curPrec, PREC_LOG_AND);
			break;
		case opOr:
			openParen(str, curPrec, PREC_LOG_OR);
			appendExp(str, b->getSubExp1(), PREC_LOG_OR);
			str << " || ";
			appendExp(str, b->getSubExp2(), PREC_LOG_OR);
			closeParen(str, curPrec, PREC_LOG_OR);
			break;
		case opBitAnd:
			openParen(str, curPrec, PREC_BIT_AND);
			appendExp(str, b->getSubExp1(), PREC_BIT_AND);
			str << " & ";
			appendExp(str, b->getSubExp2(), PREC_BIT_AND);
			closeParen(str, curPrec, PREC_BIT_AND);
			break;
		case opBitOr:
			openParen(str, curPrec, PREC_BIT_IOR);
			appendExp(str, b->getSubExp1(), PREC_BIT_IOR);
			str << " | ";
			appendExp(str, b->getSubExp2(), PREC_BIT_IOR);
			closeParen(str, curPrec, PREC_BIT_IOR);
			break;
		case opBitXor:
			openParen(str, curPrec, PREC_BIT_XOR);
			appendExp(str, b->getSubExp1(), PREC_BIT_XOR);
			str << " ^ ";
			appendExp(str, b->getSubExp2(), PREC_BIT_XOR);
			closeParen(str, curPrec, PREC_BIT_XOR);
			break;
		case opNot:
			openParen(str, curPrec, PREC_UNARY);
			appendExp(str, u->getSubExp1(), PREC_UNARY);
			closeParen(str, curPrec, PREC_UNARY);
			break;
		case opLNot:
			openParen(str, curPrec, PREC_UNARY);
			appendExp(str, u->getSubExp1(), PREC_UNARY);
			closeParen(str, curPrec, PREC_UNARY);
			break;
		case opNeg:
		case opFNeg:
			openParen(str, curPrec, PREC_UNARY);
			appendExp(str, u->getSubExp1(), PREC_UNARY);
			closeParen(str, curPrec, PREC_UNARY);
			break;
		case opAt:
		{
			// General form:
			//	s1 >> last & (1 << first-last+1)-1
			// When first == last:
			//	s1 >> last & 1
			openParen(str, curPrec, PREC_BIT_AND);
			appendExp(str, t->getSubExp1(), PREC_BIT_SHIFT);
			Exp* first = t->getSubExp2();
			Exp* last  = t->getSubExp3();
			str << " >> ";
			appendExp(str, last, PREC_BIT_SHIFT);
			str << " & ";
			if (*first == *last)
				// Can use a much shorter form; just and with 1
				str << "1";
			else {
				str << "(1 << ";
				appendExp(str, first, PREC_ADD);
				str << "-";
				appendExp(str, last, PREC_ADD);
				str << "+1)-1";
			}
			closeParen(str, curPrec, PREC_BIT_AND);
#if 0
			c = dynamic_cast<Const*>(t->getSubExp3());
			assert(c && c->getOper() == opIntConst);
			int last = c->getInt();
			str << ">>" << std::dec << last << ")";
			c = dynamic_cast<Const*>(t->getSubExp2());
			assert(c && c->getOper() == opIntConst);
			unsigned int mask = (1 << (c->getInt() - last + 1)) - 1;
			str << "&0x" << std::hex << mask;
			closeParen(str, curPrec, PREC_BIT_AND);
#endif
			break;
		}
		case opPlus:
			openParen(str, curPrec, PREC_ADD);
			appendExp(str, b->getSubExp1(), PREC_ADD);
			str << " + ";
			appendExp(str, b->getSubExp2(), PREC_ADD);
			closeParen(str, curPrec, PREC_ADD);
			break;
		case opMinus:
			openParen(str, curPrec, PREC_ADD);
			appendExp(str, b->getSubExp1(), PREC_ADD);
			str << " - ";
			appendExp(str, b->getSubExp2(), PREC_ADD);
			closeParen(str, curPrec, PREC_ADD);
			break;
		case opMemOf:
			openParen(str, curPrec, PREC_UNARY);
			if (u->getSubExp1()->getType()) {
				Exp *l = u->getSubExp1();
				Type *ty = l->getType();
				if (ty->isPointer()) {
					str << "*";
					appendExp(str, l, PREC_UNARY);
					closeParen(str, curPrec, PREC_UNARY);
					break;
				}
				str << "*(";
				appendType(str, ty);
				str << "*)";
				openParen(str, curPrec, PREC_UNARY);
				appendExp(str, l, PREC_UNARY);
				closeParen(str, curPrec, PREC_UNARY);
				break;
			}
			str << "*(int*)";
			appendExp(str, u->getSubExp1(), PREC_UNARY);
			closeParen(str, curPrec, PREC_UNARY);
			break;
		case opRegOf:
			{
				// MVE: this can likely go
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
					appendExp(str, u->getSubExp1(), PREC_NONE);
					str << "]";
				}
			}
			break;
		case opTemp:
			str << "tmp";		// Should never see this
			break;
		case opItof:
			// MVE: needs work: float/double/long double.
			str << "(float)";
			openParen(str, curPrec, PREC_UNARY);
			appendExp(str, t->getSubExp3(), PREC_UNARY);
			closeParen(str, curPrec, PREC_UNARY);
			break;
		case opFsize:
   // MVE: needs work!
			appendExp(str, t->getSubExp3(), curPrec);
			break;
		case opMult:
		case opMults:		// FIXME: check types
			openParen(str, curPrec, PREC_MULT);
			appendExp(str, b->getSubExp1(), PREC_MULT);
			str << " * ";
			appendExp(str, b->getSubExp2(), PREC_MULT);
			closeParen(str, curPrec, PREC_MULT);
			break;
		case opDiv:
		case opDivs:		// FIXME: check types
			openParen(str, curPrec, PREC_MULT);
			appendExp(str, b->getSubExp1(), PREC_MULT);
			str << " / ";
			appendExp(str, b->getSubExp2(), PREC_MULT);
			closeParen(str, curPrec, PREC_MULT);
			break;
		case opMod:
		case opMods:		// Fixme: check types
			openParen(str, curPrec, PREC_MULT);
			appendExp(str, b->getSubExp1(), PREC_MULT);
			str << " % ";
			appendExp(str, b->getSubExp2(), PREC_MULT);
			closeParen(str, curPrec, PREC_MULT);
			break;
		case opShiftL:
			openParen(str, curPrec, PREC_BIT_SHIFT);
			appendExp(str, b->getSubExp1(), PREC_BIT_SHIFT);
			str << " << ";
			appendExp(str, b->getSubExp2(), PREC_BIT_SHIFT);
			closeParen(str, curPrec, PREC_BIT_SHIFT);
			break;
		case opShiftR:
		case opShiftRA:
			openParen(str, curPrec, PREC_BIT_SHIFT);
			appendExp(str, b->getSubExp1(), PREC_BIT_SHIFT);
			str << " >> ";
			appendExp(str, b->getSubExp2(), PREC_BIT_SHIFT);
			closeParen(str, curPrec, PREC_BIT_SHIFT);
			break;
		case opTern:
			openParen(str, curPrec, PREC_COND);
			appendExp(str, t->getSubExp1(), PREC_COND);
			str << " ? ";
			appendExp(str, t->getSubExp2(), PREC_COND);
			str << " : ";
			appendExp(str, t->getSubExp3(), PREC_COND);
			closeParen(str, curPrec, PREC_COND);
			break;
		case opFPlus:
		case opFPlusd:
		case opFPlusq:
			openParen(str, curPrec, PREC_ADD);
			appendExp(str, b->getSubExp1(), PREC_ADD);
			str << " + ";
			appendExp(str, b->getSubExp2(), PREC_ADD);
			closeParen(str, curPrec, PREC_ADD);
			break;
		case opFMinus:
		case opFMinusd:
		case opFMinusq:
			openParen(str, curPrec, PREC_ADD);
			appendExp(str, b->getSubExp1(), PREC_ADD);
			str << " - ";
			appendExp(str, b->getSubExp2(), PREC_ADD);
			closeParen(str, curPrec, PREC_ADD);
			break;
		case opFMult:
		case opFMultd:
		case opFMultq:
			openParen(str, curPrec, PREC_MULT);
			appendExp(str, b->getSubExp1(), PREC_MULT);
			str << " * ";
			appendExp(str, b->getSubExp2(), PREC_MULT);
			closeParen(str, curPrec, PREC_MULT);
			break;
		case opFDiv:
		case opFDivd:
		case opFDivq:
			openParen(str, curPrec, PREC_MULT);
			appendExp(str, b->getSubExp1(), PREC_MULT);
			str << " / ";
			appendExp(str, b->getSubExp2(), PREC_MULT);
			closeParen(str, curPrec, PREC_MULT);
			break;
		case opFround:
			// Note: we need roundf or roundl depending on size of operands
			str << "round(";		// Note: math.h required
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opFtrunc:
			// Note: we need truncf or truncl depending on size of operands
			str << "trunc(";		// Note: math.h required
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opFabs:
			str << "fabs(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opFtoi:
			// Should check size!
			str << "(int)";
			appendExp(str, u->getSubExp3(), PREC_UNARY);
			break;
		case opRotateL:
		str << "ROTL(";
			appendExp(str, u->getSubExp1(), PREC_UNARY);
		str << ")";
		break;
		case opRotateR:
		str << "ROTR(";
			appendExp(str, u->getSubExp1(), PREC_UNARY);
		str << ")";
		break;
		case opRotateLC:
		str << "ROTLC(";
			appendExp(str, u->getSubExp1(), PREC_UNARY);
		str << ")";
		break;
		case opRotateRC:
		str << "ROTRC(";
			appendExp(str, u->getSubExp1(), PREC_UNARY);
		str << ")";
		break;
		case opSize:
			{
				/*Type *ty = new IntegerType(((Const*)b->getSubExp1())->getInt(), 1);
				str << "*(" << ty->getCtype(true) << " *)";
				appendExp(str, new Unary(opAddrOf, b->getSubExp2()), PREC_UNARY);*/
				appendExp(str, b->getSubExp2(), PREC_UNARY);
			}
			break;
		case opFMultsd:
		case opFMultdq:
		case opSQRTs:
		case opSQRTd:
		case opSQRTq:
		case opSignExt:
		case opTargetInst:
		case opNamedExp:
		case opGuard:
		case opVar:
		case opArg:
		case opExpand:		
		case opCastIntStar:
		case opPostVar:
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
			LOG << "not implemented for codegen " << operStrings[exp->getOper()] << "\n";
			//assert(false);
			break;
		case opFlagCall:
			{
				assert(b->getSubExp1()->getOper() == opStrConst);
				str << ((Const*)b->getSubExp1())->getStr();
				str << "(";
				Binary *l = (Binary*)b->getSubExp2();
				for (; l && l->getOper() == opList; 
					 l = (Binary*)l->getSubExp2()) {
					appendExp(str, l->getSubExp1(), PREC_NONE);
					if (l->getSubExp2()->getOper() == opList)
						str << ", ";
				}
				str << ")";
			} 
			break;
		case opList:
			appendExp(str, b->getSubExp1(), PREC_NONE);
			if (b->getSubExp2()->getOper() == opList) {
				str << ", ";
				appendExp(str, b->getSubExp2(), PREC_NONE);
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
			//	((Const*)t->getSubExp1())->getInt(),
			//	((Const*)t->getSubExp2())->getInt());
			//strcat(str, s); */
			str << "(";
			appendExp(str, t->getSubExp3(), PREC_NONE);
			str << ")";
			break;
		case opTypedExp:
			if (u->getSubExp1()->getOper() == opTypedExp &&
					*((TypedExp*)u)->getType() == *((TypedExp*)u->getSubExp1())->getType()) {
				appendExp(str, u->getSubExp1(), curPrec);
			} else if (u->getSubExp1()->getOper() == opMemOf) {
				PointerType *pty = dynamic_cast<PointerType*>(u->getSubExp1()->getSubExp1()->getType());
				Type *tt = ((TypedExp*)u)->getType();
				if (pty != NULL && (*pty->getPointsTo() == *tt ||
						(tt->isSize() && pty->getPointsTo()->getSize() == tt->getSize())))
					str << "*";
				else {
					str << "*(";
					appendType(str, tt);
					str << "*)";
				}
				openParen(str, curPrec, PREC_UNARY);
				appendExp(str, u->getSubExp1()->getSubExp1(), PREC_UNARY);
				closeParen(str, curPrec, PREC_UNARY);
			} else {
				str << "(";
				appendType(str, ((TypedExp*)u)->getType());
				str << ")";
				openParen(str, curPrec, PREC_UNARY);
				appendExp(str, u->getSubExp1(), PREC_UNARY);
				closeParen(str, curPrec, PREC_UNARY);
			}
			break;
		case opSgnEx: {
			// MVE: Needs work
			str << "/* opSgnEx */ (int) ";
			Exp* s = t->getSubExp3();
			appendExp(str, s, curPrec);
			break;
		}
		case opTruncu:
		case opTruncs: {
			str << "/* opTruncs/u */ (int) ";
			Exp* s = t->getSubExp3();
			appendExp(str, s, curPrec);
			break;
		}
		case opMachFtr: {
			str << "/* machine specific */ (int) ";
			Exp* sub = u->getSubExp1();
			assert(sub->isStrConst());
			char* s = ((Const*)sub)->getStr();
			if (s[0] == '%')		// e.g. %Y
				str << s+1;			// Just use Y
			else
				str << s;
			break;
		}
		case opFflags:
			str << "/* Fflags() */ "; break;
		case opPow:
			str << "pow(";
			appendExp(str, b->getSubExp1(), PREC_COMMA);
			str << ", ";
			appendExp(str, b->getSubExp2(), PREC_COMMA);
			str << ")";
			break;
		case opLog2:
			str << "log2(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opLog10:
			str << "log10(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opSin:
			str << "sin(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opCos:
			str << "cos(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opTan:
			str << "tan(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opArcTan:
			str << "atan(";
			appendExp(str, u->getSubExp1(), PREC_NONE);
			str << ")";
			break;
		case opSubscript:
			appendExp(str, u->getSubExp1(), curPrec);
			LOG << "ERROR: subscript in code generation of proc " <<
			  m_proc->getName() << " exp (without subscript): " << str.str().c_str()
				<< "\n";
			//assert(false);
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
					appendExp(str, b->getSubExp1()->getSubExp1(), PREC_PRIM);
					str << "->";
				} else {
					appendExp(str, b->getSubExp1(), PREC_PRIM);
					str << ".";
				}
				str << ((Const*)b->getSubExp2())->getStr();
			}
			break;
		case opArraySubscript:
			openParen(str, curPrec, PREC_PRIM);
			appendExp(str, b->getSubExp1(), PREC_PRIM);
			closeParen(str, curPrec, PREC_PRIM);
			str << "[";
			appendExp(str, b->getSubExp2(), PREC_PRIM);
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
			LOG << "ERROR: not implemented for codegen " << operStrings[exp->getOper()] << "\n";
			//assert(false);
	}

}

void CHLLCode::appendType(std::ostringstream& str, Type *typ)
{
	if (typ == NULL) return;
	// TODO: decode types
	str << typ->getCtype(true);
}

void CHLLCode::appendTypeIdent(std::ostringstream& str, Type *typ, const char *ident) {
	if (typ == NULL) return;
	if (typ->isPointer() && typ->asPointer()->getPointsTo()->isArray()) {
		appendType(str, typ->asPointer()->getPointsTo()->asArray()->getBaseType());
		str << " *" << ident;
	} else if (typ->isPointer()) {
		appendType(str, typ);
		str << ident;
	} else if (typ->isArray()) {
		ArrayType *a = typ->asArray();
		appendTypeIdent(str, a->getBaseType(), ident);
		str << "[";
		if (!a->isUnbounded())
			str << a->getLength();
		str << "]";
	} else if (typ->isVoid()) {
		// Can happen in e.g. twoproc, where really need global parameter and return analysis
		LOG << "Warning: replacing type void with int for " << ident << "\n";
		str << "int " << ident;
	} else {
		appendType(str, typ);
		str << " " << ident;
	}		
}

void CHLLCode::reset() {
	for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
		 it++) delete *it;
	lines.clear();
}

void CHLLCode::AddPretestedLoopHeader(int indLevel, Exp *cond) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "while (";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	// Note: removing the strdup() causes weird problems.
	// Looks to me that it should work (with no real operator delete(),
	// and garbage collecting...
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddPretestedLoopEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddEndlessLoopHeader(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "for(;;) {";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddEndlessLoopEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddPosttestedLoopHeader(int indLevel) {
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
	appendExp(s, cond, PREC_NONE);
	s << ");";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondHeader(int indLevel, Exp *cond)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "switch(";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCaseCondOption(int indLevel, Exp *opt)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "case ";
	appendExp(s, opt, PREC_NONE);
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

void CHLLCode::AddIfCondHeader(int indLevel, Exp *cond) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "if (";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfCondEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfElseCondHeader(int indLevel, Exp *cond) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "if (";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfElseCondOption(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "} else {";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddIfElseCondEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddGoto(int indLevel, int ord) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "goto L" << std::dec << ord << ";";
	lines.push_back(strdup(s.str().c_str()));
	usedLabels.insert(ord);
}

void CHLLCode::RemoveUnusedLabels(int maxOrd) {
	for (std::list<char *>::iterator it = lines.begin(); it != lines.end();) {
		if ((*it)[0] == 'L') {
			char *s = strdup(*it);
			*strchr(s, ':') = 0;
			int n = atoi(s+1);
			if (usedLabels.find(n) == usedLabels.end()) {
				it = lines.erase(it);
				continue;
			}
		}
		it++;
	}
}

void CHLLCode::AddContinue(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "continue;";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddBreak(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "break;";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddLabel(int indLevel, int ord) {
	std::ostringstream s;
	s << "L" << std::dec << ord << ":";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::RemoveLabel(int ord) {
	std::ostringstream s;
	s << "L" << std::dec << ord << ":";
	for (std::list<char*>::iterator it = lines.begin(); it != lines.end(); it++) {
		if (!strcmp(*it, s.str().c_str())) {
			lines.erase(it);
			break;
		}
	}
}

void CHLLCode::AddAssignmentStatement(int indLevel, Assign *asgn) {
	if (asgn->getLeft()->getOper() == opPC)
		return;

	std::ostringstream s;
	indent(s, indLevel);
	Type* asgnType = asgn->getType();
	if (asgn->getLeft()->getOper() == opMemOf && asgnType) 
		appendExp(s,
			new TypedExp(
				asgnType,
				asgn->getLeft()), PREC_ASSIGN);
	else if (asgn->getLeft()->getOper() == opGlobal &&
			 ((Location*)asgn->getLeft())->getType() && 
			 ((Location*)asgn->getLeft())->getType()->isArray())
		appendExp(s, new Binary(opArraySubscript, asgn->getLeft(),
			new Const(0)), PREC_ASSIGN);
	else
		appendExp(s, asgn->getLeft(), PREC_ASSIGN);
	if (asgn->getRight()->getOper() == opPlus && 
		*asgn->getRight()->getSubExp1() == *asgn->getLeft()) {
		// C has special syntax for this, eg += and ++
		if (asgn->getRight()->getSubExp2()->getOper() == opIntConst &&
			((Const*)asgn->getRight()->getSubExp2())->getInt() == 1) 
			s << "++";
		else {
			s << " += ";
			appendExp(s, asgn->getRight()->getSubExp2(), PREC_ASSIGN);
		}
	} else {
		s << " = ";
		appendExp(s, asgn->getRight(), PREC_ASSIGN);
	}
	s << ";";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddCallStatement(int indLevel, Proc *proc, const char *name, std::vector<Exp*> &args,
	std::vector<ReturnInfo>& rets) {
	std::ostringstream s;
	indent(s, indLevel);
	unsigned n = 0;
	if (rets.size() >= 1) {
		for (n = 0; n < rets.size(); n++) {
			if (rets[n].e) {
				appendExp(s, rets[n].e, PREC_ASSIGN);
				s << " = ";
				break;
			}
		}
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
			appendExp(s, args[i], PREC_COMMA);
		if (i < args.size() - 1) s << ", ";
	}
	s << ");";
	bool first = true;
	for (n++; n < rets.size(); n++) 
		if (rets[n].e) {
			if (first)
				s << " // OUT: ";
			else
				s << ", ";
			appendExp(s, rets[n].e, PREC_COMMA);
			first = false;
		}
	std::string str = s.str();	// Copy the whole string
	n = str.length();
	if (str.substr(n-2, 2) == ", ")
		str = str.substr(0, n-2);
	lines.push_back(strdup(str.c_str()));
}

// Ugh - almost the same as the above, but it needs to take an expression,
// not a Proc*
void CHLLCode::AddIndCallStatement(int indLevel, Exp *exp, std::vector<Exp*> &args) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "(*";
	appendExp(s, exp, PREC_NONE);
	s << ")(";
	for (unsigned int i = 0; i < args.size(); i++) {
		appendExp(s, args[i], PREC_COMMA);
		if (i < args.size() - 1) s << ", ";
	}
	s << ");";
	lines.push_back(strdup(s.str().c_str()));
}


void CHLLCode::AddReturnStatement(int indLevel, std::vector<Exp*> &returns) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "return";
	if (returns.size() >= 1) {
		s << " ";
		appendExp(s, returns[0], PREC_NONE);
	}
	s << ";";
	if (returns.size() > 1) {
		s << "/* ";
	}
	for (unsigned i = 1; i < returns.size(); i++) {
		if (i != 1)
			s << ", ";
		appendExp(s, returns[i], PREC_NONE);
	}
	if (returns.size() > 1) {
		s << "*/";
	}
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddProcStart(Signature *signature) {AddProcDec(signature, true);}
void CHLLCode::AddPrototype(Signature *signature) {AddProcDec(signature, false);}
void CHLLCode::AddProcDec(Signature *signature, bool open) {
	std::ostringstream s;
	if (signature->getNumReturns() == 0) {
		s << "void ";
	}  else {
		appendType(s, signature->getReturnType(0));
		if (!signature->getReturnType(0)->isPointer())
			s << " ";
	}
	s << signature->getName() << "(";
	for (int i = 0; i < signature->getNumParams(); i++) {
		Type *ty = signature->getParamType(i); 
		if (ty->isPointer() && ((PointerType*)ty)->getPointsTo()->isArray()) {
			// C does this by default when you pass an array
			ty = ((PointerType*)ty)->getPointsTo();
			Exp *foo = new Const("foo123412341234");
			m_proc->searchAndReplace(Location::memOf(Location::param(signature->getParamName(i)), NULL), foo);
			m_proc->searchAndReplace(Location::param(signature->getParamName(i)), foo);
			m_proc->searchAndReplace(foo, Location::param(signature->getParamName(i)));
		}
		appendTypeIdent(s, ty, signature->getParamName(i));
		if (i != signature->getNumParams() - 1)
			s << ", ";
	}
	s << ")";
	if (open)
		s << "{";
	else
		s << ";";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::AddProcEnd() {
	lines.push_back(strdup("}"));
	lines.push_back("");
}

void CHLLCode::AddLocal(const char *name, Type *type, bool last) {
	std::ostringstream s;
	indent(s, 1);
	appendTypeIdent(s, type, name);
	Exp *e = m_proc->getLocalExp(name);
	if (e) {
	  //if (e->getOper() == opSubscript && ((RefExp*)e)->getRef() == NULL &&
		if (e->getOper() == opSubscript && ((RefExp*)e)->isImplicitDef() &&
			(e->getSubExp1()->getOper() == opParam ||
			 e->getSubExp1()->getOper() == opGlobal)) {
			s << " = ";
			appendExp(s, e->getSubExp1(), PREC_NONE);
			s << ";";
		} else {
			s << "; \t\t// ";
			e->print(s);
		}
	} else
		s << ";";
	lines.push_back(strdup(s.str().c_str()));
	locals[name] = type->clone();
	if (last)
		lines.push_back(strdup(""));
}

void CHLLCode::AddGlobal(const char *name, Type *type, Exp *init) {
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
		// void (void)* global0 = foo__1B;	 ->
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
	if (init && !init->isNil()) {
		s << " = ";
		if (type->isArray())
			s << "{ ";
		appendExp(s, init, PREC_ASSIGN);
		if (type->isArray())
			s << " }";
	}
	s << ";";
	lines.push_back(strdup(s.str().c_str()));
}

void CHLLCode::print(std::ostream &os) {
	for (std::list<char*>::iterator it = lines.begin(); it != lines.end(); it++) 
		 os << *it << std::endl;
	if (m_proc == NULL)
		os << std::endl;
}

void CHLLCode::AddLineComment(char* cmt) {
	std::ostringstream s;
	s << "/* " << cmt << "*/";
	lines.push_back(strdup(s.str().c_str()));
}

