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


#include "boomerang/util/Types.h"

#include <initializer_list>
#include <type_traits>


enum class Endian
{
    Invalid = -1,
    Little = 0,
    Big = 1
};


namespace Util
{

/// return a bit mask with exactly \p bitCount of the lowest bits set to 1.
/// (example: 16 -> 0xFFFF)
inline QWord getLowerBitMask(DWord bitCount)
{
    if (bitCount >= 64) {
        return 0xFFFFFFFFFFFFFFFF;
    }
    return (1ULL << (bitCount % (8 * sizeof(QWord)))) - 1ULL;
}


inline SWord swapEndian(SWord value)
{
    value = ((value << 8) & 0xFF00) | ((value >> 8) & 0x00FF);
    return value;
}


inline DWord swapEndian(DWord value)
{
    value = ((value << 16) & 0xFFFF0000) | ((value >> 16) & 0x0000FFFF);
    value = ((value <<  8) & 0xFF00FF00) | ((value >>  8) & 0x00FF00FF);
    return value;
}


inline QWord swapEndian(QWord value)
{
    value = ((value << 32) & 0xFFFFFFFF00000000ULL) | ((value >> 32) & 0x00000000FFFFFFFFULL);
    value = ((value << 16) & 0xFFFF0000FFFF0000ULL) | ((value >> 16) & 0x0000FFFF0000FFFFULL);
    value = ((value <<  8) & 0xFF00FF00FF00FF00ULL) | ((value >>  8) & 0x00FF00FF00FF00FFULL);
    return value;
}


/**
 * Normalize endianness of a value.
 * Swaps bytes of \p value if the endianness of the source,
 * indicated by \p srcEndian, is different from the endianness
 * of the host.
 */
SWord normEndian(SWord value, Endian srcEndian);
DWord normEndian(DWord value, Endian srcEndian);
QWord normEndian(QWord value, Endian srcEndian);

/// Read values, respecting endianness
/// \sa normEndian
Byte readByte(const void *src);
SWord readWord(const void *src, Endian srcEndian);
DWord readDWord(const void *src, Endian srcEndian);
QWord readQWord(const void *src, Endian srcEndian);


/// Write values to \p dst, respecting endianness
void writeByte(void *dst, Byte value);
void writeWord(void *dst, SWord value, Endian dstEndian);
void writeDWord(void *dst, DWord value, Endian dstEndian);
void writeQWord(void *dst, QWord value, Endian dstEndian);


/**
 * Sign-extend \p src into \a TgtType.
 * Example:
 *   signExtend<int>((unsigned char)0xFF) == -1
 *
 * \param src number to sign-extend
 * \param numSrcBits Number of Bits in the source type
 *        (Mainly to counter int-promotion in (blabla & 0xFF))
 */
template<typename TgtType = int, typename SrcType>
TgtType signExtend(const SrcType& src, std::size_t numSrcBits = 8 * sizeof(SrcType))
{
    static_assert(std::is_integral<SrcType>::value, "Source type must be an integer!");
    static_assert(std::is_integral<TgtType>::value && std::is_signed<TgtType>::value, "Target type must be a signed integer!");

    // size difference, in bits
    const int sizeDifference = 8 * sizeof(TgtType) - numSrcBits;
    return (static_cast<TgtType>(static_cast<TgtType>(src) << sizeDifference)) >> sizeDifference;
}


/**
 * Tests if \p data starts with a specific byte sequence.
 * \note The size of \p data must not be smaller than \p magic.
 */
bool testMagic(const Byte *data, const std::initializer_list<Byte>& magic);

}


/// Read 4 bytes in little endian order from the address x points to
/// \deprecated Use Util::readDWord instead
#define READ4_LE_P(x)   Util::readDWord((x),  Endian::Little)

/// Read x as a 4-byte value in little endian order
/// \deprecated Use Util::readDWord instead
#define READ4_LE(x)     Util::readDWord(&(x), Endian::Little)

/// Read x as a 2-byte value in little endian order
/// \deprecated Use Util::readWord instead
#define READ2_LE(x)     Util::readWord(&(x),  Endian::Little)

/// Read x as a 4-byte value in big endian order
/// \deprecated Use Util::readDWord instead
#define READ4_BE(x)     Util::readDWord(&(x), Endian::Big)

/// Read x as a 2-byte value in big endian order
/// \deprecated Use Util::readWord instead
#define READ2_BE(x)     Util::readWord(&(x),  Endian::Big)
