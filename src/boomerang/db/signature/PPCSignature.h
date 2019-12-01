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
#include "boomerang/ssl/exp/Exp.h"


namespace CallingConvention::StdC
{
class BOOMERANG_API PPCSignature : public Signature
{
public:
    explicit PPCSignature(const QString &name);
    explicit PPCSignature(Signature &old);
    ~PPCSignature() override = default;

public:
    /// \copydoc Signature::clone
    std::shared_ptr<Signature> clone() const override;

    static bool qualified(UserProc *p, Signature &);

    /// \copydoc Signature::addReturn
    void addReturn(SharedType type, SharedExp e = nullptr) override;

    /// \copydoc Signature::getArgumentExp
    SharedExp getArgumentExp(int n) const override;

    /// \copydoc Signature::addParameter
    virtual void addParameter(const QString &name, const SharedExp &e,
                              SharedType type         = VoidType::get(),
                              const QString &boundMax = "") override;

    /// \copydoc Signature::getStackRegister
    RegNum getStackRegister() const override { return REG_PPC_G1; }

    /// \copydoc Signature::getProven
    SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPreserved
    bool isPreserved(SharedExp e) const override;

    /// \copydoc Signature::getLibraryDefines
    void getLibraryDefines(StatementList &defs) override;

    /// \copydoc Signature::isLocalOffsetPositive
    bool isLocalOffsetPositive() const override { return true; }

    /// \copydoc Signature::isPromoted
    bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    CallConv getConvention() const override { return CallConv::C; }

    /// \copydoc Signature::promote
    std::shared_ptr<Signature> promote(UserProc * /*p*/) override
    {
        // No promotions from here up, obvious idea would be c++ name mangling
        return shared_from_this();
    }
};

}
