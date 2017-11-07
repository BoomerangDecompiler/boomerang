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


/**
 * \file        register.h
 * OVERVIEW:    Header information for the Register class.
 */

#include <string>
#include <stdint.h>
#include <memory>
#include <QString>


class Type;
typedef std::shared_ptr<Type> SharedType;


/**
 * Summarises one line of the \@REGISTERS section of an SSL
 * file. This class is used extensively in sslparser.y, and there is a public
 * member of RTLInstDict called DetRegMap which gives a Register object from
 * a register index (register indices may not always be sequential, hence it's
 * not just an array of Register objects).
 */
class Register
{
public:
    Register(const QString& name = QString::null, uint16_t sizeInBits = 0, bool isFloatReg = false);
    Register(const Register&);

    Register& operator=(const Register& other);

    bool operator==(const Register& other) const;
    bool operator<(const Register& other) const;


    const QString& getName() const;

    uint16_t getSize() const;

    /// \returns true if this is a floating point register
    bool isFloat() const { return m_fltRegister; }

    /// \returns the type of this register
    SharedType getType() const;

    /// Get the mapped offset (see above)
    int getMappedOffset() const { return m_mappedOffset; }

    /// Get the mapped index (see above)
    int getMappedIndex() const { return m_mappedIndex; }

    void setName(const QString& name);

    void setSize(uint16_t newSize) { m_size = newSize; }

    void setIsFloat(bool isFloatReg) { m_fltRegister = isFloatReg; }

    /**
     * Set the mapped offset. This is the bit number where this register starts,
     * e.g. for register %ah, this is 8. For COVERS regisers, this is 0
     */
    void setMappedOffset(int i) { m_mappedOffset = i; }

    /**
     * Set the mapped index. For COVERS registers, this is the lower register
     * of the set that this register covers. For example, if the current register
     * is f28to31, i would be the index for register f28
     * For SHARES registers, this is the "parent" register, e.g. if the current
     * register is %al, the parent is %ax (note: not %eax)
     */
    void setMappedIndex(int i) { m_mappedIndex = i; }

private:
    QString m_name;
    uint16_t m_size;
    bool m_fltRegister; ///< True if this is a floating point register
    int m_mappedIndex;
    int m_mappedOffset;
};
