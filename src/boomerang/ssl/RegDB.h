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
    void clear();

public:
    bool isRegDefined(const QString &regName) const;
    bool isRegIdxDefined(RegID regID) const;

    Register *getRegByID(RegID regID);

    /// Get the index of a named register by its name.
    /// Returns -1 if the register was not found.
    RegID getRegIDByName(const QString &name) const;

    /// Get the name of the register by its index.
    /// Returns the empty string when \p regID == -1 or the register was not found.
    QString getRegNameByID(RegID regID) const;

    /// Get the size in bits of a register by its index.
    /// Returns 32 (the default register size) if the register was not found.
    int getRegSizeByID(RegID regID) const;

public:
    void addRegister(const QString &name, RegID id, int size, bool flt);

private:
    /// A map from the symbolic representation of a register (e.g. "%g0")
    /// to its index within an array of registers.
    /// This map contains both normal and special (-> -1) registers,
    /// therefore this map contains all registers.
    std::map<QString, RegID> m_regIDs;

    /// Stores info about a register such as its size, its addresss etc
    /// (see register.h).
    std::map<RegID, Register> m_regInfo;

    /// A map from symbolic representation of a special (non-addressable) register
    /// to a Register object
    std::map<QString, Register> m_specialRegInfo;
};
