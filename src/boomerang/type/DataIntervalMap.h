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
#include "boomerang/util/Address.h"
#include "boomerang/util/IntervalMap.h"


class QString;
class UserProc;


struct TypedVariable
{
    TypedVariable(Address _baseAddr, QString _name, SharedType _type)
        : baseAddr(_baseAddr)
        , size(_type->getSize())
        , name(_name)
        , type(_type)
    {
    }

    Address baseAddr; ///< The address of the variable, or the SP offset for local variables
    size_t size;      ///< The size of this type in bits
    QString name;     ///< The name of the variable
    SharedType type;  ///< The type of the variable
};


/**
 * This class is used to represent local variables in procedures, and global variables for the
 * program.
 *
 * The concept is that the data space (the current procedure's stack or the global data space) has
 * to be partitioned into separate variables of various sizes and types. If a new variable is
 * inserted that would cause an overlap, the types have to be reconciled such that they no longer
 * conflict (generally, the smaller type becomes a member of the larger type, which has to be a
 * structure or an array). Each procedure and the Prog object have a map from Address (stack offset
 * from sp{0} for locals, or native address for globals), to an object of this class. A multimap is
 * not needed, as the type of the entry specifies the overlapping.
 */
class BOOMERANG_API DataIntervalMap
{
public:
    typedef IntervalMap<Address, TypedVariable> VariableMap;
    typedef VariableMap::iterator iterator;
    typedef VariableMap::const_iterator const_iterator;

public:
    /**
     * \param userProc The user proc for which the stack variables are determined,
     * or nullptr for the global variable table
     */
    DataIntervalMap(UserProc *userProc = nullptr);

    iterator begin() { return m_varMap.begin(); }
    iterator end() { return m_varMap.end(); }
    const_iterator begin() const { return m_varMap.begin(); }
    const_iterator end() const { return m_varMap.end(); }

public:
    /// \returns true iff the interval [addr; addr+size) does not contain a variable.
    bool isClear(Address addr, unsigned size) const;
    bool isClear(Address lower, Address upper) const;
    bool isClear(const Interval<Address> interval) const;

    /// \returns the variable that overlaps with address \p addr,
    /// or nullptr if no such variable exists.
    const TypedVariable *find(Address addr) const;

    /// \returns the iterator to the variable that overlaps with \p addr,
    /// or end() if no such variable exists.
    const_iterator find_it(Address addr) const;

    /**
     * Insert a new item into this map.
     * If the space of the new variable is occupied, insertion fails unless \p forced is set to
     * true. If \p forced is true, existing types are erased/split to make room for the new
     * variable.
     *
     * \param baseAddr the address or SP offset of the new variable.
     * \param name     The name of the new variable. If the name already exists, insertion fails
     *                 unless \p forced is specified.
     * \param type     The type of the new variable.
     * \param forced   If true, force insertion even if the space of the new variable is occupied.
     */
    iterator insertItem(Address baseAddr, QString name, SharedType type, bool forced = false);

    /// \deprecated For testing only
    void deleteItem(Address addr);

    /// For test and debug
    QString toString();

private:
    /// We are inserting an item that already exists in a larger type.
    /// Check for compatibility, meet if necessary.
    void insertComponentType(TypedVariable *variable, Address addr, const QString &name,
                             SharedType type, bool forced);

    // We are entering a struct or array that overlaps existing components. Check for compatibility,
    // and move the components out of the way, meeting if necessary
    iterator replaceComponents(Address addr, const QString &name, SharedType ty, bool);
    void checkMatching(TypedVariable *pdie, Address addr, const QString &, SharedType ty, bool);

    /**
     * Clears the interval \p interval from any intersecting types.
     * Unbounded arrays that may intersect the interval are resized to the beginning of
     * \p interval
     */
    void clearRange(const Interval<Address> &interval);

private:
    VariableMap m_varMap;
    UserProc *m_proc; ///< If used for locals, has ptr to UserProc, else nullptr
};
