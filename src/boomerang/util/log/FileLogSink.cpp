#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FileLogSink.h"

#include <QDir>
#include <QFileInfo>
#include <QString>

#include <cassert>


FileLogSink::FileLogSink(const QString &filename, bool append)
    : m_logFile(filename)
{
    QIODevice::OpenMode openFlags = QFile::WriteOnly;
    if (append) {
        openFlags |= QFile::Append;
    }

    bool ok = m_logFile.open(openFlags);
    if (!ok) {
        // Parent directory might not exist. Create directories and try again.
        QFileInfo(m_logFile).dir().mkpath(".");
        ok = m_logFile.open(openFlags);

        Q_UNUSED(ok);
        assert(ok);
    }
}


FileLogSink::~FileLogSink()
{
    m_logFile.close();
}


void FileLogSink::write(const QString &s)
{
    m_logFile.write(qPrintable(s));
}


void FileLogSink::flush()
{
    m_logFile.flush();
}
