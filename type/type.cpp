/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       type.cc
 * OVERVIEW:   Implementation of the Type class: low level type information
 *============================================================================*/

/*
 * $Revision$
 *
 * 28 Apr 02 - Mike: getTempType() returns a Type* now
 * 26 Aug 03 - Mike: Fixed operator< (had to re-introduce an enum... ugh)
 */

#ifndef __TYPE_H_
#define __TYPE_H_

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include "types.h"
#include "type.h"
#include "util.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "signature.h"

/*==============================================================================
 * FUNCTION:        Type::Type
 * OVERVIEW:        Default constructor
 * PARAMETERS:      <none>
 * RETURNS:         <Not applicable>
 *============================================================================*/
Type::Type(eType id) : id(id)
{
}

VoidType::VoidType() : Type(eVoid)
{
}

FuncType::FuncType(Signature *sig) : Type(eFunc), signature(sig)
{
}

IntegerType::IntegerType(int sz, bool sign) : Type(eInteger), size(sz),
  signd(sign)
{
}

FloatType::FloatType(int sz) : Type(eFloat), size(sz)
{
}

BooleanType::BooleanType() : Type(eBoolean)
{
}

CharType::CharType() : Type(eChar)
{
}

PointerType::PointerType(Type *p) : Type(ePointer), points_to(p)
{
}

ArrayType::ArrayType(Type *p, unsigned length) : Type(eArray), base_type(p),
  length(length)
{
}

NamedType::NamedType(const char *name) : Type(eNamed), name(name)
{
}

CompoundType::CompoundType() : Type(eCompound)
{
}

/*==============================================================================
 * FUNCTION:        Type::~Type
 * OVERVIEW:        Virtual destructor
 * PARAMETERS:      <none>
 * RETURNS:         <Not applicable>
 *============================================================================*/
Type::~Type()
{
}

VoidType::~VoidType()
{
}

FuncType::~FuncType()
{
}

IntegerType::~IntegerType()
{
}

FloatType::~FloatType()
{
}

BooleanType::~BooleanType()
{
}

CharType::~CharType()
{
}

PointerType::~PointerType()
{
	delete points_to;
}

ArrayType::~ArrayType()
{
	delete base_type;
}

NamedType::~NamedType()
{
}

CompoundType::~CompoundType()
{
}

/*==============================================================================
 * FUNCTION:        *Type::clone
 * OVERVIEW:        Deep copy of this type
 * PARAMETERS:      <none>
 * RETURNS:         Copy of the type
 *============================================================================*/
Type *IntegerType::clone() const
{
    IntegerType *t = new IntegerType(size, signd);
    return t;
}

Type *FloatType::clone() const
{
    FloatType *t = new FloatType(size);
    return t;
}

Type *BooleanType::clone() const
{
    BooleanType *t = new BooleanType();
    return t;
}

Type *CharType::clone() const
{
    CharType *t = new CharType();
    return t;
}

Type *VoidType::clone() const
{
    VoidType *t = new VoidType();
    return t;
}

Type *FuncType::clone() const
{
    FuncType *t = new FuncType(signature);
    return t;
}

Type *PointerType::clone() const
{
    PointerType *t = new PointerType(points_to->clone());
    return t;
}

Type *ArrayType::clone() const
{
    ArrayType *t = new ArrayType(base_type->clone(), length);
    return t;
}

Type *NamedType::clone() const
{
    NamedType *t = new NamedType(name.c_str());
    return t;
}

Type *CompoundType::clone() const
{
    CompoundType *t = new CompoundType();
    for (unsigned i = 0; i < types.size(); i++)
        t->addType(types[i]->clone(), names[i].c_str());
    return t;
}

/*==============================================================================
 * FUNCTION:        *Type::getSize
 * OVERVIEW:        get the size of this type
 * PARAMETERS:      <none>
 * RETURNS:         Size of the type (in bits)
 *============================================================================*/
int IntegerType::getSize() const
{
    return size;
}

int FloatType::getSize() const
{
    return size;
}

int BooleanType::getSize() const
{
    return 1;
}

int CharType::getSize() const
{
    return 1;
}

int VoidType::getSize() const
{
    return 0;
}

int FuncType::getSize() const
{
    return 0; // always nagged me
}

int PointerType::getSize() const
{
    return 4; //points_to->getSize(); // yes, it was a good idea at the time
}

int ArrayType::getSize() const
{
    return base_type->getSize() * length;
}

int NamedType::getSize() const
{
    return 0; // don't know
}

int CompoundType::getSize() const
{
    int n = 0;
    for (unsigned i = 0; i < types.size(); i++)
        n += types[i]->getSize();
    return n;
}

/*==============================================================================
 * FUNCTION:        Type::parseType
 * OVERVIEW:        static Constructor from string
 * PARAMETERS:      str: string to parse
 * RETURNS:         <Not applicable>
 *============================================================================*/
Type *Type::parseType(const char *str)
{
    return NULL;
}

/*==============================================================================
 * FUNCTION:        *Type::operator==
 * OVERVIEW:        Equality comparsion.
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other
 *============================================================================*/
bool IntegerType::operator==(const Type& other) const
{
    return other.isInteger() && (size == ((IntegerType&)other).size) &&
        (signd == ((IntegerType&)other).signd);
}

bool FloatType::operator==(const Type& other) const
{
    return other.isFloat() && (size == ((FloatType&)other).size);
}

bool BooleanType::operator==(const Type& other) const
{
    return other.isBoolean();
}

bool CharType::operator==(const Type& other) const
{
    return other.isChar();
}

bool VoidType::operator==(const Type& other) const
{
    return other.isVoid();
}

bool FuncType::operator==(const Type& other) const
{
    return other.isFunc() && (*signature == *((FuncType&)other).signature);
}

bool PointerType::operator==(const Type& other) const
{
    return other.isPointer() && (*points_to == *((PointerType&)other).points_to);
}

bool ArrayType::operator==(const Type& other) const
{
    return other.isArray() && *base_type == *((ArrayType&)other).base_type &&
           ((ArrayType&)other).length == length;
}

bool NamedType::operator==(const Type& other) const
{
    return other.isNamed() && (name == ((NamedType&)other).name);
}

bool CompoundType::operator==(const Type& other) const
{
    const CompoundType &cother = (CompoundType&)other;
    if (other.isCompound() && cother.types.size() == types.size()) {
        for (unsigned i = 0; i < types.size(); i++)
            if (!(*types[i] == *cother.types[i]))
                return false;
        return true;
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        Type::operator!=
 * OVERVIEW:        Inequality comparsion.
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other
 *============================================================================*/
bool Type::operator!=(const Type& other) const
{
    return !(*this == other);
}

/*==============================================================================
 * FUNCTION:        *Type::operator-=
 * OVERVIEW:        Equality operator, ignoring sign. True if equal in broad
 *                    type and size, but not necessarily sign
 *                    Considers all float types > 64 bits to be the same
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other (ignoring sign)
 *============================================================================
bool IntegerType::operator-=(const Type& other) const
{
    if (!other.isInteger()) return false;
    return size == ((IntegerType&)other).size;
}

bool FloatType::operator-=(const Type& other) const
{
    if (!other.isFloat()) return false;
    if (size > 64 && ((FloatType&)other).size > 64)
	return true;
    return size == ((FloatType&)other).size;
}

*/
/*==============================================================================
 * FUNCTION:        *Type::operator<
 * OVERVIEW:        Defines an ordering between Type's
 *                    (and hence sets etc of Exp* using lessExpStar).
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this is less than other
 *============================================================================*/
bool IntegerType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
  	if (size < ((IntegerType&)other).size) return true;
  	if (size > ((IntegerType&)other).size) return false;
	return (signd < ((IntegerType&)other).signd);
}

bool FloatType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
	return (size < ((FloatType&)other).size);
}

bool VoidType::operator<(const Type& other) const {
    return id < other.getId();
}

bool FuncType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
    // FIXME: Need to compare signatures
    return true;
}

bool BooleanType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
    return true;
}

bool CharType::operator<(const Type& other) const {
    return id < other.getId();
}

bool PointerType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
    return (*points_to < *((PointerType&)other).points_to);
}

bool ArrayType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
    return (*base_type < *((ArrayType&)other).base_type);
}

bool NamedType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
    return (name < ((NamedType&)other).name);
}

bool CompoundType::operator<(const Type& other) const {
    if (id < other.getId()) return true;
    if (id > other.getId()) return false;
    return getSize() < other.getSize();
}

/*==============================================================================
 * FUNCTION:        *Type::getCtype
 * OVERVIEW:        Return a string representing this type
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a constant string of char
 *============================================================================*/
const char *VoidType::getCtype() const
{
    return "void";
}

const char *FuncType::getCtype() const
{
    if (signature == NULL)
	return "void (void)"; 
    std::string s; 
    if (signature->getNumReturns() == 0)
        s += "void";
    else 
        s += signature->getReturnType(0)->getCtype();
    s += " (";
    for (int i = 0; i < signature->getNumParams(); i++) {
       if (i != 0) s += ", ";
       s += signature->getParamType(i)->getCtype(); 
    }
    s += ")";
    return s.c_str();
}

const char *IntegerType::getCtype() const
{
    if (signd) {
        switch(size) {
            case 32: return "int"; break;
            case 16: return "short"; break;
            case  8: return "char"; break;
            case  1: return "bool"; break;
            case 64: return "long long"; break;
            default: return "?";
         }
    } else {
        switch (size) {
            case 32: return "unsigned int"; break;
            case 16: return "unsigned short"; break;
            case  8: return "unsigned char"; break;
            case  1: return "bool"; break;
            case 64: return "unsigned long long"; break;
            default: return "?";
        }
    }
}

const char *FloatType::getCtype() const
{
    switch (size) {
        case 32: return "float"; break;
        case 64: return "double"; break;
        default: return "double"; break;
    }
}

const char *BooleanType::getCtype() const
{
     return "bool";
}

const char *CharType::getCtype() const
{
     return "char";
}

const char *PointerType::getCtype() const
{
     std::string s = points_to->getCtype();
     s += "*";
     return s.c_str(); // memory..
}

const char *ArrayType::getCtype() const
{
    std::string s = base_type->getCtype();
    std::ostringstream ost;
    ost << "[" << length << "]";
    s += ost.str().c_str();
    return s.c_str(); // memory..
}

const char *NamedType::getCtype() const
{
     return name.c_str();
}

const char *CompoundType::getCtype() const
{
    std::string &tmp = *(new std::string("struct { "));
    for (unsigned i = 0; i < types.size(); i++) {
        tmp += types[i]->getCtype();
        if (names[i] != "") {
            tmp += " ";
            tmp += names[i];
        }
        tmp += "; ";
    }
    return tmp.c_str();
}

std::map<std::string, Type*> Type::namedTypes;

// named type accessors
void Type::addNamedType(const char *name, Type *type)
{
    namedTypes[name] = type;
}

Type *Type::getNamedType(const char *name)
{
    if (namedTypes.find(name) != namedTypes.end())
        return namedTypes[name];
    return NULL;
}

/*==============================================================================
 * FUNCTION:    getTempType
 * OVERVIEW:    Given the name of a temporary variable, return its Type
 * NOTE:        Caller must delete result
 * PARAMETERS:  name: reference to a string (e.g. "tmp", "tmpd")
 * RETURNS:     Ptr to a new Type object
 *============================================================================*/
Type* Type::getTempType(const std::string& name)
{
    Type* ty;
    char ctype = ' ';
    if (name.size() > 3) ctype = name[3];
    switch (ctype) {
        // They are all int32, except for a few specials
        case 'f': ty = new FloatType(32); break;
        case 'd': ty = new FloatType(64); break;
        case 'F': ty = new FloatType(80); break;
        case 'D': ty = new FloatType(128); break;
        case 'l': ty = new IntegerType(64); break;
        case 'h': ty = new IntegerType(16); break;
        case 'b': ty = new IntegerType(8); break;
        default:  ty = new IntegerType(32); break;
    }
    return ty;
}


/*==============================================================================
 * FUNCTION:    *Type::getTempName
 * OVERVIEW:    Return a minimal temporary name for this type. It'd be even
 *              nicer to return a unique name, but we don't know scope at
 *              this point, and even so we could still clash with a user-defined
 *              name later on :(
 * PARAMETERS:  
 * RETURNS:     a string
 *============================================================================*/
std::string IntegerType::getTempName() const
{
    switch( size ) {
        case 1:  /* Treat as a tmpb */
        case 8:  return std::string("tmpb");
        case 16: return std::string("tmph");
        case 32: return std::string("tmpi");
        case 64: return std::string("tmpl");
    }
    return std::string("tmp");
}

std::string FloatType::getTempName() const
{
    switch( size ) {
        case 32: return std::string("tmpf");
        case 64: return std::string("tmpd");
        case 80: return std::string("tmpF");
        case 128:return std::string("tmpD");
    }
    return std::string("tmp");
}

std::string Type::getTempName() const
{
    return std::string("tmp"); // what else can we do? (besides panic)
}

int NamedType::nextAlpha = 0;
NamedType* NamedType::getAlpha() {
    std::ostringstream ost;
    ost << "alpha" << nextAlpha++;
    return new NamedType(strdup(ost.str().c_str()));
}

PointerType* PointerType::getPtrAlpha() {
    return new PointerType(NamedType::getAlpha());
}
#endif  // #ifndef __TYPE_H__
