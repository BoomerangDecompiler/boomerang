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

class UserProc;


/**
 * Search an expression for a bad memof
 * (non subscripted or not linked with a symbol, i.e. local or parameter)
 */
class BadMemofFinder : public ExpVisitor
{
public:
    BadMemofFinder();
    bool isFound() { return m_found; }

private:
    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<RefExp>& exp, bool& visitChildren) override;

private:
    bool m_found;
};
