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
    Log(LogLevel level = LogLevel::Default);

    virtual ~Log();

public:
    static Log& getOrCreateLog();

    /**
     * Log a message to all log sinks.
     *
     * \param level Log level, see \ref LogLevel
     * \param file  Source file from which this function was called, usually __FILE__
     * \param line  Source line frim which this function was called, usually __LINE__
     * \param msg   Log message.
     */
    void log(LogLevel level, const char* file, int line, const QString& msg);

    /**
     * Log a message to all log sinks, replacing %1, %2, ... etc by \p args
     *
     * \param level Log level, see \ref LogLevel
     * \param file  Source file from which this function was called, usually __FILE__
     * \param line  Source line frim which this function was called, usually __LINE__
     * \param msg   Log message.
     * \param args  Arguments to replace in \p msg
     */
    template<typename... Args>
    void log(LogLevel level, const char* file, int line, const QString& msg, Args... args)
    {
        if (!canLog(level)) {
            return;
        }

        log(level, file, line, collectArgs(msg, args...));
    }

    /// \deprecated These functions are about to be removed
    virtual Log& operator<<(const QString& msg) { write(msg); return *this; }
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
    Log& addLogSink(ILogSink* s);

    Log& removeLogSink(ILogSink* s);

    Log& setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

private:
    /// Check if logging is allowed with level \p level
    bool canLog(LogLevel level) const;

    /// Write a header with column captions
    void writeLogHeader();

    /**
     * Pretty-print \p fileName to \p dstBuffer, filling up remaining space in \p dstBuffer with whitespace characters.
     *
     * \param dstBuffer Destination buffer to write to
     * \param dstCharacters Size (in characters) of the destination buffer
     * \param fileName file name to truncate
     */
    void truncateFileName(char* dstBuffer, size_t dstCharacters, const char* fileName);

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

    /**
     * Write the raw string \p msg to all log sinks.
     */
    void write(const QString& msg);

    /**
     * Given a log level, get the
     */
    QString levelToString(LogLevel level);

private:
    LogLevel m_level = LogLevel::Default;
    std::vector<ILogSink *> m_sinks;
    size_t m_fileNameOffset; ///< number of characters to chop off from __FILE__ to have a sensible file name
};

#define LOG Log::getOrCreateLog()

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
