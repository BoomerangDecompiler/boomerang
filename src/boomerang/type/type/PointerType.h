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


class PointerType : public Type
{
public:
    PointerType(SharedType p);
    virtual ~PointerType() override;
    virtual bool isPointer() const override { return true; }

    /// Set the pointer type of this pointer.
    /// E.g. for a pointer of type 'Foo *' the pointer type is 'Foo'
    void setPointsTo(SharedType p);

    SharedType getPointsTo() { return points_to; }
    const SharedType getPointsTo() const { return points_to; }
    static std::shared_ptr<PointerType> get(SharedType t) { return std::make_shared<PointerType>(t); }
    static std::shared_ptr<PointerType> newPtrAlpha();

    // Note: alpha is therefore a "reserved name" for types
    bool pointsToAlpha() const;
    int pointerDepth() const;            // Return 2 for **x
    SharedType getFinalPointsTo() const; // Return x for **x

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual void setSize(size_t sz)  override
    {
        Q_UNUSED(sz);
        assert(sz == STD_SIZE);
    }

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;

private:
    SharedType points_to;
};
