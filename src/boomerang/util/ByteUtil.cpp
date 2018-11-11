#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ByteUtil.h"

#include <cassert>


namespace Util
{
SWord normEndian(SWord value, Endian srcEndian)
{
    constexpr Endian myEndian = static_cast<Endian>(BOOMERANG_BIG_ENDIAN);
    return (myEndian == srcEndian) ? value : swapEndian(value);
}


DWord normEndian(DWord value, Endian srcEndian)
{
    constexpr Endian myEndian = static_cast<Endian>(BOOMERANG_BIG_ENDIAN);
    return (myEndian == srcEndian) ? value : swapEndian(value);
}


QWord normEndian(QWord value, Endian srcEndian)
{
    constexpr Endian myEndian = static_cast<Endian>(BOOMERANG_BIG_ENDIAN);
    return (myEndian == srcEndian) ? value : swapEndian(value);
}


Byte readByte(const void *src)
{
    assert(src);
    return *reinterpret_cast<const Byte *>(src);
}


SWord readWord(const void *src, Endian srcEndian)
{
    assert(src);
    return normEndian(*reinterpret_cast<const SWord *>(src), srcEndian);
}


DWord readDWord(const void *src, Endian srcEndian)
{
    assert(src);
    return normEndian(*reinterpret_cast<const DWord *>(src), srcEndian);
}


QWord readQWord(const void *src, Endian srcEndian)
{
    assert(src);
    return normEndian(*reinterpret_cast<const QWord *>(src), srcEndian);
}


void writeByte(void *dst, Byte value)
{
    assert(dst);
    *reinterpret_cast<Byte *>(dst) = value;
}


void writeWord(void *dst, SWord value, Endian dstEndian)
{
    assert(dst);
    *reinterpret_cast<SWord *>(dst) = normEndian(value, dstEndian);
}


void writeDWord(void *dst, DWord value, Endian dstEndian)
{
    assert(dst);
    *reinterpret_cast<DWord *>(dst) = normEndian(value, dstEndian);
}


void writeQWord(void *dst, QWord value, Endian dstEndian)
{
    assert(dst);
    *reinterpret_cast<QWord *>(dst) = normEndian(value, dstEndian);
}


bool testMagic(const Byte *buf, const std::initializer_list<Byte> &magic)
{
    for (std::size_t i = 0; i < magic.size(); i++) {
        if (buf[i] != *(magic.begin() + i)) {
            return false;
        }
    }
    return true;
}
}
