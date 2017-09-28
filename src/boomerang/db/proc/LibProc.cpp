#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LibProc.h"


#include "boomerang/db/Module.h"
#include "boomerang/db/Signature.h"
#include "boomerang/frontend/Frontend.h"


LibProc::LibProc(Address addr, const QString& name, Module* module)
    : Function(addr, nullptr, module)
{
    m_signature = module->getLibSignature(name);
}


bool LibProc::isNoReturn() const
{
    return IFrontEnd::isNoReturnCallDest(getName()) || m_signature->isNoReturn();
}


QString LibProc::toString() const
{
    return QString("[LibProc %1@%2]").arg(this->getName(), this->getEntryAddress().toString());
}


SharedExp LibProc::getProven(SharedExp left)
{
    // Just use the signature information (all we have, after all)
    return m_signature->getProven(left);
}


bool LibProc::isPreserved(SharedExp e)
{
    return m_signature->isPreserved(e);
}
