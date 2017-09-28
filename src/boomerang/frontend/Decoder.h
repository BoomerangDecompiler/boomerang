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


/***************************************************************************/ /**
 * \file       decoder.h
 * OVERVIEW:   The interface to the instruction decoder.
 ******************************************************************************/

#include "boomerang/util/Address.h"

#include <list>
#include <cstddef>
#include <QtCore/QString>

class Exp;
class RTL;
class Prog;

// These are the instruction classes defined in "A Transformational Approach to
// Binary Translation of Delayed Branches" for SPARC instructions.
// Extended for HPPA. Ignored by machines with no delay slots
enum ICLASS : Byte
{
    NCT,   // Non Control Transfer
    SD,    // Static Delayed
    DD,    // Dynamic Delayed
    SCD,   // Static Conditional Delayed
    SCDAN, // Static Conditional Delayed, Anulled if Not taken
    SCDAT, // Static Conditional Delayed, Anulled if Taken
    SU,    // Static Unconditional (not delayed)
    SKIP,  // Skip successor
    //    TRAP,            // Trap
    NOP,   // No operation (e.g. sparc BN,A)
    // HPPA only
    DU,    // Dynamic Unconditional (not delayed)
    NCTA   // Non Control Transfer, with following instr Anulled
};


/***************************************************************************/ /**
 * The DecodeResult struct contains all the information that results from
 * calling the decoder. This prevents excessive use of confusing
 * reference parameters.
 ******************************************************************************/
struct DecodeResult
{
public:
    /// Resets all the fields to their default values.
    void reset()
    {
        numBytes     = 0;
        type         = NCT;
        valid        = true;
        rtl          = nullptr;
        reDecode     = false;
        forceOutEdge = Address::ZERO;
    }

public:
    bool    valid; ///< Indicates whether or not a valid instruction was decoded.

    /**
     * The class of the instruction decoded. Will be one of the classes described in "A Transformational Approach
     * to Binary Translation of Delayed Branches" (plus two more HPPA specific entries).
     * Ignored by machines with no delay slots
     */
    ICLASS  type;

    /**
     * If true, don't add numBytes and decode there; instead, re-decode the current instruction. Needed for
     * instructions like the Pentium BSF/BSR, which emit branches (so numBytes needs to be carefully set for the
     * fall through out edge after the branch)
     */
    bool    reDecode;
    int     numBytes; ///< The number of bytes decoded in the main instruction

    /// The RTL constructed (if any).
    RTL     *rtl;

    /**
     * If non zero, this field represents a new native address to be used as the out-edge for this instruction's BB.
     * At present, only used for the SPARC call/add caller prologue
     */
    Address forceOutEdge;
};


/**
 * \brief The IDecoder class - responsible for translating raw bytes to Instruction lists
 */
class IDecoder
{
public:
    virtual ~IDecoder() = default;

    /// Decodes the machine instruction at \p pc.
    /// The decode result is stored into \p result, if the decode was successful.
    /// If the decode was not successful, the content of \p result is undefined.
    /// \returns true if decoding the instruction was successful.
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult& result) = 0;

    /// Returns machine-specific register name given it's index
    virtual QString getRegName(int idx) const = 0;

    /// Returns index of the named register
    virtual int getRegIdx(const QString& name) const = 0;

    /// Returns size of register in bits
    virtual int getRegSize(int idx) const = 0;

    /// Returns the size of the register with name \p name, in bits
    int getRegSize(const QString& name) const { return getRegSize(getRegIdx(name)); }
};
