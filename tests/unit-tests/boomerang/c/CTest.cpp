#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CTest.h"


#include "boomerang/c/ansi-c-parser.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/CharType.h"

#include <sstream>


void CTest::testSignature()
{
    std::istream *os = new std::istringstream("int printf(char *fmt, ...);");
    AnsiCParser        *p = new AnsiCParser(os, false);
    p->yyparse(Platform::PENTIUM, CallConv::C);

    QCOMPARE(p->signatures.size(), size_t(1));

    auto sig = p->signatures.front();
    QCOMPARE(sig->getName(), QString("printf"));

    // The functions have two return parameters :
    // 0 - ESP
    // 1 - Actual return
    QCOMPARE(sig->getNumReturns(), 2);
    QVERIFY(sig->getReturnType(1)->resolvesToInteger());
    SharedType t = PointerType::get(CharType::get());

    // Pentium signatures used to have esp prepended to the list of parameters; no more?
    QCOMPARE(sig->getNumParams(), 1);
    QVERIFY(*sig->getParamType(0) == *t);
    QCOMPARE(sig->getParamName(0), QString("fmt"));
    QVERIFY(sig->hasEllipsis());
    delete p;
}


QTEST_GUILESS_MAIN(CTest)
