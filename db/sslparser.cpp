#define YY_SSLParser_h_included

/*  A Bison++ parser, made from sslparser.y  */

 /* with Bison++ version bison++ Version 1.21-8, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/usr/local/lib/bison.cc"
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
#if defined( _MSDOS ) || defined(MSDOS) || defined(__MSDOS__) 
#define __MSDOS_AND_ALIKE
#endif
#if defined(_WINDOWS) && defined(_MSC_VER)
#define __HAVE_NO_ALLOCA
#define __MSDOS_AND_ALIKE
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (__MSDOS_AND_ALIKE)
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
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */

#endif /* not _AIX  not MSDOS, or __TURBOC__ or _AIX, not sparc.  */
#endif /* alloca not defined.  */
#ifdef c_plusplus
#ifndef __cplusplus
#define __cplusplus
#endif
#endif
#ifdef __cplusplus
#ifndef YY_USE_CLASS
#define YY_USE_CLASS
#endif
#else
#ifndef __STDC__
#define const
#endif
#endif
#include <stdio.h>
#define YYBISON 1  

/* #line 73 "/usr/local/lib/bison.cc" */
#line 85 "sslparser.cpp"
#line 34 "sslparser.y"

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <sstream>
#include "types.h"
#include "rtl.h"
#include "table.h"
#include "insnameelem.h"
#include "util.h"           // E.g. str()

#ifdef WIN32
#include <malloc.h>
#endif

class SSLScanner;

#line 59 "sslparser.y"
typedef union {
    Exp*            exp;
    char*           str;
    int             num;
    double          dbl;
    Exp*            regtransfer;
    
    Table*          tab;
    InsNameElem*    insel;
    std::list<std::string>*   parmlist;
    std::list<std::string>*   strlist;
    std::deque<Exp*>*    exprlist;
    std::deque<std::string>*  namelist;
    std::list<Exp*>*     explist;
    RTL*            rtlist;
} yy_SSLParser_stype;
#define YY_SSLParser_STYPE yy_SSLParser_stype
#line 77 "sslparser.y"

#include "sslscanner.h"
OPER strToTerm(char* s);        // Convert string to a Terminal (if possible)
Exp* listExpToExp(std::list<Exp*>* le);  // Convert a STL list of Exp* to opList
Exp* listStrToExp(std::list<std::string>* ls);// Convert a STL list of strings to opList
#define STD_SIZE    32          // Standard size
#define YY_SSLParser_DEBUG  1 
#define YY_SSLParser_PARSE_PARAM  \
    RTLInstDict& Dict
#define YY_SSLParser_CONSTRUCTOR_PARAM  \
    const std::string& sslFile, \
    bool trace
#define YY_SSLParser_CONSTRUCTOR_INIT  : \
   sslFile(sslFile), bFloat(false)
#define YY_SSLParser_CONSTRUCTOR_CODE  \
    std::fstream *fin = new std::fstream(sslFile.c_str(), std::ios::in); \
    theScanner = NULL; \
    if (!*fin) { \
        std::cerr << "can't open `" << sslFile << "' for reading\n"; \
	return; \
    } \
    theScanner = new SSLScanner(*fin, trace); \
    if (trace) yydebug = 1;
#define YY_SSLParser_MEMBERS  \
public: \
        SSLParser(std::istream &in, bool trace); \
        virtual ~SSLParser(); \
OPER    strToOper(const char*s); /* Convert string to an operator */ \
static  Exp* parseExp(const char *str); /* Parse an expression from a string */ \
/* The code for expanding tables and saving to the dictionary */ \
void    expandTables(InsNameElem* iname, std::list<std::string>* params, RTL* o_rtlist, \
  RTLInstDict& Dict); \
\
    /* \
     * The scanner. \
     */ \
    SSLScanner* theScanner; \
protected: \
\
    /* \
     * The file from which the SSL spec is read. \
     */ \
    std::string sslFile; \
\
    /* \
     * Result for parsing an expression. \
     */ \
    Exp *the_exp; \
\
    /* \
     * Maps SSL constants to their values. \
     */ \
    std::map<std::string,int> ConstTable; \
\
    /* \
     * maps index names to instruction name-elements \
     */ \
    std::map<std::string, InsNameElem*> indexrefmap; \
\
    /* \
     * Maps table names to Table's.\
     */ \
    std::map<std::string, Table*> TableDict; \
\
    /* \
     * True when FLOAT keyword seen; false when INTEGER keyword seen \
     * (in @REGISTER section) \
     */ \
    bool bFloat;

#line 73 "/usr/local/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_SSLParser_BISON 1
#ifndef YY_SSLParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_SSLParser_COMPATIBILITY 1
#else
#define  YY_SSLParser_COMPATIBILITY 0
#endif
#endif

#if YY_SSLParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_SSLParser_LTYPE
#define YY_SSLParser_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_SSLParser_STYPE 
#define YY_SSLParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_SSLParser_DEBUG
#define  YY_SSLParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_SSLParser_STYPE
#ifndef yystype
#define yystype YY_SSLParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_SSLParser_USE_GOTO
#define YY_SSLParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_SSLParser_USE_GOTO
#define YY_SSLParser_USE_GOTO 0
#endif

#ifndef YY_SSLParser_PURE

/* #line 117 "/usr/local/lib/bison.cc" */
#line 242 "sslparser.cpp"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_SSLParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 251 "sslparser.cpp"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_SSLParser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 258 "sslparser.cpp"

#line 123 "/usr/local/lib/bison.cc"
/* YY_SSLParser_DEBUG */
#endif


#ifndef YY_SSLParser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 268 "sslparser.cpp"

#line 128 "/usr/local/lib/bison.cc"
 /* YY_SSLParser_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_SSLParser_LSP_NEEDED
#ifndef YY_SSLParser_LTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YY_SSLParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_SSLParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_SSLParser_STYPE
#define YY_SSLParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_SSLParser_PARSE
#define YY_SSLParser_PARSE yyparse
#endif
#ifndef YY_SSLParser_LEX
#define YY_SSLParser_LEX yylex
#endif
#ifndef YY_SSLParser_LVAL
#define YY_SSLParser_LVAL yylval
#endif
#ifndef YY_SSLParser_LLOC
#define YY_SSLParser_LLOC yylloc
#endif
#ifndef YY_SSLParser_CHAR
#define YY_SSLParser_CHAR yychar
#endif
#ifndef YY_SSLParser_NERRS
#define YY_SSLParser_NERRS yynerrs
#endif
#ifndef YY_SSLParser_DEBUG_FLAG
#define YY_SSLParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_SSLParser_ERROR
#define YY_SSLParser_ERROR yyerror
#endif
#ifndef YY_SSLParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_SSLParser_PARSE_PARAM
#ifndef YY_SSLParser_PARSE_PARAM_DEF
#define YY_SSLParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_SSLParser_PARSE_PARAM
#define YY_SSLParser_PARSE_PARAM void
#endif
#endif
#if YY_SSLParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_SSLParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_SSLParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_SSLParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_SSLParser_PURE
#ifndef YYPURE
#define YYPURE YY_SSLParser_PURE
#endif
#endif
#ifdef YY_SSLParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_SSLParser_DEBUG 
#endif
#endif
#ifndef YY_SSLParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_SSLParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_SSLParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_SSLParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 236 "/usr/local/lib/bison.cc" */
#line 381 "sslparser.cpp"
#define	COND_OP	258
#define	BIT_OP	259
#define	ARITH_OP	260
#define	LOG_OP	261
#define	NAME	262
#define	REG_ID	263
#define	COND_TNAME	264
#define	DECOR	265
#define	FARITH_OP	266
#define	FPUSH	267
#define	FPOP	268
#define	TEMP	269
#define	SHARES	270
#define	CONV_FUNC	271
#define	TRANSCEND	272
#define	BIG	273
#define	LITTLE	274
#define	NAME_CALL	275
#define	NAME_LOOKUP	276
#define	ENDIANNESS	277
#define	COVERS	278
#define	INDEX	279
#define	NOT	280
#define	THEN	281
#define	LOOKUP_RDC	282
#define	BOGUS	283
#define	ASSIGN	284
#define	TO	285
#define	COLON	286
#define	S_E	287
#define	AT	288
#define	ADDR	289
#define	REG_IDX	290
#define	EQUATE	291
#define	MEM_IDX	292
#define	TOK_INTEGER	293
#define	TOK_FLOAT	294
#define	FAST	295
#define	OPERAND	296
#define	FETCHEXEC	297
#define	CAST_OP	298
#define	FLAGMACRO	299
#define	NUM	300
#define	ASSIGNSIZE	301
#define	FLOATNUM	302


#line 236 "/usr/local/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_SSLParser_CLASS
#define YY_SSLParser_CLASS SSLParser
#endif
#ifndef YY_SSLParser_INHERIT
#define YY_SSLParser_INHERIT
#endif
#ifndef YY_SSLParser_MEMBERS
#define YY_SSLParser_MEMBERS 
#endif
#ifndef YY_SSLParser_LEX_BODY
#define YY_SSLParser_LEX_BODY  
#endif
#ifndef YY_SSLParser_ERROR_BODY
#define YY_SSLParser_ERROR_BODY  
#endif
#ifndef YY_SSLParser_CONSTRUCTOR_PARAM
#define YY_SSLParser_CONSTRUCTOR_PARAM
#endif
#ifndef YY_SSLParser_CONSTRUCTOR_CODE
#define YY_SSLParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_SSLParser_CONSTRUCTOR_INIT
#define YY_SSLParser_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_SSLParser_USE_CONST_TOKEN
#define YY_SSLParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_SSLParser_USE_CONST_TOKEN != 0
#ifndef YY_SSLParser_ENUM_TOKEN
#define YY_SSLParser_ENUM_TOKEN yy_SSLParser_enum_token
#endif
#endif

class YY_SSLParser_CLASS YY_SSLParser_INHERIT
{
public: 
#if YY_SSLParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 280 "/usr/local/lib/bison.cc" */
#line 476 "sslparser.cpp"
static const int COND_OP;
static const int BIT_OP;
static const int ARITH_OP;
static const int LOG_OP;
static const int NAME;
static const int REG_ID;
static const int COND_TNAME;
static const int DECOR;
static const int FARITH_OP;
static const int FPUSH;
static const int FPOP;
static const int TEMP;
static const int SHARES;
static const int CONV_FUNC;
static const int TRANSCEND;
static const int BIG;
static const int LITTLE;
static const int NAME_CALL;
static const int NAME_LOOKUP;
static const int ENDIANNESS;
static const int COVERS;
static const int INDEX;
static const int NOT;
static const int THEN;
static const int LOOKUP_RDC;
static const int BOGUS;
static const int ASSIGN;
static const int TO;
static const int COLON;
static const int S_E;
static const int AT;
static const int ADDR;
static const int REG_IDX;
static const int EQUATE;
static const int MEM_IDX;
static const int TOK_INTEGER;
static const int TOK_FLOAT;
static const int FAST;
static const int OPERAND;
static const int FETCHEXEC;
static const int CAST_OP;
static const int FLAGMACRO;
static const int NUM;
static const int ASSIGNSIZE;
static const int FLOATNUM;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 530 "sslparser.cpp"
	,COND_OP=258
	,BIT_OP=259
	,ARITH_OP=260
	,LOG_OP=261
	,NAME=262
	,REG_ID=263
	,COND_TNAME=264
	,DECOR=265
	,FARITH_OP=266
	,FPUSH=267
	,FPOP=268
	,TEMP=269
	,SHARES=270
	,CONV_FUNC=271
	,TRANSCEND=272
	,BIG=273
	,LITTLE=274
	,NAME_CALL=275
	,NAME_LOOKUP=276
	,ENDIANNESS=277
	,COVERS=278
	,INDEX=279
	,NOT=280
	,THEN=281
	,LOOKUP_RDC=282
	,BOGUS=283
	,ASSIGN=284
	,TO=285
	,COLON=286
	,S_E=287
	,AT=288
	,ADDR=289
	,REG_IDX=290
	,EQUATE=291
	,MEM_IDX=292
	,TOK_INTEGER=293
	,TOK_FLOAT=294
	,FAST=295
	,OPERAND=296
	,FETCHEXEC=297
	,CAST_OP=298
	,FLAGMACRO=299
	,NUM=300
	,ASSIGNSIZE=301
	,FLOATNUM=302


#line 283 "/usr/local/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_SSLParser_PARSE (YY_SSLParser_PARSE_PARAM);
 virtual void YY_SSLParser_ERROR(char *msg) YY_SSLParser_ERROR_BODY;
#ifdef YY_SSLParser_PURE
#ifdef YY_SSLParser_LSP_NEEDED
 virtual int  YY_SSLParser_LEX (YY_SSLParser_STYPE *YY_SSLParser_LVAL,YY_SSLParser_LTYPE *YY_SSLParser_LLOC) YY_SSLParser_LEX_BODY;
#else
 virtual int  YY_SSLParser_LEX (YY_SSLParser_STYPE *YY_SSLParser_LVAL) YY_SSLParser_LEX_BODY;
#endif
#else
 virtual int YY_SSLParser_LEX() YY_SSLParser_LEX_BODY;
 YY_SSLParser_STYPE YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
 YY_SSLParser_LTYPE YY_SSLParser_LLOC;
#endif
 int   YY_SSLParser_NERRS;
 int    YY_SSLParser_CHAR;
#endif
#if YY_SSLParser_DEBUG != 0
 int YY_SSLParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_SSLParser_CLASS(YY_SSLParser_CONSTRUCTOR_PARAM);
public:
 YY_SSLParser_MEMBERS 
};
/* other declare folow */
#if YY_SSLParser_USE_CONST_TOKEN != 0

/* #line 314 "/usr/local/lib/bison.cc" */
#line 612 "sslparser.cpp"
const int YY_SSLParser_CLASS::COND_OP=258;
const int YY_SSLParser_CLASS::BIT_OP=259;
const int YY_SSLParser_CLASS::ARITH_OP=260;
const int YY_SSLParser_CLASS::LOG_OP=261;
const int YY_SSLParser_CLASS::NAME=262;
const int YY_SSLParser_CLASS::REG_ID=263;
const int YY_SSLParser_CLASS::COND_TNAME=264;
const int YY_SSLParser_CLASS::DECOR=265;
const int YY_SSLParser_CLASS::FARITH_OP=266;
const int YY_SSLParser_CLASS::FPUSH=267;
const int YY_SSLParser_CLASS::FPOP=268;
const int YY_SSLParser_CLASS::TEMP=269;
const int YY_SSLParser_CLASS::SHARES=270;
const int YY_SSLParser_CLASS::CONV_FUNC=271;
const int YY_SSLParser_CLASS::TRANSCEND=272;
const int YY_SSLParser_CLASS::BIG=273;
const int YY_SSLParser_CLASS::LITTLE=274;
const int YY_SSLParser_CLASS::NAME_CALL=275;
const int YY_SSLParser_CLASS::NAME_LOOKUP=276;
const int YY_SSLParser_CLASS::ENDIANNESS=277;
const int YY_SSLParser_CLASS::COVERS=278;
const int YY_SSLParser_CLASS::INDEX=279;
const int YY_SSLParser_CLASS::NOT=280;
const int YY_SSLParser_CLASS::THEN=281;
const int YY_SSLParser_CLASS::LOOKUP_RDC=282;
const int YY_SSLParser_CLASS::BOGUS=283;
const int YY_SSLParser_CLASS::ASSIGN=284;
const int YY_SSLParser_CLASS::TO=285;
const int YY_SSLParser_CLASS::COLON=286;
const int YY_SSLParser_CLASS::S_E=287;
const int YY_SSLParser_CLASS::AT=288;
const int YY_SSLParser_CLASS::ADDR=289;
const int YY_SSLParser_CLASS::REG_IDX=290;
const int YY_SSLParser_CLASS::EQUATE=291;
const int YY_SSLParser_CLASS::MEM_IDX=292;
const int YY_SSLParser_CLASS::TOK_INTEGER=293;
const int YY_SSLParser_CLASS::TOK_FLOAT=294;
const int YY_SSLParser_CLASS::FAST=295;
const int YY_SSLParser_CLASS::OPERAND=296;
const int YY_SSLParser_CLASS::FETCHEXEC=297;
const int YY_SSLParser_CLASS::CAST_OP=298;
const int YY_SSLParser_CLASS::FLAGMACRO=299;
const int YY_SSLParser_CLASS::NUM=300;
const int YY_SSLParser_CLASS::ASSIGNSIZE=301;
const int YY_SSLParser_CLASS::FLOATNUM=302;


#line 314 "/usr/local/lib/bison.cc"
 /* const YY_SSLParser_CLASS::token */
#endif
/*apres const  */
YY_SSLParser_CLASS::YY_SSLParser_CLASS(YY_SSLParser_CONSTRUCTOR_PARAM) YY_SSLParser_CONSTRUCTOR_INIT
{
#if YY_SSLParser_DEBUG != 0
YY_SSLParser_DEBUG_FLAG=0;
#endif
YY_SSLParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 325 "/usr/local/lib/bison.cc" */
#line 674 "sslparser.cpp"


#define	YYFINAL		286
#define	YYFLAG		-32768
#define	YYNTBASE	61

#define YYTRANSLATE(x) ((unsigned)(x) <= 302 ? yytranslate[x] : 107)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    55,     2,    57,     2,     2,    56,    59,
    54,     2,     2,    49,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    48,     2,
     2,     2,    60,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    52,     2,    53,     2,    58,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    50,     2,    51,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47
};

#if YY_SSLParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     4,     6,    10,    13,    15,    18,    20,    22,
    24,    26,    28,    30,    33,    37,    39,    45,    51,    55,
    56,    57,    61,    62,    66,    70,    72,    76,    83,    94,
   109,   120,   129,   133,   135,   142,   146,   152,   156,   158,
   160,   162,   165,   167,   171,   176,   178,   182,   184,   188,
   192,   195,   197,   199,   201,   203,   207,   213,   217,   221,
   227,   231,   232,   237,   239,   242,   244,   246,   249,   253,
   257,   261,   266,   271,   275,   278,   280,   282,   286,   290,
   293,   295,   299,   301,   305,   307,   308,   310,   314,   316,
   317,   324,   329,   331,   333,   336,   338,   340,   342,   346,
   348,   356,   365,   369,   377,   379,   381,   385,   389,   393,
   396,   399,   402,   406,   410,   414,   418,   422,   428,   430,
   432,   436,   440,   442,   450,   453,   457,   460,   462,   464,
   467,   471,   473
};

static const short yyrhs[] = {    97,
     0,    99,     0,    62,     0,    62,    63,    48,     0,    63,
    48,     0,    86,     0,    42,    91,     0,    74,     0,    75,
     0,   102,     0,   104,     0,    67,     0,    73,     0,    41,
    64,     0,    64,    49,    65,     0,    65,     0,    95,    36,
    50,    94,    51,     0,    95,    94,    66,    46,    99,     0,
    52,    94,    53,     0,     0,     0,    38,    68,    70,     0,
     0,    39,    69,    70,     0,    70,    49,    71,     0,    71,
     0,     8,    24,    45,     0,     8,    52,    45,    53,    24,
    45,     0,     8,    52,    45,    53,    24,    45,    23,     8,
    30,     8,     0,     8,    52,    45,    53,    24,    45,    15,
     8,    33,    52,    45,    30,    45,    53,     0,    52,    72,
    53,    52,    45,    53,    24,    45,    30,    45,     0,    52,
    72,    53,    52,    45,    53,    24,    45,     0,    72,    49,
     8,     0,     8,     0,    20,    94,    54,    50,    91,    51,
     0,     7,    36,    45,     0,     7,    36,    45,     5,    45,
     0,     7,    36,    76,     0,    77,     0,    82,     0,    84,
     0,    77,    79,     0,    79,     0,    78,    49,    77,     0,
    78,    49,    55,    55,     0,    77,     0,    50,    78,    51,
     0,    80,     0,    56,     7,    56,     0,    55,     7,    55,
     0,    57,     7,     0,     7,     0,     4,     0,     5,     0,
    11,     0,    50,    83,    51,     0,    83,    49,    55,    81,
    55,     0,    55,    81,    55,     0,    50,    85,    51,     0,
    85,    49,    55,    99,    55,     0,    55,    99,    55,     0,
     0,    88,    87,    94,    91,     0,    89,     0,    88,    10,
     0,     7,     0,    90,     0,    89,    90,     0,    56,     7,
    56,     0,    21,    45,    53,     0,    21,     7,    53,     0,
    57,    21,    45,    53,     0,    57,    21,     7,    53,     0,
    55,     7,    55,     0,    91,    92,     0,    92,     0,    97,
     0,    20,    96,    54,     0,    44,    93,    54,     0,    44,
    54,     0,    58,     0,    93,    49,     8,     0,     8,     0,
    94,    49,    95,     0,    95,     0,     0,     7,     0,    96,
    49,    99,     0,    99,     0,     0,    46,    99,    26,   100,
    36,    99,     0,    46,   100,    36,    99,     0,    12,     0,
    13,     0,    46,    99,     0,    45,     0,    47,     0,    14,
     0,    59,    99,    54,     0,   100,     0,    52,    99,    60,
    99,    31,    99,    53,     0,    52,    99,    60,    99,    31,
    99,    53,   101,     0,    34,    99,    54,     0,    16,    45,
    49,    45,    49,    99,    54,     0,    12,     0,    13,     0,
    17,    99,    54,     0,    21,     7,    53,     0,    20,    96,
    54,     0,    99,    32,     0,    99,   101,     0,    25,    99,
     0,    99,    11,    99,     0,    99,     5,    99,     0,    99,
     4,    99,     0,    99,     3,    99,     0,    99,     6,    99,
     0,    99,    21,     7,    53,    98,     0,    98,     0,     8,
     0,    35,    99,    53,     0,    37,    99,    53,     0,     7,
     0,    99,    33,    52,    99,    31,    99,    53,     0,   100,
    56,     0,    50,    45,    51,     0,    22,   103,     0,    18,
     0,    19,     0,    40,   105,     0,   105,    49,   106,     0,
   106,     0,     7,    24,     7,     0
};

#endif

#if YY_SSLParser_DEBUG != 0
static const short yyrline[] = { 0,
   208,   212,   215,   218,   220,   223,   226,   231,   233,   236,
   241,   244,   248,   251,   255,   257,   260,   278,   295,   296,
   299,   303,   303,   306,   308,   309,   312,   318,   323,   358,
   380,   395,   406,   410,   417,   425,   432,   445,   451,   457,
   461,   467,   478,   483,   492,   495,   500,   504,   509,   515,
   518,   533,   551,   556,   560,   566,   572,   578,   585,   591,
   597,   603,   608,   615,   619,   642,   646,   649,   655,   659,
   672,   681,   692,   701,   706,   716,   723,   730,   741,   745,
   748,   753,   761,   771,   778,   782,   787,   792,   797,   802,
   807,   816,   822,   825,   829,   834,   839,   843,   847,   851,
   855,   859,   869,   875,   880,   883,   888,   894,   922,   950,
   960,   968,   972,   976,   980,   984,   988,   995,  1021,  1026,
  1056,  1060,  1064,  1082,  1086,  1091,  1097,  1102,  1106,  1113,
  1117,  1120,  1124
};

static const char * const yytname[] = {   "$","error","$illegal.","COND_OP",
"BIT_OP","ARITH_OP","LOG_OP","NAME","REG_ID","COND_TNAME","DECOR","FARITH_OP",
"FPUSH","FPOP","TEMP","SHARES","CONV_FUNC","TRANSCEND","BIG","LITTLE","NAME_CALL",
"NAME_LOOKUP","ENDIANNESS","COVERS","INDEX","NOT","THEN","LOOKUP_RDC","BOGUS",
"ASSIGN","TO","COLON","S_E","AT","ADDR","REG_IDX","EQUATE","MEM_IDX","TOK_INTEGER",
"TOK_FLOAT","FAST","OPERAND","FETCHEXEC","CAST_OP","FLAGMACRO","NUM","ASSIGNSIZE",
"FLOATNUM","';'","','","'{'","'}'","'['","']'","')'","'\"'","'\\''","'$'","'_'",
"'('","'?'","specorexp","specification","parts","operandlist","operand","func_parameter",
"reglist","@1","@2","a_reglists","a_reglist","reg_table","flag_fnc","constants",
"table_assign","table_expr","str_expr","str_array","str_term","name_expand",
"bin_oper","opstr_expr","opstr_array","exprstr_expr","exprstr_array","instr",
"@3","instr_name","instr_elem","name_contract","rt_list","rt","flag_list","list_parameter",
"param","list_actualparameter","assign_rt","exp_term","exp","var_op","cast",
"endianness","esize","fastlist","fastentries","fastentry",""
};
#endif

static const short yyr1[] = {     0,
    61,    61,    61,    62,    62,    63,    63,    63,    63,    63,
    63,    63,    63,    63,    64,    64,    65,    65,    66,    66,
    68,    67,    69,    67,    70,    70,    71,    71,    71,    71,
    71,    71,    72,    72,    73,    74,    74,    75,    76,    76,
    76,    77,    77,    78,    78,    78,    79,    79,    80,    80,
    80,    80,    81,    81,    81,    82,    83,    83,    84,    85,
    85,    87,    86,    88,    88,    89,    89,    89,    90,    90,
    90,    90,    90,    90,    91,    91,    92,    92,    92,    92,
    92,    93,    93,    94,    94,    94,    95,    96,    96,    96,
    97,    97,    97,    97,    97,    98,    98,    98,    98,    98,
    98,    98,    98,    98,    98,    98,    98,    98,    98,    99,
    99,    99,    99,    99,    99,    99,    99,    99,    99,   100,
   100,   100,   100,   100,   100,   101,   102,   103,   103,   104,
   105,   105,   106
};

static const short yyr2[] = {     0,
     1,     1,     1,     3,     2,     1,     2,     1,     1,     1,
     1,     1,     1,     2,     3,     1,     5,     5,     3,     0,
     0,     3,     0,     3,     3,     1,     3,     6,    10,    14,
    10,     8,     3,     1,     6,     3,     5,     3,     1,     1,
     1,     2,     1,     3,     4,     1,     3,     1,     3,     3,
     2,     1,     1,     1,     1,     3,     5,     3,     3,     5,
     3,     0,     4,     1,     2,     1,     1,     2,     3,     3,
     3,     4,     4,     3,     2,     1,     1,     3,     3,     2,
     1,     3,     1,     3,     1,     0,     1,     3,     1,     0,
     6,     4,     1,     1,     2,     1,     1,     1,     3,     1,
     7,     8,     3,     7,     1,     1,     3,     3,     3,     2,
     2,     2,     3,     3,     3,     3,     3,     5,     1,     1,
     3,     3,     1,     7,     2,     3,     2,     1,     1,     2,
     3,     1,     3
};

static const short yydefact[] = {     0,
    66,   120,   105,   106,    98,     0,     0,    86,     0,     0,
     0,     0,     0,     0,    21,    23,     0,     0,     0,    96,
     0,    97,     0,     0,     0,     0,     0,     3,     0,    12,
    13,     8,     9,     6,    62,    64,    67,     1,   119,     2,
   100,    10,    11,     0,     0,   123,   105,   106,    90,     0,
     0,   123,     0,    85,     0,    89,     0,     0,   128,   129,
   127,   112,     0,     0,     0,     0,     0,     0,   130,   132,
    87,    14,    16,    86,    93,    94,    90,     0,    81,     7,
    76,    77,    95,   100,     0,     0,     0,     0,     0,    66,
    86,     0,     0,     5,    65,    86,    68,     0,     0,     0,
     0,     0,     0,   110,     0,     0,   111,   125,    52,    36,
     0,     0,     0,     0,    38,    39,    43,    48,    40,    41,
     0,     0,   107,     0,     0,     0,   109,    71,    70,   103,
   121,   122,     0,     0,    22,    26,    24,     0,     0,     0,
     0,    20,     0,    83,    80,     0,    75,     0,     0,     0,
    74,    69,     0,     0,    99,     0,     4,     0,   116,   115,
   114,   117,   113,     0,     0,     0,     0,     0,     0,    46,
     0,     0,     0,     0,     0,    51,    42,     0,   108,    84,
     0,    88,     0,     0,    34,     0,     0,   133,   131,    15,
    86,    86,     0,    78,     0,    79,     0,   100,    92,     0,
    73,    72,    71,    63,     0,     0,   126,    37,    53,    54,
   123,    55,     0,     0,     0,    47,     0,    56,     0,    59,
    50,    49,     0,     0,    27,     0,     0,     0,    25,     0,
     0,     0,    82,     0,     0,   118,     0,    58,    61,     0,
    44,     0,     0,     0,    35,     0,    33,     0,    17,    19,
    18,    91,     0,     0,    45,     0,     0,   104,     0,     0,
   101,   124,    57,    60,    28,     0,   102,     0,     0,     0,
     0,     0,    32,     0,     0,     0,     0,    29,    31,     0,
     0,     0,    30,     0,     0,     0
};

static const short yydefgoto[] = {   284,
    28,    29,    72,    73,   193,    30,    66,    67,   135,   136,
   186,    31,    32,    33,   115,   170,   171,   117,   118,   213,
   119,   172,   120,   173,    34,    96,    35,    36,    37,    80,
    81,   146,    53,    54,    55,    82,    39,    56,    41,   107,
    42,    61,    43,    69,    70
};

static const short yypact[] = {   154,
    82,-32768,    47,    50,-32768,    35,   232,   273,    22,    56,
   232,   232,   232,   232,-32768,-32768,    76,   113,   218,-32768,
   232,-32768,   232,   132,   148,   135,   232,   372,    61,-32768,
-32768,-32768,-32768,-32768,   153,   -13,-32768,-32768,-32768,   541,
   108,-32768,-32768,    88,   149,-32768,-32768,-32768,   232,   177,
   298,    62,    87,-32768,    97,   541,   155,   165,-32768,-32768,
-32768,   242,   323,   412,   431,    20,    20,   196,   172,-32768,
-32768,   173,-32768,     3,-32768,-32768,   232,    12,-32768,   218,
-32768,-32768,   499,    41,    13,   169,   171,    23,   332,   189,
   113,    26,   178,-32768,-32768,   113,-32768,   232,   232,   232,
   232,   232,   222,-32768,   181,   191,-32768,-32768,-32768,   236,
   103,   235,   240,   243,-32768,   130,-32768,-32768,-32768,-32768,
   206,   201,-32768,   113,   205,   232,-32768,   489,-32768,-32768,
-32768,-32768,    38,   248,   209,-32768,   209,   252,    76,   113,
   210,   -11,   124,-32768,-32768,   158,-32768,   232,   232,   232,
-32768,-32768,   208,   212,-32768,   215,-32768,   170,   556,     4,
   330,   556,   242,   229,   232,   244,   238,   130,    44,   130,
    43,    68,    91,   241,   249,-32768,-32768,   257,-32768,-32768,
   218,   541,   266,   268,-32768,   -18,    20,-32768,-32768,-32768,
   113,   113,   251,-32768,   304,-32768,   541,    48,   541,   503,
-32768,-32768,-32768,   218,   232,   537,-32768,-32768,-32768,-32768,
   241,-32768,   259,   102,   147,-32768,   260,-32768,   261,-32768,
-32768,-32768,   232,   551,-32768,   270,   313,   272,-32768,   166,
   128,   232,-32768,   232,   232,-32768,   232,-32768,-32768,    15,
   130,   120,   232,   355,-32768,   309,-32768,   294,-32768,-32768,
   541,   541,   435,   466,-32768,   285,   267,-32768,   297,   292,
   296,-32768,-32768,-32768,    17,   325,-32768,   339,   342,   312,
   321,   337,   338,   317,   362,   326,   327,-32768,-32768,   344,
   333,   322,-32768,   381,   383,-32768
};

static const short yypgoto[] = {-32768,
-32768,   356,-32768,   245,-32768,-32768,-32768,-32768,   324,   202,
-32768,-32768,-32768,-32768,-32768,   -43,-32768,  -111,-32768,   156,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   354,  -105,
   -77,-32768,   -70,   -12,   318,   396,   192,     0,   -19,   138,
-32768,-32768,-32768,-32768,   262
};


#define	YYLAST		609


static const short yytable[] = {    40,
   116,    84,   147,   142,   177,    74,    51,    92,   100,    71,
    62,    63,    64,    65,   102,    98,    99,   100,   101,   144,
    83,   174,    85,   102,   103,   158,    89,   133,    57,   153,
   227,   268,   156,   103,   228,   104,   105,   124,   141,   269,
   192,    24,    25,    26,   104,   105,   -93,   209,   210,   -94,
   211,     2,   204,   106,   212,    47,    48,     5,   177,     6,
     7,   183,   106,    49,    50,   145,    58,   154,    11,   255,
    58,   134,   150,    59,    60,   224,   149,    12,    13,    45,
    14,  -123,    68,   234,  -123,  -123,  -123,  -123,    20,   184,
    22,   215,  -123,   216,   109,    23,   108,   159,   160,   161,
   162,   163,    27,   108,    98,    99,   100,   101,    94,   109,
   -87,   180,   102,  -123,  -123,   -87,   217,    44,   218,    71,
   230,   231,   103,   209,   210,   182,   147,    74,   198,   177,
   212,  -123,   110,   104,   105,   124,   109,   111,    86,   219,
   125,   220,   112,   113,   114,   126,   147,   197,   199,   200,
   127,   106,   168,   109,    87,    88,   239,   169,   113,   114,
     1,     2,    95,   108,   206,     3,     4,     5,   214,     6,
     7,   241,   126,     8,     9,    10,   124,   194,    11,   168,
   250,    75,    76,   122,   112,   113,   114,    12,    13,    77,
    14,    15,    16,    17,    18,    19,   168,   121,    20,    21,
    22,   240,   113,   114,   197,    23,   195,   128,    24,    25,
    26,   196,    27,    78,   124,    21,   249,   129,   124,   138,
   139,   140,   244,   151,    44,   157,   152,    79,   164,    75,
    76,   251,   165,   252,   253,   166,   254,    77,    46,     2,
   167,   174,   257,    47,    48,     5,   175,     6,     7,   176,
   178,    49,    50,   179,   181,   185,    11,   187,   188,   191,
   201,    78,   103,    21,   202,    12,    13,   203,    14,    98,
    99,   100,   101,   104,   105,    79,    20,   102,    22,    52,
     2,   205,   208,    23,    47,    48,     5,   103,     6,     7,
    27,   106,    49,    50,   207,   221,   232,    11,   104,   105,
    98,    99,   100,   101,   222,   223,    12,    13,   102,    14,
   225,   233,   226,   238,   242,   243,   106,    20,   103,    22,
   247,   264,   246,   248,    23,    98,    99,   100,   101,   104,
   105,    27,   259,   102,    98,    99,   100,   101,   260,   263,
   102,   265,   102,   103,   266,   106,   271,   106,   270,   272,
   103,   123,   103,   274,   104,   105,   273,    98,    99,   100,
   101,   104,   105,   104,   105,   102,   275,   276,   277,   278,
   279,   280,   106,   281,   283,   103,   130,   282,    90,   106,
   285,   106,   286,    93,   190,   155,   104,   105,   229,    97,
   137,    91,    92,    10,   143,    38,   236,   256,   267,     0,
   189,     0,     0,     0,   106,     0,     0,     0,   258,    15,
    16,    17,    18,    19,    98,    99,   100,   101,     0,     0,
     0,     0,   102,     0,     0,     0,    24,    25,    26,     0,
     0,     0,   103,    98,    99,   100,   101,    98,    99,   100,
   101,   102,     0,   104,   105,   102,     0,     0,     0,     0,
     0,   103,     0,     0,     0,   103,     0,     0,     0,     0,
     0,   106,   104,   105,   131,     0,   104,   105,    98,    99,
   100,   101,     0,     0,     0,     0,   102,     0,     0,     0,
   106,     0,     0,   132,   106,     0,   103,   261,  -108,     0,
     0,  -108,  -108,  -108,  -108,     0,     0,   104,   105,  -108,
     0,    98,    99,   100,   101,    98,    99,   100,   101,   102,
     0,     0,     0,   102,     0,   106,     0,     0,   262,   103,
  -108,  -108,     0,   103,   148,     0,     0,     0,     0,     0,
   104,   105,     0,   235,   104,   105,     0,     0,  -108,    98,
    99,   100,   101,    98,    99,   100,   101,   102,   106,     0,
     0,   102,   106,     0,     0,     0,     0,   103,    98,    99,
   100,   103,    75,    76,     0,     0,   102,   237,   104,   105,
    77,     0,   104,   105,     0,     0,   103,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   106,   104,   105,     0,
   106,     0,     0,     0,    78,     0,    21,     0,     0,     0,
     0,   245,     0,     0,     0,   106,     0,     0,    79
};

static const short yycheck[] = {     0,
    44,    21,    80,    74,   116,    18,     7,    21,     5,     7,
    11,    12,    13,    14,    11,     3,     4,     5,     6,     8,
    21,     7,    23,    11,    21,    96,    27,     8,     7,     7,
    49,    15,     7,    21,    53,    32,    33,    49,    36,    23,
    52,    55,    56,    57,    32,    33,     0,     4,     5,     0,
     7,     8,   158,    50,    11,    12,    13,    14,   170,    16,
    17,    24,    50,    20,    21,    54,    45,    45,    25,    55,
    45,    52,    60,    18,    19,   181,    36,    34,    35,    45,
    37,     0,     7,    36,     3,     4,     5,     6,    45,    52,
    47,    49,    11,    51,     7,    52,    56,    98,    99,   100,
   101,   102,    59,    56,     3,     4,     5,     6,    48,     7,
    49,   124,    11,    32,    33,    54,    49,    36,    51,     7,
   191,   192,    21,     4,     5,   126,   204,   140,   148,   241,
    11,    50,    45,    32,    33,    49,     7,    50,     7,    49,
    54,    51,    55,    56,    57,    49,   224,   148,   149,   150,
    54,    50,    50,     7,     7,    21,    55,    55,    56,    57,
     7,     8,    10,    56,   165,    12,    13,    14,   169,    16,
    17,   215,    49,    20,    21,    22,    49,    54,    25,    50,
    53,    12,    13,     7,    55,    56,    57,    34,    35,    20,
    37,    38,    39,    40,    41,    42,    50,    49,    45,    46,
    47,    55,    56,    57,   205,    52,    49,    53,    55,    56,
    57,    54,    59,    44,    49,    46,    51,    53,    49,    24,
    49,    49,   223,    55,    36,    48,    56,    58,     7,    12,
    13,   232,    52,   234,   235,    45,   237,    20,     7,     8,
     5,     7,   243,    12,    13,    14,     7,    16,    17,     7,
    45,    20,    21,    53,    50,     8,    25,    49,     7,    50,
    53,    44,    21,    46,    53,    34,    35,    53,    37,     3,
     4,     5,     6,    32,    33,    58,    45,    11,    47,     7,
     8,    53,    45,    52,    12,    13,    14,    21,    16,    17,
    59,    50,    20,    21,    51,    55,    46,    25,    32,    33,
     3,     4,     5,     6,    56,    49,    34,    35,    11,    37,
    45,     8,    45,    55,    55,    55,    50,    45,    21,    47,
     8,    55,    53,    52,    52,     3,     4,     5,     6,    32,
    33,    59,    24,    11,     3,     4,     5,     6,    45,    55,
    11,    45,    11,    21,    53,    50,     8,    50,    24,     8,
    21,    54,    21,    33,    32,    33,    45,     3,     4,     5,
     6,    32,    33,    32,    33,    11,    30,    30,    52,     8,
    45,    45,    50,    30,    53,    21,    54,    45,     7,    50,
     0,    50,     0,    28,   140,    54,    32,    33,   187,    36,
    67,    20,    21,    22,    77,     0,   205,   242,   261,    -1,
   139,    -1,    -1,    -1,    50,    -1,    -1,    -1,    54,    38,
    39,    40,    41,    42,     3,     4,     5,     6,    -1,    -1,
    -1,    -1,    11,    -1,    -1,    -1,    55,    56,    57,    -1,
    -1,    -1,    21,     3,     4,     5,     6,     3,     4,     5,
     6,    11,    -1,    32,    33,    11,    -1,    -1,    -1,    -1,
    -1,    21,    -1,    -1,    -1,    21,    -1,    -1,    -1,    -1,
    -1,    50,    32,    33,    53,    -1,    32,    33,     3,     4,
     5,     6,    -1,    -1,    -1,    -1,    11,    -1,    -1,    -1,
    50,    -1,    -1,    53,    50,    -1,    21,    53,     0,    -1,
    -1,     3,     4,     5,     6,    -1,    -1,    32,    33,    11,
    -1,     3,     4,     5,     6,     3,     4,     5,     6,    11,
    -1,    -1,    -1,    11,    -1,    50,    -1,    -1,    53,    21,
    32,    33,    -1,    21,    26,    -1,    -1,    -1,    -1,    -1,
    32,    33,    -1,    31,    32,    33,    -1,    -1,    50,     3,
     4,     5,     6,     3,     4,     5,     6,    11,    50,    -1,
    -1,    11,    50,    -1,    -1,    -1,    -1,    21,     3,     4,
     5,    21,    12,    13,    -1,    -1,    11,    31,    32,    33,
    20,    -1,    32,    33,    -1,    -1,    21,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    50,    32,    33,    -1,
    50,    -1,    -1,    -1,    44,    -1,    46,    -1,    -1,    -1,
    -1,    51,    -1,    -1,    -1,    50,    -1,    -1,    58
};

#line 325 "/usr/local/lib/bison.cc"
 /* fattrs + tables */

/* parser code folow  */


/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: dollar marks section change
   the next  is replaced by the list of actions, each action
   as one case of the switch.  */ 

#if YY_SSLParser_USE_GOTO != 0
/* 
 SUPRESSION OF GOTO : on some C++ compiler (sun c++)
  the goto is strictly forbidden if any constructor/destructor
  is used in the whole function (very stupid isn't it ?)
 so goto are to be replaced with a 'while/switch/case construct'
 here are the macro to keep some apparent compatibility
*/
#define YYGOTO(lb) {yy_gotostate=lb;continue;}
#define YYBEGINGOTO  enum yy_labels yy_gotostate=yygotostart; \
                     for(;;) switch(yy_gotostate) { case yygotostart: {
#define YYLABEL(lb) } case lb: {
#define YYENDGOTO } } 
#define YYBEGINDECLARELABEL enum yy_labels {yygotostart
#define YYDECLARELABEL(lb) ,lb
#define YYENDDECLARELABEL  };
#else
/* macro to keep goto */
#define YYGOTO(lb) goto lb
#define YYBEGINGOTO 
#define YYLABEL(lb) lb:
#define YYENDGOTO
#define YYBEGINDECLARELABEL 
#define YYDECLARELABEL(lb)
#define YYENDDECLARELABEL 
#endif
/* LABEL DECLARATION */
YYBEGINDECLARELABEL
  YYDECLARELABEL(yynewstate)
  YYDECLARELABEL(yybackup)
/* YYDECLARELABEL(yyresume) */
  YYDECLARELABEL(yydefault)
  YYDECLARELABEL(yyreduce)
  YYDECLARELABEL(yyerrlab)   /* here on detecting error */
  YYDECLARELABEL(yyerrlab1)   /* here on error raised explicitly by an action */
  YYDECLARELABEL(yyerrdefault)  /* current state does not do anything special for the error token. */
  YYDECLARELABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */
  YYDECLARELABEL(yyerrhandle)  
YYENDDECLARELABEL
/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_SSLParser_LSP_NEEDED
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		    __ALLOCA_free(yyls,yylsa)+\
		   (num))
#else
#define __ALLOCA_return(num) \
            return( __ALLOCA_free(yyss,yyssa)+\
		    __ALLOCA_free(yyvs,yyvsa)+\
		   (num))
#endif
#else
#define __ALLOCA_return(num) return(num)
#define __ALLOCA_alloca(size) alloca(size)
#define __ALLOCA_free(ptr,ref) 
#endif

/* ENDALLOCA SIMULATION */

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_SSLParser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        __ALLOCA_return(0)
#define YYABORT         __ALLOCA_return(1)
#define YYERROR         YYGOTO(yyerrlab1)
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL          YYGOTO(yyerrlab)
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do                                                              \
  if (YY_SSLParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_SSLParser_CHAR = (token), YY_SSLParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_SSLParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_SSLParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_SSLParser_PURE
/* UNPURE */
#define YYLEX           YY_SSLParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_SSLParser_CHAR;                      /*  the lookahead symbol        */
YY_SSLParser_STYPE      YY_SSLParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_SSLParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_SSLParser_LSP_NEEDED
YY_SSLParser_LTYPE YY_SSLParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_SSLParser_LSP_NEEDED
#define YYLEX           YY_SSLParser_LEX(&YY_SSLParser_LVAL, &YY_SSLParser_LLOC)
#else
#define YYLEX           YY_SSLParser_LEX(&YY_SSLParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_SSLParser_DEBUG != 0
int YY_SSLParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif
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


#if __GNUC__ > 1                /* GNU C and GNU C++ define this.  */
#define __yy_bcopy(FROM,TO,COUNT)       __builtin_memcpy(TO,FROM,COUNT)
#else                           /* not GNU C or C++ */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */

#ifdef __cplusplus
static void __yy_bcopy (char *from, char *to, int count)
#else
#ifdef __STDC__
static void __yy_bcopy (char *from, char *to, int count)
#else
static void __yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
#endif
#endif
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}
#endif

int
#ifdef YY_USE_CLASS
 YY_SSLParser_CLASS::
#endif
     YY_SSLParser_PARSE(YY_SSLParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_SSLParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_SSLParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_SSLParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_SSLParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_SSLParser_LSP_NEEDED
  YY_SSLParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_SSLParser_LTYPE *yyls = yylsa;
  YY_SSLParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_SSLParser_PURE
  int YY_SSLParser_CHAR;
  YY_SSLParser_STYPE YY_SSLParser_LVAL;
  int YY_SSLParser_NERRS;
#ifdef YY_SSLParser_LSP_NEEDED
  YY_SSLParser_LTYPE YY_SSLParser_LLOC;
#endif
#endif

  YY_SSLParser_STYPE yyval;             /*  the variable used to return         */
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
  YY_SSLParser_NERRS = 0;
  YY_SSLParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

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

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YY_SSLParser_STYPE *yyvs1 = yyvs;
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
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YY_SSLParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_SSLParser_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_SSLParser_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_SSLParser_LSP_NEEDED
      yyls = (YY_SSLParser_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
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

  if (YY_SSLParser_CHAR == YYEMPTY)
    {
#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_SSLParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_SSLParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_SSLParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_SSLParser_CHAR);

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_SSLParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_SSLParser_CHAR, YY_SSLParser_LVAL);
#endif
	  fprintf (stderr, ")\n");
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

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrlab);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrlab);

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_SSLParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_SSLParser_CHAR != YYEOF)
    YY_SSLParser_CHAR = YYEMPTY;

  *++yyvsp = YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
  *++yylsp = YY_SSLParser_LLOC;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

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
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


/* #line 811 "/usr/local/lib/bison.cc" */
#line 1558 "sslparser.cpp"

  switch (yyn) {

case 1:
#line 209 "sslparser.y"
{
            the_exp = yyvsp[0].regtransfer;
        ;
    break;}
case 2:
#line 212 "sslparser.y"
{
            the_exp = yyvsp[0].exp;
        ;
    break;}
case 7:
#line 226 "sslparser.y"
{
            Dict.fetchExecCycle = yyvsp[0].rtlist;
        ;
    break;}
case 14:
#line 251 "sslparser.y"
{ Dict.fixupParams(); ;
    break;}
case 17:
#line 264 "sslparser.y"
{
            // Note: the below copies the list of strings!
            Dict.DetParamMap[yyvsp[-4].str].params = *yyvsp[-1].parmlist;
            Dict.DetParamMap[yyvsp[-4].str].kind = PARAM_VARIANT;
            delete yyvsp[-1].parmlist;
        ;
    break;}
case 18:
#line 278 "sslparser.y"
{
            std::map<std::string, InsNameElem*> m;
            ParamEntry &param = Dict.DetParamMap[yyvsp[-4].str];
            Exp* asgn = new AssignExp(yyvsp[-1].num, new Terminal(opNil), yyvsp[0].exp);
            // Note: The below 2 copy lists of strings (to be deleted below!)
            param.params = *yyvsp[-3].parmlist;
            param.funcParams = *yyvsp[-2].parmlist;
            param.exp = asgn;
            param.kind = PARAM_EXPR;
            
            if( param.funcParams.size() != 0 )
                param.kind = PARAM_LAMBDA;
            delete yyvsp[-3].parmlist;
            delete yyvsp[-2].parmlist;
        ;
    break;}
case 19:
#line 295 "sslparser.y"
{ yyval.parmlist = yyvsp[-1].parmlist; ;
    break;}
case 20:
#line 296 "sslparser.y"
{ yyval.parmlist = new std::list<std::string>(); ;
    break;}
case 21:
#line 300 "sslparser.y"
{
                    bFloat = false;
                ;
    break;}
case 23:
#line 303 "sslparser.y"
{
                    bFloat = true;
                ;
    break;}
case 27:
#line 313 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-2].str) != Dict.RegMap.end())
                    yyerror("Name reglist decared twice\n");
                Dict.RegMap[yyvsp[-2].str] = yyvsp[0].num;
            ;
    break;}
case 28:
#line 318 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-5].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.addRegister( yyvsp[-5].str, yyvsp[0].num, yyvsp[-3].num, bFloat);
            ;
    break;}
case 29:
#line 323 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-9].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.RegMap[yyvsp[-9].str] = yyvsp[-4].num;
                // Now for detailed Reg information
                if (Dict.DetRegMap.find(yyvsp[-4].num) != Dict.DetRegMap.end())
                    yyerror("Index used for more than one register\n");
                Dict.DetRegMap[yyvsp[-4].num].s_name(yyvsp[-9].str);
                Dict.DetRegMap[yyvsp[-4].num].s_size(yyvsp[-7].num);
                Dict.DetRegMap[yyvsp[-4].num].s_address(NULL);
                // check range is legitimate for size. 8,10
                if ((Dict.RegMap.find(yyvsp[-2].str) == Dict.RegMap.end())
                    || (Dict.RegMap.find(yyvsp[0].str) == Dict.RegMap.end()))
                   yyerror("Undefined range\n");
                else {
                    int bitsize = Dict.DetRegMap[Dict.RegMap[yyvsp[0].str]].g_size();
                    for (int i = Dict.RegMap[yyvsp[-2].str]; i != Dict.RegMap[yyvsp[0].str]; i++) {
                        if (Dict.DetRegMap.find(i) == Dict.DetRegMap.end()) {
                            yyerror("Not all regesters in range defined\n");
                            break;
                        }
                        bitsize += Dict.DetRegMap[i].g_size();
                        if (bitsize > yyvsp[-7].num) {
                            yyerror("Range exceeds size of register\n");
                            break;
                        }
                    }
                if (bitsize < yyvsp[-7].num) 
                    yyerror("Register size is exceeds regesters in range\n");
                    // copy information
                }
                Dict.DetRegMap[yyvsp[-4].num].s_mappedIndex(Dict.RegMap[yyvsp[-2].str]);
                Dict.DetRegMap[yyvsp[-4].num].s_mappedOffset(0);
                Dict.DetRegMap[yyvsp[-4].num].s_float(bFloat);
            ;
    break;}
case 30:
#line 358 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-13].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.RegMap[yyvsp[-13].str] = yyvsp[-8].num;
                // Now for detailed Reg information
                if (Dict.DetRegMap.find(yyvsp[-8].num) != Dict.DetRegMap.end())
                    yyerror("Index used for more than one register\n");
                Dict.DetRegMap[yyvsp[-8].num].s_name(yyvsp[-13].str);
                Dict.DetRegMap[yyvsp[-8].num].s_size(yyvsp[-11].num);
                Dict.DetRegMap[yyvsp[-8].num].s_address(NULL);
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
    break;}
case 31:
#line 380 "sslparser.y"
{
                if ((int)yyvsp[-8].strlist->size() != (yyvsp[0].num - yyvsp[-2].num + 1)) {
                    std::cerr << "size of register array does not match mapping to r["
                         << yyvsp[-2].num << ".." << yyvsp[0].num << "]\n";
                    exit(1);
                } else {
                    std::list<std::string>::iterator loc = yyvsp[-8].strlist->begin();
                    for (int x = yyvsp[-2].num; x <= yyvsp[0].num; x++, loc++) {
                        if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                            yyerror("Name reglist declared twice\n");
                        Dict.addRegister( loc->c_str(), x, yyvsp[-5].num, bFloat);
                    }
                    delete yyvsp[-8].strlist;
                }
            ;
    break;}
case 32:
#line 395 "sslparser.y"
{
                std::list<std::string>::iterator loc = yyvsp[-6].strlist->begin();
                for (; loc != yyvsp[-6].strlist->end(); loc++) {
                    if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                        yyerror("Name reglist declared twice\n");
		    Dict.addRegister(loc->c_str(), yyvsp[0].num, yyvsp[-3].num, bFloat);
                }
                delete yyvsp[-6].strlist;
            ;
    break;}
case 33:
#line 407 "sslparser.y"
{
                yyvsp[-2].strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 34:
#line 410 "sslparser.y"
{
                yyval.strlist = new std::list<std::string>;
                yyval.strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 35:
#line 419 "sslparser.y"
{
                // Note: $2 is a list of strings
                Dict.FlagFuncs[yyvsp[-5].str] = new FlagDef(listStrToExp(yyvsp[-4].parmlist), yyvsp[-1].rtlist);
            ;
    break;}
case 36:
#line 426 "sslparser.y"
{
                if (ConstTable.find(yyvsp[-2].str) != ConstTable.end())
                    yyerror("Constant declared twice");
                ConstTable[std::string(yyvsp[-2].str)] = yyvsp[0].num;
            ;
    break;}
case 37:
#line 432 "sslparser.y"
{
                if (ConstTable.find(yyvsp[-4].str) != ConstTable.end())
                    yyerror("Constant declared twice");
                else if (yyvsp[-1].str == std::string("-"))
                    ConstTable[std::string(yyvsp[-4].str)] = yyvsp[-2].num - yyvsp[0].num;
                else if (yyvsp[-1].str == std::string("+"))
                    ConstTable[std::string(yyvsp[-4].str)] = yyvsp[-2].num + yyvsp[0].num;
                else
                    yyerror("Constant expression must be NUM + NUM or NUM - NUM");
            ;
    break;}
case 38:
#line 446 "sslparser.y"
{
            TableDict[yyvsp[-2].str] = yyvsp[0].tab;
        ;
    break;}
case 39:
#line 452 "sslparser.y"
{
            yyval.tab = new Table(*yyvsp[0].namelist);
            delete yyvsp[0].namelist;
        ;
    break;}
case 40:
#line 457 "sslparser.y"
{
            yyval.tab = new OpTable(*yyvsp[0].namelist);
            delete yyvsp[0].namelist;
        ;
    break;}
case 41:
#line 461 "sslparser.y"
{
            yyval.tab = new ExprTable(*yyvsp[0].exprlist);
            delete yyvsp[0].exprlist;
        ;
    break;}
case 42:
#line 468 "sslparser.y"
{
            // cross-product of two str_expr's
            std::deque<std::string>::iterator i, j;
            yyval.namelist = new std::deque<std::string>;
            for (i = yyvsp[-1].namelist->begin(); i != yyvsp[-1].namelist->end(); i++)
                for (j = yyvsp[0].namelist->begin(); j != yyvsp[0].namelist->end(); j++)
                    yyval.namelist->push_back((*i) + (*j));
            delete yyvsp[-1].namelist;
            delete yyvsp[0].namelist;
        ;
    break;}
case 43:
#line 478 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 44:
#line 484 "sslparser.y"
{
            // want to append $3 to $1
            // The following causes a massive warning message about mixing
            // signed and unsigned
            yyvsp[-2].namelist->insert(yyvsp[-2].namelist->end(), yyvsp[0].namelist->begin(), yyvsp[0].namelist->end());
            delete yyvsp[0].namelist;
            yyval.namelist = yyvsp[-2].namelist;
        ;
    break;}
case 45:
#line 492 "sslparser.y"
{
            yyvsp[-3].namelist->push_back("");
        ;
    break;}
case 46:
#line 495 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 47:
#line 501 "sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 48:
#line 504 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 49:
#line 510 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>;
            yyval.namelist->push_back("");
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 50:
#line 515 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>(1, yyvsp[-1].str);
        ;
    break;}
case 51:
#line 518 "sslparser.y"
{
            std::ostringstream o;
            // expand $2 from table of names
            if (TableDict.find(yyvsp[0].str) != TableDict.end())
                if (TableDict[yyvsp[0].str]->getType() == NAMETABLE)
                    yyval.namelist = new std::deque<std::string>(TableDict[yyvsp[0].str]->records);
                else {
                    o << "name " << yyvsp[0].str << " is not a NAMETABLE.\n";
                    yyerror(STR(o));
                }
            else {
                o << "could not dereference name " << yyvsp[0].str << "\n";
                yyerror(STR(o));
            }
        ;
    break;}
case 52:
#line 533 "sslparser.y"
{
            // try and expand $1 from table of names
            // if fail, expand using '"' NAME '"' rule
            if (TableDict.find(yyvsp[0].str) != TableDict.end())
                if (TableDict[yyvsp[0].str]->getType() == NAMETABLE)
                    yyval.namelist = new std::deque<std::string>(TableDict[yyvsp[0].str]->records);
                else {
                    std::ostringstream o;
                    o << "name " << yyvsp[0].str << " is not a NAMETABLE.\n";
                    yyerror(STR(o));
                }
            else {
                yyval.namelist = new std::deque<std::string>;
                yyval.namelist->push_back(yyvsp[0].str);
            }
        ;
    break;}
case 53:
#line 552 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 54:
#line 556 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 55:
#line 560 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 56:
#line 567 "sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 57:
#line 574 "sslparser.y"
{
            yyval.namelist = yyvsp[-4].namelist;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 58:
#line 578 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 59:
#line 586 "sslparser.y"
{
            yyval.exprlist = yyvsp[-1].exprlist;
        ;
    break;}
case 60:
#line 593 "sslparser.y"
{
            yyval.exprlist = yyvsp[-4].exprlist;
            yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
    break;}
case 61:
#line 597 "sslparser.y"
{
            yyval.exprlist = new std::deque<Exp*>;
            yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
    break;}
case 62:
#line 605 "sslparser.y"
{
            yyvsp[0].insel->getrefmap(indexrefmap);
        //     $3           $4
        ;
    break;}
case 63:
#line 608 "sslparser.y"
{
            // This function expands the tables and saves the expanded RTLs
            // to the dictionary
            expandTables(yyvsp[-3].insel, yyvsp[-1].parmlist, yyvsp[0].rtlist, Dict);
        ;
    break;}
case 64:
#line 616 "sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 65:
#line 619 "sslparser.y"
{
            unsigned i;
            InsNameElem *temp;
            std::string nm = yyvsp[0].str;
            
            if (nm[0] == '^')
	            nm.replace(0, 1, "");

            // remove all " and _, from the decoration
            while ((i = nm.find("\"")) != nm.npos)
                nm.replace(i,1,"");
            // replace all '.' with '_'s from the decoration
            while ((i = nm.find(".")) != nm.npos)
	            nm.replace(i,1,"_");
            while ((i = nm.find("_")) != nm.npos)
	            nm.replace(i,1,"");
 
            temp = new InsNameElem(nm);
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(temp);
        ;
    break;}
case 66:
#line 643 "sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[0].str);
        ;
    break;}
case 67:
#line 646 "sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 68:
#line 649 "sslparser.y"
{
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(yyvsp[0].insel);
        ;
    break;}
case 69:
#line 656 "sslparser.y"
{
            yyval.insel = new InsOptionElem(yyvsp[-1].str);
        ;
    break;}
case 70:
#line 659 "sslparser.y"
{
            std::ostringstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(STR(o));
            } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->records.size())) {
                o << "Can't get element " << yyvsp[-1].num << " of table " << yyvsp[-2].str << ".\n";
                yyerror(STR(o));
            } else
                yyval.insel = new InsNameElem(TableDict[yyvsp[-2].str]->records[yyvsp[-1].num]);
        ;
    break;}
case 71:
#line 672 "sslparser.y"
{
            std::ostringstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(STR(o));
            } else
                yyval.insel = new InsListElem(yyvsp[-2].str, TableDict[yyvsp[-2].str], yyvsp[-1].str);
        ;
    break;}
case 72:
#line 681 "sslparser.y"
{
            std::ostringstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(STR(o));
            } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->records.size())) {
                o << "Can't get element " << yyvsp[-1].num << " of table " << yyvsp[-2].str << ".\n";
                yyerror(STR(o));
            } else
                yyval.insel = new InsNameElem(TableDict[yyvsp[-2].str]->records[yyvsp[-1].num]);
        ;
    break;}
case 73:
#line 692 "sslparser.y"
{
            std::ostringstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(STR(o));
            } else
                yyval.insel = new InsListElem(yyvsp[-2].str, TableDict[yyvsp[-2].str], yyvsp[-1].str);
        ;
    break;}
case 74:
#line 701 "sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[-1].str);
        ;
    break;}
case 75:
#line 707 "sslparser.y"
{
            // append any automatically generated register transfers and clear
            // the list they were stored in. Do nothing for a NOP (i.e. $2 = 0)
            if (yyvsp[0].regtransfer != NULL) {
                yyvsp[-1].rtlist->appendExp(yyvsp[0].regtransfer);
            }
            yyval.rtlist = yyvsp[-1].rtlist;
        ;
    break;}
case 76:
#line 716 "sslparser.y"
{
            yyval.rtlist = new RTL();
            if (yyvsp[0].regtransfer != NULL)
                yyval.rtlist->appendExp(yyvsp[0].regtransfer);
        ;
    break;}
case 77:
#line 724 "sslparser.y"
{
            yyval.regtransfer = yyvsp[0].regtransfer;
        ;
    break;}
case 78:
#line 730 "sslparser.y"
{
            std::ostringstream o;
            if (Dict.FlagFuncs.find(yyvsp[-2].str) != Dict.FlagFuncs.end()) {
                yyval.regtransfer = new AssignExp(new Terminal(opFlags), 
                    new Binary(opFlagCall, new Const(yyvsp[-2].str),
                    listExpToExp(yyvsp[-1].explist)));
            } else {
                o << yyvsp[-2].str << " is not declared as a flag function.\n";
                yyerror(STR(o));
            }
        ;
    break;}
case 79:
#line 741 "sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 80:
#line 745 "sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 81:
#line 748 "sslparser.y"
{
        yyval.regtransfer = NULL;
    ;
    break;}
case 82:
#line 754 "sslparser.y"
{
            // Not sure why the below is commented out (MVE)
/*          Unary* pFlag = new Unary(opRegOf, Dict.RegMap[$3]);
            $1->push_back(pFlag);
            $$ = $1;
*/          yyval.explist = 0;
        ;
    break;}
case 83:
#line 761 "sslparser.y"
{
/*          std::list<Exp*>* tmp = new std::list<Exp*>;
            Unary* pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
            tmp->push_back(pFlag);
            $$ = tmp;
*/          yyval.explist = 0;
        ;
    break;}
case 84:
#line 772 "sslparser.y"
{
            assert(yyvsp[0].str != 0);
            yyvsp[-2].parmlist->push_back(yyvsp[0].str);
            yyval.parmlist = yyvsp[-2].parmlist;
        ;
    break;}
case 85:
#line 778 "sslparser.y"
{
            yyval.parmlist = new std::list<std::string>;
            yyval.parmlist->push_back(yyvsp[0].str);
        ;
    break;}
case 86:
#line 782 "sslparser.y"
{
            yyval.parmlist = new std::list<std::string>;
        ;
    break;}
case 87:
#line 787 "sslparser.y"
{
            Dict.ParamSet.insert(yyvsp[0].str);       // Not sure if we need this set
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 88:
#line 793 "sslparser.y"
{
            yyval.explist->push_back(yyvsp[0].exp);
        ;
    break;}
case 89:
#line 797 "sslparser.y"
{
            yyval.explist = new std::list<Exp*>;
            yyval.explist->push_back(yyvsp[0].exp);
        ;
    break;}
case 90:
#line 802 "sslparser.y"
{
            yyval.explist = new std::list<Exp*>;
        ;
    break;}
case 91:
#line 810 "sslparser.y"
{
            yyval.regtransfer = new Unary(opGuard,
		    new AssignExp(yyvsp[-5].num, yyvsp[-2].exp, yyvsp[0].exp));
        ;
    break;}
case 92:
#line 816 "sslparser.y"
{
            // update the size of any generated RT's
            yyval.regtransfer = new AssignExp(yyvsp[-3].num, yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 93:
#line 822 "sslparser.y"
{
            yyval.regtransfer = new Terminal(opFpush);
        ;
    break;}
case 94:
#line 825 "sslparser.y"
{
            yyval.regtransfer = new Terminal(opFpop);
        ;
    break;}
case 95:
#line 829 "sslparser.y"
{
            yyval.regtransfer = new AssignExp(yyvsp[-1].num, 0, yyvsp[0].exp);
        ;
    break;}
case 96:
#line 835 "sslparser.y"
{
            yyval.exp = new Const(yyvsp[0].num);
        ;
    break;}
case 97:
#line 839 "sslparser.y"
{
            yyval.exp = new Const(yyvsp[0].dbl);
        ;
    break;}
case 98:
#line 843 "sslparser.y"
{
            yyval.exp = new Unary(opTemp, new Const(yyvsp[0].str));
        ;
    break;}
case 99:
#line 847 "sslparser.y"
{
            yyval.exp = yyvsp[-1].exp;
        ;
    break;}
case 100:
#line 851 "sslparser.y"
{
            yyval.exp = yyvsp[0].exp;
        ;
    break;}
case 101:
#line 855 "sslparser.y"
{
            yyval.exp = new Ternary(opTern, yyvsp[-5].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
    break;}
case 102:
#line 859 "sslparser.y"
{
            Ternary* t = new Ternary(opTern, yyvsp[-6].exp, yyvsp[-4].exp, yyvsp[-2].exp);
            Exp* e = t;
            if (yyvsp[0].num != STD_SIZE) {
			    e = new TypedExp(new IntegerType(yyvsp[0].num), t);                
            }
            yyval.exp = e;
        ;
    break;}
case 103:
#line 869 "sslparser.y"
{
            yyval.exp = new Unary(opAddrOf, yyvsp[-1].exp);
        ;
    break;}
case 104:
#line 875 "sslparser.y"
{
            yyval.exp = new Ternary(strToOper(yyvsp[-6].str), new Const(yyvsp[-5].num), new Const(yyvsp[-3].num), yyvsp[-1].exp);
        ;
    break;}
case 105:
#line 880 "sslparser.y"
{
            yyval.exp = new Terminal(opFpush);
        ;
    break;}
case 106:
#line 883 "sslparser.y"
{
            yyval.exp = new Terminal(opFpop);
        ;
    break;}
case 107:
#line 888 "sslparser.y"
{
            yyval.exp = new Unary(strToOper(yyvsp[-2].str), yyvsp[-1].exp);
        ;
    break;}
case 108:
#line 894 "sslparser.y"
{
            std::ostringstream o;
            if (indexrefmap.find(yyvsp[-1].str) == indexrefmap.end()) {
                o << "index " << yyvsp[-1].str << " not declared for use.\n";
                yyerror(STR(o));
            } else if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "table " << yyvsp[-2].str << " not declared for use.\n";
                yyerror(STR(o));
            } else if (TableDict[yyvsp[-2].str]->getType() != EXPRTABLE) {
                o << "table " << yyvsp[-2].str << " is not an expression table but "
                  "appears to be used as one.\n";
                yyerror(STR(o));
            } else if ((int)((ExprTable*)TableDict[yyvsp[-2].str])->expressions.size() <
              indexrefmap[yyvsp[-1].str]->ntokens()) {
                o << "table " << yyvsp[-2].str << " (" <<
                  ((ExprTable*)TableDict[yyvsp[-2].str])->expressions.size() <<
                  ") is too small to use " << yyvsp[-1].str << " (" <<
                  indexrefmap[yyvsp[-1].str]->ntokens() << ") as an index.\n";
                yyerror(STR(o));
            }
            // $1 is a map from string to Table*; $2 is a map from string to
            // InsNameElem*
            yyval.exp = new Binary(opExpTable, new Const(yyvsp[-2].str), new Const(yyvsp[-1].str));
        ;
    break;}
case 109:
#line 922 "sslparser.y"
{
        std::ostringstream o;
        if (Dict.ParamSet.find(yyvsp[-2].str) != Dict.ParamSet.end() ) {
            if (Dict.DetParamMap.find(yyvsp[-2].str) != Dict.DetParamMap.end()) {
                ParamEntry& param = Dict.DetParamMap[yyvsp[-2].str];
                if (yyvsp[-1].explist->size() != param.funcParams.size() ) {
                    o << yyvsp[-2].str << " requires " << param.funcParams.size()
                      << " parameters, but received " << yyvsp[-1].explist->size() << ".\n";
                    yyerror(STR(o));
                } else {
                    // Everything checks out. *phew* 
                    // Note: the below may not be right! (MVE)
                    Binary* e = new Binary(opFlagDef, new Const(yyvsp[-2].str),
                      listExpToExp(yyvsp[-1].explist));
                    yyval.exp = e;
                    delete yyvsp[-1].explist;          // Delete the list of char*s
                }
            } else {
                o << yyvsp[-2].str << " is not defined as a OPERAND function.\n";
                yyerror(STR(o));
            }
        } else {
            o << yyvsp[-2].str << ": Unrecognized name in call.\n";
            yyerror(STR(o));
        }
    ;
    break;}
case 110:
#line 951 "sslparser.y"
{
            yyval.exp = new Unary(opSignExt, yyvsp[-1].exp);
        ;
    break;}
case 111:
#line 960 "sslparser.y"
{
            // opSize is deprecated, but for old SSL files we'll make a TypedExp
            if (yyvsp[0].num == STD_SIZE)
                yyval.exp = yyvsp[-1].exp;
            else
                yyval.exp = new TypedExp(new IntegerType(yyvsp[0].num), yyvsp[-1].exp);
        ;
    break;}
case 112:
#line 968 "sslparser.y"
{
            yyval.exp = new Unary(opNot, yyvsp[0].exp);
        ;
    break;}
case 113:
#line 972 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 114:
#line 976 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 115:
#line 980 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 116:
#line 984 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 117:
#line 988 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 118:
#line 995 "sslparser.y"
{
            std::ostringstream o;
            if (indexrefmap.find(yyvsp[-2].str) == indexrefmap.end()) {
                o << "index " << yyvsp[-2].str << " not declared for use.\n";
                yyerror(STR(o));
            } else if (TableDict.find(yyvsp[-3].str) == TableDict.end()) {
                o << "table " << yyvsp[-3].str << " not declared for use.\n";
                yyerror(STR(o));
            } else if (TableDict[yyvsp[-3].str]->getType() != OPTABLE) {
                o << "table " << yyvsp[-3].str <<
                  " is not an operator table but appears to be used as one.\n";
                yyerror(STR(o));
            } else if ((int)TableDict[yyvsp[-3].str]->records.size() <
              indexrefmap[yyvsp[-2].str]->ntokens()) {
                o << "table " << yyvsp[-3].str << " is too small to use with " << yyvsp[-2].str
                  << " as an index.\n";
                yyerror(STR(o));
            }
            yyval.exp = new Ternary(opOpTable, new Const(yyvsp[-3].str), new Const(yyvsp[-2].str),
                new Binary(opList,
                    yyvsp[-4].exp,
                    new Binary(opList,
                        yyvsp[0].exp,
                        new Terminal(opNil))));
        ;
    break;}
case 119:
#line 1021 "sslparser.y"
{
            yyval.exp = yyvsp[0].exp;
        ;
    break;}
case 120:
#line 1032 "sslparser.y"
{
            std::map<std::string, int>::const_iterator it = Dict.RegMap.find(yyvsp[0].str);
            if (it == Dict.RegMap.end()) {
                std::ostringstream ost;
                ost << "register `" << yyvsp[0].str << "' is undefined";
                yyerror(STR(ost));
            } else if (it->second == -1) {
                // A special register, e.g. %npc or %CF
                // Return a Terminal for it
                OPER op = strToTerm(yyvsp[0].str);
                if (op) {
                    yyval.exp = new Terminal(op);
                } else {
                    yyval.exp = new Unary(opMachFtr,    // Machine specific feature
                            new Const(yyvsp[0].str));
                }
            }
            else {
                // A register with a constant reg nmber, e.g. %g2.
                // In this case, we want to return r[const 2]
                yyval.exp = new Unary(opRegOf, new Const(it->second));
            }
        ;
    break;}
case 121:
#line 1056 "sslparser.y"
{
            yyval.exp = new Unary(opRegOf, yyvsp[-1].exp);
        ;
    break;}
case 122:
#line 1060 "sslparser.y"
{
            yyval.exp = new Unary(opMemOf, yyvsp[-1].exp);
        ;
    break;}
case 123:
#line 1064 "sslparser.y"
{
        // This is a mixture of the param: PARM {} match
        // and the value_op: NAME {} match
            Exp* s;
            std::set<std::string>::iterator it = Dict.ParamSet.find(yyvsp[0].str);
            if (it != Dict.ParamSet.end()) {
                s = new Unary(opParam, new Const(yyvsp[0].str));
            } else if (ConstTable.find(yyvsp[0].str) != ConstTable.end()) {
                s = new Const(ConstTable[yyvsp[0].str]);
            } else {
                std::ostringstream ost;
                ost << "`" << yyvsp[0].str << "' is not a constant, definition or a";
                ost << " parameter of this instruction\n";
                yyerror(STR(ost));
            }
            yyval.exp = s;
        ;
    break;}
case 124:
#line 1082 "sslparser.y"
{
            yyval.exp = new Ternary(opAt, yyvsp[-6].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
    break;}
case 125:
#line 1086 "sslparser.y"
{
            yyval.exp = new Unary(opPostVar, yyvsp[-1].exp);
        ;
    break;}
case 126:
#line 1092 "sslparser.y"
{
            yyval.num = yyvsp[-1].num;
        ;
    break;}
case 127:
#line 1098 "sslparser.y"
{
            Dict.bigEndian = (strcmp(yyvsp[0].str, "BIG") == 0);
        ;
    break;}
case 128:
#line 1103 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 129:
#line 1106 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 133:
#line 1125 "sslparser.y"
{
            Dict.fastMap[std::string(yyvsp[-2].str)] = std::string(yyvsp[0].str);
        ;
    break;}
}

#line 811 "/usr/local/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_SSLParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_SSLParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_SSLParser_LLOC.first_line;
      yylsp->first_column = YY_SSLParser_LLOC.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

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

YYLABEL(yyerrlab)   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++YY_SSLParser_NERRS;

#ifdef YY_SSLParser_ERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      YY_SSLParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_SSLParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_SSLParser_ERROR_VERBOSE */
	YY_SSLParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_SSLParser_CHAR == YYEOF)
	YYABORT;

#if YY_SSLParser_DEBUG != 0
      if (YY_SSLParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_SSLParser_CHAR, yytname[yychar1]);
#endif

      YY_SSLParser_CHAR = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;              /* Each real token shifted decrements this */

  YYGOTO(yyerrhandle);

YYLABEL(yyerrdefault)  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) YYGOTO(yydefault);
#endif

YYLABEL(yyerrpop)   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YY_SSLParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
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
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	YYGOTO(yyerrpop);
      yyn = -yyn;
      YYGOTO(yyreduce);
    }
  else if (yyn == 0)
    YYGOTO(yyerrpop);

  if (yyn == YYFINAL)
    YYACCEPT;

#if YY_SSLParser_DEBUG != 0
  if (YY_SSLParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
  *++yylsp = YY_SSLParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 1010 "/usr/local/lib/bison.cc" */
#line 2776 "sslparser.cpp"
#line 1128 "sslparser.y"


/*==============================================================================
 * FUNCTION:        SSLParser::SSLParser
 * OVERVIEW:        Constructor for an existing stream.
 * PARAMETERS:      The stream, whether or not to debug
 * RETURNS:         <nothing>
 *============================================================================*/
SSLParser::SSLParser(std::istream &in, bool trace) : sslFile("input"), bFloat(false)
{
    theScanner = new SSLScanner(in, trace);
    if (trace) yydebug = 1; else yydebug=0;
}

/*==============================================================================
 * FUNCTION:        SSLParser::parseExp
 * OVERVIEW:        Parses an expression from a string.
 * PARAMETERS:      the string
 * RETURNS:         an expression or NULL.
 *============================================================================*/
Exp* SSLParser::parseExp(const char *str)
{
    std::istringstream ss(str);
    SSLParser p(ss, false);     // Second arg true for debugging
    RTLInstDict d;
    p.yyparse(d);
    return p.the_exp;
}

/*==============================================================================
 * FUNCTION:        SSLParser::~SSLParser
 * OVERVIEW:        Destructor.
 * PARAMETERS:      <none>
 * RETURNS:         <nothing>
 *============================================================================*/
SSLParser::~SSLParser()
{
    std::map<std::string, Table*>::iterator loc;
    if (theScanner != NULL)
        delete theScanner;
    for(loc = TableDict.begin(); loc != TableDict.end(); loc++)
        delete loc->second;
}

/*==============================================================================
 * FUNCTION:        SSLParser::yyerror
 * OVERVIEW:        Display an error message and exit.
 * PARAMETERS:      msg - an error message
 * RETURNS:         <nothing>
 *============================================================================*/
void SSLParser::yyerror(char* msg)
{
    std::cerr << sslFile << ":" << theScanner->theLine << ": " << msg << std::endl;
}

/*==============================================================================
 * FUNCTION:        SSLParser::yylex
 * OVERVIEW:        The scanner driver than returns the next token.
 * PARAMETERS:      <none>
 * RETURNS:         the next token
 *============================================================================*/
int SSLParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}

/*==============================================================================
 * FUNCTION:        SSLParser::strToOper
 * OVERVIEW:        Convert a string operator (e.g. "+f") to an OPER (opFPlus)
 * NOTE:            An attempt is made to make this moderately efficient, else
 *                    we might have a skip chain of string comparisons
 * NOTE:            This is a member of SSLParser so we can call yyerror and
 *                    have line number etc printed out
 * PARAMETERS:      s: pointer to the operator C string
 * RETURNS:         An OPER, or -1 if not found (enum opWild)
 *============================================================================*/
OPER SSLParser::strToOper(const char* s) {
    switch (s[0]) {
        case '*':
            // Could be *, *!, *f, *fsd, *fdq, *f[sdq]
            switch (s[1]) {
                case '\0': return opMult;
                case '!' : return opMults;
                case 'f' :
                    if ((s[2] == 's') && (s[3] == 'd')) return opFMultsd;
                    if ((s[2] == 'd') && (s[3] == 'q')) return opFMultdq;
                    return opFMult;
                default: break;
            }
            break;
        case '/':
            // Could be /, /!, /f, /f[sdq]
            switch (s[1]) {
                case '\0': return opDiv;
                case '!' : return opDivs;
                case 'f' : return opFDiv;
                default: break;
            }
            break;
        case '%':
            // Could be %, %!
            switch (s[1]) {
                case '\0': return opMod;
                case '!' : return opMods;
                default: break;
            }
            break;
        case '+':
            // Could be +, +f, +f[sdq]
            switch (s[1]) {
                case '\0': return opPlus;
                case 'f' : return opFPlus;
                default: break;
            }
            break;
        case '-':
            // Could be -, -f, -f[sdq]
            switch (s[1]) {
                case '\0': return opMinus;
                case 'f' : return opFMinus;
                default: break;
            }
            break;
        case 'a':
            // and, arctan, addr
            if (s[1] == 'n') return opAnd;
            if (s[1] == 'r') return opArcTan;
            if (s[1] == 'd') return opAddrOf;
                break;
        case 'c':
            // cos
            return opCos;
        case 'e':
            // execute
            return opExecute;
        case 'f':
            // fsize, ftoi, fround
            if (s[1] == 's') return opFsize;
            if (s[1] == 't') return opFtoi;
            if (s[1] == 'r') return opFround;
            break;
        case 'i':
            // itof
            return opItof;
        case 'l':
            // log2, log10, loge
            if (s[3] == '2') return opLog2;
            if (s[3] == '1') return opLog10;
            if (s[3] == 'e') return opLoge;
            break;
        case 'o':
            // or
            return opOr;
        case 'r':
            // rlc, rrc, rl, rr
            if (s[1] == 'l') {
                if (s[2] == 'c') return opRotateLC;
                return opRotateL;
            } else if (s[1] == 'r') {
                if (s[2] == 'c') return opRotateRC;
                return opRotateR;
            }
            break;
        case 's':
            // sgnex, sin, sqrt
            if (s[1] == 'g') return opSgnEx;
            if (s[1] == 'i') return opSin;
            if (s[1] == 'q') return opSqrt;
            break;
        case 't':
            // truncu, truncs, tan
            // 012345
            if (s[1] == 'a') return opTan;
            if (s[5] == 'u') return opTruncu;
            if (s[5] == 's') return opTruncs;
            break;
        case 'z':
            // zfill
            return opZfill;

        case '>':
            // >, >u, >=, >=u, >>, >>A
            switch (s[1]) {
                case '\0': return opGtr;
                case 'u': return opGtrUns;
                case '=':
                    if (s[2] == '\0') return opGtrEq;
                    return opGtrEqUns;
                case '>':
                    if (s[2] == '\0') return opShiftR;
                    return opShiftRA;
                default: break;
            }
            break;
        case '<':
            // <, <u, <=, <=u, <<
            switch (s[1]) {
                case '\0': return opLess;
                case 'u': return opLessUns;
                case '=':
                    if (s[2] == '\0') return opLessEq;
                    return opLessEqUns;
                case '<':
                    return opShiftL;
                default: break;
            }
            break;
        case '=':
            // =
            return opEquals;
        case '!':
            // !
            return opSgnEx;
            break;
        case '~':
            // ~=, ~
            if (s[1] == '=') return opNotEqual;
            return opNot;       // Bit inversion
        case '@': return opAt;
        case '&': return opBitAnd;
        case '|': return opBitOr;
        case '^': return opBitXor;
       
        default: break; 
    }
    std::ostringstream ost;
    ost << "Unknown operator " << s << std::endl;
    yyerror(STR(ost));
    return opWild;
}

OPER strToTerm(char* s) {
    // s could be %pc, %afp, %agp, %CF, %ZF, %OF, %NF
    if (s[2] == 'F') {
        if (s[1] <= 'N') {
            if (s[1] == 'C') return opCF;
            return opNF;
        } else {
            if (s[1] == 'O') return opOF;
            return opZF;
        }
    }
    if (s[1] == 'p') return opPC;
    if (s[2] == 'f') return opAFP;
    if (s[2] == 'g') return opAGP;
    return (OPER) 0;
}

/*==============================================================================
 * FUNCTION:        listExpToExp
 * OVERVIEW:        Convert a list of actual parameters in the form of a
 *                    STL list of Exps into one expression (using opList)
 * NOTE:            The expressions in the list are not cloned; they are
 *                    simply copied to the new opList
 * PARAMETERS:      le: the list of expressions
 * RETURNS:         The opList Expression
 *============================================================================*/
Exp* listExpToExp(std::list<Exp*>* le) {
    Exp* e;
    Exp** cur = &e;
    for (std::list<Exp*>::iterator it = le->begin(); it != le->end(); it++) {
        *cur = new Binary(opList);
        ((Binary*)*cur)->setSubExp1(*it);
        // cur becomes the address of the address of the second subexpression
        // In other words, cur becomes a reference to the second subexp ptr
        // Note that declaring cur as a reference doesn't work (remains a
        // reference to e)
        cur = &(*cur)->refSubExp2();
    }
    *cur = new Terminal(opNil);         // Terminate the chain
    return e;
}

/*==============================================================================
 * FUNCTION:        listStrToExp
 * OVERVIEW:        Convert a list of formal parameters in the form of a
 *                    STL list of strings into one expression (using opList)
 * PARAMETERS:      ls - the list of strings
 * RETURNS:         The opList expression
 *============================================================================*/
Exp* listStrToExp(std::list<std::string>* ls) {
    Exp* e;
    Exp** cur = &e;
    for (std::list<std::string>::iterator it = ls->begin(); it != ls->end(); it++) {
        *cur = new Binary(opList);
        // *it is a string. Convert it to a parameter
        ((Binary*)*cur)->setSubExp1(new Unary(opParam,
          new Const((char*)(*it).c_str())));
        cur = &(*cur)->refSubExp2();
    }
    *cur = new Terminal(opNil);          // Terminate the chain
    return e;
}

/*==============================================================================
 * FUNCTION:        SSLParser::expandTables
 * OVERVIEW:        Expand tables in an RTL and save to dictionary
 * NOTE:            This may generate many entries
 * PARAMETERS:      iname: Parser object representing the instruction name
 *                  params: Parser object representing the instruction params
 *                  o_rtlist: Original rtlist object (before expanding)
 *                  Dict: Ref to the dictionary that will contain the results
 *                    of the parse
 * RETURNS:         <nothing>
 *============================================================================*/
void SSLParser::expandTables(InsNameElem* iname, std::list<std::string>* params,
  RTL* o_rtlist, RTLInstDict& Dict)
{
    int i, m;
    std::string nam;
    std::ostringstream o;
    m = iname->ninstructions();
    // Expand the tables (if any) in this instruction
    for (i = 0, iname->reset(); i < m; i++, iname->increment()) {
        nam = iname->getinstruction();
        // Need to make substitutions to a copy of the RTL
        RTL* rtl = o_rtlist->clone();
        int n = rtl->getNumExp();
        Exp* srchExpr = new Binary(opExpTable, new Terminal(opWild),
            new Terminal(opWild));
        Exp* srchOp = new Ternary(opOpTable, new Terminal(opWild),
            new Terminal(opWild), new Terminal(opWild));
        for (int j=0; j < n; j++) {
            Exp* e = rtl->elementAt(j);
            std::list<Exp*> le;
            // Expression tables
            if (e->searchAll(srchExpr, le)) {
                std::list<Exp*>::iterator it;
                for (it = le.begin(); it != le.end(); it++) {
                    char* tbl = ((Const*)((Binary*)*it)->getSubExp1())
                      ->getStr();
                    char* idx = ((Const*)((Binary*)*it)->getSubExp2())
                      ->getStr();
                    Exp* repl =((ExprTable*)(TableDict[tbl]))
                      ->expressions[indexrefmap[idx]->getvalue()];
                    bool ch;
                    e = e->searchReplace(*it, repl, ch);
                    // Just in case the top level is changed...
                    rtl->updateExp(e, j);
                }
            }
            // Operator tables
			Exp* res;
			while (e->search(srchOp, res)) {
				Ternary* t;
				if (res->getOper() == opTypedExp)
				   t = (Ternary *)res->getSubExp1();
				else
				   t = (Ternary *)res;
				assert(t->getOper() == opOpTable);
                // The ternary opOpTable has a table and index
                // name as strings, then a list of 2 expressions
                // (and we want to replace it with e1 OP e2)
                char* tbl = ((Const*)t->getSubExp1()) ->getStr();
                char* idx = ((Const*)t->getSubExp2()) ->getStr();
                // The expressions to operate on are in the list
                Binary* b = (Binary*)t->getSubExp3();
                assert(b->getOper() == opList);
                Exp* e1 = b->getSubExp1();
                Exp* e2 = b->getSubExp2();  // This should be an opList too
                assert(b->getOper() == opList);
                e2 = ((Binary*)e2)->getSubExp1();
                const char* ops = ((OpTable*)(TableDict[tbl]))
                  ->records[indexrefmap[idx]->getvalue()].c_str();
                Exp* repl = new Binary(strToOper(ops), e1->clone(),
                e2->clone());
                bool ch;
                e = e->searchReplace(res, repl, ch);
                // In case the top level is changed (common)
                rtl->updateExp(e, j);
            }
        }
   
        if (Dict.appendToDict(nam, *params, *rtl) != 0) {
            o << "Pattern " << iname->getinspattern()
              << " conflicts with an earlier declaration of " << nam <<
              ".\n";
            yyerror(STR(o));
        }
    }
    delete iname;
    delete params;
    delete o_rtlist;
    indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
}
