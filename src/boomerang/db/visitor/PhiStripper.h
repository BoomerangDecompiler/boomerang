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


#include "boomerang/db/visitor/StmtModifier.h"


class ExpModifier;


class PhiStripper : public StmtModifier
{
    bool m_del; // Set true if this statment is to be deleted

public:
    PhiStripper(ExpModifier *em)
        : StmtModifier(em)
    {
        m_del = false;
    }

    virtual void visit(PhiAssign *, bool& recur) override;

    bool getDelete() const { return m_del; }
};
