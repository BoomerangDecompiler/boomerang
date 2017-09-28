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

class IFileLoader;


class ElfBinaryLoaderTest : public QObject
{
public:
    void initTestCase();

    /// test the loader using a "Hello World" program
    /// compiled with clang-4.0.0 (without debug info)
    void testElfLoadClang();

    /// test the loader using a "Hello World" program
    /// compiled with clang-4.0.0 (without debug info, static libc)
    void testElfLoadClangStatic();

    /// Test loading the pentium (Solaris) hello world program
    void testPentiumLoad();

    /// Test the ELF hash function.
    void testElfHash();
};
