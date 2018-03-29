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

class ST20Signature : public Signature
{
public:
    explicit ST20Signature(const QString& name);
    explicit ST20Signature(Signature& old);
    virtual ~ST20Signature() override = default;

public:
    std::shared_ptr<Signature> clone() const override;
    virtual bool operator==(const Signature& other) const override;
    static bool qualified(UserProc *p, Signature&);

    virtual void addReturn(SharedType type, SharedExp e = nullptr) override;
    void addParameter(SharedType type, const QString& name = QString::null,
                      const SharedExp& e = nullptr, const QString& boundMax = "") override;
    SharedExp getArgumentExp(int n) const override;

    virtual std::shared_ptr<Signature> promote(UserProc *) override;

    virtual int getStackRegister() const override { return 3; }
    virtual SharedExp getProven(SharedExp left) const override;

    virtual bool isPromoted() const override { return true; }

    // virtual bool isLocalOffsetPositive() {return true;}

    virtual Platform getPlatform() const override { return Platform::ST20; }
    virtual CallConv getConvention() const override { return CallConv::C; }
};
} // namespace StdC
} // namespace CallingConvention


