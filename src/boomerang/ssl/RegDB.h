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


#include "Register.h"

#include <map>


class RegDB
{
public:
    RegDB();
    ~RegDB();

public:
    void clear() {
        m_regIDs.clear();
        m_regInfo.clear();
        m_specialRegInfo.clear();
    }

public:
    bool isRegDefined(const QString &regName) const { return m_regIDs.find(regName) != m_regIDs.end(); }
    bool isRegIdxDefined(int regID) const { return m_regInfo.find(regID) != m_regInfo.end(); }

    Register *getRegByID(int regID)
    {
        const auto it = m_regInfo.find(regID);
        return it != m_regInfo.end() ? &it->second : nullptr;
    }

    /// Get the index of a named register by its name.
    /// Returns -1 if the register was not found.
    int getRegIDByName(const QString &name) const
    {
        const auto it = m_regIDs.find(name);
        return it != m_regIDs.end() ? it->second : -1;
    }

    /// Get the name of the register by its index.
    /// Returns the empty string when \p regID == -1 or the register was not found.
    QString getRegNameByID(int regID) const
    {
        const auto it = m_regInfo.find(regID);
        return it != m_regInfo.end() ? it->second.getName() : "";
    }

    /// Get the size in bits of a register by its index.
    /// Returns 32 (the default register size) if the register was not found.
    int getRegSizeByID(int regID) const
    {
        const auto iter = m_regInfo.find(regID);
        return iter != m_regInfo.end() ? iter->second.getSize() : 32;
    }

public:
    void addRegister(const QString &name, int id, int size, bool flt)
    {
        m_regIDs[name] = id;

        if (id == -1) {
            m_specialRegInfo.insert(std::make_pair(name, Register(name, size, flt)));
        }
        else {
            m_regInfo.insert(std::make_pair(id, Register(name, size, flt)));
        }
    }

private:
    /// A map from the symbolic representation of a register (e.g. "%g0")
    /// to its index within an array of registers.
    /// This map contains both normal and special (-> -1) registers,
    /// therefore this map contains all registers.
    std::map<QString, int> m_regIDs;

    /// Stores info about a register such as its size, its addresss etc
    /// (see register.h).
    std::map<int, Register> m_regInfo;

    /// A map from symbolic representation of a special (non-addressable) register
    /// to a Register object
    std::map<QString, Register> m_specialRegInfo;
};
