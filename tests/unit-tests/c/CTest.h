#pragma once

#include <QtTest/QTest>

/// Class for testing the C parser
class CTest : public QObject
{
	Q_OBJECT

private slots:
    /// Test parsing C signatures
	void testSignature();
};
