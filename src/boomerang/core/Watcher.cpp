#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Watcher.h"



void IWatcher::onFunctionCreated(Function *)
{
}


void IWatcher::onFunctionRemoved(Function *)
{
}


void IWatcher::onSignatureUpdated(Function *)
{
}


void IWatcher::onInstructionDecoded(Address, int)
{
}


void IWatcher::onBadDecode(Address)
{
}


void IWatcher::onStartDecode(Address, int)
{
}


void IWatcher::onEndDecode()
{
}


void IWatcher::onFunctionDecoded(Function *, Address, Address, int)
{
}


void IWatcher::onStartDecompile(UserProc *)
{
}


void IWatcher::onProcStatusChange(UserProc *)
{
}


void IWatcher::onEndDecompile(UserProc *)
{
}


void IWatcher::onFunctionDiscovered(Function *)
{
}


void IWatcher::onDecompileInProgress(UserProc *)
{
}


void IWatcher::onDecompileDebugPoint(UserProc *, const char *)
{
}


void IWatcher::onDecompilationEnd()
{
}
