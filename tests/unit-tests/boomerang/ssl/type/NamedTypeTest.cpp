#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "NamedTypeTest.h"

#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/UnionType.h"
#include "boomerang/ssl/type/CompoundType.h"


void NamedTypeTest::testConstruct()
{
    QCOMPARE(NamedType("foo").getName(), "foo");
}


void NamedTypeTest::testEquals()
{
    QCOMPARE(NamedType("foo") == FloatType(32), false);
    QCOMPARE(NamedType("foo") == NamedType("foo"), true);
    QCOMPARE(NamedType("foo") == NamedType("bar"), false);
    QCOMPARE(NamedType("") == NamedType(""), true);
}


void NamedTypeTest::testLess()
{
    QCOMPARE(NamedType("foo") < ArrayType(VoidType::get()), false);
    QCOMPARE(NamedType("foo") < CompoundType(), true);
    QCOMPARE(NamedType("foo") < NamedType("foo"), false);
    QCOMPARE(NamedType("foo") < NamedType("bar"), false);
    QCOMPARE(NamedType("bar") < NamedType("baz"), true);
}


void NamedTypeTest::testGetCtype()
{
    QCOMPARE(NamedType("").getCtype(), "");
    QCOMPARE(NamedType("foo").getCtype(), "foo");
}


void NamedTypeTest::testResolvesTo()
{
    Type::clearNamedTypes();
    QVERIFY(NamedType("Foo").resolvesTo() == nullptr);

    Type::addNamedType("Foo", FloatType::get(32));
    QVERIFY(NamedType("Foo").resolvesTo() != nullptr);
    QVERIFY(*NamedType("Foo").resolvesTo() == *FloatType::get(32));
    QVERIFY(NamedType("Bar").resolvesTo() == nullptr);

    Type::addNamedType("Bar", NamedType::get("Foo"));
    QVERIFY(NamedType("Bar").resolvesTo() != nullptr);
    QVERIFY(*NamedType("Bar").resolvesTo() == *FloatType::get(32));
}


void NamedTypeTest::testIsCompatibleWith()
{
    Type::clearNamedTypes();
    QCOMPARE(NamedType::get("")->isCompatibleWith(*NamedType::get("")), true);
    QCOMPARE(NamedType::get("Foo")->isCompatibleWith(*NamedType::get("Foo")), true);
    QCOMPARE(NamedType::get("Foo")->isCompatibleWith(*NamedType::get("Bar")), false);

    Type::addNamedType("Foo", FloatType::get(32));
    Type::addNamedType("Bar", NamedType::get("Foo"));
    Type::addNamedType("Baz", FloatType::get(32));
    Type::addNamedType("Waldo", IntegerType::get(32, Sign::Signed));

    QCOMPARE(NamedType::get("Foo")->isCompatibleWith(*NamedType::get("Baz")), true);
    QCOMPARE(NamedType::get("Bar")->isCompatibleWith(*NamedType::get("Foo")), true);
    QCOMPARE(NamedType::get("Baz")->isCompatibleWith(*FloatType::get(32)), true);
    QCOMPARE(NamedType::get("Bar")->isCompatibleWith(*FloatType::get(32)), true);
    QCOMPARE(NamedType::get("Bar")->isCompatibleWith(*NamedType::get("Waldo")), false);
    QCOMPARE(NamedType::get("Foo")->isCompatibleWith(*NamedType::get("Waldo")), false);

    QCOMPARE(NamedType::get("Quux")->isCompatibleWith(*VoidType::get()), true);
}


QTEST_GUILESS_MAIN(NamedTypeTest)
