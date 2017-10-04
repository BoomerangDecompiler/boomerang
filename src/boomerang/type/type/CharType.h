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


class CharType : public Type
{
public:
    CharType();
    virtual ~CharType();
    virtual bool isChar() const override { return true; }

    virtual SharedType clone() const override;

    static std::shared_ptr<CharType> get() { return std::make_shared<CharType>(); }
    virtual bool operator==(const Type& other) const override;

    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};
