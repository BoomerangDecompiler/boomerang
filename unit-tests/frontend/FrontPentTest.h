#include <QtTest/QTest>

class FrontPentTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/***************************************************************************/ /**
	 * FUNCTION:        FrontPentTest::test1
	 * OVERVIEW:        Test decoding some pentium instructions
	 *============================================================================*/
	void test1();

	void test2();

	void test3();

	void testFindMain();

	void testBranch();
};
