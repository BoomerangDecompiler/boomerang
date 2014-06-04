#include "log.h"
#include "statement.h"
#include "rtl.h"
#include "exp.h"
#include "managed.h"

#include <QTextStream>
#include <sstream>
Log &Log::operator<<(const Instruction *s) {
    QString tgt;
    QTextStream st(&tgt);
    s->print(st);
    *this << tgt;
    return *this;
}

Log &Log::operator<<(const Exp *e) {
    QString tgt;
    QTextStream st(&tgt);
    e->print(st);
    *this << tgt;
    return *this;
}

Log &Log::operator<<(const Type *ty) {
    std::ostringstream st;
    st << ty;
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(const Range *r) {
    QString tgt;
    QTextStream st(&tgt);
    r->print(st);
    *this << tgt;
    return *this;
}

Log &Log::operator<<(const Range &r) {
    QString tgt;
    QTextStream st(&tgt);
    r.print(st);
    *this << tgt;
    return *this;
}

Log &Log::operator<<(const RangeMap &r) {
    QString tgt;
    QTextStream st(&tgt);
    r.print(st);
    *this << tgt;
    return *this;
}

Log &Log::operator<<(const RTL *r) {
    QString tgt;
    QTextStream st(&tgt);
    r->print(st);
    *this << tgt;
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
