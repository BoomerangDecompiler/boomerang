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

/// Win32Signature is for non-thiscall signatures: all parameters pushed
class Win32Signature : public Signature
{
public:
    explicit Win32Signature(const QString& name);
    explicit Win32Signature(Signature& old);
    virtual ~Win32Signature() override = default;

public:
    virtual std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;

    static bool qualified(UserProc *p, Signature& candidate);

    void addReturn(SharedType type, SharedExp e = nullptr) override;
    void addParameter(SharedType type, const QString& name = QString::null,
                      const SharedExp& e = nullptr, const QString& boundMax = "") override;
    virtual SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;
    virtual SharedExp getStackWildcard() const override;

    virtual int getStackRegister() const override { return 28; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc
    virtual void getLibraryDefines(StatementList& defs) override; // Set list of locations def'd by library calls

    virtual bool isPromoted()        const override { return true; }
    virtual Platform getPlatform()   const override { return Platform::PENTIUM; }
    virtual CallConv getConvention() const override { return CallConv::Pascal; }
};


/// Win32TcSignature is for "thiscall" signatures, i.e. those that have register ecx as the first parameter
/// Only needs to override a few member functions; the rest can inherit from Win32Signature
class Win32TcSignature : public Win32Signature
{
public:
    explicit Win32TcSignature(const QString& name);
    explicit Win32TcSignature(Signature& old);

public:
    virtual SharedExp getArgumentExp(int n) const override;
    virtual SharedExp getProven(SharedExp left) const override;

    virtual std::shared_ptr<Signature> clone() const override;

    virtual Platform getPlatform() const override { return Platform::PENTIUM; }
    virtual CallConv getConvention() const override { return CallConv::ThisCall; }
};

}
