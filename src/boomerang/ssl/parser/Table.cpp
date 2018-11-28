#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Table.h"


Table::Table(TABLE_TYPE t)
    : TableType(t)
{
}


Table::Table(const std::deque<QString> &recs, TABLE_TYPE t /* = NAMETABLE */)
    : TableType(t)
    , Records(recs)
{
}


TABLE_TYPE Table::getType() const
{
    return TableType;
}


ExprTable::ExprTable(const std::deque<SharedExp> &exprs)
    : Table(EXPRTABLE)
    , expressions(exprs)
{
}
