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
#include "boomerang/ssl/Register.h"

#include <map>
#include <set>


class Assignment;


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
    /// \param regNum must be >= 0
    /// \returns true iff \p regNum is the index of a normal register.
    bool isRegNumDefined(RegNum regNum) const;

    /// \returns true iff \p regName is the name of a register (normal or special).
    bool isRegDefined(const QString &regName) const;

    /// \returns the register information of a normal register. Returns nullptr
    /// if the register does not exist or is a special register.
    const Register *getRegByNum(RegNum regNum) const;

    /// \returns the register information of a normal or special register.
    /// Returns 0 if the register does not exist.
    const Register *getRegByName(const QString &name) const;

    /// Get the index of a named register by its name.
    /// Returns -1 if the register was not found.
    RegNum getRegNumByName(const QString &name) const;

    /// Get the index of a named register by its name.
    /// Returns -1 if the register was not found.
    RegID getRegIDByName(const QString &name) const;

    /// Get the name of the register by its index.
    /// Returns the empty string when \p regNum == RegNumSpecial or the register was not found.
    QString getRegNameByNum(RegNum regNum) const;

    /// Get the size in bits of a register by its index.
    /// Returns 32 (the default register size) if the register was not found.
    int getRegSizeByNum(RegNum regNum) const;

public:
    /// Creates a new register.
    /// \param regType Type of the register (Int, Float, Flags)
    /// \param regNum A unique nonnegative number, or RegNumSpecial for a special register.
    ///           If \p regNum is not RegNumSpecial and \p regNum already exists, a register alias
    ///           is created in which case \p regType and \p size must match the existing register.
    /// \param name A unique name. Will fail if the name already exists.
    /// \param size The size in bits of the new register. Must be > 0.
    /// \returns true on success, false on failure.
    bool createReg(RegType regType, RegNum regNum, const QString &name, int size);

    /// Creates a register relation of two existing registers
    /// (e.g. for SHARES or COVERS constructs).
    /// \param parent name of the parent (larger) register. The RegNum of \p parent
    ///               must not be RegNumSpecial.
    /// \param child  Name of the child (smaller) register. The RegNum of \p child
    ///               may be RegNumSpecial (but should be >= 0).
    /// \param offsetInParent Offset (in bits) of the child register.
    /// \returns true on success, false on failure.
    bool createRegRelation(const QString &parent, const QString &child, int offsetInParent);

    /// Process the effects of overlapped registers for \p stmt.
    /// Example: (x86 register overlap)
    ///   Suppose \p stmt is %ax := 0x1234, and %eax and %ah are used in the procedure that contains
    ///   \p stmt. Then we need to generate two additional statements:
    ///     *32* %eax := (%eax & 0xFFFF0000) | %ax, and
    ///      *8* %ah  := %ax@[0..7]
    ///   to model the effects of overlapped registers. This procedure returns the additional
    ///   statements in a new RTL.
    ///   We do not need to generate an assignment for %al, since %al is not used in the current
    ///   procedure (This is indicated by \p usedRegs not containing %al).
    /// \returns all additional statements
    std::unique_ptr<RTL> processOverlappedRegs(Assignment *stmt,
                                               const std::set<RegNum> &usedRegs) const;

private:
    /// Emit a new statement assigning the content of \p rhs into \p lhs.
    /// There are 2 cases:
    ///  1. The LHS is larger. In this case, assign only the bits of \p lhs
    ///     that also belong to \p rhs (e.g. %eax := (%eax & 0xFFFF00FF) | (%ah << 8))
    ///     (Note: It might also be possible to emit %eax@[8:15] := %ah,
    ///     but other code does not support this yet).
    ///  2. The RHS is larger. In this case, use only the bits of the RHS
    ///     that also belong to \p lhs. (e.g. %ah := %eax@[8..15])
    ///
    /// \param original The orignal assignment
    /// \param lhs The register that is assigned to
    /// \param rhs The register that is assigned from
    /// \param offsetInParent The offset in bits of the child register (for %eax -> %ah this is 8)
    /// \returns the new register content mapping assignment.
    Assignment *emitOverlappedStmt(const Assignment *original, const Register *lhs,
                                   const Register *rhs, int offsetInParent) const;

private:
    /// A map from the symbolic representation of a register (e.g. "%g0")
    /// to its index within an array of registers.
    /// This map contains both normal and special (-> -1) registers,
    /// therefore this map contains all registers.
    std::map<QString, RegID> m_regNums;

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
