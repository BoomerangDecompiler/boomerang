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
