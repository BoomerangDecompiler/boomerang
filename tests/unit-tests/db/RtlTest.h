#pragma once

#include <QtTest/QtTest>

/**
 * Tests Register Transfer Lists
 */
class RtlTest : public QObject
{
public:

private slots:
    void initTestCase();

    /// Test appendExp and printing of RTLs
    void testAppend();

    /// Test constructor from list of expressions; cloning of RTLs
    void testClone();

    /// Test the accept function for correct visiting behaviour.
    /// \note Stub class to test.
	void testVisitor();

    /// Test the isCompare function
//    void testIsCompare();

	void testSetConscripts();
};
