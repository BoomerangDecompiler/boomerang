#include <QtTest/QTest>

class ParserTest : public QObject
{
	Q_OBJECT

private slots:
    void initTestCase();

    /***************************************************************************/ /**
    * \fn        ParserTest::testRead
    * OVERVIEW:        Test reading the SSL file
    ******************************************************************************/
	void testRead();

    /***************************************************************************/ /**
    * \fn        ParserTest::testExp
    * OVERVIEW:        Test parsing an expression
    ******************************************************************************/
	void testExp();
};
