/**
 * \file types.h
 * \brief Contains some often used basic type definitions
 */
#pragma once

#include <QObject>
#include <iosfwd>
#include <cstdint>
#include <cassert>

#include "boomerang/util/Util.h"

class QTextStream;

// Machine types
typedef uint8_t         Byte;  /*  8 bits */
typedef uint16_t        SWord; /* 16 bits */
typedef uint32_t        DWord; /* 32 bits */
typedef uint64_t        QWord; /* 64 bits */

#include "boomerang/util/Address.h"
