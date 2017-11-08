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



RangeMap RangePrivateData::getSavedRanges(Statement *insn)
{
    return m_savedInputRanges[insn];
}


void RangePrivateData::setSavedRanges(Statement *insn, RangeMap map)
{
    m_savedInputRanges[insn] = map;
}

