#pragma once

#include <QtTest/QTest>

class MicroX86DisTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/// Test the micro disassembler
	void testMicroDis1();
	void testMicroDis2();
};
