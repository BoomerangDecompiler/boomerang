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


/**
 * This class is for before type analysis. Typically, you have no info at all, or only know the size (e.g.
 * width of a register or memory transfer)
 */
class SizeType : public Type
{
public:
    SizeType();
    SizeType(unsigned sz);
    virtual ~SizeType();
    virtual SharedType clone() const override;

    static std::shared_ptr<SizeType> get(unsigned sz);

    static std::shared_ptr<SizeType> get();

    virtual bool operator==(const Type& other) const override;
    virtual bool operator<(const Type& other) const override;

    // Merge this SizeType with another type
    virtual SharedType mergeWith(SharedType other) const override;

    virtual size_t getSize() const override;

    virtual void setSize(size_t sz) override;
    virtual bool isSize() const override;
    virtual bool isComplete() override; // Basic type is unknown
    virtual QString getCtype(bool final = false) const override;
    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool) const override;

private:
    size_t size; // Size in bits, e.g. 16
};
