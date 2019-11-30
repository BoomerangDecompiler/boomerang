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


#include "boomerang/ssl/RTL.h"


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
    struct Edge
    {
        const RTL *from;
        const RTL *to;
    };

public:
    LiftedInstruction();
    LiftedInstruction(const LiftedInstruction &) = delete;
    LiftedInstruction(LiftedInstruction &&);

    ~LiftedInstruction();

    // clang-format off
    LiftedInstruction &operator=(const LiftedInstruction &) = delete;
    LiftedInstruction &operator=(LiftedInstruction &&);
    // clang-fomat on

    /// Resets all the fields to their default values.
    void reset();

    bool isSingleRTL() const { return m_rtls.size() == 1; }

    void appendRTL(std::unique_ptr<RTL> rtl, int numRTLsBefore);

    std::unique_ptr<RTL> useSingleRTL();
    RTLList useRTLs();

    RTL *getFirstRTL();
    const RTL *getFirstRTL() const;

public:
    void addEdge(const RTL *from, const RTL *to);
    const std::list<Edge> &getEdges() const { return m_edges; }

private:
    RTLList m_rtls;
    std::list<Edge> m_edges;
};
