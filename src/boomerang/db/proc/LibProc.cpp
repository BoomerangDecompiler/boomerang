#include "LibProc.h"

#include "boomerang/db/Module.h"
#include "boomerang/db/Signature.h"
#include "boomerang/frontend/Frontend.h"


bool LibProc::isNoReturn() const
{
    return IFrontEnd::isNoReturnCallDest(getName()) || m_signature->isNoReturn();
}


LibProc::LibProc(Module *mod, const QString& name, Address uNative)
    : Function(uNative, nullptr, mod)
{
    m_signature = mod->getLibSignature(name);
}


QString LibProc::toString() const
{
    return QString("[LibProc %1@%2]").arg(this->getName(), this->getNativeAddress().toString());
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
