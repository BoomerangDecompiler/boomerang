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
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySection.h"
#include "boomerang/db/binary/BinarySymbol.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/util/log/Log.h"


Global::Global(SharedType type, Address addr, const QString &name, Prog *prog)
    : m_type(type)
    , m_addr(addr)
    , m_name(name)
    , m_prog(prog)
{
    assert(type != nullptr);
    assert(addr != Address::INVALID);
}


bool Global::containsAddress(Address addr) const
{
    return addr == m_addr || Util::inRange(addr, m_addr, m_addr + getType()->getSizeInBytes());
}


SharedExp Global::getInitialValue() const
{
    const BinarySection *sect = m_prog->getSectionByAddr(m_addr);

    if (!sect || sect->isAddressBss(m_addr)) {
        // This global is in the BSS, so it can't be initialised
        // NOTE: this is not actually correct. at least for typing, BSS data can have a type
        // assigned
        // TODO: see what happens when we skip Bss check here
        return nullptr;
    }

    return readInitialValue(m_addr, m_type);
}


SharedExp Global::readInitialValue(Address uaddr, SharedType type) const
{
    const BinaryImage *image  = m_prog->getBinaryFile()->getImage();
    const BinarySection *sect = image->getSectionByAddr(uaddr);

    if (sect == nullptr) {
        return nullptr;
    }

    if (type->resolvesToPointer()) {
        Address initAddr = Address::INVALID;
        if (!image->readNativeAddr4(uaddr, initAddr) || initAddr.isZero()) {
            return Const::get(0);
        }

        const QString name = m_prog->getGlobalNameByAddr(initAddr);
        if (!name.isEmpty()) {
            // TODO: typecast?
            return Location::global(name, nullptr);
        }

        if (type->as<PointerType>()->getPointsTo()->resolvesToChar()) {
            const char *str = m_prog->getStringConstant(initAddr);

            if (str != nullptr) {
                return Const::get(str);
            }
        }
    }

    if (type->resolvesToCompound()) {
        std::shared_ptr<CompoundType> cty = type->as<CompoundType>();
        SharedExp top                     = Terminal::get(opNil);

        for (int i = cty->getNumMembers() - 1; i >= 0; i--) {
            Address addr     = uaddr + cty->getMemberOffsetByIdx(i) / 8;
            SharedType memTy = cty->getMemberTypeByIdx(i);
            SharedExp memVal = readInitialValue(addr, memTy);

            if (memVal == nullptr) {
                LOG_ERROR("Unable to read native address %1 as type %2", addr, memTy->getCtype());
                return nullptr;
            }

            top = Binary::get(opList, memVal, top);
        }

        return top;
    }

    if (type->resolvesToArray() && type->as<ArrayType>()->getBaseType()->resolvesToChar()) {
        const char *str = m_prog->getStringConstant(uaddr, true);

        if (str) {
            // Make a global string
            return Const::get(str);
        }
    }

    if (type->resolvesToArray()) {
        const int baseSize = type->as<ArrayType>()->getBaseType()->getSize() / 8;
        int numElements    = type->as<ArrayType>()->getLength();

        if ((numElements <= 0 || numElements == ARRAY_UNBOUNDED) && baseSize > 0) {
            // try to read number of elements from information
            // contained in the binary file
            QString symbolName = m_prog->getGlobalNameByAddr(uaddr);

            if (!symbolName.isEmpty()) {
                BinarySymbol *symbol = m_prog->getBinaryFile()->getSymbols()->findSymbolByName(
                    symbolName);
                numElements = (symbol ? symbol->getSize() : 0) / baseSize;
            }
        }

        // It makes no sense to read an array with unknown upper bound
        if (numElements <= 0 || numElements == ARRAY_UNBOUNDED) {
            return nullptr;
        }

        SharedExp top = Terminal::get(opNil);

        for (int i = numElements - 1; i >= 0; i--) {
            SharedExp elementVal = readInitialValue(uaddr + i * baseSize,
                                                    type->as<ArrayType>()->getBaseType());

            if (elementVal == nullptr) {
                return nullptr;
            }
            top = Binary::get(opList, elementVal, top);
        }

        return top;
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
        switch (size) {
        case 8: {
            Byte value = 0;
            if (!image->readNative1(uaddr, value)) {
                return nullptr;
            }

            return Const::get(value, IntegerType::get(size));
        }
        case 16: {
            SWord value = 0;
            if (!image->readNative2(uaddr, value)) {
                return nullptr;
            }

            return Const::get(value, IntegerType::get(size));
        }
        case 32: {
            DWord value = 0;
            if (!image->readNative4(uaddr, value)) {
                return nullptr;
            }

            return Const::get(value, IntegerType::get(size));
        }
        case 64: {
            QWord value = 0;
            if (!image->readNative8(uaddr, value)) {
                return nullptr;
            }

            return Const::get(value, IntegerType::get(size));
        }
        }
    }

    if (type->resolvesToFloat()) {
        switch (type->as<FloatType>()->getSize()) {
        case 32: {
            float val;
            return image->readNativeFloat4(uaddr, val) ? Const::get(val) : nullptr;
        }
        case 64: {
            double val;
            return image->readNativeFloat8(uaddr, val) ? Const::get(val) : nullptr;
        }
        }
    }

    return nullptr;
}


void Global::meetType(SharedType ty)
{
    bool ch = false;

    m_type = m_type->meetWith(ty, ch);
}


bool GlobalComparator::operator()(const std::shared_ptr<const Global> &g1,
                                  const std::shared_ptr<const Global> &g2) const
{
    Address addr1 = g1->getAddress();
    Address addr2 = g2->getAddress();

    if (addr1 == addr2) {
        return false;
    }
    else if (addr1 == Address::INVALID) {
        return true;
    }
    else if (addr2 == Address::INVALID) {
        return false;
    }
    else {
        return addr1 < addr2;
    }
}
