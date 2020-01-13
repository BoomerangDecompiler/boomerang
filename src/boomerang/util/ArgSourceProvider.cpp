#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ArgSourceProvider.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/statements/CallStatement.h"


ArgSourceProvider::ArgSourceProvider(CallStatement *_call)
    : call(_call)
{
    Function *procDest = call->getDestProc();

    if (procDest && procDest->isLib()) {
        src     = ArgSource::Lib;
        callSig = call->getSignature();
        n       = callSig ? callSig->getNumParams() : 0;
        i       = 0;
    }
    else if (procDest && call->getCalleeReturn() != nullptr) {
        src          = ArgSource::Callee;
        calleeParams = &static_cast<UserProc *>(procDest)->getParameters();
        pp           = calleeParams->begin();
    }
    else {
        std::shared_ptr<Signature> destSig;

        if (procDest) {
            destSig = procDest->getSignature();
        }

        if (destSig && destSig->isForced()) {
            src     = ArgSource::Lib;
            callSig = destSig;
            n       = callSig->getNumParams();
            i       = 0;
        }
        else {
            src    = ArgSource::Collector;
            defCol = call->getDefCollector();
            cc     = defCol->begin();
        }
    }
}


SharedExp ArgSourceProvider::nextArgLoc()
{
    SharedExp s;
    bool allZero;

    switch (src) {
    case ArgSource::Lib:

        if (i == n) {
            return nullptr;
        }

        s = callSig->getParamExp(i++)->clone();
        s->removeSubscripts(allZero); // e.g. m[sp{-} + 4] -> m[sp + 4]
        call->localiseComp(s);
        return s;

    case ArgSource::Callee:

        if (pp == calleeParams->end()) {
            return nullptr;
        }

        s = (*pp++)->as<Assignment>()->getLeft()->clone();
        s->removeSubscripts(allZero);

        // Localise the components. Has the effect of translating into the contect of this caller
        call->localiseComp(s);
        return s;

    case ArgSource::Collector:

        if (cc == defCol->end()) {
            return nullptr;
        }

        // Give the location, i.e. the left hand side of the assignment
        return (*cc++)->as<Assign>()->getLeft();

    default: assert(false); break;
    }

    return nullptr; // Suppress warning
}


SharedExp ArgSourceProvider::localise(SharedExp e)
{
    if (src == ArgSource::Collector) {
        // Provide the RHS of the current assignment
        SharedExp ret = (*std::prev(cc))->as<Assign>()->getRight();
        return ret;
    }

    // Else just use the call to localise
    return call->localiseExp(e);
}


SharedType ArgSourceProvider::curType(SharedExp e)
{
    Q_UNUSED(e);

    switch (src) {
    case ArgSource::Lib: return callSig->getParamType(i - 1);

    case ArgSource::Callee: {
        SharedType ty = (*std::prev(pp))->as<Assignment>()->getType();
        return ty;
    }

    case ArgSource::Collector: {
        // Mostly, there won't be a type here, I would think...
        SharedType ty = (*std::prev(cc))->getType();
        return ty;
    }

    default: assert(false); break;
    }

    return nullptr; // Suppress warning
}


bool ArgSourceProvider::exists(SharedExp e)
{
    bool allZero;

    switch (src) {
    case ArgSource::Lib:

        if (callSig->hasEllipsis()) {
            // FIXME: for now, just don't check
            return true;
        }

        for (i = 0; i < n; i++) {
            SharedExp sigParam = callSig->getParamExp(i)->clone();
            sigParam->removeSubscripts(allZero);
            call->localiseComp(sigParam);

            if (*sigParam == *e) {
                return true;
            }
        }

        return false;

    case ArgSource::Callee:
        for (pp = calleeParams->begin(); pp != calleeParams->end(); ++pp) {
            SharedExp par = (*pp)->as<Assignment>()->getLeft()->clone();
            par->removeSubscripts(allZero);
            call->localiseComp(par);

            if (*par == *e) {
                return true;
            }
        }

        return false;

    case ArgSource::Collector: return defCol->hasDefOf(e);

    default: assert(false); break;
    }

    return false; // Suppress warning
}
