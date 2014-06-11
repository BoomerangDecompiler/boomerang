#ifndef RANGEANALYSIS_H
#define RANGEANALYSIS_H
#include "Pass.h"
#include <map>
class Cfg;
class Function;
class Instruction;
class RangePrivateData;
class UserProc;
class RangeAnalysis : public FunctionPass
{
public:
    RangeAnalysis();
    bool runOnFunction(Function &F);
private:
    friend class rangeVisitor;
    void addJunctionStatements(Cfg &cfg);
    void clearRanges();
    RangePrivateData * RangeData;
    void logSuspectMemoryDefs(UserProc &UF);
};

#endif // RANGEANALYSIS_H
