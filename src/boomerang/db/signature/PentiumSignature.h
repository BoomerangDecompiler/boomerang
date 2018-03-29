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

class PentiumSignature : public Signature
{
public:
    explicit PentiumSignature(const QString& name);
    explicit PentiumSignature(Signature& old);
    virtual ~PentiumSignature() override = default;

public:
    virtual std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;

    /// FIXME: This needs changing. Would like to check that pc=pc and sp=sp
    /// (or maybe sp=sp+4) for qualifying procs. Need work to get there
    static bool qualified(UserProc *p, Signature&);

    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual void addParameter(const QString& name, const SharedExp& e,
                              SharedType type = VoidType::get(), const QString& boundMax = "") override;
    virtual SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;

    virtual int getStackRegister() const override { return 28; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc

    /// Return a list of locations defined by library calls
    virtual void getLibraryDefines(StatementList& defs) override;

    virtual bool isPromoted() const override { return true; }
    virtual Platform getPlatform() const override { return Platform::PENTIUM; }
    virtual CallConv getConvention() const override { return CallConv::C; }
    virtual bool returnCompare(const Assignment& a, const Assignment& b) const override;
    virtual bool argumentCompare(const Assignment& a, const Assignment& b) const override;
};

}
}
