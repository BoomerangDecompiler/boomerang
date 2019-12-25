#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UsedLocsVisitor.h"

#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/visitor/expvisitor/UsedLocsFinder.h"


UsedLocsVisitor::UsedLocsVisitor(ExpVisitor *v, bool countCols)
    : StmtExpVisitor(v)
    , m_countCols(countCols)
{
}


bool UsedLocsVisitor::visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren)
{
    SharedExp lhs = stmt->getLeft();
    SharedExp rhs = stmt->getRight();

    if (rhs) {
        rhs->acceptVisitor(ev);
    }

    // Special logic for the LHS. Note: PPC can have r[tmp + 30] on LHS
    if (lhs->isMemOf() || lhs->isRegOf()) {
        SharedExp child = lhs->getSubExp1(); // m[xxx] uses xxx
        // Care! Don't want the memOnly flag when inside a m[...]. Otherwise, nothing will be found
        // Also beware that ev may be a UsedLocalFinder now
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->acceptVisitor(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if (lhs->isArrayIndex() || lhs->isMemberOf()) {
        SharedExp subExp1 = lhs->getSubExp1(); // array(base, index) and member(base, offset)?? use
        subExp1->acceptVisitor(ev);            // base and index
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->acceptVisitor(ev);
    }
    else if (lhs->getOper() == opAt) { // foo@[first:last] uses foo, first, and last
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->acceptVisitor(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->acceptVisitor(ev);
        SharedExp subExp3 = lhs->getSubExp3();
        subExp3->acceptVisitor(ev);
    }

    visitChildren = false; // Don't do the usual accept logic
    return true;           // Continue the recursion
}


bool UsedLocsVisitor::visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    // Special logic for the LHS
    if (lhs->isMemOf()) {
        SharedExp child     = lhs->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->acceptVisitor(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if (lhs->isArrayIndex() || lhs->isMemberOf()) {
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->acceptVisitor(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->acceptVisitor(ev);
    }

    for (const std::shared_ptr<RefExp> &refExp : *stmt) {
        // Note: don't make the RefExp based on lhs, since it is possible that the lhs was renamed
        // in fromSSA(); use the actual expression in the PhiAssign.
        // Also note that it's possible for uu->e to be nullptr.
        // Suppose variable a can be assigned to along in-edges 0, 1, and 3;
        // inserting the phi parameter at index 3 will cause a null entry at 2
        assert(refExp->getSubExp1());
        auto temp = RefExp::get(refExp->getSubExp1(), refExp->getDef());
        temp->acceptVisitor(ev);
    }

    visitChildren = false; // Don't do the usual accept logic
    return true;           // Continue the recursion
}


bool UsedLocsVisitor::visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren)
{
    SharedExp lhs = stmt->getLeft();

    // Special logic for the LHS
    if (lhs->isMemOf()) {
        SharedExp child     = lhs->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            child->acceptVisitor(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if (lhs->isArrayIndex() || lhs->isMemberOf()) {
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->acceptVisitor(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->acceptVisitor(ev);
    }

    visitChildren = false; // Don't do the usual accept logic
    return true;           // Continue the recursion
}


bool UsedLocsVisitor::visit(const std::shared_ptr<CallStatement> &stmt, bool &visitChildren)
{
    SharedExp condExp = stmt->getDest();

    if (condExp) {
        condExp->acceptVisitor(ev);
    }


    const StatementList &arguments = stmt->getArguments();

    for (SharedStmt s : arguments) {
        // Don't want to ever collect anything from the lhs
        if (s->isAssign()) {
            s->as<Assign>()->getRight()->acceptVisitor(ev);
        }
    }

    if (m_countCols) {
        for (const std::shared_ptr<Assign> &as : *stmt->getDefCollector()) {
            as->accept(this);
        }
    }

    visitChildren = false; // Don't do the normal accept logic
    return true;           // Continue the recursion
}


bool UsedLocsVisitor::visit(const std::shared_ptr<ReturnStatement> &stmt, bool &visitChildren)
{
    // For the final pass, only consider the first return
    for (SharedStmt ret : *stmt) {
        ret->accept(this);
    }

    // Also consider the reaching definitions to be uses, so when they are the only non-empty
    // component of this ReturnStatement, they can get propagated to.
    if (m_countCols) { // But we need to ignore these "uses" unless propagating
        for (const auto &asgn : *stmt->getCollector()) {
            asgn->accept(this);
        }
    }

    visitChildren = false; // Don't do the normal accept logic
    return true;           // Continue the recursion
}


bool UsedLocsVisitor::visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren)
{
    SharedExp condExp = stmt->getCondExpr();

    if (condExp) {
        condExp->acceptVisitor(ev); // Condition is used
    }

    SharedExp lhs = stmt->getLeft();
    assert(lhs);

    if (lhs->isMemOf()) { // If dest is of form m[x]...
        SharedExp x         = lhs->getSubExp1();
        UsedLocsFinder *ulf = dynamic_cast<UsedLocsFinder *>(ev);

        if (ulf) {
            bool wasMemOnly = ulf->isMemOnly();
            ulf->setMemOnly(false);
            x->acceptVisitor(ev);
            ulf->setMemOnly(wasMemOnly);
        }
    }
    else if (lhs->isArrayIndex() || lhs->isMemberOf()) {
        SharedExp subExp1 = lhs->getSubExp1();
        subExp1->acceptVisitor(ev);
        SharedExp subExp2 = lhs->getSubExp2();
        subExp2->acceptVisitor(ev);
    }

    visitChildren = false; // Don't do the normal accept logic
    return true;           // Continue the recursion
}
