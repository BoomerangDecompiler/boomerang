#pragma once

/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       st20decoder.h
 * \brief   The definition of the instruction decoder for ST20.
 ******************************************************************************/

#include "boomerang/frontend/NJMCDecoder.h"

#include "boomerang/db/RTL.h"

#include <cstddef>
#include <list>

class Prog;
class NJMCDecoder;
class Statement;
struct DecodeResult;


class ST20Decoder : public NJMCDecoder
{
public:
    /// \copydoc NJMCDecoder::NJMCDecoder
    ST20Decoder(Prog *prog);

    /// \copydoc NJMCDecoder::decodeInstruction
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult& result) override;

private:

    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    // Exp*    dis_Eaddr(ADDRESS pc, int size = 0);
    // Exp*    dis_RegImm(ADDRESS pc);
    // Exp*    dis_Reg(unsigned r);
    // Exp*    dis_RAmbz(unsigned r);        // Special for rA of certain instructions

    RTL *createBranchRtl(Address pc, std::list<Statement *> *stmts, const char *name);
    bool isFuncPrologue(Address hostPC);

    DWord getDword(intptr_t lc); // TODO: switch back to using ADDRESS objects
    SWord getWord(intptr_t lc);
    Byte getByte(intptr_t lc);
};
