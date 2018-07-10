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

private:
    std::shared_ptr<ProcSet> decompile(ProcList *callStack);

private:
    UserProc *m_proc;
};
