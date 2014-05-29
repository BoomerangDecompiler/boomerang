
#ifndef LOG_H
#define LOG_H

#include "types.h"
#include <fstream>
#include <QtCore/QString>

class Statement;
class Exp;
class LocationSet;
class RTL;
class Range;
class RangeMap;
class Type;

class Log {
public:
    Log() { }
    virtual Log &operator<<(const QString &s) = 0;
    virtual Log &operator<<(const Statement *s);
    virtual Log &operator<<(const Exp *e);
    virtual Log &operator<<(const Type *ty);
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
            FileLogger();        // Implemented in boomerang.cpp
    void    tail();
    virtual Log &operator<<(const QString& str) {
        out << str.toStdString() << std::flush;
        return *this;
    }
    virtual ~FileLogger() {}
};
class SeparateLogger : public Log {
protected:
    std::ofstream *out;
public:
            SeparateLogger(const QString &);        // Implemented in boomerang.cpp
    void    tail();
    virtual Log &operator<<(const QString& str) {
        (*out) << str.toStdString() << std::flush;
        return *this;
    }
    virtual ~SeparateLogger() { out->close(); out = nullptr;}
};
class NullLogger : public Log {
public:
    virtual Log &operator<<(const QString& /*str*/) {
        // std::cerr << str;
        return *this;
    }
};
// For older MSVC compilers
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static std::ostream& operator<<(std::ostream& s, QWord val) {
    char szTmp[42]; // overkill, but who counts
    sprintf(szTmp, "%I64u", val);
    s << szTmp;
    return s;
}
#endif

#endif
