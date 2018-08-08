#pragma region License
/*
 * Copyright (C) 1996-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/frontend/NJMCDecoder.h"
#include "boomerang/ssl/exp/Operator.h"


class Prog;
class DecodeResult;


/**
 * Decoder for x86 instructions.
 * \note x86-64 instructions are not supported.
 */
class PentiumDecoder : public NJMCDecoder
{
public:
    /// \copydoc NJMCDecoder::NJMCDecoder
    PentiumDecoder(Prog *prog);
    PentiumDecoder(const PentiumDecoder& other) = delete;
    PentiumDecoder(PentiumDecoder&& other) = default;

    /// \copydoc NJMCDecoder::~NJMCDecoder
    virtual ~PentiumDecoder() override = default;

    PentiumDecoder& operator=(const PentiumDecoder& other) = delete;
    PentiumDecoder& operator=(PentiumDecoder&& other) = default;

public:
    /// \copydoc NJMCDecoder::decodeInstruction
    /**
     * Decodes a machine instruction and returns an RTL instance. In most cases
     * a single instruction is decoded. However, if a higher level construct
     * that may consist of multiple instructions is matched, then there may be
     * a need to return more than one RTL. The caller_prologue2 is an example of such
     * a construct which encloses an abritary instruction that must be decoded
     * into its own RTL.
     *
     * \param   pc - the native address of the pc
     * \param   delta - the difference between the above address and the
     *      host address of the pc (i.e. the address that the pc is at
     *      in the loaded object file)
     * \returns a DecodeResult structure containing all the information gathered during decoding
     */
    virtual bool decodeInstruction(Address pc, ptrdiff_t delta, DecodeResult& result) override;

private:

    /*
     * Various functions to decode the operands of an instruction into
     * a SemStr representation.
     */
    SharedExp dis_Eaddr(HostAddress hostPC, int size = 0);
    SharedExp dis_Mem(HostAddress ps);
    SharedExp addReloc(const SharedExp& e);

    bool isFuncPrologue(Address hostPC);

    /// Read bytes, words or dwords from the memory at address \p addr
    Byte getByte(HostAddress addr)   { return *reinterpret_cast<const Byte *>(addr.value()); }
    SWord getWord(HostAddress addr)  { return *reinterpret_cast<const SWord *>(addr.value()); }
    DWord getDword(HostAddress addr) { return *reinterpret_cast<const DWord *>(addr.value()); }

    /**
     * Generate statements for the BSF and BSR series (Bit Scan Forward/Reverse)
     * \param pc native PC address (start of the BSF/BSR instruction)
     * \param dest an expression for the destination register
     * \param modrm an expression for the operand being scanned
     * \param init initial value for the dest register
     * \param size sizeof(modrm) (in bits)
     * \param incdec either opPlus for Forward scans, or opMinus for Reverse scans
     * \param numBytes number of bytes this instruction
     */
    void genBSFR(Address pc, SharedExp reg, SharedExp modrm, int init, int size, OPER incdec, int numBytes, DecodeResult& result, bool debug);

private:
    int BSFRstate = 0; ///< state machine state number for decoding BSF/BSR instruction
};
