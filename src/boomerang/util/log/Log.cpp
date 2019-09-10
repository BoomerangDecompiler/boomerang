#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Log.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Exp.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/ConsoleLogSink.h"
#include "boomerang/util/log/FileLogSink.h"

#include <QDir>
#include <QFileInfo>


static Log *g_log = nullptr;


Log::Log(LogLevel level)
    : m_fileNameOffset(0)
    , m_level(level)
{
    const char *lastSrc = __FILE__;
    const char *p;

    while ((p = strstr(lastSrc + 1, "src")) != nullptr) {
        m_fileNameOffset += (p - lastSrc);
        lastSrc = p;
    }
}


Log::~Log()
{
    flush();
}


Log &Log::getOrCreateLog()
{
    if (!g_log) {
        g_log = new Log(LogLevel::Default);
    }

    return *g_log;
}


void Log::flush()
{
    for (std::unique_ptr<ILogSink> &s : m_sinks) {
        s->flush();
    }
}


void Log::log(LogLevel level, const char *file, int line, const QString &msg)
{
    const QStringList msgLines = msg.split('\n');

    for (const QString &msgLine : msgLines) {
        if (canLog(level)) {
            logDirect(level, file, line, msgLine);
        }
    }

    flush();
}


void Log::logDirect(LogLevel level, const char *file, int line, const QString &msg)
{
    char prettyFile[40]; // truncated file name
    truncateFileName(prettyFile, 40, file);

    QString header  = "%1 | %2 | %3 | %4\n";
    QString logLine = header.arg(levelToString(level)).arg(prettyFile).arg(line, 4).arg(msg);
    this->write(logLine);

    if (level == LogLevel::Fatal) {
        abort();
    }
}


void Log::addLogSink(std::unique_ptr<ILogSink> s)
{
    assert(s != nullptr);

    if (std::find(m_sinks.begin(), m_sinks.end(), s) == m_sinks.end()) {
        m_sinks.push_back(std::move(s));
    }
}


void Log::addDefaultLogSinks(const QString &outputDir)
{
    addLogSink(std::make_unique<ConsoleLogSink>());

    QFileInfo fi(QDir(outputDir), "boomerang.log");
    addLogSink(std::make_unique<FileLogSink>(fi.absoluteFilePath()));

    writeLogHeader();
}


void Log::removeAllSinks()
{
    flush();

    m_sinks.clear();
}


Log &Log::setLogLevel(LogLevel level)
{
    m_level = level;
    return *this;
}


LogLevel Log::getLogLevel() const
{
    return m_level;
}


bool Log::canLog(LogLevel level) const
{
    return level <= m_level;
}


void Log::writeLogHeader()
{
    write("Level | File                                    | Line | Message\n");
    write(QString(100, '=') + "\n");

    logDirect(LogLevel::Message, __FILE__, __LINE__, "This is Boomerang " BOOMERANG_VERSION);
    logDirect(LogLevel::Message, __FILE__, __LINE__, "Log initialized.");
    logDirect(LogLevel::Message, __FILE__, __LINE__,
              "Log level is '" + levelToString(getLogLevel()) + "'.");
}


void Log::truncateFileName(char *dstBuffer, size_t dstCharacters, const char *fileName)
{
    assert(dstBuffer);
    assert(fileName);
    assert(strlen(fileName) > m_fileNameOffset);

    fileName += m_fileNameOffset;
    size_t len = strlen(fileName);
    strncpy(dstBuffer, fileName, dstCharacters);

    if (len < dstCharacters) {
        memset(dstBuffer + len, ' ', dstCharacters - len - 1);
    }

    dstBuffer[dstCharacters - 1] = 0;
}


template<>
QString Log::collectArg<Statement>(const QString &msg, const std::shared_ptr<Statement> &s)
{
    return msg.arg(s->toString());
}


QString Log::collectArg(const QString &msg, const SharedConstExp &e)
{
    QString tgt;
    OStream os(&tgt);
    os << e;
    return msg.arg(tgt);
}


QString Log::collectArg(const QString &msg, const SharedType &ty)
{
    return msg.arg(ty->toString());
}


QString Log::collectArg(const QString &msg, const Type &ty)
{
    return msg.arg(ty.toString());
}


QString Log::collectArg(const QString &msg, const RTL *r)
{
    return msg.arg(r->toString());
}


QString Log::collectArg(const QString &msg, Address a)
{
    return msg.arg(a.toString());
}


QString Log::collectArg(const QString &msg, const LocationSet *l)
{
    return msg.arg(l->toString());
}


void Log::write(const QString &msg)
{
    for (std::unique_ptr<ILogSink> &s : m_sinks) {
        s->write(msg);
    }
}


QString Log::levelToString(LogLevel level)
{
    switch (level) {
    case LogLevel::Fatal: return QString("Fatal");

    case LogLevel::Error: return QString("Error");

    case LogLevel::Warning: return QString("Warn ");

    default: return QString("Msg  ");
    }
}
