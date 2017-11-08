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
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/ReturnStatement.h"
#include "boomerang/type/type/BooleanType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/util/Log.h"


DFATypeAnalyzer::DFATypeAnalyzer()
    : StmtModifier(nullptr)
{
}


void DFATypeAnalyzer::visitAssignment(Assignment* stmt, bool& visitChildren)
{
    auto sig = stmt->getProc()->getSignature();

    // Don't do this for the common case of an ordinary local,
    // since it generates hundreds of implicit references,
    // without any new type information
    if (stmt->getLeft()->isMemOf() && !sig->isStackLocal(stmt->getProc()->getProg(), stmt->getLeft())) {
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

        bool ch = false;
        SharedType newType = stmt->getType()->meetWith(memofType, ch);
        if (ch) {
            stmt->setType(newType);
            m_changed = true;
        }

        // Push down the fact that the memof operand is a pointer to the assignment type
        addrType = PointerType::get(stmt->getType());
        addr->descendType(addrType, ch, stmt);
    }

    visitChildren = true;
}


void DFATypeAnalyzer::visit(PhiAssign* stmt, bool& visitChildren)
{
    PhiAssign::PhiDefs& defs = stmt->getDefs();
    PhiAssign::iterator it = defs.begin();

    while (it->second.e == nullptr && it != defs.end()) {
        ++it;
    }

    assert(it != defs.end());
    SharedType meetOfArgs = it->second.getDef()->getTypeFor(stmt->getLeft());

    bool ch = false;

    for (++it; it != defs.end(); ++it) {
        PhiInfo& phinf(it->second);

        if (phinf.e == nullptr) {
            continue;
        }

        assert(phinf.getDef() != nullptr);
        SharedType typeOfDef = phinf.getDef()->getTypeFor(phinf.e);
        meetOfArgs = meetOfArgs->meetWith(typeOfDef, ch);
    }

    SharedType newType = stmt->getType()->meetWith(meetOfArgs, ch);
    if (ch) {
        stmt->setType(newType);
    }

    for (it = defs.begin(); it != defs.end(); ++it) {
        if (it->second.e == nullptr) {
            continue;
        }

        it->second.getDef()->meetWithFor(stmt->getType(), it->second.e, ch);
    }

    m_changed |= ch;

    visitAssignment(dynamic_cast<Assignment *>(stmt), visitChildren); // Handle the LHS
}


void DFATypeAnalyzer::visit(Assign* stmt, bool& visitChildren)
{
    SharedType tr = stmt->getRight()->ascendType();

    bool ch = false;

    // Note: bHighestPtr is set true, since the lhs could have a greater type
    // (more possibilities) than the rhs. Example: pEmployee = pManager.
    SharedType newType = stmt->getType()->meetWith(tr, ch, true);
    if (ch) {
        stmt->setType(newType);
    }

    // This will effect rhs = rhs MEET lhs
    stmt->getRight()->descendType(stmt->getType(), ch, stmt);

    m_changed |= ch;

    visitAssignment(stmt, ch);  // Handle the LHS wrt m[] operands
    visitChildren = true;
}


void DFATypeAnalyzer::visit(BoolAssign *stmt, bool& visitChildren)
{
    // Not properly implemented yet
    visitAssignment(stmt, visitChildren);
}


void DFATypeAnalyzer::visit(BranchStatement *stmt, bool& visitChildren)
{
    if (stmt->getCondExpr()) {
        bool ch = false;
        stmt->getCondExpr()->descendType(BooleanType::get(), ch, stmt);
        m_changed |= ch;
    }

    // Not fully implemented yet?
    visitChildren = true;
}


void DFATypeAnalyzer::visit(CallStatement* stmt, bool& visitChildren)
{
    // Iterate through the arguments
    int n = 0;

    for (Statement *aa : stmt->getArguments()) {
        Assign *param = dynamic_cast<Assign *>(aa);
        assert(param);

        if (stmt->getDestProc() && !stmt->getDestProc()->getSignature()->getParamBoundMax(n).isNull() && param->getRight()->isIntConst()) {
            QString boundmax = stmt->getDestProc()->getSignature()->getParamBoundMax(n);
            assert(param->getType()->resolvesToInteger());

            int nt = 0;

            for (StatementList::iterator aat = stmt->getArguments().begin(); aat != stmt->getArguments().end(); ++aat, ++nt) {
                if (boundmax == stmt->getDestProc()->getSignature()->getParamName(nt)) {
                    SharedType tyt = ((Assign *)*aat)->getType();

                    if (tyt->resolvesToPointer() && tyt->as<PointerType>()->getPointsTo()->resolvesToArray() &&
                        tyt->as<PointerType>()->getPointsTo()->as<ArrayType>()->isUnbounded()) {
                        tyt->as<PointerType>()->getPointsTo()->as<ArrayType>()->setLength(
                            param->getRight()->access<Const>()->getInt());
                    }

                    break;
                }
            }
        }

        // The below will ascend type, meet type with that of arg, and descend type. Note that the type of the assign
        // will already be that of the signature, if this is a library call, from updateArguments()
        visit(param, visitChildren);
        ++n;
    }

    // The destination is a pointer to a function with this function's signature (if any)
    if (stmt->getDest()) {
        bool ch = false;
        if (stmt->getSignature()) {
            stmt->getDest()->descendType(FuncType::get(stmt->getSignature()), ch, stmt);
        }
        else if (stmt->getDestProc()) {
            stmt->getDest()->descendType(FuncType::get(stmt->getSignature()), ch, stmt);
        }
        m_changed |= ch;
    }
}


void DFATypeAnalyzer::visit(ImplicitAssign* stmt, bool& visitChildren)
{
    visitAssignment(stmt, visitChildren);
}


void DFATypeAnalyzer::visit(ReturnStatement* stmt, bool& visitChildren)
{
    for (Statement *mm : stmt->getModifieds()) {
        if (!mm->isAssignment()) {
            LOG_WARN("Non assignment in modifieds of ReturnStatement");
        }

        visitAssignment(dynamic_cast<Assignment *>(mm), visitChildren);
    }

    for (Statement *rr : stmt->getReturns()) {
        if (!rr->isAssignment()) {
            LOG_WARN("Non assignment in returns of ReturnStatement");
        }

        visitAssignment(dynamic_cast<Assignment *>(rr), visitChildren);
    }

    visitChildren = true;
}
