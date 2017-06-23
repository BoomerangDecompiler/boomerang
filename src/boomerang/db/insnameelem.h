#pragma once

/*
 * Copyright (C) 2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

#include "table.h"

#include <QString>
#include <map>
#include <memory>

/// An element of an instruction name
class InsNameElem
{
public:
	InsNameElem(const QString& name);
	virtual ~InsNameElem();

	virtual size_t getNumTokens() const;
	virtual QString getInstruction() const;
	virtual QString getInsPattern() const;
	virtual void getRefMap(std::map<QString, InsNameElem *>& m);

	int getNumInstructions() const;
	void append(std::shared_ptr<InsNameElem> next);
	bool increment();
	void reset();
	int getValue() const;

protected:
	std::shared_ptr<InsNameElem> m_nextElem;
	QString m_elemName;
	size_t m_value;
};


class InsOptionElem : public InsNameElem
{
public:
	InsOptionElem(const QString& name);
	virtual size_t getNumTokens()   const override;
	virtual QString getInstruction() const override;
	virtual QString getInsPattern()  const override;
};


class InsListElem : public InsNameElem
{
public:
	InsListElem(const QString& name, const std::shared_ptr<Table>& t, const QString& idx);

	virtual size_t getNumTokens()   const override;
	virtual QString getInstruction() const override;
	virtual QString getInsPattern()  const override;
	virtual void getRefMap(std::map<QString, InsNameElem *>& m) override;

	QString getIndex() const;

protected:
	QString m_indexName;
	std::shared_ptr<Table> m_theTable;
};
