/*==============================================================================
 * FILE:       index.h
 * OVERVIEW:   Declares the enum INDEX, which is used within class Exp to
 *              denote what the top level operator is
 *============================================================================*/
/*
 * $Revision$
 *
 * 05 Apr 02 - Mike: Created
 */

// The index (and integer representation) of predefined semantic items
// (expression types)
//
// Cant we typedef this? - trent 9/4/2002
enum INDEX
{
    // Operators
    WILD = -1,
    idPlus,                  // Binary addition
    idMinus,                 // Binary subtraction
    idMult,                  // Multiplication
    idDiv,                   // Integer division
    idFPlus,                 // Binary addition(single floats)
    idFMinus,                // Binary subtraction(single floats)
    idFMult,                 // Multiplication(single floats)
    idFDiv,                  // (single floats)
    idFPlusd,                // addition(double floats)
    idFMinusd,               // subtraction(double floats)
    idFMultd,                // Multiplication(double floats)
    idFDivd,                 // Integer division(double floats)
    idFPlusq,                // addition(quad floats)
    idFMinusq,               // subtraction(quad floats)
    idFMultq,                // Multiplication(quad floats)
    idFDivq,                 // division(quad floats)
    idFMultsd,               // Multiplication(single floats--> double floats)
    idFMultdq,               // Multiplication(single floats--> double floats)
    idSQRTs,                 // sqrt of a single
    idSQRTd,                 // sqrt of a double
    idSQRTq,                 // sqrt of a quad

    idMults,                 // Multiply signed
    idDivs,                  // Divide signed
    idMod,                   // Remainder of integer division
    idMods,                  // Remainder of signed integer division
    idNeg,                   // Unary minus

    idAnd,                   // Logical and
    idOr,                    // Logical or
    idEquals,                // Equality (logical)
    idNotEqual,              // Logical !=
    idLess,                  // Logical less than (signed)
    idGtr,                   // Logical greater than (signed)
    idLessEq,                // Logical <= (signed)
    idGtrEq,                 // Logical >= (signed)
    idLessUns,               // Logical less than (unsigned)
    idGtrUns,                // Logical greater than (unsigned)
    idLessEqUns,             // Logical <= (unsigned)
    idGtrEqUns,              // Logical >= (unsigned)

    idNot,                   // Bitwise inversion
    idLNot,                  // Logical not
    idSignExt,               // Sign extend
    idBitAnd,                // Bitwise and
    idBitOr,                 // Bitwise or
    idBitXor,                // Bitwise xor
    idShiftL,                // Left shift
    idShiftR,                // Right shift
    idShiftRA,               // Right shift arithmetic
    idRotateL,               // Rotate left
    idRotateR,               // Rotate right
    idRotateLC,              // Rotate left through carry
    idRotateRC,              // Rotate right through carry

    idTern,                  // Ternary (i.e. ? : )
    idAt,                    // Bit extraction (expr@first:last)

    idMemOf,                 // Represents m[]
    idRegOf,                 // Represents r[]
    idAddrOf,                // Represents a[]
    idVar,                   // Represents l[] (recovered locations)
    idParam,                 // Parameter param`'
    idExpand,                // Expandable expression
    idTemp,                  // Temp register name
    idSize,                  // Size override
    idCastIntStar,           // Cast to int*
    idPostVar,               // Post-instruction variable marker

    idTruncu,                // Integer truncate (unsigned)
    idTruncs,                // Integer truncate (signed)
    idZfill,                 // Integer zero fill
    idSgnEx,                 // Integer sign extend

    idFsize,                 // Floating point size conversion
    idItof,                  // Integer to floating point (and size) conversion
    idFtoi,                  // Floating point to integer (and size) conversion
    idFround,                // Floating point to nearest float conversion
    idForceInt,              // Forcibly change current type to int/flt,
    idForceFlt,              //  without changing any of the bits
    idFpush,                 // Floating point stack push
    idFpop,                  // Floating point stack pop

    idSin,                   // sine
    idCos,                   // cosine
    idTan,                   // tangent
    idArcTan,                // inverse tangent
    idLog2,                  // logarithm to base 2
    idLog10,                 // logarithm to base 10
    idLoge,                  // logarithm to base e
    idSqrt,                  // square root
    idExecute,               // Execute instruction at(addr)

    idIntConst,              // integer constant
    idFltConst,              // floating point constant
    idStrConst,              // string constant
    idCodeAddr,              // idIntConst for addresses in code segment

    idTmpNul,                // A temporary used for anulling following instr

    // All machines are assumed to have these following registers:
    idPC,                    // program counter
    // This is the abstract frame pointer register (CSR/PAL analysis). 
    idAFP,                   // abstract frame pointer
    // This is the abstract global pointer register (CSR/PAL analysis)
    idAGP,                   // abstract global pointer


    // Added for type analysis
    idHLCTI,		     // High level Control transfer instruction
    idDEFINE,		     // Define Type of use with lexer

    //---------------------- "The line" --------------------------//
    // All id's greater or equal to idMachSpec are assumed to be source machine
    // specific. If any of these are left by the time the back end is called,
    // then these need to have local variables assigned for them
    idZF,                    // zero flag
    idCF,                    // carry flag
    idNF,                    // negative flag
    idOF,                    // overflow flag
    idFZF,                   // floating point zero flag
    idFLF,                   // floating point less flag
    idFGF,                   // floating point greater flag
    idCTI,                   // Control transfer instruction (boolean)
    idNEXT,                  // Next PC pseudo-register

    // Highlevel expr types - trent
    
    idArrayOf,               // Binary takes a base and an index
    idFuncCall,              // Unary takes a list of params
    idMember,                // Access a member in a structure
    idMemberThru,            // Access a member through a structure pointer
    idPostInc,               // Post increment
    idPostDec,               // Post decrement
    
    // ALWAYS LAST!
    idNumOf                  // Special index: MUST BE LAST!
};
