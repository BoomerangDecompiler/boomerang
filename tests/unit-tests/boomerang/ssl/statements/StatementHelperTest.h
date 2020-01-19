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


class StatementHelperTest : public BoomerangTestWithProject
{
    Q_OBJECT

private slots:
    void testCondToRelational();
    void testCondToRelational_data();
};
