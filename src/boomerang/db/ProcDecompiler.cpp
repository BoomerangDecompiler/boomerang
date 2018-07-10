#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProcDecompiler.h"


ProcDecompiler::ProcDecompiler(UserProc* proc)
    : m_proc(proc)
{
}


void ProcDecompiler::decompile()
{
    ProcList callStack;
    decompile(&callStack);
}


std::shared_ptr<ProcSet> ProcDecompiler::decompile(ProcList *callStack)
{
    return m_proc->decompile(*callStack);
}
