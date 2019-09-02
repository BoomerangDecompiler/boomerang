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


#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/Assign.h"

#include <memory>
#include <unordered_set>


using SharedExp = std::shared_ptr<class Exp>;


/**
 * This class implements an ordered or unordered set of statements.
 * \tparam T Statement type (Statement, Assign, CallStatement etc.)
 * \tparam Sorter Binary functor type that defines the sorting order.
 *                If Sorter == void, the set is unordered.
 */
template<typename T, typename Sorter = void,
         typename Enabler = std::enable_if<std::is_base_of<Statement, T>::value>>
class StmtSet
{
    using Set = typename std::conditional<std::is_void<Sorter>::value, std::unordered_set<std::shared_ptr<T>>,
                                          std::set<std::shared_ptr<T>, Sorter>>::type;

public:
    typedef typename Set::iterator iterator;
    typedef typename Set::const_iterator const_iterator;

public:
    iterator begin() { return m_set.begin(); }
    iterator end() { return m_set.end(); }

    const_iterator begin() const { return m_set.begin(); }
    const_iterator end() const { return m_set.end(); }

public:
    bool empty() const { return m_set.empty(); }
    void clear() { m_set.clear(); }
    int size() const { return m_set.size(); }

    void insert(const std::shared_ptr<T> &stmt)
    {
        assert(stmt != nullptr);
        m_set.insert(stmt);
    }

    /// Remove this Statement.
    /// \returns true if removed, false if not found
    bool remove(const std::shared_ptr<T> &s)
    {
        if (this->contains(s)) {
            m_set.erase(s);
            return true;
        }

        return false;
    }

    bool contains(const std::shared_ptr<T> &stmt) const { return m_set.find(stmt) != m_set.end(); }


    /// \returns true if any statement in this set defines \p loc
    bool definesLoc(SharedExp loc) const
    {
        if (!loc) {
            return false;
        }

        return std::any_of(m_set.begin(), m_set.end(),
                           [loc](const SharedStmt &stmt) { return stmt->definesLoc(loc); });
    }

    /// \returns true if this set is a subset of \p other
    bool isSubSetOf(const StmtSet &other)
    {
        if (m_set.size() > other.m_set.size()) {
            return false;
        }

        for (const std::shared_ptr<T> &stmt : *this) {
            if (!other.contains(stmt)) {
                return false;
            }
        }

        return true;
    }

    /// Set union: this = this union \p other
    void makeUnion(const StmtSet &other)
    {
        for (const std::shared_ptr<T> &stmt : other) {
            m_set.insert(stmt);
        }
    }

    /// Set intersection: this = this intersect other
    void makeIsect(const StmtSet &other)
    {
        for (auto it = m_set.begin(); it != m_set.end();) {
            if (!other.contains(*it)) {
                // Not in both sets
                it = m_set.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    /// Set difference: this = this - other
    void makeDiff(const StmtSet &other)
    {
        if (&other == this) {
            m_set.clear(); // A \ A == empty set
            return;
        }

        for (const std::shared_ptr<T> &stmt : other) {
            m_set.erase(stmt);
        }
    }

    /// Find a definition for \p loc on the LHS of each assignment in this set.
    /// If found, return pointer to the Assign with that LHS (else return nullptr)
    template<typename = std::enable_if<std::is_base_of<Assign, T>::value>>
    std::shared_ptr<Assign> lookupLoc(SharedExp loc)
    {
        if (!loc) {
            return nullptr;
        }

        std::shared_ptr<Assign> as(new Assign(loc, Terminal::get(opWild)));
        iterator ff = m_set.find(as);

        return (ff != end()) ? *ff : nullptr;
    }

private:
    Set m_set;
};


struct BOOMERANG_API lessAssign
{
    bool operator()(const std::shared_ptr<Assign> &as1, const std::shared_ptr<Assign> &as2) const;
};


typedef StmtSet<Statement> StatementSet;
typedef StmtSet<Assign, lessAssign> AssignSet;
