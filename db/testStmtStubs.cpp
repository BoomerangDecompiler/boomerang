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
void BasicBlock::getReachInAt(Statement *stmt, StatementSet &reachin, int phase) {}
void BasicBlock::getAvailInAt(Statement *stmt, StatementSet &reachin, int phase) {}
void BasicBlock::setOutEdge(int i, PBB pNewOutEdge) {}
void BasicBlock::addInEdge(PBB pNewInEdge) {}

// type
#include "typeStubs.cpp"

// Proc
Proc::Proc(Prog *prog, ADDRESS uNative, Signature *sig) {}
Proc::~Proc() {}
Signature *Proc::getSignature() {return NULL;}
Cfg* UserProc::getCFG() {return NULL;}
const char* Proc::getName() {return "";}
Prog *Proc::getProg() {return NULL;}
void UserProc::getReturnSet(LocationSet &ret) {}
UserProc::UserProc(Prog *prog, std::string& name, ADDRESS uNative)
  : Proc(prog, uNative, new Signature(name.c_str())) {}
UserProc::~UserProc() {}
std::ostream& UserProc::put(std::ostream& os) {return std::cerr;}
bool UserProc::serialize(std::ostream &ouf, int &len) {return false;}
bool UserProc::deserialize_fid(std::istream &inf, int fid) {return false;}
bool Proc::deserialize_fid(std::istream &inf, int fid) {return false;}
void UserProc::propagateStatements(int memDepth) {}
void UserProc::getStatements(StatementList &stmts) {}

// Prog
Prog::Prog() {}
Prog::~Prog() {}
Prog::Prog(BinaryFile *pBF, FrontEnd *pFE) {}
char *Prog::getStringConstant(ADDRESS uaddr) {return NULL;}
Proc* Prog::findProc(ADDRESS uAddr) const {return NULL;}
void Prog::analyse() {}
void Prog::forwardGlobalDataflow() {}
void Prog::decompile() {}
void Prog::toSSAform() {}
void Prog::initStatements() {}
UserProc* Prog::getFirstUserProc(std::list<Proc*>::iterator& it) {return 0;}
UserProc* Prog::getNextUserProc(std::list<Proc*>::iterator& it) {return 0;}

// signature
std::list<Exp*> *Signature::getCallerSave(Prog* prog) {return NULL;}
Signature::Signature(const char *nam) {}
bool Signature::operator==(const Signature& other) const {return false;}
Signature *Signature::clone() {return 0;}
bool Signature::serialize(std::ostream &ouf, int len) {return false;}
Signature *Signature::deserialize(std::istream &inf) {return 0;}
bool Signature::deserialize_fid(std::istream &inf, int fid) {return false;}
Exp *Signature::getReturnExp() {return 0;}
Exp *Signature::getReturnExp2(BinaryFile *pBF) {return NULL;}
Type *Signature::getReturnType() {return 0;}
void Signature::setReturnType(Type *t) {}
const char *Signature::getName() {return NULL;}
void Signature::setName(const char *nam) {}
void Signature::addParameter(const char *nam) {}
void Signature::addParameter(Type *type, const char *nam, Exp *e) {}
void Signature::addParameter(Exp *e) {}
void Signature::setNumParams(int n) {}
int Signature::getNumParams() {return 0;}
Exp *Signature::getParamExp(int n) {return 0;}
Type *Signature::getParamType(int n) {return 0;}
Exp *Signature::getArgumentExp(int n) {return 0;}
const char *Signature::getParamName(int n) {return 0;}
void Signature::analyse(UserProc *p) {}
Signature *Signature::promote(UserProc *p) {return 0;}
void Signature::getInternalStatements(StatementList &stmts) {}

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
