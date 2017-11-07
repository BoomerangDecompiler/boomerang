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


#include "boomerang/db/visitor/ExpVisitor.h"


/**
 * This class visits subexpressions,
 * and if a Const, sets or clears a new conscript
 */
class ConscriptSetter : public ExpVisitor
{
public:
    ConscriptSetter(int n, bool clear);

    int getLast() const;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Const>& exp) override;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Location>& exp, bool& dontVisitChildren) override;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Binary>& exp, bool& dontVisitChildren) override;

private:
    int m_curConscript;
    bool m_bInLocalGlobal; ///< True when inside a local or global
    bool m_bClear;         ///< True when clearing, not setting
};
