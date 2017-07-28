#pragma once

#include <QString>
#include <QFile>

#include <memory>
#include <fstream>
#include <vector>
#include <cassert>

#include "boomerang/util/Address.h"
#include "boomerang/util/Util.h"


class Instruction;
class Exp;
class LocationSet;
class RTL;
class Type;

using SharedType     = std::shared_ptr<Type>;
using SharedConstExp = std::shared_ptr<const Exp>;

/// Log level / verbosity.
enum class LogLevel
{
    Fatal = 0,
    Error = 1,
    Warning = 2,
    Message = 3,
    Default = 3,
    Verbose1 = 4,
    Verbose2 = 5
};


class ILogSink
{
public:
    virtual ~ILogSink() {}

    virtual void write(const QString& s) = 0;
};


class ConsoleLogSink : public ILogSink
{
public:
    virtual ~ConsoleLogSink() {}
    virtual void write(const QString& s);
};


class FileLogSink : public ILogSink
{
public:
    FileLogSink(const QString& filename);
    virtual ~FileLogSink();

    virtual void write(const QString& s);

private:
    QFile m_logFile;
};


class Log
{
public:
    Log(LogLevel level = LogLevel::Default)
        : m_level(level)
    {}

    virtual ~Log()
    {
        for (ILogSink*& sink : m_sinks) {
            delete sink;
        }
    }

public:
    void log(LogLevel level, const char* file, int line, const QString& msg)
    {
        if (!canLog(level)) {
            return;
        }

        char truncFile[40]; // truncated file name
        strncpy(truncFile, file, 40);

        QString header = "%1 | %2 | %3 | %4";
        QString logLine = header.arg(levelToString(level)).arg(truncFile).arg(line).arg(msg);
        this->write(logLine);

        if (level == LogLevel::Fatal) {
            abort();
        }
    }

    template<typename... Args>
    void log(LogLevel level, const char* file, int line, const QString& msg, Args... args)
    {
        if (!canLog(level)) {
            return;
        }

        log(level, file, line, collectArgs(msg, args...));
    }

    virtual Log& operator<<(const QString&)
    {
        return *this;
    }

    virtual Log& operator<<(const Instruction *s);
    virtual Log& operator<<(const SharedConstExp& e);
    virtual Log& operator<<(const SharedType& ty);
    virtual Log& operator<<(const Printable& ty);
    virtual Log& operator<<(const RTL *r);
    virtual Log& operator<<(int i);
    virtual Log& operator<<(size_t i);
    virtual Log& operator<<(char c);
    virtual Log& operator<<(double d);
    virtual Log& operator<<(Address a);
    virtual Log& operator<<(const LocationSet *l);

    /// Add a log sink / target. Takes ownership of the pointer.
    Log& addLogSink(ILogSink* s)
    {
        assert(s != nullptr);
        if (std::find(m_sinks.begin(), m_sinks.end(), s) == m_sinks.end()) {
            m_sinks.push_back(s);
        }
        return *this;
    }

    Log& removeLogSink(ILogSink* s)
    {
        assert(s != nullptr);
        auto it = std::find(m_sinks.begin(), m_sinks.end(), s);
        if (it != m_sinks.end()) {
            m_sinks.erase(it);
            delete s;
        }
        return *this;
    }

    Log& setLogLevel(LogLevel level) { m_level = level; return *this; }
    LogLevel getLogLevel() const { return m_level; }

private:
    bool canLog(LogLevel level) const { return level <= m_level; }

    template<typename T>
    QString collectArg(const QString& msg, const std::shared_ptr<T>& arg) { return msg.arg(arg->toString()); }
    QString collectArg(const QString& msg, const char* arg) { return msg.arg(arg); }
    QString collectArg(const QString& msg, const QString& arg) { return msg.arg(arg); }
    QString collectArg(const QString& msg, const Instruction *s);
    QString collectArg(const QString& msg, const SharedConstExp& e);
    QString collectArg(const QString& msg, const SharedType& ty);
    QString collectArg(const QString& msg, const Printable& ty);
    QString collectArg(const QString& msg, const RTL *r);
    QString collectArg(const QString& msg, int i);
    QString collectArg(const QString& msg, unsigned int arg) { return msg.arg(arg); }
    QString collectArg(const QString& msg, size_t i);
    QString collectArg(const QString& msg, char c);
    QString collectArg(const QString& msg, double d);
    QString collectArg(const QString& msg, Address a);
    QString collectArg(const QString& msg, const LocationSet *l);


    template<typename Arg>
    QString collectArgs(const QString& msg, Arg arg)
    {
        return collectArg(msg, arg);
    }


    template<typename Arg, typename... Args>
    QString collectArgs(QString msg, Arg arg, Args... args)
    {
        return collectArgs(collectArg(msg, arg), args...);
    }


    void write(const QString& msg)
    {
        for (ILogSink* s : m_sinks) {
            s->write(msg);
        }
    }


    QString levelToString(LogLevel level)
    {
        switch (level) {
            case LogLevel::Fatal:   return QString("Fatal");
            case LogLevel::Error:   return QString("Error");
            case LogLevel::Warning: return QString("Warn ");
            default:                return QString("Msg  ");
        }
    }

private:
    LogLevel m_level = LogLevel::Default;
    std::vector<ILogSink *> m_sinks;
};


/// Usage: LOG_ERROR("%1, we have a problem", "Houston");
#define LOG_FATAL(...)    LOG.log(LogLevel::Fatal,    __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...)    LOG.log(LogLevel::Error,    __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)     LOG.log(LogLevel::Warning,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_MSG(...)      LOG.log(LogLevel::Default,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_VERBOSE(...)  LOG.log(LogLevel::Verbose1, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_VERBOSE2(...) LOG.log(LogLevel::Verbose2, __FILE__, __LINE__, __VA_ARGS__)


class SeparateLogger : public Log
{
protected:
    std::ofstream *out;

public:
    SeparateLogger(const QString& filePath);
    SeparateLogger(const SeparateLogger&) {}

    virtual ~SeparateLogger();

    Log& operator<<(const QString& str) override;
};

class NullLogger : public Log
{
public:
    virtual Log& operator<<(const QString& /*str*/) override
    {
        return *this;
    }
};
