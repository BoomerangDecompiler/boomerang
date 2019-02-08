#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/core/BoomerangAPI.h"


/// The OPER of expressions. OPERs encode information about the top-level operator of the current
/// non-terminal or terminal expression.
/// \sa operToString
enum OPER
{
    // Operators
    opWildMemOf    = -6, ///< m[wild],
    opWildRegOf    = -5, ///< r[wild],
    opWildAddrOf   = -4, ///< a[wild],
    opWildIntConst = -3, ///< Terminal integer constant whose value is wild
    opWildStrConst = -2, ///< Terminal string constant whose value is wild
    opWild         = -1, ///< Wildcard (Terminal for search Exps only)

    opInvalid = 0, ///< Invalid operator

    // Integer operations
    opPlus,  ///< Binary addition
    opMinus, ///< Binary subtraction
    opMult,  ///< Multiplication
    opMults, ///< Multiply signed
    opDiv,   ///< Integer division
    opDivs,  ///< Divide signed
    opMod,   ///< Remainder of integer division
    opMods,  ///< Remainder of signed integer division
    opNeg,   ///< Unary minus

    // low-level integer operations
    opTruncu, ///< Integer truncate (unsigned)
    opTruncs, ///< Integer truncate (signed)
    opZfill,  ///< Integer zero fill
    opSgnEx,  ///< Integer sign extend

    // float operations
    opFPlus,  ///< Float addition
    opFMinus, ///< Float subtraction
    opFMult,  ///< Float multiply
    opFDiv,   ///< Float divide
    opFNeg,   ///< Floating point negate
    opFabs,   ///< floating point absolute function
    opSin,    ///< sine
    opCos,    ///< cosine
    opTan,    ///< tangent
    opArcTan, ///< inverse tangent
    opLog2,   ///< logarithm to base 2
    opLog10,  ///< logarithm to base 10
    opLoge,   ///< logarithm to base e
    opPow,    ///< raise to a power
    opSqrt,   ///< square root
    opFround, ///< Floating point to nearest float conversion
    opFtrunc, ///< chop float to int, e.g. 3.99 -> 3.00
    opFsize,  ///< Floating point size conversion
    opItof,   ///< Integer to floating point (and size) conversion
    opFtoi,   ///< Floating point to integer (and size) conversion

    // logical operations
    opAnd,       ///< Logical and
    opOr,        ///< Logical or
    opEquals,    ///< Equality (logical)
    opNotEqual,  ///< Logical !=
    opLess,      ///< Logical less than (signed)
    opGtr,       ///< Logical greater than (signed)
    opLessEq,    ///< Logical <= (signed)
    opGtrEq,     ///< Logical >= (signed)
    opLessUns,   ///< Logical less than (unsigned)
    opGtrUns,    ///< Logical greater than (unsigned)
    opLessEqUns, ///< Logical <= (unsigned)
    opGtrEqUns,  ///< Logical >= (unsigned)
    opLNot,      ///< Logical not

    // bit manipulation operations
    opBitNot, ///< Bitwise inversion
    opBitAnd, ///< Bitwise and
    opBitOr,  ///< Bitwise or
    opBitXor, ///< xor (Note: a ^ 5 is a bitwise operation, (a==b) ^ true is a logical operation)
    opShL,    ///< Left shift (zero filled from the right)
    opShR,    ///< Right shift (zero filed from the left)
    opShRA,   ///< Right shift arithmetic (MSB filled from the left)
    opRotL,   ///< Rotate left
    opRotR,   ///< Rotate right
    opRotLC,  ///< Rotate left through carry
    opRotRC,  ///< Rotate right through carry

    // constants
    opIntConst,  ///< integer constant TODO: differentiate IntConst by adding AddressConst ?
    opLongConst, ///< long integer constant
    opFltConst,  ///< floating point constant
    opStrConst,  ///< string constant
    opFuncConst, ///< a function constant (address of named function)
    opTrue,
    opFalse,

    // variables
    opParam,  ///< SSL parameter or parameter of a function
    opLocal,  ///< used to represent a local, takes a string
    opGlobal, ///< used to represent a global, takes a string
    opTemp,   ///< Temp register name

    // other operations
    opRegOf,        ///< Represents r[]
    opMemOf,        ///< Represents m[]
    opAddrOf,       ///< Represents a[]
    opTern,         ///< Ternary (i.e. ? : )
    opAt,           ///< Bit extraction (expr@[first:last] in that order)
    opMemberAccess, ///< . and -> in C
    opArrayIndex,   ///< [] in C
    opDefineAll,    ///< A wild definition
    opSubscript,    ///< Represents e{}
    opList,         ///< A binary, with expression (1) and next element
                    ///< in chain (2). Last element in chain is opNil
    opNil,          ///< This is a "nil list" terminal (e.g. no parameters)
    opTypedExp,     ///< Typed expression


    //
    // Deprecated operators
    //

    /// For frontend / parser use only
    opFlagCall,  ///< A flag call (Binary with string and params)
    opSuccessor, ///< Get the successor register of this parameter

    opMachFtr, ///< A Unary with Const(string) representing a machine specific feature
               ///< (register, instruction or whatever); the analysis better understand it
               ///< and transform it away)

    // Terminals (zero parameter special locations)
    // All machines are assumed to have these following registers:
    opPC, ///< program counter

    opFlags,  ///< This is the abstracted integer flags register terminal
    opFflags, ///< This is the abstracted floating point flags terminal

    // ---------------------- "The line" --------------------------//
    // All id's greater or equal to idMachSpec are assumed to be source machine
    // specific. If any of these are left by the time the back end is called,
    // then these need to have local variables assigned for them
    opZF,  ///< zero flag
    opCF,  ///< carry flag
    opNF,  ///< negative flag
    opOF,  ///< overflow flag
    opDF,  ///< Pentium Direction (=Down) flag
    opFZF, ///< floating point zero flag
    opFLF  ///< floating point less flag
};

/// Convert operator to string
BOOMERANG_API const char *operToString(OPER oper);
