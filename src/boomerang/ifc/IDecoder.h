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


#include "boomerang/frontend/LiftedInstruction.h"
#include "boomerang/frontend/MachineInstruction.h"
#include "boomerang/ssl/Register.h"


class Exp;
class RTL;
class Prog;
class Project;
class RTLInstDict;


/**
 * Base class for machine instruction decoders.
 * Decoders disassemble raw bytes to MachineInstructions
 * and lift them to statement lists (RTLs).
 */
class BOOMERANG_API IDecoder
{
public:
    IDecoder(Project *) {}
    virtual ~IDecoder() = default;

public:
    virtual bool initialize(Project *project) = 0;

    /**
     * Disassembles the machine instruction \p pc.
     * The result is stored into \p result, if successful.
     * If the disassembly was not successful, the content of \p result is undefined.
     *
     * \param pc Address of the instruction
     * \param delta Host - native address difference
     *
     * \returns true iff disassembling the instruction was successful.
     */
    [[nodiscard]] virtual bool disassembleInstruction(Address pc, ptrdiff_t delta,
                                                      MachineInstruction &result) = 0;

    /// Lift a disassembled instruction to an RTL
    /// \returns true if lifting the instruction was succesful.
    [[nodiscard]] virtual bool liftInstruction(const MachineInstruction &insn,
                                               LiftedInstruction &lifted) = 0;

    /// \returns machine-specific register name given its index
    virtual QString getRegNameByNum(RegNum regNum) const = 0;

    /// \returns size of register in bits
    virtual int getRegSizeByNum(RegNum regNum) const = 0;

    virtual const RTLInstDict *getDict() const = 0;

    /// \return true if this is a SPARC restore instruction.
    /// For all other architectures, this must return false.
    virtual bool isSPARCRestore(const MachineInstruction &insn) const = 0;
};
