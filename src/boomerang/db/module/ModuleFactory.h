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


class IFrontEnd;
class Module;
class Prog;
class QString;


struct IModuleFactory
{
    virtual Module *create(const QString& name, Prog *parent, IFrontEnd *fe) const = 0;
};


struct DefaultModFactory : public IModuleFactory
{
    Module *create(const QString& name, Prog *parent, IFrontEnd *fe) const override;
};


struct ClassModFactory : public IModuleFactory
{
    Module *create(const QString& name, Prog *parent, IFrontEnd *fe) const override;
};
