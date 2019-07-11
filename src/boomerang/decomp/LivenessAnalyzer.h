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


#include "boomerang/util/LocationSet.h"

#include <unordered_map>


class BasicBlock;
class ConnectionGraph;
class UserProc;


class LivenessAnalyzer
{
public:
    LivenessAnalyzer() = default;

    // Liveness
    bool calcLiveness(BasicBlock *bb, ConnectionGraph &ig, UserProc *proc);

    /// Locations that are live at the end of this BB are the union of the locations that are live
    /// at the start of its successors. \p live gets all the livenesses,
    /// and phiLocs gets a subset of these, which are due to phi statements at the top of successors
    void getLiveOut(BasicBlock *bb, LocationSet &live, LocationSet &phiLocs);

private:
    ///< Set of locations live at BB start
    std::unordered_map<BasicBlock *, LocationSet> m_liveIn;
};
