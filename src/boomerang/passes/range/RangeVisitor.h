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

#include <list>
#include <memory>

class Statement;
class Assign;
class PhiAssign;
class ImplicitAssign;
class BoolAssign;
class GotoStatement;
class BranchStatement;
class RangeMap;
class RangePrivateData;
class BasicBlock;

using SharedExp = std::shared_ptr<class Exp>;


class RangeVisitor : public StmtVisitor
{
public:
    RangeVisitor(RangePrivateData *t, std::list<Statement *>& ex_paths);

    void processRange(Statement *i);

    RangeMap getInputRanges(Statement *insn);

    void updateRanges(Statement *insn, RangeMap& output, bool notTaken = false);

    bool visit(Assign *insn) override;
    bool visit(PhiAssign *stmt) override;
    bool visit(ImplicitAssign *stmt) override;
    bool visit(BoolAssign *stmt) override;
    bool visit(GotoStatement *stmt) override;
    bool visit(BranchStatement *stmt) override;
    bool visit(CaseStatement *stmt) override;
    bool visit(CallStatement *stmt) override;
    bool visit(ReturnStatement *stmt) override;
    bool visit(ImpRefStatement *stmt) override;
    bool visit(JunctionStatement *stmt) override;

private:
    RangeMap& getRangesForOutEdgeTo(BranchStatement *b, BasicBlock *out);

    void limitOutputWithCondition(BranchStatement *stmt, RangeMap& output, const SharedExp& e);

private:
    RangePrivateData *tgt;
    std::list<Statement *>& execution_paths;

};
