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

public:
    virtual bool isPointer() const override { return true; }

    /// Set the pointer type of this pointer.
    /// E.g. for a pointer of type 'Foo *' the pointer type is 'Foo'
    void setPointsTo(SharedType p);

    /// \returns the type the pointer points to (e.g. returns void* for void **x)
    SharedType getPointsTo() { return points_to; }
    const SharedType getPointsTo() const { return points_to; }

    static std::shared_ptr<PointerType> get(SharedType t) { return std::make_shared<PointerType>(t); }

    /// \returns true if the type is void* (pointer can morph into any other pointer type)
    bool isVoidPointer() const;

    /// \returns the length of the pointer chain (e.g. returns 2 for void **x)
    int getPointerDepth() const;

    /// \returns the final type at the end of the pointer chain
    /// (e.g. returns void for void **x)
    SharedType getFinalPointsTo() const;

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;

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
