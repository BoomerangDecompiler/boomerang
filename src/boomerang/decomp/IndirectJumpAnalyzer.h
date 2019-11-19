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


class IRFragment;
class UserProc;


/**
 * Analyzes indirect jumps and calls.
 * This includes virtual calls and switch statements.
 */
class BOOMERANG_API IndirectJumpAnalyzer
{
public:
    /**
     * Analyzes compued jump or compued call BBs.
     * If the function needs to be re-decompiled because of a significant change
     * (e.g. new switch arms discovered), this function returns true.
     */
    bool decodeIndirectJmp(IRFragment *bb, UserProc *proc);

    /**
     * Called when a switch has been identified. Visits the destinations of the switch,
     * adds out edges to the BB, etc.
     *
     * \note    Used to be called as soon as a switch statement is discovered, but this causes
     * decoded but unanalyzed BBs (statements not numbered, locations not SSA renamed etc) to appear
     * in the CFG. This caused problems when there were nested switch statements. Now only called
     * when re-decoding a switch statement
     */
    void processSwitch(IRFragment *bb, UserProc *proc);

    /**
     * Find the number of cases for this switch statement. Assumes that there is a compare and
     * branch around the indirect branch.
     * \note fails test/sparc/switchAnd_cc because of the and instruction, and the compare that is
     * outside is not the compare for the upper bound. Note that you CAN have an and
     * and still have a test for an upper bound. So this needs tightening.
     *
     * TMN: It also needs to check for and handle the double indirect case; where there is one array
     * (of e.g. ubyte) that is indexed by the actual switch value, then the value from that array is
     * used as an index into the array of code pointers.
     */
    int findNumCases(const IRFragment *bb);

private:
    /// Analyze a basic block ending with a computed jump.
    bool analyzeCompJump(IRFragment *bb, UserProc *proc);

    /// Analyze a basic block ending with a computed call.
    bool analyzeCompCall(IRFragment *bb, UserProc *proc);
};
