#include "ReturnStatement.h"

#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/Visitor.h"
#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/util/Log.h"

#include "boomerang/frontend/Frontend.h"

ReturnStatement::ReturnStatement()
    : retAddr(Address::INVALID)
{
    m_kind = STMT_RET;
}


ReturnStatement::~ReturnStatement()
{
}


Statement *ReturnStatement::clone() const
{
    ReturnStatement *ret = new ReturnStatement();

    for (auto const& elem : modifieds) {
        ret->modifieds.append((ImplicitAssign *)(elem)->clone());
    }

    for (auto const& elem : returns) {
        ret->returns.append((Assignment *)(elem)->clone());
    }

    ret->retAddr = retAddr;
    ret->col.makeCloneOf(col);
    // Statement members
    ret->m_parent = m_parent;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


bool ReturnStatement::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


void ReturnStatement::generateCode(ICodeGenerator *hll, BasicBlock *pbb, int indLevel)
{
    Q_UNUSED(pbb);
    hll->addReturnStatement(indLevel, &getReturns());
}


void ReturnStatement::simplify()
{
    iterator it;

    for (it = modifieds.begin(); it != modifieds.end(); it++) {
        (*it)->simplify();
    }

    for (it = returns.begin(); it != returns.end(); it++) {
        (*it)->simplify();
    }
}


void ReturnStatement::removeReturn(SharedExp loc)
{
    if (loc->isSubscript()) {
        loc = loc->getSubExp1();
    }

    for (iterator rr = returns.begin(); rr != returns.end(); ++rr) {
        if (*((Assignment *)*rr)->getLeft() == *loc) {
            returns.erase(rr);
            return;     // Assume only one definition
        }
    }
}


void ReturnStatement::addReturn(Assignment *a)
{
    returns.append(a);
}


bool ReturnStatement::search(const Exp& search, SharedExp& result) const
{
    result = nullptr;

    for (auto rr = begin(); rr != end(); ++rr) {
        if ((*rr)->search(search, result)) {
            return true;
        }
    }

    return false;
}


bool ReturnStatement::searchAndReplace(const Exp& search, SharedExp replace, bool cc)
{
    bool change = false;

    ReturnStatement::iterator rr;

    for (rr = begin(); rr != end(); ++rr) {
        change |= (*rr)->searchAndReplace(search, replace, cc);
    }

    if (cc) {
        DefCollector::iterator dd;

        for (dd = col.begin(); dd != col.end(); ++dd) {
            change |= (*dd)->searchAndReplace(search, replace);
        }
    }

    return change;
}


bool ReturnStatement::searchAll(const Exp& search, std::list<SharedExp>& result) const
{
    bool found = false;

    for (auto rr = begin(); rr != end(); ++rr) {
        if ((*rr)->searchAll(search, result)) {
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
    bool override;

    ReturnStatement::iterator rr;

    if (!v->visit(this, override)) {
        return false;
    }

    if (override) {
        return true;
    }

    if (!v->isIgnoreCol()) {
        DefCollector::iterator dd;

        for (dd = col.begin(); dd != col.end(); ++dd) {
            if (!(*dd)->accept(v)) {
                return false;
            }
        }

        // EXPERIMENTAL: for now, count the modifieds as if they are a collector (so most, if not all of the time,
        // ignore them). This is so that we can detect better when a definition is used only once, and therefore
        // propagate anything to it
        for (rr = modifieds.begin(); rr != modifieds.end(); ++rr) {
            if (!(*rr)->accept(v)) {
                return false;
            }
        }
    }

    for (rr = returns.begin(); rr != returns.end(); ++rr) {
        if (!(*rr)->accept(v)) {
            return false;
        }
    }

    return true;
}


bool ReturnStatement::accept(StmtModifier *v)
{
    bool recur;

    v->visit(this, recur);

    if (!recur) {
        return true;
    }

    if (!v->ignoreCollector()) {
        DefCollector::iterator dd;

        for (dd = col.begin(); dd != col.end(); ++dd) {
            if (!(*dd)->accept(v)) {
                return false;
            }
        }
    }

    ReturnStatement::iterator rr;

    for (rr = modifieds.begin(); rr != modifieds.end(); ++rr) {
        if (!(*rr)->accept(v)) {
            return false;
        }
    }

    for (rr = returns.begin(); rr != returns.end(); ++rr) {
        if (!(*rr)->accept(v)) {
            return false;
        }
    }

    return true;
}


bool ReturnStatement::accept(StmtPartModifier *v)
{
    bool recur;

    v->visit(this, recur);
    ReturnStatement::iterator rr;

    for (rr = modifieds.begin(); rr != modifieds.end(); ++rr) {
        if (!(*rr)->accept(v)) {
            return false;
        }
    }

    for (rr = returns.begin(); rr != returns.end(); ++rr) {
        if (!(*rr)->accept(v)) {
            return false;
        }
    }

    return true;
}


bool ReturnStatement::definesLoc(SharedExp loc) const
{
    for (auto& elem : modifieds) {
        if ((elem)->definesLoc(loc)) {
            return true;
        }
    }

    return false;
}


void ReturnStatement::getDefinitions(LocationSet& ls) const
{
    for (auto& elem : modifieds) {
        (elem)->getDefinitions(ls);
    }
}


SharedType ReturnStatement::getTypeFor(SharedExp e) const
{
    for (auto& elem : modifieds) {
        if (*((Assignment *)elem)->getLeft() == *e) {
            return ((Assignment *)elem)->getType();
        }
    }

    return nullptr;
}


void ReturnStatement::setTypeFor(SharedExp e, SharedType ty)
{
    for (auto& elem : modifieds) {
        if (*((Assignment *)elem)->getLeft() == *e) {
            ((Assignment *)elem)->setType(ty);
            break;
        }
    }

    for (auto& elem : returns) {
        if (*((Assignment *)elem)->getLeft() == *e) {
            ((Assignment *)elem)->setType(ty);
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

    for (auto const& elem : returns) {
        QString     tgt;
        QTextStream ost(&tgt);
        ((const Assignment *)elem)->printCompact(ost, html);
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

    for (auto const& elem : modifieds) {
        QString          tgt2;
        QTextStream      ost(&tgt2);
        const Assignment *as = (const Assignment *)elem;
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
    col.print(os, html);
}


void ReturnStatement::updateModifieds()
{
    auto          sig = m_proc->getSignature();
    StatementList oldMods(modifieds);     // Copy the old modifieds

    modifieds.clear();

    if ((m_parent->getNumInEdges() == 1) && m_parent->getInEdges()[0]->getLastStmt()->isCall()) {
        CallStatement *call = (CallStatement *)m_parent->getInEdges()[0]->getLastStmt();

        if (call->getDestProc() && IFrontEnd::isNoReturnCallDest(call->getDestProc()->getName())) {
            return;
        }
    }

    // For each location in the collector, make sure that there is an assignment in the old modifieds, which will
    // be filtered and sorted to become the new modifieds
    // Ick... O(N*M) (N existing modifeds, M collected locations)

    StatementList::iterator it;

    for (DefCollector::iterator ll = col.begin(); ll != col.end(); ++ll) {
        bool      found  = false;
        Assign    *as    = (Assign *)*ll;
        SharedExp colLhs = as->getLeft();

        if (m_proc->filterReturns(colLhs)) {
            continue;     // Filtered out
        }

        for (it = oldMods.begin(); it != oldMods.end(); it++) {
            SharedExp lhs = ((Assignment *)*it)->getLeft();

            if (*lhs == *colLhs) {
                found = true;
                break;
            }
        }

        if (!found) {
            ImplicitAssign *ias = new ImplicitAssign(as->getType()->clone(), as->getLeft()->clone());
            ias->setProc(m_proc);     // Comes from the Collector
            ias->setBB(m_parent);
            oldMods.append(ias);
        }
    }

    // Mostly the old modifications will be in the correct order, and inserting will be fastest near the start of
    // the
    // new list. So read the old modifications in reverse order
    for (it = oldMods.end(); it != oldMods.begin(); ) {
        --it;     // Becuase we are using a forwards iterator backwards
        // Make sure the LHS is still in the collector
        Assignment *as = (Assignment *)*it;
        SharedExp  lhs = as->getLeft();

        if (!col.existsOnLeft(lhs)) {
            continue;     // Not in collector: delete it (don't copy it)
        }

        if (m_proc->filterReturns(lhs)) {
            continue;     // Filtered out: delete it
        }

        // Insert as, in order, into the existing set of modifications
        StatementList::iterator nn;
        bool inserted = false;

        for (nn = modifieds.begin(); nn != modifieds.end(); ++nn) {
            if (sig->returnCompare(*as, *(Assignment *)*nn)) {     // If the new assignment is less than the current one
                nn       = modifieds.insert(nn, as);               // then insert before this position
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            modifieds.insert(modifieds.end(), as);     // In case larger than all existing elements
        }
    }
}


void ReturnStatement::updateReturns()
{
    auto          sig = m_proc->getSignature();
    int           sp  = sig->getStackRegister();
    StatementList oldRets(returns);     // Copy the old returns

    returns.clear();
    // For each location in the modifieds, make sure that there is an assignment in the old returns, which will
    // be filtered and sorted to become the new returns
    // Ick... O(N*M) (N existing returns, M modifieds locations)
    StatementList::iterator dd, it;

    for (dd = modifieds.begin(); dd != modifieds.end(); ++dd) {
        bool      found = false;
        SharedExp loc   = ((Assignment *)*dd)->getLeft();

        if (m_proc->filterReturns(loc)) {
            continue;     // Filtered out
        }

        // Special case for the stack pointer: it has to be a modified (otherwise, the changes will bypass the
        // calls),
        // but it is not wanted as a return
        if (loc->isRegN(sp)) {
            continue;
        }

        for (it = oldRets.begin(); it != oldRets.end(); it++) {
            SharedExp lhs = ((Assign *)*it)->getLeft();

            if (*lhs == *loc) {
                found = true;
                break;
            }
        }

        if (!found) {
            SharedExp rhs = col.findDefFor(loc);     // Find the definition that reaches the return statement's collector
            Assign    *as = new Assign(loc->clone(), rhs->clone());
            as->setProc(m_proc);
            as->setBB(m_parent);
            oldRets.append(as);
        }
    }

    // Mostly the old returns will be in the correct order, and inserting will be fastest near the start of the
    // new list. So read the old returns in reverse order
    for (it = oldRets.end(); it != oldRets.begin(); ) {
        --it;     // Becuase we are using a forwards iterator backwards
        // Make sure the LHS is still in the modifieds
        Assign    *as = (Assign *)*it;
        SharedExp lhs = as->getLeft();

        if (!modifieds.existsOnLeft(lhs)) {
            continue;     // Not in modifieds: delete it (don't copy it)
        }

        if (m_proc->filterReturns(lhs)) {
            continue;     // Filtered out: delete it
        }

        // Preserveds are NOT returns (nothing changes, so what are we returning?)
        // Check if it is a preserved location, e.g. r29 := r29{-}
        SharedExp rhs = as->getRight();

        if (rhs->isSubscript() && rhs->access<RefExp>()->isImplicitDef() && (*rhs->getSubExp1() == *lhs)) {
            continue;     // Filter out the preserveds
        }

        // Insert as, in order, into the existing set of returns
        StatementList::iterator nn;
        bool inserted = false;

        for (nn = returns.begin(); nn != returns.end(); ++nn) {
            if (sig->returnCompare(*as, *(Assign *)*nn)) {     // If the new assignment is less than the current one
                nn       = returns.insert(nn, as);             // then insert before this position
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            returns.insert(returns.end(), as);     // In case larger than all existing elements
        }
    }
}


void ReturnStatement::removeModified(SharedExp loc)
{
    modifieds.removeDefOf(loc);
    returns.removeDefOf(loc);
}


void ReturnStatement::dfaTypeAnalysis(bool& ch)
{
    for (Statement *mm : modifieds) {
        if (!mm->isAssignment()) {
            LOG_WARN("Non assignment in modifieds of ReturnStatement");
        }

        mm->dfaTypeAnalysis(ch);
    }

    for (Statement *rr : returns) {
        if (!rr->isAssignment()) {
            LOG_WARN("Non assignment in returns of ReturnStatement");
        }

        rr->dfaTypeAnalysis(ch);
    }
}
