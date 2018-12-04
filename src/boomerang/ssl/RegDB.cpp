#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RegDB.h"


RegDB::RegDB()
{
}


RegDB::~RegDB()
{
}


void RegDB::clear()
{
    m_regIDs.clear();
    m_regInfo.clear();
    m_specialRegInfo.clear();
}


bool RegDB::isRegDefined(const QString& regName) const
{
    return m_regIDs.find(regName) != m_regIDs.end();
}


bool RegDB::isRegIdxDefined(int regID) const
{
    return m_regInfo.find(regID) != m_regInfo.end();
}


Register *RegDB::getRegByID(int regID)
{
    const auto it = m_regInfo.find(regID);
    return it != m_regInfo.end() ? &it->second : nullptr;
}


RegID RegDB::getRegIDByName(const QString& name) const
{
    const auto it = m_regIDs.find(name);
    return it != m_regIDs.end() ? it->second : -1;
}


QString RegDB::getRegNameByID(RegID regID) const
{
    const auto it = m_regInfo.find(regID);
    return it != m_regInfo.end() ? it->second.getName() : "";
}


int RegDB::getRegSizeByID(RegID regID) const
{
    const auto iter = m_regInfo.find(regID);
    return iter != m_regInfo.end() ? iter->second.getSize() : 32;
}


void RegDB::createRegister(RegType regType, RegID id, const QString& name, int size)
{
    m_regIDs[name] = id;

    if (id == RegIDSpecial) {
        m_specialRegInfo.insert(std::make_pair(name, Register(regType, name, size)));
    }
    else {
        m_regInfo.insert(std::make_pair(id, Register(regType, name, size)));
    }
}
