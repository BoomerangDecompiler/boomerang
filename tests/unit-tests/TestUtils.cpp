#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "TestUtils.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"


TestProject::TestProject()
{
    getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void BoomerangTest::initTestCase()
{
    Boomerang::get();
    Log::getOrCreateLog();

    qRegisterMetaType<SharedTypeWrapper>();
    qRegisterMetaType<SharedExpWrapper>();
}


void BoomerangTest::cleanupTestCase()
{
    Boomerang::destroy();
}

QString getFullSamplePath(const QString& relpath)
{
    return QString(BOOMERANG_TEST_BASE) + "share/boomerang/samples/" + relpath;
}
