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


struct BOOMERANG_API MachineInstruction
{
    uint32 m_id;        ///< instruction unique ID (e.g. MOV, ADD etc.)
    uint16 m_size  = 0; ///< Size in bytes
    uint8 m_groups = 0;
    bool m_valid   = false;

    std::array<char, MNEM_SIZE> m_mnem   = { 0 };
    std::array<char, OPSTR_SIZE> m_opstr = { 0 };

    std::vector<SharedConstExp> m_operands;
    QString m_variantID; ///< Unique instruction variant ID (e.g. REPSTOSB.rm8 or MOVSX.r32.rm8)

public:
    /// Enables or disables the membership in a certain group. Does not affect other groups.
    void setGroup(MIGroup groupID, bool enabled);
    bool isInGroup(MIGroup groupID) const;

    bool isValid() const;

    std::size_t getNumOperands() const { return m_operands.size(); }
};

static_assert(8 * sizeof(MachineInstruction::m_groups) >= (int)MIGroup::COUNT);
