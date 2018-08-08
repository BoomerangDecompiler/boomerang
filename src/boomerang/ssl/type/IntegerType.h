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
    explicit IntegerType(unsigned NumBits, Sign sign = Sign::Unknown);
    IntegerType(const IntegerType& other) = default;
    IntegerType(IntegerType&& other) = default;

    virtual ~IntegerType() override = default;

    IntegerType& operator=(const IntegerType& other) = default;
    IntegerType& operator=(IntegerType&& other) = default;

public:
    static std::shared_ptr<IntegerType> get(unsigned NumBits, Sign sign = Sign::Unknown);

    virtual bool isInteger() const override { return true; }
    virtual bool isComplete()  override { return signedness != Sign::Unknown && size != 0; }

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;
    virtual bool operator<(const Type& other) const override;

    virtual size_t getSize() const override; // Get size in bits

    virtual void setSize(size_t sz)  override { size = sz; }

    bool isSigned()   const { return signedness > Sign::Unknown; } ///< \returns true if definitely signed
    bool isUnsigned() const { return signedness < Sign::Unknown; } ///< \returns true if definitely unsigned

    bool isMaybeSigned()   const { return signedness >= Sign::Unknown; } ///< \returns true if signedness is signed or unknown
    bool isMaybeUnsigned() const { return signedness <= Sign::Unknown; } ///< \returns true if signedness is unsigned or unknown

    bool isSignUnknown() const { return signedness == Sign::Unknown; }

    /// A hint for signedness
    void hintAsSigned()   { signedness = std::min((Sign)((int)signedness + 1), Sign::SignedStrong); }
    void hintAsUnsigned() { signedness = std::max((Sign)((int)signedness - 1), Sign::UnsignedStrong); }

    void setSignedness(Sign sign) { signedness = sign; }
    Sign getSign() const { return signedness; }

    // Get the C type as a string. If full, output comments re the lack of sign information (in IntegerTypes).
    virtual QString getCtype(bool final = false) const override;

    virtual QString getTempName() const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool& changed, bool useHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;

private:
    size_t size;    ///< Size in bits, e.g. 16
    Sign   signedness; ///< pos=signed, neg=unsigned, 0=unknown or evenly matched
};
