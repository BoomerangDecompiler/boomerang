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


Register *RegDB::getRegByName(const QString &name)
{
    const RegID id = getRegIDByName(name);
    if (id == RegIDSpecial) {
        const auto it = m_specialRegInfo.find(name);
        return it != m_specialRegInfo.end() ? &it->second : nullptr;
    }
    else {
        return getRegByID(id);
    }
}


RegID RegDB::getRegIDByName(const QString& name) const
{
    const auto it = m_regIDs.find(name);
    return it != m_regIDs.end() ? it->second : RegIDSpecial;
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


bool RegDB::createReg(RegType regType, RegID id, const QString& name, int size)
{
    if (name.isEmpty() || size <= 0 || regType == RegType::Invalid) {
        return false;
    }

    const auto &[_, inserted] = m_regIDs.insert({ name, id });
    if (!inserted) {
        // a register with the same name (or alias) already exists
        return false;
    }

    if (id == RegIDSpecial) {
        // otherwise would have been caught above
        assert(m_specialRegInfo.find(name) == m_specialRegInfo.end());
        m_specialRegInfo.insert({ name, Register(regType, name, size) });
        return true;
    }

    const auto it = m_regInfo.find(id);
    if (it != m_regInfo.end()) {
        // register alias: only name can be different
        const Register &reg = it->second;
        if (regType != reg.getRegType() || size != reg.getSize()) {
            m_regIDs.erase(name);
            return false;
        }
    }
    else {
        m_regInfo.insert({ id, Register(regType, name, size) });
    }

    return true;
}


bool RegDB::createRegRelation(const QString &parent, const QString &child, int offsetInParent)
{
    if (parent == child) {
        return false;
    }
    else if (!isRegDefined(parent) || !isRegDefined(child)) {
        return false;
    }
    else if (getRegIDByName(parent) == RegIDSpecial) {
        // parent is a special register -> fail
        return false;
    }
    else if (m_parent.find(child) != m_parent.end() ||
             m_offsetInParent.find(child) != m_offsetInParent.end() ||
             (m_children.find(parent) != m_children.end() &&
              m_children.at(parent).find(offsetInParent) != m_children.at(parent).end())) {
        // relation already exists
        return false;
    }

    m_parent[child] = parent;
    m_offsetInParent[child] = offsetInParent;
    m_children[parent][offsetInParent] = child;
    return true;
}
