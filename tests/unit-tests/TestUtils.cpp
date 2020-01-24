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
#include "boomerang/ssl/type/VoidType.h"


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


char *toString(const SharedConstStmt &stmt)
{
    return QTest::toString(stmt->toString());
}


char *toString(const Exp& exp)
{
    return QTest::toString(exp.toString());
}


char *toString(const Type& ty)
{
    return QTest::toString(ty.toString());
}


char *toString(const LocationSet& locSet)
{
    QString tgt;
    OStream os(&tgt);
    locSet.print(os);

    return QTest::toString(tgt);
}


char *toString(const std::list<SharedExp> &list)
{
    QString result = "{ ";

    for (const SharedExp &elem : list) {
        result += elem->toString() + " ";
    }

    result += "}";
    return QTest::toString(result);
}


#define HANDLE_ENUM_VAL(x) case x: return QTest::toString(#x)


char *toString(BBType type)
{
    switch (type) {
    HANDLE_ENUM_VAL(BBType::Invalid);
    HANDLE_ENUM_VAL(BBType::Fall);
    HANDLE_ENUM_VAL(BBType::Oneway);
    HANDLE_ENUM_VAL(BBType::Twoway);
    HANDLE_ENUM_VAL(BBType::Nway);
    HANDLE_ENUM_VAL(BBType::Ret);
    HANDLE_ENUM_VAL(BBType::Call);
    HANDLE_ENUM_VAL(BBType::CompJump);
    HANDLE_ENUM_VAL(BBType::CompCall);
    }

    return QTest::toString("<unknown>");
}


char *toString(Address addr)
{
    return QTest::toString(addr.toString());
}


std::vector<MachineInstruction> createInsns(Address baseAddr, std::size_t count)
{
    std::vector<MachineInstruction> result{ count };

    for (std::size_t i=0; i<count; ++i) {
        result[i].m_addr = baseAddr + i;
        result[i].m_size = 1;
    }

    return result;
}


std::unique_ptr<RTLList> createRTLs(Address baseAddr, std::size_t numRTLs, std::size_t numStmtsPerRTL)
{
    std::unique_ptr<RTLList> rtls(new RTLList);

    for (std::size_t i = 0; i < numRTLs; i++) {
        auto rtl = std::unique_ptr<RTL>(new RTL(baseAddr + i));

        for (std::size_t j = 0; j < numStmtsPerRTL; ++j) {
            auto stmt = std::make_shared<Assign>(VoidType::get(), Terminal::get(opNil), Terminal::get(opNil));
            rtl->append(stmt);
        }

        rtls->push_back(std::move(rtl));
    }

    return rtls;
}
