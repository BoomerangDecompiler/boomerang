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


#include "boomerang/core/BoomerangAPI.h"

#include <QString>
#include <QTextStream>

#include <memory>


class Prog;


using SharedConstExp = std::shared_ptr<const class Exp>;


namespace Util
{
/**
 * Escape strings properly for code generation.
 * Turns things like newline, return, tab into \n, \r, \t etc
 * \note Assumes a C or C++ back end
 */
QString BOOMERANG_API escapeStr(const QString& str);

QTextStream& alignStream(QTextStream& str, int align);


/// Check if \p value is in [\p rangeStart, \p rangeEnd)
template<class T, class U1, class U2>
bool inRange(const T& value, const U1& rangeStart, const U2& rangeEnd)
{
    return (value >= rangeStart) && (value < rangeEnd);
}


/// Check if a value is in a container
template<typename Cont, typename T>
bool isContained(const Cont& cont, const T& value)
{
    return std::find(cont.begin(), cont.end(), value) != cont.end();
}


/// Basically the same as C++14's std::make_unique
/// that is not available in C++11. Can be removed when
/// dropping support for compilers that are not C++14 compilant.
template<typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}


template<class Container>
void clone(const Container& from, Container& to)
{
    if (&from == &to) {
        return;
    }

    to.resize(from.size());

    for (typename Container::size_type i = 0; i < from.size(); i++) {
        to[i] = from[i]->clone();
    }
}


// From m[sp +- K] return K (or -K for subtract). sp could be subscripted with {-}
int getStackOffset(SharedConstExp e, int sp);


/**
 * Return the internal index of the stack register
 * of an architecture, or -1 if the architecture does not have a stack register.
 */
int getStackRegisterIndex(const Prog *prog);

}

#define DEBUG_BUFSIZE    0x10000 // Size of the debug print buffer (65 kiB)
extern char debug_buffer[DEBUG_BUFSIZE];
