/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       st20decoder.h
 * OVERVIEW:   The definition of the instruction decoder for ST20.
 ******************************************************************************/

/*
 * $Revision$
 * 10/Mar/05 MVE and Dr Aus: Created.
 */

#ifndef ST20DECODER
#define ST20DECODER

class Prog;
class NJMCDecoder;
struct DecodeResult;

class ST20Decoder : public NJMCDecoder
{
public:
    /* Default constructor
         */
    ST20Decoder();

    /**
         * Decodes the machine instruction at pc and returns an RTL instance for
         * the instruction.
         */
    virtual DecodeResult& decodeInstruction (ADDRESS pc, int delta);

    /*
         * Disassembles the machine instruction at pc and returns the number of
         * bytes disassembled. Assembler output goes to global _assembly
         */
    virtual int decodeAssemblyInstruction (ADDRESS pc, int delta);


private:
    /*
         * Various functions to decode the operands of an instruction into
         * a SemStr representation.
         */
    //Exp*    dis_Eaddr(ADDRESS pc, int size = 0);
    //Exp*    dis_RegImm(ADDRESS pc);
    //Exp*    dis_Reg(unsigned r);
    //Exp*    dis_RAmbz(unsigned r);        // Special for rA of certain instructions

    void    unused(int x);
    RTL*    createBranchRtl(ADDRESS pc, std::list<Statement*>* stmts,
                            const char* name);
    bool    isFuncPrologue(ADDRESS hostPC);
	DWord	getDword(intptr_t lc); // TODO: switch back to using ADDRESS objects
	SWord	getWord(intptr_t lc);
	Byte	getByte(intptr_t lc);

};

#endif
