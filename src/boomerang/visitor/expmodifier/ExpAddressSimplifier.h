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


#include "boomerang/visitor/expmodifier/ExpModifier.h"


class ExpAddressSimplifier : public ExpModifier
{
public:
    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Unary>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Location>& exp, bool& visitChildren) override;
};
