/*==============================================================================
 * FILE:       func.h
 * OVERVIEW:   Provides the definition for the Func class.
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Trent: Created
 */

#ifndef __FUNC_H_
#define __FUNC_H_

#include <fstream>      // For ostream, cout etc
#include <stdio.h>      // For sprintf

/*============================================================================== 
 * Func is a function class, it hold alls information about a function.  You
 * may be familiar with "procedures" - these are the same thing except not
 * as sexy as functions.
 *============================================================================*/
class Func {
   BlockStmt *body;
public:
   void    print(ostream& os = cout);
};

#endif // __FUNC_H__
