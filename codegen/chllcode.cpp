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

CHLLCode::CHLLCode() : HLLCode()
{	
}

CHLLCode::CHLLCode(UserProc *p) : HLLCode(p)
{	
}

CHLLCode::~CHLLCode()
{
}

void CHLLCode::reset()
{
	for (std::list<CHLLToken*>::iterator it = tokens.begin(); it != tokens.end(); it++)
		delete *it;
	tokens.clear();
	indent = 1;
}


void CHLLCode::appendExp(Exp *exp)
{
	static Type *mrt; // most recent type
	if (exp == NULL) return;
        AssignExp *a = (AssignExp*)exp;
	Unary *u = (Unary*)exp;
	Binary *b = (Binary *)exp;
	Ternary *t = (Ternary *)exp;
	Const *c = (Const *)exp;

	switch(exp->getOper()) {
		case opPlus:        // Binary addition
		case opFPlus:       // Binary addition(single floats)
		case opFPlusd:      // addition(double floats)
		case opFPlusq:      // addition(quad floats)
			appendExp(b->getSubExp1());
			appendToken(exp, '+');
			appendExp(b->getSubExp2());
			break;
		case opMinus:       // Binary subtraction
		case opFMinus:      // Binary subtraction(single floats)
		case opFMinusd:     // subtraction(double floats)
		case opFMinusq:     // subtraction(quad floats)
			appendExp(b->getSubExp1());
			appendToken(exp, '-');
			appendExp(b->getSubExp2());
			break;
		case opMult:        // Multiplication
		case opFMult:       // Multiplication(single floats)
		case opFMultd:      // Multiplication(double floats)
		case opFMultq:      // Multiplication(quad floats)
		case opFMultsd:     // Multiplication(single floats--> double floats)
		case opFMultdq:     // Multiplication(single floats--> double floats)
		case opMults:       // Multiply signed
			appendExp(b->getSubExp1());
			appendToken(exp, '*');
			appendExp(b->getSubExp2());
			break;
		case opDiv:         // Integer division
		case opFDiv:        // (single floats)
		case opFDivd:       // Integer division(double floats)
		case opFDivq:       // division(quad floats)
		case opDivs:	    // Divide signed
			appendExp(b->getSubExp1());
			appendToken(exp, '/');
			appendExp(b->getSubExp2());
			break;
		case opMod:         // Remainder of integer division
		case opMods:        // Remainder of signed integer division
			appendExp(b->getSubExp1());
			appendToken(exp, '%');
			appendExp(b->getSubExp2());
			break;
		case opNeg:         // Unary minus
			appendToken(exp, '-');
			appendExp(u->getSubExp1());
			break;
		case opAnd:         // Logical and
			appendExp(b->getSubExp1());
			appendToken(exp, C_AND);
			appendExp(b->getSubExp2());
			break;
		case opOr:          // Logical or
			appendExp(b->getSubExp1());
			appendToken(exp, C_OR);
			appendExp(b->getSubExp2());
			break;
		case opEquals:      // Equality (logical)
			appendExp(b->getSubExp1());
			appendToken(exp, C_EQUAL);
			appendExp(b->getSubExp2());
			break;
		case opNotEqual:    // Logical !=
			appendExp(b->getSubExp1());
			appendToken(exp, C_NOTEQUAL);
			appendExp(b->getSubExp2());
			break;
		case opLess:        // Logical less than (signed)
		case opLessUns:     // Logical less than (unsigned)
			appendExp(b->getSubExp1());
			appendToken(exp, '<');
			appendExp(b->getSubExp2());
			break;
		case opGtr:         // Logical greater than (signed)
		case opGtrUns:      // Logical greater than (unsigned)
			appendExp(b->getSubExp1());
			appendToken(exp, '>');
			appendExp(b->getSubExp2());
			break;
		case opLessEq:      // Logical <= (signed)
		case opLessEqUns:   // Logical <= (unsigned)
			appendExp(b->getSubExp1());
			appendToken(exp, C_LESSEQ);
			appendExp(b->getSubExp2());
			break;
		case opGtrEq:       // Logical >= (signed)
		case opGtrEqUns:    // Logical >= (unsigned)
			appendExp(b->getSubExp1());
			appendToken(exp, C_GTREQ);
			appendExp(b->getSubExp2());
			break;
		case opNot:         // Bitwise inversion
			appendToken(exp, '~');
			appendExp(u->getSubExp1());
			break;
		case opLNot:        // Logical not
			appendToken(exp, '!');
			appendExp(u->getSubExp1());
			break;
		case opBitAnd:      // Bitwise and
			appendExp(b->getSubExp1());
			appendToken(exp, '&');
			appendExp(b->getSubExp2());
			break;
		case opBitOr:       // Bitwise or
			appendExp(b->getSubExp1());
			appendToken(exp, '|');
			appendExp(b->getSubExp2());
			break;
		case opBitXor:      // Bitwise xor
			appendExp(b->getSubExp1());
			appendToken(exp, '^');
			appendExp(b->getSubExp2());
			break;
		case opShiftL:      // Left shift
			appendExp(b->getSubExp1());
			appendToken(exp, C_SHIFTL);
			appendExp(b->getSubExp2());
			break;
		case opShiftR:      // Right shift
		case opShiftRA:     // Right shift arithmetic
			appendExp(b->getSubExp1());
			appendToken(exp, C_SHIFTR);
			appendExp(b->getSubExp2());
			break;
		case opAssignExp:      // Assignment
                    {
			// TODO: check size of the assign agrees with type
			// of the left, otherwise we need a cast (on the left, yuk!)
			appendExp(b->getSubExp1());
			appendToken(exp, '=');
			// TODO: check size of the assign agrees with type of 
			// the right, otherwise we need a cast
			appendExp(b->getSubExp2());
			break;
                    }
		case opTypedExp:    // Typed expression
			//assert(false); // not implemented
			appendExp(u->getSubExp1());
			break;
		case opTern:        // Ternary (i.e. ? : )
			appendExp(t->getSubExp1());
			appendToken(exp, '?');
			appendExp(t->getSubExp2());
			appendToken(exp, ':');
			appendExp(t->getSubExp3());
			break;
		case opAt:			// Bit extraction expr@first:last
			{
				if (t->getSubExp2()->getOper() != opIntConst ||
					t->getSubExp3()->getOper() != opIntConst) {
					appendToken(exp, C_BAD);
					break;
				}
				assert(t->getSubExp2()->getOper() == opIntConst);
				assert(t->getSubExp3()->getOper() == opIntConst);
				int first = ((Const*)t->getSubExp2())->getInt();
				int last = ((Const*)t->getSubExp3())->getInt();
				unsigned mask = 0;
				for (int i = 0; i < first - last + 1; i++)
					mask |= (1 << i);
				appendToken(exp, '(');
				appendToken(exp, '(');
				appendExp(t->getSubExp1());
				appendToken(exp, C_SHIFTR);
				appendExp(t->getSubExp3());
				appendToken(exp, ')');
				appendToken(exp, '&');
				appendExp(new Const(mask));	// leaky
				appendToken(exp, ')');
			}
			break;
		case opMemOf:       // Represents m[]
			{
				std::string s;
				TypedExp *s_exp;
				// symbol
				if (m_proc->findSymbolFor(exp, s, s_exp)) {
					appendToken(exp, C_SYM);
					break;
				} 
				// dereference
				if (m_proc->findSymbolFor(exp->getSubExp1(), s, s_exp)) {
					Type *ty = s_exp->getType();
					if (ty->isPointer()) {
						appendToken('*');
						appendToken(exp->getSubExp1(), C_SYM);
						break;
					}
				}
				// array ref
				if (exp->getSubExp1()->getOper() == opPlus &&
					m_proc->findSymbolFor(exp->getSubExp1()->getSubExp1(), s, s_exp)) {
					Type *ty = s_exp->getType();
					if (ty->isPointer()) {
						appendToken(exp->getSubExp1()->getSubExp1(), C_SYM);
						appendToken('[');
						appendToken(exp->getSubExp1()->getSubExp2(), C_INTEGER); // wrong!
						appendToken(']');
						break;
					}					
				}
				// unknown
				appendToken('*');
				appendToken('(');
				appendToken('(');
				appendToken(exp, C_INT);
				appendToken('*');
				appendToken(')');
				appendToken('(');
				appendExp(u->getSubExp1());
				appendToken(')');
				appendToken(')');				
			}
			break;
		case opRegOf:       // Represents r[]
			{				
				std::string s;
				TypedExp *s_exp;
				if (m_proc->findSymbolFor(exp, s, s_exp))
					appendToken(exp, C_SYM);
				else {					
					appendToken(exp, C_BADREG);
					appendExp(u->getSubExp1());
					appendToken(exp, C_BADREGEND);
				}
			}
			break;
		case opSubscript:       // Represents subscript
			{				
				std::string s;				
				TypedExp *s_exp;
				if (m_proc->findSymbolFor(exp, s, s_exp))
					appendToken(exp, C_SYM);
				else {					
					appendToken(exp, C_BADSUBSCRIPT);
					appendExp(b->getSubExp1());
					appendToken(exp, C_BADSUBSCRIPTMID);
					appendExp(b->getSubExp2());
					appendToken(exp, C_BADSUBSCRIPTEND);
				}
			}
			break;

		case opAddrOf:      // Represents a[]
			if (u->getSubExp1()->getOper() == opMemOf) {
				// should be done earlier
				appendExp(u->getSubExp1()->getSubExp1());
			} else {
				appendToken(exp, '&');
				appendExp(u->getSubExp1());
			} 
			break;
		case opIntConst:    // integer constant
			appendToken(exp, C_INTEGER);		
			break;
		case opFltConst:    // floating point constant
			appendToken(exp, C_FLOAT);
			break;
	        case opParam:
		case opLocal:
			appendToken(exp, C_NAME);
			break;
		case opStrConst:    // string constant
			appendToken(exp, C_STRING);
			break;
		case opNamedExp:
			appendToken(exp, C_NAME);
			break;
		case opZfill:
			appendExp(t->getSubExp3());
			break;
		case opSgnEx:
			appendExp(t->getSubExp3());
			break;
		default:
			appendToken(exp, C_BAD);
	}
}

void CHLLCode::AddPretestedLoopHeader(BasicBlock *bb, Exp *cond)
{
	appendToken(bb, C_WHILE);
	appendToken('(');
	appendExp(cond);
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddPretestedLoopEnd(BasicBlock *bb)
{
	appendToken('}');
}


void CHLLCode::AddEndlessLoopHeader(BasicBlock *bb)
{
	appendToken(bb, C_FOR);
	appendToken('(');
	appendToken(';');
	appendToken(';');
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddEndlessLoopEnd(BasicBlock *bb)
{
	appendToken('}');
}

void CHLLCode::AddPosttestedLoopHeader(BasicBlock *bb)
{
	appendToken(bb, C_DO);
	appendToken('{');
}

void CHLLCode::AddPosttestedLoopEnd(BasicBlock *bb, Exp *cond)
{
	appendToken('}');
	appendToken(bb, C_WHILE);
	appendToken('(');
	appendExp(cond);
	appendToken(')');
	appendToken(';');
}

void CHLLCode::AddCaseCondHeader(BasicBlock *bb, Exp *cond)
{
	appendToken(bb, C_SWITCH);
	appendToken('(');
	appendExp(cond);
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddCaseCondOption(BasicBlock *bb, Exp *opt)
{
	appendToken(bb, C_CASE);
	appendExp(opt);
	appendToken(':');
}

void CHLLCode::AddCaseCondOptionEnd(BasicBlock *bb)
{
	appendToken(bb, C_BREAK);
	appendToken(';');
}

void CHLLCode::AddCaseCondElse(BasicBlock *bb)
{
	appendToken(bb, C_DEFAULT);
	appendToken(':');
}

void CHLLCode::AddCaseCondEnd(BasicBlock *bb)
{
	appendToken('}');
}

void CHLLCode::AddIfCondHeader(BasicBlock *bb, Exp *cond)
{
	appendToken(bb, C_IF);
	appendToken('(');
	appendExp(cond);
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddIfCondEnd(BasicBlock *bb)
{
	appendToken('}');
}

void CHLLCode::AddIfElseCondHeader(BasicBlock *bb, Exp *cond)
{
	appendToken(bb, C_IF);
	appendToken('(');
	appendExp(cond);
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddIfElseCondOption(BasicBlock *bb)
{
	appendToken('}');
	appendToken(bb, C_ELSE);
	appendToken('{');
}

void CHLLCode::AddIfElseCondEnd(BasicBlock *bb)
{
	appendToken('}');
}

void CHLLCode::AddElseCondHeader(BasicBlock *bb, Exp *cond)
{
	appendToken(bb, C_IF);
	appendToken('(');
	appendToken('!');
	appendToken('(');
	appendExp(cond);
	appendToken(')');
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddElseCondEnd(BasicBlock *bb)
{
	appendToken('}');
}

void CHLLCode::AddGoto(BasicBlock *bb, BasicBlock *dest)
{
	dest->setLabelNeeded(true);
	appendToken(bb, C_GOTO);
	appendToken(dest, C_LABEL);
	appendToken(';');
}

void CHLLCode::AddLabel(BasicBlock *bb)
{
	if (bb->getLabelStr() == "") {
		std::ostringstream os;
		os << "L" << std::hex << bb->getLowAddr() << std::ends;
		std::string s = os.str();
		bb->setLabelStr(s);
	}
	appendToken(bb, C_LABEL);
	appendToken(':');
}

void CHLLCode::AddAssignmentStatement(BasicBlock *bb, Exp *exp)
{
	if (exp->isAssign() &&
		exp->getSubExp1()->getOper() == opRegOf && 
		exp->getSubExp1()->getSubExp1()->getOper() == opTemp) {
		// not interested in assignments to temp registers
		return;
	}

	appendToken(bb, C_STATEMENT);
	appendExp(exp);
	appendToken(';');
}

void CHLLCode::AddCallStatement(BasicBlock *bb, Exp *retloc, Proc *proc, std::vector<Exp*> &args)
{
	appendToken(bb, C_STATEMENT);
	if (retloc) {
		appendExp(retloc);
		appendToken('=');
	}
	appendToken(proc, C_CALLLABEL);
	appendToken('(');
	for (int i = 0; i < args.size(); i++) {
		if (i != 0)
			appendToken(',');
		appendExp(args[i]);	
	}
	appendToken(')');
	appendToken(';');
}

void CHLLCode::AddCallStatement(BasicBlock *bb, Exp *retloc, Exp *dest, std::vector<Exp*> &args)
{
	appendToken(bb, C_STATEMENT);
	if (retloc) {
		appendExp(retloc);
		appendToken('=');
	}
	appendToken('*');
	appendToken('(');
	appendExp(dest);
	appendToken(')');
	appendToken('(');
	for (int i = 0; i != args.size(); i++) {
		if (i != 0)
			appendToken(',');
		appendExp(args[i]);
	}
	appendToken(')');
	appendToken(';');
}

void CHLLCode::AddReturnStatement(BasicBlock *bb, Exp *ret)
{
	appendToken(bb, C_RETURN);
	if (ret) {
		appendExp(ret);
	}
	appendToken(';');
}

void CHLLCode::AddProcStart(Signature *signature)
{
	if (signature->getReturnType())
	    appendToken(signature->getReturnType(), C_TYPE);
	appendExp(new Unary(opParam, new Const((char*)signature->getName())));
	appendToken('(');
	for (int i = 0; i < signature->getNumParams(); i++) {
	    appendToken(signature->getParamType(i), C_TYPE);
	    appendExp(new Unary(opParam, 
			    new Const((char*)signature->getParamName(i))));
	    if (i != signature->getNumParams()-1 || signature->hasEllipsis())
	        appendToken(',');
	}
	if (signature->hasEllipsis())
	    appendToken(C_ELLIPSIS);
	appendToken(')');
	appendToken('{');
}

void CHLLCode::AddProcEnd()
{
	appendToken('}');
}

void CHLLCode::AddLocal(const char *name, Type *type)
{
	appendToken(type, C_TYPE);
	appendExp(new Unary(opLocal, new Const((char*)name)));
	appendToken(';');
}

void CHLLToken::appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code)
{
	char t = (char)tok;
	if (t == '=') s += " ";
	if (t == '}') {
		code->decIndent();		
		code->appendIndent(s);
	}
	s += t;
	if (t == '{') {
		code->appendNL(s);
		code->incIndent();
	}
	if (t == '}') {
		code->appendNL(s);
	}
	if (t == ';' && context != C_FOR) {
		code->appendNL(s);
	}
	if (t == ':') {
		code->appendNL(s);
	}
	if (t == ',' || t == ')') {
		s += ' ';
	}
	if (t == '=') s += " ";
}

void CControlToken::appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code)
{
	switch(tok) {
		case C_WHILE:
			code->appendIndent(s);
			s += "while ";
			context = tok;
			break;
		case C_DO:
			code->appendIndent(s);
			s += "do ";
			context = tok;
			break;
		case C_FOR:
			code->appendIndent(s);
			s += "for ";
			context = tok;
			break;
		case C_IF:
			code->appendIndent(s);
			s += "if ";
			context = tok;
			break;
		case C_ELSE:
			code->appendIndent(s);
			s += "else ";
			context = tok;
			break;
		case C_SWITCH:
			code->appendIndent(s);
			code->incIndent();
			s += "switch ";
			context = tok;
			break;
		case C_CASE:
			code->incIndent();
			code->appendIndent(s);
			s += "case ";
			context = tok;
			break;
		case C_BREAK:
			code->appendIndent(s);
			code->decIndent();
			s += "break";
			break;
		case C_DEFAULT:
			code->incIndent();
			code->appendIndent(s);
			s += "default";
			break;
		case C_ELLIPSIS:
			s += "...";
			break;
		case C_GOTO:
			code->appendIndent(s);
			s += "goto ";
			context = tok;
			break;
		case C_LABEL:
			if (!bb->isLabelNeeded() && !code->showAllLabels) {
				assert(context != C_GOTO);
				it++;
				assert((*it)->getTok() == (CTok)':');
			} else
				s += bb->getLabelStr().c_str();
			if (context == C_GOTO)
				context = (CTok)0;
			break;
		case C_STATEMENT:
			code->appendIndent(s);
			context = (CTok)0;			
			break;
		case C_RETURN:
			code->appendIndent(s);
			context = C_RETURN;
			s += "return ";
			break;
	}
}

void CDataToken::appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code)
{
	std::string sym;
	std::stringstream os;

	switch(tok) {
		case C_AND:			
			s += " && ";
			context = tok;
			break;
		case C_OR:
			s += " || ";
			context = tok;
			break;
		case C_EQUAL:
			s += " == ";
			context = tok;
			break;
		case C_NOTEQUAL:
			s += " != ";
			context = tok;
			break;
		case C_LESSEQ:
			s += " <= ";
			context = tok;
			break;
		case C_GTREQ:
			s += " >= ";
			context = tok;
			break;
		case C_SHIFTL:
			s += " << ";
			context = tok;
			break;
		case C_SHIFTR:
			s += " >> ";
			context = tok;
			break;
		case C_NAME:
			assert(exp->getSubExp1()->getOper() == opStrConst);
			s += ((Const*)exp->getSubExp1())->getStr();
			break;
		case C_SYM:
			{
				TypedExp *s_exp;
				assert(code->getProc()->findSymbolFor(exp, sym, s_exp));
				s += sym;
			}
			break;
		case C_BADMEM:
			s += "m[";
			break;
		case C_BADMEMEND:
			s += "]";
			break;
		case C_BADREG:
			s += "r[";
			break;
		case C_BADREGEND:
			s += "]";
			break;
		case C_BADSUBSCRIPT:
			break;
		case C_BADSUBSCRIPTMID:
			s += ".";
			break;
		case C_BADSUBSCRIPTEND:
			break;
		case C_BAD:
			exp->print(os);
			sym = os.str();
			s += sym;
			break;
		case C_INT:
			s += "int";
			break;
		case C_INTEGER:
			exp->print(os);
			sym = os.str();
			s += sym;			
			break;
		case C_FLOAT:
			exp->print(os);
			sym = os.str();
			s += sym;
			break;
		case C_STRING:
			exp->print(os);
			sym = os.str();
			s += sym;
			break;
		default:
			assert((int)tok < 255);
			s += (char)tok;
	}
}

void CProcToken::appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code)
{
	if (proc)
		s += proc->getName();
	else
		s += "COMPUTED";
}

void CTypeToken::appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code)
{
	s += type->getCtype();
	s += " ";
}

void CFmtToken::appendString(std::string &s, CTok &context, std::list<CHLLToken*>::iterator &it, CHLLCode *code)
{
	s += str;
}

void CHLLCode::appendIndent(std::string &s)
{
	for (int i = 0; i < indent; i++)
		s += "\t";
}

void CHLLCode::appendNL(std::string &s)
{
#ifdef WIN32
		s += "\r\n";
#else
		s += "\n";
#endif
}

void CHLLCode::toString(std::string &s)
{
	s = "";
	indent = 0;
	CTok context = (CTok)0;
	for (std::list<CHLLToken*>::iterator it = tokens.begin(); it != tokens.end(); it++) {
		(*it)->setPos(s.length());
		(*it)->appendString(s, context, it, this);	
	}
}

bool CHLLCode::isLabelAt(int nCharIndex)
{
	CHLLToken *tok = NULL;
	for (std::list<CHLLToken*>::iterator it = tokens.begin(); it != tokens.end(); it++) {
		if ((*it)->getPos() == nCharIndex) { tok = *it; break; }
		if ((*it)->getPos() > nCharIndex) break;
		tok = *it;
	}
	if (tok && tok->getTok() == C_LABEL) return true;
	return false;	
}

BasicBlock *CHLLCode::getBlockAt(int nCharIndex)
{
	BasicBlock *bb = NULL;
	for (std::list<CHLLToken*>::iterator it = tokens.begin(); it != tokens.end(); it++) {
		if ((*it)->getBasicBlock()) bb = (*it)->getBasicBlock();
		if ((*it)->getPos() >= nCharIndex) break;
	}
	return bb;
}

Exp *CHLLCode::getExpAt(int nCharIndex)
{
	Exp *e = NULL;
	for (std::list<CHLLToken*>::iterator it = tokens.begin(); it != tokens.end(); it++) {
		if ((*it)->getPos() >= nCharIndex) break;
		if ((*it)->getExp()) { 
			// only another subscripted exp can replace a subscripted exp
			if (e && e->getOper() == opSubscript) {
				if ((*it)->getExp()->getOper() == opSubscript)
					e = (*it)->getExp();
			} else
				e = (*it)->getExp();
		}
	}
	return e;
}

Proc *CHLLCode::getCallAt(int nCharIndex)
{
	Proc *c = NULL;
	for (std::list<CHLLToken*>::iterator it = tokens.begin(); it != tokens.end(); it++) {
		int pos = (*it)->getPos();
		if (pos >= nCharIndex) break;
		if ((*it)->getCall()) c = (*it)->getCall();
	}
	return c;
}

void CHLLCode::addFormating(int nCharIndex, const char *str)
{
	std::list<CHLLToken*>::iterator it;
	for (it = tokens.begin(); it != tokens.end(); it++) {
		int pos = (*it)->getPos();
		if (pos >= nCharIndex) break;
	}
	if ((*it)->getTok() == C_FMT)
		((CFmtToken*)(*it))->append(str);
	else
		tokens.insert(it, new CFmtToken(str));
}
