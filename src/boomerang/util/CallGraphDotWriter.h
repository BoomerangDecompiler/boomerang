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


#include <QString>


class Prog;


class CallGraphDotWriter
{
public:
    bool writeCallGraph(const Prog *prog, const QString& dstFileName = "callgraph.dot");
};
