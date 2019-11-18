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


BasicBlock::BasicBlock(Address lowAddr, Function *function)
    : m_function(function)
    , m_ir(this, lowAddr)
    , m_bbType(BBType::Invalid)
{
}


BasicBlock::BasicBlock(BBType bbType, const std::vector<MachineInstruction> &insns,
                       Function *function)
    : m_function(function)
    , m_ir(this, nullptr)
    , m_bbType(bbType)
{
    assert(!insns.empty());

    // Set the RTLs. This also updates the low and the high address of the BB.
    completeBB(insns);
}


BasicBlock::BasicBlock(const BasicBlock &bb)
    : GraphNode(bb)
    , m_function(bb.m_function)
    , m_ir(bb.m_ir)
    , m_bbType(bb.m_bbType)
{
}


BasicBlock::~BasicBlock()
{
}


BasicBlock &BasicBlock::operator=(const BasicBlock &bb)
{
    GraphNode::operator=(bb);

    m_function = bb.m_function;
    m_ir       = bb.m_ir;
    m_bbType   = bb.m_bbType;

    return *this;
}


void BasicBlock::completeBB(const std::vector<MachineInstruction> &insns)
{
    assert(!insns.empty());
    assert(m_insns.empty());

    m_insns = insns;
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
        os << bb->getIR()->getHiAddr() << "(" << bb->getIR()->getLowAddr() << ") ";
    }

    os << "\n";
    os << "  out edges: ";

    for (BasicBlock *bb : getSuccessors()) {
        os << bb->getIR()->getLowAddr() << " ";
    }

    os << "\n";

    if (m_ir.m_listOfRTLs) { // Can be null if e.g. INVALID
        for (auto &rtl : *m_ir.m_listOfRTLs) {
            rtl->print(os);
        }
    }
}
