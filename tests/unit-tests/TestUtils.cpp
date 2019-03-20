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


#include "boomerang/core/Settings.h"
#include "boomerang/util/LocationSet.h"
#include "boomerang/util/log/Log.h"


TestProject::TestProject()
{
    getSettings()->setDataDirectory(BOOMERANG_TEST_BASE "share/boomerang/");
    getSettings()->setPluginDirectory(BOOMERANG_TEST_BASE "lib/boomerang/plugins/");
}


void BoomerangTest::initTestCase()
{
    Log::getOrCreateLog();

    qRegisterMetaType<SharedTypeWrapper>();
    qRegisterMetaType<SharedExpWrapper>();
}


void BoomerangTest::cleanupTestCase()
{
}


QString getFullSamplePath(const QString& relpath)
{
    return QString(BOOMERANG_TEST_BASE) + "share/boomerang/samples/" + relpath;
}


void compareLongStrings(const QString& actual, const QString& expected)
{
    QStringList actualList = actual.split('\n');
    QStringList expectedList = expected.split('\n');

    for (int i = 0; i < std::min(actualList.length(), expectedList.length()); i++) {
        QCOMPARE(actualList[i], expectedList[i]);
    }

    QCOMPARE(actualList.length(), expectedList.length());
}


char *toString(const SharedConstExp& exp)
{
    return QTest::toString(exp->toString());
}


char *toString(const Exp& exp)
{
    return QTest::toString(exp.toString());
}


char *toString(const LocationSet& locSet)
{
    QString tgt;
    OStream os(&tgt);
    locSet.print(os);

    return QTest::toString(tgt);
}


char *toString(ICLASS type)
{
    switch (type) {
    case ICLASS::NCT:   return QTest::toString("NCT");
    case ICLASS::SD:    return QTest::toString("SD");
    case ICLASS::DD:    return QTest::toString("DD");
    case ICLASS::SCD:   return QTest::toString("SCD");
    case ICLASS::SCDAN: return QTest::toString("SCDAN");
    case ICLASS::SCDAT: return QTest::toString("SCDAT");
    case ICLASS::SU:    return QTest::toString("SU");
    case ICLASS::SKIP:  return QTest::toString("SKIP");
    case ICLASS::NOP:   return QTest::toString("NOP");
    }

    return QTest::toString("<unknown>");
}

