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
#include "boomerang/db/IBinaryImage.h"
#include "boomerang/db/IBinarySection.h"
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

    project->loadBinaryFile(HELLO_CLANG4);
    IFileLoader *loader = project->getBestLoader(HELLO_CLANG4);

    // test the loader
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->getFormat(), LoadFmt::ELF);
    QCOMPARE(loader->getMachine(), Machine::PENTIUM);
    QCOMPARE(loader->hasDebugInfo(), false);
    QCOMPARE(loader->getEntryPoint(),     Address(0x080482F0));
    QCOMPARE(loader->getMainEntryPoint(), Address(0x080483F0));

    // test the loaded image
    IBinaryImage *image = Boomerang::get()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), static_cast<size_t>(29));
    QCOMPARE(image->getSection(0)->getName(),  QString(".interp"));
    QCOMPARE(image->getSection(10)->getName(), QString(".plt"));
    QCOMPARE(image->getSection(28)->getName(), QString(".shstrtab"));
    QCOMPARE(image->getLimitTextLow(),  Address(0x08000001));
    QCOMPARE(image->getLimitTextHigh(), Address(0x0804A020));
}


void ElfBinaryLoaderTest::testElfLoadClangStatic()
{
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(HELLO_CLANG4_STATIC);
    IFileLoader *loader = project.getBestLoader(HELLO_CLANG4_STATIC);

    // test the loader
    QVERIFY(loader != nullptr);
    QCOMPARE(loader->getFormat(), LoadFmt::ELF);
    QCOMPARE(loader->getMachine(), Machine::PENTIUM);
    QCOMPARE(loader->hasDebugInfo(), false);
    QCOMPARE(loader->getEntryPoint(),     Address(0x0804884F));
    QCOMPARE(loader->getMainEntryPoint(), Address(0x080489A0));

    // test the loaded image
    IBinaryImage *image = Boomerang::get()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), static_cast<size_t>(29));
    QCOMPARE(image->getSection(0)->getName(), QString(".note.ABI-tag"));
    QCOMPARE(image->getSection(13)->getName(), QString(".eh_frame"));
    QCOMPARE(image->getSection(28)->getName(), QString(".shstrtab"));
    QCOMPARE(image->getLimitTextLow(),  Address(0x08000001));
    QCOMPARE(image->getLimitTextHigh(), Address(0x080ECDA4));
}


void ElfBinaryLoaderTest::testPentiumLoad()
{
    // Load Pentium hello world
    IProject& project = *Boomerang::get()->getOrCreateProject();

    project.loadBinaryFile(HELLO_PENTIUM);
    IFileLoader *loader = project.getBestLoader(HELLO_PENTIUM);

    QVERIFY(loader != nullptr);
    QCOMPARE(loader->getFormat(), LoadFmt::ELF);
    QCOMPARE(loader->getMachine(), Machine::PENTIUM);

    IBinaryImage *image = Boomerang::get()->getImage();
    QVERIFY(image != nullptr);

    QCOMPARE(image->getNumSections(), static_cast<size_t>(33));
    QCOMPARE(image->getSection(1)->getName(), QString(".note.ABI-tag"));
    QCOMPARE(image->getSection(32)->getName(), QString(".strtab"));
}


QTEST_MAIN(ElfBinaryLoaderTest)
