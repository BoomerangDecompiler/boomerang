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


class BasicBlock;
class UserProc;


/**
 * Analyzes indirect jumps and calls.
 * This includes virtual calls and switch statements.
 */
class BOOMERANG_API IndirectJumpAnalyzer
{
public:
    /// Find indirect jumps and calls
    /// Find any BBs of type COMPJUMP or COMPCALL. If found, analyse, and if possible decode extra
    /// code and return true
    bool decodeIndirectJmp(BasicBlock *bb, UserProc *proc);

    /**
     * Called when a switch has been identified. Visits the destinations of the switch,
     * adds out edges to the BB, etc.
     * \note    Used to be called as soon as a switch statement is discovered, but this causes
     * decoded but unanalysed BBs (statements not numbered, locations not SSA renamed etc) to appear
     * in the CFG. This caused problems when there were nested switch statements. Now only called
     * when re-decoding a switch statement
     * \param   proc Pointer to the UserProc object for this code
     */
    void processSwitch(BasicBlock *bb, UserProc *proc);

    /**
     * Find the number of cases for this switch statement. Assumes that there is a compare and
     * branch around the indirect branch. \note fails test/sparc/switchAnd_cc because of the and
     * instruction, and the compare that is outside is not the compare for the upper bound. Note
     * that you CAN have an and and still a test for an upper bound. So this needs tightening. TMN:
     * It also needs to check for and handle the double indirect case; where there is one array (of
     * e.g. ubyte) that is indexed by the actual switch value, then the value from that array is
     * used as an index into the array of code pointers.
     */
    int findNumCases(const BasicBlock *bb);

    bool analyzeCompJump(BasicBlock *bb, UserProc *proc);
    bool analyzeCompCall(BasicBlock *bb, UserProc *proc);
};
