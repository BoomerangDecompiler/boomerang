#pragma once

// Class PPCFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>
#include "include/decoder.h"
#include "db/exp.h"           // Ugh... just for enum OPER
#include "include/frontend.h" // In case included bare, e.g. ProcTest.cpp

class FrontEnd;
class PPCDecoder;
struct DecodeResult;

class CallStatement;

class PPCFrontEnd : public FrontEnd
{
public:
	PPCFrontEnd(QObject *pLoader, Prog *Program, BinaryFileFactory *pbff);

	/**
	 * Virtual destructor.
	 */
	virtual ~PPCFrontEnd();

	virtual Platform getFrontEndId() const { return PLAT_PPC; }

	virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream& os, bool frag = false, bool spec = false);

	virtual std::vector<SharedExp>& getDefaultParams();

	virtual std::vector<SharedExp>& getDefaultReturns();

	virtual ADDRESS getMainEntryPoint(bool& gotMain);
};
