/*
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       table.h
  * \brief   Provides the definition of class Table and children used by
  *               the SSL parser
  ******************************************************************************/
#ifndef TABLE_H
#define TABLE_H

#include <QString>
#include <deque>

// Kinds of SSL specification tables
enum TABLE_TYPE { NAMETABLE, OPTABLE, EXPRTABLE };

class Table {
    typedef std::deque<QString> tRecords;

  public:
    Table(const std::deque<QString> &recs, TABLE_TYPE t = NAMETABLE);
    Table(TABLE_TYPE t);
    virtual ~Table() {}
    TABLE_TYPE getType() const;
    tRecords Records;

  private:
    TABLE_TYPE TableType;
};

class OpTable : public Table {
  public:
    OpTable(const std::deque<QString> &ops);
};

class Exp;

class ExprTable : public Table {
  public:
    ExprTable(const std::deque<Exp *> &exprs);
    ~ExprTable(void);
    std::deque<Exp *> expressions;
};

#endif
