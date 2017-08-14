#include <QtTest/QTest>

class ParserTest : public QObject
{
	Q_OBJECT

private slots:
    void initTestCase();

    /// Test reading the SSL file
	void testRead();

    /// Test parsing an expression
	void testExp();
};
