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
 * ExpTransformer for removing double indirection fro  expressions.
 */
class RDIExpTransformer : public ExpTransformer
{
public:
    RDIExpTransformer() {}
    virtual SharedExp applyTo(SharedExp e, bool& bMod) override;
};
