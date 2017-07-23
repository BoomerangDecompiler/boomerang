#include "boomerang/core/BinaryFileFactory.h"

#include <QtTest/QTest>

class LoaderTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/// Test the micro disassembler
	void testMicroDis1();
	void testMicroDis2();
};
