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
 * \file       pentiumdecoder.h
 * \brief   The implementation of the instruction decoder for Pentium.
 ******************************************************************************/

/*
 * $Revision$
 * 06 Jun 02 - Trent: Created.
 *
 */

#ifndef PENTDECODER
#define PENTDECODER

class Prog;
class NJMCDecoder;
struct DecodeResult;

class PentiumDecoder : public NJMCDecoder
{
public:
                    PentiumDecoder(Prog* prog);
virtual DecodeResult& decodeInstruction (ADDRESS pc, int delta);
virtual int         decodeAssemblyInstruction (ADDRESS pc, int delta);

private:
    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    Exp *           dis_Eaddr(ADDRESS pc, int size = 0);
    Exp *           dis_Mem(ADDRESS ps);
    Exp *           addReloc(Exp *e);

    void            unused(int x);
    bool            isFuncPrologue(ADDRESS hostPC);

    Byte            getByte(intptr_t lc); //TODO: switch to using ADDRESS objects
    SWord           getWord(intptr_t lc);
    DWord           getDword(intptr_t lc);
    Byte            getByte(ADDRESS lc) {return getByte(lc.m_value);}
    SWord           getWord(ADDRESS lc) {return getWord(lc.m_value);}
    DWord           getDword(ADDRESS lc){return getDword(lc.m_value);}

    ADDRESS         lastDwordLc;
};

#endif
