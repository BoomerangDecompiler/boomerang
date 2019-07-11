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


#include "boomerang/ssl/type/Type.h"


/**
 * A return value of a function.
 */
class BOOMERANG_API Return
{
public:
    Return(SharedType _type, SharedExp _exp);

public:
    bool operator==(const Return &other) const;
    bool operator!=(const Return &other) const { return !(*this == other); }
    bool operator<(const Return &other) const;

public:
    std::shared_ptr<Return> clone() const;

    /// \returns the type of this function return.
    SharedType getType() const { return m_type; }

    SharedExp getExp() const { return m_exp; }

private:
    SharedType m_type;
    SharedExp m_exp;
};
