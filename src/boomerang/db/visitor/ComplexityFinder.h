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
 *
 */
class ComplexityFinder : public ExpVisitor
{
public:
    ComplexityFinder(UserProc *proc);
    int getDepth() { return m_count; }

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Unary>&, bool& override) override;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Binary>&, bool& override) override;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Ternary>&, bool& override) override;

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

private:
    int m_count;
    UserProc *m_proc;
};
