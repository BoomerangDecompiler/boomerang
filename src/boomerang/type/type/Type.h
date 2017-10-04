#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


/***************************************************************************/ /**
 * \file       type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *             Note that we may have a completely different system for
 *             recording high level types
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


enum TypeID
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
    eSize
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
    } u;
};

typedef std::list<ComplexTypeComp>    ComplexTypeCompList;
class Type;
typedef std::shared_ptr<Type>         SharedType;
typedef std::shared_ptr<const Type>   SharedConstType;


/**
 * Base class for all types.
 */
class Type : public std::enable_shared_from_this<Type>, public Printable
{
public:
    // Constructors
    Type(TypeID id);
    virtual ~Type();

    /// \returns the type class (ID) of this type.
    TypeID getId() const { return id; }

    /// Add a named ("typedef'd") type to the global type list.
    static void addNamedType(const QString& name, SharedType type);

    /// \returns the actual type of the named type with name \p name
    static SharedType getNamedType(const QString& name);

    /***************************************************************************/ /**
    * \brief   Given the name of a temporary variable, return its Type
    * \param   name reference to a string (e.g. "tmp", "tmpd")
    * \returns Ptr to a new Type object
    ******************************************************************************/
    static SharedType getTempType(const QString& name);

    /***************************************************************************/ /**
    * \brief        static Constructor from string
    * \param        str string to parse
    * \returns      constructed type.
    ******************************************************************************/
    static SharedType parseType(const char *str); // parse a C type

    /// \returns true if this type is a (const) char* pointer or char array.
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

    /// \returns false if some info is missing, e.g. unknown sign, size or basic type
    virtual bool isComplete() { return true; }

    /// Typecast this type to another type.
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
    virtual bool operator==(const Type& other) const = 0; ///< Considers sign
    virtual bool operator!=(const Type& other) const;     ///< Considers sign
    virtual bool operator<(const Type& other) const = 0;  ///< Considers sign

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

    /// \returns the size (in bits) of this type.
    virtual size_t getSize() const = 0;

    /// Changes the bit size of this type.
    virtual void setSize(size_t /*sz*/) { assert(false); /* Redefined in subclasses. */ }

    /// \returns the size (in bytes) of this type.
    /// Does not include struct padding.
    size_t getSizeInBytes() const { return (getSize() + 7) / 8; }

    /// \copydoc Printable::toString
    QString toString() const override;

    /// Get the C type, e.g. "unsigned int". If not final, include comment for lack of sign information.
    /// When final, choose a signedness etc
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

    /// Return true if this is a subset or equal to other
    bool isSubTypeOrEqual(SharedType other);

    /// Create a union of this Type and other. Set ch true if any change
    SharedType createUnion(SharedType other, bool& ch, bool bHighestPtr = false) const;
    static SharedType newIntegerLikeType(int size, int signedness); // Return a new Bool/Char/Int

    /// From a complex type like an array of structs with a float, return a list of components so you
    /// can construct e.g. myarray1[8].mystruct2.myfloat7
    ComplexTypeCompList& compForAddress(Address addr, DataIntervalMap& dim);

    /// Dereference this type. For most cases, return null unless you are a pointer type. But for a
    /// union of pointers, return a new union with the dereference of all members. In dfa.cpp
    SharedType dereference();

protected:
    TypeID id;
};


// Not part of the Type class, but logically belongs with it:
QTextStream& operator<<(QTextStream& os, const SharedConstType& t); ///< Print the Type pointed to by t
QTextStream& operator<<(QTextStream& os, const Type& t);            ///< Print the Type pointed to by t

#include "boomerang/type/type/NamedType.h"

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
