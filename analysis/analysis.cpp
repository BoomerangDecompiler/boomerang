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
 * OVERVIEW:        Perform any higher level analysis on a procedure. At
 *                  present, this is the removal of uses of flags (checkBBflags)
 *                  and a pass looking for constants in the text section
 *                  (checkBBconst)
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
        // Match flags
        //checkBBflags(pBB, proc);
        // Perform final simplifications
        finalSimplify(pBB);
#if USE_PROCESS_CONST       // Fatally flawed. Somehow, conversion of
                            // strings and function pointers has disappeared!
        // Check for constants
        checkBBconst(proc->getProg(), pBB);
#endif // USE_PROCESS_CONST

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

#if USE_PROCESS_CONST
/*==============================================================================
 * FUNCTION:      processConst
 * OVERVIEW:      Replace the RHS of the current assignment with a floating
 *                  point constant (or integer constant in rare cases)
 * NOTE:          Assumes that the floating point constant binary format
 *                  is compatible with the compiler used to compile this code
 * PARAMETERS:    addr: native address of the constant
 *                memExp: reference to a Exp* which is the m[] to be replaced
 *                  with the constant
 *                RHS: pointer to the RHS to be changed
 * RETURNS:       <nothing>; note that the RHS Exp* gets changed
 *============================================================================*/
void Analysis::processConst(Prog *prog, ADDRESS addr, Exp*& memExp, Exp* RHS)
{
    const void* p;
    const char* last;
    int delta;
    union {
        double d;
        int i[2];
    } value;
    p = prog->getCodeInfo(addr, last, delta);
    if (p == NULL) return;      // No code section pointer there
    int size = memExp.getType().getSize();

    // Exp*'s are deemed to be same endianness as the source machine:
    // i.e., the constant is in src endianness.
    // Let "uqbt endianness" = endianness of machine UQBT is running on now.
    // We must swap the constant during translation if 
    //     uqbt endianness != src endianness
    // since then we must swap any constant read during translation.
    bool doSwaps;
    #if ((WORDS_BIGENDIAN == 1) && (SRCENDIAN == LITTLEE)) || ((WORDS_BIGENDIAN == 0) && (SRCENDIAN == BIGE))
        doSwaps = true;
    #else
        doSwaps = false;
    #endif
        
    Exp* con = 0;
    if (memExp.getType().getType() == FLOATP) {
        if (doSwaps) {
            if (size == sizeof(float)*8) {
                int i = copySwap4((int*)p);
                value.d = (double)*(float*)&i;
            } else if (size == sizeof(double)*8) {
                int ii[2];
                ii[1] = copySwap4((int*)p);
                ii[0] = copySwap4(((int*)p)+1);
                value.d = *(double*)ii;
            }
        } else {
            if (size == sizeof(float)*8)
                value.d = (double)*(float*)p;
            else if (size == sizeof(double)*8)
                value.d = *(double*)p;
        }
        con = new Const(value.d);
    } else {
        // The constant is integer.
        int k;
        if (doSwaps) {
            if (size == sizeof(int)*8) {
                k = copySwap4((int*)p);
            } else if (size == sizeof(short)*8) {
                k = copySwap2((short*)p);
            } else
                assert(0);
        } else {
            if (size == sizeof(int)*8)
                // Note: there could be alignment issues!
                k = *(int*)p;
            else if (size == sizeof(short)*8)
                k = *(short*)p;
            else
                assert(0);
        }
        con = new Const(k);
    }

    // Replace the right hand side with the constant
    // MVE: Does this have to be type sensitive sometimes? Doubtful.
    RHS->searchReplaceAll(memExp, *con);
    delete con;
}

/*==============================================================================
 * FUNCTION:        isRegister
 * OVERVIEW:        Return true if the given expression is a register, var, or
 *                      a temp. Only constant register numbers are considered
 * PARAMETERS:      exp: pointer to the expression to be checked
 * RETURNS:         True if matched
 *============================================================================*/
bool Analysis::isRegister(const Exp* exp)
{
    if (exp->getFirstIdx() == idRegOf)
        return exp->getSecondIdx() == idIntConst;
    if (exp->getFirstIdx() == idVar)
        return true;
    if (exp->getFirstIdx() == idTemp)
        return true;
    return false;
}

/*==============================================================================
 * FUNCTION:        isConstTerm
 * OVERVIEW:        Return true if the given expression is a constant or a
 *                      register that is already known to be const
 * PARAMETERS:      exp: pointer to the expression to be checked
 *                  constMap: reference to the map of existing constant regs
 *                  value: reference to an ADDRESS that will be set to the
 *                    value of the constant
 * RETURNS:         True if matched, and value is set
 *============================================================================*/
bool Analysis::isConstTerm(const Exp* exp, const KMAP& constMap, ADDRESS& value)
{
    if (exp->getFirstIdx() == idIntConst) {
        value = exp->getSecondIdx();
        return true;
    }
    if (isRegister(exp)) {
        KMAP::const_iterator it;
        it = constMap.find(*exp);
        if (it == constMap.end())
            // Not a constant register
            return false;
        // A constant register
        value = it->second;
        return true;
    }
    // Not a simple constant or a register
    return false;
}

/*==============================================================================
 * FUNCTION:        isConst
 * OVERVIEW:        Return true if the given expression is constant
 * PARAMETERS:      exp: pointer to the expression to be checked
 *                    constMap: reference to the map of existing constant regs
 *                    value: reference to an ADDRESS that will be set to the value
 *                      of the constant
 * RETURNS:         True if matched, and value is set
 *============================================================================*/
bool Analysis::isConst(const Exp* exp, const KMAP& constMap, ADDRESS& value)
{
    if (isConstTerm(exp, constMap, value))
        return true;
    int idx = exp->getFirstIdx();
    switch (idx) {
        case idPlus:
        case idMinus:
        case idBitOr:
        {
            // Expect op k1 k2 where k1 and k2 are constants    
            Exp* k1; ADDRESS value1;
            k1 = exp->getSubExpr(0);
            if (!isConstTerm(k1, constMap, value1)) {
                delete k1;
                return false;
            }
            Exp* k2; ADDRESS value2;
            k2 = exp->getSubExpr(1);
            if (!isConstTerm(k2, constMap, value2)) {
                delete k2;
                return false;
            }
            delete k1; delete k2;
            switch (idx) {
                case idPlus:  value = value1 + value2; break;
                case idMinus: value = value1 - value2; break;
                case idBitOr: value = value1 | value2; break;
            }
            return true;
        }
    }
    return false;
}

/*==============================================================================
 * FUNCTION:        checkBBconst
 * OVERVIEW:        Check the current BB for the presence of constants; create
 *                  pointers to functions or floating point constants if needed
 * NOTE:            This is a temporary solution, which does not attempt to
 *                  do data flow analysis, and only checks the current BB
 * PARAMETERS:      pBB: Pointer to the BB to be checked
 *                  proc: Ptr to UserProc object for the current procedure
 * RETURNS:         <nothing>
 *============================================================================*/
void Analysis::checkBBconst(Prog *prog, PBB pBB)
{
    std::list<RTL*>* pRtls = pBB->getRTLs();
    if (pRtls == 0)
        return;
    // The following is a map from Exp* (representing the register, var, or
    // temp being assigned a constant, to ADDRESS (representing the constant
    // that is held there). This is needed because constant addresses are some-
    // time arrived at by operations like + or | on other constants.
    // For example, see the ninths test program (Sparc)
    KMAP constMap;

    std::list<RTL*>::iterator rit;
    for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
        int n = (*rit)->getNumExp();
        for (int i=0; i < n; i++) {
            RTAssgn* pRT = (RTAssgn*)(*rit)->elementAt(i);
            if (pRT->getKind() != RTASSGN) continue;
            // Locate any register loads where the RHS is constant. The constant
            // could be a simple constant, or a register that is already known
            // to be constant, or simple operation (+, -, |) on two of these
            Exp* LHS = pRT->getSubExp1();
            Exp* RHS = pRT->getSubExp2();
            ADDRESS value;
            if (isRegister(LHS)) {
                if (isConst(RHS, constMap, value))
                    constMap[*LHS] = value;
                else
                    // If the LHS is already in the map, delete it, because it's
                    // no longer constant
                    constMap.erase(*LHS);
            }

            // Locate any memory loads where the destination is a register.
            // We need to check more than just floating point registers, because
            // sometimes a double is loaded into two integer registers. In this
            // last case, the constants are treated as two integers.
            // The memory reference is not necessarily at the "top level", e.g.
            // r[32] := r[32] *f fsize(64, 80, m[const]);
            std::list<Exp*> result;
            if (!RHS->searchAll(memX, result))
                // No memOfs; ignore
                continue; 
            // We have at leat one memOf; go through the list
            std::list<Exp*>::iterator it;
            for (it=result.begin(); it != result.end(); it++) {
                Exp* address;
                address = (*it)->getSubExpr(0);
                if (isConst(address, constMap, value))
                    // This is what we are waiting for: assignment from a
                    // constant memory address. value is the const address
                    processConst(prog, value, **it, RHS);
                delete address;
            }
        }
    }
}
#endif // USE_PROCESS_CONST

    // analyse calls
void Analysis::analyseCalls(PBB pBB, UserProc *proc)
{
    std::list<RTL*>* rtls = pBB->getRTLs();
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

