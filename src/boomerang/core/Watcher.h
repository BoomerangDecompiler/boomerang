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


#include "boomerang/util/Address.h"


class Function;
class UserProc;


/// Virtual class to monitor the decompilation.
class IWatcher
{
public:
    IWatcher() = default;
    virtual ~IWatcher() = default;

public:
    /// Called once after a function was created.
    virtual void onFunctionCreated(Function *function);

    /// Called once after a function was removed.
    virtual void onFunctionRemoved(Function *function);

    /// Called once after the function signature was updated.
    virtual void onSignatureUpdated(Function *function);

    /// Called once on decode start.
    virtual void onStartDecode(Address start, int numBytes);

    /// Called every time an instruction is decoded.
    /// \param numBytes the size of the instruction.
    virtual void onInstructionDecoded(Address pc, int numBytes);

    /// Called every time a function was decoded completely.
    virtual void onFunctionDecoded(Function *function, Address pc, Address last, int numBytes);

    /// Called every time an invalid or unrecognized instruction is encountered.
    virtual void onBadDecode(Address pc);

    /// Called once on decode end.
    virtual void onEndDecode();

    /// Called once for every function on decompilation start (before earlyDecompile)
    virtual void onStartDecompile(UserProc *proc);

    /// Called every time the status of \p proc has changed.
    virtual void onProcStatusChange(UserProc *proc);

    /// Called once for every completely decompiled proc \p proc
    virtual void onEndDecompile(UserProc *proc);

    /// Called every time before middleDecompile is executed for \p function
    virtual void onFunctionDiscovered(Function *caller, Function *function);

    /// Called during the decompilation process when resuming decompilation of \p proc.
    virtual void onDecompileInProgress(UserProc *proc);

    /// Called when a decompilation breakpoint occurs.
    virtual void onDecompileDebugPoint(UserProc *proc, const char *description);

    /// Called once on decompilation end.
    virtual void onDecompilationEnd();
};
