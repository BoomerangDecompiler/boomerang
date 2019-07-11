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


class BOOMERANG_API FloatType : public Type
{
public:
    explicit FloatType(Size numBits);

    FloatType(const FloatType &other) = default;
    FloatType(FloatType &&other)      = default;

    virtual ~FloatType() override;

    FloatType &operator=(const FloatType &other) = default;
    FloatType &operator=(FloatType &&other) = default;

public:
    static std::shared_ptr<FloatType> get(Size numBits);

    /// \copydoc Type::operator==
    virtual bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    virtual bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    virtual SharedType clone() const override;

    /// \copydoc Type::getSize
    virtual Size getSize() const override;

    /// \copydoc Type::setSize
    virtual void setSize(Size sz) override;

    /// \copydoc Type::getCtype
    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

protected:
    /// \copydoc Type::isCompatible
    virtual bool isCompatible(const Type &other, bool all) const override;

private:
    Size m_size; ///< Size in bits, e.g. 64
};
