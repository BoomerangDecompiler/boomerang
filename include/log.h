
#ifndef LOG_H
#define LOG_H

#include "types.h"

class Statement;
class Exp;
class LocationSet;

class Log 
{
public:
	Log() { }
	virtual Log &operator<<(const char *str) = 0;
	virtual Log &operator<<(Statement *s);
	virtual Log &operator<<(Exp *e);
	virtual Log &operator<<(int i);
	virtual Log &operator<<(char c);
	virtual Log &operator<<(double d);
	virtual Log &operator<<(ADDRESS a);
	virtual Log &operator<<(LocationSet *l);
	virtual ~Log() {};
};

class FileLogger : public Log {
public:
	FileLogger();		// Implemented in boomerang.cpp
	virtual Log &operator<<(const char *str) { 
		out << str << std::flush;  
		return *this; 
	}
	virtual ~FileLogger() {};
protected:
	std::ofstream out;
};

#endif
