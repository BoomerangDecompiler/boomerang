#include "include/log.h"

#include <QtTest/QTest>
#include <iostream>
class ErrLogger : public Log
{
public:
	virtual Log& operator<<(const QString& s)
	{
		std::cerr << s.toStdString();
		return *this;
	}

	virtual ~ErrLogger() {}
};

class DfaTest : public QObject
{
	Q_OBJECT

private slots:
	void testMeetInt();
	void testMeetSize();
	void testMeetPointer();
	void testMeetUnion();
	void initTestCase();
};
