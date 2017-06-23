#include "TargetQueue.h"

#include "boomerang/db/cfg.h"
#include "boomerang/util/Log.h"


void TargetQueue::visit(Cfg *pCfg, ADDRESS uNewAddr, BasicBlock *& pNewBB)
{
	// Find out if we've already parsed the destination
	bool alreadyParsed = pCfg->label(uNewAddr, pNewBB);

	// Add this address to the back of the local queue,
	// if not already processed
	if (!alreadyParsed) {
		targets.push(uNewAddr);

		if (Boomerang::get()->traceDecoder) {
			LOG << ">" << uNewAddr << "\t";
		}
	}
}

void TargetQueue::initial(ADDRESS uAddr)
{
	targets.push(uAddr);
}


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
