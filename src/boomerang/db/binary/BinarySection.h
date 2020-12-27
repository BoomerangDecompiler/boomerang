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


#include "boomerang/util/Address.h"
#include "boomerang/util/ByteUtil.h"
#include "boomerang/util/Types.h"

#include <QString>


/// File-format independent access to sections of binary files.
class BOOMERANG_API BinarySection
{
public:
    BinarySection(Address sourceAddr, uint64 size, const QString &name = "");
    BinarySection(const BinarySection &other) = delete;
    BinarySection(BinarySection &&other)      = default;

    virtual ~BinarySection();

    BinarySection &operator=(const BinarySection &other) = delete;
    BinarySection &operator=(BinarySection &&other) = default;

public:
    HostAddress getHostAddr() const { return m_hostAddr; }
    Address getSourceAddr() const { return m_nativeAddr; }
    Endian getEndian() const { return m_endianness; }
    bool isReadOnly() const { return m_readOnly; }
    bool isCode() const { return m_code; }
    bool isData() const { return m_data; }
    int getSize() const { return m_size; }
    QString getName() const { return m_sectionName; }
    uint32_t getEntrySize() const { return m_sectionEntrySize; }

    void setBss(bool v) { m_bss = v; }
    void setCode(bool v) { m_code = v; }
    void setData(bool v) { m_data = v; }
    void setReadOnly(bool v) { m_readOnly = v; }
    void setHostAddr(HostAddress v) { m_hostAddr = v; }
    void setSourceAddr(Address v) { m_nativeAddr = v; }
    void setEntrySize(uint32_t v) { m_sectionEntrySize = v; }
    void setEndian(Endian v) { m_endianness = v; }

    void resize(uint32_t newSize);

    /// Windows's PE file sections can contain any combination of code, data and bss.
    /// As such, it can't be correctly described by BinarySection, why we need to override
    /// the behaviour of (at least) the question "Is this address in BSS".
    bool isAddressBss(Address addr) const;

    bool anyDefinedValues() const;
    void addDefinedArea(Address from, Address to);

    void setAttributeForRange(const QString &attrName, Address from, Address to);
    bool addressHasAttribute(const QString &attrName, Address addr) const;

private:
    class BinarySectionImpl *m_impl;

    QString m_sectionName;                              ///< Name of section
    Address m_nativeAddr        = Address::INVALID;     ///< Logical or native load address
    HostAddress m_hostAddr      = HostAddress::INVALID; ///< Host or actual address of data
    uint64 m_size               = 0;                    ///< Size of section in bytes
    uint32_t m_sectionEntrySize = 0;      ///< Size of one section entry (if applicable)
    unsigned m_code : 1;                  ///< Set if section contains instructions
    unsigned m_data : 1;                  ///< Set if section contains data
    unsigned m_bss : 1;                   ///< Set if section is BSS (allocated only)
    unsigned m_readOnly : 1;              ///< Set if this is a read only section
    Endian m_endianness = Endian::Little; ///< Endianness of section bytes
};
