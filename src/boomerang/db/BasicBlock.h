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


#include "boomerang/db/GraphNode.h"
#include "boomerang/db/IRFragment.h"
#include "boomerang/frontend/MachineInstruction.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Address.h"

#include <vector>


class OStream;
class RTL;


/// Kinds of basic block nodes
/// reordering these will break the save files - trent
enum class BBType
{
    Invalid  = -1, ///< invalid instruction
    Fall     = 0,  ///< fall-through node
    Oneway   = 1,  ///< unconditional branch (jmp)
    Twoway   = 2,  ///< conditional branch   (jXX)
    Nway     = 3,  ///< case branch          (jmp [off + 4*eax])
    Call     = 4,  ///< procedure call       (call)
    Ret      = 5,  ///< return               (ret)
    CompJump = 6,  ///< computed jump
    CompCall = 7,  ///< computed call        (call [eax + 0x14])
};


// index of the "then" branch of conditional jumps
#define BTHEN 0

// index of the "else" branch of conditional jumps
#define BELSE 1


/**
 * Basic Blocks hold the sematics (RTLs) of a sequential list of instructions
 * ended by a Control Transfer Instruction (CTI).
 * During decompilation, a special RTL with a zero address is prepended;
 * this RTL contains implicit assigns and phi assigns.
 */
class BOOMERANG_API BasicBlock : public GraphNode<BasicBlock>
{
public:
    class BBComparator
    {
    public:
        /// \returns bb1->getLowAddr() < bb2->getLowAddr();
        bool operator()(const BasicBlock *bb1, const BasicBlock *bb2) const;
    };

public:
    /**
     * Creates an incomplete BB.
     * \param function Enclosing function
     */
    BasicBlock(Address lowAddr, Function *function);

    /**
     * Creates a complete BB.
     * \param bbType   type of BasicBlock
     * \param rtls     rtl statements that will be contained in this BasicBlock
     * \param function Function this BasicBlock belongs to.
     */
    BasicBlock(BBType bbType, const std::vector<MachineInstruction> &bbInsns, Function *function);

    BasicBlock(const BasicBlock &other);
    BasicBlock(BasicBlock &&other) = delete;
    ~BasicBlock();

    BasicBlock &operator=(const BasicBlock &other);
    BasicBlock &operator=(BasicBlock &&other) = delete;

public:
    /// \returns the type of the BasicBlock
    inline BBType getType() const { return m_bbType; }
    inline bool isType(BBType type) const { return m_bbType == type; }
    inline void setType(BBType bbType) { m_bbType = bbType; }

    /// \returns enclosing function, nullptr if the BB does not belong to a function.
    inline const Function *getFunction() const { return m_function; }
    inline Function *getFunction() { return m_function; }

    /**
     * \returns the lowest real address associated with this BB.
     * \note although this is usually the address of the first RTL, it is not
     * always so. For example, if the BB contains just a delayed branch,and the delay
     * instruction for the branch does not affect the branch, so the delay instruction
     * is copied in front of the branch instruction. Its address will be
     * UpdateAddress()'ed to 0, since it is "not really there", so the low address
     * for this BB will be the address of the branch.
     * \sa updateBBAddresses
     */
    Address getLowAddr() const;

    /**
     * Get the highest address associated with this BB.
     * This is always the address associated with the last RTL.
     * \sa updateBBAddresses
     */
    Address getHiAddr() const;

    /// \returns true if the instructions of this BB have not been decoded yet.
    inline bool isIncomplete() const { return m_highAddr == Address::INVALID; }

public:
    IRFragment *getIR() { return &m_ir; }
    const IRFragment *getIR() const { return &m_ir; }

    std::vector<MachineInstruction> &getInsns() { return m_insns; }
    const std::vector<MachineInstruction> &getInsns() const { return m_insns; }

    /**
     * Update the RTL list of this basic block. Takes ownership of the pointer.
     * \param rtls a list of RTLs
     */
    void completeBB(const std::vector<MachineInstruction> &bbInsns);

    /// Update the high and low address of this BB if the RTL list has changed.
    void updateBBAddresses();

public:
    /**
     * Print the whole BB to the given stream
     * \param os   stream to output to
     * \param html print in html mode
     */
    void print(OStream &os) const;

    QString toString() const;

protected:
    std::vector<MachineInstruction> m_insns;

    /// The function this BB is part of, or nullptr if this BB is not part of a function.
    Function *m_function = nullptr;

    IRFragment m_ir; ///< For now, this is a single fragment, there may be more in the future

    Address m_lowAddr  = Address::ZERO;
    Address m_highAddr = Address::INVALID;

    BBType m_bbType = BBType::Invalid; ///< type of basic block
};
