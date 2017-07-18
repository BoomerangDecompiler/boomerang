#pragma once

#include "boomerang/util/Address.h"

class Function;
class UserProc;

/// Virtual class to monitor the decompilation.
class Watcher
{
public:
    Watcher() {}
    virtual ~Watcher() {}

    virtual void alert_complete() {}
    virtual void alertNew(Function *) {}
    virtual void alertRemove(Function *) {}
    virtual void alertUpdateSignature(Function *) {}
    virtual void alertDecode(Address pc, int nBytes) { Q_UNUSED(pc); Q_UNUSED(nBytes); }
    virtual void alertBadDecode(Address pc) { Q_UNUSED(pc); }
    virtual void alertStartDecode(Address start, int nBytes) { Q_UNUSED(start); Q_UNUSED(nBytes); }
    virtual void alertEndDecode() {}
    virtual void alertDecode(Function *, Address pc, Address last, int nBytes) { Q_UNUSED(pc); Q_UNUSED(last); Q_UNUSED(nBytes); }
    virtual void alertStartDecompile(UserProc *) {}
    virtual void alertProcStatusChange(UserProc *) {}
    virtual void alertDecompileSSADepth(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertDecompileBeforePropagate(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertDecompileAfterPropagate(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertDecompileAfterRemoveStmts(UserProc *, int depth) { Q_UNUSED(depth); }
    virtual void alertEndDecompile(UserProc *) {}
    virtual void alert_load(Function *) {}
    virtual void alertConsidering(Function * parent, Function *) { Q_UNUSED(parent); }
    virtual void alertDecompiling(UserProc *) {}
    virtual void alertDecompileDebugPoint(UserProc *, const char * description) { Q_UNUSED(description); }
};
