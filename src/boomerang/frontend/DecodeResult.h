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
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Types.h"

#include <cstddef>
#include <list>
#include <memory>


/**
 * The DecodeResult struct contains all the information that results from
 * lifting a MachineInstruction.
 *
 * \sa IDecoder::liftInstruction
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

    void fillRTL(std::unique_ptr<RTL> _rtl);
    std::unique_ptr<RTL> useRTL();

    RTL *getFirstRTL();
    const RTL *getFirstRTL() const;

private:
    RTLList m_rtls;

public:
    /**
     * If true, the semantics of this instruction are incomplete and it must be re-lifted
     * to retrieve all semantics. This is necessary for instructions like x86 BSF/BSR,
     * which emit branches (these instructions need to have additional RTLs at %pc+1, %pc+2 etc.
     * to account for the additional semantics)
     *
     * \warning Re-lifting must always be done until this variable is false, even if the semantics
     * are not used. Not doing so will break lifting other instructions.
     */
    bool reLift;
};
