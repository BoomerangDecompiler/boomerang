#pragma once

#include <QString>
#include <QFile>

#include <memory>
#include <fstream>
#include <vector>

#include "Address.h"

class Instruction;
class Exp;
class LocationSet;
class RTL;
class Type;
class Address;

class Printable;

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

    virtual ~Log() {}

public:
    void log(LogLevel level, QString file, int line, const QString& msg)
    {
        if (!canLog(level)) {
            return;
        }

        file.truncate(40);
        QString header = "%1 | %2 | %3 | %4";
        QString logLine = header.arg(levelToString(level)).arg(file).arg(line).arg(msg);
        this->write(logLine);
    }

    template<typename Arg, typename... Args>
    void log(LogLevel level, const char* file, int line, const QString& msg, Arg arg, Args... args)
    {
        if (!canLog(level)) {
            return;
        }

        log(level, file, line, msg.arg(arg), args...);
    }

    template<typename... Args>
    void log(LogLevel level, const char* file, int line, const QString& msg, Address arg, Args... args)
    {
        log(level, file, line, msg, arg.toString(), args...);
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
        if (std::find(m_sinks.begin(), m_sinks.end(), s) == m_sinks.end()) {
            m_sinks.push_back(s);
        }
        return *this;
    }

    Log& removeLogSink(ILogSink* s)
    {
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
#define LOG_FATAL(...)    LOG.log(LogLevel::Fatal,    __FILE__, __LINE__, __VA_ARGS__); abort()
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
