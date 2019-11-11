#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ElfBinaryLoaderTest.h"


#include "boomerang/core/Settings.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/util/log/Log.h"

#include <QLibrary>


#define HELLO_CLANG4           (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/elf/hello-clang4-dynamic"))
#define HELLO_CLANG4_STATIC    (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/hello-clang4-static"))
#define HELLO_X86              (m_project.getSettings()->getDataDirectory().absoluteFilePath("samples/x86/hello"))


/// path to the ELF loader plugin
#ifdef _WIN32
#  define ELF_LOADER    (BOOMERANG_TEST_BASE "/lib/libboomerang-ElfLoader.dll")
#else
#  define ELF_LOADER    (BOOMERANG_TEST_BASE "/lib/libboomerang-ElfLoader.so")
#endif


Q_DECLARE_METATYPE(Address)


void ElfBinaryLoaderTest::testElfLoadClang()
{
    QVERIFY(m_project.loadBinaryFile(HELLO_CLANG4));
    BinaryFile *binary = m_project.getLoadedBinaryFile();

    // test the loader
    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getFormat(),         LoadFmt::ELF);
    QCOMPARE(binary->getMachine(),        Machine::X86);
    QCOMPARE(binary->hasDebugInfo(),      false);
    QCOMPARE(binary->getEntryPoint(),     Address(0x080482F0));
    QCOMPARE(binary->getMainEntryPoint(), Address(0x080483F0));

    // test the loaded image
    BinaryImage *image = binary->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), 29);
    QCOMPARE(image->getSectionByIndex(0)->getName(),  QString(".interp"));
    QCOMPARE(image->getSectionByIndex(10)->getName(), QString(".plt"));
    QCOMPARE(image->getSectionByIndex(28)->getName(), QString(".shstrtab"));
    QCOMPARE(image->getLimitTextLow(),  Address(0x08048288));
    QCOMPARE(image->getLimitTextHigh(), Address(0x08048498));
}


void ElfBinaryLoaderTest::testLoadSolaris()
{
    // Load x86 hello world
    QVERIFY(m_project.loadBinaryFile(HELLO_X86));
    BinaryFile *binary = m_project.getLoadedBinaryFile();

    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getFormat(), LoadFmt::ELF);
    QCOMPARE(binary->getMachine(), Machine::X86);

    BinaryImage *image = m_project.getLoadedBinaryFile()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), 33);

    QFETCH(int, sectIndex);
    QFETCH(QString, sectName);
    QFETCH(Address, sectAddr);
    QFETCH(int, sectSize);

    QCOMPARE(image->getSectionByIndex(sectIndex)->getName(), sectName);
    QCOMPARE(image->getSectionByIndex(sectIndex)->getSize(), sectSize);
    QCOMPARE(image->getSectionByIndex(sectIndex)->getSourceAddr().toString(), sectAddr.toString());
}


void ElfBinaryLoaderTest::testLoadSolaris_data()
{
    QTest::addColumn<int>("sectIndex");
    QTest::addColumn<QString>("sectName");
    QTest::addColumn<Address>("sectAddr");
    QTest::addColumn<int>("sectSize");


    QTest::newRow("row0")   <<   0  <<  ".interp"          <<  Address(0x080480f4)  <<  0x000013;
    QTest::newRow("row1")   <<   1  <<  ".note.ABI-tag"    <<  Address(0x08048108)  <<  0x000020;
    QTest::newRow("row2")   <<   2  <<  ".hash"            <<  Address(0x08048128)  <<  0x000028;
    QTest::newRow("row3")   <<   3  <<  ".dynsym"          <<  Address(0x08048150)  <<  0x000050;
    QTest::newRow("row4")   <<   4  <<  ".dynstr"          <<  Address(0x080481a0)  <<  0x00004c;
    QTest::newRow("row5")   <<   5  <<  ".gnu.version"     <<  Address(0x080481ec)  <<  0x00000a;
    QTest::newRow("row6")   <<   6  <<  ".gnu.version_r"   <<  Address(0x080481f8)  <<  0x000020;
    QTest::newRow("row7")   <<   7  <<  ".rel.dyn"         <<  Address(0x08048218)  <<  0x000008;
    QTest::newRow("row8")   <<   8  <<  ".rel.plt"         <<  Address(0x08048220)  <<  0x000010;
    QTest::newRow("row9")   <<   9  <<  ".init"            <<  Address(0x08048230)  <<  0x000017;
    QTest::newRow("row10")  <<  10  <<  ".plt"             <<  Address(0x08048248)  <<  0x000030;
    QTest::newRow("row11")  <<  11  <<  ".text"            <<  Address(0x08048278)  <<  0x000160;
    QTest::newRow("row12")  <<  12  <<  ".fini"            <<  Address(0x080483d8)  <<  0x00001b;
    QTest::newRow("row13")  <<  13  <<  ".rodata"          <<  Address(0x080483f4)  <<  0x000017;
    QTest::newRow("row14")  <<  14  <<  ".eh_frame"        <<  Address(0x0804840c)  <<  0x000004;
    QTest::newRow("row15")  <<  15  <<  ".data"            <<  Address(0x08049410)  <<  0x00000c;
    QTest::newRow("row16")  <<  16  <<  ".dynamic"         <<  Address(0x0804941c)  <<  0x0000c8;
    QTest::newRow("row17")  <<  17  <<  ".ctors"           <<  Address(0x080494e4)  <<  0x000008;
    QTest::newRow("row18")  <<  18  <<  ".dtors"           <<  Address(0x080494ec)  <<  0x000008;
    QTest::newRow("row19")  <<  19  <<  ".jcr"             <<  Address(0x080494f4)  <<  0x000004;
    QTest::newRow("row20")  <<  20  <<  ".got"             <<  Address(0x080494f8)  <<  0x000018;
    QTest::newRow("row21")  <<  21  <<  ".bss"             <<  Address(0x08049510)  <<  0x000004;
    QTest::newRow("row22")  <<  22  <<  ".comment"         <<  Address(0x80000001)  <<  0x000132;
    QTest::newRow("row23")  <<  23  <<  ".debug_aranges"   <<  Address(0x80000138)  <<  0x000078;
    QTest::newRow("row24")  <<  24  <<  ".debug_pubnames"  <<  Address(0x800001b0)  <<  0x000025;
    QTest::newRow("row25")  <<  25  <<  ".debug_info"      <<  Address(0x800001d5)  <<  0x000a84;
    QTest::newRow("row26")  <<  26  <<  ".debug_abbrev"    <<  Address(0x80000c59)  <<  0x000138;
    QTest::newRow("row27")  <<  27  <<  ".debug_line"      <<  Address(0x80000d91)  <<  0x00027c;
    QTest::newRow("row28")  <<  28  <<  ".debug_frame"     <<  Address(0x80001010)  <<  0x000014;
    QTest::newRow("row29")  <<  29  <<  ".debug_str"       <<  Address(0x80001024)  <<  0x0006ba;
    QTest::newRow("row30")  <<  30  <<  ".shstrtab"        <<  Address(0x800016de)  <<  0x00012b;
    QTest::newRow("row31")  <<  31  <<  ".symtab"          <<  Address(0x8000180c)  <<  0x0006c0;
    QTest::newRow("row32")  <<  32  <<  ".strtab"          <<  Address(0x80001ecc)  <<  0x0003f2;
}



QTEST_GUILESS_MAIN(ElfBinaryLoaderTest)
