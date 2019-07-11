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


#include <QString>

#include <deque>
#include <memory>


class Exp;
using SharedExp = std::shared_ptr<Exp>;


/**
 * Provides the definition of class Table used by the SSL parser
 */
class Table
{
    typedef std::deque<QString> StringQueue;

public:
    Table(const std::deque<QString> &recs);
    Table() = default;

    Table(const Table &other) = delete;
    Table(Table &&other)      = default;

    virtual ~Table() = default;

    Table &operator=(const Table &other) = delete;
    Table &operator=(Table &&other) = default;

public:
    const StringQueue &getRecords() const { return Records; }

private:
    StringQueue Records;
};
