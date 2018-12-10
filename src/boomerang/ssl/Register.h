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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/util/Types.h"

#include <QString>

#include <memory>
#include <string>
#include <type_traits>


class Type;

typedef std::shared_ptr<Type> SharedType;

typedef uint16 RegNum;
static constexpr const RegNum RegNumSpecial = 0xFFFF;


enum class RegType
{
    Invalid = 0,
    Int     = 1,
    Float   = 2,
    Flags   = 3
};


class BOOMERANG_API RegID
{
public:
    RegID(RegType regType, RegNum num, uint16 sizeInBits);

    bool operator==(const RegID &rhs) const { return getNum() == rhs.getNum(); }
    bool operator!=(const RegID &rhs) const { return getNum() != rhs.getNum(); }

    bool operator<(const RegID &rhs) const { return getNum() < rhs.getNum(); }

public:
    RegNum getNum() const { return m_num; }
    RegType getRegType() const { return (RegType)m_regType; }
    int getSize() const { return m_size; }

public:
    uint16 m_num;
    uint16 m_regType : 3;
    uint16 m_size : 10;
    uint16 m_reserved : 3;
};


template<typename T, typename Enabler = std::enable_if<!std::is_same<T, RegID>::value>>
bool operator==(const RegID &lhs, T rhs) { return lhs.getNum() == rhs; }
template<typename T, typename Enabler = std::enable_if<!std::is_same<T, RegID>::value>>
bool operator!=(const RegID &lhs, T rhs) { return lhs.getNum() != rhs; }

template<typename T, typename Enabler = std::enable_if<!std::is_same<T, RegID>::value>>
bool operator==(T lhs, const RegID &rhs) { return lhs == rhs.getNum(); }
template<typename T, typename Enabler = std::enable_if<!std::is_same<T, RegID>::value>>
bool operator!=(T lhs, const RegID &rhs) { return lhs != rhs.getNum(); }


/**
 * Summarises one line of the \@REGISTERS section of an SSL
 * file. This class is used extensively in sslparser.y.
 */
class BOOMERANG_API Register
{
public:
    Register(RegType type, const QString &name, uint16_t sizeInBits);
    Register(const Register &);
    Register(Register &&) = default;

    ~Register() = default;

    Register &operator=(const Register &other);
    Register &operator=(Register &&other) = default;

public:
    bool operator==(const Register &other) const;
    bool operator<(const Register &other) const;

    const QString &getName() const;
    uint16_t getSize() const;

    /// \returns the type of the content of this register
    SharedType getType() const;

    /// \returns the type of the register(int, float, flags)
    RegType getRegType() const { return m_regType; }

private:
    QString m_name;
    uint16_t m_size;
    RegType m_regType;
};
