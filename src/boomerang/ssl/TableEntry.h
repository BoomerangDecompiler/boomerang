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


#include "boomerang/ssl/RTL.h"


/**
 * The TableEntry class represents a single instruction - a string/RTL pair.
 */
class BOOMERANG_API TableEntry
{
public:
    TableEntry();
    TableEntry(const std::list<QString> &params, const RTL &rtl);

public:
    /**
     * Appends the statements in \p rtl to the RTL in this TableEntry,
     * if the parameters match.
     *
     * \param params parameters of the instruction
     * \param rtl Statements of this RTL are appended.
     *
     * \returns Zero on success, non-zero on failure.
     */
    int appendRTL(const std::list<QString> &params, const RTL &rtl);

public:
    std::list<QString> m_params;
    RTL m_rtl;
};
