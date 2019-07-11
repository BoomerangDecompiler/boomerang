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


namespace CallingConvention
{
/**
 * Win32Signature is for __stdcall calling convention: parameters pushed right-to-left,
 * return value in %eax register
 */
class BOOMERANG_API Win32Signature : public Signature
{
public:
    explicit Win32Signature(const QString &name);
    explicit Win32Signature(Signature &old);
    virtual ~Win32Signature() override = default;

public:
    /// \copydoc Signature::clone
    virtual std::shared_ptr<Signature> clone() const override;

    /// \copydoc Signature::operator==
    virtual bool operator==(const Signature &other) const override;

    static bool qualified(UserProc *p, Signature &candidate);

    /// \copydoc Signature::addReturn
    void addReturn(SharedType type, SharedExp e = nullptr) override;

    /// \copydoc Signature::addParameter
    void addParameter(const QString &name, const SharedExp &e, SharedType type = VoidType::get(),
                      const QString &boundMax = "") override;

    /// \copydoc Signature::getArgumentExp
    virtual SharedExp getArgumentExp(int n) const override;

    /// \copydoc Signature::promote
    virtual std::shared_ptr<Signature> promote(UserProc *) override;

    /// \copydoc Signature::getStackRegister
    virtual RegNum getStackRegister() const override { return REG_PENT_ESP; }

    /// \copydoc Signature::getProven
    virtual SharedExp getProven(SharedExp left) const override;

    /// \copydoc Signature::isPreserved
    virtual bool isPreserved(SharedExp e) const override;

    /// \copydoc Signature::getLibraryDefines
    virtual void getLibraryDefines(StatementList &defs) override;

    /// \copydoc Signature::isPromoted
    virtual bool isPromoted() const override { return true; }

    /// \copydoc Signature::getConvention
    virtual CallConv getConvention() const override { return CallConv::Pascal; }
};


/**
 * Win32TcSignature is for "thiscall" signatures, i.e. those that have register %ecx as the first
 * parameter. Only needs to override a few member functions; the rest can inherit from
 * Win32Signature
 */
class Win32TcSignature : public Win32Signature
{
public:
    explicit Win32TcSignature(const QString &name);
    explicit Win32TcSignature(Signature &old);

public:
    /// \copydoc Win32Signature::getArgumentExp
    virtual SharedExp getArgumentExp(int n) const override;

    /// \copydoc Win32Signature::getProven
    virtual SharedExp getProven(SharedExp left) const override;

    /// \copydoc Win32Signature::clone
    virtual std::shared_ptr<Signature> clone() const override;

    /// \copydoc Win32Signature::getConvention
    virtual CallConv getConvention() const override { return CallConv::ThisCall; }
};
}
