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


class ExpSimplifier : public ExpModifier
{
public:
    ExpSimplifier() = default;
    virtual ~ExpSimplifier() = default;

public:
    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<TypedExp>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Unary>& exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Binary>& exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Ternary>& exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Location>& exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<RefExp>& exp) override;
};
