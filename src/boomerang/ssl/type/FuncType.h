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


class Signature;


class BOOMERANG_API FuncType : public Type
{
public:
    explicit FuncType(const std::shared_ptr<Signature> &sig = nullptr);

    FuncType(const FuncType &other) = default;
    FuncType(FuncType &&other)      = default;

    ~FuncType() override;

    FuncType &operator=(const FuncType &other) = default;
    FuncType &operator=(FuncType &&other) = default;

public:
    static std::shared_ptr<FuncType> get(const std::shared_ptr<Signature> &sig = nullptr);

    ///\copydoc Type::operator==
    bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    SharedType clone() const override;

    /// \copydoc Type::getSize
    Size getSize() const override;

    /// \copydoc Type::getCtype
    QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

public:
    Signature *getSignature() { return m_signature.get(); }
    const Signature *getSignature() const { return m_signature.get(); }
    void setSignature(std::shared_ptr<Signature> &sig) { m_signature = sig; }

    /// Split the C type into return and parameter parts
    /// As above, but split into the return and parameter parts
    void getReturnAndParam(QString &ret, QString &param);

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool all) const override;

private:
    std::shared_ptr<Signature> m_signature;
};
