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


class FuncType : public Type
{
public:
    static std::shared_ptr<FuncType> get(const std::shared_ptr<Signature>& sig = nullptr) { return std::make_shared<FuncType>(sig); }
    FuncType(const std::shared_ptr<Signature>& sig = nullptr);
    virtual ~FuncType() override;
    virtual bool isFunc() const override { return true; }

    virtual SharedType clone() const override;

    Signature *getSignature() { return signature.get(); }
    void setSignature(std::shared_ptr<Signature>& sig) { signature = sig; }
    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    // Split the C type into return and parameter parts
    // As above, but split into the return and parameter parts
    void getReturnAndParam(QString& ret, QString& param);

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;

private:
    std::shared_ptr<Signature> signature;
};
