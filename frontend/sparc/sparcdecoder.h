/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************//**
 * \file       sparcdecoder.h
 * \brief   The implementation of the instruction decoder for Sparc.
 ******************************************************************************/

#ifndef SPARCDECODER
#define SPARCDECODER
#include <cstddef>
class Prog;
class NJMCDecoder;
struct DecodeResult;

class SparcDecoder : public NJMCDecoder
{
public:
    /* Constructor
         */
    SparcDecoder(Prog* prog);


    /*
         * Decodes the machine instruction at pc and returns an RTL instance for
         * the instruction.
         */
    virtual DecodeResult& decodeInstruction (ADDRESS pc, ptrdiff_t delta);

    /*
         * Disassembles the machine instruction at pc and returns the number of
         * bytes disassembled. Assembler output goes to global _assembly
         */
    virtual int decodeAssemblyInstruction (ADDRESS pc, ptrdiff_t delta);

    /*
         * Indicates whether the instruction at the given address is a restore instruction.
         */
    bool    isRestore(ADDRESS hostPC);
private:
    /*
         * Various functions to decode the operands of an instruction into
         * a SemStr representation.
         */
    Exp*    dis_Eaddr(ADDRESS pc, int size = 0);
    Exp*    dis_RegImm(ADDRESS pc);
    Exp*    dis_RegLhs(unsigned r);
    Exp*    dis_RegRhs(unsigned r);

    void    unused(int x);
    RTL*    createBranchRtl(ADDRESS pc, std::list<Statement*>* stmts, const char* name);
    bool    isFuncPrologue(ADDRESS hostPC);
    DWord    getDword(ADDRESS lc);

};

#endif
