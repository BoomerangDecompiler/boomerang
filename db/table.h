/*
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file       table.h
 * \brief   Provides the definition of class Table and children used by
 *               the SSL parser
 ******************************************************************************/

/*
 * 25 Feb 01 - Simon: updated post creation
 * 10 May 02 - Mike: Mods for boomerang
 */

#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <deque>

// Kinds of SSL specification tables
enum TABLE_TYPE {
    NAMETABLE,
    OPTABLE,
    EXPRTABLE
};

class Table {
    typedef std::deque<std::string> tRecords;
public:
                        Table(const std::deque<std::string> &recs, TABLE_TYPE t = NAMETABLE);
                        Table(TABLE_TYPE t);
virtual                 ~Table() {}
        TABLE_TYPE      getType() const;
        tRecords        records;
private:
    TABLE_TYPE type;
};

class OpTable : public Table {
public:
    OpTable(const std::deque<std::string> &ops);
};

class Exp;

class ExprTable : public Table {
public:
    ExprTable(const std::deque<Exp *> &exprs);
    ~ExprTable(void);
    std::deque<Exp*> expressions;
};

#endif
