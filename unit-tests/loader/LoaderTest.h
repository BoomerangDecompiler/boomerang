#include "boomerang/core/BinaryFileFactory.h"

#include <QtTest/QTest>

class LoaderTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/// Test loading the Palm 68328 Starter.prc program
	void testPalmLoad();

	/// Test loading Windows programs
	void testWinLoad();

	/// Test the micro disassembler
	void testMicroDis1();
	void testMicroDis2();
};
