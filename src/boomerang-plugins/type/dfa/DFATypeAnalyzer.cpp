#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DFATypeAnalyzer.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BoolAssign.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/ssl/statements/ReturnStatement.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/BooleanType.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"


DFATypeAnalyzer::DFATypeAnalyzer()
    : StmtModifier(nullptr)
{
}


void DFATypeAnalyzer::visitAssignment(Assignment *stmt, bool &visitChildren)
{
    UserProc *proc = stmt->getProc();
    assert(proc != nullptr);
    std::shared_ptr<Signature> sig = proc->getSignature();

    // Don't do this for the common case of an ordinary local,
    // since it generates hundreds of implicit references,
    // without any new type information
    const int spIndex = Util::getStackRegisterIndex(stmt->getProc()->getProg());
    if (stmt->getLeft()->isMemOf() && !sig->isStackLocal(spIndex, stmt->getLeft())) {
        SharedExp addr = stmt->getLeft()->getSubExp1();
        // Meet the assignment type with *(type of the address)
        SharedType addrType = addr->ascendType();
        SharedType memofType;

        if (addrType->resolvesToPointer()) {
            memofType = addrType->as<PointerType>()->getPointsTo();
        }
        else {
            memofType = VoidType::get();
        }

        bool ch            = false;
        SharedType newType = stmt->getType()->meetWith(memofType, ch);
        if (ch) {
            stmt->setType(newType);
            m_changed = true;
        }

        // Push down the fact that the memof operand is a pointer to the assignment type
        m_changed |= addr->descendType(PointerType::get(stmt->getType()));
    }

    visitChildren = false;
}


void DFATypeAnalyzer::visit(PhiAssign *stmt, bool &visitChildren)
{
    PhiAssign::PhiDefs &defs  = stmt->getDefs();
    PhiAssign::iterator defIt = defs.begin();

    while (defIt != defs.end() && defIt->getSubExp1() == nullptr) {
        ++defIt;
    }

    if (defIt == defs.end()) {
        // phi does not have suitable defining statements, cannot infer type information
        visitChildren = false;
        return;
    }

    if (!defIt->getDef()) {
        // Cannot infer type information of parameters or uninitialized variables.
        visitChildren = false;
        return;
    }

    assert(defIt->getDef());
    SharedType meetOfArgs = defIt->getDef()->getTypeForExp(stmt->getLeft());

    bool ch = false;

    for (++defIt; defIt != defs.end(); ++defIt) {
        RefExp &phinf = *defIt;

        if (!phinf.getDef() || !phinf.getSubExp1()) {
            continue;
        }

        assert(phinf.getDef() != nullptr);
        SharedType typeOfDef = phinf.getDef()->getTypeForExp(phinf.getSubExp1());
        meetOfArgs           = meetOfArgs->meetWith(typeOfDef, ch);
    }

    SharedType newType = stmt->getType()->meetWith(meetOfArgs, ch);
    if (ch) {
        stmt->setType(newType);
    }

    for (defIt = defs.begin(); defIt != defs.end(); ++defIt) {
        if (defIt->getSubExp1() && defIt->getDef()) {
            defIt->getDef()->meetWithFor(stmt->getType(), defIt->getSubExp1(), ch);
        }
    }

    m_changed |= ch;

    visitAssignment(dynamic_cast<Assignment *>(stmt), visitChildren); // Handle the LHS
}


void DFATypeAnalyzer::visit(Assign *stmt, bool &visitChildren)
{
    SharedType tr = stmt->getRight()->ascendType();

    bool thisChanged = false;

    // Note: useHighestPtr is set true, since the lhs could have a greater type
    // (more possibilities) than the rhs.
    // Example:
    //   Employee *employee = mananger
    SharedType newType = stmt->getType()->meetWith(tr, thisChanged, true);
    if (thisChanged) {
        stmt->setType(newType);
    }

    // This will effect rhs = rhs MEET lhs
    thisChanged = stmt->getRight()->descendType(stmt->getType());
    m_changed |= thisChanged;

    visitAssignment(stmt, thisChanged); // Handle the LHS wrt m[] operands
    visitChildren = false;
}


void DFATypeAnalyzer::visit(BoolAssign *stmt, bool &visitChildren)
{
    // Not properly implemented yet
    visitAssignment(stmt, visitChildren);
}


void DFATypeAnalyzer::visit(BranchStatement *stmt, bool &visitChildren)
{
    if (stmt->getCondExpr()) {
        m_changed |= stmt->getCondExpr()->descendType(BooleanType::get());
    }

    // Not fully implemented yet?
    visitChildren = false;
}


void DFATypeAnalyzer::visit(CallStatement *callStmt, bool &visitChildren)
{
    // Iterate through the arguments
    int n = 0;

    Function *callee = callStmt->getDestProc();

    for (Statement *aa : callStmt->getArguments()) {
        assert(aa->isAssign());
        Assign *boundArg = static_cast<Assign *>(aa);

        // Check if we have something like
        //  memcpy(dst, src, 5);
        // In this case, we set the max length of both dst and src to 5
        if (callee && !callee->getSignature()->getParamBoundMax(n).isEmpty() &&
            boundArg->getRight()->isIntConst()) {
            const QString boundmax = callStmt->getDestProc()->getSignature()->getParamBoundMax(n);
            assert(boundArg->getType()->resolvesToInteger());

            int nt = 0;
            for (const Statement *arrayArg : callStmt->getArguments()) {
                if (boundmax == callStmt->getDestProc()->getSignature()->getParamName(nt++)) {
                    SharedType tyt = static_cast<const Assign *>(arrayArg)->getType();

                    if (tyt->resolvesToPointer() &&
                        tyt->as<PointerType>()->getPointsTo()->resolvesToArray() &&
                        tyt->as<PointerType>()->getPointsTo()->as<ArrayType>()->isUnbounded()) {
                        tyt->as<PointerType>()->getPointsTo()->as<ArrayType>()->setLength(
                            boundArg->getRight()->access<Const>()->getInt());
                    }

                    break;
                }
            }
        }

        // The below will ascend type, meet type with that of arg, and descend type. Note that the
        // type of the assign will already be that of the signature, if this is a library call, from
        // updateArguments()
        visit(boundArg, visitChildren);
        ++n;
    }

    // The destination is a pointer to a function with this function's signature (if any)
    if (callStmt->getDest()) {
        if (callStmt->getSignature()) {
            m_changed |= callStmt->getDest()->descendType(
                PointerType::get(FuncType::get(callStmt->getSignature())));
        }
        else if (callStmt->getDestProc()) {
            m_changed |= callStmt->getDest()->descendType(
                PointerType::get(FuncType::get(callStmt->getDestProc()->getSignature())));
        }

        if (callStmt->getDest()->isSubscript()) {
            std::shared_ptr<RefExp> ref = callStmt->getDest()->access<RefExp>();
            Statement *def              = ref->getDef();
            def->setTypeForExp(ref->getSubExp1(), FuncType::get(callStmt->getSignature()));
        }
    }

    visitChildren = false;
}


void DFATypeAnalyzer::visit(ImplicitAssign *stmt, bool &visitChildren)
{
    visitAssignment(stmt, visitChildren);
}


void DFATypeAnalyzer::visit(ReturnStatement *stmt, bool &visitChildren)
{
    for (Statement *mm : stmt->getModifieds()) {
        if (!mm->isAssignment()) {
            LOG_WARN("Non assignment in modifieds of ReturnStatement");
        }

        assert(dynamic_cast<Assignment *>(mm) != nullptr);
        visitAssignment(static_cast<Assignment *>(mm), visitChildren);
    }

    for (Statement *rr : stmt->getReturns()) {
        if (!rr->isAssignment()) {
            LOG_WARN("Non assignment in returns of ReturnStatement");
        }

        assert(dynamic_cast<Assignment *>(rr) != nullptr);
        visitAssignment(static_cast<Assignment *>(rr), visitChildren);
    }

    visitChildren = false; // don't visit the expressions
}
