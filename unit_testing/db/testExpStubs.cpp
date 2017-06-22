#include <iostream>
#include <string>
#include "boomerang/type/type.h"
#include "boomerang/include/types.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/signature.h"
#include "boomerang/util/Log.h"

class Prog;

// util
#include "boomerang/utilStubs.cpp"

// basicblock
void BasicBlock::getReachInAt(Statement *stmt, StatementSet& reachin, int phase)
{
}


void BasicBlock::getAvailInAt(Statement *stmt, StatementSet& reachin, int phase)
{
}


// type
#include "boomerang/typeStubs.cpp"

// Proc
Signature *Proc::getSignature()
{
	return nullptr;
}


Cfg *UserProc::getCFG()
{
	return nullptr;
}


const char *Proc::getName()
{
	return "";
}


Prog *Proc::getProg()
{
	return nullptr;
}


void UserProc::getReturnSet(LocationSet& ret)
{
}


// Prog
char *Prog::getStringConstant(ADDRESS uaddr)
{
	return nullptr;
}


Proc *Prog::findProc(ADDRESS uAddr) const
{
	return nullptr;
}


void Prog::analyse()
{
}


// signature
std::list<Exp *> *Signature::getCallerSave(Prog *prog)
{
	return nullptr;
}


// frontend
void FrontEnd::decode(Prog *prog, ADDRESS a)
{
}


// Misc
Boomerang::Boomerang()
{
}


Boomerang *Boomerang::boomerang = nullptr;
