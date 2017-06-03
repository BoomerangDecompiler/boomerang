#pragma once

// Class ST20FrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>
#include "include/decoder.h"
#include "db/exp.h"           // Ugh... just for enum OPER
#include "include/frontend.h" // In case included bare, e.g. ProcTest.cpp

class FrontEnd;
class ST20Decoder;
struct DecodeResult;

class CallStatement;

class ST20FrontEnd : public FrontEnd
{
public:
	ST20FrontEnd(QObject *pLoader, Prog *prog, BinaryFileFactory *pbff);

	/**
	 * Virtual destructor.
	 */
	virtual ~ST20FrontEnd();

	virtual Platform getFrontEndId() const override { return PLAT_ST20; }

	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false) override;

	virtual std::vector<SharedExp>& getDefaultParams() override;

	virtual std::vector<SharedExp>& getDefaultReturns() override;

	virtual ADDRESS getMainEntryPoint(bool& gotMain) override;
};
