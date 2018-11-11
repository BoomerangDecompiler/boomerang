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


#include "boomerang/core/Watcher.h"

#include <set>


class Statement;


class MiniDebugger : public IWatcher
{
private:
    /// \copydoc IWatcher::onDecompileDebugPoint
    void onDecompileDebugPoint(UserProc *proc, const char *description) override;

private:
    /// This is a mini command line debugger.  Feel free to expand it.
    void miniDebugger(UserProc *p, const char *description);

private:
    std::set<Statement *> watches;
};
