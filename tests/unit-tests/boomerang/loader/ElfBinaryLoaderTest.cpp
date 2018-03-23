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


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/core/Project.h"
#include "boomerang/util/Log.h"

#include <QLibrary>


#define HELLO_CLANG4           (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/elf/hello-clang4-dynamic"))
#define HELLO_CLANG4_STATIC    (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/hello-clang4-static"))
#define HELLO_PENTIUM          (Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("samples/pentium/hello"))


/// path to the ELF loader plugin
#ifdef _WIN32
#  define ELF_LOADER    (BOOMERANG_TEST_BASE "/lib/libboomerang-ElfLoader.dll")
#else
#  define ELF_LOADER    (BOOMERANG_TEST_BASE "/lib/libboomerang-ElfLoader.so")
#endif


void ElfBinaryLoaderTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    Boomerang::get()->getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void ElfBinaryLoaderTest::testElfLoadClang()
{
    IProject *project = new Project();

    QVERIFY(project->loadBinaryFile(HELLO_CLANG4));
    BinaryFile *binary = project->getLoadedBinaryFile();

    // test the loader
    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getFormat(),         LoadFmt::ELF);
    QCOMPARE(binary->getMachine(),        Machine::PENTIUM);
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
    QCOMPARE(image->getLimitTextLow(),  Address(0x08000001));
    QCOMPARE(image->getLimitTextHigh(), Address(0x0804A020));
}


void ElfBinaryLoaderTest::testElfLoadClangStatic()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();
    QVERIFY(project.loadBinaryFile(HELLO_CLANG4_STATIC));

    BinaryFile *binary = project.getLoadedBinaryFile();

    // test the loader
    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getFormat(), LoadFmt::ELF);
    QCOMPARE(binary->getMachine(), Machine::PENTIUM);
    QCOMPARE(binary->hasDebugInfo(), false);
    QCOMPARE(binary->getEntryPoint(),     Address(0x0804884F));
    QCOMPARE(binary->getMainEntryPoint(), Address(0x080489A0));

    // test the loaded image
    BinaryImage *image = project.getLoadedBinaryFile()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), 29);
    QCOMPARE(image->getSectionByIndex(0)->getName(), QString(".note.ABI-tag"));
    QCOMPARE(image->getSectionByIndex(13)->getName(), QString(".eh_frame"));
    QCOMPARE(image->getSectionByIndex(28)->getName(), QString(".shstrtab"));
    QCOMPARE(image->getLimitTextLow(),  Address(0x08000001));
    QCOMPARE(image->getLimitTextHigh(), Address(0x080ECDA4));
}


void ElfBinaryLoaderTest::testPentiumLoad()
{
    // Load Pentium hello world
    IProject& project = *Boomerang::get()->getOrCreateProject();
    QVERIFY(project.loadBinaryFile(HELLO_PENTIUM));
    BinaryFile *binary = project.getLoadedBinaryFile();

    QVERIFY(binary != nullptr);
    QCOMPARE(binary->getFormat(), LoadFmt::ELF);
    QCOMPARE(binary->getMachine(), Machine::PENTIUM);

    BinaryImage *image = project.getLoadedBinaryFile()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), 33);
    QCOMPARE(image->getSectionByIndex(1)->getName(), QString(".note.ABI-tag"));
    QCOMPARE(image->getSectionByIndex(32)->getName(), QString(".strtab"));
}


QTEST_MAIN(ElfBinaryLoaderTest)
