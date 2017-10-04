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


#include "boomerang/type/type/Type.h"

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


struct hashUnionElem
{
    size_t operator()(const UnionElement& e) const
    {
        return qHash(e.type->getCtype());
    }
};


class UnionType : public Type
{
public:
    typedef std::unordered_set<UnionElement, hashUnionElem> UnionEntrySet;
    typedef UnionEntrySet::iterator ilUnionElement;

public:
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

private:
    // Note: list, not vector, as it is occasionally desirable to insert elements without affecting iterators
    // (e.g. meetWith(another Union))
    mutable UnionEntrySet li;
};
