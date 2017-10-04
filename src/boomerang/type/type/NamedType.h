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

class NamedType : public Type
{
private:
    QString name;

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
