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


class BOOMERANG_API IntegerType : public Type
{
public:
    explicit IntegerType(Size numBits, Sign sign = Sign::Unknown);

    IntegerType(const IntegerType &other) = default;
    IntegerType(IntegerType &&other)      = default;

    virtual ~IntegerType() override = default;

    IntegerType &operator=(const IntegerType &other) = default;
    IntegerType &operator=(IntegerType &&other) = default;

public:
    static std::shared_ptr<IntegerType> get(Size numBits, Sign sign = Sign::Unknown);

    /// \copydoc Type::operator==
    virtual bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    virtual bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    virtual SharedType clone() const override;

    /// \copydoc Type::isComplete
    virtual bool isComplete() override;

    /// \copydoc Type::getCtype
    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::getSize
    virtual Size getSize() const override;

    /// \copydoc Type::setSize
    virtual void setSize(Size sz) override { m_size = sz; }

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

public:
    /// \returns true if definitely signed
    bool isSigned() const { return m_sign > Sign::Unknown; }

    /// \returns true if definitely unsigned
    bool isUnsigned() const { return m_sign < Sign::Unknown; }

    /// \returns true if signedness is signed or unknown
    bool isMaybeSigned() const { return m_sign >= Sign::Unknown; }

    /// \returns true if signedness is unsigned or unknown
    bool isMaybeUnsigned() const { return m_sign <= Sign::Unknown; }

    /// \returns true if we don't know the sign yet
    bool isSignUnknown() const { return m_sign == Sign::Unknown; }

    /// A hint for signedness
    void hintAsSigned();
    void hintAsUnsigned();

    void setSignedness(Sign sign) { m_sign = sign; }
    Sign getSign() const { return m_sign; }

protected:
    /// \copydoc Type::isCompatible
    virtual bool isCompatible(const Type &other, bool all) const override;

private:
    Size m_size; ///< Size in bits, e.g. 16
    Sign m_sign;
};
