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
 * FILE:       type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *             Note that we may have a compeltely different system for
 *              recording high level types
 *============================================================================*/

/*
 * $Revision$
 *
 * 20 Mar 01 - Mike: Added operator*= (compare, ignore sign, and consider all
 *                  floats > 64 bits to be the same
 * 26 Apr 01 - Mike: Added class typeLessSI
 * 08 Apr 02 - Mike: Changes for boomerang
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include <string>
#include <map>
#include <functional>       // For binary_function
#include "gc_cpp.h"

class Signature;

enum eType {eVoid, eFunc, eBoolean, eChar, eInteger, eFloat, ePointer,
    eArray, eNamed};    // For operator< only

class Type {
protected:
    eType id;
private:
    static std::map<std::string, Type*> namedTypes;

public:
    // Constructors
            Type(eType id);
virtual		~Type();
    eType   getId() const {return id;}

    static void addNamedType(const char *name, Type *type);
    static Type *getNamedType(const char *name);

    // Return type for given temporary variable name
    static Type* getTempType(const std::string &name);
    static Type* parseType(const char *str); // parse a C type

    // runtime type information
virtual bool isVoid() const { return false; }
virtual bool isFunc() const { return false; }
virtual bool isBoolean() const { return false; }
virtual bool isChar() const { return false; }
virtual bool isInteger() const { return false; }
virtual bool isFloat() const { return false; }
virtual bool isPointer() const { return false; }
virtual bool isArray() const { return false; }
virtual bool isNamed() const { return false; }

    // cloning
virtual Type* clone() const = 0;

    // Comparisons
virtual bool    operator==(const Type& other) const = 0;// Considers sign
virtual bool    operator!=(const Type& other) const;    // Considers sign
//virtual bool    operator-=(const Type& other) const = 0;// Ignores sign
virtual bool    operator< (const Type& other) const = 0;// Considers sign

    // Access functions
virtual int     getSize() const = 0;

    // Format functions
virtual const char *getCtype() const = 0;   // Get the C type, e.g. "unsigned int16"

virtual std::string getTempName() const; // Get a temporary name for the type

};

class VoidType : public Type {
public:
	VoidType();
virtual ~VoidType();
virtual bool isVoid() const { return true; }

virtual Type *clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};

class FuncType : public Type {
private:
	Signature *signature;
public:
	FuncType(Signature *sig = NULL);
virtual ~FuncType();
virtual bool isFunc() const { return true; }

virtual Type *clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};

class IntegerType : public Type {
private:
    int         size;               // Size in bits, e.g. 16
    bool        signd;              // True if a signed quantity

public:
	IntegerType(int sz = 32, bool sign = true);
virtual ~IntegerType();
virtual bool isInteger() const { return true; }

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;
        bool    isSigned() { return signd; }
        void    setSigned(bool b) { signd = b; }

virtual const char *getCtype() const;

virtual std::string getTempName() const;

};

class FloatType : public Type {
private:
    int         size;               // Size in bits, e.g. 16

public:
	FloatType(int sz = 64);
virtual ~FloatType();
virtual bool isFloat() const { return true; }

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

virtual std::string getTempName() const;

};

class BooleanType : public Type {
public:
	BooleanType();
virtual ~BooleanType();
virtual bool isBoolean() const { return true; }

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};

class CharType : public Type {
public:
	CharType();
virtual ~CharType();
virtual bool isChar() const { return true; }

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};


class PointerType : public Type {
private:
    Type *points_to;

public:
	PointerType(Type *p);
virtual ~PointerType();
virtual bool isPointer() const { return true; }
        Type *getPointsTo() { return points_to; }
static  PointerType* getPtrAlpha();

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};

class ArrayType : public Type {
private:
    Type *base_type;
    unsigned length;

public:
	ArrayType(Type *p, unsigned length);
virtual ~ArrayType();
virtual bool isArray() const { return true; }
        Type *getBaseType() { return base_type; }
        unsigned getLength() { return length; }

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};

class NamedType : public Type {
private:
    std::string name;
    static int nextAlpha;

public:
	NamedType(const char *name);
virtual ~NamedType();
virtual bool isNamed() const { return true; }
        const char *getName() { return name.c_str(); }
        // Get a new type variable, e.g. alpha0, alpha55
static  NamedType *getAlpha();

virtual Type* clone() const;

virtual bool    operator==(const Type& other) const;
//virtual bool    operator-=(const Type& other) const;
virtual bool    operator< (const Type& other) const;

virtual int     getSize() const;

virtual const char *getCtype() const;

};


#endif  // __TYPE_H__
