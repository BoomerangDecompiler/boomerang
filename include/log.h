
#ifndef LOG_H
#define LOG_H

#include <QString>
#include <memory>
#include <fstream>

class Instruction;
class Exp;
class LocationSet;
class RTL;
class Range;
class RangeMap;
class Type;
struct ADDRESS;

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
    virtual ~FileLogger() {}
    void tail()  override;
    Log &operator<<(const QString &str)  override;
};
class SeparateLogger : public Log {
protected:
    std::ofstream *out;

public:
    SeparateLogger(const QString &); // Implemented in boomerang.cpp
    virtual ~SeparateLogger();
    Log &operator<<(const QString &str) override;
    void tail() override;
};
class NullLogger : public Log {
public:
    virtual Log &operator<<(const QString & /*str*/) {
        return *this;
    }
};

#endif
