/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *			   Note that we may have a compeltely different system for
 *				recording high level types
 *============================================================================*/

/*
 * $Revision$
 *
 * 20 Mar 01 - Mike: Added operator*= (compare, ignore sign, and consider all
 *					floats > 64 bits to be the same
 * 26 Apr 01 - Mike: Added class typeLessSI
 * 08 Apr 02 - Mike: Changes for boomerang
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include <string>
#include <map>
#include <functional>		// For binary_function
#include <vector>
#include <assert.h>
#include <list>
#include "memo.h"
#include "types.h"			// For STD_SIZE

class Signature;
class VoidType;
class FuncType;
class BooleanType;
class CharType;
class IntegerType;
class FloatType;
class NamedType;
class PointerType;
class ArrayType;
class CompoundType;
class UnionType;
class SizeType;
class Exp;
class XMLProgParser;

enum eType {eVoid, eFunc, eBoolean, eChar, eInteger, eFloat, ePointer,
	eArray, eNamed, eCompound, eUnion, eSize};	  // For operator< mostly

class Type : public Memoisable {
protected:
	eType id;
private:
	static std::map<std::string, Type*> namedTypes;

public:
	// Constructors
			Type(eType id);
virtual		~Type();
	eType	getId() const {return id;}

	static void addNamedType(const char *name, Type *type);
	static Type *getNamedType(const char *name);

	// Return type for given temporary variable name
	static Type* getTempType(const std::string &name);
	static Type* parseType(const char *str); // parse a C type

	// runtime type information
virtual bool isVoid()		const { return false; }
virtual bool isFunc()		const { return false; }
virtual bool isBoolean()	const { return false; }
virtual bool isChar()		const { return false; }
virtual bool isInteger() 	const { return false; }
virtual bool isFloat()		const { return false; }
virtual bool isPointer()	const { return false; }
virtual bool isArray()		const { return false; }
virtual bool isNamed()		const { return false; }
virtual bool isCompound()	const { return false; }
virtual bool isUnion()		const { return false; }
virtual bool isSize()		const { return false; }

// Return false if some info is missing, e.g. unknown sign, size or basic type
virtual bool isComplete() {return true;}

	// These replace type casts
	VoidType	*asVoid();
	FuncType	*asFunc();
	BooleanType	*asBoolean();
	CharType	*asChar();
	IntegerType	*asInteger();
	FloatType	*asFloat();
	NamedType	*asNamed();
	PointerType	*asPointer();
	ArrayType	*asArray();
	CompoundType *asCompound();
	UnionType	*asUnion();

	// These replace calls to isNamed() and resolvesTo()
	bool resolvesToVoid();
	bool resolvesToFunc();
	bool resolvesToBoolean();
	bool resolvesToChar();
	bool resolvesToInteger();
	bool resolvesToFloat();
	bool resolvesToPointer();
	bool resolvesToArray();
	bool resolvesToCompound();
	bool resolvesToUnion();

	// cloning
virtual Type* clone() const = 0;

	// Comparisons
virtual bool	operator==(const Type& other) const = 0;	// Considers sign
virtual bool	operator!=(const Type& other) const;		// Considers sign
//virtual bool	  operator-=(const Type& other) const = 0;	// Ignores sign
virtual bool	operator< (const Type& other) const = 0;	// Considers sign
		bool	operator*=(const Type& other) const {		// Consider only
					return id == other.id;}				 	// broad type
virtual Exp		*match(Type *pattern);
	// Merge one type with another, e.g. size16 with integer-of-size-0 -> int16
virtual Type*	mergeWith(Type* other) { assert(0); return 0; }

	// Acccess functions
virtual int		getSize() const = 0;
virtual void	setSize(int sz) {assert(0);}

	// Print and format functions
	// Get the C type, e.g. "unsigned int". If not final, include comment
	// for lack of sign information. When final, choose a signedness etc
virtual const char *getCtype(bool final = false) const = 0;
		// Print in *i32* format
		void	starPrint(std::ostream& os);
		const char*	prints();			// For debugging

virtual std::string getTempName() const; // Get a temporary name for the type

	// Clear the named type map. This is necessary when testing; the
	// type for the first parameter to 'main' is different for sparc and pentium
static	void	clearNamedTypes() { namedTypes.clear(); }

		bool	isPointerToAlpha();

		virtual Memo *makeMemo(int mId) { return new Memo(mId); }
		virtual void readMemo(Memo *m, bool dec) { }

				// For data-flow-based type analysis only: implement the meet operator
virtual Type*	meetWith(Type* other, bool& ch) = 0;
		Type*	createUnion(Type* other);		// Create a union of this Type and other

protected:
	friend class XMLProgParser;
};	// class Type

class VoidType : public Type {
public:
	VoidType();
virtual ~VoidType();
virtual bool isVoid() const { return true; }

virtual Type *clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp *match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};

class FuncType : public Type {
private:
	Signature *signature;
public:
	FuncType(Signature *sig = NULL);
virtual ~FuncType();
virtual bool isFunc() const { return true; }

virtual Type *clone() const;

		Signature *getSignature() { return signature; }

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp *match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

// Split the C type into return and parameter parts
		void	getReturnAndParam(const char*& ret, const char*& param);

		virtual Memo *makeMemo(int mId);
		virtual void readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};

class IntegerType : public Type {
private:
		int		size;				// Size in bits, e.g. 16
		int		signedness;			// pos=signed, neg=unsigned, 0=unknown or
									// evenly matched

public:
	IntegerType(int sz = 32, int sign = 0);
virtual 		~IntegerType();
virtual bool	isInteger() const { return true; }
virtual bool	isComplete() {return signedness != 0 && size != 0;}

virtual Type*	clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Type*	mergeWith(Type* other);
virtual Exp		*match(Type *pattern);

virtual int		getSize() const;
virtual void	setSize(int sz) {size = sz;}
		// Is it signed? 0=no, 1=yes, -1 = don't know
		bool	isSigned() { return signedness >= 0; }
		// A hint for signedness
		void	bumpSigned(int sg) { signedness += sg; }
		// Do we need this? Set absolute signedness
		void	setSigned(int sg) {signedness = sg; }
		// Get the signedness
		int		getSignedness() {return signedness;}

// Get the C type as a string. If full, output comments re the lack of sign
// information (in IntegerTypes).
virtual const char *getCtype(bool final = false) const;

virtual std::string getTempName() const;

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};	// class IntegerType

class FloatType : public Type {
private:
		int		size;				// Size in bits, e.g. 16

public:
				FloatType(int sz = 64);
virtual 		~FloatType();
virtual bool	isFloat() const { return true; }

virtual Type*	clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp		*match(Type *pattern);

virtual int		getSize() const;
virtual void	setSize(int sz) {size = sz;}

virtual const char *getCtype(bool final = false) const;

virtual std::string getTempName() const;

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};	// class FloatType

class BooleanType : public Type {
public:
	BooleanType();
virtual ~BooleanType();
virtual bool isBoolean() const { return true; }

virtual Type* clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp *match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};

class CharType : public Type {
public:
	CharType();
virtual ~CharType();
virtual bool isChar() const { return true; }

virtual Type* clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp *match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};

class PointerType : public Type {
private:
		Type	*points_to;

public:
				PointerType(Type *p);
virtual			~PointerType();
virtual bool	isPointer() const { return true; }
		void	setPointsTo(Type *p) { points_to = p; }
		Type	*getPointsTo() { return points_to; }
static PointerType* newPtrAlpha();
		bool	pointsToAlpha();

virtual Type*	clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp		*match(Type *pattern);

virtual int		getSize() const;
virtual void	setSize(int sz) {assert(sz == STD_SIZE);}

virtual const char *getCtype(bool final = false) const;

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};	// class PointerType

class ArrayType : public Type {
private:
		Type	*base_type;
		unsigned length;

public:
				ArrayType(Type *p, unsigned length);
				ArrayType(Type *p);
virtual 		~ArrayType();
virtual bool	isArray() const { return true; }
		Type	*getBaseType() { return base_type; }
		void	setBaseType(Type *b) { base_type = b; }
		void	fixBaseType(Type *b);
		unsigned getLength() { return length; }
		void	setLength(unsigned n) { length = n; }
		bool	isUnbounded();

virtual Type*	clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp		*match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

virtual	Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
	ArrayType() : Type(eArray), base_type(NULL), length(0) { }
};	// class ArrayType

class NamedType : public Type {
private:
		std::string name;
		static int nextAlpha;

public:
				NamedType(const char *name);
virtual 		~NamedType();
virtual bool	isNamed() const { return true; }
		const char *getName() { return name.c_str(); }
		Type	*resolvesTo() const;
		// Get a new type variable, e.g. alpha0, alpha55
static	NamedType *getAlpha();

virtual Type*	clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp		*match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};

// The compound type represents structures, not unions
class CompoundType : public Type {
private:
		std::vector<Type*> types;
		std::vector<std::string> names;

public:
				CompoundType();
virtual			~CompoundType();
virtual bool	isCompound() const { return true; }

		void addType(Type *n, const char *str) { 
					types.push_back(n); 
					names.push_back(str);
		}
		int		getNumTypes() { return types.size(); }
		Type	*getType(int n) { assert(n < getNumTypes()); return types[n]; }
		Type	*getType(const char *nam);
		const char *getName(int n) { assert(n < getNumTypes()); return names[n].c_str(); }
		Type	*getTypeAtOffset(int n);
		const char *getNameAtOffset(int n);
		int		getOffsetTo(int n);
		int		getOffsetTo(const char *member);
		int		getOffsetRemainder(int n);

virtual Type* clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp *match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

		bool	isSuperStructOf(Type* other);		// True if this is is a superstructure of other
		bool	isSubStructOf(Type* other);			// True if this is is a substructure of other

virtual Memo	*makeMemo(int mId);
virtual void	readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};	// class CompoundType

// The union type represents the union of any number of any other types
class UnionType : public Type {
private:
	std::vector<Type*> types;
	std::vector<std::string> names;

public:
			UnionType();
virtual ~UnionType();
virtual bool isUnion() const { return true; }

	void	addType(Type *n, const char *str);
	int		getNumTypes() const { return types.size(); }
	Type	*getType(int n) { assert(n < getNumTypes()); return types[n]; }
	Type	*getType(const char *nam);
	const char *getName(int n) { assert(n < getNumTypes()); return names[n].c_str(); }

virtual Type* clone() const;

virtual bool	operator==(const Type& other) const;
//virtual bool	  operator-=(const Type& other) const;
virtual bool	operator< (const Type& other) const;
virtual Exp *match(Type *pattern);

virtual int		getSize() const;

virtual const char *getCtype(bool final = false) const;

		virtual Memo *makeMemo(int mId);
		virtual void readMemo(Memo *m, bool dec);

virtual Type*	meetWith(Type* other, bool& ch);

protected:
	friend class XMLProgParser;
};	// class UnionType

// This class is for before type analysis. Typically, you have no info at
// all, or only know the size (e.g. width of a register or memory transfer)
class SizeType : public Type {
private:
	int			size;				// Size in bits, e.g. 16
public:
				SizeType() : Type(eSize) {}
				SizeType(int sz) : Type(eSize), size(sz) {}
virtual			~SizeType() {}
virtual Type*	clone() const;
virtual bool	operator==(const Type& other) const;
virtual bool	operator< (const Type& other) const;
//virtual Exp	  *match(Type *pattern);
virtual Type*	mergeWith(Type* other);

virtual int		getSize() const;
virtual void	setSize(int sz) {size = sz;}
virtual bool	isSize() const { return true; }
virtual bool	isComplete() {return false;}	// Basic type is unknown
virtual const char* getCtype(bool final = false) const;
virtual Type*	meetWith(Type* other, bool& ch);

};	// class SizeType

// Not part of the Type class, but logically belongs with it:
std::ostream& operator<<(std::ostream& os, Type* t);  // Print the Type
													  //  pointed to by t


#endif	// __TYPE_H__
