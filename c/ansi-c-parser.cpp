#include "ansi-c-parser.h"

/*  A Bison++ parser, made from ansi-c.y  */

/* with Bison++ version bison++ Version 1.21-8, adapted from GNU bison by coetmeur@icdc.fr
  */

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* HEADER SECTION */
#if defined(_WINDOWS) && defined(_MSC_VER)
#define __HAVE_NO_ALLOCA
#define __MSDOS_AND_ALIKE
#endif

#ifndef alloca
#if defined(__GNUC__)
#define alloca __builtin_alloca

#elif defined(__MSDOS_AND_ALIKE)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
#include <cstdio>
#include <list>
#include <string>

#include "exp.h"
#include "type.h"
#include "cfg.h"
#include "proc.h"
#include "signature.h"

class AnsiCScanner;
class SymbolMods;

#include "ansi-c-scanner.h"
/* section apres lecture def, avant lecture grammaire S2 */

/* prefix */
AnsiCParser::AnsiCParser(std::istream &in, bool trace) {
#if YY_AnsiCParser_DEBUG != 0
    yydebug = 0;
#endif
    theScanner = new AnsiCScanner(in, trace);
    if (trace)
        yydebug = 1;
    else
        yydebug = 0;
}

#define YYFINAL 154
#define YYFLAG -32768
#define YYNTBASE 91

#define YYTRANSLATE(x) ((unsigned)(x) <= 333 ? yytranslate[x] : 112)

static const char yytranslate[] = {
    0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  85, 84, 86, 82, 79, 83, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  80, 87, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  90, 2,  81, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  88, 2,  89, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78};

#if YY_AnsiCParser_DEBUG != 0
static const short yyprhs[] = {0,   0,   2,   5,   6,   8,   10,  12,  14,  16,  18,  20,  24,  26,  27,  31,
                               33,  35,  36,  40,  42,  46,  50,  54,  58,  60,  64,  65,  68,  77,  79,  83,
                               94,  101, 108, 111, 119, 124, 130, 137, 142, 146, 151, 154, 157, 158, 161, 165,
                               166, 170, 173, 178, 182, 185, 189, 193, 196, 198, 200, 202, 205, 208, 211, 214,
                               216, 218, 221, 225, 227, 229, 231, 234, 239, 243, 245, 248, 251};

static const short yyrhs[] = {
    92, 0,   93, 92,  0,   0,   101, 0,  102, 0,  105, 0,   104, 0,   16,  0,   17,  0,   18,  0,   11,  79,  95, 0,
    11, 0,   0,  97,  79,  96,  0,   97, 0,   62, 0,   0,   98,  80,  100, 0,   100, 0,   19,  11,  81,  0,   20, 98,
    81, 0,   98, 82,  98,  0,   98,  83, 98,  0,  11,  0,   21,  9,   84,  0,   0,   109, 99,  0,   111, 85,  86, 9,
    84, 85,  96, 84,  0,   66,  0,   47, 109, 87, 0,   47,  111, 85,  86,  9,   84,  85,  96,  84,  87,  0,   47, 109,
    85, 96,  84, 87,  0,   63,  9,   88, 110, 89, 87,  0,   103, 87,  0,   103, 23,  109, 85,  95,  84,  87,  0,  109,
    85, 96,  84, 0,   94,  109, 85,  96, 84,  0,  22,  107, 109, 85,  96,  84,  0,   15,  11,  9,   87,  0,   11, 109,
    87, 0,   11, 106, 103, 87,  0,   13, 106, 0,  14,  106, 0,   0,   98,  80,  0,   24,  11,  84,  0,   0,   90, 11,
    81, 0,   90, 81,  0,   108, 90,  11, 81,  0,  108, 90,  81,  0,   111, 9,   0,   111, 9,   108, 0,   109, 87, 110,
    0,  109, 87, 0,   52,  0,   53,  0,  54,  0,  57,  52,  0,   57,  53,  0,   57,  54,  0,   57,  55,  0,   57, 0,
    55, 0,   55, 55,  0,   57,  55,  55, 0,   58, 0,   59,  0,   62,  0,   111, 86,  0,   111, 90,  11,  81,  0,  111,
    90, 81,  0,  9,   0,   60,  111, 0,  63,  9,  0,   63,  88,  110, 89,  0};

#endif

#if YY_AnsiCParser_DEBUG != 0
static const short yyrline[] = {0,   168, 172, 174, 178, 180, 182, 184, 188, 191, 193, 197, 201, 205, 210, 214,
                                218, 220, 224, 228, 233, 236, 239, 242, 245, 250, 252, 256, 277, 291, 295, 297,
                                311, 325, 337, 341, 353, 369, 384, 403, 409, 420, 428, 432, 436, 440, 443, 446,
                                450, 453, 456, 459, 464, 469, 477, 481, 487, 489, 491, 493, 495, 497, 499, 501,
                                503, 505, 507, 509, 511, 513, 515, 517, 521, 525, 530, 532, 538};

static const char *const yytname[] = {
    "$",              "error",            "$illegal.",       "PREINCLUDE",     "PREDEFINE",      "PREIF",
    "PREIFDEF",       "PREENDIF",         "PRELINE",         "IDENTIFIER",     "STRING_LITERAL", "CONSTANT",
    "SIZEOF",         "NODECODE",         "INCOMPLETE",      "SYMBOLREF",      "CDECL",          "PASCAL",
    "THISCALL",       "REGOF",            "MEMOF",           "MAXBOUND",       "CUSTOM",         "PREFER",
    "WITHSTACK",      "PTR_OP",           "INC_OP",          "DEC_OP",         "LEFT_OP",        "RIGHT_OP",
    "LE_OP",          "GE_OP",            "EQ_OP",           "NE_OP",          "AND_OP",         "OR_OP",
    "MUL_ASSIGN",     "DIV_ASSIGN",       "MOD_ASSIGN",      "ADD_ASSIGN",     "SUB_ASSIGN",     "LEFT_ASSIGN",
    "RIGHT_ASSIGN",   "AND_ASSIGN",       "XOR_ASSIGN",      "OR_ASSIGN",      "TYPE_NAME",      "TYPEDEF",
    "EXTERN",         "STATIC",           "AUTO",            "REGISTER",       "CHAR",           "SHORT",
    "INT",            "LONG",             "SIGNED",          "UNSIGNED",       "FLOAT",          "DOUBLE",
    "CONST",          "VOLATILE",         "VOID",            "STRUCT",         "UNION",          "ENUM",
    "ELLIPSIS",       "CASE",             "DEFAULT",         "IF",             "ELSE",           "SWITCH",
    "WHILE",          "DO",               "FOR",             "GOTO",           "CONTINUE",       "BREAK",
    "RETURN",         "','",              "':'",             "']'",            "'+'",            "'-'",
    "')'",            "'('",              "'*'",             "';'",            "'{'",            "'}'",
    "'['",            "translation_unit", "decls",           "decl",           "convention",     "num_list",
    "param_list",     "param_exp",        "exp",             "optional_bound", "param",          "type_decl",
    "func_decl",      "signature",        "symbol_ref_decl", "symbol_decl",    "symbol_mods",    "custom_options",
    "array_modifier", "type_ident",       "type_ident_list", "type",           ""};
#endif

static const short yyr1[] = {0,   91,  92,  92,  93,  93,  93,  93,  94,  94,  94,  95,  95,  95,  96,  96,
                             96,  96,  97,  97,  98,  98,  98,  98,  98,  99,  99,  100, 100, 100, 101, 101,
                             101, 101, 102, 102, 103, 103, 103, 104, 105, 105, 106, 106, 106, 107, 107, 107,
                             108, 108, 108, 108, 109, 109, 110, 110, 111, 111, 111, 111, 111, 111, 111, 111,
                             111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111, 111};

static const short yyr2[] = {0, 1, 2, 0, 1, 1,  1, 1, 1, 1, 1, 3, 1, 0, 3, 1, 1, 0, 3, 1, 3, 3, 3, 3, 1, 3,
                             0, 2, 8, 1, 3, 10, 6, 6, 2, 7, 4, 5, 6, 4, 3, 4, 2, 2, 0, 2, 3, 0, 3, 2, 4, 3,
                             2, 3, 3, 2, 1, 1,  1, 2, 2, 2, 2, 1, 1, 2, 3, 1, 1, 1, 2, 4, 3, 1, 2, 2, 4};

static const short yydefact[] = {
    3,  73, 44, 0,  8,  9,  10, 47, 0,  56, 57, 58, 64, 63, 67, 68, 0,  69, 0,  1,  3,  0,  4,  5,  0, 7,
    6,  0,  0,  44, 44, 0,  0,  0,  0,  24, 0,  0,  0,  0,  0,  0,  0,  65, 59, 60, 61, 62, 74, 75, 0, 2,
    0,  0,  34, 17, 52, 70, 0,  42, 43, 75, 0,  40, 0,  0,  0,  0,  45, 0,  0,  0,  17, 30, 0,  66, 0, 0,
    0,  17, 0,  69, 29, 0,  15, 0,  19, 26, 0,  0,  53, 0,  72, 41, 39, 20, 21, 46, 22, 23, 17, 0,  0, 0,
    55, 76, 0,  13, 36, 17, 0,  0,  27, 0,  0,  49, 0,  71, 0,  0,  0,  0,  54, 37, 12, 0,  14, 18, 0, 0,
    48, 0,  51, 38, 32, 0,  33, 13, 0,  25, 0,  50, 17, 11, 35, 0,  0,  17, 0,  0,  31, 28, 0,  0,  0};

static const short yydefgoto[] = {152, 19, 20, 21, 125, 83, 84, 85, 112, 86, 22,
                                  23,  24, 25, 26, 32,  40, 90, 87, 78,  28};

static const short yypact[] = {
    120,    -32768, 132,    16,     -32768, -32768, -32768, 12,     159,    -32768, -32768, -32768, -48,
    -39,    -32768, -32768, 159,    -32768, 2,      -32768, 120,    159,    -32768, -32768, -17,    -32768,
    -32768, -51,    -4,     39,     39,     3,      147,    -44,    48,     -32768, 52,     9,      55,
    -43,    159,    -29,    -7,     -32768, -32768, -32768, -32768, 14,     -60,    -21,    159,    -32768,
    -13,    159,    -32768, 95,     -22,    -32768, 6,      -32768, -32768, -32768, -12,    -32768, -11,
    15,     -33,    -10,    -32768, 9,      9,      -1,     95,     -32768, 17,     -32768, 159,    20,
    10,     95,     23,     11,     -32768, 25,     21,     -38,    -32768, 89,     -5,     8,      22,
    30,     -32768, -32768, -32768, -32768, -32768, -32768, -23,    -23,    95,     29,     111,    36,
    159,    -32768, 44,     121,    -32768, 95,     64,     124,    -32768, 54,     62,     -32768, 13,
    -32768, 60,     72,     67,     73,     -32768, -32768, 83,     82,     -32768, -32768, 86,     162,
    -32768, 100,    -32768, -32768, -32768, 91,     -32768, 121,    101,    -32768, 109,    -32768, 95,
    -32768, -32768, 112,    114,    95,     116,    131,    -32768, -32768, 196,    208,    -32768};

static const short yypgoto[] = {-32768, 200, -32768, -32768, 87, -54,    -32768, 28, -32768, 113, -32768,
                                -32768, 193, -32768, -32768, 32, -32768, -32768, 1,  -66,    -8};

#define YYLAST 225

static const short yytable[] = {
    42,  27,  56,  33,  56,  56,  53,  43,  48,  41,  103, 49,  61,  44,  45,  46,  47,  91,  101, 114, 35,  27,  52,
    35,  131, 106, 57,  34,  36,  37,  58,  36,  37,  27,  55,  39,  38,  68,  122, 69,  70,  71,  110, 63,  69,  70,
    118, 88,  96,  69,  70,  77,  29,  30,  80,  126, 72,  64,  73,  69,  70,  59,  60,  65,  88,  66,  67,  76,  89,
    75,  54,  88,  79,  1,   97,  93,  94,  77,  74,  57,  113, 57,  57,  58,  100, 58,  58,  92,  146, 115, 50,  50,
    88,  149, 132, -16, 95,  98,  99,  105, 109, 88,  88,  102, 1,   77,  35,  104, 107, 108, 111, 117, 116, 119, 36,
    37,  9,   10,  11,  12,  120, 13,  14,  15,  16,  121, 17,  31,  123, 1,   82,  2,   124, 128, 88,  3,   4,   5,
    6,   88,  129, 1,   7,   130, 133, 29,  30,  9,   10,  11,  12,  135, 13,  14,  15,  16,  1,   81,  31,  134, 136,
    82,  137, 4,   5,   6,   138, 8,   1,   7,   139, 140, 9,   10,  11,  12,  142, 13,  14,  15,  16,  141, 17,  18,
    9,   10,  11,  12,  144, 13,  14,  15,  16,  145, 17,  31,  153, 147, 148, 9,   10,  11,  12,  150, 13,  14,  15,
    16,  154, 17,  31,  9,   10,  11,  12,  151, 13,  14,  15,  16,  51,  17,  31,  127, 143, 62};

static const short yycheck[] = {
    8,  0,   9,  2,   9,   9,  23, 55, 16, 8,   76,  9,  9,   52, 53,  54,  55,  11, 72, 11, 11,  20, 21, 11,  11,  79,
    86, 11,  19, 20,  90,  19, 20, 32, 85, 7,   24,  80, 104, 82, 83,  40,  80,  87, 82, 83, 100, 55, 81, 82,  83,  50,
    13, 14,  53, 109, 85,  9,  87, 82, 83, 29,  30,  11, 72,  37, 11,  88,  90,  55, 87, 79, 85,  9,  84, 87,  87,  76,
    85, 86,  85, 86,  86,  90, 85, 90, 90, 81,  142, 81, 88,  88, 100, 147, 81,  84, 81, 69, 70,  89, 79, 109, 110, 86,
    9,  104, 11, 87,  85,  84, 21, 81, 90, 84,  19,  20, 52,  53, 54,  55,  9,   57, 58, 59, 60,  89, 62, 63,  84,  9,
    66, 11,  11, 9,   142, 15, 16, 17, 18, 147, 86,  9,  22,  81, 84,  13,  14,  52, 53, 54, 55,  84, 57, 58,  59,  60,
    9,  62,  63, 87,  87,  66, 79, 16, 17, 18,  84,  47, 9,   22, 84,  9,   52,  53, 54, 55, 85,  57, 58, 59,  60,  81,
    62, 63,  52, 53,  54,  55, 87, 57, 58, 59,  60,  84, 62,  63, 0,   85,  84,  52, 53, 54, 55,  87, 57, 58,  59,  60,
    0,  62,  63, 52,  53,  54, 55, 84, 57, 58,  59,  60, 20,  62, 63,  110, 137, 32};

/* fattrs + tables */

/* parser code folow  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */

#define YYGOTO(lb) goto lb
#define YYLABEL(lb) lb:

/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr, char *ref) {
    if (ptr != ref)
        free(ptr);
    return 0;
}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr, ref) __alloca_free_ptr((char *)ptr, (char *)ref)

#define __ALLOCA_return(num) return (__ALLOCA_free(yyss, yyssa) + __ALLOCA_free(yyvs, yyvsa) + (num))
#else
#define __ALLOCA_return(num) return (num)
#define __ALLOCA_alloca(size) alloca(size)
#define __ALLOCA_free(ptr, ref)
#endif

/* ENDALLOCA SIMULATION */

#define YYEMPTY -2
#define YYEOF 0
#define YYACCEPT __ALLOCA_return(0)
#define YYABORT __ALLOCA_return(1)
#define YYTERROR 1
#define YYLEX yylex()

/*  YYINITDEPTH indicates the initial size of the parser's stacks       */

#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#if __GNUC__ > 1 /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM, TO, COUNT) __builtin_memcpy(TO, FROM, COUNT)
#else /* not GNU C or C++ */
#define __yy_bcopy(FROM, TO, COUNT) memcpy(TO, FROM, COUNT)
#endif

int AnsiCParser::yyparse(platform plat, callconv cc) {
    int yystate;
    int yyn;
    short *yyssp;
    yy_AnsiCParser_stype *yyvsp;
    int yyerrstatus; /*  number of tokens to shift before error messages enabled */
    int yychar1 = 0; /*  lookahead token as an internal (translated) token number */

    short yyssa[YYINITDEPTH];                /*  the state stack                     */
    yy_AnsiCParser_stype yyvsa[YYINITDEPTH]; /*  the semantic value stack            */

    short *yyss = yyssa;                /*  refer to the stacks thru separate pointers */
    yy_AnsiCParser_stype *yyvs = yyvsa; /*  to allow yyoverflow to reallocate them elsewhere */
    int yystacksize = YYINITDEPTH;
    yy_AnsiCParser_stype yyval; /*  the variable used to return         */
    /*  semantic values from the action     */
    /*  routines                            */

    int yylen;
/* start loop, in which YYGOTO may be used. */

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug)
        fprintf(stderr, "Starting parse\n");
#endif
    yystate = 0;
    yyerrstatus = 0;
    yynerrs = 0;
    yychar = YYEMPTY; /* Cause a token to be read.  */

    /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

    yyssp = yyss - 1;
    yyvsp = yyvs;

    /* Push a new state, which is found in  yystate  .  */
    /* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
    YYLABEL(yynewstate)

    *++yyssp = yystate;

    if (yyssp >= yyss + yystacksize - 1) {
        /* Give user a chance to reallocate the stack */
        /* Use copies of these so that the &'s don't force the real ones into memory. */
        yy_AnsiCParser_stype *yyvs1 = yyvs;
        short *yyss1 = yyss;

        /* Get the current used size of the three stacks, in elements.  */
        int size = yyssp - yyss + 1;

        /* Extend the stack our own way.  */
        if (yystacksize >= YYMAXDEPTH) {
            yyerror("parser stack overflow");
            __ALLOCA_return(2);
        }
        yystacksize *= 2;
        if (yystacksize > YYMAXDEPTH)
            yystacksize = YYMAXDEPTH;
        yyss = (short *)__ALLOCA_alloca(yystacksize * sizeof(*yyssp));
        __yy_bcopy((char *)yyss1, (char *)yyss, size * sizeof(*yyssp));
        __ALLOCA_free(yyss1, yyssa);
        yyvs = new (__ALLOCA_alloca(yystacksize * sizeof(*yyvsp))) yy_AnsiCParser_stype[yystacksize];
        for(int i=0; i<size; ++i) {
            yyvs[i] = yyvs1[i];
        }
        __ALLOCA_free(yyvs1, yyvsa);

        yyssp = yyss + size - 1;
        yyvsp = yyvs + size - 1;
        if (yydebug)
            fprintf(stderr, "Stack size increased to %d\n", yystacksize);

        if (yyssp >= yyss + yystacksize - 1)
            YYABORT;
    }

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug)
        fprintf(stderr, "Entering state %d\n", yystate);
#endif

    YYGOTO(yybackup);
    YYLABEL(yybackup)

    /* Do appropriate processing given the current state.  */
    /* Read a lookahead token if we need one and don't already have one.  */
    /* YYLABEL(yyresume) */

    /* First try to decide what to do without reference to lookahead token.  */

    yyn = yypact[yystate];
    if (yyn == YYFLAG)
        YYGOTO(yydefault);

    /* Not known => get a lookahead token if don't already have one.  */

    /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

    if (yychar == YYEMPTY) {
#if YY_AnsiCParser_DEBUG != 0
        if (yydebug)
            fprintf(stderr, "Reading a token: ");
#endif
        yychar = YYLEX;
    }

    /* Convert token to internal form (in yychar1) for indexing tables with */

    if (yychar <= 0) /* This means end of input. */ {
        yychar1 = 0;
        yychar = YYEOF; /* Don't call YYLEX any more */

#if YY_AnsiCParser_DEBUG != 0
        if (yydebug)
            fprintf(stderr, "Now at end of input.\n");
#endif
    } else {
        yychar1 = YYTRANSLATE(yychar);

#if YY_AnsiCParser_DEBUG != 0
        if (yydebug) {
            fprintf(stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
            /* Give the individual parser a way to print the precise meaning
             of a token, for further debugging info.  */
            fprintf(stderr, ")\n");
        }
#endif
    }

    yyn += yychar1;
    if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
        YYGOTO(yydefault);

    yyn = yytable[yyn];

    /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

    if (yyn < 0) {
        if (yyn == YYFLAG)
            YYGOTO(yyerrlab);
        yyn = -yyn;
        YYGOTO(yyreduce);
    } else if (yyn == 0)
        YYGOTO(yyerrlab);

    if (yyn == YYFINAL)
        YYACCEPT;

/* Shift the lookahead token.  */

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug)
        fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

    /* Discard the token being shifted unless it is eof.  */
    if (yychar != YYEOF)
        yychar = YYEMPTY;

    *++yyvsp = yylval;
    /* count tokens shifted since error; after three, turn off error status.  */
    if (yyerrstatus)
        yyerrstatus--;

    yystate = yyn;
    YYGOTO(yynewstate);

    /* Do the default action for the current state.  */
    YYLABEL(yydefault)

    yyn = yydefact[yystate];
    if (yyn == 0)
        YYGOTO(yyerrlab);

    /* Do a reduction.  yyn is the number of a rule to reduce with.  */
    YYLABEL(yyreduce)
    yylen = yyr2[yyn];
    if (yylen > 0)
        yyval = yyvsp[1 - yylen]; /* implement default value of the action */

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug) {
        int i;

        fprintf(stderr, "Reducing via rule %d (line %d), ", yyn, yyrline[yyn]);

        /* Print the symbols being reduced, and their result.  */
        for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
            fprintf(stderr, "%s ", yytname[yyrhs[i]]);
        fprintf(stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

    switch (yyn) {

    case 1: {
        break;
    }
    case 2: {
        break;
    }
    case 3: {
        break;
    }
    case 4: {
        break;
    }
    case 5: {
        break;
    }
    case 6: {
        break;
    }
    case 7: {
        break;
    }
    case 8: {
        yyval.cc = CONV_C;
        break;
    }
    case 9: {
        yyval.cc = CONV_PASCAL;
        break;
    }
    case 10: {
        yyval.cc = CONV_THISCALL;
        break;
    }
    case 11: {
        yyval.num_list = yyvsp[0].num_list;
        yyval.num_list->push_front(yyvsp[-2].ival);
        break;
    }
    case 12: {
        yyval.num_list = new std::list<int>();
        yyval.num_list->push_back(yyvsp[0].ival);
        break;
    }
    case 13: {
        yyval.num_list = new std::list<int>();
        break;
    }
    case 14: {
        yyval.param_list = yyvsp[0].param_list;
        yyval.param_list->push_front(yyvsp[-2].param);
        break;
    }
    case 15: {
        yyval.param_list = new std::list<Parameter *>();
        yyval.param_list->push_back(yyvsp[0].param);

        break;
    }
    case 16: {
        yyval.param_list = new std::list<Parameter *>();
        break;
    }
    case 17: {
        yyval.param_list = new std::list<Parameter *>();
        break;
    }
    case 18: {
        yyval.param = yyvsp[0].param;
        yyval.param->setExp(yyvsp[-2].exp);

        break;
    }
    case 19: {
        yyval.param = yyvsp[0].param;

        break;
    }
    case 20: {
        yyval.exp = Location::regOf(yyvsp[-1].ival);

        break;
    }
    case 21: {
        yyval.exp = Location::memOf(yyvsp[-1].exp);

        break;
    }
    case 22: {
        yyval.exp = Binary::get(opPlus, yyvsp[-2].exp, yyvsp[0].exp);

        break;
    }
    case 23: {
        yyval.exp = Binary::get(opMinus, yyvsp[-2].exp, yyvsp[0].exp);

        break;
    }
    case 24: {
        yyval.exp = new Const(yyvsp[0].ival);

        break;
    }
    case 25: {
        yyval.bound = new Bound(0, yyvsp[-1].str);
        break;
    }
    case 26: {
        break;
    }
    case 27: {
        if (yyvsp[-1].type_ident->ty->isArray() ||
            (yyvsp[-1].type_ident->ty->isNamed() && std::static_pointer_cast<NamedType>(yyvsp[-1].type_ident->ty)->resolvesTo() &&
             std::static_pointer_cast<NamedType>(yyvsp[-1].type_ident->ty)->resolvesTo()->isArray())) {
            /* C has complex semantics for passing arrays.. seeing as
                     * we're supposedly parsing C, then we should deal with this.
                     * When you pass an array in C it is understood that you are
                     * passing that array "by reference".  As all parameters in
                     * our internal representation are passed "by value", we alter
                     * the type here to be a pointer to an array.
                     */
            yyvsp[-1].type_ident->ty = PointerType::get(yyvsp[-1].type_ident->ty);
        }
        yyval.param = new Parameter(yyvsp[-1].type_ident->ty, yyvsp[-1].type_ident->nam);
        if (yyvsp[0].bound) {
            switch (yyvsp[0].bound->kind) {
            case 0:
                yyval.param->setBoundMax(yyvsp[0].bound->nam);
            }
        }

        break;
    }
    case 28: {
        Signature *sig = Signature::instantiate(plat, cc, nullptr);
        sig->addReturn(yyvsp[-7].type);
        for (auto &elem : *yyvsp[-1].param_list)
            if (elem->name() != "...")
                sig->addParameter(elem);
            else {
                sig->addEllipsis();
                delete elem;
            }
        delete yyvsp[-1].param_list;
        yyval.param = new Parameter(PointerType::get(FuncType::get(sig)), yyvsp[-4].str);

        break;
    }
    case 29: {
        yyval.param = new Parameter(std::make_shared<VoidType>(), "...");
        break;
    }
    case 30: {
        Type::addNamedType(yyvsp[-1].type_ident->nam, yyvsp[-1].type_ident->ty);
        break;
    }
    case 31: {
        Signature *sig = Signature::instantiate(plat, cc, nullptr);
        sig->addReturn(yyvsp[-8].type);
        for (auto &elem : *yyvsp[-2].param_list)
            if (elem->name() != "...")
                sig->addParameter(elem);
            else {
                sig->addEllipsis();
                delete elem;
            }
        delete yyvsp[-2].param_list;
        Type::addNamedType(yyvsp[-5].str, PointerType::get(FuncType::get(sig)));

        break;
    }
    case 32: {
        Signature *sig = Signature::instantiate(plat, cc, yyvsp[-4].type_ident->nam);
        sig->addReturn(yyvsp[-4].type_ident->ty);
        for (auto &elem : *yyvsp[-2].param_list)
            if (elem->name() != "...")
                sig->addParameter(elem);
            else {
                sig->addEllipsis();
                delete elem;
            }
        delete yyvsp[-2].param_list;
        Type::addNamedType(yyvsp[-4].type_ident->nam, FuncType::get(sig));

        break;
    }
    case 33: {
        auto t = CompoundType::get();
        for (auto &elem : *yyvsp[-2].type_ident_list) {
            t->addType(elem->ty, elem->nam);
        }
        Type::addNamedType(QString("struct %1").arg(yyvsp[-4].str), t);
        break;
    }
    case 34: {
        signatures.push_back(yyvsp[-1].sig);

        break;
    }
    case 35: {
        yyvsp[-6].sig->setPreferedReturn(yyvsp[-4].type_ident->ty);
        yyvsp[-6].sig->setPreferedName(yyvsp[-4].type_ident->nam);
        for (std::list<int>::iterator it = yyvsp[-2].num_list->begin(); it != yyvsp[-2].num_list->end(); it++)
            yyvsp[-6].sig->addPreferedParameter(*it - 1);
        delete yyvsp[-2].num_list;
        signatures.push_back(yyvsp[-6].sig);

        break;
    }
    case 36: {
        /* Use the passed calling convention (cc) */
        Signature *sig = Signature::instantiate(plat, cc, yyvsp[-3].type_ident->nam);
        sig->addReturn(yyvsp[-3].type_ident->ty);
        for (auto &elem : *yyvsp[-1].param_list)
            if (elem->name() != "...")
                sig->addParameter(elem);
            else {
                sig->addEllipsis();
                delete elem;
            }
        delete yyvsp[-1].param_list;
        yyval.sig = sig;

        break;
    }
    case 37: {
        Signature *sig = Signature::instantiate(plat, yyvsp[-4].cc, yyvsp[-3].type_ident->nam);
        sig->addReturn(yyvsp[-3].type_ident->ty);
        for (auto &elem : *yyvsp[-1].param_list)
            if (elem->name() != "...")
                sig->addParameter(elem);
            else {
                sig->addEllipsis();
                delete elem;
            }
        delete yyvsp[-1].param_list;
        yyval.sig = sig;

        break;
    }
    case 38: {
        CustomSignature *sig = new CustomSignature(yyvsp[-3].type_ident->nam);
        if (yyvsp[-4].custom_options->exp)
            sig->addReturn(yyvsp[-3].type_ident->ty, yyvsp[-4].custom_options->exp);
        if (yyvsp[-4].custom_options->sp)
            sig->setSP(yyvsp[-4].custom_options->sp);
        for (Parameter *it : *yyvsp[-1].param_list)
            if (it->name() != "...") {
                sig->addParameter(it);
            } else {
                sig->addEllipsis();
                delete it;
            }
        delete yyvsp[-1].param_list;
        yyval.sig = sig;

        break;
    }
    case 39: {
        SymbolRef *ref = new SymbolRef(ADDRESS::g(yyvsp[-2].ival), yyvsp[-1].str);
        refs.push_back(ref);

        break;
    }
    case 40: {
        Symbol *sym = new Symbol(ADDRESS::g(yyvsp[-2].ival));
        sym->nam = yyvsp[-1].type_ident->nam;
        sym->ty = yyvsp[-1].type_ident->ty;
        symbols.push_back(sym);

        break;
    }
    case 41: {
        Symbol *sym = new Symbol(ADDRESS::g(yyvsp[-3].ival));
        sym->sig = yyvsp[-1].sig;
        sym->mods = yyvsp[-2].mods;
        symbols.push_back(sym);
        break;
    }
    case 42: {
        yyval.mods = yyvsp[0].mods;
        yyval.mods->noDecode = true;

        break;
    }
    case 43: {
        yyval.mods = yyvsp[0].mods;
        yyval.mods->incomplete = true;

        break;
    }
    case 44: {
        yyval.mods = new SymbolMods();
        break;
    }
    case 45: {
        yyval.custom_options = new CustomOptions();
        yyval.custom_options->exp = yyvsp[-1].exp;

        break;
    }
    case 46: {
        yyval.custom_options = new CustomOptions();
        yyval.custom_options->sp = yyvsp[-1].ival;

        break;
    }
    case 47: {
        yyval.custom_options = new CustomOptions();
        break;
    }
    case 48: {
        yyval.type = ArrayType::get(nullptr, yyvsp[-1].ival);

        break;
    }
    case 49: {
        yyval.type = ArrayType::get(nullptr);

        break;
    }
    case 50: {
        yyval.type = ArrayType::get(yyvsp[-3].type, yyvsp[-1].ival);

        break;
    }
    case 51: {
        yyval.type = ArrayType::get(yyvsp[-2].type);

        break;
    }
    case 52: {
        yyval.type_ident = new TypeIdent();
        yyval.type_ident->ty = yyvsp[-1].type;
        yyval.type_ident->nam = yyvsp[0].str;

        break;
    }
    case 53: {
        yyval.type_ident = new TypeIdent();
        std::static_pointer_cast<ArrayType>(yyvsp[0].type)->fixBaseType(yyvsp[-2].type);
        yyval.type_ident->ty = yyvsp[0].type;
        yyval.type_ident->nam = yyvsp[-1].str;

        break;
    }
    case 54: {
        yyval.type_ident_list = yyvsp[0].type_ident_list;
        yyval.type_ident_list->push_front(yyvsp[-2].type_ident);

        break;
    }
    case 55: {
        yyval.type_ident_list = new std::list<TypeIdent *>();
        yyval.type_ident_list->push_back(yyvsp[-1].type_ident);

        break;
    }
    case 56: {
        yyval.type = CharType::get();
        break;
    }
    case 57: {
        yyval.type = IntegerType::get(16, 1);
        break;
    }
    case 58: {
        yyval.type = IntegerType::get(32, 1);
        break;
    }
    case 59: {
        yyval.type = IntegerType::get(8, 0);
        break;
    }
    case 60: {
        yyval.type = IntegerType::get(16, 0);
        break;
    }
    case 61: {
        yyval.type = IntegerType::get(32, 0);
        break;
    }
    case 62: {
        yyval.type = IntegerType::get(32, 0);
        break;
    }
    case 63: {
        yyval.type = IntegerType::get(32, 0);
        break;
    }
    case 64: {
        yyval.type = IntegerType::get(32, 1);
        break;
    }
    case 65: {
        yyval.type = IntegerType::get(64, 1);
        break;
    }
    case 66: {
        yyval.type = IntegerType::get(64, 0);
        break;
    }
    case 67: {
        yyval.type = FloatType::get(32);
        break;
    }
    case 68: {
        yyval.type = FloatType::get(64);
        break;
    }
    case 69: {
        yyval.type = VoidType::get();
        break;
    }
    case 70: {
        yyval.type = PointerType::get(yyvsp[-1].type);
        break;
    }
    case 71: { // This isn't C, but it makes defining pointers to arrays easier
        yyval.type = ArrayType::get(yyvsp[-3].type, yyvsp[-1].ival);

        break;
    }
    case 72: { // This isn't C, but it makes defining pointers to arrays easier
        yyval.type = ArrayType::get(yyvsp[-2].type);

        break;
    }
    case 73: { //$$ = Type::getNamedType($1);
        // if ($$ == nullptr)
        yyval.type = NamedType::get(yyvsp[0].str);

        break;
    }
    case 74: {
        yyval.type = yyvsp[0].type;
        break;
    }
    case 75: {
        yyval.type = NamedType::get(QString("struct %1").arg(yyvsp[0].str));
        break;
    }
    case 76: {
        auto t = CompoundType::get();
        for (auto &elem : *yyvsp[-1].type_ident_list) {
            t->addType((elem)->ty, elem->nam);
        }
        yyval.type = t;

        break;
    }
    }

    /* the action file gets copied in in place of this dollarsign  */
    yyvsp -= yylen;
    yyssp -= yylen;

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug) {
        short *ssp1 = yyss - 1;
        fprintf(stderr, "state stack now");
        while (ssp1 != yyssp)
            fprintf(stderr, " %d", *++ssp1);
        fprintf(stderr, "\n");
    }
#endif

    *++yyvsp = yyval;

    /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

    yyn = yyr1[yyn];

    yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
    if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
        yystate = yytable[yystate];
    else
        yystate = yydefgoto[yyn - YYNTBASE];

    YYGOTO(yynewstate);

    YYLABEL(yyerrlab) /* here on detecting error */

    if (!yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
        ++yynerrs;

#ifdef YY_AnsiCParser_ERROR_VERBOSE
        yyn = yypact[yystate];

        if (yyn > YYFLAG && yyn < YYLAST) {
            int size = 0;
            char *msg;
            int x, count;

            count = 0;
            /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
            for (x = (yyn < 0 ? -yyn : 0); x < (sizeof(yytname) / sizeof(char *)); x++)
                if (yycheck[x + yyn] == x)
                    size += strlen(yytname[x]) + 15, count++;
            msg = (char *)malloc(size + 15);
            if (msg != 0) {
                strcpy(msg, "parse error");

                if (count < 5) {
                    count = 0;
                    for (x = (yyn < 0 ? -yyn : 0); x < (sizeof(yytname) / sizeof(char *)); x++)
                        if (yycheck[x + yyn] == x) {
                            strcat(msg, count == 0 ? ", expecting `" : " or `");
                            strcat(msg, yytname[x]);
                            strcat(msg, "'");
                            count++;
                        }
                }
                yyerror(msg);
                free(msg);
            } else
                yyerror("parse error; also virtual memory exceeded");
        } else
#endif /* YY_AnsiCParser_ERROR_VERBOSE */
            yyerror("parse error");
    }

    YYGOTO(yyerrlab1);
    YYLABEL(yyerrlab1) /* here on error raised explicitly by an action */

    if (yyerrstatus == 3) {
        /* if just tried and failed to reuse lookahead token after an error, discard it.  */

        /* return failure if at end of input */
        if (yychar == YYEOF)
            YYABORT;

#if YY_AnsiCParser_DEBUG != 0
        if (yydebug)
            fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

        yychar = YYEMPTY;
    }

    /* Else will try to reuse lookahead token
     after shifting the error token.  */

    yyerrstatus = 3; /* Each real token shifted decrements this */

    YYGOTO(yyerrhandle);

    YYLABEL(yyerrdefault) /* current state does not do anything special for the error token. */

#if 0
            /* This is wrong; only states that explicitly want error tokens
                     should shift them.  */
            yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
    if (yyn) YYGOTO(yydefault);
#endif

    YYLABEL(yyerrpop) /* pop the current state because it cannot handle the error token */

    if (yyssp == yyss)
        YYABORT;
    yyvsp--;
    yystate = *--yyssp;

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug) {
        short *ssp1 = yyss - 1;
        fprintf(stderr, "Error: state stack now");
        while (ssp1 != yyssp)
            fprintf(stderr, " %d", *++ssp1);
        fprintf(stderr, "\n");
    }
#endif

    YYLABEL(yyerrhandle)

    yyn = yypact[yystate];
    if (yyn == YYFLAG)
        YYGOTO(yyerrdefault);

    yyn += YYTERROR;
    if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
        YYGOTO(yyerrdefault);

    yyn = yytable[yyn];
    if (yyn < 0) {
        if (yyn == YYFLAG)
            YYGOTO(yyerrpop);
        yyn = -yyn;
        YYGOTO(yyreduce);
    } else if (yyn == 0)
        YYGOTO(yyerrpop);

    if (yyn == YYFINAL)
        YYACCEPT;

#if YY_AnsiCParser_DEBUG != 0
    if (yydebug)
        fprintf(stderr, "Shifting error token, ");
#endif

    *++yyvsp = yylval;
    yystate = yyn;
    YYGOTO(yynewstate);
    /* end loop, in which YYGOTO may be used. */
}

#include <cstdio>

int AnsiCParser::yylex() {
    int token = theScanner->yylex(yylval);
    return token;
}

void AnsiCParser::yyerror(const char *s) {
    fflush(stdout);
    printf("\n%s", theScanner->lineBuf);
    printf("\n%*s\n%*s on line %i\n", theScanner->column, "^", theScanner->column, s, theScanner->theLine);
}

AnsiCParser::~AnsiCParser() {
    delete theScanner;
    // Suppress warnings from gcc about lack of virtual destructor
}
