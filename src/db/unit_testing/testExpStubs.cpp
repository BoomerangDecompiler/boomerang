#include <iostream>
#include <string>
#include "include/type.h"
#include "include/types.h"
#include "db/cfg.h"
#include "db/proc.h"
#include "include/prog.h"
#include "include/signature.h"
#include "boom_base/log.h"

class Prog;

// util
#include "utilStubs.cpp"

// basicblock
void BasicBlock::getReachInAt(Statement *stmt, StatementSet& reachin, int phase)
{
}


void BasicBlock::getAvailInAt(Statement *stmt, StatementSet& reachin, int phase)
{
}


// type
#include "typeStubs.cpp"

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
