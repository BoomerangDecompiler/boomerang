#pragma once

// Class PPCFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>

#include "boomerang/db/exp.h"           // Ugh... just for enum OPER

#include "boomerang/frontend/frontend.h"

class IFrontEnd;
class PPCDecoder;
class CallStatement;
struct DecodeResult;


class PPCFrontEnd : public IFrontEnd
{
public:
	/// @copydoc IFrontEnd::IFrontEnd
	PPCFrontEnd(IFileLoader *pLoader, Prog *Program, BinaryFileFactory *pbff);
	
	/// @copydoc IFrontEnd::~IFrontEnd
	virtual ~PPCFrontEnd();

	/// @copydoc IFrontEnd::getFrontEndId
	virtual Platform getType() const override { return Platform::PPC; }

	/// @copydoc IFrontEnd::processProc
	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

	/// @copydoc IFrontEnd::getDefaultParams
	virtual std::vector<SharedExp>& getDefaultParams() override;

	/// @copydoc IFrontEnd::getDefaultReturns
	virtual std::vector<SharedExp>& getDefaultReturns() override;

	/// @copydoc IFrontEnd::getMainEntryPoint
	virtual ADDRESS getMainEntryPoint(bool& gotMain) override;
};
