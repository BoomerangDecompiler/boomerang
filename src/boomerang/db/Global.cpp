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


#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Const.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/Prog.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/util/Log.h"


Global::Global(SharedType type, Address addr, const QString& name, Prog *prog)
    : m_type(type)
    , m_addr(addr)
    , m_name(name)
    , m_prog(prog)
{
}


bool Global::containsAddress(Address addr) const
{
    return Util::inRange(addr, m_addr, m_addr + getType()->getSizeInBytes());
}


SharedExp Global::getInitialValue() const
{
    const BinarySection *sect = m_prog->getSectionByAddr(m_addr);

    // TODO: see what happens when we skip Bss check here
    if (sect && sect->isAddressBss(m_addr)) {
        // This global is in the BSS, so it can't be initialised
        // NOTE: this is not actually correct. at least for typing, BSS data can have a type assigned
        return nullptr;
    }

    if (sect == nullptr) {
        return nullptr;
    }

    return readInitialValue(m_addr, m_type);
}


SharedExp Global::readInitialValue(Address uaddr, SharedType type) const
{
    const BinaryImage *image = m_prog->getBinaryFile()->getImage();
    SharedExp            e   = nullptr;
    const BinarySection *sect = image->getSectionByAddr(uaddr);

    if (sect == nullptr) {
        return nullptr;
    }

    if (type->resolvesToPointer()) {
        Address init = Address(image->readNative4(uaddr));

        if (init.isZero()) {
            return Const::get(0);
        }

        QString name = m_prog->getGlobalName(init);

        if (!name.isEmpty()) {
            // TODO: typecast?
            return Location::global(name, nullptr);
        }

        if (type->as<PointerType>()->getPointsTo()->resolvesToChar()) {
            const char *str = m_prog->getStringConstant(init);

            if (str != nullptr) {
                return Const::get(str);
            }
        }
    }

    if (type->resolvesToCompound()) {
        std::shared_ptr<CompoundType> c = type->as<CompoundType>();
        auto n = e = Terminal::get(opNil);

        for (unsigned int i = 0; i < c->getNumTypes(); i++) {
            Address    addr = uaddr + c->getOffsetTo(i) / 8;
            SharedType t    = c->getTypeAtIdx(i);
            auto       v    = readInitialValue(addr, t);

            if (v == nullptr) {
                LOG_MSG("Unable to read native address %1 as type %2", addr, t->getCtype());
                v = Const::get(-1);
            }

            if (n->isNil()) {
                n = Binary::get(opList, v, n);
                e = n;
            }
            else {
                assert(n->getSubExp2()->getOper() == opNil);
                n->setSubExp2(Binary::get(opList, v, n->getSubExp2()));
                n = n->getSubExp2();
            }
        }

        return e;
    }

    if (type->resolvesToArray() && type->as<ArrayType>()->getBaseType()->resolvesToChar()) {
        const char *str = m_prog->getStringConstant(uaddr, true);

        if (str) {
            // Make a global string
            return Const::get(str);
        }
    }

    if (type->resolvesToArray()) {
        int  nelems  = -1;
        QString name     = m_prog->getGlobalName(uaddr);
        int     base_sz = type->as<ArrayType>()->getBaseType()->getSize() / 8;

        if (!name.isEmpty()) {
            auto symbol = m_prog->getBinaryFile()->getSymbols()->findSymbolByName(name);
            nelems = symbol ? symbol->getSize() : 0;
            assert(base_sz);
            nelems /= base_sz;
        }

        auto n = e = Terminal::get(opNil);

        for (int i = 0; i < nelems; i++) {
            auto v = readInitialValue(uaddr + i * base_sz, type->as<ArrayType>()->getBaseType());

            if (v == nullptr) {
                break;
            }

            if (n->isNil()) {
                n = Binary::get(opList, v, n);
                e = n;
            }
            else {
                assert(n->getSubExp2()->getOper() == opNil);
                n->setSubExp2(Binary::get(opList, v, n->getSubExp2()));
                n = n->getSubExp2();
            }

            // "null" terminated
            if ((nelems == -1) && v->isConst() && (v->access<Const>()->getInt() == 0)) {
                break;
            }
        }
    }

    if (type->resolvesToInteger() || type->resolvesToSize()) {
        int size;

        if (type->resolvesToInteger()) {
            size = type->as<IntegerType>()->getSize();
        }
        else {
            size = type->as<SizeType>()->getSize();
        }

        // Note: must respect endianness
        switch (size)
        {
        case 8:  return Const::get(image->readNative1(uaddr));
        case 16: return Const::get(image->readNative2(uaddr));
        case 32: return Const::get(image->readNative4(uaddr));
        case 64: return Const::get(image->readNative8(uaddr));
        }
    }

    if (!type->resolvesToFloat()) {
        return e;
    }

    switch (type->as<FloatType>()->getSize())
    {
        case 32: {
            float val;
            if (image->readNativeFloat4(uaddr, val)) {
                return Const::get(val);
            }
            return nullptr;
        }
        case 64: {
            double val;
            if (image->readNativeFloat8(uaddr, val)) {
                return Const::get(val);
            }
            return nullptr;
        }
    }

    return e;
}


QString Global::toString() const
{
    SharedExp init = getInitialValue();
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
