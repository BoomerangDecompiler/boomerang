#include <QtTest/QtTest>
class RtlTest : public QObject
{
public:

private slots:
    void initTestCase();

    /***************************************************************************/ /**
    * \fn        RtlTest::testAppend
    * OVERVIEW:        Test appendExp and printing of RTLs
    ******************************************************************************/
	void testAppend();

    /***************************************************************************/ /**
    * \fn        RtlTest::testClone
    * OVERVIEW:        Test constructor from list of expressions; cloning of RTLs
    ******************************************************************************/
	void testClone();

    /***************************************************************************/ /**
    * \fn        RtlTest::testVisitor
    * OVERVIEW:        Test the accept function for correct visiting behaviour.
    * NOTES:            Stub class to test.
    ******************************************************************************/
	void testVisitor();

    /***************************************************************************/ /**
    * \fn        RtlTest::testIsCompare
    * OVERVIEW:        Test the isCompare function
    ******************************************************************************/
//    void testIsCompare();

	void testSetConscripts();
};
