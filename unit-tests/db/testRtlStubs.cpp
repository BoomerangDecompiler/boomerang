#include <iostream>
#include <string>
#include "boomerang/include/types.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/prog.h"
#include "boomerang/util/Log.h"
#include "boomerang/analysis.h"

#include "boomerang/typeStubs.cpp"
#include "boomerang/signatureStubs.cpp"

// Cfg
void Cfg::dominators(DOM *d)
{
}


void Cfg::placePhiFunctions(DOM *d, int memDepth)
{
}


void Cfg::renameBlockVars(DOM *d, int n, int memDepth)
{
}


// Misc
Boomerang::Boomerang()
{
}


Boomerang *Boomerang::boomerang = nullptr;
bool isSwitch(PBB pSwitchBB, Exp *pDest, UserProc *pProc, BinaryFile *pBF)
{
	return false;
}


void processSwitch(PBB pBB, int delta, Cfg *pCfg, TargetQueue& tq, BinaryFile *pBF)
{
}


void Analysis::analyse(UserProc *proc)
{
}


ICodeGenerator *Boomerang::getCodeGenerator(UserProc *p)
{
	return 0;
}
