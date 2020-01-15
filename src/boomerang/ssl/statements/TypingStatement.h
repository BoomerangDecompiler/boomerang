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


#include "boomerang/ssl/statements/Statement.h"


/**
 * TypingStatements are statements that may have a type associated to them.
 * For example, for assignments to make sense, the types of the left hand side
 * and the right hand side must match.
 */
class BOOMERANG_API TypingStatement : public Statement
{
public:
    TypingStatement(StmtType kind, SharedType ty);
    TypingStatement(const TypingStatement &other) = default;
    TypingStatement(TypingStatement &&other)      = default;

    ~TypingStatement() override = default;

    TypingStatement &operator=(const TypingStatement &other) = default;
    TypingStatement &operator=(TypingStatement &&other) = default;

public:
    /// \returns the type of this statement.
    SharedType getType() { return m_type; }
    const SharedType &getType() const { return m_type; }
    void setType(SharedType ty) { m_type = ty; }

    /// \copydoc Statement::isTyping
    bool isTyping() const override { return true; }

protected:
    SharedType m_type; ///< The type for this assignment or reference
};
