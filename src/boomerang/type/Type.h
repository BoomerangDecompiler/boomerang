#pragma once

/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *               Note that we may have a compeltely different system for
 *                recording high level types
 ******************************************************************************/

#include "boomerang/util/Address.h"
#include "boomerang/util/Util.h"

#include <QString>
#include <QMap>
#include <QHash>
#include <QTextStream>

#include <string>
#include <map>
#include <memory>
#include <functional> // For binary_function
#include <vector>
#include <cassert>
#include <list>
#include <set>
#include <unordered_set>
#include <fstream>

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
class DataIntervalMap;
using SharedExp = std::shared_ptr<Exp>;

enum eType
{
    eVoid,
    eFunc,
    eBoolean,
    eChar,
    eInteger,
    eFloat,
    ePointer,
    eArray,
    eNamed,
    eCompound,
    eUnion,
    eSize,
    eUpper,
    eLower
}; // For operator< mostly

// TODO: untangle the dynamic-size types from static size types ( Int vs Boolean etc. )
// The following two are for Type::compForAddress()
struct ComplexTypeComp
{
    bool isArray;
    struct
    {
        QString  memberName; // Member name if offset
        unsigned index;      // Constant index if array
    }

         u;
};

typedef std::list<ComplexTypeComp>    ComplexTypeCompList;
class Type;
typedef std::shared_ptr<Type>         SharedType;
typedef std::shared_ptr<const Type>   SharedConstType;

class Type : public std::enable_shared_from_this<Type>, public Printable
{
protected:
    eType id;

private:
    static QMap<QString, SharedType> namedTypes;

public:
    // Constructors
    Type(eType id);
    virtual ~Type();
    eType getId() const { return id; }

    static void addNamedType(const QString& name, SharedType type);
    static SharedType getNamedType(const QString& name);

    /***************************************************************************/ /**
    * \brief   Given the name of a temporary variable, return its Type
    * \param   name reference to a string (e.g. "tmp", "tmpd")
    * \returns       Ptr to a new Type object
    ******************************************************************************/
    static SharedType getTempType(const QString& name);

    /***************************************************************************/ /**
    * \brief        static Constructor from string
    * \param        str string to parse
    * \returns      constructed type.
    ******************************************************************************/
    static SharedType parseType(const char *str); // parse a C type

    bool isCString() const;

    // runtime type information. Deprecated for most situations; use resolvesToTYPE()
    virtual bool isVoid() const { return false; }
    virtual bool isFunc() const { return false; }
    virtual bool isBoolean() const { return false; }
    virtual bool isChar() const { return false; }
    virtual bool isInteger() const { return false; }
    virtual bool isFloat() const { return false; }
    virtual bool isPointer() const { return false; }
    virtual bool isArray() const { return false; }
    virtual bool isNamed() const { return false; }
    virtual bool isCompound() const { return false; }
    virtual bool isUnion() const { return false; }
    virtual bool isSize() const { return false; }
    virtual bool isUpper() const { return false; }
    virtual bool isLower() const { return false; }

    // Return false if some info is missing, e.g. unknown sign, size or basic type
    virtual bool isComplete() { return true; }

    // These replace type casts
    template<class T>
    std::shared_ptr<T> as();

    template<class T>
    std::shared_ptr<const T> as() const;


    // These replace calls to isNamed() and resolvesTo()
    bool resolvesToVoid() const;
    bool resolvesToFunc() const;
    bool resolvesToBoolean() const;
    bool resolvesToChar() const;
    bool resolvesToInteger() const;
    bool resolvesToFloat() const;
    bool resolvesToPointer() const;
    bool resolvesToArray() const;
    bool resolvesToCompound() const;
    bool resolvesToUnion() const;
    bool resolvesToSize() const;
    bool resolvesToUpper() const;
    bool resolvesToLower() const;

    // cloning
    virtual SharedType clone() const = 0;

    // Comparisons
    virtual bool operator==(const Type& other) const = 0; // Considers sign

    /***************************************************************************/ /**
    * \brief        Inequality comparsion.
    * \param        other - Type being compared to
    * \returns            this != other
    ******************************************************************************/
    virtual bool operator!=(const Type& other) const;     // Considers sign

    /***************************************************************************/ /**
    * \brief        Defines an ordering between Type's
    *               (and hence sets etc of Exp* using lessExpStar).
    * \param        other - Type being compared to
    * \returns      this is less than other
    ******************************************************************************/
    virtual bool operator<(const Type& other) const = 0; // Considers sign

    bool operator*=(const Type& other) const             // Consider only
    {
        return id == other.id;
    } // broad type

    /***************************************************************************/ /**
    * \brief        Match operation.
    * \param        pattern - Type to match
    * \returns            Exp list of bindings if match or nullptr
    ******************************************************************************/
    virtual SharedExp match(SharedType pattern);

    // Constraint-based TA: merge one type with another, e.g. size16 with integer-of-size-0 -> int16
    virtual SharedType mergeWith(SharedType /*other*/) const
    {
        assert(false);
        return nullptr;
    }

    // Acccess functions

    /**
     * \brief      Get the size of this type
     * \returns    Size of the type (in bits)
     */
    virtual size_t getSize() const = 0;

    size_t getBytes() const { return (getSize() + 7) / 8; }
    virtual void setSize(size_t /*sz*/) { assert(0); }

    // Print and format functions
    QString toString() const override;

    // Get the C type, e.g. "unsigned int". If not final, include comment for lack of sign information.
    // When final, choose a signedness etc
    virtual QString getCtype(bool final = false) const = 0;

    /// Print in *i32* format
    void starPrint(QTextStream& os);
    QString prints();                    // For debugging
    void dump();                         // For debugging
    static void dumpNames();             // For debugging

    /***************************************************************************/ /**
    * \brief  Return a minimal temporary name for this type. It'd be even
    *          nicer to return a unique name, but we don't know scope at
    *          this point, and even so we could still clash with a user-defined
    *          name later on :(
    * \returns        a string
    ******************************************************************************/
    virtual QString getTempName() const; // Get a temporary name for the type

    // Clear the named type map. This is necessary when testing; the
    // type for the first parameter to 'main' is different for sparc and pentium
    static void clearNamedTypes();

    bool isPointerToAlpha();

    // TODO: the best possible thing would be to have both types as const
    // For data-flow-based type analysis only: implement the meet operator. Set ch true if any change
    // If bHighestPtr is true, then if this and other are non void* pointers, set the result to the
    // *highest* possible type compatible with both (i.e. this JOIN other)
    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr = false) const = 0;

    // When all=false (default), return true if can use this and other interchangeably; in particular,
    // if at most one of the types is compound and the first element is compatible with the other, then
    // the types are considered compatible. With all set to true, if one or both types is compound, all
    // corresponding elements must be compatible
    virtual bool isCompatibleWith(const Type& other, bool all = false) const;

    // isCompatible does most of the work; isCompatibleWith looks for complex types in other, and if so
    // reverses the parameters (this and other) to prevent many tedious repetitions
    virtual bool isCompatible(const Type& other, bool all) const = 0;

    // Return true if this is a subset or equal to other
    bool isSubTypeOrEqual(SharedType other);

    // Create a union of this Type and other. Set ch true if any change
    SharedType createUnion(SharedType other, bool& ch, bool bHighestPtr = false) const;
    static SharedType newIntegerLikeType(int size, int signedness); // Return a new Bool/Char/Int

    // From a complex type like an array of structs with a float, return a list of components so you
    // can construct e.g. myarray1[8].mystruct2.myfloat7
    ComplexTypeCompList& compForAddress(Address addr, DataIntervalMap& dim);

    // Dereference this type. For most cases, return null unless you are a pointer type. But for a
    // union of pointers, return a new union with the dereference of all members. In dfa.cpp
    SharedType dereference();

protected:
};

class VoidType : public Type
{
public:
    VoidType();
    virtual ~VoidType();
    virtual bool isVoid() const override { return true; }

    virtual SharedType clone() const override;

    static std::shared_ptr<VoidType> get() { return std::make_shared<VoidType>(); }

    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    /***************************************************************************/ /**
    * \brief        Return a string representing this type
    * \param        final if true, this is final output
    * \returns      Pointer to a constant string of char
    ******************************************************************************/
    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};

class FuncType : public Type
{
private:
    std::shared_ptr<Signature> signature;

public:
    static std::shared_ptr<FuncType> get(const std::shared_ptr<Signature>& sig = nullptr) { return std::make_shared<FuncType>(sig); }
    FuncType(const std::shared_ptr<Signature>& sig = nullptr);
    virtual ~FuncType();
    virtual bool isFunc() const override { return true; }

    virtual SharedType clone() const override;

    Signature *getSignature() { return signature.get(); }
    void setSignature(std::shared_ptr<Signature>& sig) { signature = sig; }
    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    // Split the C type into return and parameter parts
    // As above, but split into the return and parameter parts
    void getReturnAndParam(QString& ret, QString& param);

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};


/***************************************************************************/ /**
 * \brief        Deep copy of this type
 * \returns            Copy of the type
 ******************************************************************************/
class IntegerType : public Type
{
private:
    mutable size_t size;    ///< Size in bits, e.g. 16
    mutable int signedness; ///< pos=signed, neg=unsigned, 0=unknown or evenly matched

public:
    explicit IntegerType(unsigned NumBits, int sign = 0)
        : Type(eInteger)
    {
        size       = NumBits;
        signedness = sign;
        // setSubclassData(NumBits);
    }

    static std::shared_ptr<IntegerType> get(unsigned NumBits, int sign = 0);

    virtual bool isInteger() const override { return true; }
    virtual bool isComplete()  override { return signedness != 0 && size != 0; }

    virtual SharedType clone() const override;

    /***************************************************************************/ /**
    * \brief        Equality comparsion.
    * \param        other - Type being compared to
    * \returns      *this == other
    ******************************************************************************/
    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;

    // FIXME: aren't mergeWith and meetWith really the same thing?
    // Merge this IntegerType with another
    virtual SharedType mergeWith(SharedType other) const override;
    virtual SharedExp match(SharedType pattern)  override;

    virtual size_t getSize() const override; // Get size in bits

    virtual void setSize(size_t sz)  override { size = sz; }
    // Is it signed? 0=unknown, pos=yes, neg = no
    bool isSigned() { return signedness >= 0; }   // True if not unsigned
    bool isUnsigned() { return signedness <= 0; } // True if not definately signed
    // A hint for signedness
    void bumpSigned(int sg) { signedness += sg; }
    // Set absolute signedness
    void setSigned(int sg) { signedness = sg; }
    // Get the signedness
    int getSignedness() const { return signedness; }

    // Get the C type as a string. If full, output comments re the lack of sign information (in IntegerTypes).
    virtual QString getCtype(bool final = false) const override;

    virtual QString getTempName() const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};


class FloatType : public Type
{
private:
    mutable size_t size; // Size in bits, e.g. 64

public:
    explicit FloatType(int sz = 64);
    static std::shared_ptr<FloatType> get(int sz = 64);

    virtual ~FloatType();
    virtual bool isFloat() const override { return true; }

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool          operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual void setSize(size_t sz)  override { size = sz; }

    virtual QString getCtype(bool final = false) const override;

    virtual QString getTempName() const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
}; // class FloatType


class BooleanType : public Type
{
public:
    BooleanType();
    virtual ~BooleanType();
    virtual bool isBoolean() const override { return true; }
    static std::shared_ptr<BooleanType> get() { return std::make_shared<BooleanType>(); }
    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};

class CharType : public Type
{
public:
    CharType();
    virtual ~CharType();
    virtual bool isChar() const override { return true; }

    virtual SharedType clone() const override;

    static std::shared_ptr<CharType> get() { return std::make_shared<CharType>(); }
    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};

class PointerType : public Type
{
private:
    mutable SharedType points_to;

public:
    PointerType(SharedType p);
    virtual ~PointerType();
    virtual bool isPointer() const override { return true; }
    void setPointsTo(SharedType p);

    SharedType getPointsTo() { return points_to; }
    const SharedType getPointsTo() const { return points_to; }
    static std::shared_ptr<PointerType> get(SharedType t) { return std::make_shared<PointerType>(t); }
    static std::shared_ptr<PointerType> newPtrAlpha();

    // Note: alpha is therefore a "reserved name" for types
    bool pointsToAlpha() const;
    int pointerDepth() const;            // Return 2 for **x
    SharedType getFinalPointsTo() const; // Return x for **x

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual void setSize(size_t sz)  override
    {
        Q_UNUSED(sz);
        assert(sz == STD_SIZE);
    }

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};

// we actually want unbounded arrays to still work correctly when
// computing aliases.. as such, we give them a very large bound
// and hope that no-one tries to alias beyond them
#define NO_BOUND    9999999

class ArrayType : public Type
{
private:
    mutable SharedType BaseType;
    mutable size_t Length;

public:
    ArrayType(SharedType p, unsigned _length);
    ArrayType(SharedType p);
    virtual ~ArrayType();
    virtual bool isArray() const override { return true; }
    SharedType getBaseType() { return BaseType; }
    const SharedType getBaseType() const { return BaseType; }
    void setBaseType(SharedType b);
    void fixBaseType(SharedType b);

    size_t getLength() const { return Length; }
    void setLength(unsigned n) { Length = n; }
    bool isUnbounded() const;

    virtual SharedType clone() const override;

    static std::shared_ptr<ArrayType> get(SharedType p, unsigned _length) { return std::make_shared<ArrayType>(p, _length); }
    static std::shared_ptr<ArrayType> get(SharedType p) { return std::make_shared<ArrayType>(p); }
    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;

    virtual bool isCompatibleWith(const Type& other, bool all = false) const override { return isCompatible(other, all); }
    virtual bool isCompatible(const Type& other, bool all) const override;

    size_t convertLength(SharedType b) const;

protected:
    ArrayType()
        : Type(eArray)
        , BaseType(nullptr)
        , Length(0) {}
};

class NamedType : public Type
{
private:
    QString name;
    static int nextAlpha;

public:
    NamedType(const QString& _name);
    virtual ~NamedType();
    virtual bool isNamed() const override { return true; }
    QString getName() const { return name; }
    SharedType resolvesTo() const;

    // Get a new type variable, e.g. alpha0, alpha55
    static std::shared_ptr<NamedType> get(const QString& _name) { return std::make_shared<NamedType>(_name); }
    static std::shared_ptr<NamedType> getAlpha();

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
};

// The compound type represents structures, not unions
class CompoundType : public Type
{
private:
    std::vector<SharedType> types;
    std::vector<QString> names;
    int nextGenericMemberNum;
    bool generic;

public:
    CompoundType(bool generic = false);
    virtual ~CompoundType();
    virtual bool isCompound() const override { return true; }

    void addType(SharedType n, const QString& str)
    {
        // check if it is a user defined type (typedef)
        SharedType t = getNamedType(n->getCtype());

        if (t) {
            n = t;
        }

        types.push_back(n);
        names.push_back(str);
    }

    size_t getNumTypes() const { return types.size(); }
    SharedType getType(unsigned n)
    {
        assert(n < getNumTypes());
        return types[n];
    }

    SharedType getType(const QString& nam);

    QString getName(unsigned n)
    {
        assert(n < getNumTypes());
        return names[n];
    }

    void setTypeAtOffset(unsigned n, SharedType ty);
    SharedType getTypeAtOffset(unsigned n);
    void setNameAtOffset(unsigned n, const QString& nam);
    QString getNameAtOffset(size_t n);

    bool isGeneric() { return generic; }

    // Update this compound to use the fact that offset off has type ty
    void updateGenericMember(int off, SharedType ty, bool& ch); // Add a new generic member if necessary
    unsigned getOffsetTo(unsigned n);
    unsigned getOffsetTo(const QString& member);
    unsigned getOffsetRemainder(unsigned n);

    virtual SharedType clone() const override;

    static std::shared_ptr<CompoundType> get(bool generic = false) { return std::make_shared<CompoundType>(generic); }
    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;


    // Return true if this is a superstructure of other,
    // i.e. we have the same types at the same offsets as other
    bool isSuperStructOf(const SharedType& other); // True if this is is a superstructure of other

    // Return true if this is a substructure of other,
    // i.e. other has the same types at the same offsets as this

    bool isSubStructOf(SharedType other) const;    // True if this is is a substructure of other

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;

    virtual bool isCompatibleWith(const Type& other, bool all = false) const override { return isCompatible(other, all); }
    virtual bool isCompatible(const Type& other, bool all) const override;
};

// The union type represents the union of any number of any other types
struct UnionElement
{
    SharedType type;
    QString    name;
    bool operator==(const UnionElement& other) const
    {
        return *type == *other.type;
    }
};

struct  hashUnionElem
{
    size_t operator()(const UnionElement& e) const
    {
        return qHash(e.type->getCtype());
    }
};

typedef std::unordered_set<UnionElement, hashUnionElem> UnionEntrySet;
class UnionType : public Type
{
private:
    // Note: list, not vector, as it is occasionally desirable to insert elements without affecting iterators
    // (e.g. meetWith(another Union))
    mutable UnionEntrySet li;

public:
    typedef UnionEntrySet::iterator ilUnionElement;
    UnionType();
    virtual ~UnionType();
    virtual bool isUnion() const override { return true; }
    static std::shared_ptr<UnionType> get() { return std::make_shared<UnionType>(); }
    void addType(SharedType n, const QString& str);

    size_t getNumTypes() const { return li.size(); }

    // Return true if this type is already in the union. Note: linear search, but number of types is usually small
    bool findType(SharedType ty); // Return true if ty is already in the union

    ilUnionElement begin() { return li.begin(); }
    ilUnionElement end() { return li.end(); }
    // Type        *getType(const char *nam);
    // const        char *getName(int n) { assert(n < getNumTypes()); return names[n].c_str(); }

    virtual SharedType clone() const override;

    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;
    virtual SharedExp match(SharedType pattern) override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;

    virtual bool isCompatibleWith(const Type& other, bool all) const override { return isCompatible(other, all); }
    virtual bool isCompatible(const Type& other, bool all) const override;

    // if this is a union of pointer types, get the union of things they point to. In dfa.cpp
    SharedType dereferenceUnion();
};

// This class is for before type analysis. Typically, you have no info at all, or only know the size (e.g.
// width of a register or memory transfer)
class SizeType : public Type
{
private:
    mutable size_t size; // Size in bits, e.g. 16

public:
    SizeType()
        : Type(eSize) {}
    SizeType(unsigned sz)
        : Type(eSize)
        , size(sz) {}
    virtual ~SizeType() {}
    virtual SharedType clone() const override;

    static std::shared_ptr<SizeType> get(unsigned sz) { return std::make_shared<SizeType>(sz); }
    static std::shared_ptr<SizeType> get() { return std::make_shared<SizeType>(); }
    virtual bool operator==(const Type& other) const override;
    virtual bool operator<(const Type& other) const override;

//    virtual SharedExp match(SharedType pattern);

    // Merge this SizeType with another type
    virtual SharedType mergeWith(SharedType other) const override;

    virtual size_t getSize() const override;

    virtual void setSize(size_t sz) override { size = sz; }
    virtual bool isSize() const override { return true; }
    virtual bool isComplete() override { return false; } // Basic type is unknown
    virtual QString getCtype(bool final = false) const override;
    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool) const override;
};


// This class represents the upper half of its base type
// Mainly needed to represent the upper and lower half for type double
class UpperType : public Type
{
    mutable SharedType base_type;

public:
    UpperType(SharedType base)
        : Type(eUpper)
        , base_type(base) {}
    virtual ~UpperType() {}
    virtual SharedType clone() const override;
    virtual bool operator==(const Type& other) const override;
    virtual bool operator<(const Type& other) const override;

    // virtual Exp         *match(SharedType pattern);
    virtual SharedType mergeWith(SharedType other) const override;

    SharedType getBaseType() { return base_type; }
    const SharedType getBaseType() const { return base_type; }
    void setBaseType(SharedType b) { base_type = b; }

    virtual size_t getSize() const override { return base_type->getSize() / 2; }
    virtual void setSize(size_t sz) override; // Does this make sense?

    virtual bool isUpper() const override { return true; }
    virtual bool isComplete() override { return base_type->isComplete(); }
    virtual QString getCtype(bool final = false) const override;
    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
}; // class UpperType

// As above, but stores the lower half
class LowerType : public Type
{
    mutable SharedType base_type;

public:
    LowerType(SharedType base)
        : Type(eUpper)
        , base_type(base) {}
    virtual ~LowerType() {}
    virtual SharedType clone() const override;
    virtual bool operator==(const Type& other) const override;
    virtual bool operator<(const Type& other) const override;

    // virtual Exp         *match(SharedType pattern);
    virtual SharedType mergeWith(SharedType other) const override;

    SharedType getBaseType() { return base_type; }
    const SharedType getBaseType() const { return base_type; }
    void setBaseType(SharedType b) { base_type = b; }

    virtual size_t getSize() const override { return base_type->getSize() / 2; }
    virtual void setSize(size_t sz) override; // Does this make sense?

    virtual bool isLower() const override { return true; }
    virtual bool isComplete() override { return base_type->isComplete(); }
    virtual QString getCtype(bool final = false) const override;
    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;
    virtual bool isCompatible(const Type& other, bool all) const override;
}; // class LowerType

/**
 * \class DataInterval.
 * \brief This class is used to represent local variables in procedures, and the global variables for the program.
 *
 * The concept is that the data space (the current procedure's stack or the global data space) has to
 * be partitioned into separate variables of various sizes and types. If a new variable is inserted that would cause
 * an overlap, the types have to be reconciled such that they no longer conflict (generally, the smaller type becomes a
 * member of the larger type, which has to be a structure or an array).
 * Each procedure and the Prog object have a map from ADDRESS (stack offset from sp{0} for locals, or native address for
 * globals), to an object of this class. A multimap is not needed, as the type of the entry specifies the overlapping.
 */

struct DataInterval
{
    size_t     size; ///< The size of this type in bytes
    QString    name; ///< The name of the variable
    SharedType type; ///< The type of the variable
};

typedef std::pair<const Address, DataInterval> DataIntervalEntry; // For result of find() below

class DataIntervalMap
{
    std::map<Address, DataInterval> dimap;
    UserProc *proc; // If used for locals, has ptr to UserProc, else nullptr

public:
    DataIntervalMap() {}
    typedef std::map<Address, DataInterval>::iterator iterator;

    void setProc(UserProc *p) { proc = p; }    ///< Initialise the proc pointer
    DataIntervalEntry *find(Address addr);     ///< Find the DataInterval at address addr, or nullptr if none

    // Find the entry that overlaps with addr. If none, return end().
    // We have to use upper_bound and decrement the iterator,
    // because we might want an entry that starts earlier than addr yet still overlaps it
    iterator find_it(Address addr);            ///< Return an iterator to the entry for it, or end() if none

    bool isClear(Address addr, unsigned size); ///< True if from addr for size bytes is clear

    // With the forced parameter: are we forcing the name, the type, or always both?
    /// Add a new data item
    void addItem(Address addr, QString name, SharedType ty, bool forced = false);
    void deleteItem(Address addr);             // Mainly for testing?
    void expandItem(Address addr, unsigned size);
    char *prints();                            // For test and debug
    void dump();                               // For debug

private:
    // We are entering an item that already exists in a larger type. Check for compatibility, meet if necessary.
    void enterComponent(DataIntervalEntry *pdie, Address addr, const QString&, SharedType ty, bool);

    // We are entering a struct or array that overlaps existing components. Check for compatibility, and move the
    // components out of the way, meeting if necessary
    void replaceComponents(Address addr, const QString& name, SharedType ty, bool);
    void checkMatching(DataIntervalEntry *pdie, Address addr, const QString&, SharedType ty, bool);
};

// Not part of the Type class, but logically belongs with it:

QTextStream& operator<<(QTextStream& os, const SharedConstType& t); // Print the Type pointed to by t
QTextStream& operator<<(QTextStream& os, const Type& t);            // Print the Type pointed to by t


template<class T>
inline std::shared_ptr<T> Type::as()
{
    SharedType ty = shared_from_this();

    if (isNamed()) {
        ty = std::static_pointer_cast<NamedType>(ty)->resolvesTo();
    }

    auto res = std::dynamic_pointer_cast<T>(ty);
    assert(res);
    return res;
}


template<class T>
inline std::shared_ptr<const T> Type::as() const
{
    SharedConstType ty = shared_from_this();

    if (isNamed()) {
        ty = std::static_pointer_cast<const NamedType>(ty)->resolvesTo();
    }

    auto res = std::dynamic_pointer_cast<const T>(ty);
    assert(res);
    return res;
}


template<>
inline std::shared_ptr<NamedType> Type::as<NamedType>()
{
    SharedType ty  = shared_from_this();
    auto       res = std::dynamic_pointer_cast<NamedType>(ty);

    assert(res);
    return res;
}


template<>
inline std::shared_ptr<const NamedType> Type::as<NamedType>() const
{
    auto ty  = shared_from_this();
    auto res = std::dynamic_pointer_cast<const NamedType>(ty);

    assert(res);
    return res;
}
