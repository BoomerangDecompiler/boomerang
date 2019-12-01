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


#include "boomerang/ssl/type/Type.h"

#include <map>


struct BOOMERANG_API lessType
{
    bool operator()(const SharedConstType &lhs, const SharedConstType &rhs) const;
};


/// UnionTypes represent the union construct of C/C++, where different types
/// share the same memory location.
/// During decompilation, UnionTypes are also used to resolve type conflicts
/// between unrelated types.
class BOOMERANG_API UnionType : public Type
{
public:
    typedef std::pair<SharedType, QString> Member;

public:
    /// Maps the type of a union member to its name.
    typedef std::map<SharedType, QString, lessType> UnionEntries;

public:
    /// Create a new empty union type.
    UnionType();

    /// Create a new union type with unnamed members.
    UnionType(const std::initializer_list<SharedType> members);

    /// Create a new union type with named members.
    UnionType(const std::initializer_list<Member> members);

    UnionType(const UnionType &other) = default;
    UnionType(UnionType &&other)      = default;

    ~UnionType() override;

    UnionType &operator=(const UnionType &other) = default;
    UnionType &operator=(UnionType &&other) = default;

public:
    static std::shared_ptr<UnionType> get();
    static std::shared_ptr<UnionType> get(const std::initializer_list<SharedType> members);
    static std::shared_ptr<UnionType> get(const std::initializer_list<Member> members);

    /// \copydoc Type::operator==
    bool operator==(const Type &other) const override;

    /// \copydoc Type::operator<
    bool operator<(const Type &other) const override;

    /// \copydoc Type::clone
    SharedType clone() const override;

    /// \copydoc Type::getSize
    Size getSize() const override;

    /// \copydoc Type::getCtype
    QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

    /// \copydoc Type::isCompatibleWith
    bool isCompatibleWith(const Type &other, bool all) const override;

public:
    /// \returns the number of distinct types in this union.
    size_t getNumTypes() const;

    /// \returns true if this type is already in the union.
    bool hasType(SharedType ty);

    /// If this union contains only 1 type, return the one and only member type.
    /// If this union has no types, return VoidType.
    /// Otherwise, return this.
    SharedType simplify(bool &changed) const;

protected:
    /// \copydoc Type::isCompatible
    bool isCompatible(const Type &other, bool all) const override;

private:
    /**
     * Add a new type to this union.
     * \param type the type of the new member
     * \param name the name of the new member
     */
    void addType(SharedType type, const QString &name = "");

private:
    UnionEntries m_entries;
};
