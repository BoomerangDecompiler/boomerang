
#include "log.h"
#include <sstream>
#include "statement.h"
#include "exp.h"
#include "managed.h"

Log &Log::operator<<(Statement *s)
{
    std::ostringstream st;
    st << s;
    *this << st.str().c_str();
    return *this;
}

Log &Log::operator<<(Exp *e)
{
    std::ostringstream st;
    st << e;
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

Log &Log::operator<<(ADDRESS a)
{
    std::ostringstream st;
    st << std::hex << a;
    *this << st.str().c_str();
    return *this;
}

