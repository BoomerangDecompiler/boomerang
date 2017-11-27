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
 * Test an address expression (operand of a memOf) for primitiveness (i.e. if it is possible to SSA rename the memOf
 * without problems). Note that the PrimitiveTester is not used with the memOf expression, only its address expression
 */
class PrimitiveTester : public ExpVisitor
{
public:
    PrimitiveTester() = default;
    virtual ~PrimitiveTester() = default;

public:
    bool getResult() { return m_result; }

    /// \copydoc ExpVisitor::visit
    // Return true if e is a primitive expression; basically, an expression you can propagate to without causing
    // memory expression problems. See Mike's thesis for details
    // Algorithm: if find any unsubscripted location, not primitive
    //   Implicit definitions are primitive (but keep searching for non primitives)
    //   References to the results of calls are considered primitive... but only if bypassed?
    //   Other references considered non primitive
    // Start with result=true, must find primitivity in all components
    bool visit(const std::shared_ptr<Location>& exp, bool& visitChildren) override;

    /// \copydoc ExpVisitor::visit
    bool visit(const std::shared_ptr<RefExp>& exp, bool& visitChildren) override;

private:
    bool m_result = true; ///< Initialise result true: need AND of all components
};

