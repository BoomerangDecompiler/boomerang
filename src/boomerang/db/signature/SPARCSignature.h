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
class BOOMERANG_API SPARCSignature : public Signature
{
public:
    explicit SPARCSignature(const QString &name);
    explicit SPARCSignature(Signature &old);
    virtual ~SPARCSignature() override = default;

public:
    /// \copydoc Signature::clone
    virtual std::shared_ptr<Signature> clone() const override;

    /// \copydoc Signature::operator==
    virtual bool operator==(const Signature &other) const override;

    static bool qualified(UserProc *p, Signature &);

    /// \copydoc Signature::addReturn
    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;

    /// \copydoc Signature::addParameter
    virtual void addParameter(const QString &name, const SharedExp &e,
                              SharedType type         = VoidType::get(),
                              const QString &boundMax = "") override;

    /// \copydoc Signature::getArgumentExp
    virtual SharedExp getArgumentExp(int n) const override;

    /// \copydoc Signature::promote
    virtual std::shared_ptr<Signature> promote(UserProc *) override;

    /// \copydoc Signature::getStackRegister
    virtual RegNum getStackRegister() const override { return REG_SPARC_SP; }

    /// \copydoc Signature::getProven
    virtual SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPreserved
    virtual bool isPreserved(SharedExp e) const override;

    /// \copydoc Signature::getLibraryDefines
    virtual void getLibraryDefines(StatementList &defs) override;

    /// \copydoc Signature::isLocalOffsetPositive
    virtual bool isLocalOffsetPositive() const override { return true; }

    /// \copydoc Signature::isAddrOfStackLocal
    ///
    /// An override for the SPARC: [sp+0] .. [sp+88] are local variables (effectively),
    /// but [sp + >=92] are memory parameters
    virtual bool isAddrOfStackLocal(RegNum spIndex, const SharedConstExp &e) const override;

    /// \copydoc Signature::isPromoted
    virtual bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    virtual CallConv getConvention() const override { return CallConv::C; }

    /// \copydoc Signature::returnCompare
    virtual bool returnCompare(const Assignment &a, const Assignment &b) const override;

    /// \copydoc Signature::argumentCompare
    virtual bool argumentCompare(const Assignment &a, const Assignment &b) const override;
};


class SPARCLibSignature : public SPARCSignature
{
public:
    explicit SPARCLibSignature(const QString &name)
        : SPARCSignature(name)
    {
    }

    /// \copydoc SPARCSignature::clone
    virtual std::shared_ptr<Signature> clone() const override;

    /// \copydoc SPARCSignature::getProven
    virtual SharedExp getProven(SharedExp left) const override;
};

}
