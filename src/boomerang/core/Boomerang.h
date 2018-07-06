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


#include "boomerang/core/Settings.h"

#include <memory>
#include <set>


class QString;
class Function;
class UserProc;
class Project;
class IWatcher;


/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 * This is the main class of the decompiler.
 */
class Boomerang
{
private:
    /**
     * Initializes the Boomerang object.
     * The default settings are:
     * - All options disabled
     * - Infinite propagations
     * - A maximum memory depth of 99
     * - The path to the executable is "./"
     * - The output directory is "./output/"
     * - Main log stream is output on stderr
     */
    Boomerang();
    Boomerang(const Boomerang& other) = delete;
    Boomerang(Boomerang&& other) = default;

    virtual ~Boomerang() = default;

    Boomerang& operator=(const Boomerang& other) = delete;
    Boomerang& operator=(Boomerang&& other) = default;

public:
    /// \returns The global boomerang object. It will be created if it does not already exist.
    static Boomerang *get();
    static void destroy();

    /// Register a watcher to receive events about the decompilation.
    /// Does NOT take ownership of the pointer.
    void addWatcher(IWatcher *watcher);

public:
    /// Called once after a function was created.
    void alertFunctionCreated(Function *function);

    /// Called once after a function was removed.
    void alertFunctionRemoved(Function *function);

    /// Called once after the function signature was updated.
    void alertSignatureUpdated(Function *function);

    /// Called once on decode start.
    void alertStartDecode(Address start, int numBytes);

    /// Called every time an instruction is decoded.
    /// \param numBytes size of the instruction
    void alertInstructionDecoded(Address pc, int numBytes);

    /// Called every time an invalid or unrecognized instruction is encountered.
    void alertBadDecode(Address pc);

    /// Called every time a function was decoded completely.
    void alertFunctionDecoded(Function *function, Address pc, Address last, int numBytes);

    /// Called once on decode end.
    void alertEndDecode();

    /// Called once for every function on decompilation start (before earlyDecompile)
    void alertStartDecompile(UserProc *proc);

    /// Called every time the status of \p proc has changed.
    void alertProcStatusChanged(UserProc *proc);

    /// Called once for every completely decompiled proc \p proc.
    void alertEndDecompile(UserProc *proc);

    /// Called every time before middleDecompile is executed for \p function
    void alertDiscovered(Function *function);

    /// Called during the decompilation process when resuming decompilation of this proc.
    void alertDecompiling(UserProc *proc);

    void alertDecompileDebugPoint(UserProc *p, const char *description);

    /// Called once on decompilation end.
    void alertDecompilationEnd();

public:
    std::unique_ptr<Project> m_currentProject;

    std::set<IWatcher *> m_watchers;        ///< The watchers which are interested in this decompilation.
};
