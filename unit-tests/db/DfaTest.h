#pragma once

#include "boomerang/util/Log.h"

#include <QtTest/QTest>
#include <iostream>

class ErrLogger : public Log
{
public:
	virtual Log& operator<<(const QString& s) override
	{
		std::cerr << s.toStdString();
		return *this;
	}

	virtual ~ErrLogger() {}
};

class DfaTest : public QObject
{
	Q_OBJECT

private slots:
    /***************************************************************************/ /**
    * \fn        DfaTest::testMeetInt
    * OVERVIEW:        Test meeting IntegerTypes with various other types
    ******************************************************************************/
	void testMeetInt();

    /***************************************************************************/ /**
    * \fn        DfaTest::testMeetSize
    * OVERVIEW:        Test meeting IntegerTypes with various other types
    ******************************************************************************/
	void testMeetSize();

    /***************************************************************************/ /**
    * \fn        DfaTest::testMeetPointer
    * OVERVIEW:        Test meeting IntegerTypes with various other types
    ******************************************************************************/
	void testMeetPointer();

    /***************************************************************************/ /**
    * \fn        DfaTest::testMeetUnion
    * OVERVIEW:        Test meeting IntegerTypes with various other types
    ******************************************************************************/
	void testMeetUnion();
	void initTestCase();
};
