#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProjectTest.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"

#define HELLO_CLANG4    (BOOMERANG_TEST_BASE "tests/inputs/elf/hello-clang4-dynamic")

void ProjectTest::initTestCase()
{
    Boomerang::get()->getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "lib/boomerang/");
}


void ProjectTest::testLoadBinaryFile()
{
    Project project;

    QVERIFY(project.loadBinaryFile(HELLO_CLANG4));
    QVERIFY(project.loadBinaryFile(HELLO_CLANG4));

    // load while another one is loaded
    QVERIFY(!project.loadBinaryFile("invalid"));
    project.unloadBinaryFile();

    // load while no other file is loaded
    QVERIFY(!project.loadBinaryFile("invalid"));
}


void ProjectTest::testLoadSaveFile()
{
    QSKIP("Not implemented.");
}


void ProjectTest::testWriteSaveFile()
{
    QSKIP("Not implemented.");
}


void ProjectTest::testIsBinaryLoaded()
{
    Project project;

    project.loadBinaryFile(HELLO_CLANG4);
    QVERIFY(project.isBinaryLoaded());

    project.unloadBinaryFile();
    QVERIFY(!project.isBinaryLoaded());

    project.loadBinaryFile("invalid");
    QVERIFY(!project.isBinaryLoaded());

    // test if binary is loaded when loading from save file
    // TODO
}


QTEST_MAIN(ProjectTest)
