#include <iostream>
#include <string>
#include "type.h"
#include "types.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"

class Prog;

// util
#include "utilStubs.cpp"

// basicblock
void BasicBlock::getReachInAt(Statement *stmt, StatementSet &reachin, int phase) {}
void BasicBlock::getAvailInAt(Statement *stmt, StatementSet &reachin, int phase) {}

// type
#include "typeStubs.cpp"

// Proc
Signature *Proc::getSignature() {return NULL;}
Cfg* UserProc::getCFG() {return NULL;}
const char* Proc::getName() {return "";}
Prog *Proc::getProg() {return NULL;}
void UserProc::getReturnSet(LocationSet &ret) {}

// Prog
char *Prog::getStringConstant(ADDRESS uaddr) {return NULL;}
Proc* Prog::findProc(ADDRESS uAddr) const {return NULL;}
void Prog::analyse() {}

// signature
std::list<Exp*> *Signature::getCallerSave(Prog* prog) {return NULL;}

// frontend
void FrontEnd::decode(Prog *prog, ADDRESS a) {}

//Misc
Boomerang::Boomerang() {}
Boomerang *Boomerang::boomerang = NULL;
