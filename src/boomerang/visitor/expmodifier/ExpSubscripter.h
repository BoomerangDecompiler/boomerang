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


#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/visitor/expmodifier/ExpModifier.h"


/// replaces expression e with e{def}
class BOOMERANG_API ExpSubscripter : public ExpModifier
{
public:
    ExpSubscripter(const SharedExp &s, const SharedStmt &d);
    virtual ~ExpSubscripter() = default;

public:
    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Binary> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<RefExp> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp postModify(const std::shared_ptr<Terminal> &exp) override;

private:
    SharedExp m_search;
    SharedStmt m_def;
};
