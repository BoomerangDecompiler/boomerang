/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       type.cc
 * OVERVIEW:   Implementation of the Type class: low level type information
 *============================================================================*/

/*
 * $Revision$
 *
 * 28 Apr 02 - Mike: getTempType() returns a Type* now
 */

#include "type.h"


/*==============================================================================
 * FUNCTION:        Type::Type
 * OVERVIEW:        Default constructor
 * PARAMETERS:      <none>
 * RETURNS:         <Not applicable>
 *============================================================================*/
Type::Type()
  : type(INTEGER), size(32), signd(true)
{
}

/*==============================================================================
 * FUNCTION:        Type::Type
 * OVERVIEW:        Constructor with args
 * PARAMETERS:      ty: broad type (e.g. FLOAT)
 *                  sz: size in bits (defaults to 32)
 *                  sg: true if signed (defaults to true)
 * RETURNS:         <Not applicable>
 *============================================================================*/
Type::Type(LOC_TYPE ty, int sz, bool sg)
  : type(ty), size(sz), signd(sg)
{
}

/*==============================================================================
 * FUNCTION:        Type::operator==
 * OVERVIEW:        Equality comparsion.
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other
 *============================================================================*/
bool Type::operator==(const Type& other) const
{
    return (type == other.type) && (size == other.size) &&
        (signd == other.signd);
}

/*==============================================================================
 * FUNCTION:        Type::operator!=
 * OVERVIEW:        Inequality comparsion.
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other
 *============================================================================*/
bool Type::operator!=(const Type& other) const
{
    return !(*this == other);
}

/*==============================================================================
 * FUNCTION:        Type::operator-=
 * OVERVIEW:        Equality operator, ignoring sign. True if equal in broad
 *                    type and size, but not necessarily sign
 *                    Considers all float types > 64 bits to be the same
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other (ignoring sign)
 *============================================================================*/
bool Type::operator-=(const Type& other) const
{
//    return (type == other.type) && (size == other.size);
    if (type != other.type) return false;
    if ((type == FLOATP) && (size > 64) && (other.size > 64))
        return true;
    return (size == other.size);
}

/*==============================================================================
 * FUNCTION:        Type::operator*=
 * OVERVIEW:        Equality operator, considers sign, but considers all float
 *                    sizes > 64 bits to be the same
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this == other (ignoring sign)
 *============================================================================*/
bool Type::operator*=(const Type& other) const
{
    if (type != other.type) return false;
    if ((type == FLOATP) && (size > 64) && (other.size > 64))
        return true;
    if (signd != other.signd) return false;
    return (size == other.size);
}

/*==============================================================================
 * FUNCTION:        Type::operator<<
 * OVERVIEW:        Defines an ordering between Type's (and hence SemStr's).
 *                    Considers all float types > 64 bits to be equal
 * NOTE:            Not left shift! Sign is not considered in this comparison
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this is less than other (ignoring sign)
 *============================================================================*/
bool Type::operator<<(const Type& other) const
{
    if (type < other.type) return true;     // Type is "most significant" wrt
    if (type > other.type) return false;    //   the ordering
    if ((type == FLOATP) && (size > 64) && (other.size > 64))
        // These are considered equal, therefore not less
        return false;
    if (size < other.size) return true;
    if (size > other.size) return false;
    return false;                           // Equal
}

/*==============================================================================
 * FUNCTION:        Type::operator<
 * OVERVIEW:        Defines an ordering between Type's (and hence SemStr's).
 * NOTE:            Same as lessSGI, except that sign is also considered
 * PARAMETERS:      other - Type being compared to
 * RETURNS:         this is less than other
 *============================================================================*/
bool Type::operator<(const Type& other) const
{
    if (*this << other) return true;
    if (other << *this) return false;
    if (signd && !other.signd) return true;
    return false;                           // Equal
}

/*==============================================================================
 * FUNCTION:        Type::getCtype
 * OVERVIEW:        Return a string representing this type
 * PARAMETERS:      <none>
 * RETURNS:         Pointer to a constant string of char
 *============================================================================*/
const char* Type::getCtype() const
{
    switch (type) {
        case INTEGER:
            if (signd) {
                switch (size) {
                    case 32: return "int32"; break;
                    case 16: return "int16"; break;
                    case  8: return "int8"; break;
                    case  1: return "int"; break;
                    case 64: return "int64"; break;
                    default: return "?";
                }
            } else
            {
                switch (size) {
                    case 32: return "unsigned int32"; break;
                    case 16: return "unsigned int16"; break;
                    case  8: return "unsigned int8"; break;
                    case  1: return "int"; break;
                    case 64: return "unsigned int64"; break;
                    default: return "?";
                }
            }
            break;

        case FLOATP:
            switch (size) {
                case 32: return "float32"; break;
                case 64: return "float64"; break;
                default: return "floatmax"; break;
            }
            break;

        case DATA_ADDRESS:
            return "void*"; break;
            break;

        case FUNC_ADDRESS:
            return "void()"; break;         // This needs fixing
            break;

        case TVOID:
            return "void"; break;
            break;

        case VARARGS:
            return "..."; break;
            break;

        case BOOLEAN:
            return "int"; break;

        default:
            return "?"; break;
    }
}

/*==============================================================================
 * FUNCTION:        locTypeName
 * OVERVIEW:        Return a string representing this broad type
 * NOTE:            Probably not used right now (only ever needed for debugging)
 * PARAMETERS:      The broad type
 * RETURNS:         Pointer to a constant string of char
 *============================================================================*/
const char* locTypeName(LOC_TYPE ty)
{
    switch (ty) {
        case TVOID:      return "VOID";
        case INTEGER:   return "INTEGER";
        case FLOATP:     return "FLOAT";
        case DATA_ADDRESS: return "DATA_ADDRESS";
        case FUNC_ADDRESS: return "FUNC_ADDRESS";
        case VARARGS:   return "VARARGS";
        case BOOLEAN:   return "BOOLEAN";
        case UNKNOWN:   return "UNKNOWN";
    }
    return "??";
}

/*==============================================================================
 * FUNCTION:    getTempType
 * OVERVIEW:    Given the name of a temporary variable, return its Type
 * PARAMETERS:  name: reference to a string (e.g. "tmp", "tmpd")
 * RETURNS:     Ptr to a new Type object
 *============================================================================*/
Type* Type::getTempType(const std::string& name)
{
    Type* ty;
    char ctype = ' ';
    if (name.size() > 3) ctype = name[3];
    switch (ctype) {
        // They are all int32, except for a few specials
        case 'f': ty = new Type(FLOATP, 32); break;
        case 'd': ty = new Type(FLOATP, 64); break;
        case 'F': ty = new Type(FLOATP, 80); break;
        case 'D': ty = new Type(FLOATP, 128); break;
        case 'l': ty = new Type(INTEGER, 64); break;
        case 'h': ty = new Type(INTEGER, 16); break;
        case 'b': ty = new Type(INTEGER,  8); break;
        default:  ty = new Type(INTEGER, 32); break;
    }
    return ty;
}


/*==============================================================================
 * FUNCTION:    getTempName
 * OVERVIEW:    Return a minimal temporary name for this type. It'd be even
 *              nicer to return a unique name, but we don't know scope at
 *              this point, and even so we could still clash with a user-defined
 *              name later on :(
 * PARAMETERS:  
 * RETURNS:     a string
 *============================================================================*/
std::string Type::getTempName() const
{
    if( type == INTEGER ) {
        switch( size ) {
            case 1:  /* Treat as a tmpb */
            case 8:  return std::string("tmpb");
            case 16: return std::string("tmph");
            case 32: return std::string("tmpi");
            case 64: return std::string("tmpl");
        }
    } else if( type == FLOATP ) {
        switch( size ) {
            case 32: return std::string("tmpf");
            case 64: return std::string("tmpd");
            case 80: return std::string("tmpF");
            case 128:return std::string("tmpD");
        }
    }
    return std::string("tmp"); // what else can we do? (besides panic)
}
