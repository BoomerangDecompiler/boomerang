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


#include "boomerang/ifc/ILogSink.h"

#include <QFile>


/**
 * Log sink for logging to a file.
 */
class FileLogSink : public ILogSink
{
public:
    FileLogSink(const QString &filename, bool append = false);
    virtual ~FileLogSink() override;

    /// \copydoc ILogSink::write
    virtual void write(const QString &s) override;

    /// \copydoc ILogSink::flush
    virtual void flush() override;

private:
    QFile m_logFile;
};
