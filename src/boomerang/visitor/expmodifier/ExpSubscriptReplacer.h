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
#include "boomerang/ssl/statements/Statement.h"


/// replaces the subscript in e{foo} -> e{bar}
class BOOMERANG_API ExpSubscriptReplacer : public ExpModifier
{
public:
    ExpSubscriptReplacer(const SharedConstStmt &original, const SharedStmt &replacement);
    virtual ~ExpSubscriptReplacer() = default;

public:
    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<RefExp> &exp, bool &visitChildren) override;

private:
    SharedConstStmt m_orig;
    SharedStmt m_replacement;
};
