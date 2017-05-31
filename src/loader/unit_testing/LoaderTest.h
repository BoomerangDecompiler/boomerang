#include "include/BinaryFile.h"
#include <QtTest/QTest>
class LoaderTest : public QObject
{
	Q_OBJECT

protected:

private slots:
	void testSparcLoad();
	void testPentiumLoad();
	void testHppaLoad();
	void testPalmLoad();
	void testWinLoad();

	void testMicroDis1();
	void testMicroDis2();

	void testElfHash();
	void initTestCase();
};
