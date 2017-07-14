#include <QtTest/QTest>

class TypeTest : public QObject
{
	Q_OBJECT

private slots:
	/// Set up anything needed before all tests
	void initTestCase();

	/// Test type unsigned long
	void testTypeLong();

	/// Test type inequality
	void testNotEqual();

	/// Test type inequality
	void testCompound();

	/// Test the DataIntervalMap class
	void testDataInterval();

	/// Test the DataIntervalMap class with overlapping addItems
	void testDataIntervalOverlaps();
};
