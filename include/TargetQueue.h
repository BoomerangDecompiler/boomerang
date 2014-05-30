#pragma once
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <fstream>
#include "types.h"
#include "sigenum.h" // For enums platform and cc
class Cfg;
class BasicBlock;
//! Put the target queue logic into this small class
class TargetQueue {
    std::queue<ADDRESS> targets;

  public:
    void visit(Cfg *pCfg, ADDRESS uNewAddr, BasicBlock *&pNewBB);
    void initial(ADDRESS uAddr);
    ADDRESS nextAddress(const Cfg &cfg);
    void dump();

}; // class TargetQueue
