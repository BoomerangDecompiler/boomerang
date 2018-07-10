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


#include "boomerang/db/proc/UserProc.h"


class ProcDecompiler
{
public:
    ProcDecompiler(UserProc *proc);

public:
    void decompile();

public:
    /**
     * Begin the decompile process at this procedure
     * \param  path A list of pointers to procedures, representing the path from
     * the current entry point to the current procedure in the call graph. Pass an
     * empty set at the top level.
     */
    std::shared_ptr<ProcSet> decompile(ProcList *callStack);

private:
    UserProc *m_proc;
};
