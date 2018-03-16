#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Global.h"


#include "boomerang/db/Prog.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/exp/Exp.h"


Global::Global(SharedType type, Address addr, const QString& name, Prog *prog)
    : m_type(type)
    , m_addr(addr)
    , m_name(name)
    , m_program(prog)
{
}


bool Global::containsAddress(Address addr) const
{
    return Util::inRange(addr, m_addr, m_addr + getType()->getSizeInBytes());
}


SharedExp Global::getInitialValue(const Prog *prog) const
{
    const BinarySection *si = prog->getSectionByAddr(m_addr);

    // TODO: see what happens when we skip Bss check here
    if (si && si->isAddressBss(m_addr)) {
        // This global is in the BSS, so it can't be initialised
        // NOTE: this is not actually correct. at least for typing, BSS data can have a type assigned
        return nullptr;
    }

    if (si == nullptr) {
        return nullptr;
    }

    return prog->readNativeAs(m_addr, m_type);
}


QString Global::toString() const
{
    SharedExp init = getInitialValue(m_program);
    QString res  = QString("%1 %2 at %3 initial value %4")
                      .arg(m_type->toString())
                      .arg(m_name)
                      .arg(m_addr.toString())
                      .arg((init ? init->prints() : "<none>"));

    return res;
}


void Global::meetType(SharedType ty)
{
    bool ch = false;

    m_type = m_type->meetWith(ty, ch);
}
