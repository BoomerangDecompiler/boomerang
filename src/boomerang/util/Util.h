#pragma once

#include <QString>

#include "boomerang/util/types.h"

class Printable
{
public:
    virtual ~Printable() {}
    virtual QString toString() const = 0;
};


namespace Util
{

/**
 * Escape strings properly for code generation.
 */
QString escapeStr(const QString& str);

/// return a bit mask with exactly \p bitCount of the lowest bits set to 1.
/// (example: 16 -> 0xFFFF)
inline QWord getLowerBitMask(DWord bitCount)
{
    return (1ULL << (bitCount % (8*sizeof(QWord)))) - 1ULL;
}

/// Check if \p value is in [\p rangeStart, \p rangeEnd)
template<class T, class U>
bool inRange(const T& value, const U& rangeStart, const U& rangeEnd)
{
    return (value >= rangeStart) && (value < rangeEnd);
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
 * indicated by \p srcBigEndian, is different from the endianness
 * of the host.
 */
SWord normEndian(SWord value, bool srcBigEndian);
DWord normEndian(DWord value, bool srcBigEndian);
QWord normEndian(QWord value, bool srcBigEndian);

/// Read values, respecting endianness
/// \sa normEndian
Byte readByte(const void* src);
SWord readWord(const void* src, bool srcBigEndian);
DWord readDWord(const void* src, bool srcBigEndian);
QWord readQWord(const void* src, bool srcBigEndian);


/// Write values to \p dst, respecting endianness
void writeByte(void* dst, Byte value);
void writeWord(void* dst, SWord value, bool dstBigEndian);
void writeDWord(void* dst, DWord value, bool dstBigEndian);
void writeQWord(void* dst, DWord value, bool dstBigEndian);

}

