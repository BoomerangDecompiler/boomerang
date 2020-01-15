#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TypingStatement.h"


TypingStatement::TypingStatement(StmtType kind, SharedType ty)
    : Statement(kind)
    , m_type(ty)
{
}
