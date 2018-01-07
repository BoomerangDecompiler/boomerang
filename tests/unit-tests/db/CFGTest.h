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

    /// tests createBB if another (complete) BB is blocking the newly created BB.
    void testCreateBBBlocking();

    /// tests createBB if another incomplete BB is blocking the newly created BB.
    void testCreateBBBlockingIncomplete();

    /// tests creating an incomplete BB
    void testCreateIncompleteBB();

    void testRemoveBB();

    void testAddEdge();
    void testIsWellFormed();
};
