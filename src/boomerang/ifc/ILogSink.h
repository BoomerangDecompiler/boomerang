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


#include "boomerang/core/BoomerangAPI.h"


class QString;


/**
 * Abstract base class for log sinks.
 */
class BOOMERANG_API ILogSink
{
public:
    virtual ~ILogSink() = default;

public:
    /// Write a string to the log.
    virtual void write(const QString &s) = 0;

    /// If the log sink is buffered, flush the underlying buffer
    /// to the target device. If the device is unbuffered, do nothing.
    virtual void flush() = 0;
};
