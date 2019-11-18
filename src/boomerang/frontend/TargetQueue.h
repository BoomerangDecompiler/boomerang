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


#include "boomerang/util/Address.h"

#include <queue>


class ProcCFG;
class BasicBlock;


/// Put the target queue logic into this small class
class BOOMERANG_API TargetQueue
{
public:
    TargetQueue(bool traceDecoder);

public:
    /**
     * Seed the queue with an initial address.
     * \note     Can be some targets already in the queue now
     * \param    addr Native address to seed the queue with
     */
    void initial(Address addr);

    /**
     * Visit a destination as a label, i.e. check whether we need to queue it as a new BB to create
     * later.
     *
     * \note at present, it is important to visit an address BEFORE an out edge is added to that
     * address. This is because adding an out edge enters the address into the ProcCFG's BB map, and
     * it looks like the BB has already been visited, and it gets overlooked. It would be better to
     * have a scheme whereby the order of calling these functions (i.e. visit() and AddOutEdge())
     * did not matter.
     *
     * \param   cfg     the enclosing CFG
     * \param   newAddr the address to be checked
     * \param   newBB   set to the lower part of the BB if the address already exists
     *          as a non explicit label (BB has to be split)
     */
    void pushAddress(ProcCFG *cfg, Address newAddr, BasicBlock *&newBB);

    /**
     * Return the next target from the queue of non-processed targets.
     * \param   cfg the enclosing CFG
     * \returns The next address to process, or Address::INVALID if none
     *          (targets is empty)
     */
    Address popAddress(const ProcCFG &cfg);

private:
    bool m_traceDecoder;
    std::queue<Address> m_targets;
};
