#include <QtTest/QTest>

class ParserTest : public QObject
{
	Q_OBJECT

private slots:
	void testRead();
	void testExp();
	void initTestCase();
};
