#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "TestUtils.h"

#include <list>
#include <memory>

class Const;


class StmtConstFinderTest : public BoomerangTest
{
    Q_OBJECT

    void findConstants(const SharedStmt &stmt, std::list<std::shared_ptr<Const>> &constants);

private slots:
    void testFindConstants();
};
