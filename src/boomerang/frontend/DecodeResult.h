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


#include "boomerang/frontend/MachineInstruction.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/Types.h"

#include <cstddef>
#include <list>
#include <memory>


class RTL;


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

    bool valid() const { return rtl != nullptr; }

public:
    /**
     * The class of the decoded instruction. Will be one of the classes described in
     * "A Transformational Approach to Binary Translation of Delayed Branches".
     * Ignored by machines with no delay slots.
     */
    IClass iclass;

    /**
     * If true, don't add numBytes and decode there; instead, re-decode the current instruction.
     * Needed for instructions like the x86 BSF/BSR, which emit branches (so numBytes needs to
     * be carefully set for the fall through out edge after the branch)
     */
    bool reDecode;
    int numBytes; ///< The number of bytes decoded in the main instruction

    /// The RTL constructed (if any).
    std::unique_ptr<RTL> rtl;
};
