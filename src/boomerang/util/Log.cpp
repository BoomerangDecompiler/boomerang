#include "Log.h"

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/RTL.h"
#include "boomerang/db/statements/Statement.h"
#include "boomerang/db/exp/Exp.h"

#include "boomerang/db/Managed.h"

#include <QTextStream>
#include <sstream>
#include <iostream>


static Log* g_logger = nullptr;

Log& Log::getOrCreateLog()
{
    if (!g_logger) {
        g_logger = new Log(LogLevel::Default);
        g_logger->addLogSink(new ConsoleLogSink());
        g_logger->addLogSink(new FileLogSink("boomerang.log"));

        g_logger->writeLogHeader();
        LOG_MSG("This is Boomerang " BOOMERANG_VERSION);
        LOG_MSG("Log initialized.");
    }

    return *g_logger;
}

QString Log::collectArg(const QString& msg, const Instruction *s)
{
    return msg.arg(s->prints());
}

QString Log::collectArg(const QString& msg, const SharedConstExp& e)
{
    return msg.arg(e->toString());
}

QString Log::collectArg(const QString& msg, const SharedType& ty)
{
    return msg.arg(ty->toString());
}

QString Log::collectArg(const QString& msg, const Printable& ty)
{
    return msg.arg(ty.toString());
}

QString Log::collectArg(const QString& msg, const RTL *r)
{
    return msg.arg(r->prints());
}

QString Log::collectArg(const QString& msg, int i)
{
    return msg.arg(i);
}

QString Log::collectArg(const QString& msg, size_t i)
{
    return msg.arg(i);
}

QString Log::collectArg(const QString& msg, char c)
{
    return msg.arg(c);
}

QString Log::collectArg(const QString& msg, double d)
{
    return msg.arg(d);
}

QString Log::collectArg(const QString& msg, Address a)
{
    return msg.arg(a.toString());
}

QString Log::collectArg(const QString& msg, const LocationSet *l)
{
    return msg.arg(l->prints());
}




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


void Log::writeLogHeader()
{
    this->write("Level | File                                    | Line | Message\n");
    this->write(QString(100, '=') + "\n");
}


Log::Log(LogLevel level)
    : m_level(level)
{
    const char* lastSrc = __FILE__;
    const char* p = lastSrc;

    while ((p = strstr(lastSrc+1, "src/")) != nullptr) {
        m_fileNameOffset = (p-lastSrc);
        lastSrc = p;
    }
}


Log::~Log()
{
    for (ILogSink*& sink : m_sinks) {
        delete sink;
    }
}


void Log::log(LogLevel level, const char* file, int line, const QString& msg)
{
    if (!canLog(level)) {
        return;
    }

    char prettyFile[40]; // truncated file name
    truncateFileName(prettyFile, 40, file);

    QString header = "%1 | %2 | %3 | %4\n";
    QString logLine = header.arg(levelToString(level)).arg(prettyFile).arg(line, 4).arg(msg);
    this->write(logLine);

    if (level == LogLevel::Fatal) {
        abort();
    }
}


Log& Log::addLogSink(ILogSink* s)
{
    assert(s != nullptr);
    if (std::find(m_sinks.begin(), m_sinks.end(), s) == m_sinks.end()) {
        m_sinks.push_back(s);
    }
    return *this;
}


Log& Log::removeLogSink(ILogSink* s)
{
    assert(s != nullptr);
    auto it = std::find(m_sinks.begin(), m_sinks.end(), s);
    if (it != m_sinks.end()) {
        m_sinks.erase(it);
        delete s;
    }
    return *this;
}


Log& Log::setLogLevel(LogLevel level) {
    m_level = level;
    return *this;
}


LogLevel Log::getLogLevel() const {
    return m_level;
}


bool Log::canLog(LogLevel level) const {
    return level <= m_level;
}


void Log::truncateFileName(char* dstBuffer, size_t dstCharacters, const char* fileName)
{
    assert(dstBuffer);
    assert(fileName);
    assert(strlen(fileName) > m_fileNameOffset);

    fileName += m_fileNameOffset;
    size_t len = strlen(fileName);
    strncpy(dstBuffer, fileName, dstCharacters);

    if (len < dstCharacters) {
        memset(dstBuffer + len, ' ', dstCharacters - len -1);
    }
    dstBuffer[dstCharacters -1] = 0;
}


void Log::write(const QString& msg)
{
    for (ILogSink* s : m_sinks) {
        s->write(msg);
    }
}


QString Log::levelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Fatal:
        return QString("Fatal");
    case LogLevel::Error:
        return QString("Error");
    case LogLevel::Warning:
        return QString("Warn ");
    default:
        return QString("Msg  ");
    }
}

