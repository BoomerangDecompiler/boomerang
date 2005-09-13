#define YY_TransformationParser_h_included

/*  A Bison++ parser, made from transformation.y  */

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
#line 85 "transformation-parser.cpp"
#define YY_TransformationParser_DEBUG  1
#define YY_TransformationParser_PARSE_PARAM 
#define YY_TransformationParser_CONSTRUCTOR_PARAM  \
    std::istream &in, bool trace
#define YY_TransformationParser_CONSTRUCTOR_INIT 
#define YY_TransformationParser_CONSTRUCTOR_CODE  \
    theScanner = new TransformationScanner(in, trace); \
    if (trace) yydebug = 1; else yydebug = 0;
#define YY_TransformationParser_MEMBERS  \
private:        \
    TransformationScanner *theScanner; \
public: \
    virtual ~TransformationParser();
#line 32 "transformation.y"

  #include <list>
  #include <string>
  #include "exp.h"
  #include "type.h"
  #include "cfg.h"
  #include "proc.h"
  #include "signature.h"
  // For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
  #if defined(_MSC_VER) && _MSC_VER <= 1100
  #include "rtl.h"
  #endif
  #include "transformer.h"
  #include "generic.h"

  class TransformationScanner;

#line 69 "transformation.y"
typedef union {
   int ival;
   char *str;
   Type *type;
   Exp *exp;
} yy_TransformationParser_stype;
#define YY_TransformationParser_STYPE yy_TransformationParser_stype
#line 76 "transformation.y"

#include "transformation-scanner.h"

#line 73 "/usr/local/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_TransformationParser_BISON 1
#ifndef YY_TransformationParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_TransformationParser_COMPATIBILITY 1
#else
#define  YY_TransformationParser_COMPATIBILITY 0
#endif
#endif

#if YY_TransformationParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_TransformationParser_LTYPE
#define YY_TransformationParser_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_TransformationParser_STYPE 
#define YY_TransformationParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_TransformationParser_DEBUG
#define  YY_TransformationParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_TransformationParser_STYPE
#ifndef yystype
#define yystype YY_TransformationParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_TransformationParser_USE_GOTO
#define YY_TransformationParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_TransformationParser_USE_GOTO
#define YY_TransformationParser_USE_GOTO 0
#endif

#ifndef YY_TransformationParser_PURE

/* #line 117 "/usr/local/lib/bison.cc" */
#line 176 "transformation-parser.cpp"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_TransformationParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 185 "transformation-parser.cpp"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_TransformationParser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 192 "transformation-parser.cpp"

#line 123 "/usr/local/lib/bison.cc"
/* YY_TransformationParser_DEBUG */
#endif


#ifndef YY_TransformationParser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 202 "transformation-parser.cpp"

#line 128 "/usr/local/lib/bison.cc"
 /* YY_TransformationParser_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_TransformationParser_LSP_NEEDED
#ifndef YY_TransformationParser_LTYPE
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

#define YY_TransformationParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_TransformationParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_TransformationParser_STYPE
#define YY_TransformationParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_TransformationParser_PARSE
#define YY_TransformationParser_PARSE yyparse
#endif
#ifndef YY_TransformationParser_LEX
#define YY_TransformationParser_LEX yylex
#endif
#ifndef YY_TransformationParser_LVAL
#define YY_TransformationParser_LVAL yylval
#endif
#ifndef YY_TransformationParser_LLOC
#define YY_TransformationParser_LLOC yylloc
#endif
#ifndef YY_TransformationParser_CHAR
#define YY_TransformationParser_CHAR yychar
#endif
#ifndef YY_TransformationParser_NERRS
#define YY_TransformationParser_NERRS yynerrs
#endif
#ifndef YY_TransformationParser_DEBUG_FLAG
#define YY_TransformationParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_TransformationParser_ERROR
#define YY_TransformationParser_ERROR yyerror
#endif
#ifndef YY_TransformationParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_TransformationParser_PARSE_PARAM
#ifndef YY_TransformationParser_PARSE_PARAM_DEF
#define YY_TransformationParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_TransformationParser_PARSE_PARAM
#define YY_TransformationParser_PARSE_PARAM void
#endif
#endif
#if YY_TransformationParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_TransformationParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_TransformationParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_TransformationParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_TransformationParser_PURE
#ifndef YYPURE
#define YYPURE YY_TransformationParser_PURE
#endif
#endif
#ifdef YY_TransformationParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_TransformationParser_DEBUG 
#endif
#endif
#ifndef YY_TransformationParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_TransformationParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_TransformationParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_TransformationParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 236 "/usr/local/lib/bison.cc" */
#line 315 "transformation-parser.cpp"
#define	SIZEOF	258
#define	KIND	259
#define	POINTER	260
#define	COMPOUND	261
#define	ARRAY	262
#define	TYPE	263
#define	FUNC	264
#define	WHERE	265
#define	BECOMES	266
#define	REGOF	267
#define	MEMOF	268
#define	ADDROF	269
#define	CONSTANT	270
#define	IDENTIFIER	271
#define	STRING_LITERAL	272
#define	PTR_OP	273
#define	INC_OP	274
#define	DEC_OP	275
#define	LEFT_OP	276
#define	RIGHT_OP	277
#define	LE_OP	278
#define	GE_OP	279
#define	EQ_OP	280
#define	NE_OP	281
#define	AND_OP	282
#define	OR_OP	283
#define	MUL_ASSIGN	284
#define	DIV_ASSIGN	285
#define	MOD_ASSIGN	286
#define	ADD_ASSIGN	287
#define	SUB_ASSIGN	288
#define	LEFT_ASSIGN	289
#define	RIGHT_ASSIGN	290
#define	AND_ASSIGN	291
#define	XOR_ASSIGN	292
#define	OR_ASSIGN	293
#define	TYPE_NAME	294
#define	STRUCT	295
#define	UNION	296
#define	ENUM	297
#define	ELLIPSIS	298
#define	BOOL_TRUE	299
#define	BOOL_FALSE	300


#line 236 "/usr/local/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_TransformationParser_CLASS
#define YY_TransformationParser_CLASS TransformationParser
#endif
#ifndef YY_TransformationParser_INHERIT
#define YY_TransformationParser_INHERIT
#endif
#ifndef YY_TransformationParser_MEMBERS
#define YY_TransformationParser_MEMBERS 
#endif
#ifndef YY_TransformationParser_LEX_BODY
#define YY_TransformationParser_LEX_BODY  
#endif
#ifndef YY_TransformationParser_ERROR_BODY
#define YY_TransformationParser_ERROR_BODY  
#endif
#ifndef YY_TransformationParser_CONSTRUCTOR_PARAM
#define YY_TransformationParser_CONSTRUCTOR_PARAM
#endif
#ifndef YY_TransformationParser_CONSTRUCTOR_CODE
#define YY_TransformationParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_TransformationParser_CONSTRUCTOR_INIT
#define YY_TransformationParser_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_TransformationParser_USE_CONST_TOKEN
#define YY_TransformationParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */ 
#endif
#if YY_TransformationParser_USE_CONST_TOKEN != 0
#ifndef YY_TransformationParser_ENUM_TOKEN
#define YY_TransformationParser_ENUM_TOKEN yy_TransformationParser_enum_token
#endif
#endif

class YY_TransformationParser_CLASS YY_TransformationParser_INHERIT
{
public: 
#if YY_TransformationParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 280 "/usr/local/lib/bison.cc" */
#line 408 "transformation-parser.cpp"
static const int SIZEOF;
static const int KIND;
static const int POINTER;
static const int COMPOUND;
static const int ARRAY;
static const int TYPE;
static const int FUNC;
static const int WHERE;
static const int BECOMES;
static const int REGOF;
static const int MEMOF;
static const int ADDROF;
static const int CONSTANT;
static const int IDENTIFIER;
static const int STRING_LITERAL;
static const int PTR_OP;
static const int INC_OP;
static const int DEC_OP;
static const int LEFT_OP;
static const int RIGHT_OP;
static const int LE_OP;
static const int GE_OP;
static const int EQ_OP;
static const int NE_OP;
static const int AND_OP;
static const int OR_OP;
static const int MUL_ASSIGN;
static const int DIV_ASSIGN;
static const int MOD_ASSIGN;
static const int ADD_ASSIGN;
static const int SUB_ASSIGN;
static const int LEFT_ASSIGN;
static const int RIGHT_ASSIGN;
static const int AND_ASSIGN;
static const int XOR_ASSIGN;
static const int OR_ASSIGN;
static const int TYPE_NAME;
static const int STRUCT;
static const int UNION;
static const int ENUM;
static const int ELLIPSIS;
static const int BOOL_TRUE;
static const int BOOL_FALSE;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_TransformationParser_ENUM_TOKEN { YY_TransformationParser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 460 "transformation-parser.cpp"
	,SIZEOF=258
	,KIND=259
	,POINTER=260
	,COMPOUND=261
	,ARRAY=262
	,TYPE=263
	,FUNC=264
	,WHERE=265
	,BECOMES=266
	,REGOF=267
	,MEMOF=268
	,ADDROF=269
	,CONSTANT=270
	,IDENTIFIER=271
	,STRING_LITERAL=272
	,PTR_OP=273
	,INC_OP=274
	,DEC_OP=275
	,LEFT_OP=276
	,RIGHT_OP=277
	,LE_OP=278
	,GE_OP=279
	,EQ_OP=280
	,NE_OP=281
	,AND_OP=282
	,OR_OP=283
	,MUL_ASSIGN=284
	,DIV_ASSIGN=285
	,MOD_ASSIGN=286
	,ADD_ASSIGN=287
	,SUB_ASSIGN=288
	,LEFT_ASSIGN=289
	,RIGHT_ASSIGN=290
	,AND_ASSIGN=291
	,XOR_ASSIGN=292
	,OR_ASSIGN=293
	,TYPE_NAME=294
	,STRUCT=295
	,UNION=296
	,ENUM=297
	,ELLIPSIS=298
	,BOOL_TRUE=299
	,BOOL_FALSE=300


#line 283 "/usr/local/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_TransformationParser_PARSE (YY_TransformationParser_PARSE_PARAM);
 virtual void YY_TransformationParser_ERROR(char *msg) YY_TransformationParser_ERROR_BODY;
#ifdef YY_TransformationParser_PURE
#ifdef YY_TransformationParser_LSP_NEEDED
 virtual int  YY_TransformationParser_LEX (YY_TransformationParser_STYPE *YY_TransformationParser_LVAL,YY_TransformationParser_LTYPE *YY_TransformationParser_LLOC) YY_TransformationParser_LEX_BODY;
#else
 virtual int  YY_TransformationParser_LEX (YY_TransformationParser_STYPE *YY_TransformationParser_LVAL) YY_TransformationParser_LEX_BODY;
#endif
#else
 virtual int YY_TransformationParser_LEX() YY_TransformationParser_LEX_BODY;
 YY_TransformationParser_STYPE YY_TransformationParser_LVAL;
#ifdef YY_TransformationParser_LSP_NEEDED
 YY_TransformationParser_LTYPE YY_TransformationParser_LLOC;
#endif
 int   YY_TransformationParser_NERRS;
 int    YY_TransformationParser_CHAR;
#endif
#if YY_TransformationParser_DEBUG != 0
 int YY_TransformationParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_TransformationParser_CLASS(YY_TransformationParser_CONSTRUCTOR_PARAM);
public:
 YY_TransformationParser_MEMBERS 
};
/* other declare folow */
#if YY_TransformationParser_USE_CONST_TOKEN != 0

/* #line 314 "/usr/local/lib/bison.cc" */
#line 540 "transformation-parser.cpp"
const int YY_TransformationParser_CLASS::SIZEOF=258;
const int YY_TransformationParser_CLASS::KIND=259;
const int YY_TransformationParser_CLASS::POINTER=260;
const int YY_TransformationParser_CLASS::COMPOUND=261;
const int YY_TransformationParser_CLASS::ARRAY=262;
const int YY_TransformationParser_CLASS::TYPE=263;
const int YY_TransformationParser_CLASS::FUNC=264;
const int YY_TransformationParser_CLASS::WHERE=265;
const int YY_TransformationParser_CLASS::BECOMES=266;
const int YY_TransformationParser_CLASS::REGOF=267;
const int YY_TransformationParser_CLASS::MEMOF=268;
const int YY_TransformationParser_CLASS::ADDROF=269;
const int YY_TransformationParser_CLASS::CONSTANT=270;
const int YY_TransformationParser_CLASS::IDENTIFIER=271;
const int YY_TransformationParser_CLASS::STRING_LITERAL=272;
const int YY_TransformationParser_CLASS::PTR_OP=273;
const int YY_TransformationParser_CLASS::INC_OP=274;
const int YY_TransformationParser_CLASS::DEC_OP=275;
const int YY_TransformationParser_CLASS::LEFT_OP=276;
const int YY_TransformationParser_CLASS::RIGHT_OP=277;
const int YY_TransformationParser_CLASS::LE_OP=278;
const int YY_TransformationParser_CLASS::GE_OP=279;
const int YY_TransformationParser_CLASS::EQ_OP=280;
const int YY_TransformationParser_CLASS::NE_OP=281;
const int YY_TransformationParser_CLASS::AND_OP=282;
const int YY_TransformationParser_CLASS::OR_OP=283;
const int YY_TransformationParser_CLASS::MUL_ASSIGN=284;
const int YY_TransformationParser_CLASS::DIV_ASSIGN=285;
const int YY_TransformationParser_CLASS::MOD_ASSIGN=286;
const int YY_TransformationParser_CLASS::ADD_ASSIGN=287;
const int YY_TransformationParser_CLASS::SUB_ASSIGN=288;
const int YY_TransformationParser_CLASS::LEFT_ASSIGN=289;
const int YY_TransformationParser_CLASS::RIGHT_ASSIGN=290;
const int YY_TransformationParser_CLASS::AND_ASSIGN=291;
const int YY_TransformationParser_CLASS::XOR_ASSIGN=292;
const int YY_TransformationParser_CLASS::OR_ASSIGN=293;
const int YY_TransformationParser_CLASS::TYPE_NAME=294;
const int YY_TransformationParser_CLASS::STRUCT=295;
const int YY_TransformationParser_CLASS::UNION=296;
const int YY_TransformationParser_CLASS::ENUM=297;
const int YY_TransformationParser_CLASS::ELLIPSIS=298;
const int YY_TransformationParser_CLASS::BOOL_TRUE=299;
const int YY_TransformationParser_CLASS::BOOL_FALSE=300;


#line 314 "/usr/local/lib/bison.cc"
 /* const YY_TransformationParser_CLASS::token */
#endif
/*apres const  */
YY_TransformationParser_CLASS::YY_TransformationParser_CLASS(YY_TransformationParser_CONSTRUCTOR_PARAM) YY_TransformationParser_CONSTRUCTOR_INIT
{
#if YY_TransformationParser_DEBUG != 0
YY_TransformationParser_DEBUG_FLAG=0;
#endif
YY_TransformationParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 325 "/usr/local/lib/bison.cc" */
#line 600 "transformation-parser.cpp"


#define	YYFINAL		72
#define	YYFLAG		-32768
#define	YYNTBASE	59

#define YYTRANSLATE(x) ((unsigned)(x) <= 300 ? yytranslate[x] : 64)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    57,     2,     2,     2,     2,    50,     2,    56,
    55,    49,    47,    58,    48,    54,    53,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    46,    52,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    51,     2,     2,     2,     2,     2,     2,
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
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45
};

#if YY_TransformationParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     3,     4,     9,    12,    13,    17,    21,    25,    29,
    33,    37,    41,    45,    49,    53,    57,    61,    65,    69,
    73,    75,    79,    81,    85,    89,    93,    96,    99,   101,
   105,   107,   109,   113,   115
};

static const short yyrhs[] = {    60,
    59,     0,     0,    62,    61,    11,    62,     0,    10,    62,
     0,     0,    12,    15,    46,     0,    13,    62,    46,     0,
    14,    62,    46,     0,    62,    47,    62,     0,    62,    48,
    62,     0,    62,    49,    62,     0,    62,    50,    62,     0,
    62,    51,    62,     0,    62,    52,    62,     0,    62,    53,
    62,     0,    62,    27,    62,     0,    62,    28,    62,     0,
    62,    25,    62,     0,    62,    26,    62,     0,    62,    54,
    62,     0,    15,     0,     9,    62,    55,     0,    16,     0,
    56,    62,    55,     0,     4,    62,    55,     0,     8,    62,
    55,     0,    48,    62,     0,    57,    62,     0,    63,     0,
    62,    58,    62,     0,    44,     0,    45,     0,     5,    63,
    55,     0,     6,     0,    16,     0
};

#endif

#if YY_TransformationParser_DEBUG != 0
static const short yyrline[] = { 0,
    87,    89,    93,    97,    99,   103,   106,   109,   112,   115,
   118,   121,   124,   127,   130,   133,   136,   139,   142,   145,
   148,   151,   154,   161,   164,   167,   170,   173,   176,   179,
   182,   185,   190,   193,   196
};

static const char * const yytname[] = {   "$","error","$illegal.","SIZEOF","KIND",
"POINTER","COMPOUND","ARRAY","TYPE","FUNC","WHERE","BECOMES","REGOF","MEMOF",
"ADDROF","CONSTANT","IDENTIFIER","STRING_LITERAL","PTR_OP","INC_OP","DEC_OP",
"LEFT_OP","RIGHT_OP","LE_OP","GE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","MUL_ASSIGN",
"DIV_ASSIGN","MOD_ASSIGN","ADD_ASSIGN","SUB_ASSIGN","LEFT_ASSIGN","RIGHT_ASSIGN",
"AND_ASSIGN","XOR_ASSIGN","OR_ASSIGN","TYPE_NAME","STRUCT","UNION","ENUM","ELLIPSIS",
"BOOL_TRUE","BOOL_FALSE","']'","'+'","'-'","'*'","'&'","'|'","'^'","'/'","'.'",
"')'","'('","'!'","','","translation_unit","transformation","optional_where_clause",
"exp","type",""
};
#endif

static const short yyr1[] = {     0,
    59,    59,    60,    61,    61,    62,    62,    62,    62,    62,
    62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
    62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
    62,    62,    63,    63,    63
};

static const short yyr2[] = {     0,
     2,     0,     4,     2,     0,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     1,     3,     1,     3,     3,     3,     2,     2,     1,     3,
     1,     1,     3,     1,     1
};

static const short yydefact[] = {     2,
     0,     0,    34,     0,     0,     0,     0,     0,    21,    23,
    31,    32,     0,     0,     0,     2,     5,    29,     0,    35,
     0,     0,     0,     0,     0,     0,    27,     0,    28,     1,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    25,    33,    26,    22,     6,
     7,     8,    24,     4,    18,    19,    16,    17,     9,    10,
    11,    12,    13,    14,    15,    20,    30,     0,     3,     0,
     0,     0
};

static const short yydefgoto[] = {    30,
    16,    45,    17,    18
};

static const short yypact[] = {    12,
    12,     3,-32768,    12,    12,   -14,    12,    12,-32768,-32768,
-32768,-32768,    12,    12,    12,    12,    36,-32768,    23,-32768,
   -53,    70,    82,   -41,   116,   129,   175,   163,   175,-32768,
    12,    12,    12,    12,    12,    12,    12,    12,    12,    12,
    12,    12,    12,    12,     0,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   175,   175,   175,   175,   175,   175,   175,
   175,   175,   175,   175,   175,   175,   175,    12,   175,    10,
    15,-32768
};

static const short yypgoto[] = {    22,
-32768,-32768,    -1,    21
};


#define	YYLAST		233


static const short yytable[] = {    19,
    24,    47,    22,    23,    50,    25,    26,     2,     3,    71,
    68,    27,    28,    29,    72,     1,     2,     3,    20,     4,
     5,    70,    21,     6,     7,     8,     9,    10,     0,    54,
    55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
    65,    66,    67,     0,     0,    31,     0,    32,    33,    34,
    35,     0,     0,     0,     0,    11,    12,     0,     0,    13,
    32,    33,    34,    35,     0,     0,    69,    14,    15,    36,
    37,    38,    39,    40,    41,    42,    43,    46,     0,     0,
    44,     0,    36,    37,    38,    39,    40,    41,    42,    43,
     0,     0,     0,    44,    32,    33,    34,    35,     0,     0,
     0,     0,     0,     0,     0,     0,    32,    33,    34,    35,
     0,     0,     0,     0,     0,     0,    36,    37,    38,    39,
    40,    41,    42,    43,    48,     0,     0,    44,    36,    37,
    38,    39,    40,    41,    42,    43,    49,     0,     0,    44,
    32,    33,    34,    35,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    32,    33,    34,    35,     0,     0,     0,
     0,    51,    36,    37,    38,    39,    40,    41,    42,    43,
     0,     0,     0,    44,    52,    36,    37,    38,    39,    40,
    41,    42,    43,     0,     0,     0,    44,    32,    33,    34,
    35,     0,     0,     0,     0,     0,     0,     0,     0,    32,
    33,    34,    35,     0,     0,     0,     0,     0,     0,    36,
    37,    38,    39,    40,    41,    42,    43,    53,     0,     0,
    44,    36,    37,    38,    39,    40,    41,    42,    43,     0,
     0,     0,    44
};

static const short yycheck[] = {     1,
    15,    55,     4,     5,    46,     7,     8,     5,     6,     0,
    11,    13,    14,    15,     0,     4,     5,     6,    16,     8,
     9,     0,     2,    12,    13,    14,    15,    16,    -1,    31,
    32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    44,    -1,    -1,    10,    -1,    25,    26,    27,
    28,    -1,    -1,    -1,    -1,    44,    45,    -1,    -1,    48,
    25,    26,    27,    28,    -1,    -1,    68,    56,    57,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
    58,    -1,    47,    48,    49,    50,    51,    52,    53,    54,
    -1,    -1,    -1,    58,    25,    26,    27,    28,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    25,    26,    27,    28,
    -1,    -1,    -1,    -1,    -1,    -1,    47,    48,    49,    50,
    51,    52,    53,    54,    55,    -1,    -1,    58,    47,    48,
    49,    50,    51,    52,    53,    54,    55,    -1,    -1,    58,
    25,    26,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    25,    26,    27,    28,    -1,    -1,    -1,
    -1,    46,    47,    48,    49,    50,    51,    52,    53,    54,
    -1,    -1,    -1,    58,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    -1,    -1,    -1,    58,    25,    26,    27,
    28,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    25,
    26,    27,    28,    -1,    -1,    -1,    -1,    -1,    -1,    47,
    48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
    58,    47,    48,    49,    50,    51,    52,    53,    54,    -1,
    -1,    -1,    58
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

#if YY_TransformationParser_USE_GOTO != 0
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
static int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_TransformationParser_LSP_NEEDED
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
#define yyclearin       (YY_TransformationParser_CHAR = YYEMPTY)
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
  if (YY_TransformationParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_TransformationParser_CHAR = (token), YY_TransformationParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_TransformationParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_TransformationParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_TransformationParser_PURE
/* UNPURE */
#define YYLEX           YY_TransformationParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_TransformationParser_CHAR;                      /*  the lookahead symbol        */
YY_TransformationParser_STYPE      YY_TransformationParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_TransformationParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_TransformationParser_LSP_NEEDED
YY_TransformationParser_LTYPE YY_TransformationParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_TransformationParser_LSP_NEEDED
#define YYLEX           YY_TransformationParser_LEX(&YY_TransformationParser_LVAL, &YY_TransformationParser_LLOC)
#else
#define YYLEX           YY_TransformationParser_LEX(&YY_TransformationParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_TransformationParser_DEBUG != 0
int YY_TransformationParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
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
 YY_TransformationParser_CLASS::
#endif
     YY_TransformationParser_PARSE(YY_TransformationParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_TransformationParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_TransformationParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_TransformationParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_TransformationParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_TransformationParser_LSP_NEEDED
  YY_TransformationParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_TransformationParser_LTYPE *yyls = yylsa;
  YY_TransformationParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_TransformationParser_PURE
  int YY_TransformationParser_CHAR;
  YY_TransformationParser_STYPE YY_TransformationParser_LVAL;
  int YY_TransformationParser_NERRS;
#ifdef YY_TransformationParser_LSP_NEEDED
  YY_TransformationParser_LTYPE YY_TransformationParser_LLOC;
#endif
#endif

  YY_TransformationParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_TransformationParser_NERRS = 0;
  YY_TransformationParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_TransformationParser_LSP_NEEDED
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
      YY_TransformationParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_TransformationParser_LSP_NEEDED
      YY_TransformationParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_TransformationParser_LSP_NEEDED
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
#ifdef YY_TransformationParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_TransformationParser_ERROR("parser stack overflow");
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_TransformationParser_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_TransformationParser_LSP_NEEDED
      yyls = (YY_TransformationParser_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_TransformationParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_TransformationParser_DEBUG != 0
      if (YY_TransformationParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
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

  if (YY_TransformationParser_CHAR == YYEMPTY)
    {
#if YY_TransformationParser_DEBUG != 0
      if (YY_TransformationParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_TransformationParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_TransformationParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_TransformationParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_TransformationParser_DEBUG != 0
      if (YY_TransformationParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_TransformationParser_CHAR);

#if YY_TransformationParser_DEBUG != 0
      if (YY_TransformationParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_TransformationParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_TransformationParser_CHAR, YY_TransformationParser_LVAL);
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

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_TransformationParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_TransformationParser_CHAR != YYEOF)
    YY_TransformationParser_CHAR = YYEMPTY;

  *++yyvsp = YY_TransformationParser_LVAL;
#ifdef YY_TransformationParser_LSP_NEEDED
  *++yylsp = YY_TransformationParser_LLOC;
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

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
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
#line 1278 "transformation-parser.cpp"

  switch (yyn) {

case 1:
#line 88 "transformation.y"
{ ;
    break;}
case 2:
#line 90 "transformation.y"
{ ;
    break;}
case 3:
#line 94 "transformation.y"
{ new GenericExpTransformer(yyvsp[-3].exp, yyvsp[-2].exp, yyvsp[0].exp); ;
    break;}
case 4:
#line 98 "transformation.y"
{ yyval.exp = yyvsp[0].exp; ;
    break;}
case 5:
#line 100 "transformation.y"
{ yyval.exp = NULL; ;
    break;}
case 6:
#line 104 "transformation.y"
{ yyval.exp = Location::regOf(yyvsp[-1].ival);
    ;
    break;}
case 7:
#line 107 "transformation.y"
{ yyval.exp = Location::memOf(yyvsp[-1].exp);
    ;
    break;}
case 8:
#line 110 "transformation.y"
{ yyval.exp = new Unary(opAddrOf, yyvsp[-1].exp);
    ;
    break;}
case 9:
#line 113 "transformation.y"
{ yyval.exp = new Binary(opPlus, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 10:
#line 116 "transformation.y"
{ yyval.exp = new Binary(opMinus, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 11:
#line 119 "transformation.y"
{ yyval.exp = new Binary(opMult, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 12:
#line 122 "transformation.y"
{ yyval.exp = new Binary(opBitAnd, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 13:
#line 125 "transformation.y"
{ yyval.exp = new Binary(opBitOr, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 14:
#line 128 "transformation.y"
{ yyval.exp = new Binary(opBitXor, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 15:
#line 131 "transformation.y"
{ yyval.exp = new Binary(opDiv, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 16:
#line 134 "transformation.y"
{ yyval.exp = new Binary(opAnd, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 17:
#line 137 "transformation.y"
{ yyval.exp = new Binary(opOr, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 18:
#line 140 "transformation.y"
{ yyval.exp = new Binary(opEquals, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 19:
#line 143 "transformation.y"
{ yyval.exp = new Binary(opNotEqual, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 20:
#line 146 "transformation.y"
{ yyval.exp = new Binary(opMemberAccess, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 21:
#line 149 "transformation.y"
{ yyval.exp = new Const(yyvsp[0].ival);
    ;
    break;}
case 22:
#line 152 "transformation.y"
{ yyval.exp = new Binary(opFlagCall, new Const(yyvsp[-2].str), yyvsp[-1].exp);
    ;
    break;}
case 23:
#line 155 "transformation.y"
{ 
      if (strlen(yyvsp[0].str) > 2 && yyvsp[0].str[0] == 'o' && yyvsp[0].str[1] == 'p')  
          yyval.exp = new Const(yyvsp[0].str); // treat op* as a string constant
      else
          yyval.exp = new Unary(opVar, new Const(yyvsp[0].str));
    ;
    break;}
case 24:
#line 162 "transformation.y"
{ yyval.exp = yyvsp[-1].exp;
    ;
    break;}
case 25:
#line 165 "transformation.y"
{ yyval.exp = new Unary(opKindOf, yyvsp[-1].exp);
    ;
    break;}
case 26:
#line 168 "transformation.y"
{ yyval.exp = new Unary(opTypeOf, yyvsp[-1].exp);
    ;
    break;}
case 27:
#line 171 "transformation.y"
{ yyval.exp = new Unary(opNeg, yyvsp[0].exp);
    ;
    break;}
case 28:
#line 174 "transformation.y"
{ yyval.exp = new Unary(opLNot, yyvsp[0].exp);
    ;
    break;}
case 29:
#line 177 "transformation.y"
{ yyval.exp = new TypeVal(yyvsp[0].type);
    ;
    break;}
case 30:
#line 180 "transformation.y"
{ yyval.exp = new Binary(opList, yyvsp[-2].exp, new Binary(opList, yyvsp[0].exp, new Terminal(opNil)));
    ;
    break;}
case 31:
#line 183 "transformation.y"
{ yyval.exp = new Terminal(opTrue);
    ;
    break;}
case 32:
#line 186 "transformation.y"
{ yyval.exp = new Terminal(opFalse);
    ;
    break;}
case 33:
#line 191 "transformation.y"
{ yyval.type = new PointerType(yyvsp[-1].type);
    ;
    break;}
case 34:
#line 194 "transformation.y"
{ yyval.type = new CompoundType();
    ;
    break;}
case 35:
#line 197 "transformation.y"
{ yyval.type = new NamedType(yyvsp[0].str);
    ;
    break;}
}

#line 811 "/usr/local/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_TransformationParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_TransformationParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_TransformationParser_LLOC.first_line;
      yylsp->first_column = YY_TransformationParser_LLOC.first_column;
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
      ++YY_TransformationParser_NERRS;

#ifdef YY_TransformationParser_ERROR_VERBOSE
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
	      YY_TransformationParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_TransformationParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_TransformationParser_ERROR_VERBOSE */
	YY_TransformationParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_TransformationParser_CHAR == YYEOF)
	YYABORT;

#if YY_TransformationParser_DEBUG != 0
      if (YY_TransformationParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_TransformationParser_CHAR, yytname[yychar1]);
#endif

      YY_TransformationParser_CHAR = YYEMPTY;
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
#ifdef YY_TransformationParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
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

#if YY_TransformationParser_DEBUG != 0
  if (YY_TransformationParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_TransformationParser_LVAL;
#ifdef YY_TransformationParser_LSP_NEEDED
  *++yylsp = YY_TransformationParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 1010 "/usr/local/lib/bison.cc" */
#line 1660 "transformation-parser.cpp"
#line 201 "transformation.y"

#include <stdio.h>

int TransformationParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}

void TransformationParser::yyerror(char *s)
{
	fflush(stdout);
        printf("\n%s", theScanner->lineBuf);
	printf("\n%*s\n%*s on line %i\n", theScanner->column, "^", theScanner->column, s, theScanner->theLine);
}

TransformationParser::~TransformationParser()
{
    // Suppress warnings from gcc about lack of virtual destructor
}


