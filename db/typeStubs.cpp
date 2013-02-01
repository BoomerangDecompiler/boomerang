#include "type.h"

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

Type* BooleanType::clone() const {return nullptr;}
bool    BooleanType::operator==(const Type& other) const {return false;}
bool    BooleanType::operator< (const Type& other) const {return false;}
int        BooleanType::getSize() const {return 0;}
const char *BooleanType::getCtype() const {return nullptr;}

Type *VoidType::clone() const {return nullptr;}
bool    VoidType::operator==(const Type& other) const {return false;}
bool    VoidType::operator< (const Type& other) const {return false;}
int        VoidType::getSize() const {return 0;}
const char *VoidType::getCtype() const {return nullptr;}

IntegerType::IntegerType(int sz, bool sign) {}
IntegerType::~IntegerType() {}
Type* IntegerType::clone() const {return nullptr;}
bool    IntegerType::operator==(const Type& other) const {return false;}
bool    IntegerType::operator< (const Type& other) const {return false;}
int        IntegerType::getSize() const {return 0;}
const char *IntegerType::getCtype() const {return nullptr;}
std::string IntegerType::getTempName() const {return "";}

CharType::CharType() {}
CharType::~CharType() {}
Type *CharType::clone() const {return nullptr;}
bool    CharType::operator==(const Type& other) const {return false;}
bool    CharType::operator< (const Type& other) const {return false;}
int        CharType::getSize() const {return 0;}
const char *CharType::getCtype() const {return nullptr;}

PointerType::PointerType(Type* t) {}
PointerType::~PointerType() {}

FloatType::FloatType(int i) {}
FloatType::~FloatType() {}
Type *FloatType::clone() const {return nullptr;}
bool    FloatType::operator==(const Type& other) const {return false;}
bool    FloatType::operator< (const Type& other) const {return false;}
int        FloatType::getSize() const {return 0;}
const char *FloatType::getCtype() const {return nullptr;}
std::string FloatType::getTempName() const {return "";}

Type *PointerType::clone() const {return nullptr;}
bool    PointerType::operator==(const Type& other) const {return false;}
bool    PointerType::operator< (const Type& other) const {return false;}
int        PointerType::getSize() const {return 0;}
const char *PointerType::getCtype() const {return nullptr;}

FuncType::FuncType(Signature* ) {}
FuncType::~FuncType() {}
Type *FuncType::clone() const {return nullptr;}
bool    FuncType::operator==(const Type& other) const {return false;}
bool    FuncType::operator< (const Type& other) const {return false;}
int        FuncType::getSize() const {return 0;}
const char *FuncType::getCtype() const {return nullptr;}

NamedType::NamedType(const char *name) {}
NamedType::~NamedType() {}
Type *NamedType::clone() const {return nullptr;}
bool    NamedType::operator==(const Type& other) const {return false;}
bool    NamedType::operator< (const Type& other) const {return false;}
int        NamedType::getSize() const {return 0;}
const char *NamedType::getCtype() const {return nullptr;}
