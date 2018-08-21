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


#include "boomerang/core/BoomerangAPI.h"


class IFrontEnd;
class Module;
class Prog;
class QString;


struct BOOMERANG_API IModuleFactory
{
    /// Creates a new module with name \p name.
    virtual Module *create(const QString &name, Prog *prog) const = 0;
};


struct BOOMERANG_API DefaultModFactory : public IModuleFactory
{
    Module *create(const QString &name, Prog *prog) const override;
};


struct ClassModFactory : public IModuleFactory
{
    Module *create(const QString &name, Prog *prog) const override;
};
