// Strings for the OPER enum. Ugh! I wish C could print enums properly.
// Only needed for the dotty file (a debugging nicety) so don't be
// surprised if the strings become out of date.

char* operStrings[] = {
    "opPlus",                 // Binary addition
    "opMinus",                // Binary subtraction
    "opMult",                 // Multiplication
    "opDiv",                  // Integer division
    "opFPlus",                // Binary addition(single floats)
    "opFMinus",               // Binary subtraction(single floats)
    "opFMult",                // Multiplication(single floats)
    "opFDiv",                 // (single floats)
    "opFPlusd",               // addition(double floats)
    "opFMinusd",              // subtraction(double floats)
    "opFMultd",               // Multiplication(double floats)
    "opFDivd",                // Integer division(double floats)
    "opFPlusq",               // addition(quad floats)
    "opFMinusq",              // subtraction(quad floats)
    "opFMultq",               // Multiplication(quad floats)
    "opFDivq",                // division(quad floats)
    "opFMultsd",              // Multiplication(single floats--> double floats)
    "opFMultdq",              // Multiplication(single floats--> double floats)
    "opSQRTs",                // sqrt of a single
    "opSQRTd",                // sqrt of a double
    "opSQRTq",                // sqrt of a quad

    "opMults",                // Multiply signed
    "opDivs",                 // Divide signed
    "opMod",                  // Remainder of integer division
    "opMods",                 // Remainder of signed integer division
    "opNeg",                  // Unary minus

    "opAnd",                  // Logical and
    "opOr",                   // Logical or
    "opEquals",               // Equality (logical)
    "opNotEqual",             // Logical !=
    "opLess",                 // Logical less than (signed)
    "opGtr",                  // Logical greater than (signed)
    "opLessEq",               // Logical <= (signed)
    "opGtrEq",                // Logical >= (signed)
    "opLessUns",              // Logical less than (unsigned)
    "opGtrUns",               // Logical greater than (unsigned)
    "opLessEqUns",            // Logical <= (unsigned)
    "opGtrEqUns",             // Logical >= (unsigned)

    "opNot",                  // Bitwise inversion
    "opLNot",                 // Logical not
    "opSignExt",              // Sign extend
    "opBitAnd",               // Bitwise and
    "opBitOr",                // Bitwise or
    "opBitXor",               // Bitwise xor
    "opShiftL",               // Left shift
    "opShiftR",               // Right shift
    "opShiftRA",              // Right shift arithmetic
    "opRotateL",              // Rotate left
    "opRotateR",              // Rotate right
    "opRotateLC",             // Rotate left through carry
    "opRotateRC",             // Rotate right through carry
    "opTargetInst",           // Target specific instruction (Unary)

    "opAssign",               // Assignment
    "opTypedExp",             // Typed expression
	"opNamedExp",			  // Named  expression (binary, subExp1 = Const("name"), subExp2 = exp)
    "opGuard",                // Guarded expression (should be assignment)
    "opComma",                // Separate expressions in a list (e.g. params)
    "opFlagCall",             // A flag call (Binary with string and params)
    "opFlagDef",              // A flag function definition (class FlagDef)
    "opList",                 // A binary, with expression (1) and next element
    "opNameTable",            // A table of strings
    "opExpTable",             // A table of expressions
    "opOpTable",              // A table of operators
	"opSuccessor",			  // Get the successor of this register parameter

    "opTern",                 // Ternary (i.e. ? : )
    "opAt",                   // Bit extraction (expr@first:last)

    "opMemOf",                // Represents m[]
    "opRegOf",                // Represents r[]
    "opAddrOf",               // Represents a[]
    "opVar",                  // Represents l[] (recovered locations)
    "opPhi",				  // Represents phi(a1, a2, a3) .. ie SSA form merging
	"opSubscript",			  // Represents subscript(e, n) .. ie SSA renaming
    "opParam",                // Parameter param`'
	"opArg",				  // Used a temporary for arguments to calls
	"opLocal",				  // Represents a local, takes a string
    "opExpand",               // Expandable expression
    "opTemp",                 // Temp register name
    "opSize",                 // Size override
    "opCastIntStar",          // Cast to int*
    "opPostVar",              // Post-instruction variable marker (unary with
    "opMachFtr",              // A Unary with Const(string) representing a
                              // machine specific feature (register, instruction                              // or whatever; the analysis better understand it
                              // and transform it away)


    "opTruncu",               // Integer truncate (unsigned)
    "opTruncs",               // Integer truncate (signed)
    "opZfill",                // Integer zero fill
    "opSgnEx",                // Integer sign extend

    "opFsize",                // Floating point size conversion
    "opItof",                 // Integer to floating point (and size) conversion
    "opFtoi",                 // Floating point to integer (and size) conversion
    "opFround",               // Floating point to nearest float conversion
    "opForceInt",             // Forcibly change current type to int/flt,
    "opForceFlt",             //  without changing any of the bits
    "opFpush",                // Floating point stack push
    "opFpop",                 // Floating point stack pop

    "opSin",                  // sine
    "opCos",                  // cosine
    "opTan",                  // tangent
    "opArcTan",               // inverse tangent
    "opLog2",                 // logarithm to base 2
    "opLog10",                // logarithm to base 10
    "opLoge",                 // logarithm to base e
    "opSqrt",                 // square root
    "opExecute",              // Execute instruction at(addr)

    "opIntConst",             // integer constant
    "opFltConst",             // floating point constant
    "opStrConst",             // string constant
    "opAddrConst",            // address constant
    "opCodeAddr",             // opAddrConst for addresses in code segment


    "opPC",                   // program counter
    "opAFP",                  // abstract frame pointer
    "opAGP",                  // abstract global pointer
    "opNil",                  // Nil list
	"opFlags",				  // Flags

    "opAnull",                // Anull "variable"


    "opHLCTI",		        // High level Control transfer instruction
    "opDEFINE",		        // Define Type of use with lexer

    "opZF",                   // zero flag
    "opCF",                   // carry flag
    "opNF",                   // negative flag
    "opOF",                   // overflow flag
    "opFZF",                  // floating point zero flag
    "opFLF",                  // floating point less flag
    "opFGF",                  // floating point greater flag
    "opCTI",                  // Control transfer instruction (boolean)
    "opNEXT"                 // Next PC pseudo-register
};



