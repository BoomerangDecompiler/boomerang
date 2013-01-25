#include <iostream>
#include <string>
#include "type.h"
#include "types.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"
#include "pentiumfrontend.h"

class Prog;

// util
#include "utilStubs.cpp"

// basicblock
void BasicBlock::setOutEdge(int i, PBB pNewOutEdge) {}
void BasicBlock::addInEdge(PBB pNewInEdge) {}

// type
#include "typeStubs.cpp"

// Prog
Prog::Prog() {}
Prog::~Prog() {}
Prog::Prog(BinaryFile *pBF, FrontEnd *pFE) {}
char *Prog::getStringConstant(ADDRESS uaddr) {return NULL;}
Proc* Prog::findProc(ADDRESS uAddr) const {return NULL;}
void Prog::analyse() {}
void Prog::decompile() {}
void Prog::toSSAform() {}
void Prog::initStatements() {}
UserProc* Prog::getFirstUserProc(std::list<Proc*>::iterator& it) {return 0;}
UserProc* Prog::getNextUserProc(std::list<Proc*>::iterator& it) {return 0;}

// frontend
void FrontEnd::decode(Prog *prog, ADDRESS a) {}
FrontEnd::FrontEnd(BinaryFile *pBF) {}
PentiumFrontEnd::PentiumFrontEnd(BinaryFile *pBF) : FrontEnd(pBF) {}
PentiumFrontEnd::~PentiumFrontEnd() {}
FrontEnd::~FrontEnd() {}
int FrontEnd::getInst(int addr) {return 0;}
bool PentiumFrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os,
    bool spec /* = false */, PHELPER helperFunc /* = NULL */) {return false;}
ADDRESS PentiumFrontEnd::getMainEntryPoint( bool &gotMain ) {return 0;}
FrontEnd* FrontEnd::Load(const char *fname) {return 0;}
Prog *FrontEnd::decode() {return 0;}
bool FrontEnd::processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os,
  bool spec /* = false */, PHELPER helperFunc) {return false;}

// cfg
PBB Cfg::newBB(std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges) {return 0;}
void Cfg::print(std::ostream &out, bool withDF) {}
void Cfg::setEntryBB(PBB bb) {}

//Misc
Boomerang::Boomerang() {}
Boomerang *Boomerang::boomerang = NULL;

// loader
BinaryFile *BinaryFile::Load( const char *sName ) {return 0;}
