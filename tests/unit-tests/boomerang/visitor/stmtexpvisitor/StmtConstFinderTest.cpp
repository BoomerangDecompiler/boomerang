#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StmtConstFinderTest.h"

#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/visitor/expvisitor/ConstFinder.h"
#include "boomerang/visitor/stmtexpvisitor/StmtConstFinder.h"


void StmtConstFinderTest::findConstants(Statement *stmt, std::list<std::shared_ptr<Const> > &constants)
{
    ConstFinder cf(constants);
    StmtConstFinder scf(&cf);

    stmt->accept(&scf);
}


void StmtConstFinderTest::testFindConstants()
{
    Assign a(Location::regOf(REG_PENT_EAX), Binary::get(opPlus, Const::get(3), Const::get(4)));

    std::list<std::shared_ptr<Const>> lc;
    findConstants(&a, lc);

    QString     actual;
    OStream ost(&actual);

    for (auto it = lc.begin(); it != lc.end();) {
        ost << *it;

        if (++it != lc.end()) {
            ost << ", ";
        }
    }

    QCOMPARE(actual, QString("3, 4"));
}

QTEST_GUILESS_MAIN(StmtConstFinderTest)
