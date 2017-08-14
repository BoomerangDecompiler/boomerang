#pragma once

#include <QtTest/QTest>


class Win32BinaryLoaderTest : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();

	/// Test loading Windows programs
	void testWinLoad();
};
