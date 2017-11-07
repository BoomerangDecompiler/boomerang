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
 * Search an expression for flags calls, e.g. SETFFLAGS(...) & 0x45
 */
class FlagsFinder : public ExpVisitor
{
public:
    FlagsFinder();

    bool isFound() { return m_found; }

    virtual bool visit(const std::shared_ptr<Binary>& e, bool& override) override;

private:
    bool m_found;
};
