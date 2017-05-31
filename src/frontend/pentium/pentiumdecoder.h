#pragma once
/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       pentiumdecoder.h
  * \brief   The implementation of the instruction decoder for Pentium.
  ******************************************************************************/
  
#include <cstddef>

#include "njmcDecoder.h"
class Prog;
struct DecodeResult;

class PentiumDecoder : public NJMCDecoder {
  public:
    PentiumDecoder(Prog *prog);
    virtual ~PentiumDecoder() = default;
    DecodeResult &decodeInstruction(ADDRESS pc, ptrdiff_t delta) override;
    int decodeAssemblyInstruction(ADDRESS pc, ptrdiff_t delta) override;

  private:
    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    SharedExp dis_Eaddr(ADDRESS pc, int size = 0);
    SharedExp dis_Mem(ADDRESS ps);
    SharedExp addReloc(const SharedExp &e);

    bool isFuncPrologue(ADDRESS hostPC);

    Byte getByte(intptr_t lc); // TODO: switch to using ADDRESS objects
    SWord getWord(intptr_t lc);
    DWord getDword(intptr_t lc);
    Byte getByte(ADDRESS lc)   { return getByte(lc.m_value); }
    SWord getWord(ADDRESS lc)  { return getWord(lc.m_value); }
    DWord getDword(ADDRESS lc) { return getDword(lc.m_value); }

    ADDRESS lastDwordLc;
};

