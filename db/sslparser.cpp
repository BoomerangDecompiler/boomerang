#include <QDebug>
#include "sslparser.h"
/*  A Bison++ parser, made from sslparser.y  */
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

#elif(!defined(__STDC__) && defined(sparc)) || defined(__sparc__) || defined(__sparc) || defined(__sgi)
#include <alloca.h>

#elif defined(__MSDOS_AND_ALIKE)
#include <malloc.h>
#ifndef __TURBOC__
/* MS C runtime lib */
#define alloca _alloca
#endif

#elif defined(_AIX)
#include <malloc.h>
#pragma alloca

#elif defined(__hpux)
#ifdef __cplusplus
extern "C" { void *alloca(unsigned int); };
#else  /* not __cplusplus */
void *alloca();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
#include <cstdio>
#define YYBISON 1

#include "config.h"
#ifdef HAVE_LIBGC
#include "gc.h"
#else
#define NO_GARBAGE_COLLECTOR
#endif
#include <cassert>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "types.h"
#include "rtl.h"
#include "table.h"
#include "insnameelem.h"
#include "util.h" // E.g. str()
#include "statement.h"
#include "exp.h"

#include "sslscanner.h"
#include "operator.h"
#include "boomerang.h"

OPER strToTerm(const QString &s);                       // Convert string to a Terminal (if possible)
Exp *listExpToExp(std::list<Exp *> *le);       // Convert a STL list of Exp* to opList
Exp *listStrToExp(std::list<QString> *ls); // Convert a STL list of strings to opList

/*apres const  */
SSLParser::SSLParser(const QString &sslFile, bool trace) : sslFile(sslFile), bFloat(false) {
#if YY_SSLParser_DEBUG != 0
    YY_SSLParser_DEBUG_FLAG = 0;
#endif
    std::fstream *fin = new std::fstream(sslFile.toStdString(), std::ios::in);
    theScanner = nullptr;
    if (!*fin) {
        LOG_STREAM() << "can't open `" << sslFile << "' for reading\n";
        return;
    }
    theScanner = new SSLScanner(*fin, trace);
    if (trace)
        yydebug = 1;
}

#define YYFINAL 300
#define YYFLAG -32768
#define YYNTBASE 68

#define YYTRANSLATE(x) ((unsigned)(x) <= 309 ? yytranslate[x] : 115)

static const char yytranslate[] = {
    0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  62, 2,  64, 2,  2,  63, 66, 61, 2,  2,  56, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  55, 2,  2,  2,  67, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  59, 2,  60, 2,  65, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  57, 2,  58, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  1,  2,  3,  4,  5,
    6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
    35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54};

#if YY_SSLParser_DEBUG != 0
static const short yyprhs[] = {0,   0,   2,   4,   6,   10,  13,  15,  18,  20,  22,  24,  26,  28,  30,  33,  37,  39,
                               45,  51,  55,  56,  57,  61,  62,  66,  70,  72,  76,  83,  94,  109, 120, 129, 133, 135,
                               142, 146, 152, 156, 158, 160, 162, 165, 167, 171, 176, 178, 182, 184, 188, 192, 195, 197,
                               199, 201, 203, 207, 213, 217, 221, 227, 231, 232, 237, 239, 242, 244, 246, 249, 253, 257,
                               261, 266, 271, 275, 278, 280, 282, 286, 290, 293, 295, 299, 301, 305, 307, 308, 310, 314,
                               316, 317, 324, 329, 331, 333, 336, 338, 340, 344, 346, 354, 358, 366, 370, 374, 376, 378,
                               382, 386, 390, 394, 397, 400, 403, 406, 409, 413, 417, 421, 425, 429, 435, 437, 439, 443,
                               445, 449, 451, 459, 461, 464, 468, 472, 475, 477, 479, 481, 484, 488, 490};

static const short yyrhs[] = {
    104, 0,   106, 0,   69,  0,   69,  70,  55, 0,   70,  55,  0,   93,  0,   48,  98,  0,   81,  0,   82,  0,   109,
    0,   112, 0,   74,  0,   80,  0,   47,  71, 0,   71,  56,  72,  0,   72,  0,   102, 42,  57,  101, 58,  0,   102,
    101, 73,  111, 106, 0,   59,  101, 60,  0,  0,   0,   44,  75,  77,  0,   0,   45,  76,  77,  0,   77,  56,  78,
    0,   78,  0,   9,   28,  52,  0,   9,   59, 52,  60,  28,  52,  0,   9,   59,  52,  60,  28,  52,  27,  9,   36,
    9,   0,   9,   59,  52,  60,  28,  52,  17, 9,   39,  59,  52,  36,  52,  60,  0,   59,  79,  60,  59,  52,  60,
    28,  52,  36,  52,  0,   59,  79,  60,  59, 52,  60,  28,  52,  0,   79,  56,  9,   0,   9,   0,   24,  101, 61,
    57,  98,  58,  0,   7,   42,  52,  0,   7,  42,  52,  5,   52,  0,   7,   42,  83,  0,   84,  0,   89,  0,   91,
    0,   84,  86,  0,   86,  0,   85,  56,  84, 0,   85,  56,  62,  62,  0,   84,  0,   57,  85,  58,  0,   87,  0,
    63,  7,   63,  0,   62,  7,   62,  0,   64, 7,   0,   7,   0,   4,   0,   5,   0,   13,  0,   57,  90,  58,  0,
    90,  56,  62,  88,  62,  0,   62,  88,  62, 0,   57,  92,  58,  0,   92,  56,  62,  106, 62,  0,   62,  106, 62,
    0,   0,   95,  94,  101, 98,  0,   96,  0,  95,  12,  0,   7,   0,   97,  0,   96,  97,  0,   63,  7,   63,  0,
    25,  52,  60,  0,   25,  7,   60,  0,   64, 25,  52,  60,  0,   64,  25,  7,   60,  0,   62,  7,   62,  0,   98,
    99,  0,   99,  0,   104, 0,   24,  103, 61, 0,   50,  100, 61,  0,   50,  61,  0,   65,  0,   100, 56,  9,   0,
    9,   0,   101, 56,  102, 0,   102, 0,   0,  7,   0,   103, 56,  106, 0,   106, 0,   0,   111, 106, 32,  107, 42,
    106, 0,   111, 107, 42,  106, 0,   14,  0,  15,  0,   111, 106, 0,   52,  0,   53,  0,   66,  106, 61,  0,   107,
    0,   59,  106, 67,  106, 37,  106, 60,  0,  40,  106, 61,  0,   18,  52,  56,  52,  56,  106, 61,  0,   19,  106,
    61,  0,   21,  106, 61,  0,   14,  0,   15, 0,   20,  106, 61,  0,   25,  7,   60,  0,   24,  103, 61,  0,   51,
    106, 61,  0,   106, 38,  0,   106, 108, 0,  29,  106, 0,   30,  106, 0,   31,  106, 0,   106, 13,  106, 0,   106,
    5,   106, 0,   106, 4,   106, 0,   106, 3,  106, 0,   106, 6,   106, 0,   106, 25,  7,   60,  105, 0,   105, 0,
    9,   0,   41,  106, 60,  0,   10,  0,   43, 106, 60,  0,   7,   0,   106, 39,  59,  106, 37,  106, 60,  0,   16,
    0,   107, 63,  0,   51,  106, 61,  0,   57, 52,  58,  0,   26,  110, 0,   22,  0,   23,  0,   8,   0,   46,  113,
    0,   113, 56,  114, 0,   114, 0,   7,   28, 7,   0};

#endif

#if YY_SSLParser_DEBUG != 0
static const short yyrline[] = {
    0,    219,  223,  228,  231,  233,  236,  239,  244,  246,  249,  253,  256,  259,  262,  266,  268,  271,
    289,  306,  307,  310,  314,  314,  317,  319,  320,  323,  329,  334,  368,  390,  404,  415,  419,  426,
    434,  441,  454,  460,  466,  470,  476,  487,  492,  500,  503,  508,  512,  517,  523,  526,  541,  558,
    563,  567,  573,  579,  585,  592,  598,  604,  610,  615,  621,  625,  648,  652,  655,  661,  665,  678,
    687,  698,  707,  712,  722,  729,  736,  752,  756,  759,  764,  772,  782,  789,  793,  798,  803,  808,
    813,  818,  828,  834,  839,  846,  851,  856,  860,  864,  868,  873,  878,  883,  888,  893,  896,  901,
    907,  930,  956,  961,  969,  978,  982,  986,  990,  994,  998,  1002, 1006, 1013, 1036, 1041, 1068, 1072,
    1078, 1082, 1099, 1103, 1108, 1111, 1116, 1122, 1127, 1131, 1136, 1165, 1169, 1172, 1176};

static const char *const yytname[] = {
    "$",           "error",                "$illegal.",     "COND_OP",       "BIT_OP",       "ARITH_OP",
    "LOG_OP",      "NAME",                 "ASSIGNTYPE",    "REG_ID",        "REG_NUM",      "COND_TNAME",
    "DECOR",       "FARITH_OP",            "FPUSH",         "FPOP",          "TEMP",         "SHARES",
    "CONV_FUNC",   "TRUNC_FUNC",           "TRANSCEND",     "FABS_FUNC",     "BIG",          "LITTLE",
    "NAME_CALL",   "NAME_LOOKUP",          "ENDIANNESS",    "COVERS",        "INDEX",        "NOT",
    "LNOT",        "FNEG",                 "THEN",          "LOOKUP_RDC",    "BOGUS",        "ASSIGN",
    "TO",          "COLON",                "S_E",           "AT",            "ADDR",         "REG_IDX",
    "EQUATE",      "MEM_IDX",              "TOK_INTEGER",   "TOK_FLOAT",     "FAST",         "OPERAND",
    "FETCHEXEC",   "CAST_OP",              "FLAGMACRO",     "SUCCESSOR",     "NUM",          "FLOATNUM",
    "FCHS",        "';'",                  "','",           "'{'",           "'}'",          "'['",
    "']'",         "')'",                  "'\"'",          "'\\''",         "'$'",          "'_'",
    "'('",         "'?'",                  "specorasgn",    "specification", "parts",        "operandlist",
    "operand",     "func_parameter",       "reglist",       "@1",            "@2",           "a_reglists",
    "a_reglist",   "reg_table",            "flag_fnc",      "constants",     "table_assign", "table_expr",
    "str_expr",    "str_array",            "str_term",      "name_expand",   "bin_oper",     "opstr_expr",
    "opstr_array", "exprstr_expr",         "exprstr_array", "instr",         "@3",           "instr_name",
    "instr_elem",  "name_contract",        "rt_list",       "rt",            "flag_list",    "list_parameter",
    "param",       "list_actualparameter", "assign_rt",     "exp_term",      "exp",          "location",
    "cast",        "endianness",           "esize",         "assigntype",    "fastlist",     "fastentries",
    "fastentry",   ""};
#endif

static const short yyr1[] = {0,   68,  68,  68,  69,  69,  70,  70,  70,  70,  70,  70,  70,  70,  70,  71,  71,  72,
                             72,  73,  73,  75,  74,  76,  74,  77,  77,  78,  78,  78,  78,  78,  78,  79,  79,  80,
                             81,  81,  82,  83,  83,  83,  84,  84,  85,  85,  85,  86,  86,  87,  87,  87,  87,  88,
                             88,  88,  89,  90,  90,  91,  92,  92,  94,  93,  95,  95,  96,  96,  96,  97,  97,  97,
                             97,  97,  97,  98,  98,  99,  99,  99,  99,  99,  100, 100, 101, 101, 101, 102, 103, 103,
                             103, 104, 104, 104, 104, 104, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105, 105,
                             105, 105, 105, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 106, 107, 107, 107,
                             107, 107, 107, 107, 107, 107, 108, 109, 110, 110, 111, 112, 113, 113, 114};

static const short yyr2[] = {0,  1,  1,  1, 3, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 3, 1, 5, 5, 3, 0, 0, 3, 0, 3, 3, 1, 3, 6,
                             10, 14, 10, 8, 3, 1, 6, 3, 5, 3, 1, 1, 1, 2, 1, 3, 4, 1, 3, 1, 3, 3, 2, 1, 1, 1, 1, 3, 5,
                             3,  3,  5,  3, 0, 4, 1, 2, 1, 1, 2, 3, 3, 3, 4, 4, 3, 2, 1, 1, 3, 3, 2, 1, 3, 1, 3, 1, 0,
                             1,  3,  1,  0, 6, 4, 1, 1, 2, 1, 1, 3, 1, 7, 3, 7, 3, 3, 1, 1, 3, 3, 3, 3, 2, 2, 2, 2, 2,
                             3,  3,  3,  3, 3, 5, 1, 1, 3, 1, 3, 1, 7, 1, 2, 3, 3, 2, 1, 1, 1, 2, 3, 1, 3};

static const short yydefact[] = {
    0,   66,  136, 123, 125, 105, 106, 129, 0,   0,   0,   0,   86,  0,   0,   0,   0,  0,   0,   0,   0,   21,
    23,  0,   0,   0,   0,   96,  97,  0,   0,   0,   0,   0,   3,   0,   12,  13,  8,  9,   6,   62,  64,  67,
    1,   122, 2,   99,  10,  0,   11,  0,   0,   127, 105, 106, 90,  0,   0,   0,   0,  127, 0,   85,  0,   89,
    0,   0,   134, 135, 133, 113, 114, 115, 0,   0,   0,   0,   0,   0,   137, 139, 87, 14,  16,  86,  93,  94,
    90,  0,   81,  7,   76,  77,  0,   0,   0,   0,   0,   0,   66,  86,  0,   0,   5,  65,  86,  68,  0,   0,
    0,   0,   0,   0,   111, 0,   0,   112, 130, 95,  99,  52,  36,  0,   0,   0,   0,  38,  39,  43,  48,  40,
    41,  0,   0,   103, 107, 104, 0,   0,   0,   109, 71,  70,  101, 124, 126, 0,   0,  22,  26,  24,  0,   0,
    0,   0,   20,  0,   83,  80,  0,   75,  110, 0,   74,  69,  0,   0,   98,  0,   4,  0,   119, 118, 117, 120,
    116, 0,   0,   0,   0,   0,   0,   0,   0,   46,  0,   0,   0,   0,   0,   51,  42, 0,   108, 84,  0,   88,
    0,   0,   34,  0,   0,   140, 138, 15,  86,  86,  0,   78,  0,   79,  0,   73,  72, 71,  63,  0,   0,   132,
    0,   99,  92,  37,  53,  54,  127, 55,  0,   0,   0,   47,  0,   56,  0,   59,  50, 49,  0,   0,   27,  0,
    0,   0,   25,  0,   0,   0,   82,  0,   121, 0,   0,   58,  61,  0,   44,  0,   0,  0,   35,  0,   33,  0,
    17,  19,  18,  0,   0,   91,  45,  0,   0,   102, 0,   0,   100, 128, 57,  60,  28, 0,   0,   0,   0,   0,
    0,   32,  0,   0,   0,   0,   29,  31,  0,   0,   0,   30,  0,   0,   0};

static const short yydefgoto[] = {298, 34,  35,  83,  84,  208, 36,  77,  78,  149, 150, 201, 37, 38, 39, 127,
                                  185, 186, 129, 130, 228, 131, 187, 132, 188, 40,  106, 41,  42, 43, 91, 92,
                                  160, 62,  63,  64,  93,  45,  65,  47,  117, 48,  70,  49,  50, 80, 81};

static const short yypact[] = {
    146,    463,    -32768, -32768, -32768, 28,     35,     -32768, -11,    246,    246,    246,    299,    7,
    108,    246,    246,    246,    246,    246,    246,    -32768, -32768, 51,     93,     36,     246,    -32768,
    -32768, 246,    106,    131,    94,     246,    178,    90,     -32768, -32768, -32768, -32768, -32768, 156,
    80,     -32768, -32768, -32768, 610,    110,    -32768, 246,    -32768, 84,     127,    -32768, -32768, -32768,
    246,    200,    343,    358,    373,    140,    150,    -32768, 159,    610,    158,    168,    -32768, -32768,
    -32768, 82,     82,     610,    388,    449,    478,    -4,     -4,     191,    174,    -32768, -32768, 189,
    -32768, 5,      -32768, -32768, 246,    4,      -32768, 36,     -32768, -32768, 403,    18,     206,    209,
    46,     418,    212,    93,     47,     208,    -32768, -32768, 93,     -32768, 246,    246,    246,    246,
    246,    262,    -32768, 214,    222,    -32768, -32768, 552,    -17,    -32768, 273,    33,     272,    274,
    275,    -32768, 95,     -32768, -32768, -32768, -32768, 228,    223,    -32768, -32768, -32768, 93,     227,
    246,    -32768, 523,    -32768, -32768, -32768, -32768, 2,      276,    232,    -32768, 232,    283,    51,
    93,     234,    187,    160,    -32768, -32768, 183,    -32768, 3,      246,    -32768, -32768, 233,    235,
    -32768, 236,    -32768, 24,     599,    112,    175,    599,    82,     244,    246,    249,    246,    246,
    240,    95,     63,     95,     -22,    13,     192,    254,    231,    -32768, -32768, 255,    -32768, -32768,
    36,     610,    269,    285,    -32768, 171,    -4,     -32768, -32768, -32768, 93,     93,     317,    -32768,
    318,    -32768, 568,    -32768, -32768, -32768, 36,     246,    595,    -32768, 610,    132,    610,    -32768,
    -32768, -32768, 254,    -32768, 264,    297,    172,    -32768, 281,    -32768, 282,    -32768, -32768, -32768,
    246,    529,    -32768, 278,    336,    296,    -32768, 201,    177,    246,    -32768, 246,    -32768, 246,
    246,    -32768, -32768, 1,      95,     119,    246,    433,    -32768, 329,    -32768, 308,    -32768, -32768,
    610,    494,    509,    610,    -32768, 307,    328,    -32768, 320,    310,    -32768, -32768, -32768, -32768,
    25,     345,    365,    366,    332,    341,    351,    352,    330,    386,    347,    350,    -32768, -32768,
    367,    353,    354,    -32768, 410,    417,    -32768};

static const short yypgoto[] = {-32768, -32768, 384,    -32768, 266,    -32768, -32768, -32768, -32768, 355,
                                230,    -32768, -32768, -32768, -32768, -32768, -48,    -32768, -121,   -32768,
                                190,    -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, 383,
                                -134,   -90,    -32768, -79,    -20,    356,    429,    218,    0,      -47,
                                -32768, -32768, -32768, 242,    -32768, -32768, 287};

#define YYLAST 667

static const short yytable[] = {
    46,   161,  120,  128,  85,  147,  156,  192,  189,  58,   59,  60,  82,  158, 66,  71,  72,   73,  74,  75,
    76,   108,  109,  110,  111, 181,  94,   171,  -93,  95,   198, 112, 2,   99,  230, -94, 231,  216, 86,  87,
    121,  52,   282,  113,  2,   -131, 118,  155,  88,   119,  86,  87,  283, 166, 169, 148, 114,  115, 79,  67,
    88,   199,  239,  270,  192, 159,  -131, 224,  225,  232,  226, 233, 3,   4,   89,  116, 227,  54,  55,  7,
    138,  8,    9,    10,   11,  163,  89,   56,   57,   90,   183, 121, 15,  16,  17,  184, 125,  126, 167, 67,
    82,   90,   121,  18,   19,  102,  20,   113,  172,  173,  174, 175, 176, 96,  26,  27,  28,   110, 195, 98,
    114,  115,  29,   224,  225, 112,  161,  245,  246,  33,   68,  69,  227, 221, 85,  192, 122,  113, 97,  116,
    197,  123,  30,   31,   32,  104,  124,  125,  126,  161,  114, 115, 183, 1,   2,   3,   4,    124, 125, 126,
    5,    6,    7,    212,  8,   9,    10,   11,   105,  116,  12,  13,  14,  118, 252, 15,  16,   17,  218, 121,
    220,  222,  256,  133,  229, 100,  18,   19,   112,  20,   21,  22,  23,  24,  25,  118, -87,  26,  27,  28,
    113,  -87,  101,  102,  14,  29,   138,  134,  30,   31,   32,  139, 33,  114, 115, 140, 140,  220, 142, 152,
    141,  209,  21,   22,   23,  24,   25,   242,  143,  183,  153, 243, 116, 138, 255, 125, 126,  265, 259, 210,
    30,   31,   32,   138,  211, 154,  207,  266,  234,  267,  235, 268, 269, 53,  51,  3,   4,    138, 272, 264,
    54,   55,   7,    170,  8,   9,    10,   11,   164,  177,  56,  57,  165, 178, 179, 15,  16,   17,  182, 189,
    193,  190,  191,  194,  196, 200,  18,   19,   202,  20,   203, 206, 223, 213, 237, 214, 215,  26,  27,  28,
    108,  109,  110,  111,  217, 29,   61,   219,  3,    4,    112, 238, 33,  54,  55,  7,   236,  8,   9,   10,
    11,   240,  113,  56,   57,  2,    253,  248,  15,   16,   17,  108, 109, 110, 111, 114, 115,  241, 261, 18,
    19,   112,  20,   257,  258, 262,  108,  109,  110,  111,  26,  27,  28,  113, 116, 263, 112,  274, 29,  254,
    275,  108,  109,  110,  111, 33,   114,  115,  113,  278,  281, 112, 280, 284, 285, 286, 108,  109, 110, 111,
    288,  114,  115,  113,  287, 116,  112,  289,  290,  291,  279, 108, 109, 110, 111, 292, 114,  115, 113, 293,
    116,  112,  294,  295,  135, 296,  108,  109,  110,  111,  299, 114, 115, 113, 297, 116, 112,  300, 103, 136,
    205,  108,  109,  110,  111, 107,  114,  115,  113,  44,   116, 112, 244, 151, 137, 250, 108,  109, 110, 111,
    204,  114,  115,  113,  157, 116,  112,  271,  0,    144,  247, 0,   108, 109, 110, 111, 114,  115, 113, 0,
    116,  0,    112,  -127, 162, 0,    -127, -127, -127, -127, 0,   114, 115, 0,   113, 116, -127, 0,   0,   168,
    0,    108,  109,  110,  111, 0,    0,    114,  115,  0,    116, 112, 0,   0,   273, 0,   0,    108, 109, 110,
    111,  -127, -127, 113,  0,   51,   116,  112,  0,    145,  0,   0,   108, 109, 110, 111, 114,  115, 0,   113,
    -127, 0,    112,  -108, 0,   0,    -108, -108, -108, -108, 0,   0,   114, 115, 113, 116, -108, 2,   146, 0,
    0,    0,    0,    86,   87,  0,    0,    114,  115,  0,    0,   116, 0,   88,  276, 108, 109,  110, 111, 0,
    0,    -108, -108, 0,    0,   112,  116,  0,    0,    277,  0,   108, 109, 110, 111, 0,   0,    113, 0,   89,
    -108, 112,  0,    0,    180, 0,    0,    260,  0,    0,    114, 115, 0,   113, 90,  0,   0,    0,   108, 109,
    110,  111,  108,  109,  110, 249,  114,  115,  112,  116,  0,   0,   112, 108, 109, 110, 111,  0,   0,   0,
    113,  0,    0,    112,  113, 116,  0,    0,    0,    0,    0,   0,   251, 114, 115, 113, 0,    114, 115, 0,
    0,    0,    0,    0,    0,   0,    0,    0,    114,  115,  0,   0,   116, 0,   0,   0,   116,  0,   0,   0,
    0,    0,    0,    0,    0,   0,    0,    116};

static const short yycheck[] = {
    0,   91,  49, 51,  24, 9,   85,  128, 7,   9,   10,  11,  7,   9,   7,   15,  16,  17, 18,  19,  20,  3,  4,   5,
    6,   42,  26, 106, 0,  29,  28,  13,  8,   33,  56,  0,   58,  171, 14,  15,  7,   52, 17,  25,  8,   42, 63,  42,
    24,  49,  14, 15,  27, 7,   7,   59,  38,  39,  7,   52,  24,  59,  196, 62,  185, 61, 63,  4,   5,   56, 7,   58,
    9,   10,  50, 57,  13, 14,  15,  16,  56,  18,  19,  20,  21,  67,  50,  24,  25,  65, 57,  7,   29,  30, 31,  62,
    63,  64,  52, 52,  7,  65,  7,   40,  41,  25,  43,  25,  108, 109, 110, 111, 112, 7,  51,  52,  53,  5,  138, 25,
    38,  39,  59, 4,   5,  13,  216, 206, 207, 66,  22,  23,  13,  180, 154, 256, 52,  25, 7,   57,  140, 57, 62,  63,
    64,  55,  62, 63,  64, 239, 38,  39,  57,  7,   8,   9,   10,  62,  63,  64,  14,  15, 16,  163, 18,  19, 20,  21,
    12,  57,  24, 25,  26, 63,  42,  29,  30,  31,  178, 7,   180, 181, 230, 56,  184, 7,  40,  41,  13,  43, 44,  45,
    46,  47,  48, 63,  56, 51,  52,  53,  25,  61,  24,  25,  26,  59,  56,  7,   62,  63, 64,  61,  66,  38, 39,  56,
    56,  217, 60, 28,  61, 61,  44,  45,  46,  47,  48,  56,  60,  57,  56,  60,  57,  56, 62,  63,  64,  60, 238, 56,
    62,  63,  64, 56,  61, 56,  59,  247, 56,  249, 58,  251, 252, 7,   42,  9,   10,  56, 258, 58,  14,  15, 16,  55,
    18,  19,  20, 21,  62, 7,   24,  25,  63,  59,  52,  29,  30,  31,  5,   7,   52,  7,  7,   60,  57,  9,  40,  41,
    56,  43,  7,  57,  52, 60,  63,  60,  60,  51,  52,  53,  3,   4,   5,   6,   60,  59, 7,   58,  9,   10, 13,  56,
    66,  14,  15, 16,  62, 18,  19,  20,  21,  52,  25,  24,  25,  8,   62,  9,   29,  30, 31,  3,   4,   5,  6,   38,
    39,  52,  60, 40,  41, 13,  43,  62,  62,  9,   3,   4,   5,   6,   51,  52,  53,  25, 57,  59,  13,  28, 59,  62,
    52,  3,   4,  5,   6,  66,  38,  39,  25,  62,  60,  13,  52,  28,  9,   9,   3,   4,  5,   6,   39,  38, 39,  25,
    52,  57,  13, 36,  36, 59,  62,  3,   4,   5,   6,   9,   38,  39,  25,  52,  57,  13, 52,  36,  61,  52, 3,   4,
    5,   6,   0,  38,  39, 25,  60,  57,  13,  0,   34,  61,  154, 3,   4,   5,   6,   42, 38,  39,  25,  0,  57,  13,
    202, 78,  61, 217, 3,  4,   5,   6,   153, 38,  39,  25,  88,  57,  13,  257, -1,  61, 208, -1,  3,   4,  5,   6,
    38,  39,  25, -1,  57, -1,  13,  0,   61,  -1,  3,   4,   5,   6,   -1,  38,  39,  -1, 25,  57,  13,  -1, -1,  61,
    -1,  3,   4,  5,   6,  -1,  -1,  38,  39,  -1,  57,  13,  -1,  -1,  61,  -1,  -1,  3,  4,   5,   6,   38, 39,  25,
    -1,  42,  57, 13,  -1, 60,  -1,  -1,  3,   4,   5,   6,   38,  39,  -1,  25,  57,  -1, 13,  0,   -1,  -1, 3,   4,
    5,   6,   -1, -1,  38, 39,  25,  57,  13,  8,   60,  -1,  -1,  -1,  -1,  14,  15,  -1, -1,  38,  39,  -1, -1,  57,
    -1,  24,  60, 3,   4,  5,   6,   -1,  -1,  38,  39,  -1,  -1,  13,  57,  -1,  -1,  60, -1,  3,   4,   5,  6,   -1,
    -1,  25,  -1, 50,  57, 13,  -1,  -1,  32,  -1,  -1,  58,  -1,  -1,  38,  39,  -1,  25, 65,  -1,  -1,  -1, 3,   4,
    5,   6,   3,  4,   5,  37,  38,  39,  13,  57,  -1,  -1,  13,  3,   4,   5,   6,   -1, -1,  -1,  25,  -1, -1,  13,
    25,  57,  -1, -1,  -1, -1,  -1,  -1,  37,  38,  39,  25,  -1,  38,  39,  -1,  -1,  -1, -1,  -1,  -1,  -1, -1,  -1,
    38,  39,  -1, -1,  57, -1,  -1,  -1,  57,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, -1,  57};

/* fattrs + tables */

/* parser code folow  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */

/* macro to keep goto */

#define YYBEGINGOTO
#define YYLABEL(lb) lb:
#define YYDECLARELABEL(lb)

/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
static int __alloca_free_ptr(char *ptr, char *ref) {
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

#define yyerrok (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)
#define YYEMPTY -2
#define YYEOF 0
#define YYACCEPT __ALLOCA_return(0)
#define YYABORT __ALLOCA_return(1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL goto yyerrlab
#define YYRECOVERING() (!!yyerrstatus)
#define YYBACKUP(token, value)                                                                                         \
    do                                                                                                                 \
        if (yychar == YYEMPTY && yylen == 1) {                                                                         \
            yychar = (token), yylval = (value);                                                                        \
            yychar1 = YYTRANSLATE(yychar);                                                                             \
            YYPOPSTACK;                                                                                                \
            goto yybackup;                                                                                             \
        } else {                                                                                                       \
            yyerror((char *) "syntax error: cannot back up");                                                          \
            goto yyerrlab1;                                                                                            \
        }                                                                                                              \
    while (0)

#define YYTERROR 1
#define YYERRCODE 256

#ifndef YY_SSLParser_PURE
/* UNPURE */
#define YYLEX yylex()
#else
#endif

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

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy(char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy(char *from, char *to, int count)
#else
static void __yy_bcopy(from, to, count) char *from;
char *to;
int count;
#endif
#endif
{
    char *f = from;
    char *t = to;
    int i = count;

    while (i-- > 0)
        *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
SSLParser::
#endif
    yyparse(RTLInstDict &Dict)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
    /* parameter definition without protypes */
    YY_SSLParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
    int yystate;
    int yyn;
    short *yyssp;
    yy_SSLParser_stype *yyvsp;
    int yyerrstatus; /*  number of tokens to shift before error messages enabled */
    int yychar1 = 0; /*  lookahead token as an internal (translated) token number */

    short yyssa[YYINITDEPTH];              /*  the state stack                     */
    yy_SSLParser_stype yyvsa[YYINITDEPTH]; /*  the semantic value stack            */

    short *yyss = yyssa;              /*  refer to the stacks thru separate pointers */
    yy_SSLParser_stype *yyvs = yyvsa; /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_SSLParser_LSP_NEEDED
    YY_SSLParser_LTYPE yylsa[YYINITDEPTH]; /*  the location stack                  */
    YY_SSLParser_LTYPE *yyls = yylsa;
    YY_SSLParser_LTYPE *yylsp;

#define YYPOPSTACK (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK (yyvsp--, yyssp--)
#endif

    int yystacksize = YYINITDEPTH;

#ifdef YY_SSLParser_PURE
    int yychar;
    yy_SSLParser_stype yylval;
    int yynerrs;
#ifdef YY_SSLParser_LSP_NEEDED
    YY_SSLParser_LTYPE yylloc;
#endif
#endif

    yy_SSLParser_stype yyval; /*  the variable used to return         */
    /*  semantic values from the action     */
    /*  routines                            */

    int yylen;
    /* start loop, in which YYGOTO may be used. */
    YYBEGINGOTO

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG)
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
#ifdef YY_SSLParser_LSP_NEEDED
    yylsp = yyls;
#endif

    /* Push a new state, which is found in  yystate  .  */
    /* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
    YYLABEL(yynewstate)

    *++yyssp = yystate;

    if (yyssp >= yyss + yystacksize - 1) {
        /* Give user a chance to reallocate the stack */
        /* Use copies of these so that the &'s don't force the real ones into memory. */
        yy_SSLParser_stype *yyvs1 = yyvs;
        short *yyss1 = yyss;
#ifdef YY_SSLParser_LSP_NEEDED
        YY_SSLParser_LTYPE *yyls1 = yyls;
#endif

        /* Get the current used size of the three stacks, in elements.  */
        int size = yyssp - yyss + 1;

#ifdef yyoverflow
/* Each stack pointer address is followed by the size of
 the data in use in that stack, in bytes.  */
#ifdef YY_SSLParser_LSP_NEEDED
        /* This used to be a conditional around just the two extra args,
         but that might be undefined if yyoverflow is a macro.  */
        yyoverflow("parser stack overflow", &yyss1, size * sizeof(*yyssp), &yyvs1, size * sizeof(*yyvsp), &yyls1,
                   size * sizeof(*yylsp), &yystacksize);
#else
        yyoverflow("parser stack overflow", &yyss1, size * sizeof(*yyssp), &yyvs1, size * sizeof(*yyvsp), &yystacksize);
#endif

        yyss = yyss1;
        yyvs = yyvs1;
#ifdef YY_SSLParser_LSP_NEEDED
        yyls = yyls1;
#endif
#else /* no yyoverflow */
        /* Extend the stack our own way.  */
        if (yystacksize >= YYMAXDEPTH) {
            yyerror((char *)"parser stack overflow");
            __ALLOCA_return(2);
        }
        yystacksize *= 2;
        if (yystacksize > YYMAXDEPTH)
            yystacksize = YYMAXDEPTH;
        yyss = (short *)__ALLOCA_alloca(yystacksize * sizeof(*yyssp));
        __yy_bcopy((char *)yyss1, (char *)yyss, size * sizeof(*yyssp));
        __ALLOCA_free(yyss1, yyssa);
        yyvs = new (__ALLOCA_alloca(yystacksize * sizeof(*yyvsp))) yy_SSLParser_stype[yystacksize];
        for(int i=0; i<size; ++i) {
            yyvs[i] = yyvs1[i];
        }
        __ALLOCA_free(yyvs1, yyvsa);
#ifdef YY_SSLParser_LSP_NEEDED
        yyls = (YY_SSLParser_LTYPE *)__ALLOCA_alloca(yystacksize * sizeof(*yylsp));
        __yy_bcopy((char *)yyls1, (char *)yyls, size * sizeof(*yylsp));
        __ALLOCA_free(yyls1, yylsa);
#endif
#endif /* no yyoverflow */

        yyssp = yyss + size - 1;
        yyvsp = yyvs + size - 1;
#ifdef YY_SSLParser_LSP_NEEDED
        yylsp = yyls + size - 1;
#endif

#if YY_SSLParser_DEBUG != 0
        if (YY_SSLParser_DEBUG_FLAG)
            fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

        if (yyssp >= yyss + yystacksize - 1)
            YYABORT;
    }

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG)
        fprintf(stderr, "Entering state %d\n", yystate);
#endif

    goto yybackup;
    YYLABEL(yybackup)

    /* Do appropriate processing given the current state.  */
    /* Read a lookahead token if we need one and don't already have one.  */
    /* YYLABEL(yyresume) */

    /* First try to decide what to do without reference to lookahead token.  */

    yyn = yypact[yystate];
    if (yyn == YYFLAG)
        goto yydefault;

    /* Not known => get a lookahead token if don't already have one.  */

    /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

    if (yychar == YYEMPTY) {
#if YY_SSLParser_DEBUG != 0
        if (YY_SSLParser_DEBUG_FLAG)
            fprintf(stderr, "Reading a token: ");
#endif
        yychar = YYLEX;
    }

    /* Convert token to internal form (in yychar1) for indexing tables with */

    if (yychar <= 0) /* This means end of input. */
    {
        yychar1 = 0;
        yychar = YYEOF; /* Don't call YYLEX any more */

#if YY_SSLParser_DEBUG != 0
        if (YY_SSLParser_DEBUG_FLAG)
            fprintf(stderr, "Now at end of input.\n");
#endif
    } else {
        yychar1 = YYTRANSLATE(yychar);

#if YY_SSLParser_DEBUG != 0
        if (YY_SSLParser_DEBUG_FLAG) {
            fprintf(stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
/* Give the individual parser a way to print the precise meaning
 of a token, for further debugging info.  */
#ifdef YYPRINT
            YYPRINT(stderr, yychar, yylval);
#endif
            fprintf(stderr, ")\n");
        }
#endif
    }

    yyn += yychar1;
    if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
        goto yydefault;

    yyn = yytable[yyn];

    /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

    if (yyn < 0) {
        if (yyn == YYFLAG)
            goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
    } else if (yyn == 0)
        goto yyerrlab;

    if (yyn == YYFINAL)
        YYACCEPT;

/* Shift the lookahead token.  */

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG)
        fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

    /* Discard the token being shifted unless it is eof.  */
    if (yychar != YYEOF)
        yychar = YYEMPTY;

    *++yyvsp = yylval;
#ifdef YY_SSLParser_LSP_NEEDED
    *++yylsp = yylloc;
#endif

    /* count tokens shifted since error; after three, turn off error status.  */
    if (yyerrstatus)
        yyerrstatus--;

    yystate = yyn;
    goto yynewstate;

    /* Do the default action for the current state.  */
    YYLABEL(yydefault)

    yyn = yydefact[yystate];
    if (yyn == 0)
        goto yyerrlab;

    /* Do a reduction.  yyn is the number of a rule to reduce with.  */
    YYLABEL(yyreduce)
    yylen = yyr2[yyn];
    if (yylen > 0)
        yyval = yyvsp[1 - yylen]; /* implement default value of the action */

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG) {
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
        the_asgn = yyvsp[0].regtransfer;
        ;
        break;
    }
    case 2: {
        the_asgn = new Assign(new Terminal(opNil), yyvsp[0].exp);
        ;
        break;
    }
    case 7: {
        Dict.fetchExecCycle = yyvsp[0].rtlist;
        ;
        break;
    }
    case 14: {
        Dict.fixupParams();
        ;
        break;
    }
    case 17: {
        // Note: the below copies the list of strings!
        Dict.DetParamMap[yyvsp[-4].str].params = *yyvsp[-1].parmlist;
        Dict.DetParamMap[yyvsp[-4].str].kind = PARAM_VARIANT;
        // delete $4;
        ;
        break;
    }
    case 18: {
        std::map<QString, InsNameElem *> m;
        ParamEntry &param = Dict.DetParamMap[yyvsp[-4].str];
        Instruction *asgn = new Assign(yyvsp[-1].typ, new Terminal(opNil), yyvsp[0].exp);
        // Note: The below 2 copy lists of strings (to be deleted below!)
        param.params = *yyvsp[-3].parmlist;
        param.funcParams = *yyvsp[-2].parmlist;
        param.asgn = asgn;
        param.kind = PARAM_ASGN;

        if (param.funcParams.size() != 0)
            param.kind = PARAM_LAMBDA;
        // delete $2;
        // delete $3;
        ;
        break;
    }
    case 19: {
        yyval.parmlist = yyvsp[-1].parmlist;
        ;
        break;
    }
    case 20: {
        yyval.parmlist = new std::list<QString>();
        ;
        break;
    }
    case 21: {
        bFloat = false;
        ;
        break;
    }
    case 23: {
        bFloat = true;
        ;
        break;
    }
    case 27: {
        if (Dict.RegMap.find(yyvsp[-2].str) != Dict.RegMap.end())
            yyerror("Name reglist decared twice\n");
        Dict.RegMap[yyvsp[-2].str] = yyvsp[0].num;
        ;
        break;
    }
    case 28: {
        if (Dict.RegMap.find(yyvsp[-5].str) != Dict.RegMap.end())
            yyerror("Name reglist declared twice\n");
        Dict.addRegister(yyvsp[-5].str, yyvsp[0].num, yyvsp[-3].num, bFloat);
        ;
        break;
    }
    case 29: {
        if (Dict.RegMap.find(yyvsp[-9].str) != Dict.RegMap.end())
            yyerror("Name reglist declared twice\n");
        Dict.RegMap[yyvsp[-9].str] = yyvsp[-4].num;
        // Now for detailed Reg information
        if (Dict.DetRegMap.find(yyvsp[-4].num) != Dict.DetRegMap.end())
            yyerror("Index used for more than one register\n");
        Dict.DetRegMap[yyvsp[-4].num].s_name(yyvsp[-9].str);
        Dict.DetRegMap[yyvsp[-4].num].s_size(yyvsp[-7].num);
        Dict.DetRegMap[yyvsp[-4].num].s_address(nullptr);
        // check range is legitimate for size. 8,10
        if ((Dict.RegMap.find(yyvsp[-2].str) == Dict.RegMap.end()) ||
            (Dict.RegMap.find(yyvsp[0].str) == Dict.RegMap.end()))
            yyerror("Undefined range\n");
        else {
            int bitsize = Dict.DetRegMap[Dict.RegMap[yyvsp[0].str]].g_size();
            for (int i = Dict.RegMap[yyvsp[-2].str]; i != Dict.RegMap[yyvsp[0].str]; i++) {
                if (Dict.DetRegMap.find(i) == Dict.DetRegMap.end()) {
                    yyerror("Not all registers in range defined\n");
                    break;
                }
                bitsize += Dict.DetRegMap[i].g_size();
                if (bitsize > yyvsp[-7].num) {
                    yyerror("Range exceeds size of register\n");
                    break;
                }
            }
            if (bitsize < yyvsp[-7].num)
                yyerror("Register size is exceeds registers in range\n");
            // copy information
        }
        Dict.DetRegMap[yyvsp[-4].num].s_mappedIndex(Dict.RegMap[yyvsp[-2].str]);
        Dict.DetRegMap[yyvsp[-4].num].s_mappedOffset(0);
        Dict.DetRegMap[yyvsp[-4].num].s_float(bFloat);
        ;
        break;
    }
    case 30: {
        if (Dict.RegMap.find(yyvsp[-13].str) != Dict.RegMap.end())
            yyerror("Name reglist declared twice\n");
        Dict.RegMap[yyvsp[-13].str] = yyvsp[-8].num;
        // Now for detailed Reg information
        if (Dict.DetRegMap.find(yyvsp[-8].num) != Dict.DetRegMap.end())
            yyerror("Index used for more than one register\n");
        Dict.DetRegMap[yyvsp[-8].num].s_name(yyvsp[-13].str);
        Dict.DetRegMap[yyvsp[-8].num].s_size(yyvsp[-11].num);
        Dict.DetRegMap[yyvsp[-8].num].s_address(nullptr);
        // Do checks
        if (yyvsp[-11].num != (yyvsp[-1].num - yyvsp[-3].num) + 1)
            yyerror("Size does not equal range\n");
        if (Dict.RegMap.find(yyvsp[-6].str) != Dict.RegMap.end()) {
            if (yyvsp[-1].num >= Dict.DetRegMap[Dict.RegMap[yyvsp[-6].str]].g_size())
                yyerror("Range extends over target register\n");
        } else
            yyerror("Shared index not yet defined\n");
        Dict.DetRegMap[yyvsp[-8].num].s_mappedIndex(Dict.RegMap[yyvsp[-6].str]);
        Dict.DetRegMap[yyvsp[-8].num].s_mappedOffset(yyvsp[-3].num);
        Dict.DetRegMap[yyvsp[-8].num].s_float(bFloat);
        ;
        break;
    }
    case 31: {
        if ((int)yyvsp[-8].strlist->size() != (yyvsp[0].num - yyvsp[-2].num + 1)) {
            LOG_STREAM() << "size of register array does not match mapping to r[" << yyvsp[-2].num << ".." << yyvsp[0].num
                      << "]\n";
            exit(1);
        } else {
            std::list<QString>::iterator loc = yyvsp[-8].strlist->begin();
            for (int x = yyvsp[-2].num; x <= yyvsp[0].num; x++, loc++) {
                if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.addRegister(*loc, x, yyvsp[-5].num, bFloat);
            }
            // delete $2;
        };
        break;
    }
    case 32: {
        std::list<QString>::iterator loc = yyvsp[-6].strlist->begin();
        for (; loc != yyvsp[-6].strlist->end(); loc++) {
            if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                yyerror("Name reglist declared twice\n");
            Dict.addRegister(*loc, yyvsp[0].num, yyvsp[-3].num, bFloat);
        }
        // delete $2;
        ;
        break;
    }
    case 33: {
        yyvsp[-2].strlist->push_back(yyvsp[0].str);
        ;
        break;
    }
    case 34: {
        if(yyval.strlist)
            yyval.strlist->clear();
        yyval.strlist = new std::list<QString> {yyvsp[0].str};
        ;
        break;
    }
    case 35: {
        // Note: $2 is a list of strings
        Dict.FlagFuncs[yyvsp[-5].str] = new FlagDef(listStrToExp(yyvsp[-4].parmlist), yyvsp[-1].rtlist);
        ;
        break;
    }
    case 36: {
        if (ConstTable.find(yyvsp[-2].str) != ConstTable.end())
            yyerror("Constant declared twice");
        ConstTable[yyvsp[-2].str] = yyvsp[0].num;
        ;
        break;
    }
    case 37: {
        if (ConstTable.find(yyvsp[-4].str) != ConstTable.end())
            yyerror("Constant declared twice");
        else if (yyvsp[-1].str == "-")
            ConstTable[yyvsp[-4].str] = yyvsp[-2].num - yyvsp[0].num;
        else if (yyvsp[-1].str == "+")
            ConstTable[yyvsp[-4].str] = yyvsp[-2].num + yyvsp[0].num;
        else
            yyerror("Constant expression must be NUM + NUM or NUM - NUM");
        ;
        break;
    }
    case 38: {
        TableDict[yyvsp[-2].str] = yyvsp[0].tab;
        ;
        break;
    }
    case 39: {
        yyval.tab = new Table(*yyvsp[0].namelist);
        // delete $1;
        ;
        break;
    }
    case 40: {
        yyval.tab = new OpTable(*yyvsp[0].namelist);
        // delete $1;
        ;
        break;
    }
    case 41: {
        yyval.tab = new ExprTable(*yyvsp[0].exprlist);
        // delete $1;
        ;
        break;
    }
    case 42: {
        // cross-product of two str_expr's
        std::deque<QString>::iterator i, j;
        yyval.namelist = new std::deque<QString>;
        for (i = yyvsp[-1].namelist->begin(); i != yyvsp[-1].namelist->end(); i++)
            for (j = yyvsp[0].namelist->begin(); j != yyvsp[0].namelist->end(); j++)
                yyval.namelist->push_back((*i) + (*j));
        // delete $1;
        // delete $2;
        ;
        break;
    }
    case 43: {
        yyval.namelist = yyvsp[0].namelist;
        ;
        break;
    }
    case 44: {
        // want to append $3 to $1
        // The following causes a massive warning message about mixing signed and unsigned
        yyvsp[-2].namelist->insert(yyvsp[-2].namelist->end(), yyvsp[0].namelist->begin(), yyvsp[0].namelist->end());
        // delete $3;
        yyval.namelist = yyvsp[-2].namelist;
        ;
        break;
    }
    case 45: {
        yyvsp[-3].namelist->push_back("");
        ;
        break;
    }
    case 46: {
        yyval.namelist = yyvsp[0].namelist;
        ;
        break;
    }
    case 47: {
        yyval.namelist = yyvsp[-1].namelist;
        ;
        break;
    }
    case 48: {
        yyval.namelist = yyvsp[0].namelist;
        ;
        break;
    }
    case 49: {
        yyval.namelist = new std::deque<QString>;
        yyval.namelist->push_back("");
        yyval.namelist->push_back(yyvsp[-1].str);
        ;
        break;
    }
    case 50: {
        yyval.namelist = new std::deque<QString>(1, yyvsp[-1].str);
        ;
        break;
    }
    case 51: {
        std::ostringstream o;
        // expand $2 from table of names
        if (TableDict.find(yyvsp[0].str) != TableDict.end())
            if (TableDict[yyvsp[0].str]->getType() == NAMETABLE)
                yyval.namelist = new std::deque<QString>(TableDict[yyvsp[0].str]->Records);
            else {
                yyerror(qPrintable(QString("name %1  is not a NAMETABLE.\n").arg(yyvsp[0].str)));
            }
        else {
            yyerror(qPrintable(QString("could not dereference name %1\n").arg(yyvsp[0].str)));
        };
        break;
    }
    case 52: {
        // try and expand $1 from table of names. if fail, expand using '"' NAME '"' rule
        if (TableDict.find(yyvsp[0].str) != TableDict.end())
            if (TableDict[yyvsp[0].str]->getType() == NAMETABLE)
                yyval.namelist = new std::deque<QString>(TableDict[yyvsp[0].str]->Records);
            else {
                yyerror(qPrintable(QString("name %1  is not a NAMETABLE.\n").arg(yyvsp[0].str)));
            }
        else {
            yyval.namelist = new std::deque<QString>;
            yyval.namelist->push_back(yyvsp[0].str);
        };
        break;
    }
    case 53: {
        yyval.str = yyvsp[0].str;
        ;
        break;
    }
    case 54: {
        yyval.str = yyvsp[0].str;
        ;
        break;
    }
    case 55: {
        yyval.str = yyvsp[0].str;
        ;
        break;
    }
    case 56: {
        yyval.namelist = yyvsp[-1].namelist;
        ;
        break;
    }
    case 57: {
        yyval.namelist = yyvsp[-4].namelist;
        yyval.namelist->push_back(yyvsp[-1].str);
        ;
        break;
    }
    case 58: {
        yyval.namelist = new std::deque<QString>;
        yyval.namelist->push_back(yyvsp[-1].str);
        ;
        break;
    }
    case 59: {
        yyval.exprlist = yyvsp[-1].exprlist;
        ;
        break;
    }
    case 60: {
        yyval.exprlist = yyvsp[-4].exprlist;
        yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
        break;
    }
    case 61: {
        yyval.exprlist = new std::deque<Exp *>;
        yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
        break;
    }
    case 62: {
        yyvsp[0].insel->getrefmap(indexrefmap);
        //       $3            $4
        ;
        break;
    }
    case 63: {
        // This function expands the tables and saves the expanded RTLs to the dictionary
        expandTables(yyvsp[-3].insel, yyvsp[-1].parmlist, yyvsp[0].rtlist, Dict);
        ;
        break;
    }
    case 64: {
        yyval.insel = yyvsp[0].insel;
        ;
        break;
    }
    case 65: {
        QString nm = yyvsp[0].str;

        if (nm.startsWith('^'))
            nm = nm.mid(1);

        // remove all " and _, from the decoration
        nm = nm.replace("\"","").replace(".","").replace("_","");

        yyval.insel = yyvsp[-1].insel;
        yyval.insel->append(std::make_shared<InsNameElem>(nm));
        ;
        break;
    }
    case 66: {
        yyval.insel = std::make_shared<InsNameElem>(yyvsp[0].str);
        ;
        break;
    }
    case 67: {
        yyval.insel = yyvsp[0].insel;
        ;
        break;
    }
    case 68: {
        yyval.insel = yyvsp[-1].insel;
        yyval.insel->append(yyvsp[0].insel);
        ;
        break;
    }
    case 69: {
        yyval.insel = std::make_shared<InsOptionElem>(yyvsp[-1].str);
        ;
        break;
    }
    case 70: {
        if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
            yyerror(qPrintable(QString("Table %1 has not been declared.\n").arg(yyvsp[-2].str)));
        } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->Records.size())) {
            yyerror(qPrintable(QString("Can't get element %1  of table %2\n").arg(yyvsp[-1].num).arg(yyvsp[-2].str)));
        } else
            yyval.insel = std::make_shared<InsNameElem>(TableDict[yyvsp[-2].str]->Records[yyvsp[-1].num]);
        ;
        break;
    }
    case 71: {
        if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
            yyerror(qPrintable(QString("Table %1 has not been declared.\n").arg(yyvsp[-2].str)));
        } else
            yyval.insel = std::make_shared<InsListElem>(yyvsp[-2].str, TableDict[yyvsp[-2].str], yyvsp[-1].str);
        ;
        break;
    }
    case 72: {
        if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
            yyerror(qPrintable(QString("Table %1 has not been declared.\n").arg(yyvsp[-2].str)));
        } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->Records.size())) {
            yyerror(qPrintable(QString("Can't get element %1  of table %2\n").arg(yyvsp[-1].num).arg(yyvsp[-2].str)));
        } else
            yyval.insel = std::make_shared<InsNameElem>(TableDict[yyvsp[-2].str]->Records[yyvsp[-1].num]);
        ;
        break;
    }
    case 73: {
        if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
            yyerror(qPrintable(QString("Table %1 has not been declared.\n").arg(yyvsp[-2].str)));
        } else
            yyval.insel = std::make_shared<InsListElem>(yyvsp[-2].str, TableDict[yyvsp[-2].str], yyvsp[-1].str);
        ;
        break;
    }
    case 74: {
        yyval.insel = std::make_shared<InsNameElem>(yyvsp[-1].str);
        ;
        break;
    }
    case 75: {
        // append any automatically generated register transfers and clear the list they were stored in.
        // Do nothing for a NOP (i.e. $2 = 0)
        if (yyvsp[0].regtransfer != nullptr) {
            yyvsp[-1].rtlist->appendStmt(yyvsp[0].regtransfer);
        }
        yyval.rtlist = yyvsp[-1].rtlist;
        ;
        break;
    }
    case 76: {
        yyval.rtlist = std::make_shared<RTL>(ADDRESS::g(0L)); // WARN: the code here was RTL(STMT_ASSIGN), which is not right, since RTL parameter is an address
        if (yyvsp[0].regtransfer != nullptr)
            yyval.rtlist->appendStmt(yyvsp[0].regtransfer);
        ;
        break;
    }
    case 77: {
        yyval.regtransfer = yyvsp[0].regtransfer;
        ;
        break;
    }
    case 78: {
        std::ostringstream o;
        if (Dict.FlagFuncs.find(yyvsp[-2].str) != Dict.FlagFuncs.end()) {
            // Note: SETFFLAGS assigns to the floating point flags. All others to the integer flags
            bool bFloat = yyvsp[-2].str=="SETFFLAGS";
            OPER op = bFloat ? opFflags : opFlags;
            yyval.regtransfer = new Assign(
                new Terminal(op), Binary::get(opFlagCall, Const::get(yyvsp[-2].str), listExpToExp(yyvsp[-1].explist)));
        } else {
            yyerror(qPrintable(yyvsp[-2].str+" is not declared as a flag function.\n"));
        };
        break;
    }
    case 79: {
        yyval.regtransfer = 0;
        ;
        break;
    }
    case 80: {
        yyval.regtransfer = 0;
        ;
        break;
    }
    case 81: {
        yyval.regtransfer = nullptr;
        ;
        break;
    }
    case 82: {
        // Not sure why the below is commented out (MVE)
        /*            Location* pFlag = Location::regOf(Dict.RegMap[$3]);
                        $1->push_back(pFlag);
                        $$ = $1;
*/ yyval.explist = 0;
        ;
        break;
    }
    case 83: {
        /*            std::list<Exp*>* tmp = new std::list<Exp*>;
                        Unary* pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
                        tmp->push_back(pFlag);
                        $$ = tmp;
*/ yyval.explist = 0;
        ;
        break;
    }
    case 84: {
        assert(yyvsp[0].str != 0);
        yyvsp[-2].parmlist->push_back(yyvsp[0].str);
        yyval.parmlist = yyvsp[-2].parmlist;
        ;
        break;
    }
    case 85: {
        yyval.parmlist = new std::list<QString>;
        yyval.parmlist->push_back(yyvsp[0].str);
        ;
        break;
    }
    case 86: {
        if(yyval.parmlist) {
            delete yyval.parmlist;
            yyval.parmlist = nullptr;
        }
        yyval.parmlist = new std::list<QString>;
        ;
        break;
    }
    case 87: {
        Dict.ParamSet.insert(yyvsp[0].str); // MVE: Likely wrong. Likely supposed to be OPERAND params only
        yyval.str = yyvsp[0].str;
        ;
        break;
    }
    case 88: {
        yyval.explist->push_back(yyvsp[0].exp);
        ;
        break;
    }
    case 89: {
        yyval.explist = new std::list<Exp *>;
        yyval.explist->push_back(yyvsp[0].exp);
        ;
        break;
    }
    case 90: {
        yyval.explist = new std::list<Exp *>;
        ;
        break;
    }
    case 91: {
        Assign *a = new Assign(yyvsp[-5].typ, yyvsp[-2].exp, yyvsp[0].exp);
        a->setGuard(yyvsp[-4].exp);
        yyval.regtransfer = a;
        ;
        break;
    }
    case 92: {
        // update the size of any generated RT's
        yyval.regtransfer = new Assign(yyvsp[-3].typ, yyvsp[-2].exp, yyvsp[0].exp);
        ;
        break;
    }
    case 93: {
        yyval.regtransfer = new Assign(new Terminal(opNil), new Terminal(opFpush));
        ;
        break;
    }
    case 94: {
        yyval.regtransfer = new Assign(new Terminal(opNil), new Terminal(opFpop));
        ;
        break;
    }
    case 95: {
        yyval.regtransfer = new Assign(yyvsp[-1].typ, nullptr, yyvsp[0].exp);
        ;
        break;
    }
    case 96: {
        yyval.exp = new Const(yyvsp[0].num);
        ;
        break;
    }
    case 97: {
        yyval.exp = new Const(yyvsp[0].dbl);
        ;
        break;
    }
    case 98: {
        yyval.exp = yyvsp[-1].exp;
        ;
        break;
    }
    case 99: {
        yyval.exp = yyvsp[0].exp;
        ;
        break;
    }
    case 100: {
        yyval.exp = new Ternary(opTern, yyvsp[-5].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
        break;
    }
    case 101: {
        yyval.exp = new Unary(opAddrOf, yyvsp[-1].exp);
        ;
        break;
    }
    case 102: {
        yyval.exp =
            new Ternary(strToOper(yyvsp[-6].str), new Const(yyvsp[-5].num), new Const(yyvsp[-3].num), yyvsp[-1].exp);
        ;
        break;
    }
    case 103: {
        yyval.exp = new Unary(opFtrunc, yyvsp[-1].exp);
        ;
        break;
    }
    case 104: {
        yyval.exp = new Unary(opFabs, yyvsp[-1].exp);
        ;
        break;
    }
    case 105: {
        yyval.exp = new Terminal(opFpush);
        ;
        break;
    }
    case 106: {
        yyval.exp = new Terminal(opFpop);
        ;
        break;
    }
    case 107: {
        yyval.exp = new Unary(strToOper(yyvsp[-2].str), yyvsp[-1].exp);
        ;
        break;
    }
    case 108: {
        std::ostringstream o;
        if (indexrefmap.find(yyvsp[-1].str) == indexrefmap.end()) {
            yyerror(qPrintable(QString("index  %1 not declared for use.\n").arg(yyvsp[-1].str)));
        } else if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
            yyerror(qPrintable(QString("table  %1 not declared for use.\n").arg(yyvsp[-2].str)));
        } else if (TableDict[yyvsp[-2].str]->getType() != EXPRTABLE) {
            yyerror(qPrintable(QString("table  %1  is not an expression table but appears to be used as one.\n").
                               arg(yyvsp[-2].str)));
        } else if (((ExprTable *)TableDict[yyvsp[-2].str])->expressions.size() <
                   indexrefmap[yyvsp[-1].str]->ntokens()) {
            yyerror(qPrintable(QString("table %1  (%2) is too small to use %3 (%4) as an index.\n").
                               arg(yyvsp[-2].str).
                    arg(((ExprTable *)TableDict[yyvsp[-2].str])->expressions.size()).
                    arg(yyvsp[-1].str).
                    arg(indexrefmap[yyvsp[-1].str]->ntokens())
                    ));
        }
        // $1 is a map from string to Table*; $2 is a map from string to InsNameElem*
        yyval.exp = Binary::get(opExpTable, Const::get(yyvsp[-2].str), Const::get(yyvsp[-1].str));
        ;
        break;
    }
    case 109: {
        std::ostringstream o;
        if (Dict.ParamSet.find(yyvsp[-2].str) != Dict.ParamSet.end()) {
            if (Dict.DetParamMap.find(yyvsp[-2].str) != Dict.DetParamMap.end()) {
                ParamEntry &param = Dict.DetParamMap[yyvsp[-2].str];
                if (yyvsp[-1].explist->size() != param.funcParams.size()) {
                    yyerror(qPrintable(QString("%1 requires %2  parameters, but received %3\n")
                                       .arg(yyvsp[-2].str)
                                    .arg(param.funcParams.size())
                            .arg(yyvsp[-1].explist->size())
                            ));
                } else {
                    // Everything checks out. *phew*
                    // Note: the below may not be right! (MVE)
                    yyval.exp = Binary::get(opFlagDef, Const::get(yyvsp[-2].str), listExpToExp(yyvsp[-1].explist));
                    // delete $2;            // Delete the list of char*s
                }
            } else {
                yyerror(qPrintable(QString("%1 is not defined as a OPERAND function.\n").arg(yyvsp[-2].str)));
            }
        } else {
            yyerror(qPrintable(QString("Unrecognized name %1 in lambda call.\n").arg(yyvsp[-2].str)));
        };
        break;
    }
    case 110: {
        yyval.exp = makeSuccessor(yyvsp[-1].exp);
        ;
        break;
    }
    case 111: {
        yyval.exp = new Unary(opSignExt, yyvsp[-1].exp);
        ;
        break;
    }
    case 112: {
        // size casts and the opSize operator were generally deprecated, but now opSize is used to transmit
        // the size of operands that could be memOfs from the decoder to type analysis
        if (yyvsp[0].num == STD_SIZE)
            yyval.exp = yyvsp[-1].exp;
        else
            yyval.exp = Binary::get(opSize, new Const(yyvsp[0].num), yyvsp[-1].exp);
        ;
        break;
    }
    case 113: {
        yyval.exp = new Unary(opNot, yyvsp[0].exp);
        ;
        break;
    }
    case 114: {
        yyval.exp = new Unary(opLNot, yyvsp[0].exp);
        ;
        break;
    }
    case 115: {
        yyval.exp = new Unary(opFNeg, yyvsp[0].exp);
        ;
        break;
    }
    case 116: {
        yyval.exp = Binary::get(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
        break;
    }
    case 117: {
        yyval.exp = Binary::get(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
        break;
    }
    case 118: {
        yyval.exp = Binary::get(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
        break;
    }
    case 119: {
        yyval.exp = Binary::get(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
        break;
    }
    case 120: {
        yyval.exp = Binary::get(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
        break;
    }
    case 121: {
        std::ostringstream o;
        if (indexrefmap.find(yyvsp[-2].str) == indexrefmap.end()) {
            yyerror(qPrintable(QString("table %1 not declared for use.\n").arg(yyvsp[-2].str)));
        } else if (TableDict.find(yyvsp[-3].str) == TableDict.end()) {
            yyerror(qPrintable(QString("table %1 not declared for use.\n").arg(yyvsp[-3].str)));
        } else if (TableDict[yyvsp[-3].str]->getType() != OPTABLE) {
            yyerror(qPrintable(QString("table %1 is not an operator table but appears to be used as one.\n")
                               .arg(yyvsp[-3].str)));
        } else if (TableDict[yyvsp[-3].str]->Records.size() < indexrefmap[yyvsp[-2].str]->ntokens()) {
            yyerror(qPrintable(QString("table %1 is too small to use with %2 as an index.\n")
                               .arg(yyvsp[-2].str)));
        }
        yyval.exp =
            new Ternary(opOpTable, Const::get(yyvsp[-3].str), Const::get(yyvsp[-2].str),
                        Binary::get(opList, yyvsp[-4].exp, Binary::get(opList, yyvsp[0].exp, new Terminal(opNil))));
        ;
        break;
    }
    case 122: {
        yyval.exp = yyvsp[0].exp;
        ;
        break;
    }
    case 123: {
        bool isFlag = yyvsp[0].str.contains("flags");
        std::map<QString, int>::const_iterator it = Dict.RegMap.find(yyvsp[0].str);
        if (it == Dict.RegMap.end() && !isFlag) {
            std::ostringstream ost;
            yyerror(qPrintable(QString("register `%1' is undefined\n").arg(yyvsp[0].str)));
        } else if (isFlag || it->second == -1) {
            // A special register, e.g. %npc or %CF. Return a Terminal for it
            OPER op = strToTerm(yyvsp[0].str);
            if (op) {
                yyval.exp = new Terminal(op);
            } else {
                yyval.exp = new Unary(
                            opMachFtr, // Machine specific feature
                                      Const::get(yyvsp[0].str));
            }
        } else {
            // A register with a constant reg nmber, e.g. %g2.  In this case, we want to return r[const 2]
            yyval.exp = Location::regOf(it->second);
        };
        break;
    }
    case 124: {
        yyval.exp = Location::regOf(yyvsp[-1].exp);
        ;
        break;
    }
    case 125: {
        bool ok=false;
        int regNum = yyvsp[0].str.midRef(1).toInt(&ok);
        assert(ok);
        yyval.exp = Location::regOf(regNum);
        ;
        break;
    }
    case 126: {
        yyval.exp = Location::memOf(yyvsp[-1].exp);
        ;
        break;
    }
    case 127: {
        // This is a mixture of the param: PARM {} match and the value_op: NAME {} match
        Exp *s;
        std::set<QString>::iterator it = Dict.ParamSet.find(yyvsp[0].str);
        if (it != Dict.ParamSet.end()) {
            s = new Location(opParam, Const::get(yyvsp[0].str), nullptr);
        } else if (ConstTable.find(yyvsp[0].str) != ConstTable.end()) {
            s = new Const(ConstTable[yyvsp[0].str]);
        } else {
            yyerror(qPrintable(QString("`%1' is not a constant, definition or a parameter of this instruction\n")
                               .arg(yyvsp[0].str)));

            s = Const::get(0);
        }
        yyval.exp = s;
        ;
        break;
    }
    case 128: {
        yyval.exp = new Ternary(opAt, yyvsp[-6].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
        break;
    }
    case 129: {
        yyval.exp = Location::tempOf(new Const(yyvsp[0].str));
        ;
        break;
    }
    case 130: {
        yyval.exp = new Unary(opPostVar, yyvsp[-1].exp);
        ;
        break;
    }
    case 131: {
        yyval.exp = makeSuccessor(yyvsp[-1].exp);
        ;
        break;
    }
    case 132: {
        yyval.num = yyvsp[-1].num;
        ;
        break;
    }
    case 133: {
        Dict.bigEndian = yyvsp[0].str=="BIG";
        ;
        break;
    }
    case 134: {
        yyval.str = yyvsp[0].str;
        ;
        break;
    }
    case 135: {
        yyval.str = yyvsp[0].str;
        ;
        break;
    }
    case 136: {
        char c = yyvsp[0].str[1].toLatin1();
        if (c == '*')
            yyval.typ = SizeType::get(0); // MVE: should remove these
        else if (isdigit(c)) {
            // Skip star (hence midRef)
            yyval.typ = SizeType::get(atoi(qPrintable(yyvsp[0].str.mid(1))));
        } else {
            int size;
            // Skip star and letter
            size = atoi(qPrintable(yyvsp[0].str.mid(2)));
            if (size == 0)
                size = STD_SIZE;
            switch (c) {
            case 'i':
                yyval.typ = IntegerType::get(size, 1);
                break;
            case 'j':
                yyval.typ = IntegerType::get(size, 0);
                break;
            case 'u':
                yyval.typ = IntegerType::get(size, -1);
                break;
            case 'f':
                yyval.typ = FloatType::get(size);
                break;
            case 'c':
                yyval.typ = CharType::get();
                break;
            default:
                LOG_STREAM() << "Unexpected char " << c << " in assign type\n";
                yyval.typ = IntegerType::get(32);
            }
        };
        break;
    }
    case 140: {
        Dict.fastMap[yyvsp[-2].str] = QString(yyvsp[0].str);
        ;
        break;
    }
    }

    /* the action file gets copied in in place of this dollarsign  */
    yyvsp -= yylen;
    yyssp -= yylen;

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG) {
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

    goto yynewstate;

    YYLABEL(yyerrlab) /* here on detecting error */

    if (!yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
        ++yynerrs;

#ifdef YY_SSLParser_ERROR_VERBOSE
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
                yyerror((char *)"parse error; also virtual memory exceeded");
        } else
#endif /* YY_SSLParser_ERROR_VERBOSE */
            yyerror((char *)"parse error");
    }

    goto yyerrlab1;
    YYLABEL(yyerrlab1) /* here on error raised explicitly by an action */

    if (yyerrstatus == 3) {
        /* if just tried and failed to reuse lookahead token after an error, discard it.  */

        /* return failure if at end of input */
        if (yychar == YYEOF)
            YYABORT;

#if YY_SSLParser_DEBUG != 0
        if (YY_SSLParser_DEBUG_FLAG)
            fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

        yychar = YYEMPTY;
    }

    /* Else will try to reuse lookahead token
     after shifting the error token.  */

    yyerrstatus = 3; /* Each real token shifted decrements this */

    goto yyerrhandle;

    YYLABEL(yyerrdefault) /* current state does not do anything special for the error token. */

#if 0
            /* This is wrong; only states that explicitly want error tokens
                             should shift them.  */
            yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
    if (yyn) goto yydefault;
#endif

    YYLABEL(yyerrpop) /* pop the current state because it cannot handle the error token */

    if (yyssp == yyss)
        YYABORT;
    yyvsp--;
    yystate = *--yyssp;
#ifdef YY_SSLParser_LSP_NEEDED
    yylsp--;
#endif

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG) {
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
        goto yyerrdefault;

    yyn += YYTERROR;
    if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
        goto yyerrdefault;

    yyn = yytable[yyn];
    if (yyn < 0) {
        if (yyn == YYFLAG)
            goto yyerrpop;
        yyn = -yyn;
        goto yyreduce;
    } else if (yyn == 0)
        goto yyerrpop;

    if (yyn == YYFINAL)
        YYACCEPT;

#if YY_SSLParser_DEBUG != 0
    if (YY_SSLParser_DEBUG_FLAG)
        fprintf(stderr, "Shifting error token, ");
#endif

    *++yyvsp = yylval;
#ifdef YY_SSLParser_LSP_NEEDED
    *++yylsp = yylloc;
#endif

    yystate = yyn;
    goto yynewstate;
    /* end loop, in which YYGOTO may be used. */
}

/* END */
