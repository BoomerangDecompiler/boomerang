#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


/***************************************************************************/ /**
 * \file       sparcdecoder.h
 * \brief   The implementation of the instruction decoder for Sparc.
 ******************************************************************************/

#include "boomerang/frontend/NJMCDecoder.h"

class Prog;
struct DecodeResult;

class SparcMachine
{
public:
    /***************************************************************************/ /**
     * \fn     SparcMachine::dis_RegRhs
     * \brief   Decode the register on the RHS
     * \note    Replaces r[0] with const 0
     * \note    Not used by DIS_RD since don't want 0 on LHS
     * \param   reg_no - register (0-31)
     * \returns        the expression representing the register
     ******************************************************************************/
    SharedExp dis_RegRhs(uint8_t reg_no);
};

class SparcDecoder : public NJMCDecoder
{
    SparcMachine *machine;

public:
    /// \copydoc NJMCDecoder::NJMCDecoder
    SparcDecoder(Prog *prog);

    /// \copydoc NJMCDecoder::decodeInstruction
    /***************************************************************************/ /**
     * \fn     SparcDecoder::decodeInstruction
     * \brief  Attempt to decode the high level instruction at a given address.
     *
     * Return the corresponding HL type (e.g. CallStatement, GotoStatement etc). If no high level instruction exists at
     * the given address,then simply return the RTL for the low level instruction at this address. There is an option
     * to also include the low level statements for a HL instruction.
     *
     * \param pc - the native address of the pc
     * \param delta - the difference between the above address and the host address of the pc (i.e. the address
     *        that the pc is at in the loaded object file)
     * \returns a DecodeResult structure containing all the information gathered during decoding
     ******************************************************************************/
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult& result) override;

    /// Indicates whether the instruction at the given address is a restore instruction.
    /***************************************************************************/ /**
     * \brief      Check to see if the instruction at the given offset is a restore instruction
     * \param      hostPC - pointer to the code in question (host address)
     * \returns           True if a match found
     ******************************************************************************/
    bool isRestore(HostAddress hostPC);

private:

    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */

    /***************************************************************************/ /**
     * \fn         SparcDecoder::dis_Eaddr
     * \brief      Converts a dynamic address to a Exp* expression.
     *             E.g. %o7 --> r[ 15 ]
     * \param        pc - the instruction stream address of the dynamic address
     * \param        size - redundant parameter on SPARC
     * \returns    the Exp* representation of the given address
     ******************************************************************************/
    SharedExp dis_Eaddr(HostAddress pc, int size = 0);

    /***************************************************************************/ /**
     * \fn     SparcDecoder::dis_RegImm
     * \brief        Decode the register or immediate at the given address.
     * \note         Used via macro DIS_ROI
     * \param        pc - an address in the instruction stream
     * \returns      the register or immediate at the given address
     ******************************************************************************/
    SharedExp dis_RegImm(HostAddress pc);

    /***************************************************************************/ /**
     * \fn     SparcDecoder::dis_RegLhs
     * \brief   Decode the register on the LHS
     * \param   r - register (0-31)
     * \returns the expression representing the register
     ******************************************************************************/
    SharedExp dis_RegLhs(unsigned r);

    /***************************************************************************/ /**
     * \fn    SparcDecoder::createBranchRtl
     * \brief Create an RTL for a Bx instruction
     * \param pc - the location counter
     * \param stmts - ptr to list of Statement pointers
     * \param name - instruction name (e.g. "BNE,a", or "BPNE")
     * \returns            Pointer to newly created RTL, or nullptr if invalid
     ******************************************************************************/
    RTL *createBranchRtl(Address pc, std::list<Statement *> *stmts, const char *name);

    /***************************************************************************/ /**
     * \fn      isFuncPrologue()
     * \brief      Check to see if the instructions at the given offset match any callee prologue, i.e. does it look
     *                    like this offset is a pointer to a function?
     * \param      hostPC - pointer to the code in question (host address)
     * \returns           True if a match found
     ******************************************************************************/
    bool isFuncPrologue(HostAddress hostPC);

    /***************************************************************************/ /**
     * \fn        SparcDecoder::getDword
     * \brief     Returns the double starting at the given address.
     * \param     lc - address at which to decode the double
     * \returns   the decoded double
     ******************************************************************************/
    DWord getDword(HostAddress lc);
};
