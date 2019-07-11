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


#include "boomerang/util/Address.h"

#include <list>
#include <memory>


class Statement;
class OStream;


/**
 * Describes low level register transfer lists (actually lists of statements).
 *
 * \note when time permits, this class could be removed,
 * replaced with new Statements that mark the current native address
 */
class BOOMERANG_API RTL
{
    typedef std::list<Statement *> StmtList;

public:
    typedef StmtList::size_type size_type;
    typedef StmtList::value_type value_type;
    typedef StmtList::reference reference;
    typedef StmtList::const_reference const_reference;

    typedef StmtList::iterator iterator;
    typedef StmtList::const_iterator const_iterator;
    typedef StmtList::reverse_iterator reverse_iterator;
    typedef StmtList::const_reverse_iterator const_reverse_iterator;

public:
    /// \param   instrAddr the address of the instruction
    /// \param   listStmt  ptr to existing list of Statement
    explicit RTL(Address instrAddr, const std::list<Statement *> *listStmt = nullptr);

    /// Take ownership of the statements in the initializer list.
    explicit RTL(Address instrAddr, const std::initializer_list<Statement *> &statements);

    explicit RTL(const RTL &other); ///< Deep copies the content
    explicit RTL(RTL &&other) = default;

    ~RTL();

    /// Makes this RTL a deep copy of \p other.
    RTL &operator=(const RTL &other);
    RTL &operator=(RTL &&other) = default;

public:
    /// Return RTL's native address
    Address getAddress() const { return m_nativeAddr; }

    /// Set the address of this RTL
    void setAddress(Address a) { m_nativeAddr = a; }

    /**
     * Append \p s to the end of this RTL. Takes ownership of the pointer.
     * \note Leaves any flag call at the end
     * (so may push exp to second last position, instead of last)
     */
    void append(Statement *s);

    /// Append a deep copy of \p le to this RTL.
    void append(const std::list<Statement *> &le);

    /// Deep copy the elements of this RTL into the given list.
    void deepCopyList(std::list<Statement *> &dest) const;

    /**
     * Prints this object to a stream in text form.
     * \param   os   stream to output to (often cout or cerr)
     * \param   html if true output is in html
     */
    void print(OStream &os) const;

    // Print to a static buffer (mainly for debugging)
    QString toString() const;

    /// Is this RTL a call instruction?
    bool isCall() const;

    /// Use this slow function when you can't be sure that the HL Statement is last
    Statement *getHlStmt() const;

    /// Simplify all elements in this list and subsequently remove
    /// unnecessary statements (like branches with constant conditions)
    void simplify();

    const std::list<Statement *> &getStatements() const { return m_stmts; }

    // delegates to std::list
public:
    bool empty() const { return m_stmts.empty(); }

    size_type size() const { return m_stmts.size(); }

    reference front() { return m_stmts.front(); }
    reference back() { return m_stmts.back(); }
    const_reference front() const { return m_stmts.front(); }
    const_reference back() const { return m_stmts.back(); }

    iterator begin() { return m_stmts.begin(); }
    iterator end() { return m_stmts.end(); }
    const_iterator begin() const { return m_stmts.begin(); }
    const_iterator end() const { return m_stmts.end(); }

    reverse_iterator rbegin() { return m_stmts.rbegin(); }
    reverse_iterator rend() { return m_stmts.rend(); }
    const_reverse_iterator rbegin() const { return m_stmts.rbegin(); }
    const_reverse_iterator rend() const { return m_stmts.rend(); }

    void pop_front() { m_stmts.pop_front(); }
    void pop_back() { m_stmts.pop_back(); }

    void push_front(const value_type &val) { m_stmts.push_front(val); }

    void insert(iterator where, const value_type &val);
    void clear() { m_stmts.clear(); }

    iterator erase(iterator it) { return m_stmts.erase(it); }

private:
    std::list<Statement *> m_stmts;
    Address m_nativeAddr; ///< RTL's source program instruction address
};

using SharedRTL = std::shared_ptr<RTL>;
using RTLList   = std::list<std::unique_ptr<RTL>>;
