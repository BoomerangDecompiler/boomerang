#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "StatementVec.h"


#include "boomerang/db/statements/Statement.h"
#include "boomerang/util/Util.h"


void StatementVec::putAt(int idx, Statement *s)
{
    if (idx >= (int)svec.size()) {
        svec.resize(idx + 1, nullptr);
    }

    svec[idx] = s;
}


StatementVec::iterator StatementVec::remove(iterator it)
{
    /*
     *  iterator oldoldit = it;
     *  iterator oldit = it;
     *  for (it++; it != svec.end(); it++, oldit++)
     * oldit = *it;
     *  svec.resize(svec.size()-1);
     *  return oldoldit;
     */
    return svec.erase(it);
}


char *StatementVec::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    for (Statement *it : svec) {
        ost << it << ",\t";
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void StatementVec::printNums(QTextStream& os)
{
    for (iterator it = svec.begin(); it != svec.end();) {
        if (*it) {
            (*it)->printNum(os);
        }
        else {
            os << "-"; // Special case for no definition
        }

        if (++it != svec.end()) {
            os << " ";
        }
    }
}
