#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MachineInstruction.h"


void MachineInstruction::setGroup(MIGroup groupID, bool enabled)
{
    if (enabled) {
        m_groups |= 1 << (int)groupID;
    }
    else {
        m_groups &= ~(1 << (int)groupID);
    }
}


bool MachineInstruction::isInGroup(MIGroup groupID) const
{
    return (m_groups & (1 << (int)groupID)) != 0;
}


bool MachineInstruction::isValid() const
{
    return m_valid;
}
