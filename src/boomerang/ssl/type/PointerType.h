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


class BOOMERANG_API PointerType : public Type
{
public:
    explicit PointerType(SharedType p);

    PointerType(const PointerType &other) = default;
    PointerType(PointerType &&other)      = default;

    ~PointerType() override;

    PointerType &operator=(const PointerType &other) = default;
    PointerType &operator=(PointerType &&other) = default;

public:
    static std::shared_ptr<PointerType> get(SharedType pointsTo);

    /// \copydoc Type::clone
    SharedType clone() const override;

public:
    /// \copydoc Type::operator==
    bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    bool operator<(const Type &other) const override;

public:
    /// \copydoc Type::getSize
    Size getSize() const override;

    /// \copydoc Type::setSize
    void setSize(Size sz) override;

    /// \copydoc Type::getCtype
    QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

public:
    /// Set the pointer type of this pointer.
    /// E.g. for a pointer of type 'Foo *' the pointer type is 'Foo'
    void setPointsTo(SharedType p);

    /// \returns the type the pointer points to (e.g. returns void* for void **x)
    SharedType getPointsTo() { return m_pointsTo; }
    const SharedType getPointsTo() const { return m_pointsTo; }

    /// \returns the final type at the end of the pointer chain
    /// (e.g. returns void for void **x)
    SharedType getFinalPointsTo() const;

    /// \returns true if the type is void* (pointer can morph into any other pointer type)
    bool isVoidPointer() const;

    /// \returns the length of the pointer chain (e.g. returns 2 for void **x)
    int getPointerDepth() const;

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool all) const override;

private:
    SharedType m_pointsTo;
};
