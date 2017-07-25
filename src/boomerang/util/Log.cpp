#include "Log.h"

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/RTL.h"
#include "boomerang/db/statements/Statement.h"
#include "boomerang/db/exp/Exp.h"

#include "boomerang/db/Managed.h"

#include <QTextStream>
#include <sstream>
#include <iostream>


SeparateLogger::SeparateLogger(const QString& v)
{
    static QMap<QString, int> versions;

    if (!versions.contains(v)) {
        versions[v] = 0;
    }

    QDir    outDir(Boomerang::get()->getOutputDirectory());
    QString full_path = outDir.absoluteFilePath(QString("%1_%2.log").arg(v).arg(versions[v]++, 2, 10, QChar('0')));
    out = new std::ofstream(full_path.toStdString());
}


Log& Log::operator<<(const Instruction *s)
{
    QString     tgt;
    QTextStream st(&tgt);

    s->print(st);
    *this << tgt;
    return *this;
}


Log& Log::operator<<(const SharedConstExp& e)
{
    QString     tgt;
    QTextStream st(&tgt);

    e->print(st);
    *this << tgt;
    return *this;
}


Log& Log::operator<<(const SharedType& ty)
{
    std::ostringstream st;
    st << ty;
    *this << st.str().c_str();
    return *this;
}


Log& Log::operator<<(const Printable& p)
{
    *this << p.toString();
    return *this;
}


Log& Log::operator<<(const RTL *r)
{
    QString     tgt;
    QTextStream st(&tgt);

    r->print(st);
    *this << tgt;
    return *this;
}


Log& Log::operator<<(const LocationSet *l)
{
    QString     tgt;
    QTextStream st(&tgt);

    st << l;
    *this << tgt;
    return *this;
}


Log& Log::operator<<(int i)
{
    *this << QString::number(i);
    return *this;
}


Log& Log::operator<<(size_t i)
{
    *this << QString::number(i);
    return *this;
}


Log& Log::operator<<(char c)
{
    *this << QString(c);
    return *this;
}


Log& Log::operator<<(double d)
{
    *this << QString::number(d);
    return *this;
}


Log& Log::operator<<(Address a)
{
    *this << a.toString();
    return *this;
}


Log& SeparateLogger::operator<<(const QString& str)
{
    (*out) << str.toStdString() << std::flush;
    return *this;
}


SeparateLogger::~SeparateLogger()
{
    out->close();
    out = nullptr;
}


void ConsoleLogSink::write(const QString& s)
{
    std::cout << qPrintable(s);
}


FileLogSink::FileLogSink(const QString& filename)
    : m_logFile(filename)
{
    m_logFile.open(QFile::WriteOnly);
}


FileLogSink::~FileLogSink()
{
    m_logFile.close();
}


void FileLogSink::write(const QString& s)
{
    m_logFile.write(qPrintable(s));
}


