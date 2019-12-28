#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/db/GraphNode.h"
#include "boomerang/ssl/RTL.h"


class BOOMERANG_API LiftedInstructionPart : public GraphNode<LiftedInstructionPart>
{
public:
    LiftedInstructionPart(std::unique_ptr<RTL> rtl)
        : m_rtl(std::move(rtl))
    {
    }

public:
    std::unique_ptr<RTL> m_rtl;
};


/**
 * Contains all the information that results from lifting a \ref MachineInstruction.
 * Usually a single instruction is lifted to a single RTL, howewer sometimes
 * there may be multiple RTLs for an instruction (e.g. x86 BSF/BSR).
 *
 * \sa IDecoder::liftInstruction
 */
class BOOMERANG_API LiftedInstruction
{
public:
    LiftedInstruction();
    LiftedInstruction(const LiftedInstruction &) = delete;
    LiftedInstruction(LiftedInstruction &&);

    ~LiftedInstruction();

    // clang-format off
    LiftedInstruction &operator=(const LiftedInstruction &) = delete;
    LiftedInstruction &operator=(LiftedInstruction &&);
    // clang-fomat on

    void reset() { m_parts.clear(); }

    bool isSimple() const { return m_parts.size() == 1; }

    LiftedInstructionPart *addPart(std::unique_ptr<RTL> rtl);

    void addEdge(LiftedInstructionPart *from, LiftedInstructionPart *to);

    std::list<LiftedInstructionPart> use();

    RTL *getFirstRTL() { return m_parts.front().m_rtl.get(); }
    const RTL *getFirstRTL() const { return m_parts.front().m_rtl.get(); }

    std::unique_ptr<RTL> useSingleRTL()
    {
        assert(m_parts.size() == 1);
        std::unique_ptr<RTL> result = std::move(m_parts.back().m_rtl);
        m_parts.clear();
        return result;
    }

private:
    std::list<LiftedInstructionPart> m_parts;
};
