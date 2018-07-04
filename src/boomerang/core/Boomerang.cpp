#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Boomerang.h"


#include "boomerang/codegen/CCodeGenerator.h"
#include "boomerang/core/Project.h"
#include "boomerang/core/Watcher.h"
#include "boomerang/db/CFGCompressor.h"
#include "boomerang/db/binary/BinaryImage.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/frontend/Frontend.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/CFGDotWriter.h"

#include <ctime>


static Boomerang *g_boomerang;


Boomerang::Boomerang()
    : m_settings(new Settings)
{
}


Boomerang *Boomerang::get()
{
    if (!g_boomerang) {
        g_boomerang = new Boomerang();
    }

    return g_boomerang;
}


void Boomerang::destroy()
{
    delete g_boomerang;
    g_boomerang = nullptr;
}


void Boomerang::alertDecompileDebugPoint(UserProc *p, const char *description)
{
    for (IWatcher *elem : m_watchers) {
        elem->onDecompileDebugPoint(p, description);
    }
}


const char *Boomerang::getVersionStr()
{
    return BOOMERANG_VERSION;
}


void Boomerang::addWatcher(IWatcher* watcher)
{
    m_watchers.insert(watcher);
}


void Boomerang::alertFunctionCreated(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionCreated(function);
    }
}


void Boomerang::alertFunctionRemoved(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionRemoved(function);
    }
}


void Boomerang::alertSignatureUpdated(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onSignatureUpdated(function);
    }
}


void Boomerang::alertInstructionDecoded(Address pc, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->onInstructionDecoded(pc, numBytes);
    }
}


void Boomerang::alertBadDecode(Address pc)
{
    for (IWatcher *it : m_watchers) {
        it->onBadDecode(pc);
    }
}


void Boomerang::alertFunctionDecoded(Function *p, Address pc, Address last, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionDecoded(p, pc, last, numBytes);
    }
}


void Boomerang::alertStartDecode(Address start, int numBytes)
{
    for (IWatcher *it : m_watchers) {
        it->onStartDecode(start, numBytes);
    }
}


void Boomerang::alertEndDecode()
{
    for (IWatcher *it : m_watchers) {
        it->onEndDecode();
    }
}


void Boomerang::alertStartDecompile(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onStartDecompile(proc);
    }
}


void Boomerang::alertProcStatusChanged(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onProcStatusChange(proc);
    }
}


void Boomerang::alertEndDecompile(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onEndDecompile(proc);
    }
}


void Boomerang::alertDiscovered(Function* function)
{
    for (IWatcher *it : m_watchers) {
        it->onFunctionDiscovered(function);
    }
}


void Boomerang::alertDecompiling(UserProc* proc)
{
    for (IWatcher *it : m_watchers) {
        it->onDecompileInProgress(proc);
    }
}

void Boomerang::alertDecompilationEnd()
{
    for (IWatcher *w : m_watchers) {
        w->onDecompilationEnd();
    }
}
