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
 * OVERVIEW:   Concrete backend class for the "C" high level language
 *			   This class is provides methods which are specific for the C language binding.
 *			   I guess this will be the most popular output language unless we do C++.
 *============================================================================*/

/*
 * $Revision$	// 1.90.2.16
 * 20 Jun 02 - Trent: Quick and dirty implementation for debugging
 * 28 Jun 02 - Trent: Starting to look better
 * 22 May 03 - Mike: delete -> free() to keep valgrind happy
 * 16 Apr 04 - Mike: char[] replaced by ostringstreams
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "cfg.h"
#include "statement.h"
#include "exp.h"
#include "proc.h"
#include "prog.h"
#include "hllcode.h"
#include "chllcode.h"
#include "signature.h"
#include "boomerang.h"
#include "type.h"
#include "util.h"
#include "log.h"
#include <sstream>

extern char *operStrings[];

/// Empty constructor, calls HLLCode()
CHLLCode::CHLLCode() : HLLCode()
{	
}

/// Empty constructor, calls HLLCode(p)
CHLLCode::CHLLCode(UserProc *p) : HLLCode(p)
{	
}

/// Empty destructor
CHLLCode::~CHLLCode()
{
}

/// Output 4 * \a indLevel spaces to \a str
void CHLLCode::indent(std::ostringstream& str, int indLevel) {
	// Can probably do more efficiently
	for (int i=0; i < indLevel; i++)
		str << "    ";
}

/**
 * Append code for the given expression \a exp to stream \a str.
 * 
 * \param str		The stream to output to.
 * \param ext		The expresson to output.
 * \param curPrec 	The current operator precedence. Add parens around this expression if necessary.
 * \param uns 		If true, cast operands to unsigned if necessary.
 *
 * \todo This function is 700+ lines, and should be split up.
 */
void CHLLCode::appendExp(std::ostringstream& str, Exp *exp, PREC curPrec, bool uns /* = false */ ) {

	if (exp == NULL) return;

	OPER op = exp->getOper();
	// First, a crude cast if unsigned
	if (uns && op != opIntConst && op != opList/* && !DFA_TYPE_ANALYSIS */) {
		str << "(unsigned)";
		curPrec = PREC_UNARY;
	}

	// Check if it's mapped to a symbol
	if (m_proc) {
		char* sym = m_proc->lookupSym(exp);
		if (sym) {
			str << sym;
			return;
		}
	}

	Const	*c = (Const*)exp;
	Unary	*u = (Unary*)exp;
	Binary	*b = (Binary*)exp;
	Ternary *t = (Ternary*)exp;
	
	switch(op) {
		case opIntConst: {
			int K = c->getInt();
			if (uns && K < 0) {
				// An unsigned constant. Use some heuristics
				unsigned rem = (unsigned)K % 100;
				if (rem == 0 || rem == 99 || K > -128) {
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
			str << std::dec << c->getLong() << "LL"; break;
		case opFltConst:
			// What to do with precision here?
			str << c->getFlt(); break;
		case opStrConst:
			// escape string:
			str << "\"" << escapeStr(c->getStr()) << "\"";
            break;
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
					// Special C requirement: don't emit "&" for address of an array or char*
					appendExp(str, sub, curPrec);
					break;
				}
			}
			if (sub->isMemOf() && m_proc->lookupSym(sub) == NULL) {
				// Avoid &*(type*)sub, just emit sub
				appendExp(str, sub->getSubExp1(), PREC_UNARY);
			} else {
				openParen(str, curPrec, PREC_UNARY);
				str << "&";
				appendExp(str, sub, PREC_UNARY);
				closeParen(str, curPrec, PREC_UNARY);
			}
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
			if (Boomerang::get()->noDecompile) {
				str << "MEMOF(";
				appendExp(str, u->getSubExp1(), PREC_UNARY);
				str << ")";
				break;
			}
			openParen(str, curPrec, PREC_UNARY);
			if (u->getSubExp1()->getType() &&
			    !u->getSubExp1()->getType()->isVoid()) {
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
				if (n) {
					if (n[0] == '%')
						str << n+1;
					else
						str << n;
				} else {
// What is this doing in the back end???
					str << "r[";
					appendExp(str, u->getSubExp1(), PREC_NONE);
					str << "]";
				}
			}
			break;
		case opTemp:
			// Should never see this; temps should be mapped to locals now so that they get declared
			// Emit the temp name, e.g. "tmp1"
			str << ((Const*)u->getSubExp1())->getStr();
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
   			if (Boomerang::get()->noDecompile && t->getSubExp3()->isMemOf()) {
				assert(t->getSubExp1()->isIntConst());
				if (((Const*)t->getSubExp1())->getInt() == 32)
					str << "FLOAT_MEMOF(";
				else
					str << "DOUBLE_MEMOF(";
				appendExp(str, t->getSubExp3()->getSubExp1(), curPrec);
				str << ")";
				break;
			}
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
			appendExp(str, b->getSubExp1(), PREC_NONE, uns);
			if (b->getSubExp2()->getOper() == opList) {
				str << ", ";
				appendExp(str, b->getSubExp2(), PREC_NONE, uns);
			}
			break;
		case opFlags:
			str << "flags"; break;
		case opPC:
			str << "pc"; break;
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
				// We have (type)(type)x: recurse with type(x)
				appendExp(str, u->getSubExp1(), curPrec);
			} else if (u->getSubExp1()->getOper() == opMemOf) {
				// We have (tt)m[x]
				PointerType *pty = dynamic_cast<PointerType*>(u->getSubExp1()->getSubExp1()->getType());
				// pty = T(x)
				Type *tt = ((TypedExp*)u)->getType();
				if (pty != NULL && (*pty->getPointsTo() == *tt ||
						(tt->isSize() && pty->getPointsTo()->getSize() == tt->getSize())))
					str << "*";
				else {
					if (Boomerang::get()->noDecompile) {
						if (tt && tt->isFloat()) {
							if (tt->asFloat()->getSize() == 32)
								str << "FLOAT_MEMOF";
							else
								str << "DOUBLE_MEMOF";
						} else 
							str << "MEMOF";
					} else {
						str << "*(";
						appendType(str, tt);
						str << "*)";
					}
				}
				openParen(str, curPrec, PREC_UNARY);
				// Emit x
				appendExp(str, ((Location*)((TypedExp*)u)->getSubExp1())->getSubExp1(), PREC_UNARY);
				closeParen(str, curPrec, PREC_UNARY);
			} else {
				// Check for (tt)b where tt is a pointer; could be &local
				Type* tt = ((TypedExp*)u)->getType();
				Exp* b = u->getSubExp1();
				if (dynamic_cast<PointerType*>(tt)) {
					char* sym = m_proc->lookupSym(Location::memOf(b));
					if (sym) {
						openParen(str, curPrec, PREC_UNARY);
						str << "&" << sym;
						closeParen(str, curPrec, PREC_UNARY);
						break;
					}
				}
				// Otherwise, fall back to (tt)b
				str << "(";
				appendType(str, tt);
				str << ")";
				openParen(str, curPrec, PREC_UNARY);
				appendExp(str, u->getSubExp1(), PREC_UNARY);
				closeParen(str, curPrec, PREC_UNARY);
			}
			break;
		case opSgnEx: 
		case opTruncs: {
			Exp* s = t->getSubExp3();
			int toSize = ((Const*)t->getSubExp2())->getInt();
			switch (toSize) {
				case 8:		str << "(char) "; break;
				case 16:	str << "(short) "; break;
				case 64:	str << "(long long) "; break;
				default:	str << "(int) "; break;
			}
			appendExp(str, s, curPrec);
			break;
		}
		case opTruncu: {
			Exp* s = t->getSubExp3();
			int toSize = ((Const*)t->getSubExp2())->getInt();
			switch (toSize) {
				case 8:		str << "(unsigned char) "; break;
				case 16:	str << "(unsigned short) "; break;
				case 64:	str << "(unsigned long long) "; break;
				default:	str << "(unsigned int) "; break;
			}
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
		case opArrayIndex:
			openParen(str, curPrec, PREC_PRIM);
			appendExp(str, b->getSubExp1(), PREC_PRIM);
			closeParen(str, curPrec, PREC_PRIM);
			str << "[";
			appendExp(str, b->getSubExp2(), PREC_PRIM);
			str << "]";
			break;
		case opDefineAll:
			str << "<all>";
			LOG << "ERROR: should not see opDefineAll in codegen\n";
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
			LOG << "ERROR: not implemented for codegen " << operStrings[op] << "\n";
			//assert(false);
	}

}

/// Print the type represented by \a typ to \a str.
void CHLLCode::appendType(std::ostringstream& str, Type *typ)
{
	if (typ == NULL) {
		str << "int";			// Default type for C
		return;
	}
	str << typ->getCtype(true);
}

/**
 * Print the indented type to \a str.
 */
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
		str << " " << (ident ? ident : "<null>");
	}	
}

/// Remove all generated code.
void CHLLCode::reset() {
	for (std::list<char*>::iterator it = lines.begin(); it != lines.end();
		 it++) delete *it;
	lines.clear();
}

/// Adds: while( \a cond) {
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

/// Adds: }
void CHLLCode::AddPretestedLoopEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: for(;;) {
void CHLLCode::AddEndlessLoopHeader(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "for(;;) {";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: }
void CHLLCode::AddEndlessLoopEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: do {
void CHLLCode::AddPosttestedLoopHeader(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "do {";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: } while (\a cond);
void CHLLCode::AddPosttestedLoopEnd(int indLevel, Exp *cond)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "} while (";
	appendExp(s, cond, PREC_NONE);
	s << ");";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: switch(\a cond) {
void CHLLCode::AddCaseCondHeader(int indLevel, Exp *cond)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "switch(";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: case \a opt :
void CHLLCode::AddCaseCondOption(int indLevel, Exp *opt)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "case ";
	appendExp(s, opt, PREC_NONE);
	s << ":";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: break;
void CHLLCode::AddCaseCondOptionEnd(int indLevel)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "break;";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: default:
void CHLLCode::AddCaseCondElse(int indLevel)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "default:";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: }
void CHLLCode::AddCaseCondEnd(int indLevel)
{
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: if(\a cond) {
void CHLLCode::AddIfCondHeader(int indLevel, Exp *cond) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "if (";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: }
void CHLLCode::AddIfCondEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: if(\a cond) {
void CHLLCode::AddIfElseCondHeader(int indLevel, Exp *cond) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "if (";
	appendExp(s, cond, PREC_NONE);
	s << ") {";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: } else {
void CHLLCode::AddIfElseCondOption(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "} else {";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: }
void CHLLCode::AddIfElseCondEnd(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "}";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: goto L \em ord
void CHLLCode::AddGoto(int indLevel, int ord) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "goto L" << std::dec << ord << ";";
	lines.push_back(strdup(s.str().c_str()));
	usedLabels.insert(ord);
}

/**
 * Removes labels from the code which are not in usedLabels.
 * \param maxOrd UNUSED
 */
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

/// Adds: continue;
void CHLLCode::AddContinue(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "continue;";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: break;
void CHLLCode::AddBreak(int indLevel) {
	std::ostringstream s;
	indent(s, indLevel);
	s << "break;";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: L \a ord :
void CHLLCode::AddLabel(int indLevel, int ord) {
	std::ostringstream s;
	s << "L" << std::dec << ord << ":";
	lines.push_back(strdup(s.str().c_str()));
}

/// Search for the label L \a ord and remove it from the generated code.
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

/// Prints an assignment expression.
void CHLLCode::AddAssignmentStatement(int indLevel, Assign *asgn) {
	// Gerard: shouldn't these  3 types of statements be removed earlier?
	if (asgn->getLeft()->getOper() == opPC)
		return;						// Never want to see assignments to %PC
	Exp *result;
	if (asgn->getRight()->search(new Terminal(opPC), result)) // Gerard: what's this?
		return;
	// ok I want this now
	//if (asgn->getLeft()->isFlags())
	//	return;

	std::ostringstream s;
	indent(s, indLevel);
	Type* asgnType = asgn->getType();
	Exp* lhs = asgn->getLeft();
	Exp* rhs = asgn->getRight();

	if (Boomerang::get()->noDecompile && rhs->isMemOf() && lhs->getOper() == opRegOf && m_proc->getProg()->getFrontEndId() == PLAT_SPARC) {
		// add some fsize hints to rhs
		if (((Const*)lhs->getSubExp1())->getInt() >= 32 &&
			((Const*)lhs->getSubExp1())->getInt() <= 63)
			rhs = new Ternary(opFsize, new Const(32), new Const(32), rhs);
		else if (((Const*)lhs->getSubExp1())->getInt() >= 64 &&
			((Const*)lhs->getSubExp1())->getInt() <= 87)
			rhs = new Ternary(opFsize, new Const(64), new Const(64), rhs);
	}

	if (Boomerang::get()->noDecompile && lhs->isMemOf()) {
		if (asgnType && asgnType->isFloat()) {
			if (asgnType->asFloat()->getSize() == 32)
				s << "FLOAT_";
			else
				s << "DOUBLE_";
		} else if (rhs->getOper() == opFsize) {
			if (((Const*)rhs->getSubExp2())->getInt() == 32)
				s << "FLOAT_";
			else
				s << "DOUBLE_";
		} else if (rhs->getOper() == opRegOf && m_proc->getProg()->getFrontEndId() == PLAT_SPARC) {
			// yes, this is a hack
			if (((Const*)rhs->getSubExp1())->getInt() >= 32 &&
				((Const*)rhs->getSubExp1())->getInt() <= 63)
				s << "FLOAT_";
			else if (((Const*)rhs->getSubExp1())->getInt() >= 64 &&
				((Const*)rhs->getSubExp1())->getInt() <= 87)
				s << "DOUBLE_";
		}

		s << "MEMASSIGN(";
		appendExp(s, lhs->getSubExp1(), PREC_UNARY);
		s << ", ";
		appendExp(s, rhs, PREC_UNARY);
		s << ");";
		lines.push_back(strdup(s.str().c_str()));
		return;
	}
	
	if (lhs->isMemOf() && asgnType && !asgnType->isVoid()) 
		appendExp(s,
			new TypedExp(
				asgnType,
				lhs), PREC_ASSIGN);
	else if (lhs->getOper() == opGlobal &&
			 ((Location*)lhs)->getType() && 
			 ((Location*)lhs)->getType()->isArray())
		appendExp(s, new Binary(opArrayIndex,
			lhs,
			new Const(0)), PREC_ASSIGN);
	else if (lhs->getOper() == opAt &&
			((Ternary*)lhs)->getSubExp2()->isIntConst() &&
			((Ternary*)lhs)->getSubExp3()->isIntConst()) {
		// exp1@[n:m] := rhs -> exp1 = exp1 & mask | rhs << m  where mask = ~((1 << m-n+1)-1)
		Exp* exp1 = ((Ternary*)lhs)->getSubExp1();
		int n = ((Const*)((Ternary*)lhs)->getSubExp2())->getInt();
		int m = ((Const*)((Ternary*)lhs)->getSubExp3())->getInt();
		appendExp(s, exp1, PREC_ASSIGN);
		s << " = ";
		int mask = ~(((1 << (m-n+1))-1) << m);			// MSVC winges without most of these parentheses
		rhs = new Binary(opBitAnd,
			exp1,
			new Binary(opBitOr,
				new Const(mask),
				new Binary(opShiftL,
					rhs,
					new Const(m))));
		rhs = rhs->simplify();
		appendExp(s, rhs, PREC_ASSIGN);
		s << ";";
		lines.push_back(strdup(s.str().c_str()));
		return;
	} else
		appendExp(s, lhs, PREC_ASSIGN);			// Ordinary LHS
	if (rhs->getOper() == opPlus && 
			*rhs->getSubExp1() == *lhs) {
		// C has special syntax for this, eg += and ++
		// however it's not always acceptable for assigns to m[] (?)
		if (rhs->getSubExp2()->isIntConst() &&
				((Const*)rhs->getSubExp2())->getInt() == 1) 
			s << "++";
		else {
			s << " += ";
			appendExp(s, rhs->getSubExp2(), PREC_ASSIGN);
		}
	} else {
		s << " = ";
		appendExp(s, rhs, PREC_ASSIGN);
	}
	s << ";";
	lines.push_back(strdup(s.str().c_str()));
}

/**
 * Adds a call to \a proc.
 *
 * \param indLevel		A string containing spaces to the indentation level.
 * \param proc			The Proc the call is to.
 * \param name			The name the Proc has.
 * \param args			The arguments to the call.
 * \param results		The variable that will receive the return value of the function.
 *
 * \todo				Remove the \a name parameter and use Proc::getName()
 * \todo				Add assingment for when the function returns a struct.
 */
void CHLLCode::AddCallStatement(int indLevel, Proc *proc, const char *name, StatementList &args, StatementList* results)
{
	std::ostringstream s;
	indent(s, indLevel);
	if (results->size() >= 1) {
		// FIXME: Needs changing if more than one real result (return a struct)
		Exp* firstRet = ((Assignment*)*results->begin())->getLeft();
		appendExp(s, firstRet, PREC_ASSIGN);
		s << " = ";
	}
	s << name << "(";
	StatementList::iterator ss;
	bool first = true;
	int n = 0;
	for (ss = args.begin(); ss != args.end(); ++ss, ++n) {
		if (first)
			first = false;
		else
			s << ", ";
		Type *t = ((Assign*)*ss)->getType();
		Exp* arg = ((Assign*)*ss)->getRight();
		bool ok = true;
		if (t && t->isPointer() && ((PointerType*)t)->getPointsTo()->isFunc() && arg->isIntConst()) {
			Proc *p = proc->getProg()->findProc(((Const*)arg)->getInt());
			if (p) {
				s << p->getName();
				ok = false;
			}
		}
		if (ok) {
			bool needclose = false;
			if (Boomerang::get()->noDecompile && proc->getSignature()->getParamType(n) && proc->getSignature()->getParamType(n)->isPointer()) {
				s << "ADDR(";
				needclose = true;
			}
			appendExp(s, arg, PREC_COMMA);
			if (needclose)
				s << ")";
		}
	}
	s << ");";
	if (results->size() > 1) {
		bool first = true;
		s << " /* OUT: ";
		for (ss = ++results->begin(); ss != results->end(); ++ss) {
			if (first)
				first = false;
			else
				s << ", ";
			appendExp(s, ((Assignment*)*ss)->getLeft(), PREC_COMMA);
		}
		s << " */";
	}
			
	lines.push_back(strdup(s.str().c_str()));
}

/**
 * Adds an indirect call to \a exp.
 * \see AddCallStatement
 * \param results UNUSED
 * \todo Add the use of \a results like AddCallStatement.
 */
// Ugh - almost the same as the above, but it needs to take an expression, // not a Proc*
void CHLLCode::AddIndCallStatement(int indLevel, Exp *exp, StatementList &args, StatementList* results) {
//	FIXME: Need to use 'results', since we can infer some defines...
	std::ostringstream s;
	indent(s, indLevel);
	s << "(*";
	appendExp(s, exp, PREC_NONE);
	s << ")(";
	StatementList::iterator ss;
	bool first = true;
	for (ss = args.begin(); ss != args.end(); ++ss) {
		if (first)
			first = false;
		else
			s << ", ";
		Exp* arg = ((Assign*)*ss)->getRight();
		appendExp(s, arg, PREC_COMMA);
	}
	s << ");";
	lines.push_back(strdup(s.str().c_str()));
}


/**
 * Adds a return statement and returns the first expression in \a rets.
 * \todo This should be returning a struct if more than one real return value.
 */
void CHLLCode::AddReturnStatement(int indLevel, StatementList* rets) {
	// FIXME: should be returning a struct of more than one real return */
	// The stack pointer is wanted as a define in calls, and so appears in returns, but needs to be removed here
	StatementList::iterator rr;
	std::ostringstream s;
	indent(s, indLevel);
	s << "return";
	int n = rets->size();

	if (n == 0 && Boomerang::get()->noDecompile && m_proc->getSignature()->getNumReturns() > 0)
		s << " eax";
	
	if (n >= 1) {
		s << " ";
		appendExp(s, ((Assign*)*rets->begin())->getRight(), PREC_NONE);
	}
	s << ";";

	if (n > 0) {
		if (n > 1)
			s << "\t/* ";
		bool first = true;
		for (rr = ++rets->begin(); rr != rets->end(); ++rr) {
			if (first)
				first = false;
			else
				s << ", ";
			appendExp(s, ((Assign*)*rr)->getRight(), PREC_NONE);
		}
		if (n > 1)
			s << " */";
	}
	lines.push_back(strdup(s.str().c_str()));
}

/**
 * Print the start of a function, and also as a comment its address.
 */
void CHLLCode::AddProcStart(UserProc* proc) {
	std::ostringstream s;
	s << "// address: 0x" << std::hex << proc->getNativeAddress() << std::dec; 
	lines.push_back(strdup(s.str().c_str()));
	AddProcDec(proc, true);
}

/// Add a prototype (for forward declaration)
void CHLLCode::AddPrototype(UserProc* proc) {
	AddProcDec(proc, false);
}

/**
 * Print the declaration of a function.
 * \param open	False if this is just a prototype and ";" should be printed instead of "{"
 * \note Special case for \em main, prints a default C main function;
 */
void CHLLCode::AddProcDec(UserProc* proc, bool open) {
	std::ostringstream s;
	if (strncmp("main", proc->getName(), 4+1) == 0) {
		// Special case for main()
		lines.push_back("int main(int argc, char* argv[], char** envp) {");
		return;
	}
	ReturnStatement* returns = proc->getTheReturnStatement();
	if (returns == NULL || returns->getNumReturns() == 0)
		s << "void ";
	else {
		Assign* firstRet = (Assign*)*returns->begin();
		Type* retType = firstRet->getType();
		if (retType == NULL || retType->isVoid())
			// There is a real return; make it integer (Remove with AD HOC type analysis)
			retType = new IntegerType();
		appendType(s, retType);
		if (!firstRet->getType()->isPointer())	// NOTE: assumes type *proc( style
			s << " ";
	}
	s << proc->getName() << "(";
	StatementList& parameters = proc->getParameters();
	StatementList::iterator pp;
	bool first = true;
	for (pp = parameters.begin(); pp != parameters.end(); ++pp) {
		if (first)
			first = false;
		else
			s << ", ";
		Assign* as = (Assign*)*pp;
		Exp* left = as->getLeft();
		Type *ty = as->getType();
		if (ty == NULL) {
			LOG << "ERROR: no type for parameter " << left << "!\n";
			ty = new IntegerType();
		}
		char* name = proc->lookupSym(left);
		if (ty->isPointer() && ((PointerType*)ty)->getPointsTo()->isArray()) {
			// C does this by default when you pass an array, i.e. you pass &array meaning array
			// Replace all m[param] with foo, param with foo, then foo with param
			ty = ((PointerType*)ty)->getPointsTo();
			Exp *foo = new Const("foo123412341234");
			m_proc->searchAndReplace(Location::memOf(left, NULL), foo);
			m_proc->searchAndReplace(left, foo);
			m_proc->searchAndReplace(foo, left);
		}
		appendTypeIdent(s, ty, name);
	}
	s << ")";
	if (open)
		s << " {";
	else
		s << ";";
	lines.push_back(strdup(s.str().c_str()));
}

/// Adds: }
void CHLLCode::AddProcEnd() {
	lines.push_back(strdup("}"));
	lines.push_back("");
}

/**
 * Declare a local variable.
 * \param last	true if an empty line should be added.
 */
void CHLLCode::AddLocal(const char *name, Type *type, bool last) {
	std::ostringstream s;
	indent(s, 1);
	appendTypeIdent(s, type, name);
	Exp *e = m_proc->expFromSymbol(name);
	if (e) {
		// ? Should never see subscripts in the back end!
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

/**
 * Add the declaration for a global.
 * \param init	The initial value of the global.
 */
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
		Type *base_type = type->isArray() ? type->asArray()->getBaseType() : type; 
		appendExp(s, init, PREC_ASSIGN, base_type->isInteger() ? !base_type->asInteger()->isSigned() : false);
		if (type->isArray())
			s << " }";
	}
	s << ";";
	lines.push_back(strdup(s.str().c_str()));
}

/// Dump all generated code to \a os.
void CHLLCode::print(std::ostream &os) {
	for (std::list<char*>::iterator it = lines.begin(); it != lines.end(); it++) 
		 os << *it << std::endl;
	if (m_proc == NULL)
		os << std::endl;
}

/// Adds one line of comment to the code.
void CHLLCode::AddLineComment(char* cmt) {
	std::ostringstream s;
	s << "/* " << cmt << "*/";
	lines.push_back(strdup(s.str().c_str()));
}

