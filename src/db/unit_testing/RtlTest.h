#include <QtTest/QtTest>
class RtlTest : public QObject
{
public:
private slots:
	void testAppend();
	void testClone();
	void testVisitor();
	void testSetConscripts();
	void initTestCase();
};
