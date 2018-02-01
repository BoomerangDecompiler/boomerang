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
    virtual void alert_complete() {}
    virtual void alertNew(Function *) {}
    virtual void alertRemove(Function *) {}
    virtual void alertUpdateSignature(Function *) {}
    virtual void alertDecode(Address pc, int numBytes) { Q_UNUSED(pc); Q_UNUSED(numBytes); }
    virtual void alertBadDecode(Address pc) { Q_UNUSED(pc); }
    virtual void alertStartDecode(Address start, int numBytes) { Q_UNUSED(start); Q_UNUSED(numBytes); }
    virtual void alertEndDecode() {}
    virtual void alertDecode(Function *, Address pc, Address last, int numBytes) { Q_UNUSED(pc); Q_UNUSED(last); Q_UNUSED(numBytes); }
    virtual void alertStartDecompile(UserProc *) {}
    virtual void alertProcStatusChange(UserProc *) {}
    virtual void alertDecompileSSADepth(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertDecompileBeforePropagate(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertDecompileAfterPropagate(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertDecompileAfterRemoveStmts(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertEndDecompile(UserProc *) {}
    virtual void alert_load(Function *) {}
    virtual void alertDiscovered(Function *caller, Function *function) { Q_UNUSED(caller); Q_UNUSED(function); }
    virtual void alertDecompiling(UserProc *) {}
    virtual void alertDecompileDebugPoint(UserProc *, const char *description) { Q_UNUSED(description); }
};
