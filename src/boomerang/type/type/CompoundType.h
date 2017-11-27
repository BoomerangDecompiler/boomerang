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

/**
 * The compound type represents aggregate types like structures or classes.
 */
class CompoundType : public Type
{
public:
    /// Constructs an empty compound type.
    CompoundType(bool isGeneric = false);
    virtual ~CompoundType() override;

    /// \copydoc Type::isCompound
    virtual bool isCompound() const override { return true; }

    bool isGeneric() const;

    /**
     * Append a new member variable to this struct/class.
     * \param memberType the type of the new member variable.
     * \param memberName the new name of the member variable.
     */
    void addType(SharedType memberType, const QString& memberName);

    size_t getNumTypes() const { return m_types.size(); }

    /// \returns the type of the \p idx -th member variable.
    SharedType getType(unsigned idx);

    /// \returns the type of the member variable with name \p name
    SharedType getType(const QString& name);

    /// \retrns the name of the \p idx -th member variable.
    QString getName(unsigned idx);

    void setTypeAtOffset(unsigned offsetInBits, SharedType ty);
    SharedType getTypeAtOffset(unsigned offsetInBits);
    void setNameAtOffset(unsigned offsetInBits, const QString& nam);
    QString getNameAtOffset(size_t offsetInBits);

    // Update this compound to use the fact that offset off has type ty
    void updateGenericMember(int off, SharedType ty, bool& ch); // Add a new generic member if necessary
    unsigned getOffsetTo(unsigned n);
    unsigned getOffsetTo(const QString& member);
    unsigned getOffsetRemainder(unsigned n);

    virtual SharedType clone() const override;

    static std::shared_ptr<CompoundType> get(bool = false) { return std::make_shared<CompoundType>(); }
    virtual bool operator==(const Type& other) const override;

    // virtual bool        operator-=(const Type& other) const;
    virtual bool operator<(const Type& other) const override;

    virtual size_t getSize() const override;

    virtual QString getCtype(bool final = false) const override;


    /**
     * Return true if this is a superstructure of other,
     * i.e. we have the same types at the same offsets as other
     */
    bool isSuperStructOf(const SharedType& other);

    /**
     * Return true if this is a substructure of other,
     * i.e. other has the same types at the same offsets as this
     */
    bool isSubStructOf(SharedType other) const;

    virtual SharedType meetWith(SharedType other, bool& ch, bool bHighestPtr) const override;

    virtual bool isCompatibleWith(const Type& other, bool all = false) const override { return isCompatible(other, all); }
    virtual bool isCompatible(const Type& other, bool all) const override;

private:
    std::vector<SharedType> m_types;
    std::vector<QString> m_names;
    bool m_isGeneric;
    int m_nextGenericMemberNum;
};
