#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MIPSFrontEnd.h"


#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/frontend/mips/MIPSDecoder.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/log/Log.h"


#include <cassert>
#include <sstream>


MIPSFrontEnd::MIPSFrontEnd(BinaryFile *binaryFile, Prog *prog)
    : DefaultFrontEnd(binaryFile, prog)
{
    m_decoder.reset(new MIPSDecoder(prog));
}


Address MIPSFrontEnd::findMainEntryPoint(bool& gotMain)
{
    Address start = m_binaryFile->getMainEntryPoint();

    if (start != Address::INVALID) {
        gotMain = true;
        return start;
    }

    start = m_binaryFile->getEntryPoint();
    if (start != Address::INVALID) {
        gotMain = true;
        return start;
    }

    gotMain = false;
    return Address::INVALID;
}


bool MIPSFrontEnd::processProc(UserProc *proc, Address entryAddr)
{
    // Call the base class to do most of the work
    if (!DefaultFrontEnd::processProc(proc, entryAddr)) {
        return false;
    }

    // This will get done twice; no harm
    proc->setEntryBB();

    return true;
}
