/*
 * Copyright (C) 2000-2001, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       type.h
 * OVERVIEW:   Definition of the Type class: low level type information
 *             Note that we may have a compeltely different system for
 *              recording high level types
 *============================================================================*/

/*
 * $Revision$
 *
 * 20 Mar 01 - Mike: Added operator*= (compare, ignore sign, and consider all
 *                  floats > 64 bits to be the same
 * 26 Apr 01 - Mike: Added class typeLessSI
 * 08 Apr 02 - Mike: Changes for boomerang
 */

#ifndef __TYPE_H__
#define __TYPE_H__

#include <string>
#include <functional>       // For binary_function

// These are the types that we can recover and detect for locations in RTLs. The
// recovery may result either from using information in header files or through
// analysis
enum LOC_TYPE {
    TVOID = 0,               // void (for return type only)
    INTEGER,                 // integer (any size and signedness)
    FLOATP,                  // a floating point (any size), windows headers have FLOAT
    DATA_ADDRESS,            // a pointer to some data (e.g. char*, struct*,
                             // float* etc)
    FUNC_ADDRESS,            // a pointer to a function
    VARARGS,                 // variable arguments from here on, i.e. "..."
    BOOLEAN,                 // a true/false value
    UNKNOWN
};


class Type
{
    LOC_TYPE    type;               // The broad type, e.g. INTEGER
    int         size;               // Size in bits, e.g. 16
    bool        signd;              // True if a signed quantity

public:
    // Constructors
                Type();
                Type(LOC_TYPE ty, int sz = 32, bool sg = true);

    // Return type for given temporary variable name
    static Type* getTempType(const std::string &name);

    // Comparisons
    bool        operator==(const Type& other) const;    // Considers sign
    bool        operator!=(const Type& other) const;    // Considers sign
    bool        operator-=(const Type& other) const;    // Ignores sign
    bool        operator< (const Type& other) const;    // Considers sign
    bool        operator<<(const Type& other) const;    // Ignores sign 
    bool        operator*=(const Type& other) const;    // Considers all float
                                                        // sizes > 64 bits to be
                                                        // the same

    // Access functions
    int         getSize() const { return size;}
    LOC_TYPE    getType() const { return type;}
    bool        getSigned() const { return signd;}

    // Set functions
    void        setSize(int s) { size = s;}
    void        setType(LOC_TYPE t) {type = t;}
    void        setSigned(bool s) {signd = s;}

    // Update functions
    // ? Not used?
    // void        apply(int op, int newSize);    // Apply type change operator

    // Format functions
    const char* getCtype() const;   // Get the C type, e.g. "unsigned int16"
    std::string getTempName() const; // Get a temporary name for the type
};

/*==============================================================================
 * FUNCTION:    typeLessSI::operator()
 * OVERVIEW:    This class can be used as the third parameter to a map etc
 *                to produce an object which is sign insensitive
 *                Example: map<Type, SemStr, typeLessSI> is a map from Type
 *                  to SemStr where signed int32 and unsigned int32 map to the
 *                  same SemStr
 * PARAMETERS:  None
 * RETURNS:     True if x < y, disregarding sign, and considering all float
 *                types greater than 64 bits to be the same
 *============================================================================*/
class typeLessSI : public std::binary_function<Type, Type, bool> {
public:
    bool operator() (const Type& x, const Type& y) const {
        return (x << y);    // This is sign insensitive less, not "left shift"
    }
};

#endif  // __TYPE_H__
