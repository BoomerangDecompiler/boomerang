/*
 *$Revision$	// 1.6.6.1
 */
#include "log.h"
#include <sstream>
#include "statement.h"
#include "rtl.h"
#include "exp.h"
#include "managed.h"

Log &Log::operator<<(Statement *s)
{
	std::ostringstream st;
	s->print(st);
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(Exp *e)
{
	std::ostringstream st;
	e->print(st);
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(Type *ty)
{
	std::ostringstream st;
	st << ty;
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(Range *r)
{
	std::ostringstream st;
	r->print(st);
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(Range &r)
{
	std::ostringstream st;
	r.print(st);
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(RangeMap &r)
{
	std::ostringstream st;
	r.print(st);
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(RTL *r)
{
	std::ostringstream st;
	r->print(st);
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(LocationSet *l)
{
	std::ostringstream st;
	st << l;
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(int i)
{
	std::ostringstream st;
	st << std::dec << i;
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(unsigned i)
{
	std::ostringstream st;
	st << std::dec << i;
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(char c) {
	std::ostringstream st;
	st << c;
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(double d)
{
	std::ostringstream st;
	st << d;
	*this << st.str().c_str();
	return *this;
}

Log &Log::operator<<(ADDRESS a)
{
	std::ostringstream st;
	st << "0x" << std::hex << a;
	*this << st.str().c_str();
	return *this;
}

#if 0		// Mac OS/X and 64 bit machines possibly need this, but better to just cast the size_t to unsigned
Log &Log::operator<<(size_t s)
{
	std::ostringstream st;
	st << st;
	*this << st.str().c_str();
	return *this;
}
#endif

void Log::tail() {
}

void FileLogger::tail() {
	out.seekp(-200, std::ios::end);
	std::cerr << out;
}
