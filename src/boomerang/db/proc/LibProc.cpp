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


#include "boomerang/db/module/Module.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/frontend/Frontend.h"


LibProc::LibProc(Address addr, const QString& name, Module *module)
    : Function(addr, nullptr, module)
{
    if (module) {
        m_signature = module->getLibSignature(name);
    }

    if (!m_signature) {
        m_signature = Signature::instantiate(Platform::GENERIC, CallConv::INVALID, name);
    }
}


bool LibProc::isLib() const
{
    return true;
}


bool LibProc::isNoReturn() const
{
    return IFrontEnd::isNoReturnCallDest(getName()) || m_signature->isNoReturn();
}


SharedExp LibProc::getProven(SharedExp left)
{
    // Just use the signature information (all we have, after all)
    return m_signature->getProven(left);
}


SharedExp LibProc::getPremised(SharedExp)
{
    return nullptr;
}


bool LibProc::isPreserved(SharedExp e)
{
    return m_signature->isPreserved(e);
}
