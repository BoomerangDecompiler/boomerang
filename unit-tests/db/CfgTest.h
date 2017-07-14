#pragma once

#include "boomerang/db/cfg.h"

#include <QtTest/QTest>

class CfgTest : public QObject
{
	Q_OBJECT

protected:
	Cfg *m_prog;

private slots:
	void initTestCase();
	void testDominators();
	void testSemiDominators();

	/***************************************************************************/ /**
	* \fn        CfgTest::testPlacePhi
	* OVERVIEW:        Test the placing of phi functions
	******************************************************************************/
	void testPlacePhi();

	/***************************************************************************/ /**
	* \fn        CfgTest::testPlacePhi2
	* OVERVIEW:        Test a case where a phi function is not needed
	******************************************************************************/
	void testPlacePhi2();
	void testRenameVars();
};
