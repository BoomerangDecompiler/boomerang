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

    /***************************************************************************/ /**
    * \fn        CfgTest::testDominators
    * OVERVIEW:  Test the dominator frontier code
    ******************************************************************************/
	void testDominators();

    /***************************************************************************/ /**
    * \fn        CfgTest::testSemiDominators
    * OVERVIEW:        Test a case where semi dominators are different to dominators
    ******************************************************************************/
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

    /***************************************************************************/ /**
    * \fn        CfgTest::testRenameVars
    * OVERVIEW:        Test the renaming of variables
    ******************************************************************************/
	void testRenameVars();
};
