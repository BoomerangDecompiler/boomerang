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

    ~BooleanType() override;

    BooleanType &operator=(const BooleanType &other) = default;
    BooleanType &operator=(BooleanType &&other) = default;

public:
    static std::shared_ptr<BooleanType> get() { return std::make_shared<BooleanType>(); }

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

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool all) const override;
};
