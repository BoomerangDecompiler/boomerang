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
#define ARRAY_UNBOUNDED    9999999


class BOOMERANG_API ArrayType : public Type
{
public:
    /// Create a new array type of fixed length
    ArrayType(SharedType baseType, unsigned length = ARRAY_UNBOUNDED);

    ArrayType(const ArrayType& other) = default;
    ArrayType(ArrayType&& other) = default;

    virtual ~ArrayType() override = default;

    ArrayType& operator=(const ArrayType& other) = default;
    ArrayType& operator=(ArrayType&& other) = default;

public:
    virtual bool isArray() const override { return true; }

    /// \returns the type of elements of this array
    SharedType getBaseType() { return BaseType; }
    const SharedType getBaseType() const { return BaseType; }

    /// Changes type of the elements of this array.
    void setBaseType(SharedType b);
    void fixBaseType(SharedType b);

    /// \returns the number of elements in this array.
    size_t getLength() const { return m_length; }
    void setLength(unsigned n) { m_length = n; }

    /// \returns true iff we do not know the length of the array (yet)
    bool isUnbounded() const;

    virtual SharedType clone() const override;

    static std::shared_ptr<ArrayType> get(SharedType p, unsigned _length) { return std::make_shared<ArrayType>(p, _length); }
    static std::shared_ptr<ArrayType> get(SharedType p) { return std::make_shared<ArrayType>(p); }

    virtual bool operator==(const Type& other) const override;
    virtual bool operator<(const Type& other) const override;

    virtual size_t getSize() const override;
    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool& changed, bool useHighestPtr) const override;

    virtual bool isCompatibleWith(const Type& other, bool all = false) const override { return isCompatible(other, all); }
    virtual bool isCompatible(const Type& other, bool all) const override;

    size_t convertLength(SharedType b) const;

protected:
    ArrayType();

private:
    SharedType BaseType;
    size_t m_length = 0; ///< number of elements in this array
};
