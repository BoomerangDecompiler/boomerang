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
 */

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include "types.h"
#include "dataflow.h"
#include "type.h"
#include "util.h"
#include "exp.h"
#include "proc.h"
#include "signature.h"

/*==============================================================================
 * FUNCTION:        Type::Type
 * OVERVIEW:        Default constructor
 * PARAMETERS:      <none>
 * RETURNS:         <Not applicable>
 *============================================================================*/
Type::Type()
{
}

VoidType::VoidType()
{
}

FuncType::FuncType(Signature *sig) : signature(sig)
{
}

IntegerType::IntegerType(int sz, bool sign) : size(sz), signd(sign)
{
}

FloatType::FloatType(int sz) : size(sz)
{
}

BooleanType::BooleanType()
{
}

CharType::CharType()
{
}

PointerType::PointerType(Type *p) : points_to(p)
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
    return points_to->getSize();
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
 * OVERVIEW:        Defines an ordering between Type's (and hence SemStr's).
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this is less than other
 *============================================================================*/
bool IntegerType::operator<(const Type& other) const
{
    if (other.isInteger()) {
  	if (size > ((IntegerType&)other).size)
		return false;
	if (signd && ((IntegerType&)other).signd)
		return false;
    }
    return true;
}

bool FloatType::operator<(const Type& other) const
{
    if (other.isFloat()) {
  	if (size > ((FloatType&)other).size)
		return false;
    }
    return true;
}

bool VoidType::operator<(const Type& other) const
{
    return true;
}

bool FuncType::operator<(const Type& other) const
{
    return true;
}

bool BooleanType::operator<(const Type& other) const
{
    return true;
}

bool CharType::operator<(const Type& other) const
{
    return true;
}

bool PointerType::operator<(const Type& other) const
{
    return (*points_to < other);
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
    std::string s = signature->getReturnType()->getCtype();
    s += " (";
    for (unsigned int i = 0; i < signature->getNumParams(); i++) {
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

// serialization
bool IntegerType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'i', false);

	saveFID(ouf, FID_TYPE_SIZE);
	saveValue(ouf, size);

	saveFID(ouf, FID_TYPE_SIGN);
	saveValue(ouf, signd);

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool FloatType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'f', false);

	saveFID(ouf, FID_TYPE_SIZE);
	saveValue(ouf, size);

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool VoidType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'v', false);

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool FuncType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'F', false);

	if (signature) {
		saveFID(ouf, FID_TYPE_SIGNATURE);
		std::streampos pos = ouf.tellp();
		int len = -1;
		saveLen(ouf, -1, true);
		std::streampos posa = ouf.tellp();

		assert(signature->serialize(ouf, len));

		std::streampos now = ouf.tellp();
		assert((int)(now - posa) == len);
		ouf.seekp(pos);
		saveLen(ouf, len, true);
		ouf.seekp(now);
	}

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool BooleanType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'b', false);

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool CharType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'c', false);

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

bool PointerType::serialize(std::ostream &ouf, int &len)
{
	std::streampos st = ouf.tellp();

	saveValue(ouf, 'p', false);

	int l;
	points_to->serialize(ouf, l);

	saveFID(ouf, FID_TYPE_END);
	saveLen(ouf, 0);

	len = ouf.tellp() - st;
	return true;
}

Type *Type::deserialize(std::istream &inf)
{
	Type *t = NULL;

	char type;
	loadValue(inf, type, false);

	switch(type) {
	    case 'i':
		t = new IntegerType();
		break;
	    case 'f':
		t = new FloatType();
		break;
	    case 'v':
		t = new VoidType();
		break;
	    case 'F':
		t = new FuncType();
		break;
	    case 'b':
		t = new BooleanType();
		break;
	    case 'c':
		t = new CharType();
		break;
	    case 'p':
		{
			Type *to = Type::deserialize(inf);
			t = new PointerType(to);
		}
		break;
	    default:
		assert(false);
	}

	int fid;
	while ((fid = loadFID(inf)) != -1 && fid != FID_TYPE_END)
		t->deserialize_fid(inf, fid);
	assert(loadLen(inf) == 0);
	
	return t;
}

bool IntegerType::deserialize_fid(std::istream &inf, int fid)
{
	switch (fid) {
		case FID_TYPE_SIZE:
			{
				loadValue(inf, size);
			}
			break;
		case FID_TYPE_SIGN:
			{
				loadValue(inf, signd);
			}
			break;
		default:
			skipFID(inf, fid);
			return false;
	}

    return true;
}

bool FloatType::deserialize_fid(std::istream &inf, int fid)
{
	switch (fid) {
		case FID_TYPE_SIZE:
			{
				loadValue(inf, size);
			}
			break;
		default:
			skipFID(inf, fid);
			return false;
	}

    return true;
}

bool VoidType::deserialize_fid(std::istream &inf, int fid)
{
    skipFID(inf, fid);
    return false;
}

bool FuncType::deserialize_fid(std::istream &inf, int fid)
{
    switch (fid) {
		case FID_TYPE_SIGNATURE:
			{
				signature = Signature::deserialize(inf);
				assert(signature);
			}
			break;
		default:
			skipFID(inf, fid);
			return false;
	}

    return true;
}

bool BooleanType::deserialize_fid(std::istream &inf, int fid)
{
    skipFID(inf, fid);
    return false;
}

bool CharType::deserialize_fid(std::istream &inf, int fid)
{
    skipFID(inf, fid);
    return false;
}

bool PointerType::deserialize_fid(std::istream &inf, int fid)
{
    skipFID(inf, fid);
    return false;
}


