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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/Types.h"

#include <array>
#include <variant>
#include <vector>


constexpr const sint32 MNEM_SIZE  = 32;
constexpr const sint32 OPSTR_SIZE = 160;

enum class MIGroup
{
    Call = 0,
    Jump,
    Computed,
    BoolAsgn,
    COUNT
};


/**
 * These are the instruction classes defined in
 * "A Transformational Approach to Binary Translation of Delayed Branches"
 * for SPARC instructions.
 * Ignored by machines with no delay slots.
 */
enum class IClass : uint8
{
    NOP, ///< No operation (e.g. SPARC BN,A)
    NCT, ///< Non Control Transfer

    SD,    ///< Static Delayed
    DD,    ///< Dynamic Delayed
    SCD,   ///< Static Conditional Delayed
    SCDAN, ///< Static Conditional Delayed, Anulled if Not taken
    SCDAT, ///< Static Conditional Delayed, Anulled if Taken
    SU,    ///< Static Unconditional (not delayed)
    SKIP   ///< Skip successor
};


class BOOMERANG_API MachineInstruction
{
public:
    Address m_addr;      ///< Address (IP) of the instruction
    uint32 m_id;         ///< instruction unique ID (e.g. MOV, ADD etc.)
    uint16 m_size   = 0; ///< Size in bytes
    uint8 m_groups  = 0;
    bool m_valid    = false;
    IClass m_iclass = IClass::NOP;

    std::array<char, MNEM_SIZE> m_mnem   = { 0 };
    std::array<char, OPSTR_SIZE> m_opstr = { 0 };

    std::vector<SharedExp> m_operands;
    QString m_templateName; ///< Name of SSL IR template (e.g. REPSTOSB.rm8 or MOVSX.r32.rm8)

    int m_sparcCC;

public:
    /// Enables or disables the membership in a certain group. Does not affect other groups.
    void setGroup(MIGroup groupID, bool enabled);
    bool isInGroup(MIGroup groupID) const;

    bool isValid() const;

    std::size_t getNumOperands() const { return m_operands.size(); }
};

static_assert(8 * sizeof(MachineInstruction::m_groups) >= (int)MIGroup::COUNT);
