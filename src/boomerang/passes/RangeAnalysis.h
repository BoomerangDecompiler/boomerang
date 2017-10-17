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


#include "Pass.h"

class Cfg;
class Function;
class Statement;
class UserProc;

/***************************************************************************/ /**
 * \brief Detect and log possible buffer overflows
 ******************************************************************************/
class RangeAnalysis : public FunctionPass
{
public:
    RangeAnalysis();

    /// \brief Range analysis (for procedure).
    bool runOnFunction(Function& F) override;

private:
    friend class RangeVisitor;

    /***************************************************************************/ /**
     * \brief Add Junction statements
     *******************************************************************************/
    void addJunctionStatements(Cfg& cfg);
    void clearRanges();

    class RangePrivateData *RangeData;
    void logSuspectMemoryDefs(UserProc& UF);
};
