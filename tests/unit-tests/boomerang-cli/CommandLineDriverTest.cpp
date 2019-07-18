#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CommandLineDriverTest.h"


#include "boomerang-cli/CommandlineDriver.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/util/log/Log.h"

#include <iostream>


void CommandLineDriverTest::initTestCase()
{
    std::cout.rdbuf(nullptr);
    std::cerr.rdbuf(nullptr);
}


void CommandLineDriverTest::testApplyCommandline()
{
    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-h" }), 2);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--help" }), 2);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--version" }), 2);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "test.exe" }), 0);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "a", "test.exe" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "aaa", "test.exe" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->verboseOutput, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-v", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->verboseOutput, true);
    }

    {
        CommandlineDriver drv;
        Log::getOrCreateLog().setLogLevel(LogLevel::Default);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--log-level", "5", "test.exe" }), 0);
        QCOMPARE(Log::getOrCreateLog().getLogLevel(), LogLevel::Verbose2);
    }

    {
        CommandlineDriver drv;
        Log::getOrCreateLog().setLogLevel(LogLevel::Default);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--log-level", "-1", "test.exe" }), 1);
        QCOMPARE(Log::getOrCreateLog().getLogLevel(), LogLevel::Default);
    }

    {
        CommandlineDriver drv;
        Log::getOrCreateLog().setLogLevel(LogLevel::Default);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--log-level", "--", "test.exe" }), 1);
        QCOMPARE(Log::getOrCreateLog().getLogLevel(), LogLevel::Default);
    }


    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->printRTLs, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-r", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->printRTLs, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->traceDecoder, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-t", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->traceDecoder, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->dotFile, QString(""));
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-gd", "dotFile", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->dotFile, QString("dotFile"));
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-gd" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->generateCallGraph, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-gc", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->generateCallGraph, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->generateSymbols, false);
        QCOMPARE(drv.getProject()->getSettings()->stopBeforeDecompile, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-gs", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->stopBeforeDecompile, true);
        QCOMPARE(drv.getProject()->getSettings()->generateSymbols, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->stopBeforeDecompile, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--decode-only", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->stopBeforeDecompile, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->sslFileName, QString(""));
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--ssl", "sslFile", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->sslFileName, QString("sslFile"));
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--ssl" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->getOutputDirectory(), QDir("./output"));
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-o", "new_output", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->getOutputDirectory(), QString("new_output/"));
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-o" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->decodeThruIndCall, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-ic", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->decodeThruIndCall, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->decodeChildren, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nc", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->decodeChildren, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->useDataflow, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nd", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->useDataflow, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->useGlobals, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-ng", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->useGlobals, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->useLocals, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nl", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->useLocals, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->removeNull, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nn", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->removeNull, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->nameParameters, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-np", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->nameParameters, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->usePromotion, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nP", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->usePromotion, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->removeLabels, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nr", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->removeLabels, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->removeReturns, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nR", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->removeReturns, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->useTypeAnalysis, true);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-nT", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->useTypeAnalysis, false);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->numToPropagate, -1);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-p", "08", "test.exe" }), 1);
        QCOMPARE(drv.getProject()->getSettings()->numToPropagate, 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-p", "-1", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->numToPropagate, -1);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-p", "100", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->numToPropagate, 100);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-p" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->m_symbolFiles.size(), 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-sf", "symbolfile.txt", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->m_symbolFiles.size(), 1);
        QCOMPARE(drv.getProject()->getSettings()->m_symbolFiles[0], "symbolfile.txt");
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-sf" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->m_symbolMap.size(), 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-s", "sym", "0x1000", "test.exe" }), 1);
        QCOMPARE(drv.getProject()->getSettings()->m_symbolMap.size(), 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-s", "0x1000", "sym", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->m_symbolMap.size(), 1);
        QCOMPARE(drv.getProject()->getSettings()->m_symbolMap.at(Address(0x1000)), "sym");
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-s" }), 1);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-s", "0x1000" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugSwitch, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-dc", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugSwitch, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugDecoder, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-dd", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugDecoder, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugGen, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-dg", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugGen, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugLiveness, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-dl", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugLiveness, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugProof, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-dp", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugProof, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugTA, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-dt", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugTA, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->debugUnused, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-du", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->debugUnused, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->assumeABI, false);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-a", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->assumeABI, true);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->propMaxDepth, 3);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-l", "08", "test.exe" }), 1);
        QCOMPARE(drv.getProject()->getSettings()->propMaxDepth, 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-l", "10", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->propMaxDepth, 10);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-l" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "--", "test.exe" }), 0);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-E", "08", "test.exe" }), 1);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-E" }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->decodeChildren, true);
        QCOMPARE(drv.getProject()->getSettings()->decodeMain, true);
        QCOMPARE(drv.getProject()->getSettings()->m_entryPoints.size(), 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-E", "0x1000", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->decodeChildren, false);
        QCOMPARE(drv.getProject()->getSettings()->decodeMain, false);
        QCOMPARE(drv.getProject()->getSettings()->m_entryPoints.size(), 1);
        QCOMPARE(drv.getProject()->getSettings()->m_entryPoints[0], Address(0x1000));
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-e", "08", "test.exe" }), 1);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-e", }), 1);
    }

    {
        CommandlineDriver drv;
        QCOMPARE(drv.getProject()->getSettings()->decodeChildren, true);
        QCOMPARE(drv.getProject()->getSettings()->decodeMain, true);
        QCOMPARE(drv.getProject()->getSettings()->m_entryPoints.size(), 0);
        QCOMPARE(drv.applyCommandline({ "boomerang-cli", "-e", "0x1000", "test.exe" }), 0);
        QCOMPARE(drv.getProject()->getSettings()->decodeChildren, true);
        QCOMPARE(drv.getProject()->getSettings()->decodeMain, false);
        QCOMPARE(drv.getProject()->getSettings()->m_entryPoints.size(), 1);
        QCOMPARE(drv.getProject()->getSettings()->m_entryPoints[0], Address(0x1000));
    }
}


QTEST_GUILESS_MAIN(CommandLineDriverTest)
