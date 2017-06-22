#include "boomerang/include/TargetQueue.h"

#include "boomerang/util/Log.h"

#include "boomerang/db/cfg.h"

/***************************************************************************/ /**
 *
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
void TargetQueue::visit(Cfg *pCfg, ADDRESS uNewAddr, BasicBlock *& pNewBB)
{
	// Find out if we've already parsed the destination
	bool bParsed = pCfg->label(uNewAddr, pNewBB);

	// Add this address to the back of the local queue,
	// if not already processed
	if (!bParsed) {
		targets.push(uNewAddr);

		if (Boomerang::get()->traceDecoder) {
			LOG << ">" << uNewAddr << "\t";
		}
	}
}


/***************************************************************************/ /**
 *
 * \brief    Seed the queue with an initial address
 * \note        Can be some targets already in the queue now
 * \param    uAddr Native address to seed the queue with
 ******************************************************************************/
void TargetQueue::initial(ADDRESS uAddr)
{
	targets.push(uAddr);
}


/***************************************************************************/ /**
 *
 * \brief   Return the next target from the queue of non-processed
 *              targets.
 * \param   cfg - the enclosing CFG
 * \returns The next address to process, or NO_ADDRESS if none
 *          (targets is empty)
 ******************************************************************************/
ADDRESS TargetQueue::nextAddress(const Cfg& cfg)
{
	while (!targets.empty()) {
		ADDRESS address = targets.front();
		targets.pop();

		if (Boomerang::get()->traceDecoder) {
			LOG << "<" << address << "\t";
		}

		// If no label there at all, or if there is a BB, it's incomplete, then we can parse this address next
		if (!cfg.existsBB(address) || cfg.isIncomplete(address)) {
			return address;
		}
	}

	return NO_ADDRESS;
}


/**
 * Print (for debugging)
 */
void TargetQueue::dump()
{
	std::queue<ADDRESS> copy(targets);

	while (!copy.empty()) {
		ADDRESS a = copy.front();
		copy.pop();
		LOG_STREAM() << a << ", ";
	}

	LOG_STREAM() << "\n";
}
