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


class Proc;

#include <QtTest/QTest>
#include <memory>

class ProcTest : public QObject
{
private slots:
    /// Test setting and reading name, constructor, native address
    void testName();

protected:
    std::shared_ptr<Proc> m_proc;
};
