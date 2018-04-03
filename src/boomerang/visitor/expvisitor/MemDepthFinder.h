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


/// Used by range analysis
class MemDepthFinder : public ExpVisitor
{
public:
    MemDepthFinder();
    virtual ~MemDepthFinder() = default;

public:
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    int getDepth() { return depth; }

private:
    int depth;
};


