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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/frontend/DefaultFrontEnd.h"
#include "boomerang/ifc/IDecoder.h"
#include "boomerang/ssl/exp/Operator.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/Interval.h"


class ProcCFG;


/**
 * This file contains routines to manage the decoding of SPARC instructions
 * and the instantiation to RTLs, removing SPARC dependent features
 * such as delay slots in the process.
 */
class BOOMERANG_PLUGIN_API SPARCFrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    SPARCFrontEnd(Project *project);
    SPARCFrontEnd(const SPARCFrontEnd &other) = delete;
    SPARCFrontEnd(SPARCFrontEnd &&other)      = default;

    virtual ~SPARCFrontEnd() = default;

    SPARCFrontEnd &operator=(const SPARCFrontEnd &) = delete;
    SPARCFrontEnd &operator=(SPARCFrontEnd &&) = default;

public:
    /**
     * \copydoc IFrontEnd::processProc
     *
     * Builds the CFG for a procedure out of the RTLs constructed
     * during decoding. The semantics of delayed CTIs are
     * transformed into CTIs that aren't delayed.
     */
    virtual bool disassembleFragment(UserProc *proc, Address entryAddr) override;

    /// \copydoc IFrontEnd::liftProc
    bool liftProc(UserProc *proc) override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    Address findMainEntryPoint(bool &gotMain) override;

private:
    bool handleCTI(std::list<MachineInstruction> &bbInsns, UserProc *proc);

    bool liftBB(BasicBlock *bb, BasicBlock *delay, UserProc *proc);

    /// Lift the non-CTI parts of a Basic Block
    std::unique_ptr<RTLList> liftBBPart(BasicBlock *bb);

    bool liftSD(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);
    bool liftDD(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);
    bool liftSCD(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);
    bool liftSCDAN(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);
    bool liftSCDAT(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);
    bool liftSU(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);
    bool liftSKIP(BasicBlock *bb, const MachineInstruction *delayInsn, UserProc *proc);

private:
    /// Warn about an invalid or unrecognized instruction at \p pc
    void warnInvalidInstruction(Address pc);

private:
    // This struct represents a single nop instruction.
    // Used as a substitute delay slot instruction
    LiftedInstruction nop_inst;

    std::list<std::shared_ptr<CallStatement>> m_callList;
};
