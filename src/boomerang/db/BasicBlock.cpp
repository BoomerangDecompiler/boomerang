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

#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/statements/PhiAssign.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"


BasicBlock::BasicBlock(Address lowAddr, Function *function)
    : m_function(function)
    , m_ir(this, nullptr)
    , m_lowAddr(lowAddr)
    , m_bbType(BBType::Invalid)
{
}


BasicBlock::BasicBlock(BBType bbType, std::unique_ptr<RTLList> bbRTLs, Function *function)
    : m_function(function)
    , m_ir(this, nullptr)
    , m_bbType(bbType)
{
    assert(bbRTLs);

    // Set the RTLs. This also updates the low and the high address of the BB.
    completeBB(std::move(bbRTLs));
}


BasicBlock::BasicBlock(const BasicBlock &bb)
    : GraphNode(bb)
    , m_function(bb.m_function)
    , m_ir(this, nullptr)
    , m_lowAddr(bb.m_lowAddr)
    , m_highAddr(bb.m_highAddr)
    , m_bbType(bb.m_bbType)
// m_labelNeeded is initialized to false, not copied
{
    if (bb.m_ir.m_listOfRTLs) {
        // make a deep copy of the RTL list
        std::unique_ptr<RTLList> newList(new RTLList());
        newList->resize(bb.m_ir.m_listOfRTLs->size());

        RTLList::const_iterator srcIt = bb.m_ir.m_listOfRTLs->begin();
        RTLList::const_iterator endIt = bb.m_ir.m_listOfRTLs->end();
        RTLList::iterator destIt      = newList->begin();

        while (srcIt != endIt) {
            *destIt++ = std::make_unique<RTL>(**srcIt++);
        }
        completeBB(std::move(newList));
    }
}


BasicBlock::~BasicBlock()
{
}


BasicBlock &BasicBlock::operator=(const BasicBlock &bb)
{
    GraphNode::operator=(bb);

    m_function = bb.m_function;
    m_lowAddr  = bb.m_lowAddr;
    m_highAddr = bb.m_highAddr;
    m_bbType   = bb.m_bbType;
    // m_labelNeeded is initialized to false, not copied

    if (bb.m_ir.m_listOfRTLs) {
        // make a deep copy of the RTL list
        std::unique_ptr<RTLList> newList(new RTLList());
        newList->resize(bb.m_ir.m_listOfRTLs->size());

        RTLList::const_iterator srcIt = bb.m_ir.m_listOfRTLs->begin();
        RTLList::const_iterator endIt = bb.m_ir.m_listOfRTLs->end();
        RTLList::iterator destIt      = newList->begin();

        while (srcIt != endIt) {
            *destIt++ = std::make_unique<RTL>(**srcIt++);
        }
        completeBB(std::move(newList));
    }

    return *this;
}


void BasicBlock::completeBB(std::unique_ptr<RTLList> rtls)
{
    assert(m_ir.m_listOfRTLs == nullptr);
    assert(rtls != nullptr);
    assert(!rtls->empty());

    m_ir.m_listOfRTLs = std::move(rtls);
    updateBBAddresses();

    bool firstRTL = true;

    for (auto &rtl : *m_ir.m_listOfRTLs) {
        for (const SharedStmt &stmt : *rtl) {
            assert(stmt != nullptr);
            stmt->setBB(this);
        }

        if (!firstRTL) {
            assert(rtl->getAddress() != Address::ZERO);
        }

        firstRTL = false;
    }
}


QString BasicBlock::toString() const
{
    QString tgt;
    OStream ost(&tgt);
    print(ost);
    return tgt;
}


void BasicBlock::print(OStream &os) const
{
    switch (getType()) {
    case BBType::Oneway: os << "Oneway BB"; break;
    case BBType::Twoway: os << "Twoway BB"; break;
    case BBType::Nway: os << "Nway BB"; break;
    case BBType::Call: os << "Call BB"; break;
    case BBType::Ret: os << "Ret BB"; break;
    case BBType::Fall: os << "Fall BB"; break;
    case BBType::CompJump: os << "Computed jump BB"; break;
    case BBType::CompCall: os << "Computed call BB"; break;
    case BBType::Invalid: os << "Invalid BB"; break;
    }

    os << ":\n";
    os << "  in edges: ";

    for (BasicBlock *bb : getPredecessors()) {
        os << bb->getHiAddr() << "(" << bb->getLowAddr() << ") ";
    }

    os << "\n";
    os << "  out edges: ";

    for (BasicBlock *bb : getSuccessors()) {
        os << bb->getLowAddr() << " ";
    }

    os << "\n";

    if (m_ir.m_listOfRTLs) { // Can be null if e.g. INVALID
        for (auto &rtl : *m_ir.m_listOfRTLs) {
            rtl->print(os);
        }
    }
}


Address BasicBlock::getLowAddr() const
{
    return m_lowAddr;
}


Address BasicBlock::getHiAddr() const
{
    return m_highAddr;
}


void BasicBlock::updateBBAddresses()
{
    if ((m_ir.m_listOfRTLs == nullptr) || m_ir.m_listOfRTLs->empty()) {
        m_highAddr = Address::INVALID;
        return;
    }

    Address a = m_ir.m_listOfRTLs->front()->getAddress();

    if (a.isZero() && (m_ir.m_listOfRTLs->size() > 1)) {
        RTLList::iterator it = m_ir.m_listOfRTLs->begin();
        Address add2         = (*++it)->getAddress();

        // This is a bit of a hack for 286 programs, whose main actually starts at offset 0. A
        // better solution would be to change orphan BBs' addresses to Address::INVALID, but I
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

    assert(m_ir.m_listOfRTLs != nullptr);
    m_highAddr = m_ir.m_listOfRTLs->back()->getAddress();
}
