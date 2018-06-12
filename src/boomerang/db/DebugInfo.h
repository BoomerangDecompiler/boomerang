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


#include "boomerang/util/Address.h"

#include <memory>


class QString;
using SharedType = std::shared_ptr<class Type>;


namespace DebugInfo
{
    SharedType typeFromDebugInfo(const QString& name, Address addr);
}
