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


/**
 * This class visits subexpressions,
 * and if a Const, sets or clears a new conscript
 */
class ConscriptSetter : public ExpModifier
{
public:
    ConscriptSetter(int n, bool clear);
    virtual ~ConscriptSetter() = default;

public:
    int getLast() const;

    /// \copydoc ExpVisitor::preModify
    virtual SharedExp preModify(const std::shared_ptr<Const>& exp) override;

    /// \copydoc ExpVisitor::preModify
    virtual SharedExp preModify(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::preModify
    virtual SharedExp preModify(const std::shared_ptr<Binary>& exp, bool& visitChildren) override;

private:
    int m_curConscript;
    bool m_inLocalGlobal; ///< True when inside a local or global
    bool m_clear;         ///< True when clearing, not setting
};
