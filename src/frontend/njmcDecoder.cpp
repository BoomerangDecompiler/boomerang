/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       njmcDecoder.cpp
 * \brief   This file contains the machine independent decoding functionality.
 ******************************************************************************/

#include "njmcDecoder.h"
#include "db/rtl.h"
#include "db/exp.h"
#include "db/register.h"
#include "db/cfg.h"
#include "db/proc.h"
#include "db/prog.h"
#include "boom_base/BinaryFile.h"
#include "boom_base/log.h"
#include "include/util.h"

#include <cassert>
#include <cstdarg> // For varargs
#include <cstring>

/**********************************
* NJMCDecoder methods.
**********************************/

/***************************************************************************/ /**
 * \fn       NJMCDecoder::NJMCDecoder
 * \brief
 * \param       prog: Pointer to the Prog object
 *
 ******************************************************************************/
NJMCDecoder::NJMCDecoder(Prog *prg)
	: prog(prg)
	, Image(Boomerang::get()->getImage())
{
}


/***************************************************************************/ /**
 * \brief   Given an instruction name and a variable list of expressions representing the actual operands of
 *              the instruction, use the RTL template dictionary to return the instantiated RTL representing the
 *              semantics of the instruction. This method also displays a disassembly of the instruction if the
 *              relevant compilation flag has been set.
 * \param   pc  native PC
 * \param   name - instruction name
 * \param   ... - Semantic String ptrs representing actual operands
 * \returns an instantiated list of Exps
 ******************************************************************************/
std::list<Instruction *> *NJMCDecoder::instantiate(ADDRESS pc, const char *name, const std::initializer_list<SharedExp>& args)
{
	// Get the signature of the instruction and extract its parts
	std::pair<QString, unsigned> sig = RTLDict.getSignature(name);
	QString      opcode      = sig.first;
	unsigned int numOperands = sig.second;
	assert(numOperands == args.size());
	Q_UNUSED(numOperands);

	// Put the operands into a vector
	std::vector<SharedExp> actuals(args);

	if (DEBUG_DECODER) {
		QTextStream q_cout(stdout);
		// Display a disassembly of this instruction if requested
		q_cout << pc << ": " << name << " ";

		for (const SharedExp& itd : actuals) {
			if (itd->isIntConst()) {
				int val = itd->access<Const>()->getInt();

				if ((val > 100) || (val < -100)) {
					q_cout << "0x" << QString::number(val, 16);
				}
				else {
					q_cout << val;
				}
			}
			else {
				itd->print(q_cout);
			}
		}

		q_cout << '\n';
	}

	std::list<Instruction *> *instance = RTLDict.instantiateRTL(opcode, pc, actuals);

	return instance;
}


/***************************************************************************/ /**
 * \brief   Similarly to NJMCDecoder::instantiate, given a parameter name and a list of Exp*'s representing
 * sub-parameters, return a fully substituted Exp for the whole expression
 * \note    Caller must delete result
 * \param   name - parameter name
 *          ... - Exp* representing actual operands
 * \returns an instantiated list of Exps
 ******************************************************************************/
SharedExp NJMCDecoder::instantiateNamedParam(char *name, const std::initializer_list<SharedExp>& args)
{
	if (RTLDict.ParamSet.find(name) == RTLDict.ParamSet.end()) {
		LOG_STREAM() << "No entry for named parameter '" << name << "'\n";
		return nullptr;
	}

	assert(RTLDict.DetParamMap.find(name) != RTLDict.DetParamMap.end());
	ParamEntry& ent = RTLDict.DetParamMap[name];

	if ((ent.m_kind != PARAM_ASGN) && (ent.m_kind != PARAM_LAMBDA)) {
		LOG_STREAM() << "Attempt to instantiate expressionless parameter '" << name << "'\n";
		return nullptr;
	}

	// Start with the RHS
	assert(ent.m_asgn->getKind() == STMT_ASSIGN);
	SharedExp result   = ((Assign *)ent.m_asgn)->getRight()->clone();
	auto      arg_iter = args.begin();

	for (auto& elem : ent.m_params) {
		Location  formal(opParam, Const::get(elem), nullptr);
		SharedExp actual = *arg_iter++;
		bool      change;
		result = result->searchReplaceAll(formal, actual, change);
	}

	return result;
}


/***************************************************************************/ /**
 * \brief   In the event that it's necessary to synthesize the call of a named parameter generated with
 *          instantiateNamedParam(), this substituteCallArgs() will substitute the arguments that follow into
 *          the expression.
 * \note    Should only be used after instantiateNamedParam(name, ..);
 * \note    exp (the pointer) could be changed
 * \param   name - parameter name
 * \param   exp - expression to instantiate into
 * \param   ... - Exp* representing actual operands
 * \returns an instantiated list of Exps
 ******************************************************************************/
void NJMCDecoder::substituteCallArgs(char *name, SharedExp *exp, const std::initializer_list<SharedExp>& args)
{
	if (RTLDict.ParamSet.find(name) == RTLDict.ParamSet.end()) {
		LOG_STREAM() << "No entry for named parameter '" << name << "'\n";
		return;
	}

	ParamEntry& ent = RTLDict.DetParamMap[name];

	/*if (ent.kind != PARAM_ASGN && ent.kind != PARAM_LAMBDA) {
	 *          LOG_STREAM() << "Attempt to instantiate expressionless parameter '" << name << "'\n";
	 *          return;
	 *  }*/
	auto arg_iter = args.begin();

	for (auto& elem : ent.m_funcParams) {
		Location  formal(opParam, Const::get(elem), nullptr);
		SharedExp actual = *arg_iter++;
		bool      change;
		*exp = (*exp)->searchReplaceAll(formal, actual, change);
	}
}


/***************************************************************************/ /**
 * These are functions used to decode instruction operands into
 * Exp*s.
 ******************************************************************************/

/***************************************************************************/ /**
 * \brief   Converts a numbered register to a suitable expression.
 * \param   regNum - the register number, e.g. 0 for eax
 * \returns the Exp* for the register NUMBER (e.g. "int 36" for %f4)
 ******************************************************************************/
SharedExp NJMCDecoder::dis_Reg(int regNum)
{
	return Location::regOf(regNum);
}


/***************************************************************************/ /**
 * \brief        Converts a number to a Exp* expression.
 * \param        num - a number
 * \returns             the Exp* representation of the given number
 ******************************************************************************/
SharedExp NJMCDecoder::dis_Num(unsigned num)
{
	return Const::get(num); // TODO: what about signed values ?
}


/***************************************************************************/ /**
 * \brief   Process an unconditional jump instruction
 *              Also check if the destination is a label (MVE: is this done?)
 * \param   name name of instruction (for debugging)
 * \param   size size of instruction in bytes
 * \param   relocd
 * \param   delta
 * \param   pc native pc
 * \param   stmts list of statements (?)
 * \param   result ref to decoder result object
 ******************************************************************************/
void NJMCDecoder::unconditionalJump(const char *name, int size, ADDRESS relocd, ptrdiff_t delta, ADDRESS pc,
									std::list<Instruction *> *stmts, DecodeResult& result)
{
	result.rtl      = new RTL(pc, stmts);
	result.numBytes = size;
	GotoStatement *jump = new GotoStatement();
	jump->setDest((relocd - delta).native());
	result.rtl->appendStmt(jump);
	SHOW_ASM(name << " 0x" << relocd - delta)
}


/***************************************************************************/ /**
 * \brief   Process an indirect jump instruction
 * \param   name name of instruction (for debugging)
 * \param   size size of instruction in bytes
 * \param   dest destination Exp*
 * \param   pc native pc
 * \param   stmts list of statements (?)
 * \param   result ref to decoder result object
 ******************************************************************************/
void NJMCDecoder::computedJump(const char *name, int size, SharedExp dest, ADDRESS pc, std::list<Instruction *> *stmts,
							   DecodeResult& result)
{
	result.rtl      = new RTL(pc, stmts);
	result.numBytes = size;
	GotoStatement *jump = new GotoStatement();
	jump->setDest(dest);
	jump->setIsComputed(true);
	result.rtl->appendStmt(jump);
	SHOW_ASM(name << " " << dest)
}


/***************************************************************************/ /**
 * \brief   Process an indirect call instruction
 * \param   name name of instruction (for debugging)
 * \param   size size of instruction in bytes
 * \param   dest destination Exp*
 * \param   pc native pc
 * \param   stmts list of statements (?)
 * \param   result ref to decoder result object
 ******************************************************************************/
void NJMCDecoder::computedCall(const char *name, int size, SharedExp dest, ADDRESS pc, std::list<Instruction *> *stmts,
							   DecodeResult& result)
{
	result.rtl      = new RTL(pc, stmts);
	result.numBytes = size;
	CallStatement *call = new CallStatement();
	call->setDest(dest);
	call->setIsComputed(true);
	result.rtl->appendStmt(call);
	SHOW_ASM(name << " " << dest)
}


QString NJMCDecoder::getRegName(int idx) const
{
	static QString nullres;

	for (const std::pair<QString, int>& elem : RTLDict.RegMap) {
		if (elem.second == idx) {
			return elem.first;
		}
	}

	return nullres;
}


int NJMCDecoder::getRegSize(int idx) const
{
	auto iter = RTLDict.DetRegMap.find(idx);

	if (iter == RTLDict.DetRegMap.end()) {
		return 32;
	}

	return iter->second.getSize();
}


int NJMCDecoder::getRegIdx(const QString& name) const
{
	auto iter = RTLDict.RegMap.find(name);

	if (iter == RTLDict.RegMap.end()) {
		assert(!"Failed to find named register");
		return -1;
	}

	return iter->second;
}
