#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ReturnStatement.h"


#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/frontend/DefaultFrontEnd.h"
#include "boomerang/ifc/ICodeGenerator.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtmodifier/StmtModifier.h"
#include "boomerang/visitor/stmtmodifier/StmtPartModifier.h"
#include "boomerang/visitor/stmtvisitor/StmtVisitor.h"


ReturnStatement::ReturnStatement()
    : m_retAddr(Address::INVALID)
{
    m_kind = StmtType::Ret;
}


ReturnStatement::~ReturnStatement()
{
    qDeleteAll(m_returns);
    qDeleteAll(m_modifieds);
}


Statement *ReturnStatement::clone() const
{
    ReturnStatement *ret = new ReturnStatement();

    for (auto const& elem : m_modifieds) {
        ret->m_modifieds.append(static_cast<ImplicitAssign *>(elem)->clone());
    }

    for (auto const& elem : m_returns) {
        ret->m_returns.append(static_cast<Assignment *>(elem)->clone());
    }

    ret->m_retAddr = m_retAddr;
    ret->m_col.makeCloneOf(m_col);

    // Statement members
    ret->m_bb     = m_bb;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


ReturnStatement::iterator ReturnStatement::erase(ReturnStatement::iterator it)
{
    auto endIt = end();
    Q_UNUSED(endIt);
    assert(it != endIt);

    Statement *removed = *it;
    it = m_returns.erase(it);
    delete removed;

    return it;
}


bool ReturnStatement::accept(StmtVisitor *visitor) const
{
    return visitor->visit(this);
}


void ReturnStatement::generateCode(ICodeGenerator *gen, const BasicBlock *)
{
    gen->addReturnStatement(&getReturns());
}


void ReturnStatement::simplify()
{
    for (Statement *s : m_modifieds) {
        s->simplify();
    }

    for (Statement *s : m_returns) {
        s->simplify();
    }
}


void ReturnStatement::addReturn(Assignment *a)
{
    m_returns.append(a);
}


bool ReturnStatement::search(const Exp& pattern, SharedExp& result) const
{
    result = nullptr;

    for (auto rr = begin(); rr != end(); ++rr) {
        if ((*rr)->search(pattern, result)) {
            return true;
        }
    }

    return false;
}


bool ReturnStatement::searchAndReplace(const Exp& pattern, SharedExp replace, bool cc)
{
    bool change = false;

    ReturnStatement::iterator rr;

    for (rr = begin(); rr != end(); ++rr) {
        change |= (*rr)->searchAndReplace(pattern, replace, cc);
    }

    if (cc) {
        DefCollector::iterator dd;

        for (dd = m_col.begin(); dd != m_col.end(); ++dd) {
            change |= (*dd)->searchAndReplace(pattern, replace);
        }
    }

    return change;
}


bool ReturnStatement::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    bool found = false;

    for (auto rr = begin(); rr != end(); ++rr) {
        if ((*rr)->searchAll(pattern, result)) {
            found = true;
        }
    }

    return found;
}


bool ReturnStatement::usesExp(const Exp& e) const
{
    SharedExp where;

    for (auto rr = begin(); rr != end(); ++rr) {
        if ((*rr)->search(e, where)) {
            return true;
        }
    }

    return false;
}


bool ReturnStatement::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;

    if (!v->visit(this, visitChildren)) {
        return false;
    }
    else if (!visitChildren) {
        return true;
    }

    if (!v->isIgnoreCol()) {
        for (DefCollector::iterator dd = m_col.begin(); dd != m_col.end(); ++dd) {
            if (!(*dd)->accept(v)) {
                return false;
            }
        }

        // EXPERIMENTAL: for now, count the modifieds as if they are a collector (so most, if not all of the time,
        // ignore them). This is so that we can detect better when a definition is used only once, and therefore
        // propagate anything to it
        for (ReturnStatement::iterator rr = m_modifieds.begin(); rr != m_modifieds.end(); ++rr) {
            if (!(*rr)->accept(v)) {
                return false;
            }
        }
    }

    for (Statement *stmt : m_returns) {
        if (!stmt->accept(v)) {
            return false;
        }
    }

    return true;
}


bool ReturnStatement::accept(StmtModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    if (!visitChildren) {
        return true;
    }

    if (!v->ignoreCollector()) {
        DefCollector::iterator dd;

        for (dd = m_col.begin(); dd != m_col.end(); ++dd) {
            if (!(*dd)->accept(v)) {
                return false;
            }
        }
    }

    for (Statement *stmt : m_modifieds) {
        if (!stmt->accept(v)) {
            return false;
        }
    }

    for (Statement *stmt : m_returns) {
        if (!stmt->accept(v)) {
            return false;
        }
    }

    return true;
}


bool ReturnStatement::accept(StmtPartModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    for (Statement *stmt : m_modifieds) {
        if (!stmt->accept(v)) {
            return false;
        }
    }

    for (Statement *stmt : m_returns) {
        if (!stmt->accept(v)) {
            return false;
        }
    }

    return true;
}


bool ReturnStatement::definesLoc(SharedExp loc) const
{
    /// Does a ReturnStatement define anything?
    /// Not really, the locations are already defined earlier in the procedure.
    /// However, nothing comes after the return statement, so it doesn't hurt
    /// to pretend it does, and this is a place to store the return type(s) for example.
    /// FIXME: seems it would be cleaner to say that Return Statements don't define anything.

    for (auto& elem : m_modifieds) {
        if ((elem)->definesLoc(loc)) {
            return true;
        }
    }

    return false;
}


void ReturnStatement::getDefinitions(LocationSet& ls, bool assumeABICompliance) const
{
    for (auto& elem : m_modifieds) {
        (elem)->getDefinitions(ls, assumeABICompliance);
    }
}


SharedConstType ReturnStatement::getTypeFor(SharedConstExp e) const
{
    for (auto& elem : m_modifieds) {
        if (*static_cast<Assignment *>(elem)->getLeft() == *e) {
            return static_cast<Assignment *>(elem)->getType();
        }
    }

    return nullptr;
}


SharedType ReturnStatement::getTypeFor(SharedExp e)
{
    for (auto& elem : m_modifieds) {
        if (*static_cast<Assignment *>(elem)->getLeft() == *e) {
            return static_cast<Assignment *>(elem)->getType();
        }
    }

    return nullptr;
}


void ReturnStatement::setTypeFor(SharedExp e, SharedType ty)
{
    for (auto& elem : m_modifieds) {
        if (*static_cast<Assignment *>(elem)->getLeft() == *e) {
            static_cast<Assignment *>(elem)->setType(ty);
            break;
        }
    }

    for (auto& elem : m_returns) {
        if (*static_cast<Assignment *>(elem)->getLeft() == *e) {
            static_cast<Assignment *>(elem)->setType(ty);
            return;
        }
    }
}


#define RETSTMT_COLS    120
void ReturnStatement::print(QTextStream& os, bool html) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

    if (html) {
        os << "</td><td>";
        os << "<a name=\"stmt" << m_number << "\">";
    }

    os << "RET";
    bool     first  = true;
    unsigned column = 19;

    for (const Statement *stmt : m_returns) {
        QString     tgt;
        QTextStream ost(&tgt);
        static_cast<const Assignment *>(stmt)->printCompact(ost, html);
        unsigned len = tgt.length();

        if (first) {
            first = false;
            os << " ";
        }
        else if (column + 4 + len > RETSTMT_COLS) {     // 4 for command 3 spaces
            if (column != RETSTMT_COLS - 1) {
                os << ",";                              // Comma at end of line
            }

            os << "\n                ";
            column = 16;
        }
        else {
            os << ",   ";
            column += 4;
        }

        os << tgt;
        column += len;
    }

    if (html) {
        os << "</a><br>";
    }
    else {
        os << "\n              ";
    }

    os << "Modifieds: ";
    first  = true;
    column = 25;

    for (auto const& elem : m_modifieds) {
        QString          tgt2;
        QTextStream      ost(&tgt2);
        const Assignment *as = static_cast<const Assignment *>(elem);
        const SharedType ty  = as->getType();

        if (ty) {
            ost << "*" << ty << "* ";
        }

        ost << as->getLeft();
        unsigned len = tgt2.length();

        if (first) {
            first = false;
        }
        else if (column + 3 + len > RETSTMT_COLS) {     // 3 for comma and 2 spaces
            if (column != RETSTMT_COLS - 1) {
                os << ",";                              // Comma at end of line
            }

            os << "\n                ";
            column = 16;
        }
        else {
            os << ",  ";
            column += 3;
        }

        os << tgt2;
        column += len;
    }

    // Collected reaching definitions
    if (html) {
        os << "<br>";
    }
    else {
        os << "\n              ";
    }

    os << "Reaching definitions: ";
    m_col.print(os, html);
}


void ReturnStatement::updateModifieds()
{
    auto          sig = m_proc->getSignature();
    StatementList oldMods(m_modifieds);     // Copy the old modifieds

    m_modifieds.clear();

    if ((m_bb->getNumPredecessors() == 1) && m_bb->getPredecessors()[0]->getLastStmt()->isCall()) {
        CallStatement *call = static_cast<CallStatement *>(m_bb->getPredecessors()[0]->getLastStmt());

        DefaultFrontEnd *fe = m_proc->getProg()->getFrontEnd();
        if (call->getDestProc() && fe->isNoReturnCallDest(call->getDestProc()->getName())) {
            return;
        }
    }

    // For each location in the collector, make sure that there is an assignment in the old modifieds, which will
    // be filtered and sorted to become the new modifieds
    // Ick... O(N*M) (N existing modifeds, M collected locations)

    for (DefCollector::iterator ll = m_col.begin(); ll != m_col.end(); ++ll) {
        bool      found  = false;
        Assign    *as    = static_cast<Assign *>(*ll);
        SharedExp colLhs = as->getLeft();

        if (m_proc->filterReturns(colLhs)) {
            continue;     // Filtered out
        }

        for (Statement *s : oldMods) {
            SharedExp lhs = static_cast<Assignment *>(s)->getLeft();

            if (*lhs == *colLhs) {
                found = true;
                break;
            }
        }

        if (!found) {
            ImplicitAssign *ias = new ImplicitAssign(as->getType()->clone(), as->getLeft()->clone());
            ias->setProc(m_proc);     // Comes from the Collector
            ias->setBB(m_bb);
            oldMods.append(ias);
        }
    }

    // Mostly the old modifications will be in the correct order, and inserting will be fastest near the start of
    // the
    // new list. So read the old modifications in reverse order
    for (StatementList::reverse_iterator it = oldMods.rbegin(); it != oldMods.rend(); ++it) {
        // Make sure the LHS is still in the collector
        Assignment *as = static_cast<Assignment *>(*it);
        SharedExp  lhs = as->getLeft();

        if (!m_col.existsOnLeft(lhs)) {
            delete *it;
            continue;     // Not in collector: delete it (don't copy it)
        }

        if (m_proc->filterReturns(lhs)) {
            delete *it;
            continue;     // Filtered out: delete it
        }

        m_modifieds.append(as);
    }

    m_modifieds.sort([&sig] (const Statement *mod1, const Statement *mod2) {
            return sig->returnCompare(*static_cast<const Assignment *>(mod1), *static_cast<const Assignment *>(mod2));
        });
}


void ReturnStatement::updateReturns()
{
    auto          sig = m_proc->getSignature();
    int           sp  = sig->getStackRegister();

    StatementList oldRets(m_returns);     // Copy the old returns
    m_returns.clear();

    // For each location in the modifieds, make sure that there is an assignment in the old returns, which will
    // be filtered and sorted to become the new returns
    // Ick... O(N*M) (N existing returns, M modifieds locations)
    for (auto dd = m_modifieds.begin(); dd != m_modifieds.end(); ++dd) {
        bool      found = false;
        SharedExp loc   = static_cast<Assignment *>(*dd)->getLeft();

        if (m_proc->filterReturns(loc)) {
            continue;     // Filtered out
        }

        // Special case for the stack pointer: it has to be a modified (otherwise, the changes will bypass the
        // calls),
        // but it is not wanted as a return
        if (loc->isRegN(sp)) {
            continue;
        }

        for (StatementList::iterator it = oldRets.begin(); it != oldRets.end(); ++it) {
            SharedExp lhs = static_cast<Assign *>(*it)->getLeft();

            if (*lhs == *loc) {
                found = true;
                break;
            }
        }

        if (!found) {
            SharedExp rhs = m_col.findDefFor(loc);     // Find the definition that reaches the return statement's collector
            Assign    *as = new Assign(loc->clone(), rhs->clone());
            as->setProc(m_proc);
            as->setBB(m_bb);
            oldRets.append(as);
        }
    }

    for (Statement *stmt : oldRets) {
        // Make sure the LHS is still in the modifieds
        assert(stmt->isAssign());
        Assign    *as = static_cast<Assign *>(stmt);
        SharedExp lhs = as->getLeft();

        if (!m_modifieds.existsOnLeft(lhs)) {
            delete stmt;
            continue;     // Not in modifieds: delete it (don't copy it)
        }

        if (m_proc->filterReturns(lhs)) {
            delete stmt;
            continue;     // Filtered out: delete it
        }

        // Preserveds are NOT returns (nothing changes, so what are we returning?)
        // Check if it is a preserved location, e.g. r29 := r29{-}
        SharedExp rhs = as->getRight();

        if (rhs->isSubscript() && rhs->access<RefExp>()->isImplicitDef() && (*rhs->getSubExp1() == *lhs)) {
            delete stmt;
            continue;     // Filter out the preserveds
        }

        m_returns.append(as);
    }

    m_returns.sort([&sig] (const Statement *ret1, const Statement *ret2) {
            return sig->returnCompare(*static_cast<const Assignment *>(ret1), *static_cast<const Assignment *>(ret2));
        });
}


void ReturnStatement::removeModified(SharedExp loc)
{
    Statement *mod = m_modifieds.removeFirstDefOf(loc);
    if (mod) {
        delete mod;
    }

    Statement *ret = m_returns.removeFirstDefOf(loc);
    if (ret) {
        delete ret;
    }
}
