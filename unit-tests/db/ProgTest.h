#pragma once

#include <QtTest/QTest>

class Prog;

class ProgTest : public QObject
{
    Q_OBJECT

private slots:
    /***************************************************************************/ /**
    * FUNCTION:        ProgTest::testName
    * OVERVIEW:        Test setting and reading name
    *============================================================================*/
	void testName();

private:
	Prog *m_prog;
};
