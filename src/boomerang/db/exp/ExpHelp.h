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



/**
 * A class for comparing Exp*s (comparing the actual expressions).
 * Type sensitive.
 */
struct lessExpStar
{
    bool operator()(const SharedConstExp& x, const SharedConstExp& y) const;
};


void printChild(const SharedExp& e, int ind);
