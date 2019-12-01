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


/**
 * This class is for before type analysis. Typically, you have no info at all,
 * or only know the size (e.g. width of a register or memory transfer)
 */
class BOOMERANG_API SizeType : public Type
{
public:
    SizeType();
    SizeType(Size sz);

    SizeType(const SizeType &other) = default;
    SizeType(SizeType &&other)      = default;

    ~SizeType() override;

    SizeType &operator=(const SizeType &other) = default;
    SizeType &operator=(SizeType &&other) = default;

public:
    static std::shared_ptr<SizeType> get();
    static std::shared_ptr<SizeType> get(Size sz);

    /// \copydoc Type::operator==
    bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    SharedType clone() const override;

    /// \copydoc Type::getSize
    Size getSize() const override;

    /// \copydoc Type::setSize
    void setSize(Size sz) override;

    /// \copydoc Type::isComplete
    bool isComplete() override;

    /// \copydoc Type::getCtype
    QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool) const override;

private:
    Size m_size; ///< Size in bits, e.g. 16
};
