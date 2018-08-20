#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MIPSDecoder.h"

#include "boomerang/core/Project.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/log/Log.h"

#include <cassert>


MIPSDecoder::MIPSDecoder(Prog *prog)
    : NJMCDecoder(prog, "ssl/mips.ssl")
{}


bool MIPSDecoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult &result)
{
    Q_UNUSED(pc);
    Q_UNUSED(delta);

    // ADDRESS hostPC = pc+delta;

    // Clear the result structure;
    result.reset();
    // The actual list of instantiated statements
    // std::list<Statement*>* stmts = nullptr;
    // ADDRESS nextPC = Address::INVALID;
    // Decoding goes here.... TODO
    result.valid = false;
    return result.valid;
}
