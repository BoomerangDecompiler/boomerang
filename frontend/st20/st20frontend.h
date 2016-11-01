#ifndef ST20FRONTEND_H
#define ST20FRONTEND_H

// Class ST20FrontEnd: derived from FrontEnd, with source machine specific
// behaviour

#include <set>
#include "decoder.h"
#include "exp.h"      // Ugh... just for enum OPER
#include "frontend.h" // In case included bare, e.g. ProcTest.cpp

class FrontEnd;
class ST20Decoder;
struct DecodeResult;
class CallStatement;

class ST20FrontEnd : public FrontEnd {
  public:
    ST20FrontEnd(QObject *pLoader, Prog *prog, BinaryFileFactory *pbff);
    /**
         * Virtual destructor.
         */
    virtual ~ST20FrontEnd();

    virtual platform getFrontEndId() { return PLAT_ST20; }

    virtual bool processProc(ADDRESS uAddr, UserProc *pProc, QTextStream &os, bool frag = false, bool spec = false);

    virtual std::vector<SharedExp> &getDefaultParams();
    virtual std::vector<SharedExp> &getDefaultReturns();

    virtual ADDRESS getMainEntryPoint(bool &gotMain);
};

#endif
