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


#include "StatementSet.h"

#include <list>


class LocationSet;
class Statement;
class Assignment;


using SharedExp      = std::shared_ptr<class Exp>;
using SharedConstExp = std::shared_ptr<const class Exp>;


/**
 * A non-owning list of Statements.
 */
class BOOMERANG_API StatementList
{
    typedef std::list<Statement *> List;

    typedef List::size_type size_type;
    typedef List::reference reference;
    typedef List::const_reference const_reference;
    typedef List::pointer pointer;

public:
    typedef List::iterator iterator;
    typedef List::const_iterator const_iterator;
    typedef List::reverse_iterator reverse_iterator;
    typedef List::const_reverse_iterator const_reverse_iterator;

public:
    iterator begin() { return m_list.begin(); }
    iterator end() { return m_list.end(); }

    const_iterator begin() const { return m_list.begin(); }
    const_iterator end() const { return m_list.end(); }

    reverse_iterator rbegin() { return m_list.rbegin(); }
    reverse_iterator rend() { return m_list.rend(); }

    const_reverse_iterator rbegin() const { return m_list.rbegin(); }
    const_reverse_iterator rend() const { return m_list.rend(); }

public:
    bool empty() const { return m_list.empty(); }
    size_t size() const { return m_list.size(); }

    void resize(size_t newSize) { m_list.resize(newSize, nullptr); }

    const_reference front() const { return m_list.front(); }
    const_reference back() const { return m_list.back(); }

    void clear() { m_list.clear(); }

    iterator erase(iterator it) { return m_list.erase(it); }

    iterator insert(iterator where, Statement *stmt) { return m_list.insert(where, stmt); }

    template<typename Comp = std::less<Statement *>>
    void sort(Comp comp)
    {
        return m_list.sort(comp);
    }

    /**
     * Special intersection method: *this := a intersect b
     * A special intersection operator; *this becomes the intersection
     * of StatementList a (assumed to be a list of Assignment *'s)
     * with the LocationSet b.
     * Used for calculating returns for a CallStatement
     */
    void makeIsect(StatementList &a, LocationSet &b);

    void append(Statement *s);
    void append(const StatementList &list);
    void append(const StatementSet &set);

    /// \returns true if successfully removed, false if not found
    bool remove(Statement *stmt);

    /// Remove the first definition where \p loc appears on the left
    /// \returns the removed statement, or nullptr if not found.
    /// \note statements in this list are assumed to be assignments
    Statement *removeFirstDefOf(SharedExp loc);

    /// Return true if loc appears on the left of any statements in this list
    /// Note: statements in this list are assumed to be assignments
    bool existsOnLeft(const SharedExp &loc)
        const; ///< True if loc exists on the LHS of any Assignment in this list

    /// Find the first Assignment with loc on the LHS
    const Assignment *findOnLeft(SharedConstExp loc) const;
    Assignment *findOnLeft(SharedExp loc); ///< Return the first stmt with loc on the LHS

    QString toString() const;

private:
    List m_list;
};
