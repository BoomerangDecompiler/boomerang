
#ifndef LOG_H
#define LOG_H

#include "types.h"
#include <fstream>

class Statement;
class Exp;
class LocationSet;
class RTL;

class Log 
{
public:
	Log() { }
	virtual Log &operator<<(const char *str) = 0;
	virtual Log &operator<<(Statement *s);
	virtual Log &operator<<(Exp *e);
	virtual Log &operator<<(RTL *r);
	virtual Log &operator<<(int i);
	virtual Log &operator<<(char c);
	virtual Log &operator<<(double d);
	virtual Log &operator<<(ADDRESS a);
	virtual Log &operator<<(LocationSet *l);
			Log &operator<<(std::string& s) {return operator<<(s.c_str());}
	virtual ~Log() {};
	virtual void tail();
};

class FileLogger : public Log {
protected:
	std::ofstream out;
public:
			FileLogger();		// Implemented in boomerang.cpp
	void	tail();
	virtual Log &operator<<(const char *str) { 
		out << str << std::flush;  
		return *this; 
	}
	virtual ~FileLogger() {};
};

// For older MSVC compilers
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
static std::ostream& operator<<(std::ostream& s, QWord val)
{
	char szTmp[42]; // overkill, but who counts
	sprintf(szTmp, "%I64u", val);
	s << szTmp;
	return s;
}
#endif

#endif
