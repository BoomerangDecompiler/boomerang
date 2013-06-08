/*
 * Copyright (C) 2004, Mike Van Emmerik and Trent Waddington
 */
/***************************************************************************//**
 * \file       generic.cpp
 * OVERVIEW:   Implementation of the GenericExpTransformer and related classes.
 *============================================================================*/
/*
 * $Revision$
 * 01 Sep 06 - Mike: this was written a while ago, and is now broken because it assumes Exp::getType(), which no longer
 *    exists since ADHOC TA has been removed
 * 17 Apr 04 - Trent: Created
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <numeric>      // For accumulate
#include <algorithm>    // For std::max()
#include <map>          // In decideType()
#include <sstream>      // Need gcc 3.0 or better
#include "types.h"
#include "statement.h"
#include "cfg.h"
#include "exp.h"
#include "register.h"
#include "rtl.h"
#include "proc.h"
#include "boomerang.h"
#include "transformer.h"
#include "generic.h"
#include "log.h"

extern const char* operStrings[];

Exp *GenericExpTransformer::applyFuncs(Exp *rhs)
{
    Exp *call, *callw = Binary::get(opFlagCall, Const::get("memberAtOffset"), Terminal::get(opWild));
    if (rhs->search(callw, call)) {
        assert(call->getSubExp2()->getOper() == opList);
        Exp *p1 = applyFuncs(call->getSubExp2()->getSubExp1());
        Exp *p2 = applyFuncs(call->getSubExp2()->getSubExp2()->getSubExp1());
        assert(p1->getOper() == opTypeVal);
        assert(p2->getOper() == opIntConst);
#if 0
        Type *ty = p1->getType();
        assert(ty && ty->resolvesToCompound());
#else
        Type* ty = nullptr;        // Note: will cause a segfault
#endif
        // probably need to make this func take bits in future
        int offset = ((Const*)p2)->getInt() * 8;
        const char *member = ty->asCompound()->getNameAtOffset(offset);
        Exp *result = Const::get((char*)member);
        bool change;
        rhs = rhs->searchReplace(callw->clone(), result->clone(), change);
        assert(change);
#if 0
        LOG << "replaced " << call << " with " << result << "\n";
#endif
    }
    callw = Binary::get(opFlagCall, Const::get("offsetToMember"), Terminal::get(opWild));
    if (rhs->search(callw, call)) {
        assert(call->getSubExp2()->getOper() == opList);
        Exp *p1 = applyFuncs(call->getSubExp2()->getSubExp1());
        Exp *p2 = applyFuncs(call->getSubExp2()->getSubExp2()->getSubExp1());
        assert(p1->getOper() == opTypeVal);
        assert(p2->getOper() == opStrConst);
#if 0    // ADHOC TA
        Type *ty = p1->getType();
        assert(ty && ty->resolvesToCompound());
#else
        Type* ty = nullptr;            // Note: will cause a segfault
#endif
        const char *member = ((Const*)p2)->getStr();
        int offset = ty->asCompound()->getOffsetTo(member) / 8;
        Exp *result = new Const(offset);
        bool change;
        rhs = rhs->searchReplace(callw->clone(), result->clone(), change);
        assert(change);
#if 0
        LOG << "replaced " << call << " with " << result << "\n";
#endif
    }
    callw = Binary::get(opFlagCall, new Const("plus"), new Terminal(opWild));
    if (rhs->search(callw, call)) {
        assert(call->getSubExp2()->getOper() == opList);
        Exp *p1 = applyFuncs(call->getSubExp2()->getSubExp1());
        Exp *p2 = applyFuncs(call->getSubExp2()->getSubExp2()->getSubExp1());
        assert(p1->getOper() == opIntConst);
        assert(p2->getOper() == opIntConst);
        int a = ((Const*)p1)->getInt();
        int b = ((Const*)p2)->getInt();
        Exp *result = new Const(a + b);
        bool change;
        rhs = rhs->searchReplace(callw->clone(), result->clone(), change);
        assert(change);
#if 0
        LOG << "replaced " << call << " with " << result << "\n";
#endif
    }
    callw = Binary::get(opFlagCall, new Const("neg"), new Terminal(opWild));
    if (rhs->search(callw, call)) {
        Exp *p1 = applyFuncs(call->getSubExp2());
        assert(p1->getOper() == opIntConst);
        int a = ((Const*)p1)->getInt();
        Exp *result = new Const(-a);
        bool change;
        rhs = rhs->searchReplace(callw->clone(), result->clone(), change);
        assert(change);
#if 0
        LOG << "replaced " << call << " with " << result << "\n";
#endif
    }
    return rhs;
}

bool GenericExpTransformer::checkCond(Exp *cond, Exp *bindings)
{
    switch(cond->getOper()) {
        case opAnd:
            return checkCond(cond->getSubExp1(), bindings) &&
                   checkCond(cond->getSubExp2(), bindings);
        case opEquals:
            {
                Exp *lhs = cond->getSubExp1(), *rhs = cond->getSubExp2();
                for (Exp *l = bindings; l->getOper() != opNil; l = l->getSubExp2()) {
                    Exp *e = l->getSubExp1();
                    bool change = false;
                    lhs = lhs->searchReplaceAll(e->getSubExp1()->clone(), e->getSubExp2()->clone(), change);
#if 0
                    if (change)
                        LOG << "replaced " << e->getSubExp1() << " with " << e->getSubExp2() << "\n";
#endif
                    change = false;
                    rhs = rhs->searchReplaceAll(e->getSubExp1()->clone(), e->getSubExp2()->clone(), change);
#if 0
                    if (change)
                        LOG << "replaced " << e->getSubExp1() << " with " << e->getSubExp2() << "\n";
#endif
                }
                if (lhs->getOper() == opTypeOf) {
#if 0                // ADHOC TA
                    Type *ty = lhs->getSubExp1()->getType();
#else
                    Type* ty = nullptr;
#endif
                    if (ty == nullptr) {
#if 0
                        if (VERBOSE)
                            LOG << "no type for typeof " << lhs << "\n";
#endif
                        return false;
                    }
                    lhs = new TypeVal(ty);
#if 0
                    LOG << "got typeval: " << lhs << "\n";
#endif
                }
                if (lhs->getOper() == opKindOf) {
                    OPER op = lhs->getSubExp1()->getOper();
                    lhs = new Const(operStrings[op]);
                }
                rhs = applyFuncs(rhs);

#if 0
                LOG << "check equals in cond: " << lhs << " == " << rhs << "\n";
#endif

                if (lhs->getOper() == opVar) {
                    Exp *le;
                    for (le = bindings; le->getOper() != opNil && le->getSubExp2()->getOper() != opNil; le = le->getSubExp2())
                        ;
                    assert(le->getOper() != opNil);
                    le->setSubExp2(Binary::get(opList, Binary::get(opEquals, lhs->clone(), rhs->clone()), new Terminal(opNil)));
#if 0
                    LOG << "bindings now: " << bindings << "\n";
#endif
                    return true;
                }

                if (*lhs == *rhs)
                    return true;

#if 0            // ADHOC TA
                if (lhs->getOper() == opTypeVal && rhs->getOper() == opTypeVal &&
                    lhs->getType()->resolvesToCompound() &&
                    rhs->getType()->isCompound())
                    return true;
#endif

                Exp *new_bindings = lhs->match(rhs);
                if (new_bindings == nullptr)
                    return false;

#if 0
                LOG << "matched lhs with rhs, bindings: " << new_bindings << "\n";
#endif

                Exp *le;
                for (le = bindings; le->getOper() != opNil && le->getSubExp2()->getOper() != opNil; le = le->getSubExp2())
                    ;
                assert(le->getOper() != opNil);
                le->setSubExp2(new_bindings);

#if 0
                LOG << "bindings now: " << bindings << "\n";
#endif

                return true;
            }
        default:
            LOG << "don't know how to handle oper "
                << operStrings[cond->getOper()] << " in cond.\n";
    }
    return false;
}

Exp *GenericExpTransformer::applyTo(Exp *e, bool &bMod)
{
    bool change;
    Exp *bindings = e->match(match);
    if (bindings == nullptr) {
#if 0
        if (e->getOper() == match->getOper())
            LOG << "match failed: " << e << " with " << match << "\n";
#endif
        return e;
    }

    if (where) {
        Exp *cond = where->clone();
        if (checkCond(cond, bindings) == false)
            return e;
    }

    LOG << "applying generic exp transformer match: " << match;
    if (where)
        LOG << " where: " << where;
    LOG << " become: " << become;
    LOG << " to: " << e;
    LOG << " bindings: " << bindings << "\n";

    e = become->clone();
    for (Exp *l = bindings; l->getOper() != opNil; l = l->getSubExp2())
        e = e->searchReplaceAll(l->getSubExp1()->getSubExp1(),
                                l->getSubExp1()->getSubExp2(),
                                change);

    LOG << "calculated result: " << e << "\n";
    bMod = true;

    Exp *r;
    if (e->search(new Unary(opVar, new Terminal(opWild)), r)) {
        LOG << "error: variable " << r << " in result!\n";
        assert(false);
    }

    return e;
}

