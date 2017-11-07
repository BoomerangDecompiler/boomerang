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

#include <list>

/**
 *
 */
class ConstFinder : public ExpVisitor
{
public:
    ConstFinder(std::list<std::shared_ptr<Const> >& _lc);

    virtual ~ConstFinder() override = default;

    // This is the code (apart from definitions) to find all constants in a Statement
    virtual bool visit(const std::shared_ptr<Const>& e) override;
    virtual bool visit(const std::shared_ptr<Location>& e, bool& override) override;

private:
    std::list<std::shared_ptr<Const> >& m_constList;
};
