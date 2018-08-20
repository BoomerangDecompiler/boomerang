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
class BOOMERANG_API MIPSSignature : public Signature
{
public:
    explicit MIPSSignature(const QString &name);
    virtual ~MIPSSignature() override = default;

public:
    /// \copydoc Signature::clone
    virtual std::shared_ptr<Signature> clone() const override;

    static bool qualified(UserProc *p, Signature &);

    /// \copydoc Signature::addReturn
    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;

    /// \copydoc Signature::getArgumentExp
    virtual SharedExp getArgumentExp(int n) const override;

    /// \copydoc Signature::addParameter
    virtual void addParameter(const QString &name, const SharedExp &e,
                              SharedType type         = VoidType::get(),
                              const QString &boundMax = "") override;

    /// \copydoc Signature::getStackRegister
    virtual int getStackRegister() const override { return 29; }

    /// \copydoc Signature::getProven
    virtual SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPreserved
    virtual bool isPreserved(SharedExp e) const override;

    /// \copydoc Signature::getLibraryDefines
    virtual void getLibraryDefines(StatementList &defs) override;

    /// \copydoc Signature::isLocalOffsetNegative
    virtual bool isLocalOffsetNegative() const override { return false; }

    /// \copydoc Signature::isPromoted
    virtual bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    virtual CallConv getConvention() const override { return CallConv::C; }
};
}
}
