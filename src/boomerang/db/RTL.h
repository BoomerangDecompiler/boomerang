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
class QTextStream;


/**
 * Describes low level register transfer lists (actually lists of statements).
 *
 * \note when time permits, this class could be removed,
 * replaced with new Statements that mark the current native address
 */
class RTL : public std::list<Statement *>
{
    friend class BasicBlock;

public:
    RTL();

    /// \param   instrAddr the address of the instruction
    /// \param   listStmt  ptr to existing list of Statement
    RTL(Address instrAddr, const std::list<Statement *> *listStmt = nullptr);

    /// Makes this RTL a deep copy of \p other.
    RTL(const RTL& other);

    ~RTL();

    /// Creates a deep copy clone of this RTL.
    RTL *clone() const;

    /// Makes this RTL a deep copy of \p other.
    const RTL& operator=(const RTL& other);

    // Common enquiry methods

    /// Return RTL's native address
    Address getAddress() const { return m_nativeAddr; }

    /// Set the address of this RTL
    void setAddress(Address a) { m_nativeAddr = a; }

    // Statement list editing methods

    /**
     *  Append \p s to the end of this RTL.
     * \note   Exception: Leaves any flag call at the end
     * (so may push exp to second last position, instead of last)
     * \note   \p s is NOT copied.
     */
    void append(Statement *s);

    /// Append a deep copy of \p le to this RTL.
    void append(std::list<Statement *>& le);

    /// Deep copy the elements of this RTL into the given list.
    void deepCopyList(std::list<Statement *>& dest) const;

    /**
     * Prints this object to a stream in text form.
     * \param   os   stream to output to (often cout or cerr)
     * \param   html if true output is in html
     */
    void print(QTextStream& os, bool html = false) const;

    // Print to a static buffer (mainly for debugging)
    char *prints() const;

    /// Dump the content of this RTL to stderr.
    void dump() const;

    /// Is this RTL a call instruction?
    bool isCall() const;

    /// Use this slow function when you can't be sure that the HL Statement is last
    /// Get the "special" (High Level) Statement this RTL (else nullptr)
    Statement *getHlStmt() const;

protected:
    /// Simplify all elements in this list and subsequently remove
    /// unnecessary statements (like branches with constant conditions)
    void simplify();

private:
    Address m_nativeAddr; ///< RTL's source program instruction address
};

using SharedRTL = std::shared_ptr<RTL>;
using RTLList   = std::list<RTL *>;
