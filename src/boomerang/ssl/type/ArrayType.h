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


#include "boomerang/ssl/type/Type.h"

/**
 * We actually want unbounded arrays to still work correctly when
 * computing aliases.. as such, we give them a very large bound
 * and hope that no-one tries to alias beyond them
 */
#define ARRAY_UNBOUNDED ((uint64)9999999)


class BOOMERANG_API ArrayType : public Type
{
public:
    /// Create a new array type of fixed length
    explicit ArrayType(SharedType baseType, uint64 length = ARRAY_UNBOUNDED);

    ArrayType(const ArrayType &other) = default;
    ArrayType(ArrayType &&other)      = default;

    ~ArrayType() override = default;

    ArrayType &operator=(const ArrayType &other) = default;
    ArrayType &operator=(ArrayType &&other) = default;

public:
    static std::shared_ptr<ArrayType> get(SharedType p, uint64 length = ARRAY_UNBOUNDED);

public:
    /// \copydoc Type::operator==
    bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    SharedType clone() const override;

    /// \copydoc Type::getSize
    Size getSize() const override;

    /// \copydoc Type::getCtype
    QString getCtype(bool final = false) const override;

    /// \copydoc Type::isCompatibleWith
    bool isCompatibleWith(const Type &other, bool all = false) const override;

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

public:
    /// \returns the type of elements of this array
    SharedType getBaseType() { return m_baseType; }
    const SharedType getBaseType() const { return m_baseType; }

    /// Changes type of the elements of this array.
    void setBaseType(SharedType b);

    /// \returns the number of elements in this array.
    uint64 getLength() const { return m_length; }
    void setLength(unsigned n) { m_length = n; }

    /// \returns true iff we do not know the length of the array (yet)
    bool isUnbounded() const;

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool all) const override;

private:
    /// \returns the new number of elements that fit in this array when converting
    /// the base type to \p newBaseType
    uint64 convertLength(SharedType newBaseType) const;

private:
    SharedType m_baseType;
    uint64 m_length = 0; ///< number of elements in this array
};
