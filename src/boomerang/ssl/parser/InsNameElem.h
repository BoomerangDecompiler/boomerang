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


#include "boomerang/ssl/parser/Table.h"

#include <QString>

#include <map>
#include <memory>


/// An element of an instruction name
class InsNameElem : public std::enable_shared_from_this<InsNameElem>
{
public:
    InsNameElem(const QString &name);
    InsNameElem(const InsNameElem &other) = delete;
    InsNameElem(InsNameElem &&other)      = default;

    virtual ~InsNameElem();

    InsNameElem &operator=(const InsNameElem &other) = delete;
    InsNameElem &operator=(InsNameElem &&other) = default;

public:
    virtual size_t getNumTokens() const;
    virtual QString getInstruction() const;
    virtual QString getInsPattern() const;
    virtual void getRefMap(std::map<QString, std::shared_ptr<InsNameElem>> &map);

    int getNumInstructions() const;
    void append(std::shared_ptr<InsNameElem> next);
    bool increment();
    void reset();
    int getValue() const;

protected:
    std::shared_ptr<InsNameElem> m_nextElem;
    QString m_elemName;
    size_t m_value = 0;
};


class InsOptionElem : public InsNameElem
{
public:
    InsOptionElem(const QString &name);

    size_t getNumTokens() const override;
    QString getInstruction() const override;
    QString getInsPattern() const override;
};


class InsListElem : public InsNameElem
{
public:
    InsListElem(const QString &name, const std::shared_ptr<Table> &t, const QString &idx);

    size_t getNumTokens() const override;
    QString getInstruction() const override;
    QString getInsPattern() const override;
    void getRefMap(std::map<QString, std::shared_ptr<InsNameElem>> &m) override;

    QString getIndex() const;

protected:
    QString m_indexName;
    std::shared_ptr<Table> m_theTable;
};
