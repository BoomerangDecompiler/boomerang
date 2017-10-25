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


class VoidType : public Type
{
public:
    VoidType();
    virtual ~VoidType() override;
    virtual bool isVoid() const override { return true; }

    virtual SharedType clone() const override;

    static std::shared_ptr<VoidType> get() { return std::make_shared<VoidType>(); }

    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    /**
     * \brief        Return a string representing this type
     * \param        final if true, this is final output
     * \returns      Pointer to a constant string of char
     */
    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};
