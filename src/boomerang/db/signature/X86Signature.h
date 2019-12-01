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


namespace CallingConvention::StdC
{
class BOOMERANG_API X86Signature : public Signature
{
public:
    explicit X86Signature(const QString &name);
    explicit X86Signature(Signature &old);
    ~X86Signature() override = default;

public:
    ///\copydoc Signature::clone
    std::shared_ptr<Signature> clone() const override;

    /// \copydoc Signature::operator==
    bool operator==(const Signature &other) const override;

    /// FIXME: This needs changing. Would like to check that pc=pc and sp=sp
    /// (or maybe sp=sp+4) for qualifying procs. Need work to get there
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
    RegNum getStackRegister() const override;

    /// \copydoc Signature::getProven
    SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPreserved
    bool isPreserved(SharedExp e) const override;

    /// \copydoc Signature::getLibraryDefines
    void getLibraryDefines(StatementList &defs) override;

    /// \copydoc Signature::isPromoted
    bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    CallConv getConvention() const override { return CallConv::C; }

    /// \copydoc Signature::returnCompare
    bool returnCompare(const Assignment &a, const Assignment &b) const override;

    /// \copydoc Signature::argumentCompare
    bool argumentCompare(const Assignment &a, const Assignment &b) const override;
};

}
