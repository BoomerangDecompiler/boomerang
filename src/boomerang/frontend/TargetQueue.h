#pragma once

#include "boomerang/util/Address.h"

#include <queue>

class Cfg;
class BasicBlock;

/// Put the target queue logic into this small class
class TargetQueue
{
    std::queue<Address> targets;

public:
    /***************************************************************************/ /**
    * \brief   Visit a destination as a label, i.e. check whether we need to queue it as a new BB to create later.
    *              Note: at present, it is important to visit an address BEFORE an out edge is added to that address.
    *              This is because adding an out edge enters the address into the Cfg's BB map, and it looks like the
    *              BB has already been visited, and it gets overlooked. It would be better to have a scheme whereby
    *              the order of calling these functions (i.e. visit() and AddOutEdge()) did not matter.
    * \param   pCfg - the enclosing CFG
    * \param   uNewAddr - the address to be checked
    * \param   pNewBB - set to the lower part of the BB if the address already exists
    *          as a non explicit label (BB has to be split)
    ******************************************************************************/
    void visit(Cfg *pCfg, Address uNewAddr, BasicBlock *& pNewBB);


    /***************************************************************************/ /**
    * \brief    Seed the queue with an initial address
    * \note        Can be some targets already in the queue now
    * \param    uAddr Native address to seed the queue with
    ******************************************************************************/
    void initial(Address uAddr);

    /***************************************************************************/ /**
    * \brief   Return the next target from the queue of non-processed
    *              targets.
    * \param   cfg - the enclosing CFG
    * \returns The next address to process, or Address::INVALID if none
    *          (targets is empty)
    ******************************************************************************/
       Address nextAddress(const Cfg& cfg);

    /// Print (for debugging)
    void dump();
};
