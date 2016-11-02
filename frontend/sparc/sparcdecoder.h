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
  * \file       sparcdecoder.h
  * \brief   The implementation of the instruction decoder for Sparc.
  ******************************************************************************/

#ifndef SPARCDECODER
#define SPARCDECODER
#include "njmcDecoder.h"

#include <cstddef>

class Prog;
struct DecodeResult;
class SparcMachine {
public:
    SharedExp dis_RegRhs(uint8_t reg_no);
};

class SparcDecoder : public NJMCDecoder {
    SparcMachine *machine;
public:
    SparcDecoder(Prog *prog);
    DecodeResult &decodeInstruction(ADDRESS pc, ptrdiff_t delta) override;
    int decodeAssemblyInstruction(ADDRESS pc, ptrdiff_t delta)  override;

    /*
     * Indicates whether the instruction at the given address is a restore instruction.
     */
    bool isRestore(ADDRESS hostPC);

  private:
    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    SharedExp dis_Eaddr(ADDRESS pc, int size = 0);
    SharedExp dis_RegImm(ADDRESS pc);
    SharedExp dis_RegLhs(unsigned r);

    RTL *createBranchRtl(ADDRESS pc, std::list<Instruction *> *stmts, const char *name);
    bool isFuncPrologue(ADDRESS hostPC);
    DWord getDword(ADDRESS lc);
};

#endif
