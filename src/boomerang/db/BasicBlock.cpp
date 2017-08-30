/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file  basicblock.cpp
 * \brief Implementation of the BasicBlock class.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/
#include "BasicBlock.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/Global.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/Visitor.h"

#include "boomerang/codegen/ICodeGenerator.h"

#include "boomerang/type/Type.h"

#include "boomerang/util/Types.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <inttypes.h>


BasicBlock::BasicBlock(Function *parent)
    : m_inEdgesVisited(0) // From Doug's code
    , m_numForwardInEdges(-1)
    , m_traversed(TravType::Untraversed)
    , m_immPDom(nullptr)
    , m_loopHead(nullptr)
    , m_caseHead(nullptr)
    , m_condFollow(nullptr)
    , m_loopFollow(nullptr)
    , m_latchNode(nullptr)
    , m_structuringType(StructType::Seq)
    , m_unstructuredType(UnstructType::Structured)
    , m_overlappedRegProcessingDone(false) // others
{
    m_parent = parent;
}


BasicBlock::~BasicBlock()
{
    if (m_listOfRTLs) {
        // Delete the RTLs
        for (RTL *it : *m_listOfRTLs) {
            delete it;
        }

        // and delete the list
        delete m_listOfRTLs;
        m_listOfRTLs = nullptr;
    }
}


BasicBlock::BasicBlock(const BasicBlock& bb)
    : m_nodeType(bb.m_nodeType)
    , m_labelNum(bb.m_labelNum)
    , m_incomplete(bb.m_incomplete) // m_labelNeeded is initialized to false, not copied
    , m_jumpRequired(bb.m_jumpRequired)
    , m_inEdges(bb.m_inEdges)
    , m_outEdges(bb.m_outEdges)
    // From Doug's code
    , m_ord(bb.m_ord)
    , m_revOrd(bb.m_revOrd)
    , m_inEdgesVisited(bb.m_inEdgesVisited)
    , m_numForwardInEdges(bb.m_numForwardInEdges)
    , m_traversed(bb.m_traversed)
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
{
    setRTLs(bb.m_listOfRTLs);
    m_parent = bb.m_parent;
}


BasicBlock::BasicBlock(Function *parent, std::list<RTL *> *pRtls, BBType bbType)
    : m_nodeType(bbType)
    , m_incomplete(false)
    , m_inEdgesVisited(0) // From Doug's code
    , m_numForwardInEdges(-1)
    , m_traversed(TravType::Untraversed)
    , m_immPDom(nullptr)
    , m_loopHead(nullptr)
    , m_caseHead(nullptr)
    , m_condFollow(nullptr)
    , m_loopFollow(nullptr)
    , m_latchNode(nullptr)
    , m_structuringType(StructType::Seq)
    , m_unstructuredType(UnstructType::Structured)
    , m_overlappedRegProcessingDone(false) // Others
{
    m_parent = parent;

    // Set the RTLs
    setRTLs(pRtls);
}


bool BasicBlock::isCaseOption()
{
    if (m_caseHead) {
        for (unsigned int i = 0; i < m_caseHead->getNumOutEdges() - 1; i++) {
            if (m_caseHead->getOutEdge(i) == this) {
                return true;
            }
        }
    }

    return false;
}


bool BasicBlock::isTraversed()
{
    return m_traversedMarker;
}


void BasicBlock::setTraversed(bool bTraversed)
{
    m_traversedMarker = bTraversed;
}


void BasicBlock::setRTLs(std::list<RTL *> *rtls)
{
    // should we delete old ones here? breaks some things - trent
    m_listOfRTLs = rtls;

    // Used to set the link between the last instruction (a call) and this BB if this is a call BB
}


void BasicBlock::updateType(BBType bbType)
{
    m_nodeType = bbType;
}


void BasicBlock::setJumpRequired()
{
    m_jumpRequired = true;
}


bool BasicBlock::isJumpRequired()
{
    return m_jumpRequired;
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

    if (m_labelNum) {
        os << "L" << m_labelNum << ": ";
    }

    switch (getType())
    {
    case BBType::Oneway:   os << "Oneway BB"; break;
    case BBType::Twoway:   os << "Twoway BB"; break;
    case BBType::Nway:     os << "Nway BB";   break;
    case BBType::Call:     os << "Call BB";   break;
    case BBType::Ret:      os << "Ret BB";    break;
    case BBType::Fall:     os << "Fall BB";   break;
    case BBType::CompJump: os << "Computed jump BB"; break;
    case BBType::CompCall: os << "Computed call BB"; break;
    case BBType::Invalid:  os << "Invalid BB"; break;
    }

    os << "@" << this->getLowAddr() << ":\n";
    os << "  in edges: ";

    for (BasicBlock *bb : m_inEdges) {
        os << bb->getHiAddr() << "(" << bb->getLowAddr() << ") ";
    }

    os << "\n";
    os << "  out edges: ";

    for (BasicBlock *bb : m_outEdges) {
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

    if (m_jumpRequired) {
        if (html) {
            os << "<br>";
        }

        os << "Synthetic out edge(s) to ";

        // assert(TargetOutEdges == OutEdges.size());
        for (BasicBlock *outEdge : m_outEdges) {
            if (outEdge && outEdge->m_labelNum) {
                os << "L" << outEdge->m_labelNum << " ";
            }
        }

        os << '\n';
    }
}


void BasicBlock::printToLog()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    LOG_MSG(tgt);
}


bool BasicBlock::isBackEdge(size_t inEdge) const
{
    const BasicBlock *in = m_inEdges[inEdge];

    return this == in || (m_DFTfirst < in->m_DFTfirst && m_DFTlast > in->m_DFTlast);
}


Address BasicBlock::getLowAddr() const
{
    if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
        return Address::ZERO;
    }

    Address a = m_listOfRTLs->front()->getAddress();

    if (a.isZero() && (m_listOfRTLs->size() > 1)) {
        std::list<RTL *>::iterator it = m_listOfRTLs->begin();
        Address add2 = (*++it)->getAddress();

        // This is a bit of a hack for 286 programs, whose main actually starts at offset 0. A better solution would be
        // to change orphan BBs' addresses to Address::INVALID, but I suspect that this will cause many problems. MVE
        if (add2 < Address(0x10)) {
            // Assume that 0 is the real address
            return Address::ZERO;
        }

        return add2;
    }

    return a;
}


Address BasicBlock::getHiAddr() const
{
    assert(m_listOfRTLs != nullptr);
    return m_listOfRTLs->back()->getAddress();
}


std::list<RTL *> *BasicBlock::getRTLs()
{
    return m_listOfRTLs;
}


const std::list<RTL *> *BasicBlock::getRTLs() const
{
    return m_listOfRTLs;
}


RTL *BasicBlock::getRTLWithStatement(Statement *stmt)
{
    if (m_listOfRTLs == nullptr) {
        return nullptr;
    }

    for (RTL *rtl : *m_listOfRTLs) {
        for (Statement *it1 : *rtl) {
            if (it1 == stmt) {
                return rtl;
            }
        }
    }

    return nullptr;
}


std::vector<BasicBlock *>& BasicBlock::getInEdges()
{
    return m_inEdges;
}


const std::vector<BasicBlock *>& BasicBlock::getOutEdges()
{
    return m_outEdges;
}


void BasicBlock::setInEdge(size_t i, BasicBlock *pNewInEdge)
{
    m_inEdges[i] = pNewInEdge;
}


void BasicBlock::setOutEdge(size_t i, BasicBlock *pNewOutEdge)
{
    if (m_outEdges.empty()) {
        assert(i == 0);
        m_outEdges.push_back(pNewOutEdge);       // TODO: why is it allowed to set new edge in empty m_OutEdges array ?
    }
    else {
        assert(i < m_outEdges.size());
        m_outEdges[i] = pNewOutEdge;
    }
}


BasicBlock *BasicBlock::getOutEdge(size_t i)
{
    if (i < m_outEdges.size()) {
        return m_outEdges[i];
    }
    else {
        return nullptr;
    }
}


BasicBlock *BasicBlock::getCorrectOutEdge(Address a)
{
    for (BasicBlock *it : m_outEdges) {
        if (it->getLowAddr() == a) {
            return it;
        }
    }

    return nullptr;
}


void BasicBlock::addInEdge(BasicBlock *pNewInEdge)
{
    m_inEdges.push_back(pNewInEdge);
}


void BasicBlock::deleteInEdge(BasicBlock *edge)
{
    for (auto it = m_inEdges.begin(); it != m_inEdges.end(); it++) {
        if (*it == edge) {
            it = m_inEdges.erase(it);
            break;
        }
    }
}


void BasicBlock::deleteEdge(BasicBlock *edge)
{
    edge->deleteInEdge(this);

    for (auto it = m_outEdges.begin(); it != m_outEdges.end(); it++) {
        if (*it == edge) {
            m_outEdges.erase(it);
            break;
        }
    }
}


unsigned BasicBlock::getDFTOrder(int& first, int& last)
{
    first++;
    m_DFTfirst = first;

    unsigned numTraversed = 1;
    m_traversedMarker = true;

    for (BasicBlock *child : m_outEdges) {
        if (child->m_traversedMarker == false) {
            numTraversed = numTraversed + child->getDFTOrder(first, last);
        }
    }

    last++;
    m_DFTlast = last;

    return numTraversed;
}


unsigned BasicBlock::getRevDFTOrder(int& first, int& last)
{
    first++;
    m_DFTrevfirst = first;

    unsigned numTraversed = 1;
    m_traversedMarker = true;

    for (BasicBlock *parent : m_inEdges) {
        if (parent->m_traversedMarker == false) {
            numTraversed = numTraversed + parent->getRevDFTOrder(first, last);
        }
    }

    last++;
    m_DFTrevlast = last;

    return numTraversed;
}


bool BasicBlock::lessAddress(BasicBlock *bb1, BasicBlock *bb2)
{
    return bb1->getLowAddr() < bb2->getLowAddr();
}


bool BasicBlock::lessFirstDFT(BasicBlock *bb1, BasicBlock *bb2)
{
    return bb1->m_DFTfirst < bb2->m_DFTfirst;
}


bool BasicBlock::lessLastDFT(BasicBlock *bb1, BasicBlock *bb2)
{
    return bb1->m_DFTlast < bb2->m_DFTlast;
}


Address BasicBlock::getCallDest()
{
    Function* dest = getCallDestProc();
    return dest ? dest->getEntryAddress() : Address::INVALID;
}


Function *BasicBlock::getCallDestProc()
{
    if (!isType(BBType::Call) || m_listOfRTLs->empty()) {
        return nullptr;
    }

    RTL *lastRtl = m_listOfRTLs->back();

    for (auto it = lastRtl->rbegin(); it != lastRtl->rend(); it++) {
        if ((*it)->getKind() == STMT_CALL) {
            return ((CallStatement *)(*it))->getDestProc();
        }
    }

    return nullptr;
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


SharedExp BasicBlock::getCond()
{
    // the condition will be in the last rtl
    assert(m_listOfRTLs);
    RTL *last = m_listOfRTLs->back();
    // it should contain a BranchStatement
    BranchStatement *bs = dynamic_cast<BranchStatement*>(last->getHlStmt());

    if (bs && (bs->getKind() == STMT_BRANCH)) {
        return bs->getCondExpr();
    }

    throw LastStatementNotABranchError(last->getHlStmt());
}


SharedExp BasicBlock::getDest() noexcept(false)
{
    // The destianation will be in the last rtl
    assert(m_listOfRTLs);
    RTL *lastRtl = m_listOfRTLs->back();
    // It should contain a GotoStatement or derived class
    Statement   *lastStmt = lastRtl->getHlStmt();
    CaseStatement *cs       = dynamic_cast<CaseStatement *>(lastStmt);

    if (cs) {
        // Get the expression from the switch info
        SWITCH_INFO *si = cs->getSwitchInfo();

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

    throw LastStatementNotAGotoError(lastStmt);
}


void BasicBlock::setCond(SharedExp e) noexcept(false)
{
    // the condition will be in the last rtl
    assert(m_listOfRTLs);
    RTL *last = m_listOfRTLs->back();
    // it should contain a BranchStatement
    assert(!last->empty());

    for (auto it = last->rbegin(); it != last->rend(); it++) {
        if ((*it)->getKind() == STMT_BRANCH) {
            ((BranchStatement *)(*it))->setCondExpr(e);
            return;
        }
    }

    throw LastStatementNotABranchError(nullptr);
}


BasicBlock *BasicBlock::getLoopBody()
{
    assert(m_structType == SBBType::PreTestLoop || m_structType == SBBType::PostTestLoop || m_structType == SBBType::EndlessLoop);
    assert(m_outEdges.size() == 2);

    if (m_outEdges[0] != m_loopFollow) {
        return m_outEdges[0];
    }

    return m_outEdges[1];
}


bool BasicBlock::isAncestorOf(BasicBlock *other)
{
    return((m_loopStamps[0] < other->m_loopStamps[0] && m_loopStamps[1] > other->m_loopStamps[1]) ||
           (m_revLoopStamps[0] < other->m_revLoopStamps[0] && m_revLoopStamps[1] > other->m_revLoopStamps[1]));

    /*    return (m_DFTfirst < other->m_DFTfirst && m_DFTlast > other->m_DFTlast) ||
     *  (m_DFTrevlast < other->m_DFTrevlast &&
     *   m_DFTrevfirst > other->m_DFTrevfirst);*/
}


void BasicBlock::simplify()
{
    if (m_listOfRTLs) {
        for (RTL *elem : *m_listOfRTLs) {
            elem->simplify();
        }
    }

    if (isType(BBType::Twoway)) {
        assert(m_outEdges.size() > 1);

        if ((m_listOfRTLs == nullptr) || m_listOfRTLs->empty()) {
            updateType(BBType::Fall);
        }
        else {
            RTL *last = m_listOfRTLs->back();

            if (last->size() == 0) {
                updateType(BBType::Fall);
            }
            else if (last->back()->isGoto()) {
                updateType(BBType::Oneway);
            }
            else if (!last->back()->isBranch()) {
                updateType(BBType::Fall);
            }
        }

        if (isType(BBType::Fall)) {
            // set out edges to be the second one
            LOG_VERBOSE("Turning TWOWAY into FALL: %1 %2", m_outEdges[0]->getLowAddr(), m_outEdges[1]->getLowAddr());

            BasicBlock *redundant = m_outEdges[0];
            m_outEdges[0] = m_outEdges[1];
            m_outEdges.resize(1);
            LOG_VERBOSE("Redundant edge to address %1", redundant->getLowAddr());
            LOG_VERBOSE("  inedges:");

            std::vector<BasicBlock *> rinedges = redundant->m_inEdges;
            redundant->m_inEdges.clear();

            for (BasicBlock *redundant_edge : rinedges) {
                if (redundant_edge != this) {
                    LOG_VERBOSE("    %1", redundant_edge->getLowAddr());
                    redundant->m_inEdges.push_back(redundant_edge);
                }
                else {
                    LOG_VERBOSE("    %1 (ignored)", redundant_edge->getLowAddr());
                }
            }

            // redundant->m_iNumInEdges = redundant->m_InEdges.size();
            LOG_VERBOSE("  after: %1", m_outEdges[0]->getLowAddr());
        }

        if (isType(BBType::Oneway)) {
            // set out edges to be the first one
            LOG_VERBOSE("Turning TWOWAY into ONEWAY: %1 %2", m_outEdges[0]->getLowAddr(), m_outEdges[1]->getLowAddr());

            BasicBlock *redundant = m_outEdges[BELSE];
            m_outEdges.resize(1);
            LOG_VERBOSE("redundant edge to address %1", redundant->getLowAddr());
            LOG_VERBOSE("  inedges:");

            std::vector<BasicBlock *> rinedges = redundant->m_inEdges;
            redundant->m_inEdges.clear();

            for (BasicBlock *redundant_edge : rinedges) {
                if (redundant_edge != this) {
                    LOG_VERBOSE("    %1", redundant_edge->getLowAddr());
                    redundant->m_inEdges.push_back(redundant_edge);
                }
                else {
                    LOG_VERBOSE("    %1 (ignored)", redundant_edge->getLowAddr());
                }
            }

            // redundant->m_iNumInEdges = redundant->m_InEdges.size();
            LOG_VERBOSE("  after: %1", m_outEdges[0]->getLowAddr());
        }
    }
}


bool BasicBlock::hasBackEdgeTo(BasicBlock *dest)
{
    return dest == this || dest->isAncestorOf(this);
}


bool BasicBlock::allParentsGenerated()
{
    for (BasicBlock *in : m_inEdges) {
        if (!in->hasBackEdgeTo(this) && (in->m_traversed != TravType::DFS_Codegen)) {
            return false;
        }
    }

    return true;
}


Function *BasicBlock::getDestProc()
{
    // The last Statement of the last RTL should be a CallStatement
    CallStatement *call = (CallStatement *)(m_listOfRTLs->back()->getHlStmt());

    assert(call->getKind() == STMT_CALL);
    Function *proc = call->getDestProc();

    if (proc == nullptr) {
        LOG_FATAL("Indirect calls not handled yet.");
        assert(false);
    }

    return proc;
}


void BasicBlock::setLoopStamps(int& time, std::vector<BasicBlock *>& order)
{
    // timestamp the current node with the current time and set its traversed
    // flag
    m_traversed     = TravType::DFS_LNum;
    m_loopStamps[0] = time;

    // recurse on unvisited children and set inedges for all children
    for (BasicBlock *out : m_outEdges) {
        // set the in edge from this child to its parent (the current node)
        // (not done here, might be a problem)
        // outEdges[i]->inEdges.Add(this);

        // recurse on this child if it hasn't already been visited
        if (out->m_traversed != TravType::DFS_LNum) {
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
    m_traversed        = TravType::DFS_RNum;
    m_revLoopStamps[0] = time;

    // recurse on the unvisited children in reverse order
    for (int i = (int)m_outEdges.size() - 1; i >= 0; i--) {
        // recurse on this child if it hasn't already been visited
        if (m_outEdges[i]->m_traversed != TravType::DFS_RNum) {
            m_outEdges[i]->setRevLoopStamps(++time);
        }
    }

    // set the the second loopStamp value
    m_revLoopStamps[1] = ++time;
}


void BasicBlock::setRevOrder(std::vector<BasicBlock *>& order)
{
    // Set this node as having been traversed during the post domimator DFS ordering traversal
    m_traversed = TravType::DFS_PDom;

    // recurse on unvisited children
    for (BasicBlock *in : m_inEdges) {
        if (in->m_traversed != TravType::DFS_PDom) {
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

    m_traversed = TravType::DFS_Case;

    // don't tag this node if it is the case header under investigation
    if (this != head) {
        m_caseHead = head;
    }

    // if this is a nested case header, then it's member nodes will already have been tagged so skip straight to its
    // follow
    if (isType(BBType::Nway) && (this != head)) {
        if (m_condFollow && (m_condFollow->m_traversed != TravType::DFS_Case) && (m_condFollow != follow)) {
            m_condFollow->setCaseHead(head, follow);
        }
    }
    else {
        // traverse each child of this node that:
        //   i) isn't on a back-edge,
        //  ii) hasn't already been traversed in a case tagging traversal and,
        // iii) isn't the follow node.
        for (BasicBlock *out : m_outEdges) {
            if (!hasBackEdgeTo(out) && (out->m_traversed != TravType::DFS_Case) && (out != follow)) {
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
        else if (m_outEdges[BELSE] == m_condFollow) {
            m_conditionHeaderType = CondType::IfThen;
        }
        else if (m_outEdges[BTHEN] == m_condFollow) {
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
    assert((m_structuringType == StructType::Cond || m_structuringType == StructType::LoopCond) && m_conditionHeaderType != CondType::Case);
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
    assert(m_parent == proc);
    // Check the first RTL (if any)
    assert(m_listOfRTLs);
    s->setBB(this);
    s->setProc(proc);

    if (!m_listOfRTLs->empty()) {
        RTL *rtl = m_listOfRTLs->front();

        if (rtl->getAddress().isZero()) {
            // Append to this RTL
            rtl->appendStmt(s);
            return;
        }
    }

    // Otherwise, prepend a new RTL
    std::list<Statement *> listStmt = { s };
    RTL *rtl = new RTL(Address::ZERO, &listStmt);
    m_listOfRTLs->push_front(rtl);
}


////////////////////////////////////////////////////

// Check for overlap of liveness between the currently live locations (liveLocs) and the set of locations in ls
// Also check for type conflicts if DFA_TYPE_ANALYSIS
// This is a helper function that is not directly declared in the BasicBlock class
void checkForOverlap(LocationSet& liveLocs, LocationSet& ls, ConnectionGraph& ig, UserProc *)
{
    // For each location to be considered
    for (SharedExp u : ls) {
        if (!u->isSubscript()) {
            continue; // Only interested in subscripted vars
        }

        auto r = std::static_pointer_cast<RefExp>(u);
        // Interference if we can find a live variable which differs only in the reference
        SharedExp dr;

        if (liveLocs.findDifferentRef(r, dr)) {
            assert(dr->access<RefExp>()->getDef() != nullptr);
            assert(u->access<RefExp>()->getDef() != nullptr);
            // We have an interference between r and dr. Record it
            ig.connect(r, dr);

            if (DEBUG_LIVENESS) {
                LOG_VERBOSE("Interference of %1 with %2", dr, r);
            }
        }

        // Add the uses one at a time. Note: don't use makeUnion, because then we don't discover interferences
        // from the same statement, e.g.  blah := r24{2} + r24{3}
        liveLocs.insert(u);
    }
}


bool BasicBlock::calcLiveness(ConnectionGraph& ig, UserProc *myProc)
{
    // Start with the liveness at the bottom of the BB
    LocationSet liveLocs, phiLocs;

    getLiveOut(liveLocs, phiLocs);
    // Do the livensses that result from phi statements at successors first.
    // FIXME: document why this is necessary
    checkForOverlap(liveLocs, phiLocs, ig, myProc);
    // For each RTL in this BB
    std::list<RTL *>::reverse_iterator rit;

    if (m_listOfRTLs) { // this can be nullptr
        for (rit = m_listOfRTLs->rbegin(); rit != m_listOfRTLs->rend(); ++rit) {
            std::list<Statement *>::reverse_iterator sit;

            // For each statement this RTL
            for (sit = (*rit)->rbegin(); sit != (*rit)->rend(); ++sit) {
                Statement *s = *sit;
                LocationSet defs;
                s->getDefinitions(defs);
                // The definitions don't have refs yet
                defs.addSubscript(s /* , myProc->getCFG() */);

                // Definitions kill uses. Now we are moving to the "top" of statement s
                liveLocs.makeDiff(defs);

                // Phi functions are a special case. The operands of phi functions are uses, but they don't interfere
                // with each other (since they come via different BBs). However, we don't want to put these uses into
                // liveLocs, because then the livenesses will flow to all predecessors. Only the appropriate livenesses
                // from the appropriate phi parameter should flow to the predecessor. This is done in getLiveOut()
                if (s->isPhi()) {
                    continue;
                }

                // Check for livenesses that overlap
                LocationSet uses;
                s->addUsedLocs(uses);
                checkForOverlap(liveLocs, uses, ig, myProc);

                if (DEBUG_LIVENESS) {
                    LOG_MSG(" ## liveness: at top of %1, liveLocs is %2", s, liveLocs.prints());
                }
            }
        }
    }

    // liveIn is what we calculated last time
    if (!(liveLocs == m_liveIn)) {
        m_liveIn = liveLocs;
        return true; // A change
    }

    // No change
    return false;
}


void BasicBlock::getLiveOut(LocationSet& liveout, LocationSet& phiLocs)
{
    Cfg *cfg(((UserProc *)m_parent)->getCFG());

    liveout.clear();

    for (BasicBlock *currBB : m_outEdges) {
        // First add the non-phi liveness
        liveout.makeUnion(currBB->m_liveIn); // add successor liveIn to this liveout set.

        // The first RTL will have the phi functions, if any
        if ((currBB->m_listOfRTLs == nullptr) || (currBB->m_listOfRTLs->size() == 0)) {
            continue;
        }

        RTL *phiRtl = currBB->m_listOfRTLs->front();
        assert(phiRtl);

        for (Statement *st : *phiRtl) {
            // Only interested in phi assignments. Note that it is possible that some phi assignments have been
            // converted to ordinary assignments. So the below is a continue, not a break.
            if (!st->isPhi()) {
                continue;
            }

            PhiAssign *pa = (PhiAssign *)st;

            for (std::pair<const BasicBlock *, PhiInfo> v : pa->getDefs()) {
                if (!cfg->existsBB(v.first)) {
                    LOG_WARN("Someone removed BB that defined the PHI! Need to update PhiAssign defs");
                }
            }

            // Get the jth operand to the phi function; it has a use from BB *this
            // assert(j>=0);
            Statement *def = pa->getStmtAt(this);

            if (!def) {
                std::deque<BasicBlock *> to_visit(m_inEdges.begin(), m_inEdges.end());
                std::set<BasicBlock *> tried { this };

                // TODO: this looks like a hack ?  but sometimes PhiAssign has value which is defined in parent of
                // 'this'
                //  BB1 1  - defines r20
                //  BB2 33 - transfers control to BB3
                //  BB3 40 - r10 = phi { 1 }
                while (!to_visit.empty()) {
                    BasicBlock *pbb = to_visit.back();

                    if (tried.find(pbb) != tried.end()) {
                        to_visit.pop_back();
                        continue;
                    }

                    def = pa->getStmtAt(pbb);

                    if (def) {
                        break;
                    }

                    tried.insert(pbb);
                    to_visit.pop_back();

                    for (BasicBlock *bb : pbb->m_inEdges) {
                        if (tried.find(bb) != tried.end()) { // already tried
                            continue;
                        }

                        to_visit.push_back(bb);
                    }
                }
            }

            SharedExp r = RefExp::get(pa->getLeft()->clone(), def);
            assert(def);
            liveout.insert(r);
            phiLocs.insert(r);

            if (DEBUG_LIVENESS) {
                LOG_MSG(" ## Liveness: adding %1 due due to ref to phi %2 in BB at %3",
                        r, st, getLowAddr());
            }
        }
    }
}


//    //    //    //    //    //    //    //    //    //    //    //    //
//                                                //
//         Indirect jump and call analyses        //
//                                                //
//    //    //    //    //    //    //    //    //    //    //    //    //

// Switch High Level patterns

// With array processing, we get a new form, call it form 'a' (don't confuse with form 'A'):
// Pattern: <base>{}[<index>]{} where <index> could be <var> - <Kmin>
// TODO: use initializer lists
static SharedExp forma =
    RefExp::get(Binary::get(opArrayIndex,
                            RefExp::get(Terminal::get(opWild), (Statement *)-1),
                            Terminal::get(opWild)),
                (Statement *)-1);

// Pattern: m[<expr> * 4 + T ]
static SharedExp formA = Location::memOf(
    Binary::get(opPlus,
                Binary::get(opMult,
                            Terminal::get(opWild),
                            Const::get(4)),
                Terminal::get(opWildIntConst)));

// With array processing, we get a new form, call it form 'o' (don't confuse with form 'O'):
// Pattern: <base>{}[<index>]{} where <index> could be <var> - <Kmin>
// NOT COMPLETED YET!
static SharedExp formo =
    RefExp::get(Binary::get(opArrayIndex,
                            RefExp::get(Terminal::get(opWild), (Statement *)-1),
                            Terminal::get(opWild)),
                (Statement *)-1);

// Pattern: m[<expr> * 4 + T ] + T
static SharedExp formO =
    Binary::get(opPlus,
                Location::memOf(Binary::get(opPlus,
                                            Binary::get(opMult,
                                                        Terminal::get(opWild),
                                                        Const::get(4)),
                                            Terminal::get(opWildIntConst))),
                Terminal::get(opWildIntConst));

// Pattern: %pc + m[%pc     + (<expr> * 4) + k]
// where k is a small constant, typically 28 or 20
static SharedExp formR =
    Binary::get(opPlus,
                Terminal::get(opPC),
                Location::memOf(Binary::get(opPlus,
                                            Terminal::get(opPC),
                                            Binary::get(opPlus,
                                                        Binary::get(opMult,
                                                                    Terminal::get(opWild),
                                                                    Const::get(4)),
                                                        Const::get(opWildIntConst)))));

// Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
// where k is a smallish constant, e.g. 288 (/usr/bin/vi 2.6, 0c4233c).
static SharedExp formr =
    Binary::get(opPlus,
                Terminal::get(opPC),
                Location::memOf(Binary::get(opPlus,
                                            Terminal::get(opPC),
                                            Binary::get(opMinus,
                                                        Binary::get(opMult,
                                                                    Terminal::get(opWild), Const::get(4)),
                                                        Terminal::get(opWildIntConst)))));


struct SwitchForm
{
    SharedConstExp pattern;
    char           type;
};

SwitchForm hlForms[] =
{
    { forma, 'a' },
    { formA, 'A' },
    { formo, 'o' },
    { formO, 'O' },
    { formR, 'R' },
    { formr, 'r' }
};

// Vcall high level patterns
// Pattern 0: global<wild>[0]
static SharedExp vfc_funcptr =
    Binary::get(opArrayIndex,
                Location::get(opGlobal, Terminal::get(opWildStrConst), nullptr),
                Const::get(0));

// Pattern 1: m[ m[ <expr> + K1 ] + K2 ]
// K1 is vtable offset, K2 is virtual function offset (could come from m[A2], if A2 is in read-only memory
static SharedExp vfc_both = Location::memOf(
    Binary::get(opPlus,
                Location::memOf(Binary::get(opPlus,
                                            Terminal::get(opWild),
                                            Terminal::get(opWildIntConst))),
                Terminal::get(opWildIntConst)));

// Pattern 2: m[ m[ <expr> ] + K2]
static SharedExp vfc_vto =
    Location::memOf(Binary::get(opPlus,
                                Location::memOf(Terminal::get(opWild)),
                                Terminal::get(opWildIntConst)));

// Pattern 3: m[ m[ <expr> + K1] ]
static SharedExp vfc_vfo =
    Location::memOf(Location::memOf(Binary::get(opPlus,
                                                Terminal::get(opWild),
                                                Terminal::get(opWildIntConst))));

// Pattern 4: m[ m[ <expr> ] ]
static SharedExp vfc_none = Location::memOf(Location::memOf(Terminal::get(opWild)));

static SharedExp hlVfc[] = { vfc_funcptr, vfc_both, vfc_vto, vfc_vfo, vfc_none };

void findSwParams(char form, SharedExp e, SharedExp& expr, Address& T)
{
    switch (form)
    {
    case 'a':
        {
            // Pattern: <base>{}[<index>]{}
            e = e->getSubExp1();
            SharedExp base = e->getSubExp1();

            if (base->isSubscript()) {
                base = base->getSubExp1();
            }

            auto     con     = base->access<Const, 1>();
            QString  gloName = con->getStr();
            UserProc *p      = std::static_pointer_cast<Location>(base)->getProc();
            Prog     *prog   = p->getProg();
            T    = prog->getGlobalAddr(gloName);
            expr = e->getSubExp2();
            break;
        }

    case 'A':
        {
            // Pattern: m[<expr> * 4 + T ]
            if (e->isSubscript()) {
                e = e->getSubExp1();
            }

            // b will be (<expr> * 4) + T
            SharedExp b = e->getSubExp1();
            T    = b->access<Const, 2>()->getAddr();
            b    = b->getSubExp1(); // b is now <expr> * 4
            expr = b->getSubExp1();
            break;
        }

    case 'O':
        {       // Form O
            // Pattern: m[<expr> * 4 + T ] + T
            T = e->access<Const, 2>()->getAddr();
            // l = m[<expr> * 4 + T ]:
            SharedExp l = e->getSubExp1();

            if (l->isSubscript()) {
                l = l->getSubExp1();
            }

            // b = <expr> * 4 + T:
            SharedExp b = l->getSubExp1();
            // b = <expr> * 4:
            b = b->getSubExp1();
            // expr = <expr>:
            expr = b->getSubExp1();
            break;
        }

    case 'R':
        {
            // Pattern: %pc + m[%pc     + (<expr> * 4) + k]
            T = Address::ZERO; // ?
            // l = m[%pc  + (<expr> * 4) + k]:
            SharedExp l = e->getSubExp2();

            if (l->isSubscript()) {
                l = l->getSubExp1();
            }

            // b = %pc    + (<expr> * 4) + k:
            SharedExp b = l->getSubExp1();
            // b = (<expr> * 4) + k:
            b = b->getSubExp2();
            // b = <expr> * 4:
            b = b->getSubExp1();
            // expr = <expr>:
            expr = b->getSubExp1();
            break;
        }

    case 'r':
        {
            // Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
            T = Address::ZERO; // ?
            // b = %pc + m[%pc + ((<expr> * 4) - k)]:
            SharedExp b = e->getSubExp1();
            // l = m[%pc + ((<expr> * 4) - k)]:
            SharedExp l = b->getSubExp2();

            if (l->isSubscript()) {
                l = l->getSubExp1();
            }

            // b = %pc + ((<expr> * 4) - k)
            b = l->getSubExp1();
            // b = ((<expr> * 4) - k):
            b = b->getSubExp2();
            // b = <expr> * 4:
            b = b->getSubExp1();
            // expr = <expr>
            expr = b->getSubExp1();
            break;
        }

    default:
        expr = nullptr;
        T    = Address::INVALID;
    }

    // normalize address to native
    T = T.native();
}


int BasicBlock::findNumCases()
{
    // should actually search from the statement to i
    for (BasicBlock *in : m_inEdges) {          // For each in-edge
        if (!in->isType(BBType::Twoway)) {      // look for a two-way BB
            continue;                           // Ignore all others
        }

        assert(in->m_listOfRTLs && in->m_listOfRTLs->size());
        RTL *lastRtl = in->m_listOfRTLs->back();
        assert(!lastRtl->empty());
        BranchStatement *lastStmt = (BranchStatement *)lastRtl->back();
        SharedExp       pCond     = lastStmt->getCondExpr();

        if (pCond->getArity() != 2) {
            continue;
        }

        SharedExp rhs = pCond->getSubExp2();

        if (!rhs->isIntConst()) {
            continue;
        }

        int  k  = std::static_pointer_cast<Const>(rhs)->getInt();
        OPER op = pCond->getOper();

        if ((op == opGtr) || (op == opGtrUns)) {
            return k + 1;
        }

        if ((op == opGtrEq) || (op == opGtrEqUns)) {
            return k;
        }

        if ((op == opLess) || (op == opLessUns)) {
            return k;
        }

        if ((op == opLessEq) || (op == opLessEqUns)) {
            return k + 1;
        }
    }

    LOG_WARN("Could not find number of cases for n-way at address %1", getLowAddr());
    return 3; // Bald faced guess if all else fails
}


/// Find all the possible constant values that the location defined by s could be assigned with
static void findConstantValues(const Statement *s, std::list<int>& dests)
{
    if (s == nullptr) {
        return;
    }

    if (s->isPhi()) {
        // For each definition, recurse
        for (const auto& it : *((PhiAssign *)s)) {
            findConstantValues(it.second.def(), dests);
        }
    }
    else if (s->isAssign()) {
        SharedExp rhs = ((Assign *)s)->getRight();

        if (rhs->isIntConst()) {
            dests.push_back(rhs->access<Const>()->getInt());
        }
    }
}


bool BasicBlock::decodeIndirectJmp(UserProc *proc)
{
#if CHECK_REAL_PHI_LOOPS
    rtlit rit;
    StatementList::iterator sit;
    Statement               *s = getFirstStmt(rit, sit);

    for (s = getFirstStmt(rit, sit); s; s = getNextStmt(rit, sit)) {
        if (!s->isPhi()) {
            continue;
        }

        Statement      *originalPhi = s;
        InstructionSet workSet, seenSet;
        workSet.insert(s);
        seenSet.insert(s);

        do {
            PhiAssign *pi = (PhiAssign *)*workSet.begin();
            workSet.remove(pi);
            PhiAssign::Definitions::iterator it;

            for (it = pi->begin(); it != pi->end(); it++) {
                if (it->def == nullptr) {
                    continue;
                }

                if (!it->def->isPhi()) {
                    continue;
                }

                if (seenSet.exists(it->def)) {
                    LOG_VERBOSE("Real phi loop involving statements %1 and %2",
                                originalPhi->getNumber(), pi->getNumber());
                    break;
                }
                else {
                    workSet.insert(it->def);
                    seenSet.insert(it->def);
                }
            }
        } while (workSet.size());
    }
#endif

    if (isType(BBType::CompJump)) {
        assert(m_listOfRTLs->size() > 0);
        RTL *lastRtl = m_listOfRTLs->back();

        if (DEBUG_SWITCH) {
            LOG_MSG("decodeIndirectJmp: %1", lastRtl->prints());
        }

        assert(!lastRtl->empty());
        CaseStatement *lastStmt = (CaseStatement *)lastRtl->back();
        // Note: some programs might not have the case expression propagated to, because of the -l switch (?)
        // We used to use ordinary propagation here to get the memory expression, but now it refuses to propagate memofs
        // because of the alias safety issue. Eventually, we should use an alias-safe incremental propagation, but for
        // now we'll assume no alias problems and force the propagation
        bool convert; // FIXME: uninitialized value passed to propagateTo
        lastStmt->propagateTo(convert, nullptr, nullptr, true /* force */);
        SharedExp e = lastStmt->getDest();

        char form = 0;

        for (auto& val : hlForms) {
            if (*e *= *val.pattern) { // *= compare ignores subscripts
                form = val.type;

                if (DEBUG_SWITCH) {
                    LOG_MSG("Indirect jump matches form %1", form);
                }

                break;
            }
        }

        if (form) {
            SWITCH_INFO *swi = new SWITCH_INFO;
            swi->chForm = form;
                     Address   T;
            SharedExp expr;
            findSwParams(form, e, expr, T);

            if (expr) {
                swi->uTable    = T;
                swi->iNumTable = findNumCases();

                // TMN: Added actual control of the array members, to possibly truncate what findNumCases()
                // thinks is the number of cases, when finding the first array element not pointing to code.
                if (form == 'A') {
                    Prog *prog = proc->getProg();

                    for (int iPtr = 0; iPtr < swi->iNumTable; ++iPtr) {
                                          Address uSwitch = Address(prog->readNative4(swi->uTable + iPtr * 4));

                        if ((uSwitch >= prog->getLimitTextHigh()) || (uSwitch < prog->getLimitTextLow())) {
                            if (DEBUG_SWITCH) {
                                LOG_MSG("Truncating type A indirect jump array to %1 entries "
                                    "due to finding an array entry pointing outside valid code; %2 isn't in %3..%4",
                                    iPtr, uSwitch, prog->getLimitTextLow(), prog->getLimitTextHigh());
                            }

                            // Found an array that isn't a pointer-to-code. Assume array has ended.
                            swi->iNumTable = iPtr;
                            break;
                        }
                    }
                }

                if (swi->iNumTable <= 0) {
                    LOG_MSG("Switch analysis failure at address %1", this->getLowAddr());
                    return false;
                }

                // TODO: missing form = 'R' iOffset is not being set
                swi->iUpper = swi->iNumTable - 1;
                swi->iLower = 0;

                if ((expr->getOper() == opMinus) && expr->getSubExp2()->isIntConst()) {
                    swi->iLower  = std::static_pointer_cast<Const>(expr->getSubExp2())->getInt();
                    swi->iUpper += swi->iLower;
                    expr         = expr->getSubExp1();
                }

                swi->pSwitchVar = expr;
                lastStmt->setDest((SharedExp)nullptr);
                lastStmt->setSwitchInfo(swi);
                return swi->iNumTable != 0;
            }
        }
        else {
            // Did not match a switch pattern. Perhaps it is a Fortran style goto with constants at the leaves of the
            // phi tree. Basically, a location with a reference, e.g. m[r28{-} - 16]{87}
            if (e->isSubscript()) {
                SharedExp sub = e->getSubExp1();

                if (sub->isLocation()) {
                    // Yes, we have <location>{ref}. Follow the tree and store the constant values that <location>
                    // could be assigned to in dests
                    std::list<int> dests;
                    findConstantValues(std::static_pointer_cast<RefExp>(e)->getDef(), dests);
                    // The switch info wants an array of native addresses
                    size_t num_dests = dests.size();

                    if (num_dests) {
                        int *destArray = new int[num_dests];
                        std::copy(dests.begin(), dests.end(), destArray);
                        SWITCH_INFO *swi = new SWITCH_INFO;
                        swi->chForm     = 'F';                          // The "Fortran" form
                        swi->pSwitchVar = e;
                        swi->uTable     = Address(HostAddress(destArray).value()); // WARN: HACK HACK HACK Abuse the uTable member as a pointer
                        swi->iNumTable  = (int)num_dests;
                        swi->iLower     = 1;                            // Not used, except to compute
                        swi->iUpper     = (int)num_dests;               // the number of options
                        lastStmt->setDest((SharedExp)nullptr);
                        lastStmt->setSwitchInfo(swi);
                        return true;
                    }
                }
            }
        }

        return false;
    }
    else if (isType(BBType::CompCall)) {
        assert(m_listOfRTLs->size() > 0);
        RTL *lastRtl = m_listOfRTLs->back();

        if (DEBUG_SWITCH) {
            LOG_MSG("decodeIndirectJmp: COMPCALL:");
            LOG_MSG("%1", lastRtl->prints());
        }

        assert(!lastRtl->empty());
        CallStatement *lastStmt = (CallStatement *)lastRtl->back();
        SharedExp     e         = lastStmt->getDest();
        // Indirect calls may sometimes not be propagated to, because of limited propagation (-l switch).
        // Propagate to e, but only keep the changes if the expression matches (don't want excessive propagation to
        // a genuine function pointer expression, even though it's hard to imagine).
        e = e->propagateAll();
        // We also want to replace any m[K]{-} with the actual constant from the (presumably) read-only data section
        ConstGlobalConverter cgc(proc->getProg());
        e = e->accept(&cgc);
        // Simplify the result, e.g. for m[m[(r24{16} + m[0x8048d74]{-}) + 12]{-}]{-} get
        // m[m[(r24{16} + 20) + 12]{-}]{-}, want m[m[r24{16} + 32]{-}]{-}. Note also that making the
        // ConstGlobalConverter a simplifying expression modifier won't work in this case, since the simplifying
        // converter will only simplify the direct parent of the changed expression (which is r24{16} + 20).
        e = e->simplify();

        if (DEBUG_SWITCH) {
            LOG_MSG("decodeIndirect: propagated and const global converted call expression is %1", e);
        }

        int  n          = sizeof(hlVfc) / sizeof(SharedExp);
        bool recognised = false;
        int  i;

        for (i = 0; i < n; i++) {
            if (*e *= *hlVfc[i]) { // *= compare ignores subscripts
                recognised = true;

                if (DEBUG_SWITCH) {
                    LOG_MSG("Indirect call matches form %1", i);
                }

                break;
            }
        }

        if (!recognised) {
            return false;
        }

        lastStmt->setDest(e); // Keep the changes to the indirect call expression
        int       K1, K2;
        SharedExp vtExp, t1;
        Prog      *prog = proc->getProg();

        switch (i)
        {
        case 0:
            {
                // This is basically an indirection on a global function pointer.  If it is initialised, we have a
                // decodable entry point.  Note: it could also be a library function (e.g. Windows)
                // Pattern 0: global<name>{0}[0]{0}
                K2 = 0;

                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e  = e->getSubExp1(); // e is global<name>{0}[0]
                t1 = e->getSubExp2();
                auto t1_const = std::static_pointer_cast<Const>(t1);

                if (e->isArrayIndex() && (t1->isIntConst()) && (t1_const->getInt() == 0)) {
                    e = e->getSubExp1(); // e is global<name>{0}
                }

                if (e->isSubscript()) {
                    e = e->getSubExp1();                                                        // e is global<name>
                }

                std::shared_ptr<Const> con  = std::static_pointer_cast<Const>(e->getSubExp1()); // e is <name>
                Global                 *global = prog->getGlobal(con->getStr());
                assert(global);
                // Set the type to pointer to function, if not already
                SharedType ty = global->getType();

                if (!ty->isPointer() && !std::static_pointer_cast<PointerType>(ty)->getPointsTo()->isFunc()) {
                    global->setType(PointerType::get(FuncType::get()));
                }

                Address addr = global->getAddress();
                // FIXME: not sure how to find K1 from here. I think we need to find the earliest(?) entry in the data
                // map that overlaps with addr
                // For now, let K1 = 0:
                K1    = 0;
                vtExp = Const::get(addr);
                break;
            }

        case 1:
            {
                // Example pattern: e = m[m[r27{25} + 8]{-} + 8]{-}
                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e = e->getSubExp1();             // e = m[r27{25} + 8]{-} + 8
                SharedExp rhs = e->getSubExp2(); // rhs = 8
                K2 = std::static_pointer_cast<Const>(rhs)->getInt();
                SharedExp lhs = e->getSubExp1(); // lhs = m[r27{25} + 8]{-}

                if (lhs->isSubscript()) {
                    lhs = lhs->getSubExp1(); // lhs = m[r27{25} + 8]
                }

                vtExp = lhs;
                lhs   = lhs->getSubExp1(); // lhs =   r27{25} + 8
                SharedExp CK1 = lhs->getSubExp2();
                K1 = std::static_pointer_cast<Const>(CK1)->getInt();
                break;
            }

        case 2:
            {
                // Example pattern: e = m[m[r27{25}]{-} + 8]{-}
                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e = e->getSubExp1();             // e = m[r27{25}]{-} + 8
                SharedExp rhs = e->getSubExp2(); // rhs = 8
                K2 = std::static_pointer_cast<Const>(rhs)->getInt();
                SharedExp lhs = e->getSubExp1(); // lhs = m[r27{25}]{-}

                if (lhs->isSubscript()) {
                    lhs = lhs->getSubExp1(); // lhs = m[r27{25}]
                }

                vtExp = lhs;
                K1    = 0;
                break;
            }

        case 3:
            {
                // Example pattern: e = m[m[r27{25} + 8]{-}]{-}
                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e  = e->getSubExp1(); // e = m[r27{25} + 8]{-}
                K2 = 0;

                if (e->isSubscript()) {
                    e = e->getSubExp1(); // e = m[r27{25} + 8]
                }

                vtExp = e;
                SharedExp lhs = e->getSubExp1(); // lhs =   r27{25} + 8
                // Exp* object = ((Binary*)lhs)->getSubExp1();
                SharedExp CK1 = lhs->getSubExp2();
                K1 = std::static_pointer_cast<Const>(CK1)->getInt();
                break;
            }

        case 4:

            // Example pattern: e = m[m[r27{25}]{-}]{-}
            if (e->isSubscript()) {
                e = e->getSubExp1();
            }

            e  = e->getSubExp1(); // e = m[r27{25}]{-}
            K2 = 0;

            if (e->isSubscript()) {
                e = e->getSubExp1(); // e = m[r27{25}]
            }

            vtExp = e;
            K1    = 0;
            // Exp* object = ((Unary*)e)->getSubExp1();
            break;

        default:
            K1    = K2 = -1; // Suppress warnings
            vtExp = nullptr;
        }

        if (DEBUG_SWITCH) {
            LOG_MSG("Form %1: from statement %2 get e = %3, K1 = %4, K2 = %5, vtExp = %6",
                    i, lastStmt->getNumber(), lastStmt->getDest(), K1, K2, vtExp);
        }

        // The vt expression might not be a constant yet, because of expressions not fully propagated, or because of
        // m[K] in the expression (fixed with the ConstGlobalConverter).  If so, look it up in the defCollector in the
        // call
        vtExp = lastStmt->findDefFor(vtExp);

        if (vtExp && DEBUG_SWITCH) {
            LOG_MSG("VT expression boils down to this: %1", vtExp);
        }

        // Danger. For now, only do if -ic given
        bool decodeThru = SETTING(decodeThruIndCall);

        if (decodeThru && vtExp && vtExp->isIntConst()) {
            Address addr  = std::static_pointer_cast<Const>(vtExp)->getAddr();
            Address pfunc = Address(prog->readNative4(addr));

            if (prog->findProc(pfunc) == nullptr) {
                // A new, undecoded procedure
                if (SETTING(noDecodeChildren)) {
                    return false;
                }

                prog->decodeEntryPoint(pfunc);
                // Since this was not decoded, this is a significant change, and we want to redecode the current
                // function now that the callee has been decoded
                return true;
            }
        }
    }

    return false;
}


void BasicBlock::processSwitch(UserProc *proc)
{
    RTL           *last(m_listOfRTLs->back());
    CaseStatement *lastStmt((CaseStatement *)last->getHlStmt());
    SWITCH_INFO   *si(lastStmt->getSwitchInfo());

    SETTING(debugSwitch) = true;

    if (SETTING(debugSwitch)) {
        LOG_MSG("Processing switch statement type %1 with table at %2, %3 entries, lo=%4, hi=%5",
                si->chForm, si->uTable, si->iNumTable, si->iLower, si->iUpper);
    }

    Address switchDestination;
    int     iNumOut, iNum;
    iNumOut = si->iUpper - si->iLower + 1;
    iNum    = iNumOut;

    // Emit an NWAY BB instead of the COMPJUMP. Also update the number of out edges.
    updateType(BBType::Nway);

    Prog *prog(proc->getProg());
    Cfg  *cfg(proc->getCFG());
    // Where there are repeated switch cases, we have repeated out-edges from the BB. Example:
    // switch (x) {
    //   case 3: case 5:
    //        do something;
    //        break;
    //     case 4: case 10:
    //        do something else
    // ... }
    // The switch statement is emitted assuming one out-edge for each switch value, which is assumed to be iLower+i
    // for the ith zero-based case. It may be that the code for case 5 above will be a goto to the code for case 3,
    // but a smarter back end could group them
    std::list<Address> dests;

    for (int i = 0; i < iNum; i++) {
        // Get the destination address from the switch table.
        if (si->chForm == 'H') {
            int iValue = prog->readNative4(si->uTable + i * 2);

            if (iValue == -1) {
                continue;
            }

            switchDestination = Address(prog->readNative4(si->uTable + i * 8 + 4));
        }
        else if (si->chForm == 'F') {
            switchDestination = Address(((int *)si->uTable.value())[i]);
        }
        else {
            switchDestination = Address(prog->readNative4(si->uTable + i * 4));
        }

        if ((si->chForm == 'O') || (si->chForm == 'R') || (si->chForm == 'r')) {
            // Offset: add table address to make a real pointer to code.  For type R, the table is relative to the
            // branch, so take iOffset. For others, iOffset is 0, so no harm
            if (si->chForm != 'R') {
                assert(si->iOffset == 0);
            }

            switchDestination += si->uTable - si->iOffset;
        }

        if (switchDestination < prog->getLimitTextHigh()) {
            // tq.visit(cfg, uSwitch, this);
            cfg->addOutEdge(this, switchDestination, true);
            // Remember to decode the newly discovered switch code arms, if necessary
            // Don't do it right now, in case there are recursive switch statements (e.g. app7win.exe from
            // hackthissite.org)
            dests.push_back(switchDestination);
        }
        else {
            LOG_MSG("Switch table entry branches to past end of text section %1", switchDestination);

            // TMN: If we reached an array entry pointing outside the program text, we can be quite confident the array
            // has ended. Don't try to pull any more data from it.
            LOG_MSG("Assuming the end of the pointer-array has been reached at index %1", i);

            // TODO: Elevate this logic to the code calculating iNumTable, but still leave this code as a safeguard.
            // Q: Should iNumOut and m_iNumOutEdges really be adjusted (iNum - i) ?
            // assert(iNumOut        >= (iNum - i));
            assert(int(m_outEdges.size()) >= (iNum - i));
            size_t remove_from_this = m_outEdges.size() - (iNum - i);
            // remove last (iNum - i) out edges
            m_outEdges.erase(m_outEdges.begin() + remove_from_this, m_outEdges.end());
            break;
        }
    }

    // Decode the newly discovered switch code arms, if any, and if not already decoded
    int count = 0;

    for (Address addr : dests) {
        char tmp[1024];
        count++;
        sprintf(tmp, "before decoding fragment %i of %zu (%s)", count, dests.size(), qPrintable(addr.toString()));
        Boomerang::get()->alertDecompileDebugPoint(proc, tmp);
        prog->decodeFragment(proc, addr);
    }
}


bool BasicBlock::undoComputedBB(Statement *stmt)
{
    RTL *last = m_listOfRTLs->back();

    for (auto rr = last->rbegin(); rr != last->rend(); rr++) {
        if (*rr == stmt) {
            updateType(BBType::Call);
            LOG_MSG("undoComputedBB for statement %1", stmt);
            return true;
        }
    }

    return false;
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


bool BasicBlock::searchAndReplace(const Exp& search, SharedExp replace)
{
    bool ch = false;

    for (RTL *rtl_it : *m_listOfRTLs) {
        for (auto& elem : *rtl_it) {
            ch |= (elem)->searchAndReplace(search, replace);
        }
    }

    return ch;
}
