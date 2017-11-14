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

class Statement;


/**
 * A modifying visitor to process all references in an expression, bypassing calls (and phi statements if they have been
 * replaced by copy assignments), and performing simplification on the direct parent of the expression that is modified.
 * \note this is sometimes not enough! Consider changing (r+x)+K2) where x gets changed to K1. Now you have (r+K1)+K2,
 * but simplifying only the parent doesn't simplify the K1+K2.
 * Used to also propagate, but this became unwieldy with -l propagation limiting
 */
class CallBypasser : public SimpExpModifier
{
public:
    CallBypasser(Statement *enclosing);

    /// \copydoc SimpExpModifier::postVisit
    virtual SharedExp postVisit(const std::shared_ptr<RefExp>& exp) override;

    /// \copydoc SimpExpModifier::postVisit
    virtual SharedExp postVisit(const std::shared_ptr<Location>& exp) override;

private:
    /// Statement that is being modified at present, for debugging only
    Statement *m_enclosingStmt;
};

