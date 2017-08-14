/***************************************************************************/ /**
 * \file    ProcTest.h
 * Provides the interface for the ProcTest class, which tests the Proc class
 *============================================================================*/

/*
 * $Revision: 1.5 $
 *
 * 23 Apr 02 - Mike: Created
 */

class Proc;

#include <QtTest/QTest>
#include <memory>

class ProcTest : public QObject
{
private slots:
    /// Test setting and reading name, constructor, native address
	void testName();

protected:
	std::shared_ptr<Proc> m_proc;

};
