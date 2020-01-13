#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IRFragment.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/log/Log.h"


IRFragment::IRFragment(FragID fragID, BasicBlock *bb, Address lowAddr)
    : m_id(fragID)
    , m_bb(bb)
    , m_lowAddr(lowAddr)
{
}


IRFragment::IRFragment(FragID fragID, BasicBlock *bb, std::unique_ptr<RTLList> rtls)
    : m_id(fragID)
    , m_fragType(FragType::Fall)
    , m_bb(bb)
    , m_listOfRTLs(std::move(rtls))
{
    assert(m_listOfRTLs != nullptr);
    assert(!m_listOfRTLs->empty());

    updateAddresses();
}


bool IRFragment::operator<(const IRFragment &rhs) const
{
    return m_id < rhs.m_id;
}


UserProc *IRFragment::getProc()
{
    return m_bb ? m_bb->getProc() : nullptr;
}


const UserProc *IRFragment::getProc() const
{
    return m_bb ? m_bb->getProc() : nullptr;
}


RTL *IRFragment::getLastRTL()
{
    return m_listOfRTLs ? m_listOfRTLs->back().get() : nullptr;
}


const RTL *IRFragment::getLastRTL() const
{
    return m_listOfRTLs ? m_listOfRTLs->back().get() : nullptr;
}


SharedStmt IRFragment::getFirstStmt(RTLIterator &rit, RTL::iterator &sit)
{
    if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
        return nullptr;
    }

    rit = m_listOfRTLs->begin();

    while (rit != m_listOfRTLs->end()) {
        auto &rtl = *rit;
        sit       = rtl->begin();

        if (sit != rtl->end()) {
            return *sit;
        }

        ++rit;
    }

    return nullptr;
}


SharedStmt IRFragment::getNextStmt(RTLIterator &rit, RTL::iterator &sit)
{
    if (++sit != (*rit)->end()) {
        return *sit; // End of current RTL not reached, so return next
    }

    // Else, find next non-empty RTL & return its first statement
    do {
        if (++rit == m_listOfRTLs->end()) {
            return nullptr; // End of all RTLs reached, return null Statement
        }
    } while ((*rit)->empty()); // Ignore all RTLs with no statements

    sit = (*rit)->begin(); // Point to 1st statement at start of next RTL
    return *sit;           // Return first statement
}


SharedStmt IRFragment::getPrevStmt(RTLRIterator &rit, RTL::reverse_iterator &sit)
{
    if (++sit != (*rit)->rend()) {
        return *sit; // Beginning of current RTL not reached, so return next
    }

    // Else, find prev non-empty RTL & return its last statement
    do {
        if (++rit == m_listOfRTLs->rend()) {
            return nullptr; // End of all RTLs reached, return null Statement
        }
    } while ((*rit)->empty()); // Ignore all RTLs with no statements

    sit = (*rit)->rbegin(); // Point to last statement at end of prev RTL
    return *sit;            // Return last statement
}


SharedStmt IRFragment::getLastStmt(RTLRIterator &rit, RTL::reverse_iterator &sit)
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    rit = m_listOfRTLs->rbegin();

    while (rit != m_listOfRTLs->rend()) {
        auto &rtl = *rit;
        sit       = rtl->rbegin();

        if (sit != rtl->rend()) {
            return *sit;
        }

        ++rit;
    }

    return nullptr;
}


SharedStmt IRFragment::getFirstStmt()
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    for (auto &rtl : *m_listOfRTLs) {
        if (!rtl->empty()) {
            return rtl->front();
        }
    }

    return nullptr;
}

const SharedConstStmt IRFragment::getFirstStmt() const
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    for (auto &rtl : *m_listOfRTLs) {
        if (!rtl->empty()) {
            return rtl->front();
        }
    }

    return nullptr;
}


SharedStmt IRFragment::getLastStmt()
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    RTLRIterator revIt = m_listOfRTLs->rbegin();

    while (revIt != m_listOfRTLs->rend()) {
        auto &rtl = *revIt++;

        if (!rtl->empty()) {
            return rtl->back();
        }
    }

    return nullptr;
}


const SharedConstStmt IRFragment::getLastStmt() const
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    RTLRIterator revIt = m_listOfRTLs->rbegin();

    while (revIt != m_listOfRTLs->rend()) {
        auto &rtl = *revIt++;

        if (!rtl->empty()) {
            return rtl->back();
        }
    }

    return nullptr;
}


void IRFragment::appendStatementsTo(StatementList &stmts) const
{
    const RTLList *rtls = getRTLs();

    if (!rtls) {
        return;
    }

    for (const auto &rtl : *rtls) {
        for (SharedStmt &st : *rtl) {
            assert(st->getFragment() == this);
            stmts.append(st);
        }
    }
}


std::shared_ptr<ImplicitAssign> IRFragment::addImplicitAssign(const SharedExp &lhs)
{
    assert(m_listOfRTLs);

    if (m_listOfRTLs->empty() || m_listOfRTLs->front()->getAddress() != Address::ZERO) {
        m_listOfRTLs->push_front(std::unique_ptr<RTL>(new RTL(Address::ZERO)));
    }

    // do not allow BB with 2 zero address RTLs
    assert(m_listOfRTLs->size() < 2 ||
           (*std::next(m_listOfRTLs->begin()))->getAddress() != Address::ZERO);

    for (const SharedStmt &s : *m_listOfRTLs->front()) {
        if (s->isPhi() && *s->as<PhiAssign>()->getLeft() == *lhs) {
            // phis kill implicits; don't add an implict assign
            // if we already have a phi assigning to the LHS
            return nullptr;
        }
        else if (s->isImplicit() && *s->as<ImplicitAssign>()->getLeft() == *lhs) {
            // already present
            return s->as<ImplicitAssign>();
        }
    }

    // no phi or implicit assigning to the LHS already
    std::shared_ptr<ImplicitAssign> newImplicit(new ImplicitAssign(lhs));
    newImplicit->setFragment(this);

    if (m_bb) {
        newImplicit->setProc(m_bb->getProc());
    }

    m_listOfRTLs->front()->append(newImplicit);
    return newImplicit;
}


std::shared_ptr<PhiAssign> IRFragment::addPhi(const SharedExp &usedExp)
{
    assert(m_listOfRTLs);

    if (m_listOfRTLs->empty() || m_listOfRTLs->front()->getAddress() != Address::ZERO) {
        m_listOfRTLs->push_front(std::unique_ptr<RTL>(new RTL(Address::ZERO)));
    }

    // do not allow BB with 2 zero address RTLs
    assert(m_listOfRTLs->size() < 2 ||
           (*std::next(m_listOfRTLs->begin()))->getAddress() != Address::ZERO);

    for (auto existingIt = m_listOfRTLs->front()->begin();
         existingIt != m_listOfRTLs->front()->end();) {
        SharedStmt s = *existingIt;
        if (s->isPhi() && *s->as<PhiAssign>()->getLeft() == *usedExp) {
            // already present
            return s->as<PhiAssign>();
        }
        else if (s->isAssignment() && *s->as<Assignment>()->getLeft() == *usedExp) {
            // the LHS is already assigned to properly, don't create a second assignment
            return nullptr;
        }

        ++existingIt;
    }

    std::shared_ptr<PhiAssign> phi(new PhiAssign(usedExp));
    phi->setFragment(this);

    if (m_bb) {
        phi->setProc(m_bb->getProc());
    }

    m_listOfRTLs->front()->append(phi);
    return phi;
}


void IRFragment::clearPhis()
{
    RTLIterator rit;
    StatementList::iterator sit;
    for (SharedStmt s = getFirstStmt(rit, sit); s; s = getNextStmt(rit, sit)) {
        if (!s->isPhi()) {
            continue;
        }

        s->as<PhiAssign>()->getDefs().clear();
    }
}


bool IRFragment::hasStatement(const SharedStmt &stmt) const
{
    if (!stmt || !m_listOfRTLs) {
        return false;
    }

    for (const auto &rtl : *m_listOfRTLs) {
        for (const SharedStmt &s : *rtl) {
            if (s == stmt) {
                return true;
            }
        }
    }

    return false;
}


void IRFragment::removeRTL(RTL *rtl)
{
    if (!m_listOfRTLs) {
        return;
    }

    RTLList::iterator it = std::find_if(
        m_listOfRTLs->begin(), m_listOfRTLs->end(),
        [rtl](const std::unique_ptr<RTL> &rtl2) { return rtl == rtl2.get(); });

    if (it != m_listOfRTLs->end()) {
        m_listOfRTLs->erase(it);
        updateAddresses();
    }
}


Address IRFragment::getLowAddr() const
{
    return m_lowAddr;
}


Address IRFragment::getHiAddr() const
{
    return m_highAddr;
}


void IRFragment::updateAddresses()
{
    if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
        m_highAddr = Address::INVALID;
        return;
    }

    Address a = m_listOfRTLs->front()->getAddress();

    if (a.isZero() && (m_listOfRTLs->size() > 1)) {
        RTLList::iterator it = m_listOfRTLs->begin();
        const Address add2   = (*++it)->getAddress();

        // This is a bit of a hack for 286 programs, whose main actually starts at offset 0. A
        // better solution would be to change orphan fragments' addresses to Address::INVALID, but I
        // suspect that this will cause many problems. MVE
        if (add2 < Address(0x10)) {
            // Assume that 0 is the real address
            m_lowAddr = Address::ZERO;
        }
        else {
            m_lowAddr = add2;
        }
    }
    else {
        m_lowAddr = a;
    }

    assert(m_listOfRTLs != nullptr);
    m_highAddr = m_listOfRTLs->back()->getAddress();
}


bool IRFragment::isEmpty() const
{
    if (getRTLs() == nullptr) {
        return true;
    }

    for (const auto &rtl : *getRTLs()) {
        if (!rtl->empty()) {
            return false;
        }
    }

    return true;
}


bool IRFragment::isEmptyJump() const
{
    if (m_listOfRTLs == nullptr || m_listOfRTLs->empty()) {
        return false;
    }
    else if (m_listOfRTLs->back()->size() != 1) {
        return false;
    }
    else if (!m_listOfRTLs->back()->back()->isGoto()) {
        return false;
    }
    else {
        for (auto it = m_listOfRTLs->begin(); it != std::prev(m_listOfRTLs->end()); ++it) {
            if (!(*it)->empty()) {
                return false;
            }
        }
    }

    return true;
}


Function *IRFragment::getCallDestProc() const
{
    if (!m_bb->isType(BBType::Call) || !m_listOfRTLs || m_listOfRTLs->empty()) {
        return nullptr;
    }

    RTL *lastRTL = m_listOfRTLs->back().get();

    // search backwards for a CallStatement
    for (auto it = lastRTL->rbegin(); it != lastRTL->rend(); ++it) {
        if ((*it)->getKind() == StmtType::Call) {
            return (*it)->as<CallStatement>()->getDestProc();
        }
    }

    return nullptr;
}


SharedExp IRFragment::getCond() const
{
    // the condition will be in the last rtl
    if (!m_listOfRTLs || m_listOfRTLs->empty()) {
        return nullptr;
    }

    RTL *last = m_listOfRTLs->back().get();
    if (!last->getHlStmt() || !last->getHlStmt()->isBranch()) {
        return nullptr;
    }

    return last->getHlStmt()->as<BranchStatement>()->getCondExpr();
}


SharedExp IRFragment::getDest() const
{
    // The destianation will be in the last rtl
    if (!m_listOfRTLs || m_listOfRTLs->empty()) {
        return nullptr;
    }

    const RTL *lastRTL = getLastRTL();

    // It should contain a GotoStatement or derived class
    SharedStmt lastStmt = lastRTL->getHlStmt();
    if (!lastStmt) {
        if (getNumSuccessors() > 0) {
            return Const::get(m_bb->getSuccessor(BTHEN)->getLowAddr());
        }
        else {
            return nullptr;
        }
    }

    if (lastStmt->isCase()) {
        // Get the expression from the switch info
        const SwitchInfo *si = lastStmt->as<CaseStatement>()->getSwitchInfo();

        if (si) {
            return si->switchExp;
        }
    }
    else if (lastStmt->isGoto()) {
        return lastStmt->as<GotoStatement>()->getDest();
    }

    LOG_ERROR("Last statement of fragment at address %1 is not a goto!", m_bb->getLowAddr());
    return nullptr;
}


void IRFragment::setCond(const SharedExp &e)
{
    // the condition will be in the last rtl
    assert(m_listOfRTLs);
    assert(!m_listOfRTLs->empty());

    RTL *last = m_listOfRTLs->back().get();
    assert(!last->empty());

    // it should contain a BranchStatement
    for (auto it = last->rbegin(); it != last->rend(); ++it) {
        if ((*it)->isBranch()) {
            (*it)->as<BranchStatement>()->setCondExpr(e);
            return;
        }
    }
}


void IRFragment::simplify()
{
    if (m_listOfRTLs) {
        for (auto &rtl : *m_listOfRTLs) {
            rtl->simplify();
        }
    }

    if (isType(FragType::Twoway)) {
        assert(getNumSuccessors() > 1);

        if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
            setType(FragType::Fall);
        }
        else {
            RTL *last = m_listOfRTLs->back().get();

            if (last->size() == 0) {
                setType(FragType::Fall);
            }
            else if (last->back()->isGoto()) {
                setType(FragType::Oneway);
            }
            else if (!last->back()->isBranch()) {
                setType(FragType::Fall);
            }
            else if (getNumSuccessors() == 2 && getSuccessor(BTHEN) == getSuccessor(BELSE)) {
                setType(FragType::Oneway);
            }
        }

        if (isType(FragType::Fall)) {
            IRFragment *redundant = getSuccessor(BTHEN);
            removeSuccessor(redundant);
            redundant->removePredecessor(this);
        }
        else if (isType(FragType::Oneway)) {
            IRFragment *redundant = getSuccessor(BELSE);
            removeSuccessor(redundant);
            redundant->removePredecessor(this);
        }

        assert(m_bb->getProc()->getCFG()->isWellFormed());
    }
}


void IRFragment::print(OStream &os) const
{
    switch (getType()) {
    case FragType::Oneway: os << "Oneway"; break;
    case FragType::Twoway: os << "Twoway"; break;
    case FragType::Nway: os << "Nway"; break;
    case FragType::Call: os << "Call"; break;
    case FragType::Ret: os << "Ret"; break;
    case FragType::Fall: os << "Fall"; break;
    case FragType::CompJump: os << "Computed Jump"; break;
    case FragType::CompCall: os << "Computed Call"; break;
    case FragType::Invalid: os << "Invalid"; break;
    }

    os << " Fragment:\n";
    os << "  in edges: ";

    for (IRFragment *frag : getPredecessors()) {
        os << frag->getHiAddr() << "(" << frag->getLowAddr() << ") ";
    }

    os << "\n";
    os << "  out edges: ";

    for (IRFragment *frag : getSuccessors()) {
        os << frag->getLowAddr() << " ";
    }

    os << "\n";

    if (m_listOfRTLs) { // Can be null if e.g. INVALID
        for (auto &rtl : *m_listOfRTLs) {
            rtl->print(os);
        }
    }
}


QString IRFragment::toString() const
{
    QString result;
    OStream os(&result);
    print(os);
    return result;
}
