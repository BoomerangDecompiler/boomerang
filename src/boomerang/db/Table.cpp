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


/***************************************************************************/ /**
 * \file  Table.cpp
 * \brief Provides the implementation of classes Table, OpTable, and ExprTable
 ******************************************************************************/

Table::Table(TABLE_TYPE t)
    : TableType(t)
{
}


Table::Table(const std::deque<QString>& recs, TABLE_TYPE t /* = NAMETABLE */)
    : Records(recs)
    , TableType(t)
{
}


TABLE_TYPE Table::getType() const
{
    return TableType;
}


OpTable::OpTable(const std::deque<QString>& ops)
    : Table(ops, OPTABLE)
{
}


ExprTable::ExprTable(const std::deque<SharedExp>& exprs)
    : Table(EXPRTABLE)
    , expressions(exprs)
{
}


ExprTable::~ExprTable()
{
//    std::deque<SharedExp>::iterator loc;
//    for (loc = expressions.begin(); loc != expressions.end(); ++loc)
//        delete (*loc);
}
