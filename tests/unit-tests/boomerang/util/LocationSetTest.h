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


#include <QTest>


class LocationSetTest : public QObject
{
public:
    Q_OBJECT

private slots:
    /// Set up anything needed before all tests
    void initTestCase();

    void testAssign();
    void testCompare();

    void testEmpty();
    void testSize();
    void testClear();
    void testInsert();
    void testRemove();
    void testContains();
    void testExistsImplicit();
    void testFindNS();
    void testFindDifferentRef();
    void testAddSubscript();
    void testMakeUnion();
    void testMakeDiff();
    void testSubstitute();
};
