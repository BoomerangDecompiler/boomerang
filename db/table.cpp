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
#include "types.h"
#include "statement.h"
#include "exp.h"

#include <cassert>

Table::Table(TABLE_TYPE t) : type(t) {}

Table::Table(const std::deque<std::string> &recs, TABLE_TYPE t /* = NAMETABLE */) : records(recs), type(t) {}

TABLE_TYPE Table::getType() const { return type; }

OpTable::OpTable(const std::deque<std::string> &ops) : Table(ops, OPTABLE) {}

ExprTable::ExprTable(const std::deque<Exp *> &exprs) : Table(EXPRTABLE), expressions(exprs) {}

ExprTable::~ExprTable(void) {
    std::deque<Exp *>::iterator loc;
    for (loc = expressions.begin(); loc != expressions.end(); ++loc)
        delete (*loc);
}
