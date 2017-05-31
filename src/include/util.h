#pragma once
/***************************************************************************/ /**
  * \file       util.h
  * OVERVIEW:   Provides the definition for the miscellaneous bits and pieces
  *                 implemented in the util.so library
  ******************************************************************************/

#include <QString>
#include <string>

struct Printable {
    virtual QString toString() const = 0;
};

// was a workaround
#define STR(x) (char *)(x.str().c_str())

QString escapeStr(const QString &str);
