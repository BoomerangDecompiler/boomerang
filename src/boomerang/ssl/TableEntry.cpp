#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TableEntry.h"


TableEntry::TableEntry()
    : m_rtl(Address::INVALID)
{
}


TableEntry::TableEntry(const std::list<QString> &params, const RTL &rtl)
    : m_rtl(rtl)
{
    std::copy(params.begin(), params.end(), std::back_inserter(m_params));
}


int TableEntry::appendRTL(const std::list<QString> &params, const RTL &rtl)
{
    if (!std::equal(m_params.begin(), m_params.end(), params.begin(), params.end())) {
        return -1;
    }

    m_rtl.append(rtl.getStatements());
    return 0;
}
