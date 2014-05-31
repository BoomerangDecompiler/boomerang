#include "log.h"
#include <sstream>
#include "statement.h"
#include "rtl.h"
#include "exp.h"
#include "managed.h"

Log &Log::operator<<(const Statement *s) {
    std::ostringstream st;
    s->print(st);
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const Exp *e) {
    std::ostringstream st;
    e->print(st);
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const Type *ty) {
    std::ostringstream st;
    st << ty;
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const Range *r) {
    std::ostringstream st;
    r->print(st);
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const Range &r) {
    std::ostringstream st;
    r.print(st);
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const RangeMap &r) {
    std::ostringstream st;
    r.print(st);
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const RTL *r) {
    std::ostringstream st;
    r->print(st);
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const LocationSet *l) {
    std::ostringstream st;
    st << l;
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(int i) {
    *this << QString::number(i);
    return *this;
}

Log &Log::operator<<(size_t i) {
    *this << QString::number(i);
    return *this;
}

Log &Log::operator<<(char c) {
    *this << QString(c);
    return *this;
}

Log &Log::operator<<(double d) {
    *this << QString::number(d);
    return *this;
}

Log &Log::operator<<(ADDRESS a) {
    std::ostringstream st;
    st << "0x" << std::hex << a;
    *this << "0x" << QString::number(a.m_value, 16);
    return *this;
}

void Log::tail() {}

void FileLogger::tail() {
    out.seekp(-200, std::ios::end);
    std::cerr << out;
}
void SeparateLogger::tail() {
    out->seekp(-200, std::ios::end);
    std::cerr << out;
}
