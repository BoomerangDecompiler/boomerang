#include "type.h"

Type *Type::deserialize(std::istream &inf) {return NULL;}
VoidType::VoidType() {}
VoidType::~VoidType() {}
BooleanType::BooleanType() {}
BooleanType::~BooleanType() {}
Type::Type() {}
Type::~Type() {}
Type *Type::getNamedType(const char *name) {return 0;}
bool Type::operator!=(const Type& other) const {return false;}
std::string Type::getTempName() const {return "";}
void Type::addNamedType(const char *name, Type *type) {}

Type* BooleanType::clone() const {return NULL;}
bool    BooleanType::operator==(const Type& other) const {return false;}
bool    BooleanType::operator< (const Type& other) const {return false;}
int     BooleanType::getSize() const {return 0;}
const char *BooleanType::getCtype() const {return NULL;}
bool BooleanType::serialize(std::ostream &ouf, int &len) {return false;}
bool BooleanType::deserialize_fid(std::istream &inf, int fid) {return false;}

Type *VoidType::clone() const {return NULL;}
bool    VoidType::operator==(const Type& other) const {return false;}
bool    VoidType::operator< (const Type& other) const {return false;}
int     VoidType::getSize() const {return 0;}
const char *VoidType::getCtype() const {return NULL;}
bool VoidType::serialize(std::ostream &ouf, int &len) {return false;}
bool VoidType::deserialize_fid(std::istream &inf, int fid) {return false;}

IntegerType::IntegerType(int sz, bool sign) {}
IntegerType::~IntegerType() {}
Type* IntegerType::clone() const {return NULL;}
bool    IntegerType::operator==(const Type& other) const {return false;}
bool    IntegerType::operator< (const Type& other) const {return false;}
int     IntegerType::getSize() const {return 0;}
const char *IntegerType::getCtype() const {return NULL;}
std::string IntegerType::getTempName() const {return "";}
bool IntegerType::serialize(std::ostream &ouf, int &len) {return false;}
bool IntegerType::deserialize_fid(std::istream &inf, int fid) {return false;}

CharType::CharType() {}
CharType::~CharType() {}
Type *CharType::clone() const {return NULL;}
bool    CharType::operator==(const Type& other) const {return false;}
bool    CharType::operator< (const Type& other) const {return false;}
int     CharType::getSize() const {return 0;}
const char *CharType::getCtype() const {return NULL;}
bool CharType::serialize(std::ostream &ouf, int &len) {return false;}
bool CharType::deserialize_fid(std::istream &inf, int fid) {return false;}

PointerType::PointerType(Type* t) {}
PointerType::~PointerType() {}

FloatType::FloatType(int i) {}
FloatType::~FloatType() {}
Type *FloatType::clone() const {return NULL;}
bool    FloatType::operator==(const Type& other) const {return false;}
bool    FloatType::operator< (const Type& other) const {return false;}
bool FloatType::serialize(std::ostream &ouf, int &len) {return false;}
bool FloatType::deserialize_fid(std::istream &inf, int fid) {return false;}
int     FloatType::getSize() const {return 0;}
const char *FloatType::getCtype() const {return NULL;}
std::string FloatType::getTempName() const {return "";}

Type *PointerType::clone() const {return NULL;}
bool    PointerType::operator==(const Type& other) const {return false;}
bool    PointerType::operator< (const Type& other) const {return false;}
int     PointerType::getSize() const {return 0;}
const char *PointerType::getCtype() const {return NULL;}
bool PointerType::serialize(std::ostream &ouf, int &len) {return false;}
bool PointerType::deserialize_fid(std::istream &inf, int fid) {return false;}

FuncType::FuncType(Signature* ) {}
FuncType::~FuncType() {}
Type *FuncType::clone() const {return NULL;}
bool    FuncType::operator==(const Type& other) const {return false;}
bool    FuncType::operator< (const Type& other) const {return false;}
bool FuncType::serialize(std::ostream &ouf, int &len) {return false;}
bool FuncType::deserialize_fid(std::istream &inf, int fid) {return false;}
int     FuncType::getSize() const {return 0;}
const char *FuncType::getCtype() const {return NULL;}

NamedType::NamedType(const char *name) {}
NamedType::~NamedType() {}
Type *NamedType::clone() const {return NULL;}
bool    NamedType::operator==(const Type& other) const {return false;}
bool    NamedType::operator< (const Type& other) const {return false;}
bool NamedType::serialize(std::ostream &ouf, int &len) {return false;}
bool NamedType::deserialize_fid(std::istream &inf, int fid) {return false;}
int     NamedType::getSize() const {return 0;}
const char *NamedType::getCtype() const {return NULL;}
