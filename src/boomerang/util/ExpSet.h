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


#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/OStream.h"

#include <memory>
#include <set>
#include <unordered_set>



/**
 * A class ordered or unordered sets of expressions.
 * \tparam T the type of expression to store in the set
 * \tparam Sorter Binary functor type that defines the sorting order.
 *                If Sorter == void, the set is unordered.
 */
template<
        typename T,
        typename Sorter = void,
        typename Enabler = std::enable_if<std::is_base_of<Exp, T>::value>
    >
class ExpSet
{
protected:
    using Set = typename std::conditional<std::is_void<Sorter>::value,
        std::unordered_set<std::shared_ptr<T>>, std::set<std::shared_ptr<T>, Sorter>>::type;

public:
    typedef typename Set::iterator       iterator;
    typedef typename Set::const_iterator const_iterator;

public:
    bool operator!=(const ExpSet& other) const { return !(*this == other); }
    bool operator==(const ExpSet& other) const
    {
        // We want to compare the locations, not the pointers
        if (size() != other.size()) {
            return false;
        }

        return std::equal(begin(), end(), other.begin(),
            [](const std::shared_ptr<T>& exp1, const std::shared_ptr<T>& exp2) {
                return *exp1 == *exp2;
            });
    }

public:
    iterator begin() { return m_set.begin(); }
    iterator end()   { return m_set.end();   }
    const_iterator begin() const { return m_set.begin(); }
    const_iterator end()   const { return m_set.end();   }

public:
    bool empty() const { return m_set.empty(); }
    int size() const { return m_set.size(); }
    void clear() { m_set.clear(); }

    /// Insert the given expression
    void insert(const std::shared_ptr<T>& exp) { m_set.insert(exp); }

    /// \param loc is not modified, and could be const'd if not for std::set requirements
    void remove(const std::shared_ptr<T>& loc)
    {
        iterator it = m_set.find(loc);

        if (it != end()) {
            m_set.erase(it);
        }
    }

    /// Remove location, given iterator
    iterator erase(iterator ll) { return m_set.erase(ll); }

    /// Return true if the expression exists in the set
    bool contains(const std::shared_ptr<const T>& exp) const
    {
        return m_set.find(std::const_pointer_cast<T>(exp)) != m_set.end();
    }

    /// Make this set the union of itself and other
    void makeUnion(const ExpSet& other)
    {
        for (const std::shared_ptr<T>& exp : other) {
            m_set.insert(exp);
        }
    }

    /// Make this set the set difference of itself and other
    void makeDiff(const ExpSet& other)
    {
        for (const SharedExp& exp : other) {
            m_set.erase(exp);
        }
    }

    void print(OStream& os) const
    {
        for (auto it = begin(); it != end(); ++it) {
            if (it != begin()) {
                os << ",\t";
            }

            os << *it;
        }
    }
protected:
    Set m_set;
};

