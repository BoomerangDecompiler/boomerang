/*
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       table.cpp
 * \brief   Provides the implementation of classes Table, OpTable, and
 *               ExprTable
 ******************************************************************************/

#include "table.h"

#include "boomerang/db/exp.h"


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
