/*
 * Copyright (C) 2000, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       coverage.h
 * OVERVIEW:   Interface for an class that keeps track of the amount of the
 *             source program that has been covered (decoded)
 *============================================================================*/
/*
 * $Revision$
 * 09 Jun 00 - Mike: Created
 */

#ifndef __COVERAGE_H__
#define __COVERAGE_H__

#include <iostream>
#include <map>
#include "types.h"

typedef std::map<ADDRESS, ADDRESS>::iterator COV_IT;     // Iterator types
typedef std::map<ADDRESS, ADDRESS>::const_iterator COV_CIT;

class Coverage {
    // The following map represents a set of [start, finish+1] pairs
    std::map<ADDRESS, ADDRESS>    ranges;

public:
    /*
     * Add a range from st up to but not including fi
     */
    void    addRange(ADDRESS st, ADDRESS fi);
    /*
     * Add all the ranges from another Coverage object
     */
    void    addRanges(const Coverage& other);
    /*
     * Print this Coverage object to stream os
     */
    void    print(std::ostream& os = std::cout) const;
    /*
     * Get the total coverage (in bytes) for all the ranges in this object
     */
    unsigned totalCover() const;
    /*
     * Get the first gap (between ranges) for this Coverage object
     */
    bool    getFirstGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it) const;

    /*
     * Get the next gap (between ranges) for this Coverage object
     */
    bool    getNextGap(ADDRESS& a1, ADDRESS& a2, COV_CIT& it) const;

    /*
     * Find the end of the range overlapping the given address
     */
    ADDRESS findEndRange(ADDRESS a1);

    /*
     * Return the start address in the first range
     */
    ADDRESS getStartOfFirst() const;

    /*
     * Return the end address of the last range
     */
    ADDRESS getEndOfLast() const;

};

#endif      // __COVERAGE_H__

