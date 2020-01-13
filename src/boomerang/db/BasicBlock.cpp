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


BasicBlock::BasicBlock(Address lowAddr)
    : m_bbType(BBType::Invalid)
{
    m_lowAddr = lowAddr;
}


BasicBlock::BasicBlock(BBType bbType, const std::vector<MachineInstruction> &insns)
    : m_bbType(bbType)
{
    assert(!insns.empty());

    // Set the RTLs. This also updates the low and the high address of the BB.
    completeBB(insns);
}


BasicBlock::BasicBlock(const BasicBlock &bb)
    : GraphNode(bb)
    , m_proc(bb.m_proc)
    , m_lowAddr(bb.m_lowAddr)
    , m_highAddr(bb.m_highAddr)
    , m_bbType(bb.m_bbType)
{
}


BasicBlock::~BasicBlock()
{
}


BasicBlock &BasicBlock::operator=(const BasicBlock &bb)
{
    if (this == &bb) {
        return *this;
    }

    GraphNode::operator=(bb);

    m_proc     = bb.m_proc;
    m_bbType   = bb.m_bbType;
    m_lowAddr  = bb.m_lowAddr;
    m_highAddr = bb.m_highAddr;

    return *this;
}


void BasicBlock::completeBB(const std::vector<MachineInstruction> &insns)
{
    assert(!insns.empty());
    assert(m_insns.empty());

    m_insns = insns;

    m_lowAddr  = m_insns.front().m_addr;
    m_highAddr = m_insns.back().m_addr + m_insns.back().m_size;
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

    os << "@[" << getLowAddr() << "," << getHiAddr() << "):\n";
    os << "  in edges: ";

    for (BasicBlock *bb : getPredecessors()) {
        os << bb->getLowAddr() << " ";
    }

    os << "\n";
    os << "  out edges: ";

    for (BasicBlock *bb : getSuccessors()) {
        os << bb->getLowAddr() << " ";
    }

    os << "\n";

    for (const MachineInstruction &insn : m_insns) {
        os << insn.m_addr << " " << insn.m_mnem.data() << " " << insn.m_opstr.data() << "\n";
    }
}
