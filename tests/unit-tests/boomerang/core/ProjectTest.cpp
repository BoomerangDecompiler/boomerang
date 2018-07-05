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
#include "boomerang/core/Settings.h"
#include "boomerang/db/Prog.h"


void ProjectTest::testLoadBinaryFile()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    project.loadPlugins();

    QVERIFY(project.loadBinaryFile(getFullSamplePath("elf/hello-clang4-dynamic")));
    QVERIFY(project.loadBinaryFile(getFullSamplePath("elf/hello-clang4-dynamic")));

    // load while another one is loaded
    QVERIFY(!project.loadBinaryFile("invalid"));
    project.unloadBinaryFile();

    // load while no other file is loaded
    QVERIFY(!project.loadBinaryFile("invalid"));
}


void ProjectTest::testLoadSaveFile()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    QVERIFY(!project.loadSaveFile("invalid"));
}


void ProjectTest::testWriteSaveFile()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    QVERIFY(!project.writeSaveFile("invalid"));
}


void ProjectTest::testIsBinaryLoaded()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    project.loadPlugins();

    QVERIFY(project.loadBinaryFile(getFullSamplePath("elf/hello-clang4-dynamic")));
    QVERIFY(project.isBinaryLoaded());

    project.unloadBinaryFile();
    QVERIFY(!project.isBinaryLoaded());

    QVERIFY(!project.loadBinaryFile("invalid"));
    QVERIFY(!project.isBinaryLoaded());

    project.unloadBinaryFile();
    // test if binary is loaded when loading from save file
    // TODO
}


void ProjectTest::testDecodeBinaryFile()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    project.loadPlugins();

    QVERIFY(!project.decodeBinaryFile());

    QVERIFY(project.loadBinaryFile(getFullSamplePath("elf/hello-clang4-dynamic")));
    QVERIFY(project.decodeBinaryFile());
    QVERIFY(project.decodeBinaryFile()); // re-decode this file
}


void ProjectTest::testDecompileBinaryFile()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    project.loadPlugins();

    QVERIFY(!project.decodeBinaryFile());

    QVERIFY(project.loadBinaryFile(getFullSamplePath("elf/hello-clang4-dynamic")));
    QVERIFY(project.decodeBinaryFile());
    QVERIFY(project.decompileBinaryFile());
}


void ProjectTest::testGenerateCode()
{
    Project project;
    project.getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    project.getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
    project.loadPlugins();

    QVERIFY(!project.generateCode());

    QVERIFY(project.loadBinaryFile(getFullSamplePath("elf/hello-clang4-dynamic")));
    QVERIFY(project.decodeBinaryFile());
    QVERIFY(project.decompileBinaryFile());

    QVERIFY(project.generateCode(project.getProg()->getRootModule()));
    QVERIFY(project.generateCode());
}


QTEST_GUILESS_MAIN(ProjectTest)
