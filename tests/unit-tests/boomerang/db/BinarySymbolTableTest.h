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


class BinarySymbolTableTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testSize();
    void testEmpty();
    void testClear();

    void testCreateSymbol();
    void testFindSymbolByAddress();
    void testFindSymbolByName();
    void testRenameSymbol();
};
