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



class CustomSignature : public Signature
{
public:
    CustomSignature(const QString& name);
    virtual ~CustomSignature() override = default;

public:
    virtual bool isPromoted() const override { return true; }
    virtual std::shared_ptr<Signature> clone() const override;

    void setSP(int spReg);

    virtual int getStackRegister() const override { return m_spReg; }

protected:
    int m_spReg;
};

