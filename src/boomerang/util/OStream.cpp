#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "OStream.h"

#include <QFile>
#include <QSaveFile>
#include <QString>
#include <QTextStream>


OStream::OStream(QFile *tgt)
    : m_os(new QTextStream(tgt))
{
}


OStream::OStream(QString *tgt)
    : m_os(new QTextStream(tgt))
{
}


OStream::OStream(FILE *tgt)
    : m_os(new QTextStream(tgt))
{
}


OStream::OStream(QSaveFile *tgt)
    : m_os(new QTextStream(tgt))
{
}


OStream::~OStream()
{
    delete m_os;
}


OStream &OStream::operator<<(const QString &rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(const QTextStreamManipulator &rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(size_t rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(int rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(unsigned int rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(double rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(const char *rhs)
{
    *m_os << rhs;
    return *this;
}


OStream &OStream::operator<<(char rhs)
{
    *m_os << rhs;
    return *this;
}


void OStream::flush()
{
    m_os->flush();
}
