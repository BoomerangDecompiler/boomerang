/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       rtl.h
 * Definition of the classes that describe an RTL, a low-level
 *       register transfer list. Higher-level RTLs (instance
 *       of class HLJump, HLCall, etc.) represent information about
 *       a control transfer instruction (CTI) in the source program.
 *       analysis code adds information to existing higher-level
 *       RTLs and sometimes creates new higher-level RTLs (e.g., for
 *       switch statements).
 ******************************************************************************/

#pragma once

#include "boomerang/db/Register.h"   // for Register
#include "boomerang/type/Type.h"     // for Type
#include "boomerang/util/Types.h" // for ADDRESS

#include <functional>                // for less
#include <list>                      // for list
#include <map>                       // for map
#include <set>                       // for set
#include <string>                    // for string
#include <utility>                   // for pair
#include <vector>                    // for vector
#include <QMap>
#include <memory>


class Exp;         // lines 38-38
class Statement; // lines 47-47
class QTextStream;
class QString;

enum StmtType : uint8_t;
using SharedRTL = std::shared_ptr<class RTL>;
using SharedExp = std::shared_ptr<Exp>;


/***************************************************************************/ /**
 * Class RTL: describes low level register transfer lists (actually lists of statements).
 *
 * \note when time permits, this class could be removed,
 * replaced with new Statements that mark the current native address
 ******************************************************************************/
class RTL : public std::list<Statement *>
{
    friend class BasicBlock;

public:
    RTL();

    /***************************************************************************/ /**
    * \brief   Constructor.
    * \param   instNativeAddr - the native address of the instruction
    * \param   listStmt - ptr to existing list of Statement
    ******************************************************************************/
    RTL(Address instNativeAddr, const std::list<Statement *> *listStmt = nullptr);

    /***************************************************************************/ /**
     * \brief        Copy constructor. A deep clone is made of the given object
     *               so that the lists of Exps do not share memory.
     * \param        other RTL to copy from
     ******************************************************************************/
    RTL(const RTL& other);
    ~RTL();

    /***************************************************************************/ /**
     * \brief        Deep copy clone; deleting the clone will not affect this
     *               RTL object
     * \returns      Pointer to a new RTL that is a clone of this one
     ******************************************************************************/
    RTL *clone() const;

    /***************************************************************************/ /**
     * \brief        Assignment copy (deep).
     * \param        other - RTL to copy
     * \returns      a reference to this object
     ******************************************************************************/
    RTL& operator=(const RTL& other);

    /// Common enquiry methods
    Address getAddress() const { return m_nativeAddr; }    ///< Return RTL's native address
    void setAddress(Address a) { m_nativeAddr = a; } ///< Set the address

    // Statement list editing methods

    /***************************************************************************/ /**
     * \brief        Append the given Statement at the end of this RTL
     * \note         Exception: Leaves any flag call at the end (so may push exp
     *               to second last position, instead of last)
     * \note         stmt is NOT copied. This is different to how UQBT was!
     * \param        s pointer to Statement to append
     ******************************************************************************/
    void appendStmt(Statement *s); // Add s to end of RTL.

    /***************************************************************************/ /**
     * \brief  Append a given list of Statements to this RTL
     * \note   A copy of the Statements in le are appended
     * \param  le - list of Statements to insert
     ******************************************************************************/
    void appendListStmt(std::list<Statement *>& le);

    /***************************************************************************/ /**
     * \brief        Make a copy of this RTLs list of Exp* to the given list
     * \param        dest Ref to empty list to copy to
     ******************************************************************************/
    void deepCopyList(std::list<Statement *>& dest) const;

    /***************************************************************************/ /**
     * \brief   Prints this object to a stream in text form.
     * \param   os - stream to output to (often cout or cerr)
     * \param   html - if true output is in html
     ******************************************************************************/
    void print(QTextStream& os, bool html = false) const;
    void dump() const;

    /// Is this RTL a call instruction?
    bool isCall() const;


    /// Use this slow function when you can't be sure that the HL Statement is last
    /// Get the "special" (High Level) Statement this RTL (else nullptr)
    Statement *getHlStmt() const;
    char *prints() const; // Print to a string (mainly for debugging)

protected:
    void simplify();

private:
    Address m_nativeAddr; ///< RTL's source program instruction address
};
