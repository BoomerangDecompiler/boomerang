/*
 * Copyright (C) 2003, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   exphelp.h
 * OVERVIEW:   Element comparison functions for expressions and statements
 *============================================================================*/

/*
 * $Revision$
 *
 * 01 Jul 05 - Mike: added header
 */

#ifndef __EXPHELP_H__
#define __EXPHELP_H__

#include	<map>

class Exp;
class Assignment;

/*
 * A class for comparing Exp*s (comparing the actual expressions)
 * Type sensitive
 */
class lessExpStar : public std::binary_function<Exp*, Exp*, bool> {
public:
	bool operator()(const Exp* x, const Exp* y) const;
};


/*
 * A class for comparing Exp*s (comparing the actual expressions)
 * Type insensitive
 */
class lessTI : public std::binary_function<Exp*, Exp*, bool> {
public:
	bool operator()(const Exp* x, const Exp* y) const;
};

// Compare assignments by their left hand sides (only). Implemented in statement.cpp
class lessAssignment : public std::binary_function<Assignment*, Assignment*, bool> {
public:
	bool operator()(const Assignment* x, const Assignment* y) const;
};

// A type for an "interference graph". Needed by various classes to implement the transforation out of SSA form.
typedef std::map<Exp*, Exp*, lessExpStar> igraph;

#endif		// __EXPHELP_H__
