/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file       type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *               Note that we may have a compeltely different system for
 *                recording high level types
 *============================================================================*/

/*
 * $Revision$
 *
 * 20 Mar 01 - Mike: Added operator*= (compare, ignore sign, and consider all floats > 64 bits to be the same
 * 26 Apr 01 - Mike: Added class typeLessSI
 * 08 Apr 02 - Mike: Changes for boomerang
 * 25 Sep 04 - Mike: Added UnionType; beginnings of data-flow based type analysis
 * 26 Oct 04 - Mike: Added UpperType and LowerType; isCompatible()
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include <string>
#include <map>
#include <functional>        // For binary_function
#include <vector>
#include <assert.h>
#include <list>
#include <fstream>
#include "memo.h"
#include "types.h"            // For STD_SIZE

class Signature;
class UserProc;
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
class UpperType;
class LowerType;
class Exp;
class XMLProgParser;
class DataIntervalMap;

enum eType {eVoid, eFunc, eBoolean, eChar, eInteger, eFloat, ePointer, eArray, eNamed, eCompound, eUnion, eSize,
        eUpper, eLower};      // For operator< mostly

// The following two are for Type::compForAddress()
struct ComplexTypeComp {
        bool        isArray;
        union {
            char*    memberName;            // Member name if offset
            unsigned index;                // Constant index if array
        } u;
};
typedef std::list<ComplexTypeComp> ComplexTypeCompList;

class Type {
protected:
        eType        id;
private:
static    std::map<std::string, Type*> namedTypes;

public:
                    // Constructors
                    Type(eType id);
virtual                ~Type();
        eType        getId() const {return id;}

static void            addNamedType(const char *name, Type *type);
static Type            *getNamedType(const char *name);

                    // Return type for given temporary variable name
static Type*        getTempType(const std::string &name);
static Type*        parseType(const char *str); // parse a C type

bool    isCString();

                    // runtime type information. Deprecated for most situations; use resolvesToTYPE()
virtual bool        isVoid()        const { return false; }
virtual bool        isFunc()        const { return false; }
virtual bool        isBoolean()        const { return false; }
virtual bool        isChar()        const { return false; }
virtual bool        isInteger()     const { return false; }
virtual bool        isFloat()        const { return false; }
virtual bool        isPointer()        const { return false; }
virtual bool        isArray()        const { return false; }
virtual bool        isNamed()        const { return false; }
virtual bool        isCompound()    const { return false; }
virtual bool        isUnion()        const { return false; }
virtual bool        isSize()        const { return false; }
virtual bool        isUpper()        const { return false; }
virtual bool        isLower()        const { return false; }

// Return false if some info is missing, e.g. unknown sign, size or basic type
virtual bool        isComplete() {return true;}

                    // These replace type casts
        VoidType    *asVoid();
        FuncType    *asFunc();
        BooleanType    *asBoolean();
        CharType    *asChar();
        IntegerType    *asInteger();
        FloatType    *asFloat();
        NamedType    *asNamed();
        PointerType    *asPointer();
        ArrayType    *asArray();
        CompoundType *asCompound();
        UnionType    *asUnion();
        SizeType    *asSize();
        UpperType    *asUpper();
        LowerType    *asLower();

                    // These replace calls to isNamed() and resolvesTo()
        bool        resolvesToVoid();
        bool        resolvesToFunc();
        bool        resolvesToBoolean();
        bool        resolvesToChar();
        bool        resolvesToInteger();
        bool        resolvesToFloat();
        bool        resolvesToPointer();
        bool        resolvesToArray();
        bool        resolvesToCompound();
        bool        resolvesToUnion();
        bool        resolvesToSize();
        bool        resolvesToUpper();
        bool        resolvesToLower();

                    // cloning
virtual Type*        clone() const = 0;

                    // Comparisons
virtual bool        operator==(const Type& other) const = 0;    // Considers sign
virtual bool        operator!=(const Type& other) const;        // Considers sign
//virtual bool        operator-=(const Type& other) const = 0;    // Ignores sign
virtual bool        operator< (const Type& other) const = 0;    // Considers sign
        bool        operator*=(const Type& other) const {        // Consider only
                        return id == other.id;}                     // broad type
virtual Exp            *match(Type *pattern);
                    // Constraint-based TA: merge one type with another, e.g. size16 with integer-of-size-0 -> int16
virtual Type*        mergeWith(Type* other) { assert(0); return 0; }

                    // Acccess functions
virtual size_t      getSize() const = 0;
        unsigned    getBytes() const {return (getSize() + 7) / 8; }
virtual void        setSize(size_t sz) {assert(0);}

                    // Print and format functions
                    // Get the C type, e.g. "unsigned int". If not final, include comment for lack of sign information.
                    // When final, choose a signedness etc
virtual const char    *getCtype(bool final = false) const = 0;

        void        starPrint(std::ostream& os);
        const char* prints();            // For debugging
        void        dump();                // For debugging
static  void        dumpNames();        // For debugging

virtual std::string getTempName() const; // Get a temporary name for the type

                    // Clear the named type map. This is necessary when testing; the
                    // type for the first parameter to 'main' is different for sparc and pentium
static    void        clearNamedTypes() { namedTypes.clear(); }

        bool        isPointerToAlpha();

                    // For data-flow-based type analysis only: implement the meet operator. Set ch true if any change
                    // If bHighestPtr is true, then if this and other are non void* pointers, set the result to the
                    // *highest* possible type compatible with both (i.e. this JOIN other)
virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr = false) = 0;
                    // When all=false (default), return true if can use this and other interchangeably; in particular,
                    // if at most one of the types is compound and the first element is compatible with the other, then
                    // the types are considered compatible. With all set to true, if one or both types is compound, all
                    // corresponding elements must be compatible
virtual    bool        isCompatibleWith(Type* other, bool all = false);
                    // isCompatible does most of the work; isCompatibleWith looks for complex types in other, and if so
                    // reverses the parameters (this and other) to prevent many tedious repetitions
virtual bool        isCompatible(Type* other, bool all) = 0;
                    // Return true if this is a subset or equal to other
        bool        isSubTypeOrEqual(Type* other);
                    // Create a union of this Type and other. Set ch true if any change
        Type*        createUnion(Type* other, bool& ch, bool bHighestPtr = false);
static    Type*        newIntegerLikeType(int size, int signedness);    // Return a new Bool/Char/Int
                    // From a complex type like an array of structs with a float, return a list of components so you
                    // can construct e.g. myarray1[8].mystruct2.myfloat7
        ComplexTypeCompList& compForAddress(ADDRESS addr, DataIntervalMap& dim);
                    // Dereference this type. For most cases, return null unless you are a pointer type. But for a
                    // union of pointers, return a new union with the dereference of all members. In dfa.cpp
        Type*        dereference();

protected:
    friend class XMLProgParser;
};    // class Type

class VoidType : public Type {
public:
                    VoidType();
virtual                ~VoidType();
virtual bool        isVoid() const { return true; }

virtual Type        *clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool          operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp*        match(Type *pattern);

virtual size_t      getSize() const;

virtual const char* getCtype(bool final = false) const;

virtual Type*       meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};

class FuncType : public Type {
private:
    Signature*      signature;
public:
                    FuncType(Signature *sig = nullptr);
virtual             ~FuncType();
virtual bool        isFunc() const { return true; }

virtual Type*       clone() const;

        Signature*  getSignature() { return signature; }
        void        setSignature(Signature* sig) {signature = sig;}

virtual bool        operator==(const Type& other) const;
//virtual bool          operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp*        match(Type *pattern);

virtual size_t      getSize() const;

virtual const char* getCtype(bool final = false) const;

                    // Split the C type into return and parameter parts
        void        getReturnAndParam(const char*& ret, const char*& param);

virtual Type*       meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};

class IntegerType : public Type {
private:
        size_t      size;            // Size in bits, e.g. 16
        int         signedness;        // pos=signed, neg=unsigned, 0=unknown or evenly matched

public:
                    IntegerType(int sz = STD_SIZE, int sign = 0);
virtual             ~IntegerType();
virtual bool        isInteger() const { return true; }
virtual bool        isComplete() {return signedness != 0 && size != 0;}

virtual Type*       clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool          operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Type*       mergeWith(Type* other);
virtual Exp*        match(Type *pattern);

virtual size_t      getSize() const;            // Get size in bits
virtual void        setSize(size_t sz) {size = sz;}
                    // Is it signed? 0=unknown, pos=yes, neg = no
        bool        isSigned() { return signedness >= 0; }        // True if not unsigned
        bool        isUnsigned() {return signedness <= 0; }        // True if not definately signed
                    // A hint for signedness
        void        bumpSigned(int sg) { signedness += sg; }
                    // Set absolute signedness
        void        setSigned(int sg) {signedness = sg; }
                    // Get the signedness
        int         getSignedness() {return signedness;}

// Get the C type as a string. If full, output comments re the lack of sign information (in IntegerTypes).
virtual const char* getCtype(bool final = false) const;

virtual std::string getTempName() const;

virtual Type*       meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};    // class IntegerType

class FloatType : public Type {
private:
        unsigned    size;                // Size in bits, e.g. 64

public:
                    FloatType(int sz = 64);
virtual             ~FloatType();
virtual bool        isFloat() const { return true; }

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool          operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;
virtual void        setSize(size_t sz) {size = sz;}

virtual const char    *getCtype(bool final = false) const;

virtual std::string    getTempName() const;

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};    // class FloatType

class BooleanType : public Type {
public:
                    BooleanType();
virtual                ~BooleanType();
virtual bool        isBoolean() const { return true; }

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;

virtual const char    *getCtype(bool final = false) const;

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};

class CharType : public Type {
public:
                    CharType();
virtual                ~CharType();
virtual bool        isChar() const { return true; }

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;

virtual const char    *getCtype(bool final = false) const;

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};

class PointerType : public Type {
private:
        Type    *points_to;

public:
                    PointerType(Type *p);
virtual                ~PointerType();
virtual bool        isPointer() const { return true; }
        void        setPointsTo(Type *p);
        Type        *getPointsTo() { return points_to; }
static  PointerType *newPtrAlpha();
        bool        pointsToAlpha();
        int         pointerDepth();        // Return 2 for **x
        Type*       getFinalPointsTo();    // Return x for **x

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp *       match(Type *pattern);

virtual size_t      getSize() const;
virtual void        setSize(size_t sz) {assert(sz == STD_SIZE);}

virtual const char *getCtype(bool final = false) const;

virtual Type *      meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};    // class PointerType

class ArrayType : public Type {
private:
        Type        *base_type;
        unsigned    length;

public:
                    ArrayType(Type *p, unsigned length);
                    ArrayType(Type *p);
virtual             ~ArrayType();
virtual bool        isArray() const { return true; }
        Type        *getBaseType() { return base_type; }
        void        setBaseType(Type *b);
        void        fixBaseType(Type *b);
        unsigned    getLength() { return length; }
        void        setLength(unsigned n) { length = n; }
        bool        isUnbounded() const;

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;

virtual const char    *getCtype(bool final = false) const;

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatibleWith(Type* other, bool all = false) {return isCompatible(other, all);}
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
                    ArrayType() : Type(eArray), base_type(nullptr), length(0) { }
};    // class ArrayType

class NamedType : public Type {
private:
        std::string name;
static    int            nextAlpha;

public:
                    NamedType(const char *name);
virtual             ~NamedType();
virtual bool        isNamed() const { return true; }
        const char    *getName() { return name.c_str(); }
        Type        *resolvesTo() const;
                    // Get a new type variable, e.g. alpha0, alpha55
static    NamedType    *getAlpha();

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;

virtual const char    *getCtype(bool final = false) const;

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};        // class NamedType

// The compound type represents structures, not unions
class CompoundType : public Type {
private:
        std::vector<Type*> types;
        std::vector<std::string> names;
        int            nextGenericMemberNum;
        bool        generic;
public:
                    CompoundType(bool generic = false);
virtual                ~CompoundType();
virtual bool        isCompound() const { return true; }

        void        addType(Type *n, const char *str) {
                        // check if it is a user defined type (typedef)
                        Type *t=getNamedType(n->getCtype());
                        if ( t ) n = t;
                        types.push_back(n);
                        names.push_back(str);
                    }
        unsigned    getNumTypes() { return types.size(); }
        Type        *getType(unsigned n) { assert(n < getNumTypes()); return types[n]; }
        Type        *getType(const char *nam);
        const char    *getName(unsigned n) { assert(n < getNumTypes()); return names[n].c_str(); }
        void        setTypeAtOffset(unsigned n, Type* ty);
        Type        *getTypeAtOffset(unsigned n);
        void        setNameAtOffset(unsigned n, const char *nam);
        const char    *getNameAtOffset(unsigned n);
        bool        isGeneric() {return generic;}
        void        updateGenericMember(int off, Type* ty, bool& ch);    // Add a new generic member if necessary
        unsigned    getOffsetTo(unsigned n);
        unsigned    getOffsetTo(const char *member);
        unsigned    getOffsetRemainder(unsigned n);

virtual Type*        clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;

virtual const char *getCtype(bool final = false) const;

        bool        isSuperStructOf(Type* other);        // True if this is is a superstructure of other
        bool        isSubStructOf(Type* other);            // True if this is is a substructure of other

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatibleWith(Type* other, bool all = false) {return isCompatible(other, all);}
virtual bool        isCompatible(Type* other, bool all);

protected:
    friend class XMLProgParser;
};    // class CompoundType

// The union type represents the union of any number of any other types
struct UnionElement {
        Type*        type;
        std::string name;
};
class UnionType : public Type {
private:
        // Note: list, not vector, as it is occasionally desirable to insert elements without affecting iterators
        // (e.g. meetWith(another Union))
        std::list<UnionElement> li;

public:
                    UnionType();
virtual                ~UnionType();
virtual bool        isUnion() const { return true; }

        void        addType(Type *n, const char *str);
        int            getNumTypes() const { return li.size(); }
        bool        findType(Type* ty);                // Return true if ty is already in the union
        //Type        *getType(int n) { assert(n < getNumTypes()); return types[n]; }
        //Type        *getType(const char *nam);
        //const        char *getName(int n) { assert(n < getNumTypes()); return names[n].c_str(); }

virtual Type* clone() const;

virtual bool        operator==(const Type& other) const;
//virtual bool        operator-=(const Type& other) const;
virtual bool        operator< (const Type& other) const;
virtual Exp            *match(Type *pattern);

virtual size_t      getSize() const;

virtual const char *getCtype(bool final = false) const;

virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatibleWith(Type* other, bool all) {return isCompatible(other, all);}
virtual bool        isCompatible(Type* other, bool all);
                    // if this is a union of pointer types, get the union of things they point to. In dfa.cpp
        Type*        dereferenceUnion();

protected:
    friend class XMLProgParser;
};    // class UnionType

// This class is for before type analysis. Typically, you have no info at all, or only know the size (e.g.
// width of a register or memory transfer)
class SizeType : public Type {
private:
    size_t        size;                // Size in bits, e.g. 16
public:
                    SizeType() : Type(eSize) {}
                    SizeType(unsigned sz) : Type(eSize), size(sz) {}
virtual                ~SizeType() {}
virtual Type*        clone() const;
virtual bool        operator==(const Type& other) const;
virtual bool        operator< (const Type& other) const;
//virtual Exp          *match(Type *pattern);
virtual Type*        mergeWith(Type* other);

virtual size_t      getSize() const;
virtual void        setSize(size_t sz) {size = sz;}
virtual bool        isSize() const { return true; }
virtual bool        isComplete() {return false;}    // Basic type is unknown
virtual const char* getCtype(bool final = false) const;
virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

    friend class XMLProgParser;
};    // class SizeType

// This class represents the upper half of its base type
// Mainly needed to represent the upper and lower half for type double
class UpperType : public Type {
        Type*        base_type;

public:
                    UpperType(Type* base) : Type(eUpper), base_type(base) { }
virtual                ~UpperType() { }
virtual    Type*        clone() const;
virtual bool        operator==(const Type& other) const;
virtual bool        operator< (const Type& other) const;
//virtual Exp         *match(Type *pattern);
virtual Type*        mergeWith(Type* other);
        Type        *getBaseType() { return base_type; }
        void        setBaseType(Type *b) { base_type = b; }

virtual size_t      getSize() const {return base_type->getSize()/2;}
virtual void        setSize(size_t sz);        // Does this make sense?
virtual bool        isUpper() const { return true; }
virtual bool        isComplete() {return base_type->isComplete();}
virtual const char* getCtype(bool final = false) const;
virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

};  // class UpperType

// As above, but stores the lower half
class LowerType : public Type {
        Type*        base_type;

public:
                    LowerType(Type* base) : Type(eUpper), base_type(base) { }
virtual                ~LowerType() { }
virtual    Type*        clone() const;
virtual bool        operator==(const Type& other) const;
virtual bool        operator< (const Type& other) const;
//virtual Exp         *match(Type *pattern);
virtual Type*        mergeWith(Type* other);
        Type        *getBaseType() { return base_type; }
        void        setBaseType(Type *b) { base_type = b; }

virtual size_t      getSize() const {return base_type->getSize()/2;}
virtual void        setSize(size_t sz);        // Does this make sense?
virtual bool        isLower() const { return true; }
virtual bool        isComplete() {return base_type->isComplete();}
virtual const char* getCtype(bool final = false) const;
virtual Type*        meetWith(Type* other, bool& ch, bool bHighestPtr);
virtual bool        isCompatible(Type* other, bool all);

};  // class LowerType


/**
 * Class DataInterval. This class is used to represent local variables in procedures, and the global variables for
 * the program. The concept is that the data space (the current procedure's stack or the global data space) has to
 * be partitioned into separate variables of various sizes and types. If a new variable is inserted that would cause
 * an overlap, the types have to be reconciled such that they no longer conflict (generally, the smaller type becomes a
 * member of the larger type, which has to be a structure or an array).
 * Each procedure and the Prog object have a map from ADDRESS (stack offset from sp{0} for locals, or native address for
 * globals), to an object of this class. A multimap is not needed, as the type of the entry specifies the overlapping.
 */

struct DataInterval {
        size_t    size;                // The size of this type in bytes
        std::string    name;                // The name of the variable
        Type*        type;                // The type of the variable
};

typedef std::pair<const ADDRESS, DataInterval> DataIntervalEntry;        // For result of find() below

class DataIntervalMap {
        std::map<ADDRESS, DataInterval> dimap;
        UserProc*    proc;                            // If used for locals, has ptr to UserProc, else nullptr
public:
                    DataIntervalMap() {}
typedef    std::map<ADDRESS, DataInterval>::iterator iterator;
        void        setProc(UserProc* p) {proc = p;}// Initialise the proc pointer
        DataIntervalEntry* find(ADDRESS addr);        // Find the DataInterval at address addr, or nullptr if none
        iterator    find_it(ADDRESS addr);            // Return an iterator to the entry for it, or end() if none
        bool        isClear(ADDRESS addr, unsigned size);        // True if from addr for size bytes is clear
        void        addItem(ADDRESS addr, const char* name, Type* ty, bool forced = false);
        void        deleteItem(ADDRESS addr);        // Mainly for testing?
        void        expandItem(ADDRESS addr, unsigned size);
        char *      prints();                        // For test and debug
        void        dump();                            // For debug

private:
        void        enterComponent(DataIntervalEntry* pdie, ADDRESS addr, const char *name, Type* ty, bool forced);
        void        replaceComponents(ADDRESS addr, const char *name, Type* ty, bool forced);
        void        checkMatching(DataIntervalEntry* pdie, ADDRESS addr, const char* name, Type* ty, bool forced);
};

// Not part of the Type class, but logically belongs with it:
std::ostream& operator<<(std::ostream& os, Type* t);  // Print the Type pointed to by t


#endif    // __TYPE_H__
