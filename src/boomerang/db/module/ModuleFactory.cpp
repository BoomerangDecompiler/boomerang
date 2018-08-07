#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ModuleFactory.h"

#include "boomerang/db/module/Class.h"


Module *DefaultModFactory::create(const QString& name, Prog *parent, DefaultFrontEnd *fe) const
{
    return new Module(name, parent, fe);
}


Module * ClassModFactory::create(const QString& name, Prog *parent, DefaultFrontEnd *fe) const
{
    return new Class(name, parent, fe);
}
