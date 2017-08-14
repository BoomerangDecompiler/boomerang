#pragma once

#include <QtTest/QTest>

class PalmBinaryLoaderTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/// Test loading the Palm 68328 Starter.prc program
	void testPalmLoad();
};
