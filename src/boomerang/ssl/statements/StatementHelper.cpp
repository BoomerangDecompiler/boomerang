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
            condExp = Binary::get(op,
                                  condExp->getSubExp2()->getSubExp1()->clone(),                // P1
                                  condExp->getSubExp2()->getSubExp2()->getSubExp1()->clone()); // P2
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
             *               /      \
             *        opFlagCall    opIntConst
             *        /        \         mask
             *    Const        opList
             * "SETFFLAGS"     /    \
             *                P1    opList
             *                      /    \
             *                     P2    opNil
             */
            SharedExp flagsParam = condExp->getSubExp2()->getSubExp1();
            SharedExp test       = flagsParam;

            if (test->isSubscript()) {
                test = test->getSubExp1();
            }

            if (test->isTemp()) {
                return false; // Just not propagated yet
            }

            int mask = 0;

            if (flagsParam->getOper() == opBitAnd) {
                SharedExp setFlagsParam = flagsParam->getSubExp2();

                if (setFlagsParam->isIntConst()) {
                    mask = setFlagsParam->access<Const>()->getInt();
                }
            }

            if (!flagsParam->getSubExp1() || !flagsParam->getSubExp2() || (mask & ~0x41) != 0) {
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;
            }

            const SharedExp at_opFlagsCall_List = flagsParam->getSubExp1()->getSubExp2();
            if (!at_opFlagsCall_List || !at_opFlagsCall_List->getSubExp1() ||
                !at_opFlagsCall_List->getSubExp2() ||
                !at_opFlagsCall_List->getSubExp2()->getSubExp1()) {
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;
            }

            // Sometimes the mask includes the 0x4 bit, but we expect that to be off all the time.
            // So effectively the branch is for any one of the (one or two) bits being on. For
            // example, if the mask is 0x41, we are branching of less (0x1) or equal (0x41).
            mask &= 0x41;
            OPER _op;

            switch (mask) {
            case 0:
                LOG_WARN("Unhandled x86 branch if parity with condExp = %1", condExp);
                return false;

            case 1: _op = jtCond == BranchType::JPAR ? opLess : opGtrEq; break;

            case 0x40: _op = opEquals; break;

            case 0x41: _op = jtCond == BranchType::JPAR ? opLessEq : opGtr; break;

            default:
                _op = opWild; // Not possible, but avoid a compiler warning
                break;
            }

            condExp = Binary::get(_op, at_opFlagsCall_List->getSubExp1()->clone(),
                                  at_opFlagsCall_List->getSubExp2()->getSubExp1()->clone());
            return true; // This is a floating point comparison
        }

        default: break;
        }

        if (op != opWild) {
            condExp = Binary::get(op, condExp->getSubExp2()->getSubExp1()->clone(), Const::get(0));
        }
    }
    else if ((condOp == opFlagCall) &&
             condExp->access<Const, 1>()->getStr().startsWith("SETFFLAGS")) {
        OPER op = opWild;

        switch (jtCond) {
        case BranchType::JE: op = opEquals; break;

        case BranchType::JNE: op = opNotEqual; break;

        case BranchType::JMI: op = opLess; break;

        case BranchType::JPOS: op = opGtrEq; break;

        case BranchType::JSL: op = opLess; break;

        case BranchType::JSLE: op = opLessEq; break;

        case BranchType::JSGE: op = opGtrEq; break;

        case BranchType::JSG: op = opGtr; break;

        default: break;
        }

        if (op != opWild) {
            condExp = Binary::get(op, condExp->getSubExp2()->getSubExp1()->clone(),
                                  condExp->getSubExp2()->getSubExp2()->getSubExp1()->clone());
        }
    }
    // ICK! This is all X86 SPECIFIC... needs to go somewhere else.
    // Might be of the form (SETFFLAGS(...) & MASK) RELOP INTCONST where MASK could be a combination
    // of 1, 4, and 40, and relop could be == or ~=.  There could also be an XOR 40h after the AND
    // From MSVC 6, we can also see MASK = 0x44, 0x41, 0x5 followed by jump if (even) parity (see
    // above) %fflags = 0..0.0 00 > %fflags = 0..0.1 01 < %fflags = 1..0.0 40 = %fflags = 1..1.1 45
    // not comparable Example: (SETTFLAGS(...) & 1) ~= 0 left = SETFFLAGS(...) & 1 left1 =
    // SETFFLAGS(...) left2 = int 1, k = 0, mask = 1
    else if ((condOp == opEquals) || (condOp == opNotEqual)) {
        SharedExp left  = condExp->getSubExp1();
        SharedExp right = condExp->getSubExp2();
        bool hasXor40   = false;

        if ((left->getOper() == opBitXor) && right->isIntConst()) {
            SharedExp r2 = left->getSubExp2();

            if (r2->isIntConst()) {
                int k2 = r2->access<Const>()->getInt();

                if (k2 == 0x40) {
                    hasXor40 = true;
                    left     = left->getSubExp1();
                }
            }
        }

        if ((left->getOper() == opBitAnd) && right->isIntConst()) {
            SharedExp left1 = left->getSubExp1();
            SharedExp left2 = left->getSubExp2();
            int k           = right->access<Const>()->getInt();
            // Only interested in 40, 1
            k &= 0x41;

            if ((left1->getOper() == opFlagCall) && left2->isIntConst()) {
                int mask = left2->access<Const>()->getInt();
                // Only interested in 1, 40
                mask &= 0x41;
                OPER op = opWild;

                if (hasXor40) {
                    assert(k == 0);
                    op = condOp;
                }
                else {
                    switch (mask) {
                    case 1:

                        if (((condOp == opEquals) && (k == 0)) ||
                            ((condOp == opNotEqual) && (k == 1))) {
                            op = opGtrEq;
                        }
                        else {
                            op = opLess;
                        }

                        break;

                    case 0x40:

                        if (((condOp == opEquals) && (k == 0)) ||
                            ((condOp == opNotEqual) && (k == 0x40))) {
                            op = opNotEqual;
                        }
                        else {
                            op = opEquals;
                        }

                        break;

                    case 0x41:

                        switch (k) {
                        case 0:

                            if (condOp == opEquals) {
                                op = opGtr;
                            }
                            else {
                                op = opLessEq;
                            }

                            break;

                        case 1:

                            if (condOp == opEquals) {
                                op = opLess;
                            }
                            else {
                                op = opGtrEq;
                            }

                            break;

                        case 0x40:

                            if (condOp == opEquals) {
                                op = opEquals;
                            }
                            else {
                                op = opNotEqual;
                            }

                            break;

                        default: LOG_FATAL("k is %1", QString::number(k, 16));
                        }

                        break;

                    case 0: condExp = Terminal::get(opFalse); return false;

                    default: LOG_FATAL("Mask is %1", QString::number(mask, 16));
                    }
                }

                if (op != opWild) {
                    condExp = Binary::get(op, left1->getSubExp2()->getSubExp1(),
                                          left1->getSubExp2()->getSubExp2()->getSubExp1());
                    return true; // This is now a float comparison
                }
            }
        }
    }
    else if (condExp->isFlagCall() && condExp->access<Const, 1>()->getStr() == "SAHFFLAGS") {
        const SharedExp param = condExp->access<Exp, 2, 1>();
        if (param->isFlagCall() && param->access<Const, 1>()->getStr() == "SETFFLAGS") {
            // Can happen e.g. with the following assembly:
            //  fucompp
            //  fnstsw ax
            //  sahf
            //  jne <foo>
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
