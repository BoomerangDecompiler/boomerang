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


class BOOMERANG_API BooleanType : public Type
{
public:
    BooleanType();
    BooleanType(const BooleanType& other) = default;
    BooleanType(BooleanType&& other) = default;

    virtual ~BooleanType() override;

    BooleanType& operator=(const BooleanType& other) = default;
    BooleanType& operator=(BooleanType&& other) = default;

public:
    virtual bool isBoolean() const override { return true; }
    static std::shared_ptr<BooleanType> get() { return std::make_shared<BooleanType>(); }

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    virtual bool operator<(const Type& other) const override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool& changed, bool useHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};
