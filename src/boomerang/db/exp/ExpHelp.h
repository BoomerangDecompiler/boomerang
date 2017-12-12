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


#include <memory>

class Exp;
using SharedExp      = std::shared_ptr<Exp>;
using SharedConstExp = std::shared_ptr<const Exp>;
class Assign;
class Assignment;


/**
 * A class for comparing Exp*s (comparing the actual expressions).
 * Type sensitive.
 */
struct lessExpStar
{
    bool operator()(const SharedConstExp& x, const SharedConstExp& y) const;
};

/**
 * A class for comparing Exp*s (comparing the actual expressions).
 * Type insensitive.
 */
struct lessTI
{
    bool operator()(const SharedExp& x, const SharedExp& y) const;
};

/// Compare assignments by their left hand sides (only).
struct lessAssignment
{
    bool operator()(const Assignment *x, const Assignment *y) const;
};

// Repeat the above for Assigns; sometimes the #include ordering is such that the compiler doesn't know that an Assign
// is a subclass of Assignment
struct lessAssign
{
    bool operator()(const Assign *x, const Assign *y) const;
};

void printChild(const SharedExp& e, int ind);
