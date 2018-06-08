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


class ElfBinaryLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    /// test the loader using a "Hello World" program
    /// compiled with clang-4.0.0 (without debug info)
    void testElfLoadClang();

    /// Test loading the pentium (Solaris) hello world program
    void testPentiumLoad();
    void testPentiumLoad_data();
};
