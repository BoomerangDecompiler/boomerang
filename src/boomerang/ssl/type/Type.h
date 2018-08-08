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


#include "boomerang/ifc/IPrintable.h"

#include <cassert>
#include <memory>


class Exp;
class Type;

class QTextStream;

using SharedExp       = std::shared_ptr<Exp>;
using SharedType      = std::shared_ptr<Type>;
using SharedConstType = std::shared_ptr<const Type>;


/// For operator< mostly
enum class TypeClass
{
    Void,
    Func,
    Boolean,
    Char,
    Integer,
    Float,
    Pointer,
    Array,
    Named,
    Compound,
    Union,
    Size
};


/// Signedgess of integer-like variables
enum class Sign : int8_t
{
    UnsignedStrong = -2,
    Unsigned = -1,
    Unknown = 0,
    Signed = 1,
    SignedStrong = 2
};



/**
 * Base class for all types.
 * Types contain low level type information.
 * Note that we may have a completely different system for
 * recording high level types
 */
class BOOMERANG_API Type : public std::enable_shared_from_this<Type>, public IPrintable
{
public:
    // Constructors
    Type(TypeClass id);
    Type(const Type& other) = default;
    Type(Type&& other) = default;

    virtual ~Type() override;

    Type& operator=(const Type& other) = default;
    Type& operator=(Type&& other) = default;

public:
    /// \returns the type class of this type.
    TypeClass getId() const { return id; }

    /// Add a named ("typedef'd") type to the global type list.
    static void addNamedType(const QString& name, SharedType type);

    /// \returns the actual type of the named type with name \p name
    static SharedType getNamedType(const QString& name);

    /**
     * Given the name of a temporary variable, return its Type
     * \param   name reference to a string (e.g. "tmp", "tmpd")
     * \returns Ptr to a new Type object
     */
    static SharedType getTempType(const QString& name);

    /**
     * parse a C type from a string.
     * \param        str string to parse
     * \returns      constructed type.
     */
    static SharedType parseType(const char *str);

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

    /// Resolve the original type across named types.
    /// If the type is not named, return this.
    SharedType resolveNamedType();
    SharedConstType resolveNamedType() const;

    // cloning
    virtual SharedType clone() const = 0;

    // Comparisons
    virtual bool operator==(const Type& other) const = 0; ///< Considers sign
    virtual bool operator!=(const Type& other) const;     ///< Considers sign
    virtual bool operator<(const Type& other) const = 0;  ///< Considers sign


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

    QString prints();                    // For debugging

    /**
     * Return a minimal temporary name for this type. It'd be even
     * nicer to return a unique name, but we don't know scope at
     * this point, and even so we could still clash with a user-defined
     * name later on. :(
     */
    virtual QString getTempName() const; // Get a temporary name for the type

    /**
     * Clear the named type map. This is necessary when testing; the
     * type for the first parameter to 'main' is different for SPARC and Pentium
     */
    static void clearNamedTypes();

    /**
     * For data-flow-based type analysis only: implement the meet operator.
     * Set \p changed true if any change. If \p useHighestPtr is true,
     * then if this and other are non void* pointers, set the result to the
     * *highest* possible type compatible with both (i.e. this JOIN other)
     * \todo the best possible thing would be to have both types as const
     */
    virtual SharedType meetWith(SharedType other, bool& changed, bool useHighestPtr = false) const = 0;

    /**
     * When all=false (default), return true if can use this and other interchangeably; in particular,
     * if at most one of the types is compound and the first element is compatible with the other, then
     * the types are considered compatible. With all set to true, if one or both types is compound, all
     * corresponding elements must be compatible
     */
    virtual bool isCompatibleWith(const Type& other, bool all = false) const;

    /**
     * isCompatible does most of the work; isCompatibleWith looks for complex types in other, and if so
     * reverses the parameters (this and other) to prevent many tedious repetitions
     */
    virtual bool isCompatible(const Type& other, bool all) const = 0;

    /// Return true if this is a subset or equal to other
    bool isSubTypeOrEqual(SharedType other);

    /// Create a union of this Type and other. Set ch true if any change
    SharedType createUnion(SharedType other, bool& changed, bool useHighestPtr = false) const;

    static SharedType newIntegerLikeType(int size, Sign signedness); // Return a new Bool/Char/Int

    /// Dereference this type. For most cases, return null unless you are a pointer type. But for a
    /// union of pointers, return a new union with the dereference of all members. In dfa.cpp
    SharedType dereference();

protected:
    TypeClass id;
};


// Not part of the Type class, but logically belongs with it:
QTextStream& operator<<(QTextStream& os, const SharedConstType& ty); ///< Print the Type pointed to by t
QTextStream& operator<<(QTextStream& os, const Type& ty);            ///< Print the Type pointed to by t


template<class T>
inline std::shared_ptr<T> Type::as()
{
    SharedType ty = resolveNamedType();
    assert(std::dynamic_pointer_cast<T>(ty) != nullptr);
    return std::static_pointer_cast<T>(ty);
}


template<class T>
inline std::shared_ptr<const T> Type::as() const
{
    SharedConstType ty = resolveNamedType();
    assert(std::dynamic_pointer_cast<const T>(ty) != nullptr);
    return std::static_pointer_cast<const T>(ty);
}
