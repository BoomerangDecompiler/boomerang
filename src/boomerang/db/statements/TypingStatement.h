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


#include "boomerang/db/statements/Statement.h"

/**
 * TypingStatement is an abstract subclass of Statement.
 * It has a type, representing the type of a reference or an assignment
 */
class TypingStatement : public Statement
{
public:
    TypingStatement(SharedType ty); ///< Constructor
    virtual ~TypingStatement() override = default;

    // Get and set the type.
    SharedType getType() { return m_type; }
    const SharedType& getType() const { return m_type; }
    void setType(SharedType ty) { m_type = ty; }

    virtual bool isTyping() const override { return true; }

protected:
    SharedType m_type; ///< The type for this assignment or reference
};
