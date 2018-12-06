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


/**
 * Manages and provides access to register information of a single architecture.
 */
class BOOMERANG_API RegDB
{
public:
    RegDB();
    ~RegDB();

public:
    /// Removes all registers and their relations from this db.
    void clear();

public:
    /// \param regID must be >= 0
    /// \returns true iff \p regID is the index of a normal register.
    bool isRegIdxDefined(RegID regID) const;

    /// \returns true iff \p regName is the name of a register (normal or special).
    bool isRegDefined(const QString &regName) const;

    /// \returns the register information of a normal register. Returns nullptr
    /// if the register does not exist or is a special register.
    Register *getRegByID(RegID regID);

    /// \returns the register information of a normal or special register.
    /// Returns 0 if the register does not exist.
    Register *getRegByName(const QString &name);

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
    /// Creates a new register.
    /// \param regType Type of the register (Int, Float, Flags)
    /// \param id A unique nonnegative number, or RegIDSpecial for a special register.
    ///           If \p id is not RegIDSpecial and \p id already exists, a register alias
    ///           is created in which case \p regType and \p size must match the existing register.
    /// \param name A unique name. Will fail if the name already exists.
    /// \param size The size in bits of the new register. Must be > 0.
    /// \returns true on success, false on failure.
    bool createReg(RegType regType, RegID id, const QString &name, int size);

    /// Creates a register relation of two existing registers
    /// (e.g. for SHARES or COVERS constructs).
    /// \param parent name of the parent (larger) register. The RegID of \p parent
    ///               must not be RegIDSpecial.
    /// \param child  Name of the child (smaller) register. The RegID of \p child
    ///               may be RegIDSpecial (but should be >= 0).
    /// \param offsetInParent Offset (in bits) of the child register.
    /// \returns true on success, false on failure.
    bool createRegRelation(const QString &parent, const QString &child, int offsetInParent);

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

    /// Register coverage information
    std::map<QString, QString> m_parent;                  ///< child -> parent
    std::map<QString, int> m_offsetInParent;              ///< child -> offset (if parent exists)
    std::map<QString, std::map<int, QString>> m_children; ///< parent -> (offset -> child)
};
