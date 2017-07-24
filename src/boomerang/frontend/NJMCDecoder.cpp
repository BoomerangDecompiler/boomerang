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

#include "NJMCDecoder.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

#include "boomerang/db/RTL.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/Proc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/statements/Assignment.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/exp/Exp.h"
#include "boomerang/db/exp/Location.h"

#include <cassert>
#include <cstdarg> // For varargs
#include <cstring>


NJMCDecoder::NJMCDecoder(Prog *prg)
    : m_prog(prg)
    , m_image(Boomerang::get()->getImage())
{
}


std::list<Instruction *> *NJMCDecoder::instantiate(Address pc, const char *name, const std::initializer_list<SharedExp>& args)
{
    // Get the signature of the instruction and extract its parts
    std::pair<QString, unsigned> sig = m_rtlDict.getSignature(name);
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

    std::list<Instruction *> *instance = m_rtlDict.instantiateRTL(opcode, pc, actuals);

    return instance;
}


SharedExp NJMCDecoder::instantiateNamedParam(char *name, const std::initializer_list<SharedExp>& args)
{
    if (m_rtlDict.ParamSet.find(name) == m_rtlDict.ParamSet.end()) {
        LOG_STREAM() << "No entry for named parameter '" << name << "'\n";
        return nullptr;
    }

    assert(m_rtlDict.DetParamMap.find(name) != m_rtlDict.DetParamMap.end());
    ParamEntry& ent = m_rtlDict.DetParamMap[name];

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


void NJMCDecoder::substituteCallArgs(char *name, SharedExp *exp, const std::initializer_list<SharedExp>& args)
{
    if (m_rtlDict.ParamSet.find(name) == m_rtlDict.ParamSet.end()) {
        LOG_STREAM() << "No entry for named parameter '" << name << "'\n";
        return;
    }

    ParamEntry& ent = m_rtlDict.DetParamMap[name];

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


SharedExp NJMCDecoder::dis_Reg(int regNum)
{
    return Location::regOf(regNum);
}


SharedExp NJMCDecoder::dis_Num(unsigned num)
{
    return Const::get(num); // TODO: what about signed values ?
}


void NJMCDecoder::processUnconditionalJump(const char *name, int size, HostAddress relocd, ptrdiff_t delta, Address pc,
                                           std::list<Instruction *> *stmts, DecodeResult& result)
{
    result.rtl      = new RTL(pc, stmts);
    result.numBytes = size;
    GotoStatement *jump = new GotoStatement();
    jump->setDest(Address((relocd - delta).value()));
    result.rtl->appendStmt(jump);
    SHOW_ASM(name << " " << relocd - delta)
}


void NJMCDecoder::processComputedJump(const char *name, int size, SharedExp dest, Address pc, std::list<Instruction *> *stmts,
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


void NJMCDecoder::processComputedCall(const char *name, int size, SharedExp dest, Address pc, std::list<Instruction *> *stmts,
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
    for (const std::pair<QString, int>& elem : m_rtlDict.RegMap) {
        if (elem.second == idx) {
            return elem.first;
        }
    }

    return QString("");
}


int NJMCDecoder::getRegSize(int idx) const
{
    auto iter = m_rtlDict.DetRegMap.find(idx);

    if (iter == m_rtlDict.DetRegMap.end()) {
        return 32;
    }

    return iter->second.getSize();
}


int NJMCDecoder::getRegIdx(const QString& name) const
{
    auto iter = m_rtlDict.RegMap.find(name);

    if (iter == m_rtlDict.RegMap.end()) {
        assert(!"Failed to find named register");
        return -1;
    }

    return iter->second;
}
