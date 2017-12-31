#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "BasicBlock.h"


#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/Global.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/visitor/ConstGlobalConverter.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/util/ConnectionGraph.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Util.h"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <inttypes.h>


BasicBlock::BasicBlock(Address lowAddr, Function *function)
    : m_function(function)
    , m_lowAddr(lowAddr)
    , m_bbType(BBType::Invalid)
{
}


BasicBlock::BasicBlock(BBType bbType, std::unique_ptr<RTLList> pRtls, Function *function)
    : m_function(function)
    , m_bbType(bbType)
{
    // Set the RTLs. This also updates the low and the high address of the BB.
    setRTLs(std::move(pRtls));
}


BasicBlock::BasicBlock(const BasicBlock& bb)
    : m_function(bb.m_function)
    , m_bbType(bb.m_bbType)
    // m_labelNeeded is initialized to false, not copied
    , m_predecessors(bb.m_predecessors)
    , m_successors(bb.m_successors)
    , m_travType(bb.m_travType)
    , m_ord(bb.m_ord)
    , m_revOrd(bb.m_revOrd)
    , m_inEdgesVisited(bb.m_inEdgesVisited)
    , m_numForwardInEdges(bb.m_numForwardInEdges)
    , m_loopCondType(bb.m_loopCondType)
    , m_structType(bb.m_structType)
    , m_immPDom(bb.m_immPDom)
    , m_loopHead(bb.m_loopHead)
    , m_caseHead(bb.m_caseHead)
    , m_condFollow(bb.m_condFollow)
    , m_loopFollow(bb.m_loopFollow)
    , m_latchNode(bb.m_latchNode)
    , m_structuringType(bb.m_structuringType)
    , m_unstructuredType(bb.m_unstructuredType)
    , m_loopHeaderType(bb.m_loopHeaderType)
    , m_conditionHeaderType(bb.m_conditionHeaderType)
{
    // make a deep copy of the RTL list
    std::unique_ptr<RTLList> newList(new RTLList());
    newList->resize(bb.m_listOfRTLs->size());

    RTLList::const_iterator srcIt = bb.m_listOfRTLs->begin();
    RTLList::const_iterator endIt = bb.m_listOfRTLs->end();
    RTLList::iterator destIt = newList->begin();

    while (srcIt != endIt) {
        *destIt++ = new RTL(**srcIt++);
    }

    setRTLs(std::move(newList));
}


BasicBlock::~BasicBlock()
{
}


BasicBlock& BasicBlock::operator=(const BasicBlock& bb)
{
    *this = BasicBlock(bb);
    return *this;
}


bool BasicBlock::isCaseOption()
{
    if (!m_caseHead) {
        return false;
    }

    for (int i = 0; i < m_caseHead->getNumSuccessors() - 1; i++) {
        if (m_caseHead->getSuccessor(i) == this) {
            return true;
        }
    }

    return false;
}


void BasicBlock::setRTLs(std::unique_ptr<RTLList> rtls)
{
    if (m_listOfRTLs) {
        qDeleteAll(*m_listOfRTLs);
    }

    m_listOfRTLs = std::move(rtls);
    updateBBAddress();
}


const char *BasicBlock::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void BasicBlock::dump()
{
    QTextStream ost(stderr);

    print(ost);
}


void BasicBlock::print(QTextStream& os, bool html)
{
    if (html) {
        os << "<br>";
    }

    if (m_labelNeeded) {
        os << "L" << getLowAddr().toString() << ": ";
    }

    switch (getType())
    {
    case BBType::Oneway:
        os << "Oneway BB";
        break;

    case BBType::Twoway:
        os << "Twoway BB";
        break;

    case BBType::Nway:
        os << "Nway BB";
        break;

    case BBType::Call:
        os << "Call BB";
        break;

    case BBType::Ret:
        os << "Ret BB";
        break;

    case BBType::Fall:
        os << "Fall BB";
        break;

    case BBType::CompJump:
        os << "Computed jump BB";
        break;

    case BBType::CompCall:
        os << "Computed call BB";
        break;

    case BBType::Invalid:
        os << "Invalid BB";
        break;
    }

    os << ":\n";
    os << "  in edges: ";

    for (BasicBlock *bb : m_predecessors) {
        os << bb->getHiAddr() << "(" << bb->getLowAddr() << ") ";
    }

    os << "\n";
    os << "  out edges: ";

    for (BasicBlock *bb : m_successors) {
        os << bb->getLowAddr() << " ";
    }

    os << "\n";

    if (m_listOfRTLs) { // Can be zero if e.g. INVALID
        if (html) {
            os << "<table>\n";
        }

        for (RTL *r : *m_listOfRTLs) {
            r->print(os, html);
        }

        if (html) {
            os << "</table>\n";
        }
    }
}


void BasicBlock::printToLog()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    LOG_MSG(tgt);
}


Address BasicBlock::getLowAddr() const
{
    return m_lowAddr;
}


Address BasicBlock::getHiAddr() const
{
    return m_highAddr;
}


RTLList *BasicBlock::getRTLs()
{
    return m_listOfRTLs.get();
}


const RTLList *BasicBlock::getRTLs() const
{
    return m_listOfRTLs.get();
}


const std::vector<BasicBlock *>& BasicBlock::getPredecessors() const
{
    return m_predecessors;
}


const std::vector<BasicBlock *>& BasicBlock::getSuccessors() const
{
    return m_successors;
}


void BasicBlock::setPredecessor(int i, BasicBlock *predecessor)
{
    assert(Util::inRange(i, 0, getNumPredecessors()));
    m_predecessors[i] = predecessor;
}


void BasicBlock::setSuccessor(int i, BasicBlock *successor)
{
    assert(Util::inRange(i, 0, getNumSuccessors()));
    m_successors[i] = successor;
}


BasicBlock *BasicBlock::getPredecessor(int i)
{
    return Util::inRange(i, 0, getNumPredecessors()) ? m_predecessors[i] : nullptr;
}


const BasicBlock * BasicBlock::getPredecessor(int i) const
{
    return Util::inRange(i, 0, getNumPredecessors()) ? m_predecessors[i] : nullptr;
}


BasicBlock *BasicBlock::getSuccessor(int i)
{
    return Util::inRange(i, 0, getNumSuccessors()) ? m_successors[i] : nullptr;
}


const BasicBlock *BasicBlock::getSuccessor(int i) const
{
    return Util::inRange(i, 0, getNumSuccessors()) ? m_successors[i] : nullptr;
}


void BasicBlock::addPredecessor(BasicBlock *pNewInEdge)
{
    m_predecessors.push_back(pNewInEdge);
}


void BasicBlock::addSuccessor(BasicBlock* successor)
{
    m_successors.push_back(successor);
}


void BasicBlock::removePredecessor(BasicBlock *pred)
{
    m_predecessors.erase(std::remove(m_predecessors.begin(), m_predecessors.end(), pred), m_predecessors.end());
}


void BasicBlock::removeSuccessor(BasicBlock *succ)
{
    m_successors.erase(std::remove(m_successors.begin(), m_successors.end(), succ), m_successors.end());
}


Address BasicBlock::getCallDest()
{
    Function *dest = getCallDestProc();

    return dest ? dest->getEntryAddress() : Address::INVALID;
}


Function *BasicBlock::getCallDestProc()
{
    if (!isType(BBType::Call) || !m_listOfRTLs || m_listOfRTLs->empty()) {
        return nullptr;
    }

    RTL *lastRtl = m_listOfRTLs->back();

    // search backwards for a CallStatement
    for (auto it = lastRtl->rbegin(); it != lastRtl->rend(); it++) {
        if ((*it)->getKind() == StmtType::Call) {
            return static_cast<CallStatement *>(*it)->getDestProc();
        }
    }

    return nullptr;
}


Function *BasicBlock::getDestProc()
{
    // The last Statement of the last RTL should be a CallStatement
    CallStatement *call = (CallStatement *)(m_listOfRTLs->back()->getHlStmt());

    assert(call->getKind() == StmtType::Call);
    Function *proc = call->getDestProc();

    if (proc == nullptr) {
        LOG_FATAL("Indirect calls not handled yet.");
        assert(false);
    }

    return proc;
}


Statement *BasicBlock::getFirstStmt(rtlit& rit, StatementList::iterator& sit)
{
    if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
        return nullptr;
    }

    rit = m_listOfRTLs->begin();

    while (rit != m_listOfRTLs->end()) {
        RTL *rtl = *rit;
        sit = rtl->begin();

        if (sit != rtl->end()) {
            return *sit;
        }

        rit++;
    }

    return nullptr;
}


Statement *BasicBlock::getNextStmt(rtlit& rit, StatementList::iterator& sit)
{
    if (++sit != (*rit)->end()) {
        return *sit; // End of current RTL not reached, so return next
    }

    // Else, find next non-empty RTL & return its first statement
    do {
        if (++rit == m_listOfRTLs->end()) {
            return nullptr;    // End of all RTLs reached, return null Statement
        }
    } while ((*rit)->empty()); // Ignore all RTLs with no statements

    sit = (*rit)->begin();     // Point to 1st statement at start of next RTL
    return *sit;               // Return first statement
}


Statement *BasicBlock::getPrevStmt(rtlrit& rit, StatementList::reverse_iterator& sit)
{
    if (++sit != (*rit)->rend()) {
        return *sit; // Beginning of current RTL not reached, so return next
    }

    // Else, find prev non-empty RTL & return its last statement
    do {
        if (++rit == m_listOfRTLs->rend()) {
            return nullptr;    // End of all RTLs reached, return null Statement
        }
    } while ((*rit)->empty()); // Ignore all RTLs with no statements

    sit = (*rit)->rbegin();    // Point to last statement at end of prev RTL
    return *sit;               // Return last statement
}


Statement *BasicBlock::getLastStmt(rtlrit& rit, StatementList::reverse_iterator& sit)
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    rit = m_listOfRTLs->rbegin();

    while (rit != m_listOfRTLs->rend()) {
        RTL *rtl = *rit;
        sit = rtl->rbegin();

        if (sit != rtl->rend()) {
            return *sit;
        }

        rit++;
    }

    return nullptr;
}


Statement *BasicBlock::getFirstStmt()
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    for (RTL *rtl : *m_listOfRTLs) {
        if (!rtl->empty()) {
            return rtl->front();
        }
    }

    return nullptr;
}


Statement *BasicBlock::getLastStmt()
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    rtlrit rit = m_listOfRTLs->rbegin();

    while (rit != m_listOfRTLs->rend()) {
        RTL *rtl = *rit;

        if (!rtl->empty()) {
            return rtl->back();
        }

        rit++;
    }

    return nullptr;
}


void BasicBlock::getStatements(StatementList& stmts) const
{
    const std::list<RTL *> *rtls = getRTLs();

    if (!rtls) {
        return;
    }

    for (const RTL *rtl : *rtls) {
        for (Statement *st : *rtl) {
            if (st->getBB() == nullptr) { // TODO: why statement would have nullptr BB here ?
                st->setBB(const_cast<BasicBlock *>(this));
            }

            stmts.append(st);
        }
    }
}


SharedExp BasicBlock::getCond() const
{
    // the condition will be in the last rtl
    assert(m_listOfRTLs);
    RTL *last = m_listOfRTLs->back();

    // it should contain a BranchStatement
    BranchStatement *bs = dynamic_cast<BranchStatement *>(last->getHlStmt());

    if (bs) {
        assert(bs->getKind() == StmtType::Branch);
        return bs->getCondExpr();
    }

    return nullptr;
}


SharedExp BasicBlock::getDest() const
{
    // The destianation will be in the last rtl
    assert(m_listOfRTLs);
    RTL *lastRtl = m_listOfRTLs->back();

    // It should contain a GotoStatement or derived class
    Statement     *lastStmt = lastRtl->getHlStmt();
    CaseStatement *cs       = dynamic_cast<CaseStatement *>(lastStmt);

    if (cs) {
        // Get the expression from the switch info
        SwitchInfo *si = cs->getSwitchInfo();

        if (si) {
            return si->pSwitchVar;
        }
    }
    else {
        GotoStatement *gs = dynamic_cast<GotoStatement *>(lastStmt);

        if (gs) {
            return gs->getDest();
        }
    }

    LOG_ERROR("Last statement of BB at address %1 is not a goto!", this->getLowAddr());
    return nullptr;
}


void BasicBlock::setCond(SharedExp e)
{
    // the condition will be in the last rtl
    assert(m_listOfRTLs);
    RTL *last = m_listOfRTLs->back();
    assert(!last->empty());

    // it should contain a BranchStatement
    for (auto it = last->rbegin(); it != last->rend(); it++) {
        if ((*it)->getKind() == StmtType::Branch) {
            assert(dynamic_cast<BranchStatement *>(*it) != nullptr);
            static_cast<BranchStatement *>(*it)->setCondExpr(e);
            return;
        }
    }
}


BasicBlock *BasicBlock::getLoopBody()
{
    assert(m_structType == SBBType::PreTestLoop
        || m_structType == SBBType::PostTestLoop
        || m_structType == SBBType::EndlessLoop);

    assert(m_successors.size() == 2);

    if (m_successors[0] == m_loopFollow) {
        return m_successors[1];
    }
    else {
        return m_successors[0];
    }
}


bool BasicBlock::isAncestorOf(const BasicBlock *other) const
{
    return ((m_loopStamps[0]   < other->m_loopStamps[0]    && m_loopStamps[1] > other->m_loopStamps[1]) ||
           (m_revLoopStamps[0] < other->m_revLoopStamps[0] && m_revLoopStamps[1] > other->m_revLoopStamps[1]));
}


void BasicBlock::simplify()
{
    if (m_listOfRTLs) {
        for (RTL *elem : *m_listOfRTLs) {
            elem->simplify();
        }
    }

    if (isType(BBType::Twoway)) {
        assert(m_successors.size() > 1);

        if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
            setType(BBType::Fall);
        }
        else {
            RTL *last = m_listOfRTLs->back();

            if (last->size() == 0) {
                setType(BBType::Fall);
            }
            else if (last->back()->isGoto()) {
                setType(BBType::Oneway);
            }
            else if (!last->back()->isBranch()) {
                setType(BBType::Fall);
            }
        }

        if (isType(BBType::Fall)) {
            // set out edges to be the second one
            LOG_VERBOSE("Turning TWOWAY into FALL: %1 %2", m_successors[0]->getLowAddr(), m_successors[1]->getLowAddr());

            BasicBlock *redundant = m_successors[0];
            m_successors[0] = m_successors[1];
            m_successors.resize(1);
            LOG_VERBOSE("Redundant edge to address %1", redundant->getLowAddr());
            LOG_VERBOSE("  inedges:");

            std::vector<BasicBlock *> rinedges = redundant->m_predecessors;
            redundant->m_predecessors.clear();

            for (BasicBlock *redundant_edge : rinedges) {
                if (redundant_edge != this) {
                    LOG_VERBOSE("    %1", redundant_edge->getLowAddr());
                    redundant->m_predecessors.push_back(redundant_edge);
                }
                else {
                    LOG_VERBOSE("    %1 (ignored)", redundant_edge->getLowAddr());
                }
            }

            // redundant->m_iNumInEdges = redundant->m_InEdges.size();
            LOG_VERBOSE("  after: %1", m_successors[0]->getLowAddr());
        }

        if (isType(BBType::Oneway)) {
            // set out edges to be the first one
            LOG_VERBOSE("Turning TWOWAY into ONEWAY: %1 %2", m_successors[0]->getLowAddr(), m_successors[1]->getLowAddr());

            BasicBlock *redundant = m_successors[BELSE];
            m_successors.resize(1);
            LOG_VERBOSE("redundant edge to address %1", redundant->getLowAddr());
            LOG_VERBOSE("  inedges:");

            std::vector<BasicBlock *> rinedges = redundant->m_predecessors;
            redundant->m_predecessors.clear();

            for (BasicBlock *redundant_edge : rinedges) {
                if (redundant_edge != this) {
                    LOG_VERBOSE("    %1", redundant_edge->getLowAddr());
                    redundant->m_predecessors.push_back(redundant_edge);
                }
                else {
                    LOG_VERBOSE("    %1 (ignored)", redundant_edge->getLowAddr());
                }
            }

            // redundant->m_iNumInEdges = redundant->m_InEdges.size();
            LOG_VERBOSE("  after: %1", m_successors[0]->getLowAddr());
        }
    }
}


bool BasicBlock::hasBackEdgeTo(const BasicBlock *dest) const
{
    return dest == this || dest->isAncestorOf(this);
}


bool BasicBlock::isPredecessorOf(const BasicBlock* bb) const
{
    return std::find(m_successors.begin(), m_successors.end(), bb) != m_successors.end();
}


bool BasicBlock::isSuccessorOf(const BasicBlock* bb) const
{
    return std::find(m_predecessors.begin(), m_predecessors.end(), bb) != m_predecessors.end();
}


void BasicBlock::setLoopStamps(int& time, std::vector<BasicBlock *>& order)
{
    // timestamp the current node with the current time
    // and set its traversed flag
    m_travType     = TravType::DFS_LNum;
    m_loopStamps[0] = time;

    // recurse on unvisited children and set inedges for all children
    for (BasicBlock *out : m_successors) {
        // set the in edge from this child to its parent (the current node)
        // (not done here, might be a problem)
        // outEdges[i]->inEdges.Add(this);

        // recurse on this child if it hasn't already been visited
        if (out->m_travType != TravType::DFS_LNum) {
            out->setLoopStamps(++time, order);
        }
    }

    // set the the second loopStamp value
    m_loopStamps[1] = ++time;

    // add this node to the ordering structure as well as recording its position within the ordering
    m_ord = (int)order.size();
    order.push_back(this);
}


void BasicBlock::setRevLoopStamps(int& time)
{
    // timestamp the current node with the current time and set its traversed flag
    m_travType        = TravType::DFS_RNum;
    m_revLoopStamps[0] = time;

    // recurse on the unvisited children in reverse order
    for (int i = (int)m_successors.size() - 1; i >= 0; i--) {
        // recurse on this child if it hasn't already been visited
        if (m_successors[i]->m_travType != TravType::DFS_RNum) {
            m_successors[i]->setRevLoopStamps(++time);
        }
    }

    m_revLoopStamps[1] = ++time;
}


void BasicBlock::setRevOrder(std::vector<BasicBlock *>& order)
{
    // Set this node as having been traversed during the post domimator DFS ordering traversal
    m_travType = TravType::DFS_PDom;

    // recurse on unvisited children
    for (BasicBlock *in : m_predecessors) {
        if (in->m_travType != TravType::DFS_PDom) {
            in->setRevOrder(order);
        }
    }

    // add this node to the ordering structure and record the post dom. order of this node as its index within this
    // ordering structure
    m_revOrd = (int)order.size();
    order.push_back(this);
}


void BasicBlock::setCaseHead(BasicBlock *head, BasicBlock *follow)
{
    assert(!m_caseHead);

    m_travType = TravType::DFS_Case;

    // don't tag this node if it is the case header under investigation
    if (this != head) {
        m_caseHead = head;
    }

    // if this is a nested case header, then it's member nodes
    // will already have been tagged so skip straight to its follow
    if (isType(BBType::Nway) && (this != head)) {
        if (m_condFollow && (m_condFollow->m_travType != TravType::DFS_Case) && (m_condFollow != follow)) {
            m_condFollow->setCaseHead(head, follow);
        }
    }
    else {
        // traverse each child of this node that:
        //   i) isn't on a back-edge,
        //  ii) hasn't already been traversed in a case tagging traversal and,
        // iii) isn't the follow node.
        for (BasicBlock *out : m_successors) {
            if (!hasBackEdgeTo(out) && (out->m_travType != TravType::DFS_Case) && (out != follow)) {
                out->setCaseHead(head, follow);
            }
        }
    }
}


void BasicBlock::setStructType(StructType structType)
{
    // if this is a conditional header, determine exactly which type of conditional header it is (i.e. switch, if-then,
    // if-then-else etc.)
    if (structType == StructType::Cond) {
        if (isType(BBType::Nway)) {
            m_conditionHeaderType = CondType::Case;
        }
        else if (m_successors[BELSE] == m_condFollow) {
            m_conditionHeaderType = CondType::IfThen;
        }
        else if (m_successors[BTHEN] == m_condFollow) {
            m_conditionHeaderType = CondType::IfElse;
        }
        else {
            m_conditionHeaderType = CondType::IfThenElse;
        }
    }

    m_structuringType = structType;
}


void BasicBlock::setUnstructType(UnstructType unstructType)
{
    assert((m_structuringType == StructType::Cond || m_structuringType == StructType::LoopCond) && m_conditionHeaderType != CondType::Case);
    m_unstructuredType = unstructType;
}


UnstructType BasicBlock::getUnstructType() const
{
    assert((m_structuringType == StructType::Cond || m_structuringType == StructType::LoopCond));
    // fails when cenerating code for switches; not sure if actually needed TODO
    // assert(m_conditionHeaderType != CondType::Case);

    return m_unstructuredType;
}


void BasicBlock::setLoopType(LoopType l)
{
    assert(m_structuringType == StructType::Loop || m_structuringType == StructType::LoopCond);
    m_loopHeaderType = l;

    // set the structured class (back to) just Loop if the loop type is PreTested OR it's PostTested and is a single
    // block loop
    if ((m_loopHeaderType == LoopType::PreTested) || ((m_loopHeaderType == LoopType::PostTested) && (this == m_latchNode))) {
        m_structuringType = StructType::Loop;
    }
}


LoopType BasicBlock::getLoopType() const
{
    assert(m_structuringType == StructType::Loop || m_structuringType == StructType::LoopCond);
    return m_loopHeaderType;
}


void BasicBlock::setCondType(CondType c)
{
    assert(m_structuringType == StructType::Cond || m_structuringType == StructType::LoopCond);
    m_conditionHeaderType = c;
}


CondType BasicBlock::getCondType() const
{
    assert(m_structuringType == StructType::Cond || m_structuringType == StructType::LoopCond);
    return m_conditionHeaderType;
}


bool BasicBlock::inLoop(BasicBlock *header, BasicBlock *latch)
{
    assert(header->m_latchNode == latch);
    assert(header == latch ||
           ((header->m_loopStamps[0] > latch->m_loopStamps[0] && latch->m_loopStamps[1] > header->m_loopStamps[1]) ||
            (header->m_loopStamps[0] < latch->m_loopStamps[0] && latch->m_loopStamps[1] < header->m_loopStamps[1])));
    // this node is in the loop if it is the latch node OR
    // this node is within the header and the latch is within this when using the forward loop stamps OR
    // this node is within the header and the latch is within this when using the reverse loop stamps
    return this == latch || (header->m_loopStamps[0] < m_loopStamps[0] && m_loopStamps[1] < header->m_loopStamps[1] &&
                             m_loopStamps[0] < latch->m_loopStamps[0] && latch->m_loopStamps[1] < m_loopStamps[1]) ||
           (header->m_revLoopStamps[0] < m_revLoopStamps[0] && m_revLoopStamps[1] < header->m_revLoopStamps[1] &&
            m_revLoopStamps[0] < latch->m_revLoopStamps[0] && latch->m_revLoopStamps[1] < m_revLoopStamps[1]);
}


void BasicBlock::prependStmt(Statement *s, UserProc *proc)
{
    assert(m_function == proc);
    // Check the first RTL (if any)
    assert(m_listOfRTLs);
    s->setBB(this);
    s->setProc(proc);

    if (!m_listOfRTLs->empty()) {
        RTL *rtl = m_listOfRTLs->front();

        if (rtl->getAddress().isZero()) {
            // Append to this RTL
            rtl->append(s);
            updateBBAddress();
            return;
        }
    }

    // Otherwise, prepend a new RTL
    std::list<Statement *> listStmt = { s };
    RTL *rtl = new RTL(Address::ZERO, &listStmt);
    m_listOfRTLs->push_front(rtl);

    updateBBAddress();
}


bool BasicBlock::searchAll(const Exp& search_for, std::list<SharedExp>& results)
{
    bool ch = false;

    for (RTL *rtl_it : *m_listOfRTLs) {
        for (Statement *e : *rtl_it) {
            SharedExp res; // searchAll can be used here too, would it change anything ?

            if (e->search(search_for, res)) {
                ch = true;
                results.push_back(res);
            }
        }
    }

    return ch;
}


bool BasicBlock::searchAndReplace(const Exp& pattern, SharedExp replacement)
{
    bool ch = false;

    for (RTL *rtl_it : *m_listOfRTLs) {
        for (auto& elem : *rtl_it) {
            ch |= (elem)->searchAndReplace(pattern, replacement);
        }
    }

    return ch;
}


void BasicBlock::updateBBAddress()
{
    if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
        // should not happen
        assert(false);
        return;
    }

    Address a = m_listOfRTLs->front()->getAddress();

    if (a.isZero() && (m_listOfRTLs->size() > 1)) {
        std::list<RTL *>::iterator it = m_listOfRTLs->begin();
        Address add2 = (*++it)->getAddress();

        // This is a bit of a hack for 286 programs, whose main actually starts at offset 0. A better solution would be
        // to change orphan BBs' addresses to Address::INVALID, but I suspect that this will cause many problems. MVE
        if (add2 < Address(0x10)) {
            // Assume that 0 is the real address
            m_lowAddr =  Address::ZERO;
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


bool BasicBlock::hasStatement(const Statement *stmt) const
{
    if (!m_listOfRTLs) {
        return false;
    }

    for (const RTL *rtl : *m_listOfRTLs) {
        for (const Statement *s : *rtl) {
            if (s == stmt) {
                return true;
            }
        }
    }

    return false;
}


bool BasicBlock::hasBackEdge()
{
    for (auto bb : m_successors) {
        if (hasBackEdgeTo(bb)) {
            return true;
        }
    }

    return false;
}

void BasicBlock::setLabelRequired(bool required)
{
    m_labelNeeded = required;
}
