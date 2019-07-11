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

#include <list>


/**
 * This is the code (apart from definitions) to find all constants in a Statement
 */
class BOOMERANG_API ConstFinder : public ExpVisitor
{
public:
    ConstFinder(std::list<std::shared_ptr<Const>> &results);
    virtual ~ConstFinder() = default;

public:
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

    /// \copydoc ExpVisitor::preVisit
    virtual bool visit(const std::shared_ptr<Const> &exp) override;

private:
    std::list<std::shared_ptr<Const>> &m_constList;
};
