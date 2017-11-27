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


#include "boomerang/db/visitor/SimpExpModifier.h"


/**
 * A class to propagate everything, regardless, to this expression. Does not consider memory expressions and whether
 * the address expression is primitive. Use with caution; mostly Statement::propagateTo() should be used.
 */
class ExpPropagator : public SimpExpModifier
{
public:
    ExpPropagator();
    virtual ~ExpPropagator() = default;

public:
    bool isChanged() { return m_changed; }
    void clearChanged() { m_changed = false; }

    /// \copydoc SimpExpModifier::postVisit
    // Ugh! This is still a separate propagation mechanism from Statement::propagateTo()
    SharedExp postVisit(const std::shared_ptr<RefExp>& exp) override;

private:
    bool m_changed;
};

