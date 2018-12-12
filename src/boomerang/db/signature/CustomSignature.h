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


class BOOMERANG_API CustomSignature : public Signature
{
public:
    CustomSignature(const QString &name);
    virtual ~CustomSignature() override = default;

public:
    /// \copydoc Signature::isPromoted
    virtual bool isPromoted() const override { return true; }

    /// \copydoc Signature::clone
    virtual std::shared_ptr<Signature> clone() const override;

    void setSP(int spReg);

    /// \copydoc Signature::getStackRegister
    virtual RegNum getStackRegister() const override { return m_spReg; }

protected:
    RegNum m_spReg;
};
