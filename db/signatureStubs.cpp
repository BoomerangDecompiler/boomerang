#include "signature.h"

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
void Signature::print(std::ostream &out) {}
int Signature::getStackRegister(Prog* prog) {return 0;}
Signature *Signature::instantiate(const char *str, const char *nam)
{return 0;}
