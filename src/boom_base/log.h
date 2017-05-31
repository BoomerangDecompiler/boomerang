#pragma once

#include <QString>
#include <memory>
#include <fstream>

#include "boom_base/boomerang.h"

class Instruction;
class Exp;
class LocationSet;
class RTL;
class Type;
struct ADDRESS;
struct Printable;
using SharedType = std::shared_ptr<Type>;
using SharedConstExp = std::shared_ptr<const Exp>;
class Log {
public:
    Log() {}
    virtual Log &operator<<(const QString &s) = 0;
    virtual Log &operator<<(const Instruction *s);
    virtual Log &operator<<(const SharedConstExp &e);
    virtual Log &operator<<(const SharedType &ty);
    virtual Log &operator<<(const Printable &ty);
    virtual Log &operator<<(const RTL *r);
    virtual Log &operator<<(int i);
    virtual Log &operator<<(size_t i);
    virtual Log &operator<<(char c);
    virtual Log &operator<<(double d);
    virtual Log &operator<<(ADDRESS a);
    virtual Log &operator<<(const LocationSet *l);
    virtual ~Log() {}
};

class FileLogger : public Log {
protected:
    std::ofstream out;
public:
    FileLogger(); // Implemented in boomerang.cpp
    virtual ~FileLogger() {}
    Log &operator<<(const QString &str)  override;
};
class SeparateLogger : public Log {
protected:
    std::ofstream *out;

public:
    SeparateLogger(const QString &); // Implemented in boomerang.cpp
    virtual ~SeparateLogger();
    Log &operator<<(const QString &str) override;
};
class NullLogger : public Log {
public:
    virtual Log &operator<<(const QString & /*str*/) {
        return *this;
    }
};
