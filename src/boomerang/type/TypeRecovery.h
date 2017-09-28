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


#include <QString>

class Function;
class Prog;


struct ITypeRecovery
{
    virtual ~ITypeRecovery() {}
    virtual QString name() = 0;
    virtual void    recoverFunctionTypes(Function *) = 0;
    virtual void    recoverProgramTypes(Prog *)      = 0;
};


struct TypeRecoveryCommon : public ITypeRecovery
{
    virtual void recoverProgramTypes(Prog *v) override;
};
