#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
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
