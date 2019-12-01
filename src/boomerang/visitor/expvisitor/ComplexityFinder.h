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
 *
 */
class ComplexityFinder : public ExpVisitor
{
public:
    ComplexityFinder(UserProc *proc);
    virtual ~ComplexityFinder() = default;

public:
    int getDepth() { return m_count; }

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Unary> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Binary> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Ternary> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

private:
    int m_count = 0;
    UserProc *m_proc;
};
