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

class PPCSignature : public Signature
{
public:
    explicit PPCSignature(const QString& name);
    explicit PPCSignature(Signature& old);
    virtual ~PPCSignature() override = default;

public:
    virtual std::shared_ptr<Signature> clone() const override;

    static bool qualified(UserProc *p, Signature&);
    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    virtual SharedExp getArgumentExp(int n) const override;
    virtual void addParameter(const QString& name, const SharedExp& e,
                              SharedType type = VoidType::get(), const QString& boundMax = "") override;

    virtual int getStackRegister() const override { return 1; }
    virtual SharedExp getProven(SharedExp left) const override;
    virtual bool isPreserved(SharedExp e) const override;         // Return whether e is preserved by this proc
    virtual void getLibraryDefines(StatementList& defs) override; // Set list of locations def'd by library calls

    virtual bool isLocalOffsetPositive() const override { return true; }
    virtual bool isPromoted() const override { return true; }
    virtual CallConv getConvention() const override { return CallConv::C; }

    std::shared_ptr<Signature> promote(UserProc * /*p*/) override
    {
        // No promotions from here up, obvious idea would be c++ name mangling
        return shared_from_this();
    }
};

}
}
