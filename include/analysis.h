/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * FILE:        analysis.h
 * OVERVIEW:    interface for the analysis object.
 *============================================================================*/
/*
 * $Revision$
 * Created by Trent
 * 07 Oct 02 - Trent: Created
 */

#ifndef _ANALYSIS_H_
#define _ANALYSIS_H_

typedef std::map<Exp*, ADDRESS, lessExpStar> KMAP;

class Analysis {
public:
    Analysis() { }
    ~Analysis() { }
    void analyse(UserProc* proc);

protected:
    OPER anyFlagsUsed(Exp* s);
    int copySwap4(int* w);
    int copySwap2(short* h);
    void findDefs(PBB pBB, UserProc* proc,
                  int ty, bool flt, std::list<RTL*>::iterator rit, 
                  bool withAssign);
    void matchJScond(std::list<RTL*>* pOrigRtls, PBB pOrigBB,
                     std::list<RTL*>::reverse_iterator rrit, UserProc* proc,
                     std::list<RTL*>::iterator rit, int ty);
    void matchAssign(std::list<RTL*>* pOrigRtls, PBB pOrigBB,
                     std::list<RTL*>::reverse_iterator rrit, UserProc* proc,
                     std::list<RTL*>::iterator rit, int flagUsed);
    void processSubFlags(RTL* rtl, std::list<RTL*>::reverse_iterator rrit,
                         UserProc* proc, RTL_KIND kd);
    void processAddFlags(RTL* rtl, std::list<RTL*>::reverse_iterator rrit,
                         UserProc* proc, RTL_KIND kd);
    void checkBBflags(PBB pBB, UserProc* proc);
    void checkBBconst(PBB pBB);
    void finalSimplify(PBB pBB);
    bool isFlagFloat(Exp* rt, UserProc* proc);
    void analyseCalls(PBB pBB, UserProc *proc);
};

#endif
