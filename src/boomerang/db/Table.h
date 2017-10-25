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


/**
 * \file       table.h
 * \brief   Provides the definition of class Table and children used by
 *               the SSL parser
 */

#include <QString>
#include <deque>
#include <memory>

// Kinds of SSL specification tables
enum TABLE_TYPE
{
    NAMETABLE,
    OPTABLE,
    EXPRTABLE
};

class Table
{
    typedef std::deque<QString> StringQueue;

public:
    Table(const std::deque<QString>& recs, TABLE_TYPE t = NAMETABLE);
    Table(TABLE_TYPE t);
    virtual ~Table() {}
    TABLE_TYPE getType() const;

    StringQueue Records;

private:
    TABLE_TYPE TableType;
};

class OpTable : public Table
{
public:
    OpTable(const std::deque<QString>& ops);
};

class Exp;
using SharedExp = std::shared_ptr<Exp>;

class ExprTable : public Table
{
public:
    ExprTable(const std::deque<SharedExp>& exprs);
    ~ExprTable();
    std::deque<SharedExp> expressions;
};
