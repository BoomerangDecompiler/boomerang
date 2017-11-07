#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FlagsFinder.h"


#include "boomerang/db/exp/Binary.h"


FlagsFinder::FlagsFinder()
    : m_found(false)
{
}


bool FlagsFinder::visit(const std::shared_ptr<Binary>& e, bool& override)
{
    if (e->isFlagCall()) {
        m_found = true;
        return false; // Don't continue searching
    }

    override = false;
    return true;
}
