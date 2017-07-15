#pragma once

#include "Pass.h"
#include <map>
class Cfg;
class Function;
class Instruction;
struct RangePrivateData;

class UserProc;

/***************************************************************************/ /**
 * \brief Detect and log possible buffer overflows
 ******************************************************************************/
class RangeAnalysis : public FunctionPass
{
public:
    RangeAnalysis();
    virtual ~RangeAnalysis() {}

    /// \brief Range analysis (for procedure).
    bool runOnFunction(Function& F) override;

private:
    friend class RangeVisitor;

    /***************************************************************************/ /**
     * \brief Add Junction statements
     *******************************************************************************/
    void addJunctionStatements(Cfg& cfg);
    void clearRanges();

    RangePrivateData *RangeData;
    void logSuspectMemoryDefs(UserProc& UF);
};
