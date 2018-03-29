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

class SparcSignature : public Signature
{
public:
    explicit SparcSignature(const QString& name);
    explicit SparcSignature(Signature& old);
    virtual ~SparcSignature() override = default;

public:
    virtual std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;
    static bool qualified(UserProc *p, Signature&);

    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual void addParameter(SharedType type, const QString& name = QString::null,
                              const SharedExp& e = nullptr, const QString& boundMax = "") override;
    virtual SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;

    virtual int getStackRegister() const override { return 14; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc

    /// Return a list of locations defined by library calls
    virtual void getLibraryDefines(StatementList& defs) override;

    /// Stack offsets can be negative (inherited) or positive:
    virtual bool isLocalOffsetPositive() const override { return true; }

    /// An override for testing locals
    /// An override for the SPARC: [sp+0] .. [sp+88] are local variables (effectively), but [sp + >=92] are memory parameters
    virtual bool isAddrOfStackLocal(Prog *prog, const SharedExp& e) const override;

    virtual bool isPromoted() const override { return true; }
    virtual Platform getPlatform() const override { return Platform::SPARC; }
    virtual CallConv getConvention() const override { return CallConv::C; }
    virtual bool returnCompare(const Assignment& a, const Assignment& b) const override;
    virtual bool argumentCompare(const Assignment& a, const Assignment& b) const override;
};


class SparcLibSignature : public SparcSignature
{
public:
    explicit SparcLibSignature(const QString& name)
        : SparcSignature(name) {}

    virtual std::shared_ptr<Signature> clone() const override;
    virtual SharedExp getProven(SharedExp left) const override;
};

}
}
