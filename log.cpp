
#include "log.h"

Log &Log::operator<<(modifer m)
{
    switch(m) {
        case dec:
            printDec = true;
            printHex = false;
            break;
        case hex:
            printHex = true;
            printDec = false;
            break;
        case endl:
            return *this << "\n";
    }
    return *this;
}

