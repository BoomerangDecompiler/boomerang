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


/**
 * Search an expression for flags calls, e.g. SETFFLAGS(...) & 0x45
 */
class BOOMERANG_API FlagsFinder : public ExpVisitor
{
public:
    FlagsFinder();
    virtual ~FlagsFinder() = default;

public:
    bool isFound() { return m_found; }

    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Binary> &exp, bool &visitChildren) override;

private:
    bool m_found;
};
