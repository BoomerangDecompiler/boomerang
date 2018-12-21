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


class BOOMERANG_API VoidType : public Type
{
public:
    VoidType();
    VoidType(const VoidType &other) = default;
    VoidType(VoidType &&other)      = default;

    virtual ~VoidType() override;

    VoidType &operator=(const VoidType &other) = default;
    VoidType &operator=(VoidType &&other) = default;

public:
    virtual SharedType clone() const override;

    static std::shared_ptr<VoidType> get() { return std::make_shared<VoidType>(); }

    virtual bool operator==(const Type &other) const override;

    virtual bool operator<(const Type &other) const override;

    virtual size_t getSize() const override;

    /**
     * Return a string representing this type
     * \param        final if true, this is final output
     * \returns      Pointer to a constant string of char
     */
    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;
    virtual bool isCompatible(const Type &other, bool all) const override;
};
