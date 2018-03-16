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


#include "boomerang/util/Types.h"
#include "boomerang/util/Address.h"

#include <QString>


class QVariant;


/// All information about the sections is contained in these structures.
class BinarySection
{
public:
    BinarySection(Address sourceAddr, uint64 size, const QString& name = "");
    BinarySection(const BinarySection& other);
    BinarySection(BinarySection&& other) = default;

    virtual ~BinarySection();

    BinarySection& operator=(const BinarySection& other) = delete;
    BinarySection& operator=(BinarySection&& other) = default;

public:
    HostAddress getHostAddr() const { return m_hostAddr; }
    Address getSourceAddr()   const { return m_nativeAddr; }
    uint8_t getEndian()       const { return m_endianness; }
    bool isReadOnly()         const { return m_readOnly; }
    bool isCode()             const { return m_code; }
    bool isData()             const { return m_data; }
    uint32_t getSize()        const { return m_size; }
    QString getName()         const { return m_sectionName; }
    uint32_t getEntrySize()   const { return m_sectionEntrySize; }

    void setBss(bool v)             { m_bss = v; }
    void setCode(bool v)            { m_code = v; }
    void setData(bool v)            { m_data = v; }
    void setReadOnly(bool v)        { m_readOnly = v; }
    void setHostAddr(HostAddress v) { m_hostAddr = v; }
    void setSourceAddr(Address v)   { m_nativeAddr = v; }
    void setEntrySize(uint32_t v)   { m_sectionEntrySize = v; }
    void setEndian(uint8_t v)       { m_endianness = v; }

    /// Windows's PE file sections can contain any combination of code, data and bss.
    /// As such, it can't be correctly described by BinarySection, why we need to override
    /// the behaviour of (at least) the question "Is this address in BSS".
    bool isAddressBss(Address a) const;
    bool anyDefinedValues() const;

    void resize(uint32_t newSize);

    void clearDefinedArea();
    void addDefinedArea(Address from, Address to);
    void setAttributeForRange(const QString& name, const QVariant& val, Address from, Address to);
    QVariantMap getAttributesForRange(Address from, Address to);
    QVariant attributeInRange(const QString& attrib, Address from, Address to) const;

private:
    class BinarySectionImpl *m_impl;

    QString               m_sectionName;                     ///< Name of section
    Address               m_nativeAddr = Address::INVALID;   ///< Logical or native load address
    HostAddress           m_hostAddr = HostAddress::INVALID; ///< Host or actual address of data
    uint64                m_size = 0;                        ///< Size of section in bytes
    uint32_t              m_sectionEntrySize = 0;            ///< Size of one section entry (if applicable)
    unsigned              m_code     : 1;                    ///< Set if section contains instructions
    unsigned              m_data     : 1;                    ///< Set if section contains data
    unsigned              m_bss      : 1;                    ///< Set if section is BSS (allocated only)
    unsigned              m_readOnly : 1;                    ///< Set if this is a read only section
    uint8_t               m_endianness = 0;                  ///< 0 Little endian, 1 Big endian
};
