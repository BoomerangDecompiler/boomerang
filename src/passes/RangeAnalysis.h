#pragma once

#include "Pass.h"
#include <map>
class Cfg;
class Function;
class Instruction;
struct RangePrivateData;

class UserProc;
class RangeAnalysis : public FunctionPass
{
public:
	RangeAnalysis();
	bool runOnFunction(Function& F) override;

private:
	friend class rangeVisitor;
	void addJunctionStatements(Cfg& cfg);
	void clearRanges();

	RangePrivateData *RangeData;
	void logSuspectMemoryDefs(UserProc& UF);
};
