#define YY_SSLParser_h_included

/*  A Bison++ parser, made from sslparser.y  */

 /* with Bison++ version bison++ Version 1.21-7, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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
#ifndef _MSDOS
#ifdef MSDOS
#define _MSDOS
#endif
#endif
/* turboc */
#ifdef __MSDOS__
#ifndef _MSDOS
#define _MSDOS
#endif
#endif

#ifndef alloca
#if defined( __GNUC__)
#define alloca __builtin_alloca

#elif (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc)  || defined (__sgi)
#include <alloca.h>

#elif defined (_MSDOS)
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

/* #line 77 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 89 "sslparser.cpp"
#line 35 "sslparser.y"

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

#line 60 "sslparser.y"
typedef union {
    Exp*            exp;
    char*           str;
    int             num;
    double          dbl;
    Statement*      regtransfer;
    
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
#line 78 "sslparser.y"

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
static  Statement* parseExp(const char *str); /* Parse an expression or assignment from a string */ \
/* The code for expanding tables and saving to the dictionary */ \
void    expandTables(InsNameElem* iname, std::list<std::string>* params, RTL* o_rtlist, \
  RTLInstDict& Dict); \
Exp*	makeSuccessor(Exp* e);	/* Get successor (of register expression) */ \
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
     * Result for parsing an assignment. \
     */ \
    Statement *the_asgn; \
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

#line 77 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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

/* #line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 247 "sslparser.cpp"

#line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/*  YY_SSLParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 256 "sslparser.cpp"

#line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* prefix */
#ifndef YY_SSLParser_DEBUG

/* #line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 263 "sslparser.cpp"

#line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* YY_SSLParser_DEBUG */
#endif


#ifndef YY_SSLParser_LSP_NEEDED

/* #line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 273 "sslparser.cpp"

#line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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

/* #line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 386 "sslparser.cpp"
#define	COND_OP	258
#define	BIT_OP	259
#define	ARITH_OP	260
#define	LOG_OP	261
#define	NAME	262
#define	REG_ID	263
#define	REG_NUM	264
#define	COND_TNAME	265
#define	DECOR	266
#define	FARITH_OP	267
#define	FPUSH	268
#define	FPOP	269
#define	TEMP	270
#define	SHARES	271
#define	CONV_FUNC	272
#define	TRANSCEND	273
#define	BIG	274
#define	LITTLE	275
#define	NAME_CALL	276
#define	NAME_LOOKUP	277
#define	ENDIANNESS	278
#define	COVERS	279
#define	INDEX	280
#define	NOT	281
#define	THEN	282
#define	LOOKUP_RDC	283
#define	BOGUS	284
#define	ASSIGN	285
#define	TO	286
#define	COLON	287
#define	S_E	288
#define	AT	289
#define	ADDR	290
#define	REG_IDX	291
#define	EQUATE	292
#define	MEM_IDX	293
#define	TOK_INTEGER	294
#define	TOK_FLOAT	295
#define	FAST	296
#define	OPERAND	297
#define	FETCHEXEC	298
#define	CAST_OP	299
#define	FLAGMACRO	300
#define	SUCCESSOR	301
#define	NUM	302
#define	ASSIGNSIZE	303
#define	FLOATNUM	304


#line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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

/* #line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 483 "sslparser.cpp"
static const int COND_OP;
static const int BIT_OP;
static const int ARITH_OP;
static const int LOG_OP;
static const int NAME;
static const int REG_ID;
static const int REG_NUM;
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
static const int SUCCESSOR;
static const int NUM;
static const int ASSIGNSIZE;
static const int FLOATNUM;


#line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 539 "sslparser.cpp"
	,COND_OP=258
	,BIT_OP=259
	,ARITH_OP=260
	,LOG_OP=261
	,NAME=262
	,REG_ID=263
	,REG_NUM=264
	,COND_TNAME=265
	,DECOR=266
	,FARITH_OP=267
	,FPUSH=268
	,FPOP=269
	,TEMP=270
	,SHARES=271
	,CONV_FUNC=272
	,TRANSCEND=273
	,BIG=274
	,LITTLE=275
	,NAME_CALL=276
	,NAME_LOOKUP=277
	,ENDIANNESS=278
	,COVERS=279
	,INDEX=280
	,NOT=281
	,THEN=282
	,LOOKUP_RDC=283
	,BOGUS=284
	,ASSIGN=285
	,TO=286
	,COLON=287
	,S_E=288
	,AT=289
	,ADDR=290
	,REG_IDX=291
	,EQUATE=292
	,MEM_IDX=293
	,TOK_INTEGER=294
	,TOK_FLOAT=295
	,FAST=296
	,OPERAND=297
	,FETCHEXEC=298
	,CAST_OP=299
	,FLAGMACRO=300
	,SUCCESSOR=301
	,NUM=302
	,ASSIGNSIZE=303
	,FLOATNUM=304


#line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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

/* #line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 623 "sslparser.cpp"
const int YY_SSLParser_CLASS::COND_OP=258;
const int YY_SSLParser_CLASS::BIT_OP=259;
const int YY_SSLParser_CLASS::ARITH_OP=260;
const int YY_SSLParser_CLASS::LOG_OP=261;
const int YY_SSLParser_CLASS::NAME=262;
const int YY_SSLParser_CLASS::REG_ID=263;
const int YY_SSLParser_CLASS::REG_NUM=264;
const int YY_SSLParser_CLASS::COND_TNAME=265;
const int YY_SSLParser_CLASS::DECOR=266;
const int YY_SSLParser_CLASS::FARITH_OP=267;
const int YY_SSLParser_CLASS::FPUSH=268;
const int YY_SSLParser_CLASS::FPOP=269;
const int YY_SSLParser_CLASS::TEMP=270;
const int YY_SSLParser_CLASS::SHARES=271;
const int YY_SSLParser_CLASS::CONV_FUNC=272;
const int YY_SSLParser_CLASS::TRANSCEND=273;
const int YY_SSLParser_CLASS::BIG=274;
const int YY_SSLParser_CLASS::LITTLE=275;
const int YY_SSLParser_CLASS::NAME_CALL=276;
const int YY_SSLParser_CLASS::NAME_LOOKUP=277;
const int YY_SSLParser_CLASS::ENDIANNESS=278;
const int YY_SSLParser_CLASS::COVERS=279;
const int YY_SSLParser_CLASS::INDEX=280;
const int YY_SSLParser_CLASS::NOT=281;
const int YY_SSLParser_CLASS::THEN=282;
const int YY_SSLParser_CLASS::LOOKUP_RDC=283;
const int YY_SSLParser_CLASS::BOGUS=284;
const int YY_SSLParser_CLASS::ASSIGN=285;
const int YY_SSLParser_CLASS::TO=286;
const int YY_SSLParser_CLASS::COLON=287;
const int YY_SSLParser_CLASS::S_E=288;
const int YY_SSLParser_CLASS::AT=289;
const int YY_SSLParser_CLASS::ADDR=290;
const int YY_SSLParser_CLASS::REG_IDX=291;
const int YY_SSLParser_CLASS::EQUATE=292;
const int YY_SSLParser_CLASS::MEM_IDX=293;
const int YY_SSLParser_CLASS::TOK_INTEGER=294;
const int YY_SSLParser_CLASS::TOK_FLOAT=295;
const int YY_SSLParser_CLASS::FAST=296;
const int YY_SSLParser_CLASS::OPERAND=297;
const int YY_SSLParser_CLASS::FETCHEXEC=298;
const int YY_SSLParser_CLASS::CAST_OP=299;
const int YY_SSLParser_CLASS::FLAGMACRO=300;
const int YY_SSLParser_CLASS::SUCCESSOR=301;
const int YY_SSLParser_CLASS::NUM=302;
const int YY_SSLParser_CLASS::ASSIGNSIZE=303;
const int YY_SSLParser_CLASS::FLOATNUM=304;


#line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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

/* #line 329 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 687 "sslparser.cpp"


#define	YYFINAL		290
#define	YYFLAG		-32768
#define	YYNTBASE	63

#define YYTRANSLATE(x) ((unsigned)(x) <= 304 ? yytranslate[x] : 109)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    57,     2,    59,     2,     2,    58,    61,
    56,     2,     2,    51,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    50,     2,
     2,     2,    62,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    54,     2,    55,     2,    60,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    52,     2,    53,     2,     2,     2,     2,     2,
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
    46,    47,    48,    49
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
   397,   400,   403,   406,   410,   414,   418,   422,   426,   432,
   434,   436,   440,   442,   446,   448,   456,   459,   463,   467,
   470,   472,   474,   477,   481,   483
};

static const short yyrhs[] = {    99,
     0,   101,     0,    64,     0,    64,    65,    50,     0,    65,
    50,     0,    88,     0,    43,    93,     0,    76,     0,    77,
     0,   104,     0,   106,     0,    69,     0,    75,     0,    42,
    66,     0,    66,    51,    67,     0,    67,     0,    97,    37,
    52,    96,    53,     0,    97,    96,    68,    48,   101,     0,
    54,    96,    55,     0,     0,     0,    39,    70,    72,     0,
     0,    40,    71,    72,     0,    72,    51,    73,     0,    73,
     0,     8,    25,    47,     0,     8,    54,    47,    55,    25,
    47,     0,     8,    54,    47,    55,    25,    47,    24,     8,
    31,     8,     0,     8,    54,    47,    55,    25,    47,    16,
     8,    34,    54,    47,    31,    47,    55,     0,    54,    74,
    55,    54,    47,    55,    25,    47,    31,    47,     0,    54,
    74,    55,    54,    47,    55,    25,    47,     0,    74,    51,
     8,     0,     8,     0,    21,    96,    56,    52,    93,    53,
     0,     7,    37,    47,     0,     7,    37,    47,     5,    47,
     0,     7,    37,    78,     0,    79,     0,    84,     0,    86,
     0,    79,    81,     0,    81,     0,    80,    51,    79,     0,
    80,    51,    57,    57,     0,    79,     0,    52,    80,    53,
     0,    82,     0,    58,     7,    58,     0,    57,     7,    57,
     0,    59,     7,     0,     7,     0,     4,     0,     5,     0,
    12,     0,    52,    85,    53,     0,    85,    51,    57,    83,
    57,     0,    57,    83,    57,     0,    52,    87,    53,     0,
    87,    51,    57,   101,    57,     0,    57,   101,    57,     0,
     0,    90,    89,    96,    93,     0,    91,     0,    90,    11,
     0,     7,     0,    92,     0,    91,    92,     0,    58,     7,
    58,     0,    22,    47,    55,     0,    22,     7,    55,     0,
    59,    22,    47,    55,     0,    59,    22,     7,    55,     0,
    57,     7,    57,     0,    93,    94,     0,    94,     0,    99,
     0,    21,    98,    56,     0,    45,    95,    56,     0,    45,
    56,     0,    60,     0,    95,    51,     8,     0,     8,     0,
    96,    51,    97,     0,    97,     0,     0,     7,     0,    98,
    51,   101,     0,   101,     0,     0,    48,   101,    27,   102,
    37,   101,     0,    48,   102,    37,   101,     0,    13,     0,
    14,     0,    48,   101,     0,    47,     0,    49,     0,    15,
     0,    61,   101,    56,     0,   102,     0,    54,   101,    62,
   101,    32,   101,    55,     0,    54,   101,    62,   101,    32,
   101,    55,   103,     0,    35,   101,    56,     0,    17,    47,
    51,    47,    51,   101,    56,     0,    13,     0,    14,     0,
    18,   101,    56,     0,    22,     7,    55,     0,    21,    98,
    56,     0,    46,   101,    56,     0,   101,    33,     0,   101,
   103,     0,    26,   101,     0,   101,    12,   101,     0,   101,
     5,   101,     0,   101,     4,   101,     0,   101,     3,   101,
     0,   101,     6,   101,     0,   101,    22,     7,    55,   100,
     0,   100,     0,     8,     0,    36,   101,    55,     0,     9,
     0,    38,   101,    55,     0,     7,     0,   101,    34,    54,
   101,    32,   101,    55,     0,   102,    58,     0,    46,   101,
    56,     0,    52,    47,    53,     0,    23,   105,     0,    19,
     0,    20,     0,    41,   107,     0,   107,    51,   108,     0,
   108,     0,     7,    25,     7,     0
};

#endif

#if YY_SSLParser_DEBUG != 0
static const short yyrline[] = { 0,
   210,   214,   219,   222,   224,   227,   230,   235,   237,   240,
   245,   248,   252,   255,   259,   261,   264,   282,   299,   300,
   303,   307,   307,   310,   312,   313,   316,   322,   327,   362,
   384,   399,   410,   414,   421,   429,   436,   449,   455,   461,
   465,   471,   482,   487,   496,   499,   504,   508,   513,   519,
   522,   537,   555,   560,   564,   570,   576,   582,   589,   595,
   601,   607,   612,   619,   623,   646,   650,   653,   659,   663,
   676,   685,   696,   705,   710,   720,   727,   734,   747,   751,
   754,   759,   767,   777,   784,   788,   793,   798,   803,   808,
   813,   823,   829,   834,   840,   845,   850,   854,   858,   862,
   866,   870,   880,   886,   891,   894,   899,   905,   933,   960,
   965,   975,   983,   987,   991,   995,   999,  1003,  1010,  1036,
  1041,  1071,  1075,  1081,  1085,  1103,  1107,  1110,  1115,  1121,
  1126,  1130,  1137,  1141,  1144,  1148
};

static const char * const yytname[] = {   "$","error","$illegal.","COND_OP",
"BIT_OP","ARITH_OP","LOG_OP","NAME","REG_ID","REG_NUM","COND_TNAME","DECOR",
"FARITH_OP","FPUSH","FPOP","TEMP","SHARES","CONV_FUNC","TRANSCEND","BIG","LITTLE",
"NAME_CALL","NAME_LOOKUP","ENDIANNESS","COVERS","INDEX","NOT","THEN","LOOKUP_RDC",
"BOGUS","ASSIGN","TO","COLON","S_E","AT","ADDR","REG_IDX","EQUATE","MEM_IDX",
"TOK_INTEGER","TOK_FLOAT","FAST","OPERAND","FETCHEXEC","CAST_OP","FLAGMACRO",
"SUCCESSOR","NUM","ASSIGNSIZE","FLOATNUM","';'","','","'{'","'}'","'['","']'",
"')'","'\"'","'\\''","'$'","'_'","'('","'?'","specorasgn","specification","parts",
"operandlist","operand","func_parameter","reglist","@1","@2","a_reglists","a_reglist",
"reg_table","flag_fnc","constants","table_assign","table_expr","str_expr","str_array",
"str_term","name_expand","bin_oper","opstr_expr","opstr_array","exprstr_expr",
"exprstr_array","instr","@3","instr_name","instr_elem","name_contract","rt_list",
"rt","flag_list","list_parameter","param","list_actualparameter","assign_rt",
"exp_term","exp","var_op","cast","endianness","esize","fastlist","fastentries",
"fastentry",""
};
#endif

static const short yyr1[] = {     0,
    63,    63,    63,    64,    64,    65,    65,    65,    65,    65,
    65,    65,    65,    65,    66,    66,    67,    67,    68,    68,
    70,    69,    71,    69,    72,    72,    73,    73,    73,    73,
    73,    73,    74,    74,    75,    76,    76,    77,    78,    78,
    78,    79,    79,    80,    80,    80,    81,    81,    82,    82,
    82,    82,    83,    83,    83,    84,    85,    85,    86,    87,
    87,    89,    88,    90,    90,    91,    91,    91,    92,    92,
    92,    92,    92,    92,    93,    93,    94,    94,    94,    94,
    94,    95,    95,    96,    96,    96,    97,    98,    98,    98,
    99,    99,    99,    99,    99,   100,   100,   100,   100,   100,
   100,   100,   100,   100,   100,   100,   100,   100,   100,   100,
   101,   101,   101,   101,   101,   101,   101,   101,   101,   101,
   102,   102,   102,   102,   102,   102,   102,   102,   103,   104,
   105,   105,   106,   107,   107,   108
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
     7,     8,     3,     7,     1,     1,     3,     3,     3,     3,
     2,     2,     2,     3,     3,     3,     3,     3,     5,     1,
     1,     3,     1,     3,     1,     7,     2,     3,     3,     2,
     1,     1,     2,     3,     1,     3
};

static const short yydefact[] = {     0,
    66,   121,   123,   105,   106,    98,     0,     0,    86,     0,
     0,     0,     0,     0,     0,    21,    23,     0,     0,     0,
     0,    96,     0,    97,     0,     0,     0,     0,     0,     3,
     0,    12,    13,     8,     9,     6,    62,    64,    67,     1,
   120,     2,   100,    10,    11,     0,     0,   125,   105,   106,
    90,     0,     0,   125,     0,    85,     0,    89,     0,     0,
   131,   132,   130,   113,     0,     0,     0,     0,     0,     0,
   133,   135,    87,    14,    16,    86,    93,    94,    90,     0,
    81,     7,    76,    77,     0,    95,   100,     0,     0,     0,
     0,     0,    66,    86,     0,     0,     5,    65,    86,    68,
     0,     0,     0,     0,     0,     0,   111,     0,     0,   112,
   127,    52,    36,     0,     0,     0,     0,    38,    39,    43,
    48,    40,    41,     0,     0,   107,     0,     0,     0,   109,
    71,    70,   103,   122,   124,     0,     0,    22,    26,    24,
     0,     0,     0,     0,    20,     0,    83,    80,     0,    75,
   110,     0,     0,     0,    74,    69,     0,     0,    99,     0,
     4,     0,   117,   116,   115,   118,   114,     0,     0,     0,
     0,     0,     0,    46,     0,     0,     0,     0,     0,    51,
    42,     0,   108,    84,     0,    88,     0,     0,    34,     0,
     0,   136,   134,    15,    86,    86,     0,    78,     0,    79,
     0,   100,    92,     0,    73,    72,    71,    63,     0,     0,
   129,    37,    53,    54,   125,    55,     0,     0,     0,    47,
     0,    56,     0,    59,    50,    49,     0,     0,    27,     0,
     0,     0,    25,     0,     0,     0,    82,     0,     0,   119,
     0,    58,    61,     0,    44,     0,     0,     0,    35,     0,
    33,     0,    17,    19,    18,    91,     0,     0,    45,     0,
     0,   104,     0,     0,   101,   126,    57,    60,    28,     0,
   102,     0,     0,     0,     0,     0,    32,     0,     0,     0,
     0,    29,    31,     0,     0,     0,    30,     0,     0,     0
};

static const short yydefgoto[] = {   288,
    30,    31,    74,    75,   197,    32,    68,    69,   138,   139,
   190,    33,    34,    35,   118,   174,   175,   120,   121,   217,
   122,   176,   123,   177,    36,    99,    37,    38,    39,    82,
    83,   149,    55,    56,    57,    84,    41,    58,    43,   110,
    44,    63,    45,    71,    72
};

static const short yypact[] = {   142,
    85,-32768,-32768,    10,    37,-32768,    -3,   197,   246,    33,
    34,   197,   197,   197,   197,-32768,-32768,    45,    56,   579,
   197,-32768,   197,-32768,   197,    63,   137,   104,   197,   368,
    97,-32768,-32768,-32768,-32768,-32768,   147,    77,-32768,-32768,
-32768,   569,   113,-32768,-32768,    -1,   121,-32768,-32768,-32768,
   197,   188,   307,    57,    89,-32768,   110,   569,   152,   153,
-32768,-32768,-32768,   223,   320,   400,   425,     1,     1,   177,
   162,-32768,-32768,   166,-32768,     4,-32768,-32768,   197,    54,
-32768,   579,-32768,-32768,   340,   500,    75,    14,   165,   167,
    36,   345,   200,    56,    64,   176,-32768,-32768,    56,-32768,
   197,   197,   197,   197,   197,   227,-32768,   194,   202,-32768,
-32768,-32768,   260,    35,   255,   259,   262,-32768,   135,-32768,
-32768,-32768,-32768,   229,   218,-32768,    56,   222,   197,-32768,
   496,-32768,-32768,-32768,-32768,    -9,   272,   232,-32768,   232,
   283,    45,    56,   237,    92,   111,-32768,-32768,   119,-32768,
    80,   197,   197,   197,-32768,-32768,   236,   241,-32768,   242,
-32768,   257,   573,   476,   164,   573,   223,   243,   197,   248,
   252,   135,    60,   135,   -29,   189,   199,   249,   245,-32768,
-32768,   258,-32768,-32768,   579,   569,   267,   271,-32768,   -17,
     1,-32768,-32768,-32768,    56,    56,   273,-32768,   312,-32768,
   569,    83,   569,   532,-32768,-32768,-32768,   579,   197,   537,
-32768,-32768,-32768,-32768,   249,-32768,   265,    27,   172,-32768,
   270,-32768,   274,-32768,-32768,-32768,   197,   566,-32768,   275,
   325,   281,-32768,   226,    72,   197,-32768,   197,   197,-32768,
   197,-32768,-32768,    43,   135,   216,   197,   365,-32768,   303,
-32768,   289,-32768,-32768,   569,   569,   438,   461,-32768,   280,
   282,-32768,   291,   292,   304,-32768,-32768,-32768,    11,   330,
-32768,   350,   352,   314,   331,   333,   335,   326,   373,   336,
   337,-32768,-32768,   351,   338,   339,-32768,   386,   388,-32768
};

static const short yypgoto[] = {-32768,
-32768,   363,-32768,   276,-32768,-32768,-32768,-32768,   344,   204,
-32768,-32768,-32768,-32768,-32768,   -45,-32768,  -115,-32768,   154,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   364,   -46,
   -80,-32768,   -71,   -12,   341,   414,   206,     0,   -20,   151,
-32768,-32768,-32768,-32768,   290
};


#define	YYLAST		639


static const short yytable[] = {    42,
   119,   150,    87,   181,   145,   112,    76,    53,   136,   -93,
    73,    64,    65,    66,    67,   187,   101,   102,   103,   104,
    85,   219,    86,   220,    88,   105,   272,   162,    92,   101,
   102,   103,   104,   231,   273,   106,   -94,   232,   105,    59,
   144,   112,   157,    47,   188,   113,   107,   108,   106,   178,
   114,    70,    61,    62,   137,   115,   116,   117,   181,   107,
   108,   147,    73,   213,   214,   109,   215,     2,     3,    89,
   160,   216,    49,    50,     6,   154,     7,     8,   109,    60,
    51,    52,   158,   243,  -125,    12,   172,  -125,  -125,  -125,
  -125,   173,   116,   117,    13,    14,  -125,    15,    95,   259,
   163,   164,   165,   166,   167,    21,    22,   -87,    24,   148,
    60,   153,   -87,    25,   184,   208,  -128,  -125,  -125,   238,
    29,    46,   127,   234,   235,    91,   254,   150,   186,   181,
    76,   202,   111,    26,    27,    28,  -125,  -128,   228,   127,
   111,   112,   127,    90,   128,   196,    97,   150,     1,     2,
     3,   201,   203,   204,     4,     5,     6,    98,     7,     8,
   129,   129,     9,    10,    11,   130,   198,    12,   210,   199,
   111,   124,   218,   245,   200,   105,    13,    14,   112,    15,
    16,    17,    18,    19,    20,   106,   172,    21,    22,    23,
    24,   115,   116,   117,   125,    25,   107,   108,    26,    27,
    28,   141,    29,    48,     2,     3,   131,   132,   201,    49,
    50,     6,   142,     7,     8,   109,   143,    51,    52,   213,
   214,   155,    12,   172,   156,   161,   248,   216,   244,   116,
   117,    13,    14,   168,    15,   255,    46,   256,   257,   221,
   258,   222,    21,    22,   106,    24,   261,   169,   170,   223,
    25,   224,    54,     2,     3,   107,   108,    29,    49,    50,
     6,   178,     7,     8,   171,   179,    51,    52,   180,    77,
    78,    12,   183,   185,   109,   182,   127,    79,   253,   189,
    13,    14,   191,    15,   101,   102,   103,   104,   195,   192,
   205,    21,    22,   105,    24,   206,   207,   209,   212,    25,
   211,    80,   226,   106,    23,   225,    29,   127,   227,   101,
   102,   103,   104,   229,   107,   108,    81,   230,   105,   237,
   236,   242,   101,   102,   103,   104,   246,   263,   106,   250,
   247,   105,   251,   109,   252,   264,   267,   269,   268,   107,
   108,   106,   101,   102,   103,   104,   270,   101,   102,   103,
   104,   105,   107,   108,   274,   109,   105,   275,   109,   276,
   277,   106,   126,   279,   278,   280,   106,   101,   102,   103,
   104,   109,   107,   108,    93,   133,   105,   107,   108,   281,
   282,   285,   283,   284,   286,   289,   106,   290,    94,    95,
    11,   109,    96,   287,   233,   151,   109,   107,   108,   260,
   159,   100,   101,   102,   103,   104,    16,    17,    18,    19,
    20,   105,   140,    40,   240,   271,   109,     0,   194,   146,
   262,   106,     0,     0,    26,    27,    28,   101,   102,   103,
   104,   193,   107,   108,     0,     0,   105,     0,     0,     0,
   101,   102,   103,   104,     0,     0,   106,     0,     0,   105,
     0,   109,     0,     0,   134,     0,     0,   107,   108,   106,
     0,     0,     0,   101,   102,   103,   104,     0,     0,     0,
   107,   108,   105,     0,     0,     0,   109,     0,     0,   135,
   103,     0,   106,     0,     0,     0,     0,   105,     0,   109,
     0,     0,   265,   107,   108,  -108,     0,   106,  -108,  -108,
  -108,  -108,   101,   102,   103,   104,     0,  -108,   107,   108,
     0,   105,   109,     0,     0,   266,     0,     0,     0,     0,
     0,   106,     0,     0,     0,     0,   152,   109,  -108,  -108,
     0,     0,   107,   108,   101,   102,   103,   104,     0,   101,
   102,   103,   104,   105,     0,     0,     0,  -108,   105,     0,
     0,   109,     0,   106,     0,     0,     0,     0,   106,     0,
     0,     0,     0,   239,   107,   108,     0,     0,   241,   107,
   108,   101,   102,   103,   104,   101,   102,   103,    77,    78,
   105,     0,     0,   109,   105,     0,    79,     0,   109,     0,
   106,    77,    78,     0,   106,     0,     0,     0,     0,    79,
     0,   107,   108,     0,     0,   107,   108,     0,     0,     0,
    80,     0,     0,    23,     0,     0,     0,     0,   249,     0,
   109,     0,     0,    80,   109,    81,    23,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    81
};

static const short yycheck[] = {     0,
    46,    82,    23,   119,    76,     7,    19,     8,     8,     0,
     7,    12,    13,    14,    15,    25,     3,     4,     5,     6,
    21,    51,    23,    53,    25,    12,    16,    99,    29,     3,
     4,     5,     6,    51,    24,    22,     0,    55,    12,     7,
    37,     7,     7,    47,    54,    47,    33,    34,    22,     7,
    52,     7,    19,    20,    54,    57,    58,    59,   174,    33,
    34,     8,     7,     4,     5,    52,     7,     8,     9,     7,
     7,    12,    13,    14,    15,    62,    17,    18,    52,    47,
    21,    22,    47,    57,     0,    26,    52,     3,     4,     5,
     6,    57,    58,    59,    35,    36,    12,    38,    22,    57,
   101,   102,   103,   104,   105,    46,    47,    51,    49,    56,
    47,    37,    56,    54,   127,   162,    37,    33,    34,    37,
    61,    37,    51,   195,   196,    22,    55,   208,   129,   245,
   143,   152,    58,    57,    58,    59,    52,    58,   185,    51,
    58,     7,    51,     7,    56,    54,    50,   228,     7,     8,
     9,   152,   153,   154,    13,    14,    15,    11,    17,    18,
    51,    51,    21,    22,    23,    56,    56,    26,   169,    51,
    58,    51,   173,   219,    56,    12,    35,    36,     7,    38,
    39,    40,    41,    42,    43,    22,    52,    46,    47,    48,
    49,    57,    58,    59,     7,    54,    33,    34,    57,    58,
    59,    25,    61,     7,     8,     9,    55,    55,   209,    13,
    14,    15,    51,    17,    18,    52,    51,    21,    22,     4,
     5,    57,    26,    52,    58,    50,   227,    12,    57,    58,
    59,    35,    36,     7,    38,   236,    37,   238,   239,    51,
   241,    53,    46,    47,    22,    49,   247,    54,    47,    51,
    54,    53,     7,     8,     9,    33,    34,    61,    13,    14,
    15,     7,    17,    18,     5,     7,    21,    22,     7,    13,
    14,    26,    55,    52,    52,    47,    51,    21,    53,     8,
    35,    36,    51,    38,     3,     4,     5,     6,    52,     7,
    55,    46,    47,    12,    49,    55,    55,    55,    47,    54,
    53,    45,    58,    22,    48,    57,    61,    51,    51,     3,
     4,     5,     6,    47,    33,    34,    60,    47,    12,     8,
    48,    57,     3,     4,     5,     6,    57,    25,    22,    55,
    57,    12,     8,    52,    54,    47,    57,    47,    57,    33,
    34,    22,     3,     4,     5,     6,    55,     3,     4,     5,
     6,    12,    33,    34,    25,    52,    12,     8,    52,     8,
    47,    22,    56,    31,    34,    31,    22,     3,     4,     5,
     6,    52,    33,    34,     7,    56,    12,    33,    34,    54,
     8,    31,    47,    47,    47,     0,    22,     0,    21,    22,
    23,    52,    30,    55,   191,    56,    52,    33,    34,   246,
    56,    38,     3,     4,     5,     6,    39,    40,    41,    42,
    43,    12,    69,     0,   209,   265,    52,    -1,   143,    79,
    56,    22,    -1,    -1,    57,    58,    59,     3,     4,     5,
     6,   142,    33,    34,    -1,    -1,    12,    -1,    -1,    -1,
     3,     4,     5,     6,    -1,    -1,    22,    -1,    -1,    12,
    -1,    52,    -1,    -1,    55,    -1,    -1,    33,    34,    22,
    -1,    -1,    -1,     3,     4,     5,     6,    -1,    -1,    -1,
    33,    34,    12,    -1,    -1,    -1,    52,    -1,    -1,    55,
     5,    -1,    22,    -1,    -1,    -1,    -1,    12,    -1,    52,
    -1,    -1,    55,    33,    34,     0,    -1,    22,     3,     4,
     5,     6,     3,     4,     5,     6,    -1,    12,    33,    34,
    -1,    12,    52,    -1,    -1,    55,    -1,    -1,    -1,    -1,
    -1,    22,    -1,    -1,    -1,    -1,    27,    52,    33,    34,
    -1,    -1,    33,    34,     3,     4,     5,     6,    -1,     3,
     4,     5,     6,    12,    -1,    -1,    -1,    52,    12,    -1,
    -1,    52,    -1,    22,    -1,    -1,    -1,    -1,    22,    -1,
    -1,    -1,    -1,    32,    33,    34,    -1,    -1,    32,    33,
    34,     3,     4,     5,     6,     3,     4,     5,    13,    14,
    12,    -1,    -1,    52,    12,    -1,    21,    -1,    52,    -1,
    22,    13,    14,    -1,    22,    -1,    -1,    -1,    -1,    21,
    -1,    33,    34,    -1,    -1,    33,    34,    -1,    -1,    -1,
    45,    -1,    -1,    48,    -1,    -1,    -1,    -1,    53,    -1,
    52,    -1,    -1,    45,    52,    60,    48,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    60
};

#line 329 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (YY_SSLParser_CHAR = YYEMPTY)
#define YYEMPTY         -2
#define YYEOF           0
#define YYACCEPT        return(0)
#define YYABORT         return(1)
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
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_SSLParser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_SSLParser_LSP_NEEDED
      yyls = (YY_SSLParser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
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


/* #line 783 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 1547 "sslparser.cpp"

  switch (yyn) {

case 1:
#line 211 "sslparser.y"
{
            the_asgn = yyvsp[0].regtransfer;
        ;
    break;}
case 2:
#line 214 "sslparser.y"
{
            the_asgn = new Assign(
                new Terminal(opNil),
                yyvsp[0].exp);
        ;
    break;}
case 7:
#line 230 "sslparser.y"
{
            Dict.fetchExecCycle = yyvsp[0].rtlist;
        ;
    break;}
case 14:
#line 255 "sslparser.y"
{ Dict.fixupParams(); ;
    break;}
case 17:
#line 268 "sslparser.y"
{
            // Note: the below copies the list of strings!
            Dict.DetParamMap[yyvsp[-4].str].params = *yyvsp[-1].parmlist;
            Dict.DetParamMap[yyvsp[-4].str].kind = PARAM_VARIANT;
            delete yyvsp[-1].parmlist;
        ;
    break;}
case 18:
#line 282 "sslparser.y"
{
            std::map<std::string, InsNameElem*> m;
            ParamEntry &param = Dict.DetParamMap[yyvsp[-4].str];
            Statement* asgn = new Assign(yyvsp[-1].num, new Terminal(opNil), yyvsp[0].exp);
            // Note: The below 2 copy lists of strings (to be deleted below!)
            param.params = *yyvsp[-3].parmlist;
            param.funcParams = *yyvsp[-2].parmlist;
            param.asgn = asgn;
            param.kind = PARAM_ASGN;
            
            if( param.funcParams.size() != 0 )
                param.kind = PARAM_LAMBDA;
            delete yyvsp[-3].parmlist;
            delete yyvsp[-2].parmlist;
        ;
    break;}
case 19:
#line 299 "sslparser.y"
{ yyval.parmlist = yyvsp[-1].parmlist; ;
    break;}
case 20:
#line 300 "sslparser.y"
{ yyval.parmlist = new std::list<std::string>(); ;
    break;}
case 21:
#line 304 "sslparser.y"
{
                    bFloat = false;
                ;
    break;}
case 23:
#line 307 "sslparser.y"
{
                    bFloat = true;
                ;
    break;}
case 27:
#line 317 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-2].str) != Dict.RegMap.end())
                    yyerror("Name reglist decared twice\n");
                Dict.RegMap[yyvsp[-2].str] = yyvsp[0].num;
            ;
    break;}
case 28:
#line 322 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-5].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.addRegister( yyvsp[-5].str, yyvsp[0].num, yyvsp[-3].num, bFloat);
            ;
    break;}
case 29:
#line 327 "sslparser.y"
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
#line 362 "sslparser.y"
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
#line 384 "sslparser.y"
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
#line 399 "sslparser.y"
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
#line 411 "sslparser.y"
{
                yyvsp[-2].strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 34:
#line 414 "sslparser.y"
{
                yyval.strlist = new std::list<std::string>;
                yyval.strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 35:
#line 423 "sslparser.y"
{
                // Note: $2 is a list of strings
                Dict.FlagFuncs[yyvsp[-5].str] = new FlagDef(listStrToExp(yyvsp[-4].parmlist), yyvsp[-1].rtlist);
            ;
    break;}
case 36:
#line 430 "sslparser.y"
{
                if (ConstTable.find(yyvsp[-2].str) != ConstTable.end())
                    yyerror("Constant declared twice");
                ConstTable[std::string(yyvsp[-2].str)] = yyvsp[0].num;
            ;
    break;}
case 37:
#line 436 "sslparser.y"
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
#line 450 "sslparser.y"
{
            TableDict[yyvsp[-2].str] = yyvsp[0].tab;
        ;
    break;}
case 39:
#line 456 "sslparser.y"
{
            yyval.tab = new Table(*yyvsp[0].namelist);
            delete yyvsp[0].namelist;
        ;
    break;}
case 40:
#line 461 "sslparser.y"
{
            yyval.tab = new OpTable(*yyvsp[0].namelist);
            delete yyvsp[0].namelist;
        ;
    break;}
case 41:
#line 465 "sslparser.y"
{
            yyval.tab = new ExprTable(*yyvsp[0].exprlist);
            delete yyvsp[0].exprlist;
        ;
    break;}
case 42:
#line 472 "sslparser.y"
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
#line 482 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 44:
#line 488 "sslparser.y"
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
#line 496 "sslparser.y"
{
            yyvsp[-3].namelist->push_back("");
        ;
    break;}
case 46:
#line 499 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 47:
#line 505 "sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 48:
#line 508 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 49:
#line 514 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>;
            yyval.namelist->push_back("");
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 50:
#line 519 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>(1, yyvsp[-1].str);
        ;
    break;}
case 51:
#line 522 "sslparser.y"
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
#line 537 "sslparser.y"
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
#line 556 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 54:
#line 560 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 55:
#line 564 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 56:
#line 571 "sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 57:
#line 578 "sslparser.y"
{
            yyval.namelist = yyvsp[-4].namelist;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 58:
#line 582 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 59:
#line 590 "sslparser.y"
{
            yyval.exprlist = yyvsp[-1].exprlist;
        ;
    break;}
case 60:
#line 597 "sslparser.y"
{
            yyval.exprlist = yyvsp[-4].exprlist;
            yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
    break;}
case 61:
#line 601 "sslparser.y"
{
            yyval.exprlist = new std::deque<Exp*>;
            yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
    break;}
case 62:
#line 609 "sslparser.y"
{
            yyvsp[0].insel->getrefmap(indexrefmap);
        //     $3           $4
        ;
    break;}
case 63:
#line 612 "sslparser.y"
{
            // This function expands the tables and saves the expanded RTLs
            // to the dictionary
            expandTables(yyvsp[-3].insel, yyvsp[-1].parmlist, yyvsp[0].rtlist, Dict);
        ;
    break;}
case 64:
#line 620 "sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 65:
#line 623 "sslparser.y"
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
#line 647 "sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[0].str);
        ;
    break;}
case 67:
#line 650 "sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 68:
#line 653 "sslparser.y"
{
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(yyvsp[0].insel);
        ;
    break;}
case 69:
#line 660 "sslparser.y"
{
            yyval.insel = new InsOptionElem(yyvsp[-1].str);
        ;
    break;}
case 70:
#line 663 "sslparser.y"
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
#line 676 "sslparser.y"
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
#line 685 "sslparser.y"
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
#line 696 "sslparser.y"
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
#line 705 "sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[-1].str);
        ;
    break;}
case 75:
#line 711 "sslparser.y"
{
            // append any automatically generated register transfers and clear
            // the list they were stored in. Do nothing for a NOP (i.e. $2 = 0)
            if (yyvsp[0].regtransfer != NULL) {
                yyvsp[-1].rtlist->appendStmt(yyvsp[0].regtransfer);
            }
            yyval.rtlist = yyvsp[-1].rtlist;
        ;
    break;}
case 76:
#line 720 "sslparser.y"
{
            yyval.rtlist = new RTL(STMT_ASSIGN);
            if (yyvsp[0].regtransfer != NULL)
                yyval.rtlist->appendStmt(yyvsp[0].regtransfer);
        ;
    break;}
case 77:
#line 728 "sslparser.y"
{
            yyval.regtransfer = yyvsp[0].regtransfer;
        ;
    break;}
case 78:
#line 734 "sslparser.y"
{
            std::ostringstream o;
            if (Dict.FlagFuncs.find(yyvsp[-2].str) != Dict.FlagFuncs.end()) {
                yyval.regtransfer = new Assign(
                    new Terminal(opFlags),
                    new Binary(opFlagCall,
                        new Const(yyvsp[-2].str),
                        listExpToExp(yyvsp[-1].explist)));
            } else {
                o << yyvsp[-2].str << " is not declared as a flag function.\n";
                yyerror(STR(o));
            }
        ;
    break;}
case 79:
#line 747 "sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 80:
#line 751 "sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 81:
#line 754 "sslparser.y"
{
        yyval.regtransfer = NULL;
    ;
    break;}
case 82:
#line 760 "sslparser.y"
{
            // Not sure why the below is commented out (MVE)
/*          Unary* pFlag = new Unary(opRegOf, Dict.RegMap[$3]);
            $1->push_back(pFlag);
            $$ = $1;
*/          yyval.explist = 0;
        ;
    break;}
case 83:
#line 767 "sslparser.y"
{
/*          std::list<Exp*>* tmp = new std::list<Exp*>;
            Unary* pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
            tmp->push_back(pFlag);
            $$ = tmp;
*/          yyval.explist = 0;
        ;
    break;}
case 84:
#line 778 "sslparser.y"
{
            assert(yyvsp[0].str != 0);
            yyvsp[-2].parmlist->push_back(yyvsp[0].str);
            yyval.parmlist = yyvsp[-2].parmlist;
        ;
    break;}
case 85:
#line 784 "sslparser.y"
{
            yyval.parmlist = new std::list<std::string>;
            yyval.parmlist->push_back(yyvsp[0].str);
        ;
    break;}
case 86:
#line 788 "sslparser.y"
{
            yyval.parmlist = new std::list<std::string>;
        ;
    break;}
case 87:
#line 793 "sslparser.y"
{
            Dict.ParamSet.insert(yyvsp[0].str);       // Not sure if we need this set
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 88:
#line 799 "sslparser.y"
{
            yyval.explist->push_back(yyvsp[0].exp);
        ;
    break;}
case 89:
#line 803 "sslparser.y"
{
            yyval.explist = new std::list<Exp*>;
            yyval.explist->push_back(yyvsp[0].exp);
        ;
    break;}
case 90:
#line 808 "sslparser.y"
{
            yyval.explist = new std::list<Exp*>;
        ;
    break;}
case 91:
#line 816 "sslparser.y"
{
            Assign* a = new Assign(yyvsp[-5].num, yyvsp[-2].exp, yyvsp[0].exp);
            a->setGuard(yyvsp[-4].exp);
            yyval.regtransfer = a;
        ;
    break;}
case 92:
#line 823 "sslparser.y"
{
            // update the size of any generated RT's
            yyval.regtransfer = new Assign(yyvsp[-3].num, yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 93:
#line 829 "sslparser.y"
{
            yyval.regtransfer = new Assign(
                new Terminal(opNil),
                new Terminal(opFpush));
        ;
    break;}
case 94:
#line 834 "sslparser.y"
{
            yyval.regtransfer = new Assign(
                new Terminal(opNil),
                new Terminal(opFpop));
        ;
    break;}
case 95:
#line 840 "sslparser.y"
{
            yyval.regtransfer = new Assign(yyvsp[-1].num, 0, yyvsp[0].exp);
        ;
    break;}
case 96:
#line 846 "sslparser.y"
{
            yyval.exp = new Const(yyvsp[0].num);
        ;
    break;}
case 97:
#line 850 "sslparser.y"
{
            yyval.exp = new Const(yyvsp[0].dbl);
        ;
    break;}
case 98:
#line 854 "sslparser.y"
{
            yyval.exp = new Unary(opTemp, new Const(yyvsp[0].str));
        ;
    break;}
case 99:
#line 858 "sslparser.y"
{
            yyval.exp = yyvsp[-1].exp;
        ;
    break;}
case 100:
#line 862 "sslparser.y"
{
            yyval.exp = yyvsp[0].exp;
        ;
    break;}
case 101:
#line 866 "sslparser.y"
{
            yyval.exp = new Ternary(opTern, yyvsp[-5].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
    break;}
case 102:
#line 870 "sslparser.y"
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
#line 880 "sslparser.y"
{
            yyval.exp = new Unary(opAddrOf, yyvsp[-1].exp);
        ;
    break;}
case 104:
#line 886 "sslparser.y"
{
            yyval.exp = new Ternary(strToOper(yyvsp[-6].str), new Const(yyvsp[-5].num), new Const(yyvsp[-3].num), yyvsp[-1].exp);
        ;
    break;}
case 105:
#line 891 "sslparser.y"
{
            yyval.exp = new Terminal(opFpush);
        ;
    break;}
case 106:
#line 894 "sslparser.y"
{
            yyval.exp = new Terminal(opFpop);
        ;
    break;}
case 107:
#line 899 "sslparser.y"
{
            yyval.exp = new Unary(strToOper(yyvsp[-2].str), yyvsp[-1].exp);
        ;
    break;}
case 108:
#line 905 "sslparser.y"
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
#line 933 "sslparser.y"
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
                    yyval.exp = new Binary(opFlagDef,
                            new Const(yyvsp[-2].str),
                            listExpToExp(yyvsp[-1].explist));
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
#line 960 "sslparser.y"
{
			yyval.exp = makeSuccessor(yyvsp[-1].exp);
		;
    break;}
case 111:
#line 966 "sslparser.y"
{
            yyval.exp = new Unary(opSignExt, yyvsp[-1].exp);
        ;
    break;}
case 112:
#line 975 "sslparser.y"
{
            // opSize is deprecated, but for old SSL files we'll make a TypedExp
            if (yyvsp[0].num == STD_SIZE)
                yyval.exp = yyvsp[-1].exp;
            else
                yyval.exp = new TypedExp(new IntegerType(yyvsp[0].num), yyvsp[-1].exp);
        ;
    break;}
case 113:
#line 983 "sslparser.y"
{
            yyval.exp = new Unary(opNot, yyvsp[0].exp);
        ;
    break;}
case 114:
#line 987 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 115:
#line 991 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 116:
#line 995 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 117:
#line 999 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 118:
#line 1003 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 119:
#line 1010 "sslparser.y"
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
case 120:
#line 1036 "sslparser.y"
{
            yyval.exp = yyvsp[0].exp;
        ;
    break;}
case 121:
#line 1047 "sslparser.y"
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
case 122:
#line 1071 "sslparser.y"
{
            yyval.exp = new Unary(opRegOf, yyvsp[-1].exp);
        ;
    break;}
case 123:
#line 1075 "sslparser.y"
{
            int regNum;
            sscanf(yyvsp[0].str, "r%d", &regNum);
            yyval.exp = new Unary(opRegOf, new Const(regNum));
        ;
    break;}
case 124:
#line 1081 "sslparser.y"
{
            yyval.exp = new Unary(opMemOf, yyvsp[-1].exp);
        ;
    break;}
case 125:
#line 1085 "sslparser.y"
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
case 126:
#line 1103 "sslparser.y"
{
            yyval.exp = new Ternary(opAt, yyvsp[-6].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
    break;}
case 127:
#line 1107 "sslparser.y"
{
            yyval.exp = new Unary(opPostVar, yyvsp[-1].exp);
        ;
    break;}
case 128:
#line 1110 "sslparser.y"
{
			yyval.exp = makeSuccessor(yyvsp[-1].exp);
		;
    break;}
case 129:
#line 1116 "sslparser.y"
{
            yyval.num = yyvsp[-1].num;
        ;
    break;}
case 130:
#line 1122 "sslparser.y"
{
            Dict.bigEndian = (strcmp(yyvsp[0].str, "BIG") == 0);
        ;
    break;}
case 131:
#line 1127 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 132:
#line 1130 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 136:
#line 1149 "sslparser.y"
{
            Dict.fastMap[std::string(yyvsp[-2].str)] = std::string(yyvsp[0].str);
        ;
    break;}
}

#line 783 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
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
	       (unsigned)x < (sizeof(yytname) / sizeof(char *)); x++)
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
		       (unsigned)x < (sizeof(yytname) / sizeof(char *)); x++)
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

/* #line 982 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 2794 "sslparser.cpp"
#line 1152 "sslparser.y"


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
 * OVERVIEW:        Parses an assignment from a string.
 * PARAMETERS:      the string
 * RETURNS:         an Assignment or NULL.
 *============================================================================*/
Statement* SSLParser::parseExp(const char *str) {
    std::istringstream ss(str);
    SSLParser p(ss, false);     // Second arg true for debugging
    RTLInstDict d;
    p.yyparse(d);
    return p.the_asgn;
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
        int n = rtl->getNumStmt();
        Exp* srchExpr = new Binary(opExpTable, new Terminal(opWild),
            new Terminal(opWild));
        Exp* srchOp = new Ternary(opOpTable, new Terminal(opWild),
            new Terminal(opWild), new Terminal(opWild));
        for (int j=0; j < n; j++) {
            Statement* s = rtl->elementAt(j);
            std::list<Exp*> le;
            // Expression tables
            assert(s->getKind() == STMT_ASSIGN);
            if (((Assign*)s)->searchAll(srchExpr, le)) {
                std::list<Exp*>::iterator it;
                for (it = le.begin(); it != le.end(); it++) {
                    char* tbl = ((Const*)((Binary*)*it)->getSubExp1())
                      ->getStr();
                    char* idx = ((Const*)((Binary*)*it)->getSubExp2())
                      ->getStr();
                    Exp* repl =((ExprTable*)(TableDict[tbl]))
                      ->expressions[indexrefmap[idx]->getvalue()];
                    s->searchAndReplace(*it, repl);
                    // Just in case the top level is changed...
                    rtl->updateStmt(s, j);
                }
            }
            // Operator tables
			Exp* res;
			while (s->search(srchOp, res)) {
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
                s->searchAndReplace(res, repl);
                // In case the top level is changed (common)
                rtl->updateStmt(s, j);
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

/*==============================================================================
 * FUNCTION:        SSLParser::makeSuccessor
 * OVERVIEW:        Make the successor of the given expression, e.g. given
 *					  r[2], return succ( r[2] ) (using opSuccessor)
 *					We can't do the successor operation here, because the
 *					  parameters are not yet instantiated (still of the form
 *					  param(rd)). Actual successor done in Exp::fixSuccessor()
 * NOTE:            The given expression should be of the form  r[const]
 * NOTE:			The parameter expresion is copied (not cloned) in the result
 * PARAMETERS:      The expression to find the successor of
 * RETURNS:         The modified expression
 *============================================================================*/
Exp* SSLParser::makeSuccessor(Exp* e) {
	return new Unary(opSuccessor, e);
}
