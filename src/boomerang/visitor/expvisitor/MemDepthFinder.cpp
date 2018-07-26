#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MemDepthFinder.h"


#include "boomerang/ssl/exp/Location.h"


MemDepthFinder::MemDepthFinder()
    : depth(0)
{
}


bool MemDepthFinder::preVisit(const std::shared_ptr<Location>& e, bool& visitChildren)
{
    if (e->isMemOf()) {
        ++depth;
    }

    visitChildren = true;
    return true;
}

