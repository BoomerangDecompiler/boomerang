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


#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class UserProc;


/**
 * Search an expression for a bad memof
 * (non subscripted or not linked with a symbol, i.e. local or parameter)
 */
class BadMemofFinder : public ExpVisitor
{
public:
    BadMemofFinder();
    virtual ~BadMemofFinder() = default;

public:
    bool isFound() { return m_found; }

public:
    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<RefExp> &exp, bool &visitChildren) override;

private:
    bool m_found;
};
