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

#include <set>


class Prog;
class UserProc;
class Cfg;

class QString;
class QTextStream;

typedef std::set<UserProc *> ProcSet;


/**
 * Writes the CFG of functions to a file in the Graphviz dot format.
 */
class BOOMERANG_API CfgDotWriter
{
public:
    /// Write the CFG of all procedures in the program.
    void writeCFG(const Prog *prog, const QString& filename);

    /// write the CFG of all procedures in \p procs to \p filename
    void writeCFG(const ProcSet& procs, const QString& filename);

private:
    void writeCFG(const Cfg *cfg, QTextStream& os);
};
