/*==============================================================================
 * FILE:       type.h
 * OVERVIEW:   Provides the definition for the Type class.
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Trent: Created
 */

#ifndef __TYPE_H_
#define __TYPE_H_

#include <fstream>      // For ostream, cout etc
#include <stdio.h>      // For sprintf

/*============================================================================== 
 * Type is a type class, it holds all information about a type.  This includes
 * both low level and high level types such as structs and unions.
 *
 *============================================================================*/
class Type {
public:
   void    print(ostream& os = cout);
};

#endif // __TYPE_H__
