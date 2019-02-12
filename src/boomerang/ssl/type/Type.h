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

#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/util/Types.h"

#include <QString>

#include <cassert>
#include <memory>


class Exp;
class OStream;
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
    Unsigned       = -1,
    Unknown        = 0,
    Signed         = 1,
    SignedStrong   = 2
};


/**
 * Base class for all types.
 * Types contain low level type information.
 * Note that we may have a completely different system for
 * recording high level types (i.e. class hierachy)
 */
class BOOMERANG_API Type : public std::enable_shared_from_this<Type>
{
public:
    typedef uint64 Size;

public:
    // Constructors
    Type(TypeClass id);
    Type(const Type &other) = default;
    Type(Type &&other)      = default;

    virtual ~Type();

    Type &operator=(const Type &other) = default;
    Type &operator=(Type &&other) = default;

public:
    // Comparisons
    virtual bool operator==(const Type &other) const = 0; ///< Considers sign
    virtual bool operator!=(const Type &other) const;     ///< Considers sign
    virtual bool operator<(const Type &other) const = 0;  ///< Considers sign

public:
    /// \returns the type class of this type.
    inline TypeClass getId() const { return m_id; }

    // runtime type information. Deprecated for most situations; use resolvesToTYPE()
    // clang-format off
    inline bool isVoid()     const { return getId() == TypeClass::Void;     }
    inline bool isFunc()     const { return getId() == TypeClass::Func;     }
    inline bool isBoolean()  const { return getId() == TypeClass::Boolean;  }
    inline bool isChar()     const { return getId() == TypeClass::Char;     }
    inline bool isInteger()  const { return getId() == TypeClass::Integer;  }
    inline bool isFloat()    const { return getId() == TypeClass::Float;    }
    inline bool isPointer()  const { return getId() == TypeClass::Pointer;  }
    inline bool isArray()    const { return getId() == TypeClass::Array;    }
    inline bool isNamed()    const { return getId() == TypeClass::Named;    }
    inline bool isCompound() const { return getId() == TypeClass::Compound; }
    inline bool isUnion()    const { return getId() == TypeClass::Union;    }
    inline bool isSize()     const { return getId() == TypeClass::Size;     }
    // clang-format on

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
    bool resolvesToFuncPtr() const;

    /// \returns false if some info is missing, e.g. unknown sign, size or basic type
    virtual bool isComplete() { return true; }

    /// \returns true if this type is a (const) char* pointer or char array.
    bool isCString() const;

    /**
     * When all=false (default), return true if can use this and other interchangeably; in
     * particular, if at most one of the types is compound and the first element is compatible with
     * the other, then the types are considered compatible. With all set to true, if one or both
     * types is compound, all corresponding elements must be compatible
     */
    virtual bool isCompatibleWith(const Type &other, bool all = false) const;

    /// Return true if this is a subset or equal to other
    bool isSubTypeOrEqual(SharedType other) const;

public:
    /// Typecast this type to another type.
    template<class T>
    typename std::enable_if<std::is_base_of<Type, T>::value, std::shared_ptr<T>>::type as();

    template<class T>
    typename std::enable_if<std::is_base_of<Type, T>::value, std::shared_ptr<const T>>::type
    as() const;

public:
    /// Clone this type
    virtual SharedType clone() const = 0;

    /// \returns the size (in bits) of this type.
    virtual Size getSize() const = 0;

    /// \returns the size (in bytes) of this type.
    /// Does not include struct padding.
    Size getSizeInBytes() const { return (getSize() + 7) / 8; }

    /// Changes the bit size of this type.
    virtual void setSize(Size newSize);

public:
    /// Resolve the original type across named types.
    /// If the type is not named, return this.
    SharedType resolveNamedType();
    SharedConstType resolveNamedType() const;

    /// Add a named ("typedef'd") type to the global type list.
    static void addNamedType(const QString &name, SharedType type);

    /// \returns the actual type of the named type with name \p name
    static SharedType getNamedType(const QString &name);

    /**
     * Clear the named type map. This is necessary when testing; the
     * type for the first parameter to 'main' is different for SPARC and Pentium
     */
    static void clearNamedTypes();

    /// Create a union of this Type and other. Set \p changed to true if any change
    SharedType createUnion(SharedType other, bool &changed, bool useHighestPtr = false) const;

    /// \returns a new Bool/Char/Int
    static SharedType newIntegerLikeType(Size sizeInBits, Sign signedness);

public:
    QString toString() const;

    /// Get the C type, e.g. "unsigned int". If not final, include comment for lack of sign
    /// information. When final, choose a signedness etc
    virtual QString getCtype(bool final = false) const = 0;

    /**
     * For data-flow-based type analysis only: implement the meet operator.
     * Set \p changed true if any change. If \p useHighestPtr is true,
     * then if this and other are non void* pointers, set the result to the
     * *highest* possible type compatible with both (i.e. this JOIN other)
     * \todo the best possible thing would be to have both types as const
     */
    virtual SharedType meetWith(SharedType other, bool &changed,
                                bool useHighestPtr = false) const = 0;

protected:
    /**
     * isCompatible does most of the work; isCompatibleWith looks for complex types in other, and if
     * so reverses the parameters (this and other) to prevent many tedious repetitions
     */
    virtual bool isCompatible(const Type &other, bool all) const = 0;

protected:
    TypeClass m_id;
};


// Not part of the Type class, but logically belongs with it:
OStream &operator<<(OStream &os, const SharedConstType &ty); ///< Print the Type pointed to by t
OStream &operator<<(OStream &os, const Type &ty);            ///< Print the Type pointed to by t


template<class T>
inline typename std::enable_if<std::is_base_of<Type, T>::value, std::shared_ptr<T>>::type Type::as()
{
    SharedType ty = resolveNamedType();
    assert(std::dynamic_pointer_cast<T>(ty) != nullptr);
    return std::static_pointer_cast<T>(ty);
}


template<class T>
inline typename std::enable_if<std::is_base_of<Type, T>::value, std::shared_ptr<const T>>::type
Type::as() const
{
    SharedConstType ty = resolveNamedType();
    assert(std::dynamic_pointer_cast<const T>(ty) != nullptr);
    return std::static_pointer_cast<const T>(ty);
}
