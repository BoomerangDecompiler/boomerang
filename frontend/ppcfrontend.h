
#ifndef PPCFRONTEND_H
#define PPCFRONTEND_H

// Class PPCFrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>
#include "decoder.h"
#include "exp.h"			// Ugh... just for enum OPER
#include "frontend.h"		// In case included bare, e.g. ProcTest.cpp

class FrontEnd;
class PPCDecoder;
struct DecodeResult;
class CallStatement;

class PPCFrontEnd : public FrontEnd
{
public:
  PPCFrontEnd(BinaryFile *pBF);
	/**
	 * Virtual destructor.
	 */
virtual ~PPCFrontEnd();

virtual platform getFrontEndId() { return PLAT_PPC; }

virtual bool		processProc(ADDRESS uAddr, UserProc* pProc, std::ofstream &os, bool frag = false,
						bool spec = false);


virtual std::vector<Exp*> &getDefaultParams();
virtual std::vector<Exp*> &getDefaultReturns();

virtual ADDRESS getMainEntryPoint( bool &gotMain );
	
};

#endif
