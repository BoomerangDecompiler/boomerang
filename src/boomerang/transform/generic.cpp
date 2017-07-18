/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file       generic.cpp
 * OVERVIEW:   Implementation of the GenericExpTransformer and related classes.
 ******************************************************************************/

#include "generic.h"

#include "boomerang/include/transformer.h"

#include "boomerang/db/cfg.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypeVal.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/types.h"

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better


extern const char *operStrings[];

SharedExp GenericExpTransformer::applyFuncs(SharedExp rhs)
{
    SharedExp call, callw = Binary::get(opFlagCall, Const::get("memberAtOffset"), Terminal::get(opWild));

    if (rhs->search(*callw, call)) {
        assert(call->getSubExp2()->getOper() == opList);
        SharedExp p1 = applyFuncs(call->getSubExp2()->getSubExp1());
        SharedExp p2 = applyFuncs(call->getSubExp2()->getSubExp2()->getSubExp1());
        assert(p1->getOper() == opTypeVal);
        assert(p2->getOper() == opIntConst);

        SharedType ty = nullptr; // Note: will cause a segfault

        // probably need to make this func take bits in future
        int       offset = p2->access<Const>()->getInt() * 8;
        QString   member = ty->as<CompoundType>()->getNameAtOffset(offset);
        SharedExp result = Const::get(member);
        bool      change;
        rhs = rhs->searchReplace(*callw, result->clone(), change);
        assert(change);
    }

    callw = Binary::get(opFlagCall, Const::get("offsetToMember"), Terminal::get(opWild));

    if (rhs->search(*callw, call)) {
        assert(call->getSubExp2()->getOper() == opList);
        SharedExp p1 = applyFuncs(call->getSubExp2()->getSubExp1());
        SharedExp p2 = applyFuncs(call->getSubExp2()->getSubExp2()->getSubExp1());
        assert(p1->getOper() == opTypeVal);
        assert(p2->getOper() == opStrConst);
        SharedType ty = nullptr; // Note: will cause a segfault

        QString   member = p2->access<Const>()->getStr();
        int       offset = ty->as<CompoundType>()->getOffsetTo(member) / 8;
        SharedExp result = Const::get(offset);
        bool      change;
        rhs = rhs->searchReplace(*callw, result->clone(), change);
        assert(change);
    }

    callw = Binary::get(opFlagCall, Const::get("plus"), Terminal::get(opWild));

    if (rhs->search(*callw, call)) {
        assert(call->getSubExp2()->getOper() == opList);
        SharedExp p1 = applyFuncs(call->getSubExp2()->getSubExp1());
        SharedExp p2 = applyFuncs(call->getSubExp2()->getSubExp2()->getSubExp1());
        assert(p1->getOper() == opIntConst);
        assert(p2->getOper() == opIntConst);
        int       a      = p1->access<Const>()->getInt();
        int       b      = p2->access<Const>()->getInt();
        SharedExp result = Const::get(a + b);
        bool      change;
        rhs = rhs->searchReplace(*callw, result->clone(), change);
        assert(change);
    }

    callw = Binary::get(opFlagCall, Const::get("neg"), Terminal::get(opWild));

    if (rhs->search(*callw, call)) {
        SharedExp p1 = applyFuncs(call->getSubExp2());
        assert(p1->getOper() == opIntConst);
        int       a      = p1->access<Const>()->getInt();
        SharedExp result = Const::get(-a);
        bool      change;
        rhs = rhs->searchReplace(*callw, result->clone(), change);
        assert(change);
    }

    return rhs;
}


bool GenericExpTransformer::checkCond(SharedExp cond, SharedExp bindings)
{
    switch (cond->getOper())
    {
    case opAnd:
        return checkCond(cond->getSubExp1(), bindings) && checkCond(cond->getSubExp2(), bindings);

    case opEquals:
        {
            SharedExp lhs = cond->getSubExp1(), rhs = cond->getSubExp2();

            for (SharedExp l = bindings; l->getOper() != opNil; l = l->getSubExp2()) {
                SharedExp e      = l->getSubExp1();
                bool      change = false;
                lhs = lhs->searchReplaceAll(*e->getSubExp1(), e->getSubExp2()->clone(), change);

                change = false;
                rhs    = rhs->searchReplaceAll(*e->getSubExp1(), e->getSubExp2()->clone(), change);
            }

            if (lhs->getOper() == opTypeOf) {
                SharedType ty = nullptr;

                if (ty == nullptr) {
                    if (VERBOSE) {
                        LOG << "no type for typeof " << lhs << "\n";
                    }

                    return false;
                }

                lhs = TypeVal::get(ty);
                LOG << "got typeval: " << lhs << "\n";
            }

            if (lhs->getOper() == opKindOf) {
                OPER op = lhs->getSubExp1()->getOper();
                lhs = Const::get(operStrings[op]);
            }

            rhs = applyFuncs(rhs);

            if (lhs->getOper() == opVar) {
                SharedExp le;

                for (le = bindings; le->getOper() != opNil && le->getSubExp2()->getOper() != opNil; le = le->getSubExp2()) {
                }

                assert(le->getOper() != opNil);
                le->setSubExp2(Binary::get(opList, Binary::get(opEquals, lhs->clone(), rhs->clone()), Terminal::get(opNil)));
                return true;
            }

            if (*lhs == *rhs) {
                return true;
            }

            SharedExp new_bindings = lhs->match(rhs);

            if (new_bindings == nullptr) {
                return false;
            }


            SharedExp le;

            for (le = bindings; le->getOper() != opNil && le->getSubExp2()->getOper() != opNil; le = le->getSubExp2()) {
            }

            assert(le->getOper() != opNil);
            le->setSubExp2(new_bindings);

            return true;
        }

    default:
        LOG << "don't know how to handle oper " << operStrings[cond->getOper()] << " in cond.\n";
    }

    return false;
}


SharedExp GenericExpTransformer::applyTo(SharedExp e, bool& bMod)
{
    bool      change;
    SharedExp bindings = e->match(match);

    if (bindings == nullptr) {
        return e;
    }

    if (where) {
        SharedExp cond = where->clone();

        if (checkCond(cond, bindings) == false) {
            return e;
        }
    }

    LOG << "applying generic exp transformer match: " << match;

    if (where) {
        LOG << " where: " << where;
    }

    LOG << " become: " << become;
    LOG << " to: " << e;
    LOG << " bindings: " << bindings << "\n";

    e = become->clone();

    for (SharedExp l = bindings; l->getOper() != opNil; l = l->getSubExp2()) {
        e = e->searchReplaceAll(*l->getSubExp1()->getSubExp1(), l->getSubExp1()->getSubExp2(), change);
    }

    LOG << "calculated result: " << e << "\n";
    bMod = true;

    SharedExp r;

    if (e->search(Unary(opVar, Terminal::get(opWild)), r)) {
        LOG << "error: variable " << r << " in result!\n";
        assert(false);
    }

    return e;
}
