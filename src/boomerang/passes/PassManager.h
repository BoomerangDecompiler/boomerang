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


#include "boomerang/passes/PassGroup.h"
#include "boomerang/passes/Pass.h"

#include <QMap>

#include <memory>


class Prog;


class PassManager
{
public:
    PassManager();

    static PassManager *get();

public:
    /// Creates a pass group with name \p name and elements \p passes
    /// \returns true iff creation was successful.
    bool createPassGroup(const QString& name, const std::initializer_list<IPass *> &passes);

    /// \returns the pass of type \p passType
    IPass *getPass(PassID passID);

    /// Execute a single pass on \p proc.
    /// \returns true iff the pass updated \p proc
    bool executePass(IPass *pass, UserProc *proc);
    bool executePass(PassID passID, UserProc *proc);

    /// Execute all passes in the pass group with name \p name on \p proc.
    /// \returns true iff at least 1 pass updated \p proc
    bool executePassGroup(const QString& name, UserProc *proc);

private:
    void registerPass(PassID passType, std::unique_ptr<IPass> pass);

private:
    std::vector<std::unique_ptr<IPass>> m_passes;
    QMap<QString, PassGroup> m_passGroups;
};
