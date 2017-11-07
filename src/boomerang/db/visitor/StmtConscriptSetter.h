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


#include "boomerang/db/visitor/StmtVisitor.h"

class StmtConscriptSetter : public StmtVisitor
{
public:
    StmtConscriptSetter(int n, bool _bClear)
        : m_curConscript(n)
        , m_clear(_bClear) {}
    int getLast() const { return m_curConscript; }

    virtual bool visit(Assign *stmt) override;
    virtual bool visit(PhiAssign *stmt) override;
    virtual bool visit(ImplicitAssign *stmt) override;
    virtual bool visit(BoolAssign *stmt) override;
    virtual bool visit(CaseStatement *stmt) override;
    virtual bool visit(CallStatement *stmt) override;
    virtual bool visit(ReturnStatement *stmt) override;
    virtual bool visit(BranchStatement *stmt) override;
    virtual bool visit(ImpRefStatement *stmt) override;

private:
    int m_curConscript;
    bool m_clear;
};
