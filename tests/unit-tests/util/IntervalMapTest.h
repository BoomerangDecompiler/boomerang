#include <QtTest/QTest>

class IntervalMapTest : public QObject
{
	Q_OBJECT

private slots:
	/// Set up anything needed before all tests
	void initTestCase();

    /// test isEmpty()
    void testIsEmpty();

    /// test find()
    void testFind();
};
