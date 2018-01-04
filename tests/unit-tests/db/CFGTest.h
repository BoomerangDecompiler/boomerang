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


#include <QtTest/QTest>


/**
 * Tests for the Control Flow Graph
 */
class CFGTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testCreateBB();
    void testCreateIncompleteBB();
    void testRemoveBB();

    void testAddEdge();
    void testIsWellFormed();
};
