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


#include "boomerang/util/Address.h"
#include "boomerang/util/Types.h"

#include <memory>
#include <vector>


class ILogSink;
class Statement;
class Exp;
class LocationSet;
class RTL;
class Type;


using SharedType     = std::shared_ptr<Type>;
using SharedConstExp = std::shared_ptr<const Exp>;


/// Log level / verbosity.
enum class LogLevel
{
    Fatal    = 0,
    Error    = 1,
    Warning  = 2,
    Message  = 3,
    Default  = 3,
    Verbose1 = 4,
    Verbose2 = 5
};


/**
 * Class for logging messages, warnings and errors.
 * Logs can have multiple LogSinks to enable writing to multiple targets simultaneously.
 *
 * Log messages have different levels (see \ref LogLevel).
 * The default behavior is to omit verbose log messages from being logged;
 * this behavior can be overridden by calling \ref setLogLevel.
 */
class BOOMERANG_API Log
{
public:
    /// Create a log.
    /// \param level Default logging level.
    Log(LogLevel level = LogLevel::Default);
    Log(const Log &other) = delete;
    Log(Log &&)           = delete;

    virtual ~Log();

    Log &operator=(const Log &other) = delete;
    Log &operator=(Log &&other) = delete;

public:
    /**
     * Get or create the default log.
     * The default behavior is to log to stdout
     * and to the default log file "boomerang.log".
     */
    static Log &getOrCreateLog();

    /**
     * Log a message to all log sinks.
     *
     * \param level Log level, see \ref LogLevel
     * \param file  Source file from which this function was called, usually __FILE__
     * \param line  Source line frim which this function was called, usually __LINE__
     * \param msg   Log message.
     */
    void log(LogLevel level, const char *file, int line, const QString &msg);

    /// Same as \ref Log::log, but does not split multiline strings
    void logDirect(LogLevel level, const char *file, int line, const QString &msg);

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
    void log(LogLevel level, const char *file, int line, const QString &msg, Args... args)
    {
        if (!canLog(level)) {
            return;
        }

        log(level, file, line, collectArgs(msg, args...));
    }

    void flush();

    /// Add a log sink / target. Takes ownership of the pointer.
    void addLogSink(std::unique_ptr<ILogSink> s);
    void addDefaultLogSinks(const QString &outputDir);

    void removeAllSinks();

    Log &setLogLevel(LogLevel level);
    LogLevel getLogLevel() const;

private:
    /// Check if logging is allowed with level \p level
    bool canLog(LogLevel level) const;

    /// Write a header with column captions
    void writeLogHeader();

    /**
     * Pretty-print \p fileName to \p dstBuffer, filling up remaining space in \p dstBuffer with
     * whitespace characters.
     *
     * \param dstBuffer Destination buffer to write to
     * \param dstCharacters Size (in characters) of the destination buffer
     * \param fileName file name to truncate
     */
    void truncateFileName(char *dstBuffer, size_t dstCharacters, const char *fileName);

    /**
     * Replace format arguments in \p msg.
     * \sa QString::arg
     */
    template<typename T>
    QString collectArg(const QString &msg, const std::shared_ptr<T> &arg)
    {
        QString tgt;
        OStream os(&tgt);
        os << arg;
        return msg.arg(tgt);
    }

    QString collectArg(const QString &msg, const char *arg) { return msg.arg(arg); }
    QString collectArg(const QString &msg, const QString &arg) { return msg.arg(arg); }
    QString collectArg(const QString &msg, const SharedConstExp &e);
    QString collectArg(const QString &msg, const SharedType &ty);
    QString collectArg(const QString &msg, const Type &ty);
    QString collectArg(const QString &msg, const RTL *r);
    QString collectArg(const QString &msg, const LocationSet *l);
    QString collectArg(const QString &msg, Address addr);

    // Integral or floating point PODs
    template<typename Arg,
             typename std::enable_if<
                 std::is_integral<Arg>::value || std::is_floating_point<Arg>::value, int>::type = 0>
    QString collectArg(const QString &msg, Arg arg)
    {
        return msg.arg(arg);
    }

    template<typename Arg>
    QString collectArgs(const QString &msg, Arg arg)
    {
        return collectArg(msg, arg);
    }

    template<typename Arg, typename... Args>
    QString collectArgs(QString msg, Arg arg, Args... args)
    {
        return collectArgs(collectArg(msg, arg), args...);
    }

    /// Write the raw string \p msg to all log sinks.
    void write(const QString &msg);

    /// Given a log level, get the name of the log level as a string.
    QString levelToString(LogLevel level);

private:
    /**
     * number of characters to chop off from the beginning of__FILE__
     * to have a sensible file name
     */
    size_t m_fileNameOffset;
    LogLevel m_level = LogLevel::Default;
    std::vector<std::unique_ptr<ILogSink>> m_sinks;
};

template<>
QString Log::collectArg<Statement>(const QString &msg, const std::shared_ptr<Statement> &s);


/// Usage: LOG_ERROR("%1, we have a problem", "Houston");
#define LOG_FATAL(...) Log::getOrCreateLog().log(LogLevel::Fatal, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Log::getOrCreateLog().log(LogLevel::Error, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) Log::getOrCreateLog().log(LogLevel::Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_MSG(...) Log::getOrCreateLog().log(LogLevel::Default, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_VERBOSE(...)                                                                           \
    Log::getOrCreateLog().log(LogLevel::Verbose1, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_VERBOSE2(...)                                                                          \
    Log::getOrCreateLog().log(LogLevel::Verbose2, __FILE__, __LINE__, __VA_ARGS__)
