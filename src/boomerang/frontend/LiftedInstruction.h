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


/**
 * A single part of a lifted instruction.
 */
class BOOMERANG_API LiftedInstructionPart : public GraphNode<LiftedInstructionPart>
{
public:
    LiftedInstructionPart(std::unique_ptr<RTL> rtl);

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

public:
    /// Remove all added instruction parts from this instruction.
    void reset();

    /// \returns true if this instruction only contains a single part.
    bool isSimple() const;

    /// Add a new instruction part to this instruction.
    /// No edges are added between instruction parts.
    LiftedInstructionPart *addPart(std::unique_ptr<RTL> rtl);

    /// Add an edge between two instruction parts.
    void addEdge(LiftedInstructionPart *from, LiftedInstructionPart *to);

    /// Moves all constructed instruction parts into a list and returns it.
    std::list<LiftedInstructionPart> use();

    RTL *getFirstRTL() { return m_parts.front().m_rtl.get(); }
    const RTL *getFirstRTL() const { return m_parts.front().m_rtl.get(); }

    /// Same as \ref LiftedInstruction::use, but specialized for simple instructions
    /// consisting only of a single RTL.
    std::unique_ptr<RTL> useSingleRTL();

private:
    std::list<LiftedInstructionPart> m_parts;
};
