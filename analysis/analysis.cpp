/*
 * Copyright (C) 1999-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       analysis.cpp
 * OVERVIEW:   Perform higher level analysis on a procedure. The procedure has
 *              been decoded, and parameter analysis is complete, but a few
 *              source machine idioms may remain, especially flags. This module
 *              attempts to remove the flag uses
 *============================================================================*/

/*
 * $Revision$
 *
 * 10 Jul 02 - Mike: Mods for boomerang
 */

#include <assert.h>

#include <sstream>
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "decoder.h"
#include "analysis.h"
#include "boomerang.h"

// For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#include "rtl.h"
#endif

#define DEBUG_ANALYSIS 0        // Non zero for debugging

// A global array for initialising the below
int memXinit[] = {opMemOf, -1};
// A global Exp* representing a wildcard memory expression
//Exp* memX(memXinit, memXinit+2);


/*==============================================================================
 * FUNCTION:        simplify
 * OVERVIEW:        Perform any simplifications, post decoding.
 * PARAMETERS:      pBB: pointer to the BB to be simplified
 * RETURNS:         <none>
 *============================================================================*/
void Analysis::finalSimplify(PBB pBB)
{
    std::list<RTL*>* pRtls = pBB->getRTLs();
	if (pRtls == NULL)
		return;
    std::list<RTL*>::iterator rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        for (int i=0; i < (*rit)->getNumStmt(); i++) {
            Statement* rt = (*rit)->elementAt(i);
            rt->simplifyAddr();
            // Also simplify everything; in particular, stack offsets are
            // often negative, so we at least canonicalise [esp + -8] to [esp-8]
            rt->simplify();
        }
    }
}

/*==============================================================================
 * FUNCTION:        analyse
 * OVERVIEW:        THis was originally to perform any higher level analysis
 *                  on a procedure. At present, it is only a few miscallaneous
 *                  details
 * PARAMETERS:      proc: pointer to a Proc object for the procedure being
 *                  analysed
 * RETURNS:         <none>
 *============================================================================*/
void Analysis::analyse(UserProc* proc) {
    Cfg* cfg = proc->getCFG();
    std::list<PBB>::iterator it;
    PBB pBB = cfg->getFirstBB(it);
    while (pBB)
    {
        // Perform final simplifications
       finalSimplify(pBB);

        // analyse calls
        analyseCalls(pBB, proc);

        pBB = cfg->getNextBB(it);
    }

    cfg->simplify();
}

#if DEBUG_ANALYSIS
static int findDefsCallDepth = 0;
#endif


/*==============================================================================
 * FUNCTION:        copySwap4
 * OVERVIEW:        Load a 4 byte word from little to big endianness,
 *                  or vice versa
 *                  Similar to _swap4 macro, but this for translate time
 * NOTE:            Processes char at a time, so avoids alignment hassles
 * PARAMETERS:      w: Pointer to source word
 * RETURNS:         The source word as an integer
 *============================================================================*/
int Analysis::copySwap4(int* w)
{
    char* p = (char*)w;
    int ret;
    char* q = (char*)(&ret+1);
    *--q = *p++; *--q = *p++; *--q = *p++; *--q = *p;
    return ret;
}

/*==============================================================================
 * FUNCTION:        copySwap2
 * OVERVIEW:        Load a 2 byte short word from little to big endianness,
 *                  or vice versa
 *                  Similar to _swap2 macro, but this for translate time
 * NOTE:            Processes char at a time, so avoids alignment hassles
 * PARAMETERS:      w: Pointer to source short word
 * RETURNS:         The source short word as an integer
 *============================================================================*/
int Analysis::copySwap2(short* h)
{
    char* p = (char*)h;
    short ret;
    char* q = (char*)(&ret);
    *++q = *p++; *--q = *p;
    return (int)ret;
}


    // analyse calls
void Analysis::analyseCalls(PBB pBB, UserProc *proc)
{
    std::list<RTL*>* rtls = pBB->getRTLs();
	if (rtls == NULL)
		return;
    for (std::list<RTL*>::iterator it = rtls->begin(); it != rtls->end(); 
      it++) {
        if (!(*it)->isCall()) continue;
        CallStatement* call = (CallStatement*)(*it)->getList().back();
        if (call->getDestProc() == NULL && !call->isComputed()) {
            Proc *p = proc->getProg()->findProc(call->getFixedDest());
            if (p == NULL) {
                LOG << "cannot find proc for dest " 
                    << call->getFixedDest() << " in call at " 
                    << (*it)->getAddress() << "\n";
                assert(p);
            }
            call->setDestProc(p);
        }
        call->setSigArguments();
    }
}

