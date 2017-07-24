#pragma once

#include <QString>
#include <memory>
#include <fstream>


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


class Log
{
public:
    Log(LogLevel level = LogLevel::Default)
        : m_level(level)
    {}

    virtual ~Log() {}

public:
    virtual Log& operator<<(const QString& s) = 0;

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

    void setLogLevel(LogLevel level) { m_level = level; }
    LogLevel getLogLevel() const { return m_level; }

private:
    bool canLog(LogLevel level) const { return level <= m_level; }

private:
    LogLevel m_level = LogLevel::Default;
};

/// Sets the outputfile to be the file "log" in the default output directory.
class FileLogger : public Log
{
protected:
    std::ofstream out;

public:
    FileLogger();
    virtual ~FileLogger() {}
    Log& operator<<(const QString& str)  override;
};

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
