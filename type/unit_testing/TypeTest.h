#include <QtTest/QTest>

class TypeTest : public QObject {
    Q_OBJECT
private slots:
    void testTypeLong();
    void testNotEqual();
    void testCompound();

    void testDataInterval();
    void testDataIntervalOverlaps();
    void setUp();
    void tearDown();
};
