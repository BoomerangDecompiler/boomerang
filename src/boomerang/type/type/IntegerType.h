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

class IntegerType : public Type
{
public:
    explicit IntegerType(unsigned NumBits, int sign = 0)
        : Type(eInteger)
    {
        size       = NumBits;
        signedness = sign;
        // setSubclassData(NumBits);
    }

    static std::shared_ptr<IntegerType> get(unsigned NumBits, int sign = 0);

    virtual bool isInteger() const override { return true; }
    virtual bool isComplete()  override { return signedness != 0 && size != 0; }

    virtual SharedType clone() const override;

    /***************************************************************************/ /**
    * \brief        Equality comparsion.
    * \param        other - Type being compared to
    * \returns      *this == other
    ******************************************************************************/
    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;

    // FIXME: aren't mergeWith and meetWith really the same thing?
    // Merge this IntegerType with another
    virtual SharedType mergeWith(SharedType other) const override;
    virtual SharedExp match(SharedType pattern)  override;

    virtual size_t getSize() const override; // Get size in bits

    virtual void setSize(size_t sz)  override { size = sz; }
    // Is it signed? 0=unknown, pos=yes, neg = no
    bool isSigned() { return signedness >= 0; }   // True if not unsigned
    bool isUnsigned() { return signedness <= 0; } // True if not definately signed
    // A hint for signedness
    void bumpSigned(int sg) { signedness += sg; }
    // Set absolute signedness
    void setSigned(int sg) { signedness = sg; }
    // Get the signedness
    int getSignedness() const { return signedness; }

    // Get the C type as a string. If full, output comments re the lack of sign information (in IntegerTypes).
    virtual QString getCtype(bool final = false) const override;

    virtual QString getTempName() const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;

private:
    mutable size_t size;    ///< Size in bits, e.g. 16
    mutable int signedness; ///< pos=signed, neg=unsigned, 0=unknown or evenly matched
};
