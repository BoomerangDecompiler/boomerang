#pragma once

#include <QtTest/QTest>

class Prog;

/**
 * Test the Prog class.
 */
class ProgTest : public QObject
{
    Q_OBJECT

private slots:
    /// Test setting and reading name
	void testName();

private:
	Prog *m_prog;
};
