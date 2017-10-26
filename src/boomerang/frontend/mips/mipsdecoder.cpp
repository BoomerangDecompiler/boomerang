#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "mipsdecoder.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/Proc.h"
#include "boomerang/db/RTL.h"

#include <cassert>


MIPSDecoder::MIPSDecoder(Prog *_prog)
    : NJMCDecoder(_prog)
{
    m_rtlDict.readSSLFile(Boomerang::get()->getSettings()->getDataDirectory().absoluteFilePath("ssl/mips.ssl"));
}


bool MIPSDecoder::decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult& result)
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
