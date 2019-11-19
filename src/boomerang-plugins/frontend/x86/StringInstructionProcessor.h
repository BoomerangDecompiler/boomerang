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


#include <memory>


class IRFragment;
class BranchStatement;
class UserProc;
class RTL;


/**
 * Finds BasicBlocks that contain string instructions (e.g. rep movsd)
 * and processes them.
 */
class StringInstructionProcessor
{
public:
    StringInstructionProcessor(UserProc *proc);

public:
    /// \returns true iff any change.
    bool processStringInstructions();

private:
    /**
     * Split the given BB at the RTL given, and turn it into the BranchStatement given. Sort out all
     * the in and out edges.
     *
     *    bb -> +----+    +----+ <= bb
     *   Change | A  | to | A  |            where A or B could be empty. S is the string
     *          |    |    |    |            instruction (which will branch to itself and to the
     *          +----+    +----+            start of the next instruction, i.e. the start of B,
     *          | S  |      |               if B is non empty).
     *          +----+      V
     *          | B  |    +----+ < skipBB
     *          |    |    +-b1-+            \p skipBranch is just a branch for the skip part
     *          +----+      |   \___
     *                      V       \
     *                    +----+ < rptBB
     *                    | S' |  |  |      S' = S less the skip and repeat parts
     *                    +-b2-+  |  |      \p rptBranch is a branch for the repeat part
     *                      | \__/  /
     *                      V      /
     *                    +----+ < newBB
     *                    | B  |
     *                    |    |
     *                    +----+
     * S is an RTL with 6 statements representing one string instruction (so this function is highly
     * specialised for the job of replacing the %SKIP and %RPT parts of string instructions)
     */
    IRFragment *splitForBranch(IRFragment *bb, RTL *stringRTL,
                               std::shared_ptr<BranchStatement> skipBranch,
                               std::shared_ptr<BranchStatement> rptBranch);

private:
    UserProc *m_proc;
};
