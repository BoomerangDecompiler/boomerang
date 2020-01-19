#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementHelper.h"

#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/util/log/Log.h"

#include <cassert>


// Common to BranchStatement and BoolAssign
// Return true if this is now a floating point Branch
bool condToRelational(SharedExp &condExp, BranchType jtCond)
{
    condExp     = condExp->simplifyArith()->simplify();
    OPER condOp = condExp->getOper();

    if ((condOp == opFlagCall) && condExp->access<Const, 1>()->getStr().startsWith("SUBFLAGS")) {
        OPER op = opWild;
        // Special for PPC unsigned compares; may be other cases in the future
        bool makeUns = condExp->access<Const, 1>()->getStr().startsWith("SUBFLAGSNL");

        switch (jtCond) {
        case BranchType::JE: op = opEquals; break;
        case BranchType::JNE: op = opNotEqual; break;
        case BranchType::JSL: op = makeUns ? opLessUns : opLess; break;
        case BranchType::JSLE: op = makeUns ? opLessEqUns : opLessEq; break;
        case BranchType::JSGE: op = makeUns ? opGtrEqUns : opGtrEq; break;
        case BranchType::JSG: op = makeUns ? opGtrUns : opGtr; break;

        case BranchType::JUL: op = opLessUns; break;

        case BranchType::JULE: op = opLessEqUns; break;

        case BranchType::JUGE: op = opGtrEqUns; break;

        case BranchType::JUG: op = opGtrUns; break;

        case BranchType::JMI:
            /*
             *       condExp
             *      /      \
             *  Const      opList
             * "SUBFLAGS"  /    \
             *            P1    opList
             *                  /    \
             *                 P2    opList
             *                       /    \
             *                      P3    opNil
             */
            condExp = Binary::get(opLess, // P3 < 0
                                  condExp->access<const Exp, 2, 2, 2, 1>()->clone(), Const::get(0));
            break;

        case BranchType::JPOS:
            condExp = Binary::get(opGtrEq, // P3 >= 0
                                  condExp->access<const Exp, 2, 2, 2, 1>()->clone(), Const::get(0));
            break;

        case BranchType::JOF:
        case BranchType::JNOF:
        case BranchType::JPAR:
        case BranchType::JNPAR: break;

        case BranchType::INVALID: assert(false); break;
        }

        if (op != opWild) {
            condExp = Binary::get(op, condExp->access<Exp, 2, 1>()->clone(), // P1
                                  condExp->access<Exp, 2, 2, 1>()->clone()); // P2
        }
    }
    else if (condOp == opFlagCall &&
             (condExp->access<Const, 1>()->getStr().startsWith("LOGICALFLAGS") ||
              condExp->access<Const, 1>()->getStr().startsWith("INCDECFLAGS"))) {
        OPER op = opWild;

        switch (jtCond) {
        case BranchType::JE: op = opEquals; break;

        case BranchType::JNE: op = opNotEqual; break;

        case BranchType::JMI: op = opLess; break;

        case BranchType::JPOS: op = opGtrEq; break;

        // FIXME: This next set is quite shakey. Really, we should pull all the individual flag
        // definitions out of the flag definitions, and substitute these into the equivalent
        // conditions for the branches (a big, ugly job).
        case BranchType::JSL: op = opLess; break;

        case BranchType::JSLE: op = opLessEq; break;

        case BranchType::JSGE: op = opGtrEq; break;

        case BranchType::JSG: op = opGtr; break;

        // These next few seem to fluke working fine on architectures like x86 which
        // clear the carry on all logical operations.
        case BranchType::JUL:
            // NOTE: this is equivalent to never branching, since nothing
            // can be unsigned less than zero
            op = opLessUns;
            break;

        case BranchType::JULE: op = opLessEqUns; break;

        case BranchType::JUGE:
            op = opGtrEqUns;
            break; // Similarly, this is equivalent to always branching

        case BranchType::JUG: op = opGtrUns; break;

        case BranchType::JPAR:
        case BranchType::JNPAR: {
            // This is x86 specific too; see below for more notes.

            /*
             *              condExp
             *              /     \
             *          Const      opList
             * "LOGICALFLAGS8"     /    \
             *               opBitAnd    opNil
             *             (flagsParam)
             *               /      \
             *        opFlagCall    opIntConst
             *        /        \         (mask)
             *    Const        opList
             * "SETFFLAGS"     /    \
             *                P1    opList
             *                      /    \
             *                     P2    opNil
             */
            const SharedExp flagsParam = condExp->access<Exp, 2, 1>();

            if (flagsParam->isTemp() ||
                (flagsParam->isSubscript() && flagsParam->access<Exp, 1>()->isTemp())) {
                return false;
            }
            else if (flagsParam->getOper() != opBitAnd || !flagsParam->getSubExp2()->isIntConst()) {
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;
            }
            else if (!flagsParam->getSubExp1()->isFlagCall() ||
                     flagsParam->access<Const, 1, 1>()->getStr() != "SETFFLAGS") {
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;
            }

            const int mask = flagsParam->access<Const, 2>()->getInt();
            if (mask == 0 || (mask & ~0x41) != 0) {
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;
            }
            else if ((flagsParam->access<Exp, 1, 2>()->getOper() != opList) ||
                     (flagsParam->access<Exp, 1, 2, 2>()->getOper() != opList) ||
                     (flagsParam->access<Exp, 1, 2, 2, 2>()->getOper() != opNil)) {
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;
            }

            const SharedExp P1 = flagsParam->access<Exp, 1, 2, 1>();
            const SharedExp P2 = flagsParam->access<Exp, 1, 2, 2, 1>();

            // Sometimes the mask includes the 0x4 bit, but we expect that to be off all the time.
            // So effectively the branch is for any one of the (one or two) bits being on. For
            // example, if the mask is 0x41, we are branching of less (0x1) or equal (0x40).

            switch (mask) {
            case 1: {
                condExp = Binary::get(jtCond == BranchType::JPAR ? opLess : opGtrEq, P1->clone(),
                                      P2->clone());
                return true;
            }

            case 0x40: {
                condExp = Binary::get(opEquals, P1->clone(), P2->clone());
                return true;
            }

            case 0x41: {
                condExp = Binary::get(jtCond == BranchType::JPAR ? opLessEq : opGtr, P1->clone(),
                                      P2->clone());
                return true;
            }

            default: assert(false); break;
            }
        }

        case BranchType::JOF:
        case BranchType::JNOF: break;

        case BranchType::INVALID: assert(false);
        }

        if (op != opWild) {
            condExp = Binary::get(op, condExp->getSubExp2()->getSubExp1()->clone(), Const::get(0));
        }
    }
    else if ((condOp == opFlagCall) &&
             condExp->access<Const, 1>()->getStr().startsWith("SETFFLAGS")) {
        OPER op = opWild;

        // clang-format off
        switch (jtCond) {
        case BranchType::JE:   op = opEquals;   break;
        case BranchType::JNE:  op = opNotEqual; break;
        case BranchType::JMI:  op = opLess;     break;
        case BranchType::JPOS: op = opGtrEq;    break;
        case BranchType::JSL:  op = opLess;     break;
        case BranchType::JSLE: op = opLessEq;   break;
        case BranchType::JSGE: op = opGtrEq;    break;
        case BranchType::JSG:  op = opGtr;      break;
        default: break;
        }

        if (op != opWild) {
            condExp = Binary::get(op, condExp->access<Exp, 2, 1>()->clone(),
                                  condExp->access<Exp, 2, 2, 1>()->clone());
        }
    }
    // ICK! This is all X86 SPECIFIC... needs to go somewhere else.
    // Might be of the form (SETFFLAGS(...) & MASK) RELOP INTCONST where MASK could be a combination
    // of 1, 4, and 40, and relop could be == or ~=.
    // There could also be an XOR 40h after the AND from MSVC 6, we can also see
    // MASK = 0x44, 0x41, 0x5 followed by jump if (even) parity (see above)
    // %fflags = 0..0.0 00 > %fflags = 0..0.1 01 < %fflags = 1..0.0 40 = %fflags = 1..1.1 45
    // not comparable
    // Example: (SETTFLAGS(...) & 1) ~= 0
    //  left = SETFFLAGS(...) & 1
    //  left1 = SETFFLAGS(...)
    //  left2 = int 1, k = 0, mask = 1
    else if ((condOp == opEquals) || (condOp == opNotEqual)) {
        SharedExp cmpLhs = condExp->getSubExp1();
        SharedExp cmpRhs = condExp->getSubExp2();
        bool hasXor40    = false;

        if ((cmpLhs->getOper() == opBitXor) && cmpRhs->isIntConst()) {
            if (cmpLhs->getSubExp2()->isIntConst()) {
                if (cmpLhs->access<Const, 2>()->getInt() == 0x40) {
                    hasXor40 = true;
                    cmpLhs     = cmpLhs->getSubExp1();
                }
            }
        }

        if (cmpLhs->getOper() != opBitAnd || !cmpRhs->isIntConst()) {
            return false;
        }

        SharedExp andLhs = cmpLhs->getSubExp1();
        SharedExp andRhs = cmpLhs->getSubExp2();
        const int k      = cmpRhs->access<Const>()->getInt() & 0x41; // Only interested in 40, 1

        if ((andLhs->getOper() != opFlagCall) || !andRhs->isIntConst()) {
            return false;
        }
        else if (hasXor40) {
            assert(k == 0);
            condExp = Binary::get(condOp, andLhs->access<Exp, 2, 1>(),
                                  andLhs->access<Exp, 2, 2, 1>());
            return true;
        }

        // Only interested in 1, 40
        const int mask = andRhs->access<Const>()->getInt() & 0x41;
        OPER op = opWild;

        switch (mask) {
        case 1:
            if ((condOp == opEquals && k == 0) || (condOp == opNotEqual && k == 1)) {
                op = opGtrEq;
            }
            else {
                op = opLess;
            }
            break;

        case 0x40:
            if ((condOp == opEquals && k == 0) ||(condOp == opNotEqual && k == 0x40)) {
                op = opNotEqual;
            }
            else {
                op = opEquals;
            }
            break;

        case 0x41:
            switch (k) {
            case 0: op = (condOp == opEquals) ? opGtr : opLessEq; break;
            case 1: op = (condOp == opEquals) ? opLess : opGtrEq; break;
            case 0x40: op = (condOp == opEquals) ? opEquals : opNotEqual; break;
            default: LOG_FATAL("k is %1", QString::number(k, 16));
            }

            break;

        case 0: condExp = Terminal::get(opFalse); return false;

        default: LOG_FATAL("Mask is %1", QString::number(mask, 16));
        }

        if (op != opWild) {
            condExp = Binary::get(op, andLhs->access<Exp, 2, 1>(),
                                    andLhs->access<Exp, 2, 2, 1>());
            return true; // This is now a float comparison
        }
    }
    else if (condExp->isFlagCall() && condExp->access<Const, 1>()->getStr() == "SAHFFLAGS") {
        if (condExp->getSubExp2()->getOper() == opNil || condExp->access<Exp, 2, 2>()->getOper() != opNil) {
            LOG_WARN("Unhandled x86 branch with condExp = %1", condExp);
            return false;
        }

        const SharedExp param = condExp->access<Exp, 2, 1>();
        if (param->isFlagCall() && param->access<Const, 1>()->getStr() == "SETFFLAGS") {
            // Can happen e.g. with the following assembly:
            //  fucompp
            //  fnstsw ax
            //  sahf
            //  jne <foo>

            if (param->getSubExp2()->getOper() == opNil || param->access<Exp, 2, 2>()->getOper() == opNil ||
                param->access<Const, 2, 2, 2>()->getOper() != opNil) {
                LOG_WARN("Unhandled x86 branch with condExp = %1", condExp);
                return false;
            }

            const SharedExp floatParam1 = param->access<Const, 2, 1>();
            const SharedExp floatParam2 = param->access<Const, 2, 2, 1>();

            // We have:
            // %ZF = floatParam1 == floatParam2
            // %CF = floatParam1 <  floatParam2
            // %PF = 0 (if not unordered)
            switch (jtCond) {
            case BranchType::JNE: { // ~%ZF
                condExp = Binary::get(opNotEqual, floatParam1, floatParam2);
                return true;
            }
            case BranchType::JE: { // %ZF
                condExp = Binary::get(opEquals, floatParam1, floatParam2);
                return true;
            }
            case BranchType::JPAR: { // %PF
                condExp = Terminal::get(opFalse);
                return true;
            }
            case BranchType::JNPAR: { // ~%PF
                condExp = Terminal::get(opTrue);
                return true;
            }
            case BranchType::JUL: { // %CF
                condExp = Binary::get(opLess, floatParam1, floatParam2);
                return true;
            }
            case BranchType::JUGE: { // ~%CF
                condExp = Binary::get(opGtrEq, floatParam1, floatParam2);
                return true;
            }
            case BranchType::JULE: { // %CF | %ZF
                condExp = Binary::get(opLessEq, floatParam1, floatParam2);
                return true;
            }
            case BranchType::JUG: { // ~%CF & ~%ZF
                condExp = Binary::get(opGtr, floatParam1, floatParam2);
                return true;
            }
            default: break;
            }
        }
    }

    return false;
}
