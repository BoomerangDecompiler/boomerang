/*
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       table.cc
 * OVERVIEW:   Provides the implementation of classes Table, OpTable, and
 *             ExprTable
 *============================================================================*/

/*
 * 25 Feb 01 - Simon: sorted out dependancies
 * 01 May 02 - Mike: mods for boomerang
 */


#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include "types.h"
#include "table.h"
#include "exp.h"

Table::Table(TABLE_TYPE t) :
    type(t)
{
}

Table::Table(std::deque<std::string>& recs, TABLE_TYPE t /* = NAMETABLE */) :
    records(recs),type(t)
{
}

TABLE_TYPE Table::getType() const { return type; }

OpTable::OpTable(std::deque<std::string>& ops) :
    Table(ops, OPTABLE)
{
}

ExprTable::ExprTable(std::deque<Exp*>& exprs) :
    Table(EXPRTABLE),expressions(exprs)
{
}

ExprTable::~ExprTable(void)
{
	std::deque<Exp*>::iterator loc;
	for (loc = expressions.begin(); loc != expressions.end(); loc++)
		delete (*loc);
}
