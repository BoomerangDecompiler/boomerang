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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/passes/Pass.h"

#include <QMap>

#include <memory>


class Prog;


class BOOMERANG_API PassManager
{
public:
    PassManager();
    PassManager(const PassManager &) = delete;
    PassManager(PassManager &&)      = delete;

    ~PassManager();

    PassManager &operator=(const PassManager &) = delete;
    PassManager &operator=(PassManager &&) = delete;

    static PassManager *get();

public:
    /// \returns the pass of type \p passType
    IPass *getPass(PassID passID);

    /// Execute a single pass on \p proc.
    /// \returns true iff the pass updated \p proc
    bool executePass(IPass *pass, UserProc *proc);
    bool executePass(PassID passID, UserProc *proc);

private:
    void registerPass(PassID passType, std::unique_ptr<IPass> pass);

private:
    std::vector<std::unique_ptr<IPass>> m_passes;
};
