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
 * FILE:	   type.cpp
 * OVERVIEW:   Implementation of the Type class: low level type information
 *============================================================================*/

/*
 * $Revision$
 *
 * 28 Apr 02 - Mike: getTempType() returns a Type* now
 * 26 Aug 03 - Mike: Fixed operator< (had to re-introduce an enum... ugh)
 * 17 Jul 04 - Mike: Fixed some functions that were returning the buffers
 *			   of std::strings allocated on the stack (affected Windows)
 * 23 Jul 04 - Mike: Implement SizeType
 */

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
#include "boomerang.h"
// For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#include "rtl.h"
#endif


/*==============================================================================
 * FUNCTION:		Type::Type
 * OVERVIEW:		Default constructor
 * PARAMETERS:		<none>
 * RETURNS:			<Not applicable>
 *============================================================================*/
Type::Type(eType id) : id(id) {
}

VoidType::VoidType() : Type(eVoid) {
}

FuncType::FuncType(Signature *sig) : Type(eFunc), signature(sig) {
}

IntegerType::IntegerType(int sz, int sign) : Type(eInteger), size(sz), signedness(sign) {
}

FloatType::FloatType(int sz) : Type(eFloat), size(sz) {
}

BooleanType::BooleanType() : Type(eBoolean) {
}

CharType::CharType() : Type(eChar) {
}

void PointerType::setPointsTo(Type* p) {
	if (p == this) {					// Note: comparing pointers
		points_to = new VoidType();		// Can't point to self; impossible to compare, print, etc
		if (VERBOSE)
			LOG << "Warning: attempted to create pointer to self: " << (unsigned)this << "\n";
	} else
		points_to = p;
}

PointerType::PointerType(Type *p) : Type(ePointer) {
	setPointsTo(p);
}
ArrayType::ArrayType(Type *p, unsigned length) : Type(eArray), base_type(p), length(length)
{
}

// we actually want unbounded arrays to still work correctly when
// computing aliases.. as such, we give them a very large bound
// and hope that no-one tries to alias beyond them
#define NO_BOUND 9999999

ArrayType::ArrayType(Type *p) : Type(eArray), base_type(p), length(NO_BOUND)
{
}

bool ArrayType::isUnbounded() const {
	return length == NO_BOUND;
}

void ArrayType::setBaseType(Type* b) {
	// MVE: not sure if this is always the right thing to do
	if (length != NO_BOUND) {
		int baseSize = base_type->getSize()/8;		// Old base size (one element) in bytes
		if (baseSize == 0) baseSize = 1;			// Count void as size 1
		baseSize *= length;							// Old base size (length elements) in bytes
		int newSize = b->getSize()/8;
		if (newSize == 0) newSize = 1;
		length = baseSize / newSize;				// Preserve same byte size for array
	}
	base_type = b;
}
		

NamedType::NamedType(const char *name) : Type(eNamed), name(name)
{
}

CompoundType::CompoundType() : Type(eCompound)
{
}

UnionType::UnionType() : Type(eUnion)
{
}

/*==============================================================================
 * FUNCTION:		Type::~Type
 * OVERVIEW:		Virtual destructor
 * PARAMETERS:		<none>
 * RETURNS:			<Not applicable>
 *============================================================================*/
Type::~Type() { }
VoidType::~VoidType() { }
FuncType::~FuncType() { }
IntegerType::~IntegerType() { }
FloatType::~FloatType() { }
BooleanType::~BooleanType() { }
CharType::~CharType() { }
PointerType::~PointerType() {
	// delete points_to;		// Easier for test code (which doesn't use garbage collection)
}
ArrayType::~ArrayType() {
	// delete base_type;
}
NamedType::~NamedType() { }
CompoundType::~CompoundType() { }
UnionType::~UnionType() { }

/*==============================================================================
 * FUNCTION:		*Type::clone
 * OVERVIEW:		Deep copy of this type
 * PARAMETERS:		<none>
 * RETURNS:			Copy of the type
 *============================================================================*/
Type *IntegerType::clone() const
{
	IntegerType *t = new IntegerType(size, signedness);
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

Type *UnionType::clone() const {
	UnionType *u = new UnionType();
	std::list<UnionElement>::const_iterator it;
	for (it = li.begin(); it != li.end(); it++)
		u->addType(it->type, it->name.c_str());
	return u;
}

Type *SizeType::clone() const
{
	SizeType *t = new SizeType(size);
	return t;
}

Type* UpperType::clone() const
{
	UpperType* t = new UpperType(base_type->clone());
	return t;
}

Type* LowerType::clone() const
{
	LowerType* t = new LowerType(base_type->clone());
	return t;
}


/*==============================================================================
 * FUNCTION:		*Type::getSize
 * OVERVIEW:		Get the size of this type
 * PARAMETERS:		<none>
 * RETURNS:			Size of the type (in bits)
 *============================================================================*/
int IntegerType::getSize() const { return size; }
int	  FloatType::getSize() const { return size; }
int BooleanType::getSize() const { return 1; }
int	   CharType::getSize() const { return 8; }
int	   VoidType::getSize() const { return 0; }
int	   FuncType::getSize() const { return 0; /* always nagged me */ }
int PointerType::getSize() const {
	//points_to->getSize(); // yes, it was a good idea at the time
	return STD_SIZE;
}
int ArrayType::getSize() const {
	return base_type->getSize() * length;
}
int NamedType::getSize() const {
	Type *ty = resolvesTo();
	if (ty)
		return ty->getSize();
	if (VERBOSE)
		LOG << "WARNING: Unknown size for named type " << name.c_str() << "\n";
	return 0; // don't know
}
int CompoundType::getSize() const {
	int n = 0;
	for (unsigned i = 0; i < types.size(); i++)
		n += types[i]->getSize();
	return n;
}
int UnionType::getSize() const {
	int max = 0;
	std::list<UnionElement>::const_iterator it;
	for (it = li.begin(); it != li.end(); it++) {
		int sz = it->type->getSize();
		if (sz > max) max = sz;
	}
	return max;
}
int SizeType::getSize() const { return size; }



Type *CompoundType::getType(const char *nam)
{
	for (unsigned i = 0; i < types.size(); i++)
		if (names[i] == nam)
			return types[i];
	return NULL;
}

#if 0
Type *UnionType::getType(const char *nam)
{
	for (unsigned i = 0; i < types.size(); i++)
		if (names[i] == nam)
			return types[i];
	return NULL;
}
#endif

Type *CompoundType::getTypeAtOffset(int n)
{
	int offset = 0;
	for (unsigned i = 0; i < types.size(); i++) {
		//if (offset >= n && n < offset + types[i]->getSize())
		if (offset <= n && n < offset + types[i]->getSize())
			//return getType(offset == n ? i : i - 1);
			return types[i];
		offset += types[i]->getSize();
	}
	return NULL;
}

const char *CompoundType::getNameAtOffset(int n)
{
	int offset = 0;
	for (unsigned i = 0; i < types.size(); i++) {
		//if (offset >= n && n < offset + types[i]->getSize())
		if (offset <= n && n < offset + types[i]->getSize())
			//return getName(offset == n ? i : i - 1);
			return names[i].c_str();
		offset += types[i]->getSize();
	}
	return NULL;
}

int CompoundType::getOffsetTo(int n)
{
	int offset = 0;
	for (int i = 0; i < n; i++) {
		offset += types[i]->getSize();
	}
	return offset;
}

int CompoundType::getOffsetTo(const char *member)
{
	int offset = 0;
	for (unsigned i = 0; i < types.size(); i++) {
		if (names[i] == member)
			return offset;
		offset += types[i]->getSize();
	}
	return -1;
}

int CompoundType::getOffsetRemainder(int n)
{
	int r = n;
	int offset = 0;
	for (unsigned i = 0; i < types.size(); i++) {
		offset += types[i]->getSize();
		if (offset > n)
			break;
		r -= types[i]->getSize();
	}
	return r;
}

/*==============================================================================
 * FUNCTION:		Type::parseType
 * OVERVIEW:		static Constructor from string
 * PARAMETERS:		str: string to parse
 * RETURNS:			<Not applicable>
 *============================================================================*/
Type *Type::parseType(const char *str)
{
	return NULL;
}

/*==============================================================================
 * FUNCTION:		*Type::operator==
 * OVERVIEW:		Equality comparsion.
 * PARAMETERS:		other - Type being compared to
 * RETURNS:			this == other
 *============================================================================*/
bool IntegerType::operator==(const Type& other) const {
	return other.isInteger() && 
		// Note: zero size matches any other size (wild, or unknown, size)
		(size==0 || ((IntegerType&)other).size==0 ||
		  size == ((IntegerType&)other).size) &&
		(signedness < 0 || ((IntegerType&)other).signedness < 0 ||
		  signedness == ((IntegerType&)other).signedness);
}

bool FloatType::operator==(const Type& other) const {
	return other.isFloat() && 
	  (size == 0 || ((FloatType&)other).size == 0 || 
	  (size == ((FloatType&)other).size));
}

bool BooleanType::operator==(const Type& other) const {
	return other.isBoolean();
}

bool CharType::operator==(const Type& other) const {
	return other.isChar();
}

bool VoidType::operator==(const Type& other) const {
	return other.isVoid();
}

bool FuncType::operator==(const Type& other) const {
	return other.isFunc() && (*signature == *((FuncType&)other).signature);
}

static int pointerCompareNest = 0;
bool PointerType::operator==(const Type& other) const {
//	return other.isPointer() && (*points_to == *((PointerType&)other).points_to);
	if (!other.isPointer()) return false;
	if (++pointerCompareNest >= 20) {
		std::cerr << "PointerType operator== nesting depth exceeded!\n";
		return true;
	}
	bool ret = (*points_to == *((PointerType&)other).points_to);
	pointerCompareNest--;
	return ret;
}

bool ArrayType::operator==(const Type& other) const {
	return other.isArray() && *base_type == *((ArrayType&)other).base_type &&
		   ((ArrayType&)other).length == length;
}

bool NamedType::operator==(const Type& other) const {
	return other.isNamed() && (name == ((NamedType&)other).name);
}

bool CompoundType::operator==(const Type& other) const {
	const CompoundType &cother = (CompoundType&)other;
	if (other.isCompound() && cother.types.size() == types.size()) {
		for (unsigned i = 0; i < types.size(); i++)
			if (!(*types[i] == *cother.types[i]))
				return false;
		return true;
	}
	return false;
}

bool UnionType::operator==(const Type& other) const {
	const UnionType &uother = (UnionType&)other;
	std::list<UnionElement>::const_iterator it1, it2;
	if (other.isUnion() && uother.li.size() == li.size()) {
		for (it1 = li.begin(), it2 = uother.li.begin(); it1 != li.end(); it1++, it2++)
			if (!(*it1->type == *it2->type))
				return false;
		return true;
	}
	return false;
}

bool SizeType::operator==(const Type& other) const {
	return other.isSize() && (size == ((SizeType&)other).size);
}

/*==============================================================================
 * FUNCTION:		Type::operator!=
 * OVERVIEW:		Inequality comparsion.
 * PARAMETERS:		other - Type being compared to
 * RETURNS:			this == other
 *============================================================================*/
bool Type::operator!=(const Type& other) const
{
	return !(*this == other);
}

/*==============================================================================
 * FUNCTION:		*Type::operator-=
 * OVERVIEW:		Equality operator, ignoring sign. True if equal in broad
 *					  type and size, but not necessarily sign
 *					  Considers all float types > 64 bits to be the same
 * PARAMETERS:		other - Type being compared to
 * RETURNS:			this == other (ignoring sign)
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
 * FUNCTION:		*Type::operator<
 * OVERVIEW:		Defines an ordering between Type's
 *					  (and hence sets etc of Exp* using lessExpStar).
 * PARAMETERS:		other - Type being compared to
 * RETURNS:			this is less than other
 *============================================================================*/
bool IntegerType::operator<(const Type& other) const {
	if (id < other.getId()) return true;
	if (id > other.getId()) return false;
	if (size < ((IntegerType&)other).size) return true;
	if (size > ((IntegerType&)other).size) return false;
	return (signedness < ((IntegerType&)other).signedness);
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
	return getSize() < other.getSize();		// This won't separate structs of the same size!! MVE
}

bool UnionType::operator<(const Type& other) const {
	if (id < other.getId()) return true;
	if (id > other.getId()) return false;
	return getNumTypes() < ((const UnionType&)other).getNumTypes();
}

bool SizeType::operator<(const Type& other) const {
	if (id < other.getId()) return true;
	if (id > other.getId()) return false;
	return (size < ((SizeType&)other).size);
}

/*==============================================================================
 * FUNCTION:		*Type::match
 * OVERVIEW:		Match operation.
 * PARAMETERS:		pattern - Type to match
 * RETURNS:			Exp list of bindings if match or NULL
 *============================================================================*/
Exp *Type::match(Type *pattern)
{
	if (pattern->isNamed()) {
		LOG << "type match: " << this->getCtype() << " to " << pattern->getCtype() << "\n";
		return new Binary(opList, 
			new Binary(opEquals, 
				new Unary(opVar,
					new Const((char*)pattern->asNamed()->getName())), 
				new TypeVal(this->clone())), 
			new Terminal(opNil));
	}
	return NULL;
}

Exp *IntegerType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *FloatType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *BooleanType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *CharType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *VoidType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *FuncType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *PointerType::match(Type *pattern)
{
	if (pattern->isPointer()) {
		LOG << "got pointer match: " << this->getCtype() << " to " << pattern->getCtype() << "\n";
		return points_to->match(pattern->asPointer()->getPointsTo());
	}
	return Type::match(pattern);
}

Exp *ArrayType::match(Type *pattern)
{
	if (pattern->isArray())
		return base_type->match(pattern);
	return Type::match(pattern);
}

Exp *NamedType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *CompoundType::match(Type *pattern)
{
	return Type::match(pattern);
}

Exp *UnionType::match(Type *pattern)
{
	return Type::match(pattern);
}


/*==============================================================================
 * FUNCTION:		*Type::getCtype
 * OVERVIEW:		Return a string representing this type
 * PARAMETERS:		final: if true, this is final output
 * RETURNS:			Pointer to a constant string of char
 *============================================================================*/
const char *VoidType::getCtype(bool final) const { return "void"; }

const char *FuncType::getCtype(bool final) const {
	if (signature == NULL)
	return "void (void)"; 
	std::string s; 
	if (signature->getNumReturns() == 0)
		s += "void";
	else 
		s += signature->getReturnType(0)->getCtype(final);
	s += " (";
	for (int i = 0; i < signature->getNumParams(); i++) {
	   if (i != 0) s += ", ";
	   s += signature->getParamType(i)->getCtype(final); 
	}
	s += ")";
	return strdup(s.c_str());
}

// As above, but split into the return and parameter parts
void FuncType::getReturnAndParam(const char*& ret, const char*& param) {
	if (signature == NULL) {
		ret = "void";
		param = "(void)";
		return;
	}
	if (signature->getNumReturns() == 0)
		ret = "void";
	else 
		ret = signature->getReturnType(0)->getCtype();
	std::string s; 
	s += " (";
	for (int i = 0; i < signature->getNumParams(); i++) {
	   if (i != 0) s += ", ";
	   s += signature->getParamType(i)->getCtype(); 
	}
	s += ")";
	param = strdup(s.c_str());
}

const char *IntegerType::getCtype(bool final) const {
	if (signedness >= 0) {
		std::string s;
		if (!final && signedness == 0)
			s = "/*signed?*/";
		switch(size) {
			case 32: s += "int"; break;
			case 16: s += "short"; break;
			case  8: s += "char"; break;
			case  1: s += "bool"; break;
			case 64: s += "long long"; break;
			default: 
				if (!final) s += "?";	// To indicate invalid/unknown size
				s += "int";
		}
		return strdup(s.c_str());
	} else {
		switch (size) {
			case 32: return "unsigned int"; break;
			case 16: return "unsigned short"; break;
			case  8: return "unsigned char"; break;
			case  1: return "bool"; break;
			case 64: return "unsigned long long"; break;
			default: if (final) return "unsigned int"; 
				else return "?unsigned int";
		}
	}
}

const char *FloatType::getCtype(bool final) const {
	switch (size) {
		case 32: return "float"; break;
		case 64: return "double"; break;
		default: return "double"; break;
	}
}

const char *BooleanType::getCtype(bool final) const { return "bool"; }

const char *CharType::getCtype(bool final) const { return "char"; }

const char *PointerType::getCtype(bool final) const {
	 std::string s = points_to->getCtype(final);
	 if (points_to->isPointer())
		s += "*";
	 else
		s += " *";
	 return strdup(s.c_str()); // memory..
}

const char *ArrayType::getCtype(bool final) const {
	std::string s = base_type->getCtype(final);
	std::ostringstream ost;
	if (isUnbounded())
		ost << "[]";
	else
		ost << "[" << length << "]";
	s += ost.str().c_str();
	return strdup(s.c_str()); // memory..
}

const char *NamedType::getCtype(bool final) const { return name.c_str(); }

const char *CompoundType::getCtype(bool final) const {
	std::string &tmp = *(new std::string("struct { "));
	for (unsigned i = 0; i < types.size(); i++) {
		tmp += types[i]->getCtype(final);
		if (names[i] != "") {
			tmp += " ";
			tmp += names[i];
		}
		tmp += "; ";
	}
	tmp += "}";
	return strdup(tmp.c_str());
}

const char *UnionType::getCtype(bool final) const {
	std::string &tmp = *(new std::string("union { "));
	std::list<UnionElement>::const_iterator it;
	for (it = li.begin(); it != li.end(); it++) {
		tmp += it->type->getCtype(final);
		if (it->name != "") {
			tmp += " ";
			tmp += it->name;
		}
		tmp += "; ";
	}
	tmp += "}";
	return strdup(tmp.c_str());
}

const char* SizeType::getCtype(bool final) const {
	if (final) {
		// Make a signed integer type of the same size
		IntegerType it(size);
		return it.getCtype(final);
	}
	// Emit a comment and the size
	std::ostringstream ost;
	ost << "/*size " << std::dec << size << "*/";
	return strdup(ost.str().c_str());
}

const char* Type::prints() {
	return getCtype(false);			// For debugging
}

std::map<std::string, Type*> Type::namedTypes;

// named type accessors
void Type::addNamedType(const char *name, Type *type)
{
	if (namedTypes.find(name) != namedTypes.end()) {
		if (*type != *namedTypes[name]) {
			std::cerr << "addNamedType: name " << name << " type " << type->getCtype() << " != " <<
				namedTypes[name]->getCtype() << "\n" << std::flush;
			assert(false);
		}
	} else {
		namedTypes[name] = type->clone();
	}
}

Type *Type::getNamedType(const char *name)
{
	if (namedTypes.find(name) != namedTypes.end())
		return namedTypes[name];
	return NULL;
}

/*==============================================================================
 * FUNCTION:	getTempType
 * OVERVIEW:	Given the name of a temporary variable, return its Type
 * NOTE:		Caller must delete result
 * PARAMETERS:	name: reference to a string (e.g. "tmp", "tmpd")
 * RETURNS:		Ptr to a new Type object
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
 * FUNCTION:	*Type::getTempName
 * OVERVIEW:	Return a minimal temporary name for this type. It'd be even
 *				nicer to return a unique name, but we don't know scope at
 *				this point, and even so we could still clash with a user-defined
 *				name later on :(
 * PARAMETERS:	
 * RETURNS:		a string
 *============================================================================*/
std::string IntegerType::getTempName() const
{
	switch( size ) {
		case 1:	 /* Treat as a tmpb */
		case 8:	 return std::string("tmpb");
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

PointerType* PointerType::newPtrAlpha() {
	return new PointerType(NamedType::getAlpha());
}

// Note: alpha is therefore a "reserved name" for types
bool PointerType::pointsToAlpha() {
	// void* counts as alpha* (and may replace it soon)
	if (points_to->isVoid()) return true;
	if (!points_to->isNamed()) return false;
	return strncmp(((NamedType*)points_to)->getName(), "alpha", 5) == 0;
}

int PointerType::pointerDepth() {
	int d = 1;
	Type* pt = points_to;
	while (pt->isPointer()) {
		pt = pt->asPointer()->getPointsTo();
		d++;
	}
	return d;
}

Type* PointerType::getFinalPointsTo() {
	Type* pt = points_to;
	while (pt->isPointer()) {
		pt = pt->asPointer()->getPointsTo();
	}
	return pt;
}

Type *NamedType::resolvesTo() const
{
	Type *ty = getNamedType(name.c_str());
	if (ty && ty->isNamed())
		return ((NamedType*)ty)->resolvesTo();
	return ty;
}

void ArrayType::fixBaseType(Type *b)
{
	if (base_type == NULL)
		base_type = b;
	else {
		assert(base_type->isArray());
		base_type->asArray()->fixBaseType(b);
	}
}

VoidType *Type::asVoid()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	VoidType *res = dynamic_cast<VoidType*>(ty);
	assert(res);
	return res;
}

FuncType *Type::asFunc()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	FuncType *res = dynamic_cast<FuncType*>(ty);
	assert(res);
	return res;
}

BooleanType *Type::asBoolean()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	BooleanType *res = dynamic_cast<BooleanType*>(ty);
	assert(res);
	return res;
}

CharType *Type::asChar()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	CharType *res = dynamic_cast<CharType*>(ty);
	assert(res);
	return res;
}

IntegerType *Type::asInteger()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	IntegerType *res = dynamic_cast<IntegerType*>(ty);
	assert(res);
	return res;
}

FloatType *Type::asFloat()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	FloatType *res = dynamic_cast<FloatType*>(ty);
	assert(res);
	return res;
}

NamedType *Type::asNamed()
{
	Type *ty = this;
	NamedType *res = dynamic_cast<NamedType*>(ty);
	assert(res);
	return res;
}

PointerType *Type::asPointer()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	PointerType *res = dynamic_cast<PointerType*>(ty);
	assert(res);
	return res;
}

ArrayType *Type::asArray()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	ArrayType *res = dynamic_cast<ArrayType*>(ty);
	assert(res);
	return res;
}

CompoundType *Type::asCompound()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	CompoundType *res = dynamic_cast<CompoundType*>(ty);
	assert(res);
	return res;
}

UnionType *Type::asUnion()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	UnionType *res = dynamic_cast<UnionType*>(ty);
	assert(res);
	return res;
}

UpperType *Type::asUpper() {
	Type* ty = this;
	if (isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	UpperType* res = dynamic_cast<UpperType*>(ty);
	assert(res);
	return res;
}

LowerType *Type::asLower() {
	Type* ty = this;
	if (isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	LowerType* res = dynamic_cast<LowerType*>(ty);
	assert(res);
	return res;
}

bool Type::resolvesToVoid()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isVoid();
}

bool Type::resolvesToFunc()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isFunc();
}

bool Type::resolvesToBoolean()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isBoolean();
}

bool Type::resolvesToChar()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isChar();
}

bool Type::resolvesToInteger()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isInteger();
}

bool Type::resolvesToFloat()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isFloat();
}

bool Type::resolvesToPointer()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isPointer();
}

bool Type::resolvesToArray()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isArray();
}

bool Type::resolvesToCompound()
{
	Type *ty = this;
	if (ty->isNamed())
		ty = ((NamedType*)ty)->resolvesTo();
	return ty && ty->isCompound();
}

bool Type::isPointerToAlpha() {
	return isPointer() && asPointer()->pointsToAlpha();
}

void Type::starPrint(std::ostream& os) {
	os << "*" << this << "*";
}

// A crude shortcut representation of a type
std::ostream& operator<<(std::ostream& os, Type* t) {
	if (t == NULL) return os << '0';
	switch (t->getId()) {
		case eInteger: {
			int sg = ((IntegerType*)t)->getSignedness();
			// 'j' for either i or u, don't know which
			os << (sg == 0 ? 'j' : sg>0 ? 'i' : 'u');
			os << std::dec << t->asInteger()->getSize();
			break;
		}
		case eFloat:	os << 'f'; os << std::dec << t->asFloat()->getSize(); break;
		case ePointer:	os << t->asPointer()->getPointsTo() << '*'; break;
		case eSize:		os << std::dec << t->getSize(); break;
		case eChar:		os << 'c'; break;
		case eVoid:		os << 'v'; break;
		case eBoolean:	os << 'b'; break;
		case eCompound:	os << "struct"; break; 
		case eUnion:	os << "union"; break;
		case eFunc:		os << "func"; break;
		case eArray:	os << '[' << t->asArray()->getBaseType() << ']'; break;
		case eNamed:	os << t->asNamed()->getName(); break;
		case eUpper:	os << "U(" << t->asUpper()->getBaseType() << ')'; break;
		case eLower:	os << "L(" << t->asLower()->getBaseType() << ')'; break;
	}
	return os;
}

// Merge this IntegerType with another
Type* IntegerType::mergeWith(Type* other) {
	if (*this == *other) return this;
	if (!other->isInteger()) return NULL;		// Can you merge with a pointer?
	IntegerType* oth = (IntegerType*)other;
	IntegerType* ret = (IntegerType*)this->clone();
	if (size == 0) ret->setSize(oth->getSize());
	if (signedness == 0) ret->setSigned(oth->getSignedness());
	return ret;
}

// Merge this SizeType with another type
Type* SizeType::mergeWith(Type* other) {
	Type* ret = other->clone();
	ret->setSize(size);
	return ret;
}

class FuncTypeMemo : public Memo {
public:
	FuncTypeMemo(int m) : Memo(m) { }
	Signature *signature;
};

Memo *FuncType::makeMemo(int mId)
{
	FuncTypeMemo *m = new FuncTypeMemo(mId);
	m->signature = signature;

	signature->takeMemo(mId);
	return m;
}

void FuncType::readMemo(Memo *mm, bool dec)
{
	FuncTypeMemo *m = dynamic_cast<FuncTypeMemo*>(mm);
	signature = m->signature;

	//signature->restoreMemo(m->mId, dec);
}

class IntegerTypeMemo : public Memo {
public:
	IntegerTypeMemo(int m) : Memo(m) { }
	int size;
	int signedness;
};

Memo *IntegerType::makeMemo(int mId)
{
	IntegerTypeMemo *m = new IntegerTypeMemo(mId);
	m->size = size;
	m->signedness = signedness;
	return m;
}

void IntegerType::readMemo(Memo *mm, bool dec)
{
	IntegerTypeMemo *m = dynamic_cast<IntegerTypeMemo*>(mm);
	size = m->size;
	signedness = m->signedness;
}

class FloatTypeMemo : public Memo {
public:
	FloatTypeMemo(int m) : Memo(m) { }
	int size;
};

Memo *FloatType::makeMemo(int mId)
{
	FloatTypeMemo *m = new FloatTypeMemo(mId);
	m->size = size;
	return m;
}

void FloatType::readMemo(Memo *mm, bool dec)
{
	FloatTypeMemo *m = dynamic_cast<FloatTypeMemo*>(mm);
	size = m->size;
}

class PointerTypeMemo : public Memo {
public:
	PointerTypeMemo(int m) : Memo(m) { }
	Type *points_to;
};

Memo *PointerType::makeMemo(int mId)
{
	PointerTypeMemo *m = new PointerTypeMemo(mId);
	m->points_to = points_to;

	points_to->takeMemo(mId);

	return m;
}

void PointerType::readMemo(Memo *mm, bool dec)
{
	PointerTypeMemo *m = dynamic_cast<PointerTypeMemo*>(mm);
	points_to = m->points_to;

	points_to->restoreMemo(m->mId, dec);
}

class ArrayTypeMemo : public Memo {
public:
	ArrayTypeMemo(int m) : Memo(m) { }
	Type *base_type;
	unsigned length;
};

Memo *ArrayType::makeMemo(int mId)
{
	ArrayTypeMemo *m = new ArrayTypeMemo(mId);
	m->base_type = base_type;
	m->length = length;

	base_type->takeMemo(mId);

	return m;
}

void ArrayType::readMemo(Memo *mm, bool dec)
{
	ArrayTypeMemo *m = dynamic_cast<ArrayTypeMemo*>(mm);
	length = m->length;
	base_type = m->base_type;

	base_type->restoreMemo(m->mId, dec);
}

class NamedTypeMemo : public Memo {
public:
	NamedTypeMemo(int m) : Memo(m) { }
	std::string name;
	int nextAlpha;
};

Memo *NamedType::makeMemo(int mId)
{
	NamedTypeMemo *m = new NamedTypeMemo(mId);
	m->name = name;
	m->nextAlpha = nextAlpha;
	return m;
}

void NamedType::readMemo(Memo *mm, bool dec)
{
	NamedTypeMemo *m = dynamic_cast<NamedTypeMemo*>(mm);
	name = m->name;
	nextAlpha = m->nextAlpha;
}

class CompoundTypeMemo : public Memo {
public:
	CompoundTypeMemo(int m) : Memo(m) { }
	std::vector<Type*> types;
	std::vector<std::string> names;
};

Memo *CompoundType::makeMemo(int mId)
{
	CompoundTypeMemo *m = new CompoundTypeMemo(mId);
	m->types = types;
	m->names = names;

	for (std::vector<Type*>::iterator it = types.begin(); it != types.end(); it++)
		(*it)->takeMemo(mId);
	return m;
}

void CompoundType::readMemo(Memo *mm, bool dec)
{
	CompoundTypeMemo *m = dynamic_cast<CompoundTypeMemo*>(mm);
	types = m->types;
	names = m->names;

	for (std::vector<Type*>::iterator it = types.begin(); it != types.end(); it++)
		(*it)->restoreMemo(m->mId, dec);
}

class UnionTypeMemo : public Memo {
public:
	UnionTypeMemo(int m) : Memo(m) { }
	std::list<UnionElement> li;
};

Memo *UnionType::makeMemo(int mId)
{
	UnionTypeMemo *m = new UnionTypeMemo(mId);
	m->li = li;

	for (std::list<UnionElement>::iterator it = li.begin(); it != li.end(); it++)
		it->type->takeMemo(mId);		// Is this right? What about the names? MVE
	return m;
}

void UnionType::readMemo(Memo *mm, bool dec)
{
	UnionTypeMemo *m = dynamic_cast<UnionTypeMemo*>(mm);
	li = m->li;

	for (std::list<UnionElement>::iterator it = li.begin(); it != li.end(); it++)
		it->type->restoreMemo(m->mId, dec);
}

void UnionType::addType(Type *n, const char *str) {
	if (n->isUnion()) {
		UnionType* utp = (UnionType*)n;
		// Note: need to check for name clashes eventually
		li.insert(li.end(), utp->li.begin(), utp->li.end());
	} else {
		if (n->isPointer() && n->asPointer()->getPointsTo() == this) {		// Note: pointer comparison
			n = new PointerType(new VoidType);
			if (VERBOSE)
				LOG << "Warning: attempt to union with pointer to self!\n";
		}
		UnionElement ue;
		ue.type = n;
		ue.name = str;
		li.push_back(ue);
	}
}

// Return true if this is a superstructure of other, i.e. we have the same types at the same offsets as other
bool CompoundType::isSuperStructOf(Type* other) {
	if (!other->isCompound()) return false;
	CompoundType* otherCmp = other->asCompound();
	unsigned n = otherCmp->types.size();
	if (n > types.size()) return false;
	for (unsigned i=0; i < n; i++)
		if (otherCmp->types[i] != types[i]) return false;
	return true;
}

// Return true if this is a substructure of other, i.e. other has the same types at the same offsets as this
bool CompoundType::isSubStructOf(Type* other) {
	if (!other->isCompound()) return false;
	CompoundType* otherCmp = other->asCompound();
	unsigned n = types.size();
	if (n > otherCmp->types.size()) return false;
	for (unsigned i=0; i < n; i++)
		if (otherCmp->types[i] != types[i]) return false;
	return true;
}

// Return true if this type is already in the union. Note: linear search, but number of types is usually small
bool UnionType::findType(Type* ty) {
	std::list<UnionElement>::iterator it;
	for (it = li.begin(); it != li.end(); it++) {
		if (*it->type == *ty)
			return true;
	}
	return false;
}
