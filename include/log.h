
#ifndef LOG_H
#define LOG_H

#include "types.h"

#include <QtCore/QString>
#include <memory>
#include <fstream>

class Instruction;
class Exp;
class LocationSet;
class RTL;
class Range;
class RangeMap;
class Type;
typedef std::shared_ptr<Type> SharedType;
class Log {
  public:
    Log() {}
    virtual Log &operator<<(const QString &s) = 0;
    virtual Log &operator<<(const Instruction *s);
    virtual Log &operator<<(const Exp *e);
    virtual Log &operator<<(const SharedType &ty);
    virtual Log &operator<<(const RTL *r);
    virtual Log &operator<<(const Range *r);
    virtual Log &operator<<(const Range &r);
    virtual Log &operator<<(const RangeMap &r);
    virtual Log &operator<<(int i);
    virtual Log &operator<<(size_t i);
    virtual Log &operator<<(char c);
    virtual Log &operator<<(double d);
    virtual Log &operator<<(ADDRESS a);
    virtual Log &operator<<(const LocationSet *l);
    virtual ~Log() {}
    virtual void tail();
};

class FileLogger : public Log {
  protected:
    std::ofstream out;

  public:
    FileLogger(); // Implemented in boomerang.cpp
    void tail();
    virtual Log &operator<<(const QString &str) {
        out << str.toStdString() << std::flush;
        return *this;
    }
    virtual ~FileLogger() {}
};
class SeparateLogger : public Log {
  protected:
    std::ofstream *out;

  public:
    SeparateLogger(const QString &); // Implemented in boomerang.cpp
    void tail();
    virtual Log &operator<<(const QString &str) {
        (*out) << str.toStdString() << std::flush;
        return *this;
    }
    virtual ~SeparateLogger() {
        out->close();
        out = nullptr;
    }
};
class NullLogger : public Log {
  public:
    virtual Log &operator<<(const QString & /*str*/) {
        // LOG_STREAM() << str;
        return *this;
    }
};

#endif
