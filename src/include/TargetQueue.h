#pragma once

#include "include/types.h"

#include <queue>
class Cfg;
class BasicBlock;
//! Put the target queue logic into this small class
class TargetQueue
{
	std::queue<ADDRESS> targets;

public:
	void visit(Cfg *pCfg, ADDRESS uNewAddr, BasicBlock *& pNewBB);
	void initial(ADDRESS uAddr);
	ADDRESS nextAddress(const Cfg& cfg);
	void dump();
}; // class TargetQueue
