#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/transform/ExpTransformer.h"


/**
 * Provides the definition for the generic exp tranformer.
 */
class GenericExpTransformer : public ExpTransformer
{
protected:
    SharedExp match, where, become;

    bool checkCond(SharedExp cond, SharedExp bindings);
    SharedExp applyFuncs(SharedExp rhs);

public:
    GenericExpTransformer(SharedExp _match, SharedExp _where, SharedExp _become)
        : match(_match)
        , where(_where)
        , become(_become) {}
    virtual SharedExp applyTo(SharedExp e, bool& bMod) override;
};
