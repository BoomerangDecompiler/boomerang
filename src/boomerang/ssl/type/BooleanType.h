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
    BooleanType(const BooleanType &other) = default;
    BooleanType(BooleanType &&other)      = default;

    virtual ~BooleanType() override;

    BooleanType &operator=(const BooleanType &other) = default;
    BooleanType &operator=(BooleanType &&other) = default;

public:
    static std::shared_ptr<BooleanType> get() { return std::make_shared<BooleanType>(); }

    /// \copydoc Type::operator==
    virtual bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    virtual bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    virtual SharedType clone() const override;

    /// \copydoc Type::getSize
    virtual Size getSize() const override;

    /// \copydoc Type::getCtype
    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

protected:
    /// \copydoc Type::isCompatible
    virtual bool isCompatible(const Type &other, bool all) const override;
};
