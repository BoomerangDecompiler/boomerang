/*==============================================================================
 * FILE:       decl.h
 * OVERVIEW:   Provides the definition for the Decl class.
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Trent: Created
 */

#ifndef __DECL_H_
#define __DECL_H_

#include <fstream>      // For ostream, cout etc
#include <stdio.h>      // For sprintf

/*============================================================================== 
 * Decl is a declaration class, it holds all information about a declaration.
 *
 *============================================================================*/
class Decl {
public:
   void    print(ostream& os = cout);
};

#endif // __DECL_H__
