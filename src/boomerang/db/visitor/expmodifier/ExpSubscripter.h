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


#include "boomerang/db/visitor/expmodifier/ExpModifier.h"


class Statement;


/**
 *
 */
class ExpSubscripter : public ExpModifier
{
public:
    ExpSubscripter(const SharedExp& s, Statement *d);
    virtual ~ExpSubscripter() = default;

public:
    /// \copydoc ExpModifier::preVisit
    SharedExp preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::preVisit
    SharedExp preVisit(const std::shared_ptr<Binary>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::preVisit
    SharedExp preVisit(const std::shared_ptr<RefExp>& exp, bool& visitChildren) override;

    /// \copydoc ExpModifier::preVisit
    SharedExp preVisit(const std::shared_ptr<Terminal>& exp) override;

private:
    SharedExp m_search;
    Statement *m_def;
};



