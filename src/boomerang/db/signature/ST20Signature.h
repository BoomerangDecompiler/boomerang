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


#include "boomerang/db/signature/Signature.h"


namespace CallingConvention
{
namespace StdC
{

class BOOMERANG_API ST20Signature : public Signature
{
public:
    explicit ST20Signature(const QString& name);
    explicit ST20Signature(Signature& old);
    virtual ~ST20Signature() override = default;

public:
    /// \copydoc Signature::clone
    std::shared_ptr<Signature> clone() const override;

    /// \copydoc Signature::operator==
    virtual bool operator==(const Signature& other) const override;

    static bool qualified(UserProc *p, Signature&);

    /// \copydoc Signature::addReturn
    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;

    /// \copydoc Signature::addParameter
    void addParameter(const QString& name, const SharedExp& e,
                      SharedType type = VoidType::get(), const QString& boundMax = "") override;

    /// \copydoc Signature::getArgumentExp
    SharedExp getArgumentExp(int n) const override;

    /// \copydoc Signature::promote
    virtual std::shared_ptr<Signature> promote(UserProc *) override;

    /// \copydoc Signature::getStackRegister
    virtual int getStackRegister() const override { return 3; }

    /// \copydoc Signature::getProven
    virtual SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPromoted
    virtual bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    virtual CallConv getConvention() const override { return CallConv::C; }
};
} // namespace StdC
} // namespace CallingConvention


