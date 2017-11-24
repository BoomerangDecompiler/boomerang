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


#include "boomerang/type/type/Type.h"


class FloatType : public Type
{
public:
    explicit FloatType(int sz = 64);
    FloatType(const FloatType& other) = default;
    FloatType(FloatType&& other) = default;

    virtual ~FloatType() override;

    FloatType& operator=(const FloatType& other) = default;
    FloatType& operator=(FloatType&& other) = default;

public:
    static std::shared_ptr<FloatType> get(int sz = 64);

    virtual bool isFloat() const override { return true; }

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;

    virtual size_t getSize() const override;

    virtual void setSize(size_t sz)  override { size = sz; }

    virtual QString getCtype(bool final = false) const override;

    virtual QString getTempName() const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;

private:
    size_t size; // Size in bits, e.g. 64
};
