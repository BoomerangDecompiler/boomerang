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
 * \file       ppcdecoder.h
 * \brief   The implementation of the instruction decoder for PPC.
 ******************************************************************************/
#ifndef PPCDECODER
#define PPCDECODER
#include <cstdlib>
class Prog;
class NJMCDecoder;
struct DecodeResult;

class PPCDecoder : public NJMCDecoder
{
public:
    /* Default constructor
         */
    PPCDecoder(Prog* prog);

    /*
         * Decodes the machine instruction at pc and returns an RTL instance for
         * the instruction.
         */
    virtual DecodeResult&   decodeInstruction (ADDRESS pc, ptrdiff_t delta);

    /*
         * Disassembles the machine instruction at pc and returns the number of
         * bytes disassembled. Assembler output goes to global _assembly
         */
    virtual int             decodeAssemblyInstruction (ADDRESS pc, ptrdiff_t delta);


private:
    /*
             * Various functions to decode the operands of an instruction into an Exp* representation.
             */
    Exp*            dis_Eaddr(ADDRESS pc, int size = 0);
    Exp*            dis_RegImm(ADDRESS pc);
    Exp*            dis_Reg(unsigned r);
    Exp*            dis_RAmbz(unsigned r);        // Special for rA of certain instructions

    void            unused(int);
    RTL*            createBranchRtl(ADDRESS pc, std::list<Statement*>* stmts, const char* name);
    bool            isFuncPrologue(ADDRESS);
    DWord           getDword(ADDRESS lc);
};

#endif
