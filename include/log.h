
#ifndef LOG_H
#define LOG_H

class Log 
{
public:
    Log() : printDec(true), printHex(false) { }
    virtual Log &operator<<(const char *str) = 0;
    typedef enum { dec, hex, endl } modifer;
    virtual Log &operator<<(modifer m);
protected:
    bool printDec;
    bool printHex;
};

#endif
