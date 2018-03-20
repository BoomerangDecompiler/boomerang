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


class BinaryImageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testGetNumSections();
    void testHasSections();
    void testCreateSection();
    void testGetSectionByIndex();
    void testGetSectionByName();
    void testGetSectionByAddr();

    void testUpdateTextLimits();

    void testRead();
    void testWrite();

    void testIsReadOnly();
};
