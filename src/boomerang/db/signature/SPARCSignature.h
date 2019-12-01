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
    ~SPARCSignature() override = default;

public:
    /// \copydoc Signature::clone
    std::shared_ptr<Signature> clone() const override;

    /// \copydoc Signature::operator==
    bool operator==(const Signature &other) const override;

    static bool qualified(UserProc *p, Signature &);

    /// \copydoc Signature::addReturn
    void addReturn(SharedType type, SharedExp e = nullptr) override;

    /// \copydoc Signature::addParameter
    virtual void addParameter(const QString &name, const SharedExp &e,
                              SharedType type         = VoidType::get(),
                              const QString &boundMax = "") override;

    /// \copydoc Signature::getArgumentExp
    SharedExp getArgumentExp(int n) const override;

    /// \copydoc Signature::promote
    std::shared_ptr<Signature> promote(UserProc *) override;

    /// \copydoc Signature::getStackRegister
    RegNum getStackRegister() const override { return REG_SPARC_SP; }

    /// \copydoc Signature::getProven
    SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPreserved
    bool isPreserved(SharedExp e) const override;

    /// \copydoc Signature::getLibraryDefines
    void getLibraryDefines(StatementList &defs) override;

    /// \copydoc Signature::isLocalOffsetPositive
    bool isLocalOffsetPositive() const override { return true; }

    /// \copydoc Signature::isAddrOfStackLocal
    ///
    /// An override for the SPARC: [sp+0] .. [sp+88] are local variables (effectively),
    /// but [sp + >=92] are memory parameters
    bool isAddrOfStackLocal(RegNum spIndex, const SharedConstExp &e) const override;

    /// \copydoc Signature::isPromoted
    bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    CallConv getConvention() const override { return CallConv::C; }

    /// \copydoc Signature::returnCompare
    bool returnCompare(const Assignment &a, const Assignment &b) const override;

    /// \copydoc Signature::argumentCompare
    bool argumentCompare(const Assignment &a, const Assignment &b) const override;
};


class SPARCLibSignature : public SPARCSignature
{
public:
    explicit SPARCLibSignature(const QString &name)
        : SPARCSignature(name)
    {
    }

    /// \copydoc SPARCSignature::clone
    std::shared_ptr<Signature> clone() const override;

    /// \copydoc SPARCSignature::getProven
    SharedExp getProven(SharedExp left) const override;
};

}
