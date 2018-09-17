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
#include "boomerang/util/Types.h"

#include <cstdio>


class QFile;
class QSaveFile;
class QString;
class QTextStream;
class QTextStreamManipulator;


/**
 * Wrapper facade around QTextStream providing
 * only output functionality to a target.
 */
class BOOMERANG_API OStream
{
public:
    OStream(QFile *tgt);
    OStream(QString *tgt);
    OStream(FILE *tgt);
    OStream(QSaveFile *tgt);

    OStream(const OStream &) = delete;
    OStream(OStream &&)      = delete;

    ~OStream();

    OStream &operator=(const OStream &) = delete;
    OStream &operator=(OStream &&) = delete;

public:
    OStream &operator<<(const QString &rhs);
    OStream &operator<<(const QTextStreamManipulator &rhs);
    OStream &operator<<(uint64 rhs);
    OStream &operator<<(int rhs);
    OStream &operator<<(unsigned int rhs);
    OStream &operator<<(double rhs);
    OStream &operator<<(const char *rhs);
    OStream &operator<<(char rhs);

    /// \copydoc QTextStream::flush
    void flush();

private:
    QTextStream *m_os;
};
