/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/** \file BinaryFile.cpp
 * \brief This file contains the implementation of the class BinaryFile
 *
 * This file implements the abstract BinaryFile class.
 * All classes derived from this class must implement the Load()
 * function.
*/

/***************************************************************************/ /**
  * Dependencies.
  ******************************************************************************/

#include "BinaryFile.h"
#include "IBinaryImage.h"

#include <cstring>
#include <cstdio>
#include <cstddef>
#include <boomerang.h>


///////////////////////
// Trivial functions //
// Overridden if reqd//
///////////////////////


