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
#line 36 "sslparser.y"

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

#line 61 "sslparser.y"
typedef union {
    Exp*            exp;
    char*           str;
    int             num;
    double          dbl;
    Statement*      regtransfer;
    Type*           typ;
    
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
#line 80 "sslparser.y"

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
#line 244 "sslparser.cpp"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_SSLParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 253 "sslparser.cpp"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_SSLParser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 260 "sslparser.cpp"

#line 123 "/usr/local/lib/bison.cc"
/* YY_SSLParser_DEBUG */
#endif


#ifndef YY_SSLParser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 270 "sslparser.cpp"

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
#line 383 "sslparser.cpp"
#define	COND_OP	258
#define	BIT_OP	259
#define	ARITH_OP	260
#define	LOG_OP	261
#define	NAME	262
#define	ASSIGNTYPE	263
#define	REG_ID	264
#define	REG_NUM	265
#define	COND_TNAME	266
#define	DECOR	267
#define	FARITH_OP	268
#define	FPUSH	269
#define	FPOP	270
#define	TEMP	271
#define	SHARES	272
#define	CONV_FUNC	273
#define	TRUNC_FUNC	274
#define	TRANSCEND	275
#define	FABS_FUNC	276
#define	BIG	277
#define	LITTLE	278
#define	NAME_CALL	279
#define	NAME_LOOKUP	280
#define	ENDIANNESS	281
#define	COVERS	282
#define	INDEX	283
#define	NOT	284
#define	THEN	285
#define	LOOKUP_RDC	286
#define	BOGUS	287
#define	ASSIGN	288
#define	TO	289
#define	COLON	290
#define	S_E	291
#define	AT	292
#define	ADDR	293
#define	REG_IDX	294
#define	EQUATE	295
#define	MEM_IDX	296
#define	TOK_INTEGER	297
#define	TOK_FLOAT	298
#define	FAST	299
#define	OPERAND	300
#define	FETCHEXEC	301
#define	CAST_OP	302
#define	FLAGMACRO	303
#define	SUCCESSOR	304
#define	NUM	305
#define	FLOATNUM	306


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
#line 482 "sslparser.cpp"
static const int COND_OP;
static const int BIT_OP;
static const int ARITH_OP;
static const int LOG_OP;
static const int NAME;
static const int ASSIGNTYPE;
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
static const int TRUNC_FUNC;
static const int TRANSCEND;
static const int FABS_FUNC;
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
static const int FLOATNUM;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 540 "sslparser.cpp"
	,COND_OP=258
	,BIT_OP=259
	,ARITH_OP=260
	,LOG_OP=261
	,NAME=262
	,ASSIGNTYPE=263
	,REG_ID=264
	,REG_NUM=265
	,COND_TNAME=266
	,DECOR=267
	,FARITH_OP=268
	,FPUSH=269
	,FPOP=270
	,TEMP=271
	,SHARES=272
	,CONV_FUNC=273
	,TRUNC_FUNC=274
	,TRANSCEND=275
	,FABS_FUNC=276
	,BIG=277
	,LITTLE=278
	,NAME_CALL=279
	,NAME_LOOKUP=280
	,ENDIANNESS=281
	,COVERS=282
	,INDEX=283
	,NOT=284
	,THEN=285
	,LOOKUP_RDC=286
	,BOGUS=287
	,ASSIGN=288
	,TO=289
	,COLON=290
	,S_E=291
	,AT=292
	,ADDR=293
	,REG_IDX=294
	,EQUATE=295
	,MEM_IDX=296
	,TOK_INTEGER=297
	,TOK_FLOAT=298
	,FAST=299
	,OPERAND=300
	,FETCHEXEC=301
	,CAST_OP=302
	,FLAGMACRO=303
	,SUCCESSOR=304
	,NUM=305
	,FLOATNUM=306


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
#line 626 "sslparser.cpp"
const int YY_SSLParser_CLASS::COND_OP=258;
const int YY_SSLParser_CLASS::BIT_OP=259;
const int YY_SSLParser_CLASS::ARITH_OP=260;
const int YY_SSLParser_CLASS::LOG_OP=261;
const int YY_SSLParser_CLASS::NAME=262;
const int YY_SSLParser_CLASS::ASSIGNTYPE=263;
const int YY_SSLParser_CLASS::REG_ID=264;
const int YY_SSLParser_CLASS::REG_NUM=265;
const int YY_SSLParser_CLASS::COND_TNAME=266;
const int YY_SSLParser_CLASS::DECOR=267;
const int YY_SSLParser_CLASS::FARITH_OP=268;
const int YY_SSLParser_CLASS::FPUSH=269;
const int YY_SSLParser_CLASS::FPOP=270;
const int YY_SSLParser_CLASS::TEMP=271;
const int YY_SSLParser_CLASS::SHARES=272;
const int YY_SSLParser_CLASS::CONV_FUNC=273;
const int YY_SSLParser_CLASS::TRUNC_FUNC=274;
const int YY_SSLParser_CLASS::TRANSCEND=275;
const int YY_SSLParser_CLASS::FABS_FUNC=276;
const int YY_SSLParser_CLASS::BIG=277;
const int YY_SSLParser_CLASS::LITTLE=278;
const int YY_SSLParser_CLASS::NAME_CALL=279;
const int YY_SSLParser_CLASS::NAME_LOOKUP=280;
const int YY_SSLParser_CLASS::ENDIANNESS=281;
const int YY_SSLParser_CLASS::COVERS=282;
const int YY_SSLParser_CLASS::INDEX=283;
const int YY_SSLParser_CLASS::NOT=284;
const int YY_SSLParser_CLASS::THEN=285;
const int YY_SSLParser_CLASS::LOOKUP_RDC=286;
const int YY_SSLParser_CLASS::BOGUS=287;
const int YY_SSLParser_CLASS::ASSIGN=288;
const int YY_SSLParser_CLASS::TO=289;
const int YY_SSLParser_CLASS::COLON=290;
const int YY_SSLParser_CLASS::S_E=291;
const int YY_SSLParser_CLASS::AT=292;
const int YY_SSLParser_CLASS::ADDR=293;
const int YY_SSLParser_CLASS::REG_IDX=294;
const int YY_SSLParser_CLASS::EQUATE=295;
const int YY_SSLParser_CLASS::MEM_IDX=296;
const int YY_SSLParser_CLASS::TOK_INTEGER=297;
const int YY_SSLParser_CLASS::TOK_FLOAT=298;
const int YY_SSLParser_CLASS::FAST=299;
const int YY_SSLParser_CLASS::OPERAND=300;
const int YY_SSLParser_CLASS::FETCHEXEC=301;
const int YY_SSLParser_CLASS::CAST_OP=302;
const int YY_SSLParser_CLASS::FLAGMACRO=303;
const int YY_SSLParser_CLASS::SUCCESSOR=304;
const int YY_SSLParser_CLASS::NUM=305;
const int YY_SSLParser_CLASS::FLOATNUM=306;


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
#line 692 "sslparser.cpp"


#define	YYFINAL		297
#define	YYFLAG		-32768
#define	YYNTBASE	65

#define YYTRANSLATE(x) ((unsigned)(x) <= 306 ? yytranslate[x] : 112)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    59,     2,    61,     2,     2,    60,    63,
    58,     2,     2,    53,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    52,     2,
     2,     2,    64,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    56,     2,    57,     2,    62,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    54,     2,    55,     2,     2,     2,     2,     2,
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
    46,    47,    48,    49,    50,    51
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
   348,   356,   365,   369,   377,   381,   385,   387,   389,   393,
   397,   401,   405,   408,   411,   414,   418,   422,   426,   430,
   434,   440,   442,   444,   448,   450,   454,   456,   464,   467,
   471,   475,   478,   480,   482,   484,   487,   491,   493
};

static const short yyrhs[] = {   101,
     0,   103,     0,    66,     0,    66,    67,    52,     0,    67,
    52,     0,    90,     0,    46,    95,     0,    78,     0,    79,
     0,   106,     0,   109,     0,    71,     0,    77,     0,    45,
    68,     0,    68,    53,    69,     0,    69,     0,    99,    40,
    54,    98,    55,     0,    99,    98,    70,   108,   103,     0,
    56,    98,    57,     0,     0,     0,    42,    72,    74,     0,
     0,    43,    73,    74,     0,    74,    53,    75,     0,    75,
     0,     9,    28,    50,     0,     9,    56,    50,    57,    28,
    50,     0,     9,    56,    50,    57,    28,    50,    27,     9,
    34,     9,     0,     9,    56,    50,    57,    28,    50,    17,
     9,    37,    56,    50,    34,    50,    57,     0,    56,    76,
    57,    56,    50,    57,    28,    50,    34,    50,     0,    56,
    76,    57,    56,    50,    57,    28,    50,     0,    76,    53,
     9,     0,     9,     0,    24,    98,    58,    54,    95,    55,
     0,     7,    40,    50,     0,     7,    40,    50,     5,    50,
     0,     7,    40,    80,     0,    81,     0,    86,     0,    88,
     0,    81,    83,     0,    83,     0,    82,    53,    81,     0,
    82,    53,    59,    59,     0,    81,     0,    54,    82,    55,
     0,    84,     0,    60,     7,    60,     0,    59,     7,    59,
     0,    61,     7,     0,     7,     0,     4,     0,     5,     0,
    13,     0,    54,    87,    55,     0,    87,    53,    59,    85,
    59,     0,    59,    85,    59,     0,    54,    89,    55,     0,
    89,    53,    59,   103,    59,     0,    59,   103,    59,     0,
     0,    92,    91,    98,    95,     0,    93,     0,    92,    12,
     0,     7,     0,    94,     0,    93,    94,     0,    60,     7,
    60,     0,    25,    50,    57,     0,    25,     7,    57,     0,
    61,    25,    50,    57,     0,    61,    25,     7,    57,     0,
    59,     7,    59,     0,    95,    96,     0,    96,     0,   101,
     0,    24,   100,    58,     0,    48,    97,    58,     0,    48,
    58,     0,    62,     0,    97,    53,     9,     0,     9,     0,
    98,    53,    99,     0,    99,     0,     0,     7,     0,   100,
    53,   103,     0,   103,     0,     0,   108,   103,    30,   104,
    40,   103,     0,   108,   104,    40,   103,     0,    14,     0,
    15,     0,   108,   103,     0,    50,     0,    51,     0,    16,
     0,    63,   103,    58,     0,   104,     0,    56,   103,    64,
   103,    35,   103,    57,     0,    56,   103,    64,   103,    35,
   103,    57,   105,     0,    38,   103,    58,     0,    18,    50,
    53,    50,    53,   103,    58,     0,    19,   103,    58,     0,
    21,   103,    58,     0,    14,     0,    15,     0,    20,   103,
    58,     0,    25,     7,    57,     0,    24,   100,    58,     0,
    49,   103,    58,     0,   103,    36,     0,   103,   105,     0,
    29,   103,     0,   103,    13,   103,     0,   103,     5,   103,
     0,   103,     4,   103,     0,   103,     3,   103,     0,   103,
     6,   103,     0,   103,    25,     7,    57,   102,     0,   102,
     0,     9,     0,    39,   103,    57,     0,    10,     0,    41,
   103,    57,     0,     7,     0,   103,    37,    56,   103,    35,
   103,    57,     0,   104,    60,     0,    49,   103,    58,     0,
    54,    50,    55,     0,    26,   107,     0,    22,     0,    23,
     0,     8,     0,    44,   110,     0,   110,    53,   111,     0,
   111,     0,     7,    28,     7,     0
};

#endif

#if YY_SSLParser_DEBUG != 0
static const short yyrline[] = { 0,
   216,   220,   225,   228,   230,   233,   236,   241,   243,   246,
   251,   254,   258,   261,   265,   267,   270,   288,   305,   306,
   309,   313,   313,   316,   318,   319,   322,   328,   333,   368,
   390,   405,   416,   420,   427,   435,   442,   455,   461,   467,
   471,   477,   488,   493,   502,   505,   510,   514,   519,   525,
   528,   543,   561,   566,   570,   576,   582,   588,   595,   601,
   607,   613,   618,   625,   629,   652,   656,   659,   665,   669,
   682,   691,   702,   711,   716,   726,   733,   740,   757,   761,
   764,   769,   777,   787,   794,   798,   803,   808,   813,   818,
   823,   833,   839,   844,   850,   856,   861,   865,   869,   873,
   877,   881,   891,   897,   902,   907,   912,   915,   920,   926,
   954,   981,   986,   996,  1004,  1008,  1012,  1016,  1020,  1024,
  1031,  1057,  1062,  1093,  1097,  1103,  1107,  1125,  1129,  1132,
  1137,  1143,  1148,  1152,  1157,  1185,  1189,  1192,  1196
};

static const char * const yytname[] = {   "$","error","$illegal.","COND_OP",
"BIT_OP","ARITH_OP","LOG_OP","NAME","ASSIGNTYPE","REG_ID","REG_NUM","COND_TNAME",
"DECOR","FARITH_OP","FPUSH","FPOP","TEMP","SHARES","CONV_FUNC","TRUNC_FUNC",
"TRANSCEND","FABS_FUNC","BIG","LITTLE","NAME_CALL","NAME_LOOKUP","ENDIANNESS",
"COVERS","INDEX","NOT","THEN","LOOKUP_RDC","BOGUS","ASSIGN","TO","COLON","S_E",
"AT","ADDR","REG_IDX","EQUATE","MEM_IDX","TOK_INTEGER","TOK_FLOAT","FAST","OPERAND",
"FETCHEXEC","CAST_OP","FLAGMACRO","SUCCESSOR","NUM","FLOATNUM","';'","','","'{'",
"'}'","'['","']'","')'","'\"'","'\\''","'$'","'_'","'('","'?'","specorasgn",
"specification","parts","operandlist","operand","func_parameter","reglist","@1",
"@2","a_reglists","a_reglist","reg_table","flag_fnc","constants","table_assign",
"table_expr","str_expr","str_array","str_term","name_expand","bin_oper","opstr_expr",
"opstr_array","exprstr_expr","exprstr_array","instr","@3","instr_name","instr_elem",
"name_contract","rt_list","rt","flag_list","list_parameter","param","list_actualparameter",
"assign_rt","exp_term","exp","var_op","cast","endianness","esize","assigntype",
"fastlist","fastentries","fastentry",""
};
#endif

static const short yyr1[] = {     0,
    65,    65,    65,    66,    66,    67,    67,    67,    67,    67,
    67,    67,    67,    67,    68,    68,    69,    69,    70,    70,
    72,    71,    73,    71,    74,    74,    75,    75,    75,    75,
    75,    75,    76,    76,    77,    78,    78,    79,    80,    80,
    80,    81,    81,    82,    82,    82,    83,    83,    84,    84,
    84,    84,    85,    85,    85,    86,    87,    87,    88,    89,
    89,    91,    90,    92,    92,    93,    93,    93,    94,    94,
    94,    94,    94,    94,    95,    95,    96,    96,    96,    96,
    96,    97,    97,    98,    98,    98,    99,   100,   100,   100,
   101,   101,   101,   101,   101,   102,   102,   102,   102,   102,
   102,   102,   102,   102,   102,   102,   102,   102,   102,   102,
   102,   102,   103,   103,   103,   103,   103,   103,   103,   103,
   103,   103,   104,   104,   104,   104,   104,   104,   104,   104,
   105,   106,   107,   107,   108,   109,   110,   110,   111
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
     7,     8,     3,     7,     3,     3,     1,     1,     3,     3,
     3,     3,     2,     2,     2,     3,     3,     3,     3,     3,
     5,     1,     1,     3,     1,     3,     1,     7,     2,     3,
     3,     2,     1,     1,     1,     2,     3,     1,     3
};

static const short yydefact[] = {     0,
    66,   135,   123,   125,   107,   108,    98,     0,     0,     0,
     0,    86,     0,     0,     0,     0,     0,     0,    21,    23,
     0,     0,     0,     0,    96,    97,     0,     0,     0,     0,
     0,     3,     0,    12,    13,     8,     9,     6,    62,    64,
    67,     1,   122,     2,   100,    10,     0,    11,     0,     0,
   127,   107,   108,    90,     0,     0,     0,     0,   127,     0,
    85,     0,    89,     0,     0,   133,   134,   132,   115,     0,
     0,     0,     0,     0,     0,   136,   138,    87,    14,    16,
    86,    93,    94,    90,     0,    81,     7,    76,    77,     0,
     0,     0,     0,     0,     0,    66,    86,     0,     0,     5,
    65,    86,    68,     0,     0,     0,     0,     0,     0,   113,
     0,     0,   114,   129,    95,   100,    52,    36,     0,     0,
     0,     0,    38,    39,    43,    48,    40,    41,     0,     0,
   105,   109,   106,     0,     0,     0,   111,    71,    70,   103,
   124,   126,     0,     0,    22,    26,    24,     0,     0,     0,
     0,    20,     0,    83,    80,     0,    75,   112,     0,    74,
    69,     0,     0,    99,     0,     4,     0,   119,   118,   117,
   120,   116,     0,     0,     0,     0,     0,     0,     0,     0,
    46,     0,     0,     0,     0,     0,    51,    42,     0,   110,
    84,     0,    88,     0,     0,    34,     0,     0,   139,   137,
    15,    86,    86,     0,    78,     0,    79,     0,    73,    72,
    71,    63,     0,     0,   131,     0,   100,    92,    37,    53,
    54,   127,    55,     0,     0,     0,    47,     0,    56,     0,
    59,    50,    49,     0,     0,    27,     0,     0,     0,    25,
     0,     0,     0,    82,     0,   121,     0,     0,    58,    61,
     0,    44,     0,     0,     0,    35,     0,    33,     0,    17,
    19,    18,     0,     0,    91,    45,     0,     0,   104,     0,
     0,   101,   128,    57,    60,    28,     0,   102,     0,     0,
     0,     0,     0,    32,     0,     0,     0,     0,    29,    31,
     0,     0,     0,    30,     0,     0,     0
};

static const short yydefgoto[] = {   295,
    32,    33,    79,    80,   204,    34,    73,    74,   145,   146,
   197,    35,    36,    37,   123,   181,   182,   125,   126,   224,
   127,   183,   128,   184,    38,   102,    39,    40,    41,    87,
    88,   156,    60,    61,    62,    89,    43,    63,    45,   113,
    46,    68,    47,    48,    76,    77
};

static const short yypact[] = {   146,
   457,-32768,-32768,-32768,    54,    61,-32768,    52,   201,   201,
   201,   251,     7,   159,   201,   201,   201,   201,-32768,-32768,
    65,   110,   531,   201,-32768,-32768,   201,   133,   135,   127,
   201,   423,   106,-32768,-32768,-32768,-32768,-32768,   151,    70,
-32768,-32768,-32768,   630,   113,-32768,   201,-32768,    87,   140,
-32768,-32768,-32768,   201,   187,   293,   306,   319,   -21,    -9,
-32768,    56,   630,   142,   157,-32768,-32768,-32768,    14,   332,
   473,   499,     3,     3,   190,   170,-32768,-32768,   175,-32768,
     6,-32768,-32768,   201,    -2,-32768,   531,-32768,-32768,   375,
    16,   172,   173,    19,   388,   192,   110,    21,   185,-32768,
-32768,   110,-32768,   201,   201,   201,   201,   201,   231,-32768,
   188,   191,-32768,-32768,   577,    23,-32768,   241,   267,   242,
   248,   249,-32768,   356,-32768,-32768,-32768,-32768,   209,   205,
-32768,-32768,-32768,   110,   214,   201,-32768,   572,-32768,-32768,
-32768,-32768,    17,   254,   220,-32768,   220,   270,    65,   110,
   224,    22,    85,-32768,-32768,    86,-32768,    59,   201,-32768,
-32768,   222,   225,-32768,   230,-32768,    50,   636,   199,   132,
   636,    14,   234,   201,   226,   201,   201,   238,   356,    72,
   356,    96,   145,   148,   235,   243,-32768,-32768,   255,-32768,
-32768,   531,   630,   245,   263,-32768,   -15,     3,-32768,-32768,
-32768,   110,   110,   296,-32768,   311,-32768,   592,-32768,-32768,
-32768,   531,   201,   617,-32768,   630,    60,   630,-32768,-32768,
-32768,   235,-32768,   256,    30,   447,-32768,   274,-32768,   281,
-32768,-32768,-32768,   201,   426,-32768,   284,   337,   292,-32768,
   174,   126,   201,-32768,   201,-32768,   201,   201,-32768,-32768,
     1,   356,   111,   201,   401,-32768,   321,-32768,   300,-32768,
-32768,   630,   512,   537,   630,-32768,   294,   280,-32768,   302,
   297,   304,-32768,-32768,-32768,    13,   331,-32768,   352,   353,
   315,   329,   333,   336,   316,   362,   324,   325,-32768,-32768,
   342,   334,   326,-32768,   382,   385,-32768
};

static const short yypgoto[] = {-32768,
-32768,   355,-32768,   239,-32768,-32768,-32768,-32768,   322,   197,
-32768,-32768,-32768,-32768,-32768,   -48,-32768,  -119,-32768,   144,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   358,  -144,
   -85,-32768,   -77,   -16,   318,   399,   195,     0,   -44,   131,
-32768,-32768,   215,-32768,-32768,   260
};


#define	YYLAST		690


static const short yytable[] = {    44,
   124,   157,   116,   152,   188,    81,   154,   185,    56,    57,
    58,   143,    78,    64,    69,    70,    71,    72,   104,   105,
   106,   107,   212,    90,   167,   162,    91,   165,   108,   279,
    95,   -87,   104,   105,   106,   107,   -87,   238,   109,   280,
   109,   239,   108,   134,   194,   151,   115,   235,   135,   110,
   111,   110,   111,   -93,   109,   155,    65,     2,   144,   266,
   -94,   188,   177,    82,    83,   110,   111,   112,   163,   112,
    65,    75,   195,    84,   134,   220,   221,   203,   222,   159,
     3,     4,   114,   112,   223,    52,    53,     7,   250,     8,
     9,    10,    11,   117,    98,    54,    55,    85,  -130,   248,
    15,    50,   134,   168,   169,   170,   171,   172,   136,    16,
    17,    86,    18,   137,   220,   221,    78,   191,  -130,   114,
    24,    25,    26,   223,   241,   242,   157,    27,    28,    29,
    30,   217,   188,    81,    31,   193,   118,   136,   206,    92,
   119,    93,   205,   207,   108,   120,   121,   122,   226,   157,
   227,    94,     1,     2,     3,     4,   109,   100,   208,     5,
     6,     7,   101,     8,     9,    10,    11,   110,   111,    12,
    13,    14,   114,   214,    15,   216,   218,   252,   134,   225,
    66,    67,   261,    16,    17,   112,    18,    19,    20,    21,
    22,    23,   129,   130,    24,    25,    26,   228,   138,   229,
   230,    27,   231,   106,    28,    29,    30,    51,    31,     3,
     4,   108,   216,   139,    52,    53,     7,   148,     8,     9,
    10,    11,   149,   109,    54,    55,   134,   150,   260,    15,
   160,    49,   161,   255,   110,   111,   166,   173,    16,    17,
   175,    18,   262,   174,   263,   178,   264,   265,   185,    24,
    25,    26,   112,   268,   186,   187,    27,    59,   189,     3,
     4,   190,   196,    31,    52,    53,     7,   192,     8,     9,
    10,    11,   198,   117,    54,    55,   199,   202,   209,    15,
   215,   210,   104,   105,   106,   107,   211,   219,    16,    17,
   213,    18,   108,   232,   236,   104,   105,   106,   107,    24,
    25,    26,   233,     2,   109,   108,    27,   234,   104,   105,
   106,   107,   237,    31,   249,   110,   111,   109,   108,   244,
   179,   104,   105,   106,   107,   180,   121,   122,   110,   111,
   109,   108,   253,   112,   104,   105,   106,   107,   275,   254,
   257,   110,   111,   109,   108,   258,   112,   259,   270,   271,
   131,   276,   274,   277,   110,   111,   109,   112,   281,   112,
   282,   283,   117,   132,   284,   285,   286,   110,   111,   287,
   289,   288,   112,   290,   291,   292,   133,   104,   105,   106,
   107,   296,   294,   293,   297,   112,    99,   108,   201,   140,
   104,   105,   106,   107,   240,   147,   267,   103,    42,   109,
   108,   153,   278,   104,   105,   106,   107,   246,   200,   179,
   110,   111,   109,   108,   120,   121,   122,     0,   243,     0,
     0,     0,     0,   110,   111,   109,     0,     0,   112,    96,
     0,     0,   158,     2,     0,     0,   110,   111,     0,    82,
    83,   112,     0,     0,     0,   164,    97,    98,    14,    84,
     0,     0,     0,   117,   112,     0,  -127,     0,   269,  -127,
  -127,  -127,  -127,     0,    19,    20,    21,    22,    23,  -127,
     0,     0,     0,    85,     0,   104,   105,   106,   107,     0,
   256,    28,    29,    30,     0,   108,     0,    86,     0,     0,
     0,     0,  -127,  -127,     0,     0,    49,   109,     0,     0,
   179,   104,   105,   106,   107,   251,   121,   122,   110,   111,
  -127,   108,     0,     0,   104,   105,   106,   107,     0,     0,
     0,     0,     0,   109,   108,     0,   112,     0,     0,   141,
     0,     0,     0,     0,   110,   111,   109,     0,     2,   104,
   105,   106,   107,     0,    82,    83,     0,   110,   111,   108,
     0,     0,   112,     0,    84,   142,     0,     0,     0,     0,
     0,   109,     0,     0,     0,   112,     0,     0,   272,     0,
     0,  -110,   110,   111,  -110,  -110,  -110,  -110,    85,   104,
   105,   106,   107,     0,  -110,     0,     0,     0,     0,   108,
   112,     0,    86,   273,   104,   105,   106,   107,     0,     0,
     0,   109,     0,     0,   108,     0,   176,  -110,  -110,     0,
     0,     0,   110,   111,     0,     0,   109,     0,     0,   104,
   105,   106,   107,     0,     0,  -110,   245,   110,   111,   108,
   112,     0,   104,   105,   106,   107,     0,     0,   104,   105,
   106,   109,   108,     0,     0,   112,     0,     0,   108,     0,
     0,   247,   110,   111,   109,     0,     0,     0,     0,     0,
   109,     0,     0,     0,     0,   110,   111,     0,     0,     0,
   112,   110,   111,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,   112,     0,     0,     0,     0,     0,   112
};

static const short yycheck[] = {     0,
    49,    87,    47,    81,   124,    22,     9,     7,     9,    10,
    11,     9,     7,     7,    15,    16,    17,    18,     3,     4,
     5,     6,   167,    24,   102,     7,    27,     7,    13,    17,
    31,    53,     3,     4,     5,     6,    58,    53,    25,    27,
    25,    57,    13,    53,    28,    40,    47,   192,    58,    36,
    37,    36,    37,     0,    25,    58,    50,     8,    56,    59,
     0,   181,    40,    14,    15,    36,    37,    54,    50,    54,
    50,     7,    56,    24,    53,     4,     5,    56,     7,    64,
     9,    10,    60,    54,    13,    14,    15,    16,    59,    18,
    19,    20,    21,     7,    25,    24,    25,    48,    40,    40,
    29,    50,    53,   104,   105,   106,   107,   108,    53,    38,
    39,    62,    41,    58,     4,     5,     7,   134,    60,    60,
    49,    50,    51,    13,   202,   203,   212,    56,    59,    60,
    61,   176,   252,   150,    63,   136,    50,    53,    53,     7,
    54,     7,    58,    58,    13,    59,    60,    61,    53,   235,
    55,    25,     7,     8,     9,    10,    25,    52,   159,    14,
    15,    16,    12,    18,    19,    20,    21,    36,    37,    24,
    25,    26,    60,   174,    29,   176,   177,   226,    53,   180,
    22,    23,    57,    38,    39,    54,    41,    42,    43,    44,
    45,    46,    53,     7,    49,    50,    51,    53,    57,    55,
    53,    56,    55,     5,    59,    60,    61,     7,    63,     9,
    10,    13,   213,    57,    14,    15,    16,    28,    18,    19,
    20,    21,    53,    25,    24,    25,    53,    53,    55,    29,
    59,    40,    60,   234,    36,    37,    52,     7,    38,    39,
    50,    41,   243,    56,   245,     5,   247,   248,     7,    49,
    50,    51,    54,   254,     7,     7,    56,     7,    50,     9,
    10,    57,     9,    63,    14,    15,    16,    54,    18,    19,
    20,    21,    53,     7,    24,    25,     7,    54,    57,    29,
    55,    57,     3,     4,     5,     6,    57,    50,    38,    39,
    57,    41,    13,    59,    50,     3,     4,     5,     6,    49,
    50,    51,    60,     8,    25,    13,    56,    53,     3,     4,
     5,     6,    50,    63,    59,    36,    37,    25,    13,     9,
    54,     3,     4,     5,     6,    59,    60,    61,    36,    37,
    25,    13,    59,    54,     3,     4,     5,     6,    59,    59,
    57,    36,    37,    25,    13,     9,    54,    56,    28,    50,
    58,    50,    59,    57,    36,    37,    25,    54,    28,    54,
     9,     9,     7,    58,    50,    37,    34,    36,    37,    34,
     9,    56,    54,    50,    50,    34,    58,     3,     4,     5,
     6,     0,    57,    50,     0,    54,    32,    13,   150,    58,
     3,     4,     5,     6,   198,    74,   253,    40,     0,    25,
    13,    84,   272,     3,     4,     5,     6,   213,   149,    54,
    36,    37,    25,    13,    59,    60,    61,    -1,   204,    -1,
    -1,    -1,    -1,    36,    37,    25,    -1,    -1,    54,     7,
    -1,    -1,    58,     8,    -1,    -1,    36,    37,    -1,    14,
    15,    54,    -1,    -1,    -1,    58,    24,    25,    26,    24,
    -1,    -1,    -1,     7,    54,    -1,     0,    -1,    58,     3,
     4,     5,     6,    -1,    42,    43,    44,    45,    46,    13,
    -1,    -1,    -1,    48,    -1,     3,     4,     5,     6,    -1,
    55,    59,    60,    61,    -1,    13,    -1,    62,    -1,    -1,
    -1,    -1,    36,    37,    -1,    -1,    40,    25,    -1,    -1,
    54,     3,     4,     5,     6,    59,    60,    61,    36,    37,
    54,    13,    -1,    -1,     3,     4,     5,     6,    -1,    -1,
    -1,    -1,    -1,    25,    13,    -1,    54,    -1,    -1,    57,
    -1,    -1,    -1,    -1,    36,    37,    25,    -1,     8,     3,
     4,     5,     6,    -1,    14,    15,    -1,    36,    37,    13,
    -1,    -1,    54,    -1,    24,    57,    -1,    -1,    -1,    -1,
    -1,    25,    -1,    -1,    -1,    54,    -1,    -1,    57,    -1,
    -1,     0,    36,    37,     3,     4,     5,     6,    48,     3,
     4,     5,     6,    -1,    13,    -1,    -1,    -1,    -1,    13,
    54,    -1,    62,    57,     3,     4,     5,     6,    -1,    -1,
    -1,    25,    -1,    -1,    13,    -1,    30,    36,    37,    -1,
    -1,    -1,    36,    37,    -1,    -1,    25,    -1,    -1,     3,
     4,     5,     6,    -1,    -1,    54,    35,    36,    37,    13,
    54,    -1,     3,     4,     5,     6,    -1,    -1,     3,     4,
     5,    25,    13,    -1,    -1,    54,    -1,    -1,    13,    -1,
    -1,    35,    36,    37,    25,    -1,    -1,    -1,    -1,    -1,
    25,    -1,    -1,    -1,    -1,    36,    37,    -1,    -1,    -1,
    54,    36,    37,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    54,    -1,    -1,    -1,    -1,    -1,    54
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
#line 1597 "sslparser.cpp"

  switch (yyn) {

case 1:
#line 217 "sslparser.y"
{
            the_asgn = yyvsp[0].regtransfer;
        ;
    break;}
case 2:
#line 220 "sslparser.y"
{
            the_asgn = new Assign(
                new Terminal(opNil),
                yyvsp[0].exp);
        ;
    break;}
case 7:
#line 236 "sslparser.y"
{
            Dict.fetchExecCycle = yyvsp[0].rtlist;
        ;
    break;}
case 14:
#line 261 "sslparser.y"
{ Dict.fixupParams(); ;
    break;}
case 17:
#line 274 "sslparser.y"
{
            // Note: the below copies the list of strings!
            Dict.DetParamMap[yyvsp[-4].str].params = *yyvsp[-1].parmlist;
            Dict.DetParamMap[yyvsp[-4].str].kind = PARAM_VARIANT;
            //delete $4;
        ;
    break;}
case 18:
#line 288 "sslparser.y"
{
            std::map<std::string, InsNameElem*> m;
            ParamEntry &param = Dict.DetParamMap[yyvsp[-4].str];
            Statement* asgn = new Assign(yyvsp[-1].typ, new Terminal(opNil), yyvsp[0].exp);
            // Note: The below 2 copy lists of strings (to be deleted below!)
            param.params = *yyvsp[-3].parmlist;
            param.funcParams = *yyvsp[-2].parmlist;
            param.asgn = asgn;
            param.kind = PARAM_ASGN;
            
            if( param.funcParams.size() != 0 )
                param.kind = PARAM_LAMBDA;
            //delete $2;
            //delete $3;
        ;
    break;}
case 19:
#line 305 "sslparser.y"
{ yyval.parmlist = yyvsp[-1].parmlist; ;
    break;}
case 20:
#line 306 "sslparser.y"
{ yyval.parmlist = new std::list<std::string>(); ;
    break;}
case 21:
#line 310 "sslparser.y"
{
                    bFloat = false;
                ;
    break;}
case 23:
#line 313 "sslparser.y"
{
                    bFloat = true;
                ;
    break;}
case 27:
#line 323 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-2].str) != Dict.RegMap.end())
                    yyerror("Name reglist decared twice\n");
                Dict.RegMap[yyvsp[-2].str] = yyvsp[0].num;
            ;
    break;}
case 28:
#line 328 "sslparser.y"
{
                if (Dict.RegMap.find(yyvsp[-5].str) != Dict.RegMap.end())
                    yyerror("Name reglist declared twice\n");
                Dict.addRegister( yyvsp[-5].str, yyvsp[0].num, yyvsp[-3].num, bFloat);
            ;
    break;}
case 29:
#line 333 "sslparser.y"
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
#line 368 "sslparser.y"
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
#line 390 "sslparser.y"
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
                    //delete $2;
                }
            ;
    break;}
case 32:
#line 405 "sslparser.y"
{
                std::list<std::string>::iterator loc = yyvsp[-6].strlist->begin();
                for (; loc != yyvsp[-6].strlist->end(); loc++) {
                    if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                        yyerror("Name reglist declared twice\n");
		    Dict.addRegister(loc->c_str(), yyvsp[0].num, yyvsp[-3].num, bFloat);
                }
                //delete $2;
            ;
    break;}
case 33:
#line 417 "sslparser.y"
{
                yyvsp[-2].strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 34:
#line 420 "sslparser.y"
{
                yyval.strlist = new std::list<std::string>;
                yyval.strlist->push_back(yyvsp[0].str);
            ;
    break;}
case 35:
#line 429 "sslparser.y"
{
                // Note: $2 is a list of strings
                Dict.FlagFuncs[yyvsp[-5].str] = new FlagDef(listStrToExp(yyvsp[-4].parmlist), yyvsp[-1].rtlist);
            ;
    break;}
case 36:
#line 436 "sslparser.y"
{
                if (ConstTable.find(yyvsp[-2].str) != ConstTable.end())
                    yyerror("Constant declared twice");
                ConstTable[std::string(yyvsp[-2].str)] = yyvsp[0].num;
            ;
    break;}
case 37:
#line 442 "sslparser.y"
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
#line 456 "sslparser.y"
{
            TableDict[yyvsp[-2].str] = yyvsp[0].tab;
        ;
    break;}
case 39:
#line 462 "sslparser.y"
{
            yyval.tab = new Table(*yyvsp[0].namelist);
            //delete $1;
        ;
    break;}
case 40:
#line 467 "sslparser.y"
{
            yyval.tab = new OpTable(*yyvsp[0].namelist);
            //delete $1;
        ;
    break;}
case 41:
#line 471 "sslparser.y"
{
            yyval.tab = new ExprTable(*yyvsp[0].exprlist);
            //delete $1;
        ;
    break;}
case 42:
#line 478 "sslparser.y"
{
            // cross-product of two str_expr's
            std::deque<std::string>::iterator i, j;
            yyval.namelist = new std::deque<std::string>;
            for (i = yyvsp[-1].namelist->begin(); i != yyvsp[-1].namelist->end(); i++)
                for (j = yyvsp[0].namelist->begin(); j != yyvsp[0].namelist->end(); j++)
                    yyval.namelist->push_back((*i) + (*j));
            //delete $1;
            //delete $2;
        ;
    break;}
case 43:
#line 488 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 44:
#line 494 "sslparser.y"
{
            // want to append $3 to $1
            // The following causes a massive warning message about mixing
            // signed and unsigned
            yyvsp[-2].namelist->insert(yyvsp[-2].namelist->end(), yyvsp[0].namelist->begin(), yyvsp[0].namelist->end());
            //delete $3;
            yyval.namelist = yyvsp[-2].namelist;
        ;
    break;}
case 45:
#line 502 "sslparser.y"
{
            yyvsp[-3].namelist->push_back("");
        ;
    break;}
case 46:
#line 505 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 47:
#line 511 "sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 48:
#line 514 "sslparser.y"
{
            yyval.namelist = yyvsp[0].namelist;
        ;
    break;}
case 49:
#line 520 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>;
            yyval.namelist->push_back("");
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 50:
#line 525 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>(1, yyvsp[-1].str);
        ;
    break;}
case 51:
#line 528 "sslparser.y"
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
#line 543 "sslparser.y"
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
#line 562 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 54:
#line 566 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 55:
#line 570 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 56:
#line 577 "sslparser.y"
{
            yyval.namelist = yyvsp[-1].namelist;
        ;
    break;}
case 57:
#line 584 "sslparser.y"
{
            yyval.namelist = yyvsp[-4].namelist;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 58:
#line 588 "sslparser.y"
{
            yyval.namelist = new std::deque<std::string>;
            yyval.namelist->push_back(yyvsp[-1].str);
        ;
    break;}
case 59:
#line 596 "sslparser.y"
{
            yyval.exprlist = yyvsp[-1].exprlist;
        ;
    break;}
case 60:
#line 603 "sslparser.y"
{
            yyval.exprlist = yyvsp[-4].exprlist;
            yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
    break;}
case 61:
#line 607 "sslparser.y"
{
            yyval.exprlist = new std::deque<Exp*>;
            yyval.exprlist->push_back(yyvsp[-1].exp);
        ;
    break;}
case 62:
#line 615 "sslparser.y"
{
            yyvsp[0].insel->getrefmap(indexrefmap);
        //     $3           $4
        ;
    break;}
case 63:
#line 618 "sslparser.y"
{
            // This function expands the tables and saves the expanded RTLs
            // to the dictionary
            expandTables(yyvsp[-3].insel, yyvsp[-1].parmlist, yyvsp[0].rtlist, Dict);
        ;
    break;}
case 64:
#line 626 "sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 65:
#line 629 "sslparser.y"
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
 
            temp = new InsNameElem(nm.c_str());
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(temp);
        ;
    break;}
case 66:
#line 653 "sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[0].str);
        ;
    break;}
case 67:
#line 656 "sslparser.y"
{
            yyval.insel = yyvsp[0].insel;
        ;
    break;}
case 68:
#line 659 "sslparser.y"
{
            yyval.insel = yyvsp[-1].insel;
            yyval.insel->append(yyvsp[0].insel);
        ;
    break;}
case 69:
#line 666 "sslparser.y"
{
            yyval.insel = new InsOptionElem(yyvsp[-1].str);
        ;
    break;}
case 70:
#line 669 "sslparser.y"
{
            std::ostringstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(STR(o));
            } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->records.size())) {
                o << "Can't get element " << yyvsp[-1].num << " of table " << yyvsp[-2].str << ".\n";
                yyerror(STR(o));
            } else
                yyval.insel = new InsNameElem(TableDict[yyvsp[-2].str]->records[yyvsp[-1].num].c_str());
        ;
    break;}
case 71:
#line 682 "sslparser.y"
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
#line 691 "sslparser.y"
{
            std::ostringstream o;
            if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
                o << "Table " << yyvsp[-2].str << " has not been declared.\n";
                yyerror(STR(o));
            } else if ((yyvsp[-1].num < 0) || (yyvsp[-1].num >= (int)TableDict[yyvsp[-2].str]->records.size())) {
                o << "Can't get element " << yyvsp[-1].num << " of table " << yyvsp[-2].str << ".\n";
                yyerror(STR(o));
            } else
                yyval.insel = new InsNameElem(TableDict[yyvsp[-2].str]->records[yyvsp[-1].num].c_str());
        ;
    break;}
case 73:
#line 702 "sslparser.y"
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
#line 711 "sslparser.y"
{
            yyval.insel = new InsNameElem(yyvsp[-1].str);
        ;
    break;}
case 75:
#line 717 "sslparser.y"
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
#line 726 "sslparser.y"
{
            yyval.rtlist = new RTL(STMT_ASSIGN);
            if (yyvsp[0].regtransfer != NULL)
                yyval.rtlist->appendStmt(yyvsp[0].regtransfer);
        ;
    break;}
case 77:
#line 734 "sslparser.y"
{
            yyval.regtransfer = yyvsp[0].regtransfer;
        ;
    break;}
case 78:
#line 740 "sslparser.y"
{
            std::ostringstream o;
            if (Dict.FlagFuncs.find(yyvsp[-2].str) != Dict.FlagFuncs.end()) {
                // Note: SETFFLAGS assigns to the floating point flags
                // All others to the integer flags
                bool bFloat = strcmp(yyvsp[-2].str, "SETFFLAGS") == 0;
                OPER op = bFloat ? opFflags : opFlags;
                yyval.regtransfer = new Assign(
                    new Terminal(op),
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
#line 757 "sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 80:
#line 761 "sslparser.y"
{
            yyval.regtransfer = 0;
        ;
    break;}
case 81:
#line 764 "sslparser.y"
{
        yyval.regtransfer = NULL;
    ;
    break;}
case 82:
#line 770 "sslparser.y"
{
            // Not sure why the below is commented out (MVE)
/*          Location* pFlag = Location::regOf(Dict.RegMap[$3]);
            $1->push_back(pFlag);
            $$ = $1;
*/          yyval.explist = 0;
        ;
    break;}
case 83:
#line 777 "sslparser.y"
{
/*          std::list<Exp*>* tmp = new std::list<Exp*>;
            Unary* pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
            tmp->push_back(pFlag);
            $$ = tmp;
*/          yyval.explist = 0;
        ;
    break;}
case 84:
#line 788 "sslparser.y"
{
            assert(yyvsp[0].str != 0);
            yyvsp[-2].parmlist->push_back(yyvsp[0].str);
            yyval.parmlist = yyvsp[-2].parmlist;
        ;
    break;}
case 85:
#line 794 "sslparser.y"
{
            yyval.parmlist = new std::list<std::string>;
            yyval.parmlist->push_back(yyvsp[0].str);
        ;
    break;}
case 86:
#line 798 "sslparser.y"
{
            yyval.parmlist = new std::list<std::string>;
        ;
    break;}
case 87:
#line 803 "sslparser.y"
{
            Dict.ParamSet.insert(yyvsp[0].str);       // Not sure if we need this set
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 88:
#line 809 "sslparser.y"
{
            yyval.explist->push_back(yyvsp[0].exp);
        ;
    break;}
case 89:
#line 813 "sslparser.y"
{
            yyval.explist = new std::list<Exp*>;
            yyval.explist->push_back(yyvsp[0].exp);
        ;
    break;}
case 90:
#line 818 "sslparser.y"
{
            yyval.explist = new std::list<Exp*>;
        ;
    break;}
case 91:
#line 826 "sslparser.y"
{
            Assign* a = new Assign(yyvsp[-5].typ, yyvsp[-2].exp, yyvsp[0].exp);
            a->setGuard(yyvsp[-4].exp);
            yyval.regtransfer = a;
        ;
    break;}
case 92:
#line 833 "sslparser.y"
{
            // update the size of any generated RT's
            yyval.regtransfer = new Assign(yyvsp[-3].typ, yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 93:
#line 839 "sslparser.y"
{
            yyval.regtransfer = new Assign(
                new Terminal(opNil),
                new Terminal(opFpush));
        ;
    break;}
case 94:
#line 844 "sslparser.y"
{
            yyval.regtransfer = new Assign(
                new Terminal(opNil),
                new Terminal(opFpop));
        ;
    break;}
case 95:
#line 850 "sslparser.y"
{
        //  $1      $2
            yyval.regtransfer = new Assign(yyvsp[-1].typ, NULL, yyvsp[0].exp);
        ;
    break;}
case 96:
#line 857 "sslparser.y"
{
            yyval.exp = new Const(yyvsp[0].num);
        ;
    break;}
case 97:
#line 861 "sslparser.y"
{
            yyval.exp = new Const(yyvsp[0].dbl);
        ;
    break;}
case 98:
#line 865 "sslparser.y"
{
            yyval.exp = new Unary(opTemp, new Const(yyvsp[0].str));
        ;
    break;}
case 99:
#line 869 "sslparser.y"
{
            yyval.exp = yyvsp[-1].exp;
        ;
    break;}
case 100:
#line 873 "sslparser.y"
{
            yyval.exp = yyvsp[0].exp;
        ;
    break;}
case 101:
#line 877 "sslparser.y"
{
            yyval.exp = new Ternary(opTern, yyvsp[-5].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
    break;}
case 102:
#line 881 "sslparser.y"
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
#line 891 "sslparser.y"
{
            yyval.exp = new Unary(opAddrOf, yyvsp[-1].exp);
        ;
    break;}
case 104:
#line 897 "sslparser.y"
{
            yyval.exp = new Ternary(strToOper(yyvsp[-6].str), new Const(yyvsp[-5].num), new Const(yyvsp[-3].num), yyvsp[-1].exp);
        ;
    break;}
case 105:
#line 902 "sslparser.y"
{
            yyval.exp = new Unary(opFtrunc, yyvsp[-1].exp);
        ;
    break;}
case 106:
#line 907 "sslparser.y"
{
            yyval.exp = new Unary(opFabs, yyvsp[-1].exp);
        ;
    break;}
case 107:
#line 912 "sslparser.y"
{
            yyval.exp = new Terminal(opFpush);
        ;
    break;}
case 108:
#line 915 "sslparser.y"
{
            yyval.exp = new Terminal(opFpop);
        ;
    break;}
case 109:
#line 920 "sslparser.y"
{
            yyval.exp = new Unary(strToOper(yyvsp[-2].str), yyvsp[-1].exp);
        ;
    break;}
case 110:
#line 926 "sslparser.y"
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
case 111:
#line 954 "sslparser.y"
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
                    //delete $2;          // Delete the list of char*s
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
case 112:
#line 981 "sslparser.y"
{
			yyval.exp = makeSuccessor(yyvsp[-1].exp);
		;
    break;}
case 113:
#line 987 "sslparser.y"
{
            yyval.exp = new Unary(opSignExt, yyvsp[-1].exp);
        ;
    break;}
case 114:
#line 996 "sslparser.y"
{
            // opSize is deprecated, but for old SSL files we'll make a TypedExp
            if (yyvsp[0].num == STD_SIZE)
                yyval.exp = yyvsp[-1].exp;
            else
                yyval.exp = new TypedExp(new IntegerType(yyvsp[0].num), yyvsp[-1].exp);
        ;
    break;}
case 115:
#line 1004 "sslparser.y"
{
            yyval.exp = new Unary(opNot, yyvsp[0].exp);
        ;
    break;}
case 116:
#line 1008 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 117:
#line 1012 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 118:
#line 1016 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 119:
#line 1020 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 120:
#line 1024 "sslparser.y"
{
            yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
        ;
    break;}
case 121:
#line 1031 "sslparser.y"
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
case 122:
#line 1057 "sslparser.y"
{
            yyval.exp = yyvsp[0].exp;
        ;
    break;}
case 123:
#line 1068 "sslparser.y"
{
            bool isFlag = strstr(yyvsp[0].str, "flags") != 0;
            std::map<std::string, int>::const_iterator it = Dict.RegMap.find(yyvsp[0].str);
            if (it == Dict.RegMap.end() && !isFlag) {
                std::ostringstream ost;
                ost << "register `" << yyvsp[0].str << "' is undefined";
                yyerror(STR(ost));
            } else if (isFlag || it->second == -1) {
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
                yyval.exp = Location::regOf(it->second);
            }
        ;
    break;}
case 124:
#line 1093 "sslparser.y"
{
            yyval.exp = Location::regOf(yyvsp[-1].exp);
        ;
    break;}
case 125:
#line 1097 "sslparser.y"
{
            int regNum;
            sscanf(yyvsp[0].str, "r%d", &regNum);
            yyval.exp = Location::regOf(regNum);
        ;
    break;}
case 126:
#line 1103 "sslparser.y"
{
            yyval.exp = Location::memOf(yyvsp[-1].exp);
        ;
    break;}
case 127:
#line 1107 "sslparser.y"
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
case 128:
#line 1125 "sslparser.y"
{
            yyval.exp = new Ternary(opAt, yyvsp[-6].exp, yyvsp[-3].exp, yyvsp[-1].exp);
        ;
    break;}
case 129:
#line 1129 "sslparser.y"
{
            yyval.exp = new Unary(opPostVar, yyvsp[-1].exp);
        ;
    break;}
case 130:
#line 1132 "sslparser.y"
{
			yyval.exp = makeSuccessor(yyvsp[-1].exp);
		;
    break;}
case 131:
#line 1138 "sslparser.y"
{
            yyval.num = yyvsp[-1].num;
        ;
    break;}
case 132:
#line 1144 "sslparser.y"
{
            Dict.bigEndian = (strcmp(yyvsp[0].str, "BIG") == 0);
        ;
    break;}
case 133:
#line 1149 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 134:
#line 1152 "sslparser.y"
{
            yyval.str = yyvsp[0].str;
        ;
    break;}
case 135:
#line 1158 "sslparser.y"
{
            char c = yyvsp[0].str[1];
            if (c == '*') yyval.typ = new IntegerType;
            if (isdigit(c)) {
                int size;
                // Skip star (hence +1)
                sscanf(yyvsp[0].str+1, "%d", &size);
                yyval.typ = new IntegerType(size);
            } else {
                int size;
                // Skip star and letter
                sscanf(yyvsp[0].str+2, "%d", &size);
                if (size == 0) size = STD_SIZE;
                switch (c) {
                    case 'i': yyval.typ = new IntegerType(size); break;
                    case 'f': yyval.typ = new FloatType(size); break;
                    case 'c': yyval.typ = new CharType; break;
                    default:
                        std::cerr << "Unexpected char " << c <<
                            " in assign type\n";
                        yyval.typ = new IntegerType;
                }
            }
        ;
    break;}
case 139:
#line 1197 "sslparser.y"
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
#line 2889 "sslparser.cpp"
#line 1200 "sslparser.y"


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
            // fsize, ftoi, fround NOTE: ftrunc handled separately
            // because it is a unary
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
        case 'p':
            // pow
            return opPow;
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
    // s could be %pc, %afp, %agp, %CF, %ZF, %OF, %NF, %DF, %flags, %fflags
    if (s[2] == 'F') {
        if (s[1] <= 'N') {
            if (s[1] == 'C') return opCF;
            if (s[1] == 'N') return opNF;
            return opDF;
        } else {
            if (s[1] == 'O') return opOF;
            return opZF;
        }
    }
    if (s[1] == 'p') return opPC;
    if (s[1] == 'a') {
        if (s[2] == 'f') return opAFP;
        if (s[2] == 'g') return opAGP;
    } else if (s[1] == 'f') {
        if (s[2] == 'l') return opFlags;
        if (s[2] == 'f') return opFflags;
    }
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
            }
        }
   
        if (Dict.appendToDict(nam, *params, *rtl) != 0) {
            o << "Pattern " << iname->getinspattern()
              << " conflicts with an earlier declaration of " << nam <<
              ".\n";
            yyerror(STR(o));
        }
    }
    //delete iname;
    //delete params;
    //delete o_rtlist;
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
