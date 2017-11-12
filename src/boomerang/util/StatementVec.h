#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include <vector>

class Statement;
class QTextStream;


class StatementVec
{
    std::vector<Statement *> svec; // For now, use use standard vector

public:
    typedef std::vector<Statement *>::iterator           iterator;
    typedef std::vector<Statement *>::reverse_iterator   reverse_iterator;

    size_t size() const { return svec.size(); } ///< Number of elements
    iterator begin() { return svec.begin(); }
    iterator end() { return svec.end(); }
    reverse_iterator rbegin() { return svec.rbegin(); }
    reverse_iterator rend() { return svec.rend(); }

    // Get/put at position idx (0 based)
    Statement *operator[](size_t idx) { return svec[idx]; }
    void putAt(int idx, Statement *s);
    iterator remove(iterator it);
    char *prints(); ///< Print to string (for debugging)
    void dump();    ///< Print to standard error for debugging

    // Print just the numbers to stream os
    void printNums(QTextStream& os);

    void clear() { svec.clear(); }
    bool operator==(const StatementVec& o) const ///< Compare if equal
    {
        return svec == o.svec;
    }

    bool operator<(const StatementVec& o) const ///< Compare if less
    {
        return svec < o.svec;
    }

    void append(Statement *s) { svec.push_back(s); }
    void erase(iterator it) { svec.erase(it); }
};

