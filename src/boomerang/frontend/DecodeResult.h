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
#include "boomerang/util/Types.h"

#include <cstddef>
#include <list>
#include <memory>


class RTL;


/**
 * These are the instruction classes defined in
 * "A Transformational Approach to Binary Translation of Delayed Branches"
 * for SPARC instructions.
 * Ignored by machines with no delay slots.
 */
enum ICLASS : uint8
{
    NCT,   ///< Non Control Transfer
    SD,    ///< Static Delayed
    DD,    ///< Dynamic Delayed
    SCD,   ///< Static Conditional Delayed
    SCDAN, ///< Static Conditional Delayed, Anulled if Not taken
    SCDAT, ///< Static Conditional Delayed, Anulled if Taken
    SU,    ///< Static Unconditional (not delayed)
    SKIP,  ///< Skip successor
    NOP    ///< No operation (e.g. SPARC BN,A)
};


/**
 * The DecodeResult struct contains all the information that results from
 * calling the decoder. This prevents excessive use of confusing
 * reference parameters.
 */
class BOOMERANG_API DecodeResult
{
public:
    DecodeResult();
    DecodeResult(const DecodeResult &) = delete;
    DecodeResult(DecodeResult &&);

    ~DecodeResult();

    // clang-format off
    DecodeResult &operator=(const DecodeResult &) = delete;
    DecodeResult &operator=(DecodeResult &&);
    // clang-fomat on

    /// Resets all the fields to their default values.
    void reset();

public:
    bool valid; ///< Indicates whether or not a valid instruction was decoded.

    /**
     * The class of the instruction decoded. Will be one of the classes described in
     * "A Transformational Approach to Binary Translation of Delayed Branches".
     * Ignored by machines with no delay slots.
     */
    ICLASS type;

    /**
     * If true, don't add numBytes and decode there; instead, re-decode the current instruction.
     * Needed for instructions like the Pentium BSF/BSR, which emit branches (so numBytes needs to
     * be carefully set for the fall through out edge after the branch)
     */
    bool reDecode;
    int numBytes; ///< The number of bytes decoded in the main instruction

    /// The RTL constructed (if any).
    std::unique_ptr<RTL> rtl;

    /**
     * If non zero, this field represents a new native address to be used as the out-edge for this
     * instruction's BB. At present, only used for the SPARC call/add caller prologue
     */
    Address forceOutEdge;
};
