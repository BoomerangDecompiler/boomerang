#pragma once

// Class ST20FrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>
#include "include/decoder.h"
#include "db/exp.h"           // Ugh... just for enum OPER

#include "boomerang-frontend/frontend.h"

class IFrontEnd;
class ST20Decoder;
class CallStatement;

struct DecodeResult;

class ST20FrontEnd : public IFrontEnd
{
public:
	ST20FrontEnd(IFileLoader *pLoader, Prog *prog, BinaryFileFactory *pbff);

	/**
	 * Virtual destructor.
	 */
	virtual ~ST20FrontEnd();

	virtual Platform getType() const override { return Platform::ST20; }

	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

	virtual std::vector<SharedExp>& getDefaultParams() override;

	virtual std::vector<SharedExp>& getDefaultReturns() override;

	virtual ADDRESS getMainEntryPoint(bool& gotMain) override;
};
