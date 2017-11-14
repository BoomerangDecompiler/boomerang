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

/**
 *
 */
class PhiStripper : public StmtModifier
{
public:
    PhiStripper(ExpModifier *em);

    bool getDelete() const { return m_del; }

    /// \copydoc StmtModifier::visit
    virtual void visit(PhiAssign *stmt, bool& visitChildren) override;

private:
    bool m_del; ///< Set true if this statment is to be deleted
};
