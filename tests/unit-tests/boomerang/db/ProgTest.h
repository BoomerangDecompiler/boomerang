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
