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

#include <unordered_map>


struct BOOMERANG_API hashType
{
    size_t operator()(const SharedConstType &ty) const;
};

struct BOOMERANG_API equalType
{
    size_t operator()(const SharedConstType &lhs, const SharedConstType &rhs) const
    {
        return *lhs == *rhs;
    }
};


class BOOMERANG_API UnionType : public Type
{
public:
    typedef std::pair<SharedType, QString> Member;

public:
    typedef std::unordered_map<SharedType, QString, hashType, equalType> UnionEntries;

public:
    /// Create a new empty union type.
    UnionType();

    /// Create a new union type with unnamed members.
    UnionType(const std::initializer_list<SharedType> members);
    UnionType(const std::initializer_list<Member> members);

    UnionType(const UnionType &other) = default;
    UnionType(UnionType &&other)      = default;

    virtual ~UnionType() override;

    UnionType &operator=(const UnionType &other) = default;
    UnionType &operator=(UnionType &&other) = default;

public:
    static std::shared_ptr<UnionType> get() { return std::make_shared<UnionType>(); }
    static std::shared_ptr<UnionType> get(const std::initializer_list<SharedType> members)
    {
        return std::make_shared<UnionType>(members);
    }

    static std::shared_ptr<UnionType> get(const std::initializer_list<Member> members)
    {
        return std::make_shared<UnionType>(members);
    }

    size_t getNumTypes() const;

    // Return true if this type is already in the union. Note: linear search, but number of types is
    // usually small
    bool hasType(SharedType ty); // Return true if ty is already in the union

    virtual SharedType clone() const override;

    virtual bool operator==(const Type &other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type &other) const override;

    /// \returns the size in bits of this union
    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;

    /// \copydoc Type::meetWith
    virtual SharedType meetWith(SharedType other, bool &changed, bool useHighestPtr) const override;

    virtual bool isCompatibleWith(const Type &other, bool all) const override
    {
        return isCompatible(other, all);
    }

    virtual bool isCompatible(const Type &other, bool all) const override;

private:
    /**
     * Add a new type to this union
     * \param type the type of the new member
     * \param name the name of the new member
     */
    void addType(SharedType type, const QString &name = "");

    /// If this union contains only 1 type, return the one and only member type.
    /// If this union has no types, return VoidType.
    /// Otherwise, return this.
    SharedType simplify(bool &changed) const;

private:
    UnionEntries m_entries;
};
