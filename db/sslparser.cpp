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
#line 37 "sslparser.y"

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <sstream>
#include "types.h"
#include "rtl.h"
#include "table.h"
#include "insnameelem.h"
#include "util.h"			// E.g. str()

#ifdef WIN32
#include <malloc.h>
#endif

class SSLScanner;

#line 62 "sslparser.y"
typedef union {
	Exp*			exp;
	char*			str;
	int				num;
	double			dbl;
	Statement*		regtransfer;
	Type*			typ;
	
	Table*			tab;
	InsNameElem*	insel;
	std::list<std::string>*	  parmlist;
	std::list<std::string>*	  strlist;
	std::deque<Exp*>*	 exprlist;
	std::deque<std::string>*  namelist;
	std::list<Exp*>*	 explist;
	RTL*			rtlist;
} yy_SSLParser_stype;
#define YY_SSLParser_STYPE yy_SSLParser_stype
#line 81 "sslparser.y"

#include "sslscanner.h"
OPER strToTerm(char* s);		// Convert string to a Terminal (if possible)
Exp* listExpToExp(std::list<Exp*>* le);	 // Convert a STL list of Exp* to opList
Exp* listStrToExp(std::list<std::string>* ls);// Convert a STL list of strings to opList
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
OPER	strToOper(const char*s); /* Convert string to an operator */ \
static	Statement* parseExp(const char *str); /* Parse an expression or assignment from a string */ \
/* The code for expanding tables and saving to the dictionary */ \
void	expandTables(InsNameElem* iname, std::list<std::string>* params, RTL* o_rtlist, RTLInstDict& Dict); \
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
#define	LNOT	285
#define	FNEG	286
#define	THEN	287
#define	LOOKUP_RDC	288
#define	BOGUS	289
#define	ASSIGN	290
#define	TO	291
#define	COLON	292
#define	S_E	293
#define	AT	294
#define	ADDR	295
#define	REG_IDX	296
#define	EQUATE	297
#define	MEM_IDX	298
#define	TOK_INTEGER	299
#define	TOK_FLOAT	300
#define	FAST	301
#define	OPERAND	302
#define	FETCHEXEC	303
#define	CAST_OP	304
#define	FLAGMACRO	305
#define	SUCCESSOR	306
#define	NUM	307
#define	FLOATNUM	308
#define	FCHS	309


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
#line 483 "sslparser.cpp"
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
static const int LNOT;
static const int FNEG;
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
static const int FCHS;


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 544 "sslparser.cpp"
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
	,LNOT=285
	,FNEG=286
	,THEN=287
	,LOOKUP_RDC=288
	,BOGUS=289
	,ASSIGN=290
	,TO=291
	,COLON=292
	,S_E=293
	,AT=294
	,ADDR=295
	,REG_IDX=296
	,EQUATE=297
	,MEM_IDX=298
	,TOK_INTEGER=299
	,TOK_FLOAT=300
	,FAST=301
	,OPERAND=302
	,FETCHEXEC=303
	,CAST_OP=304
	,FLAGMACRO=305
	,SUCCESSOR=306
	,NUM=307
	,FLOATNUM=308
	,FCHS=309


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
#line 633 "sslparser.cpp"
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
const int YY_SSLParser_CLASS::LNOT=285;
const int YY_SSLParser_CLASS::FNEG=286;
const int YY_SSLParser_CLASS::THEN=287;
const int YY_SSLParser_CLASS::LOOKUP_RDC=288;
const int YY_SSLParser_CLASS::BOGUS=289;
const int YY_SSLParser_CLASS::ASSIGN=290;
const int YY_SSLParser_CLASS::TO=291;
const int YY_SSLParser_CLASS::COLON=292;
const int YY_SSLParser_CLASS::S_E=293;
const int YY_SSLParser_CLASS::AT=294;
const int YY_SSLParser_CLASS::ADDR=295;
const int YY_SSLParser_CLASS::REG_IDX=296;
const int YY_SSLParser_CLASS::EQUATE=297;
const int YY_SSLParser_CLASS::MEM_IDX=298;
const int YY_SSLParser_CLASS::TOK_INTEGER=299;
const int YY_SSLParser_CLASS::TOK_FLOAT=300;
const int YY_SSLParser_CLASS::FAST=301;
const int YY_SSLParser_CLASS::OPERAND=302;
const int YY_SSLParser_CLASS::FETCHEXEC=303;
const int YY_SSLParser_CLASS::CAST_OP=304;
const int YY_SSLParser_CLASS::FLAGMACRO=305;
const int YY_SSLParser_CLASS::SUCCESSOR=306;
const int YY_SSLParser_CLASS::NUM=307;
const int YY_SSLParser_CLASS::FLOATNUM=308;
const int YY_SSLParser_CLASS::FCHS=309;


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
#line 702 "sslparser.cpp"


#define	YYFINAL		300
#define	YYFLAG		-32768
#define	YYNTBASE	68

#define YYTRANSLATE(x) ((unsigned)(x) <= 309 ? yytranslate[x] : 115)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    62,     2,    64,     2,     2,    63,    66,
    61,     2,     2,    56,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    55,     2,
     2,     2,    67,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    59,     2,    60,     2,    65,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    57,     2,    58,     2,     2,     2,     2,     2,
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
    46,    47,    48,    49,    50,    51,    52,    53,    54
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
   317,   324,   329,   331,   333,   336,   338,   340,   344,   346,
   354,   358,   366,   370,   374,   376,   378,   382,   386,   390,
   394,   397,   400,   403,   406,   409,   413,   417,   421,   425,
   429,   435,   437,   439,   443,   445,   449,   451,   459,   461,
   464,   468,   472,   475,   477,   479,   481,   484,   488,   490
};

static const short yyrhs[] = {   104,
     0,   106,     0,    69,     0,    69,    70,    55,     0,    70,
    55,     0,    93,     0,    48,    98,     0,    81,     0,    82,
     0,   109,     0,   112,     0,    74,     0,    80,     0,    47,
    71,     0,    71,    56,    72,     0,    72,     0,   102,    42,
    57,   101,    58,     0,   102,   101,    73,   111,   106,     0,
    59,   101,    60,     0,     0,     0,    44,    75,    77,     0,
     0,    45,    76,    77,     0,    77,    56,    78,     0,    78,
     0,     9,    28,    52,     0,     9,    59,    52,    60,    28,
    52,     0,     9,    59,    52,    60,    28,    52,    27,     9,
    36,     9,     0,     9,    59,    52,    60,    28,    52,    17,
     9,    39,    59,    52,    36,    52,    60,     0,    59,    79,
    60,    59,    52,    60,    28,    52,    36,    52,     0,    59,
    79,    60,    59,    52,    60,    28,    52,     0,    79,    56,
     9,     0,     9,     0,    24,   101,    61,    57,    98,    58,
     0,     7,    42,    52,     0,     7,    42,    52,     5,    52,
     0,     7,    42,    83,     0,    84,     0,    89,     0,    91,
     0,    84,    86,     0,    86,     0,    85,    56,    84,     0,
    85,    56,    62,    62,     0,    84,     0,    57,    85,    58,
     0,    87,     0,    63,     7,    63,     0,    62,     7,    62,
     0,    64,     7,     0,     7,     0,     4,     0,     5,     0,
    13,     0,    57,    90,    58,     0,    90,    56,    62,    88,
    62,     0,    62,    88,    62,     0,    57,    92,    58,     0,
    92,    56,    62,   106,    62,     0,    62,   106,    62,     0,
     0,    95,    94,   101,    98,     0,    96,     0,    95,    12,
     0,     7,     0,    97,     0,    96,    97,     0,    63,     7,
    63,     0,    25,    52,    60,     0,    25,     7,    60,     0,
    64,    25,    52,    60,     0,    64,    25,     7,    60,     0,
    62,     7,    62,     0,    98,    99,     0,    99,     0,   104,
     0,    24,   103,    61,     0,    50,   100,    61,     0,    50,
    61,     0,    65,     0,   100,    56,     9,     0,     9,     0,
   101,    56,   102,     0,   102,     0,     0,     7,     0,   103,
    56,   106,     0,   106,     0,     0,   111,   106,    32,   107,
    42,   106,     0,   111,   107,    42,   106,     0,    14,     0,
    15,     0,   111,   106,     0,    52,     0,    53,     0,    66,
   106,    61,     0,   107,     0,    59,   106,    67,   106,    37,
   106,    60,     0,    40,   106,    61,     0,    18,    52,    56,
    52,    56,   106,    61,     0,    19,   106,    61,     0,    21,
   106,    61,     0,    14,     0,    15,     0,    20,   106,    61,
     0,    25,     7,    60,     0,    24,   103,    61,     0,    51,
   106,    61,     0,   106,    38,     0,   106,   108,     0,    29,
   106,     0,    30,   106,     0,    31,   106,     0,   106,    13,
   106,     0,   106,     5,   106,     0,   106,     4,   106,     0,
   106,     3,   106,     0,   106,     6,   106,     0,   106,    25,
     7,    60,   105,     0,   105,     0,     9,     0,    41,   106,
    60,     0,    10,     0,    43,   106,    60,     0,     7,     0,
   106,    39,    59,   106,    37,   106,    60,     0,    16,     0,
   107,    63,     0,    51,   106,    61,     0,    57,    52,    58,
     0,    26,   110,     0,    22,     0,    23,     0,     8,     0,
    46,   113,     0,   113,    56,   114,     0,   114,     0,     7,
    28,     7,     0
};

#endif

#if YY_SSLParser_DEBUG != 0
static const short yyrline[] = { 0,
   215,   219,   224,   227,   229,   232,   235,   240,   242,   245,
   250,   253,   257,   260,   264,   266,   269,   287,   304,   305,
   308,   312,   312,   315,   317,   318,   321,   327,   332,   366,
   388,   402,   413,   417,   424,   432,   439,   452,   458,   464,
   468,   474,   485,   490,   499,   502,   507,   511,   516,   522,
   525,   540,   558,   563,   567,   573,   579,   585,   592,   598,
   604,   610,   615,   622,   626,   649,   653,   656,   662,   666,
   679,   688,   699,   708,   713,   723,   730,   737,   754,   758,
   761,   766,   774,   784,   791,   795,   800,   805,   810,   815,
   820,   830,   836,   841,   848,   853,   858,   862,   866,   870,
   885,   891,   896,   901,   906,   909,   914,   920,   944,   970,
   975,   985,   995,   999,  1003,  1007,  1011,  1015,  1019,  1023,
  1030,  1053,  1058,  1089,  1093,  1099,  1103,  1122,  1126,  1131,
  1134,  1139,  1145,  1150,  1154,  1159,  1188,  1192,  1195,  1199
};

static const char * const yytname[] = {   "$","error","$illegal.","COND_OP",
"BIT_OP","ARITH_OP","LOG_OP","NAME","ASSIGNTYPE","REG_ID","REG_NUM","COND_TNAME",
"DECOR","FARITH_OP","FPUSH","FPOP","TEMP","SHARES","CONV_FUNC","TRUNC_FUNC",
"TRANSCEND","FABS_FUNC","BIG","LITTLE","NAME_CALL","NAME_LOOKUP","ENDIANNESS",
"COVERS","INDEX","NOT","LNOT","FNEG","THEN","LOOKUP_RDC","BOGUS","ASSIGN","TO",
"COLON","S_E","AT","ADDR","REG_IDX","EQUATE","MEM_IDX","TOK_INTEGER","TOK_FLOAT",
"FAST","OPERAND","FETCHEXEC","CAST_OP","FLAGMACRO","SUCCESSOR","NUM","FLOATNUM",
"FCHS","';'","','","'{'","'}'","'['","']'","')'","'\"'","'\\''","'$'","'_'",
"'('","'?'","specorasgn","specification","parts","operandlist","operand","func_parameter",
"reglist","@1","@2","a_reglists","a_reglist","reg_table","flag_fnc","constants",
"table_assign","table_expr","str_expr","str_array","str_term","name_expand",
"bin_oper","opstr_expr","opstr_array","exprstr_expr","exprstr_array","instr",
"@3","instr_name","instr_elem","name_contract","rt_list","rt","flag_list","list_parameter",
"param","list_actualparameter","assign_rt","exp_term","exp","location","cast",
"endianness","esize","assigntype","fastlist","fastentries","fastentry",""
};
#endif

static const short yyr1[] = {     0,
    68,    68,    68,    69,    69,    70,    70,    70,    70,    70,
    70,    70,    70,    70,    71,    71,    72,    72,    73,    73,
    75,    74,    76,    74,    77,    77,    78,    78,    78,    78,
    78,    78,    79,    79,    80,    81,    81,    82,    83,    83,
    83,    84,    84,    85,    85,    85,    86,    86,    87,    87,
    87,    87,    88,    88,    88,    89,    90,    90,    91,    92,
    92,    94,    93,    95,    95,    96,    96,    96,    97,    97,
    97,    97,    97,    97,    98,    98,    99,    99,    99,    99,
    99,   100,   100,   101,   101,   101,   102,   103,   103,   103,
   104,   104,   104,   104,   104,   105,   105,   105,   105,   105,
   105,   105,   105,   105,   105,   105,   105,   105,   105,   105,
   106,   106,   106,   106,   106,   106,   106,   106,   106,   106,
   106,   106,   107,   107,   107,   107,   107,   107,   107,   107,
   107,   108,   109,   110,   110,   111,   112,   113,   113,   114
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
     6,     4,     1,     1,     2,     1,     1,     3,     1,     7,
     3,     7,     3,     3,     1,     1,     3,     3,     3,     3,
     2,     2,     2,     2,     2,     3,     3,     3,     3,     3,
     5,     1,     1,     3,     1,     3,     1,     7,     1,     2,
     3,     3,     2,     1,     1,     1,     2,     3,     1,     3
};

static const short yydefact[] = {     0,
    66,   136,   123,   125,   105,   106,   129,     0,     0,     0,
     0,    86,     0,     0,     0,     0,     0,     0,     0,     0,
    21,    23,     0,     0,     0,     0,    96,    97,     0,     0,
     0,     0,     0,     3,     0,    12,    13,     8,     9,     6,
    62,    64,    67,     1,   122,     2,    99,    10,     0,    11,
     0,     0,   127,   105,   106,    90,     0,     0,     0,     0,
   127,     0,    85,     0,    89,     0,     0,   134,   135,   133,
   113,   114,   115,     0,     0,     0,     0,     0,     0,   137,
   139,    87,    14,    16,    86,    93,    94,    90,     0,    81,
     7,    76,    77,     0,     0,     0,     0,     0,     0,    66,
    86,     0,     0,     5,    65,    86,    68,     0,     0,     0,
     0,     0,     0,   111,     0,     0,   112,   130,    95,    99,
    52,    36,     0,     0,     0,     0,    38,    39,    43,    48,
    40,    41,     0,     0,   103,   107,   104,     0,     0,     0,
   109,    71,    70,   101,   124,   126,     0,     0,    22,    26,
    24,     0,     0,     0,     0,    20,     0,    83,    80,     0,
    75,   110,     0,    74,    69,     0,     0,    98,     0,     4,
     0,   119,   118,   117,   120,   116,     0,     0,     0,     0,
     0,     0,     0,     0,    46,     0,     0,     0,     0,     0,
    51,    42,     0,   108,    84,     0,    88,     0,     0,    34,
     0,     0,   140,   138,    15,    86,    86,     0,    78,     0,
    79,     0,    73,    72,    71,    63,     0,     0,   132,     0,
    99,    92,    37,    53,    54,   127,    55,     0,     0,     0,
    47,     0,    56,     0,    59,    50,    49,     0,     0,    27,
     0,     0,     0,    25,     0,     0,     0,    82,     0,   121,
     0,     0,    58,    61,     0,    44,     0,     0,     0,    35,
     0,    33,     0,    17,    19,    18,     0,     0,    91,    45,
     0,     0,   102,     0,     0,   100,   128,    57,    60,    28,
     0,     0,     0,     0,     0,     0,    32,     0,     0,     0,
     0,    29,    31,     0,     0,     0,    30,     0,     0,     0
};

static const short yydefgoto[] = {   298,
    34,    35,    83,    84,   208,    36,    77,    78,   149,   150,
   201,    37,    38,    39,   127,   185,   186,   129,   130,   228,
   131,   187,   132,   188,    40,   106,    41,    42,    43,    91,
    92,   160,    62,    63,    64,    93,    45,    65,    47,   117,
    48,    70,    49,    50,    80,    81
};

static const short yypact[] = {   146,
   463,-32768,-32768,-32768,    28,    35,-32768,   -11,   246,   246,
   246,   299,     7,   108,   246,   246,   246,   246,   246,   246,
-32768,-32768,    51,    93,    36,   246,-32768,-32768,   246,   106,
   131,    94,   246,   178,    90,-32768,-32768,-32768,-32768,-32768,
   156,    80,-32768,-32768,-32768,   610,   110,-32768,   246,-32768,
    84,   127,-32768,-32768,-32768,   246,   200,   343,   358,   373,
   140,   150,-32768,   159,   610,   158,   168,-32768,-32768,-32768,
    82,    82,   610,   388,   449,   478,    -4,    -4,   191,   174,
-32768,-32768,   189,-32768,     5,-32768,-32768,   246,     4,-32768,
    36,-32768,-32768,   403,    18,   206,   209,    46,   418,   212,
    93,    47,   208,-32768,-32768,    93,-32768,   246,   246,   246,
   246,   246,   262,-32768,   214,   222,-32768,-32768,   552,   -17,
-32768,   273,    33,   272,   274,   275,-32768,    95,-32768,-32768,
-32768,-32768,   228,   223,-32768,-32768,-32768,    93,   227,   246,
-32768,   523,-32768,-32768,-32768,-32768,     2,   276,   232,-32768,
   232,   283,    51,    93,   234,   187,   160,-32768,-32768,   183,
-32768,     3,   246,-32768,-32768,   233,   235,-32768,   236,-32768,
    24,   599,   112,   175,   599,    82,   244,   246,   249,   246,
   246,   240,    95,    63,    95,   -22,    13,   192,   254,   231,
-32768,-32768,   255,-32768,-32768,    36,   610,   269,   285,-32768,
   171,    -4,-32768,-32768,-32768,    93,    93,   317,-32768,   318,
-32768,   568,-32768,-32768,-32768,    36,   246,   595,-32768,   610,
   132,   610,-32768,-32768,-32768,   254,-32768,   264,   297,   172,
-32768,   281,-32768,   282,-32768,-32768,-32768,   246,   529,-32768,
   278,   336,   296,-32768,   201,   177,   246,-32768,   246,-32768,
   246,   246,-32768,-32768,     1,    95,   119,   246,   433,-32768,
   329,-32768,   308,-32768,-32768,   610,   494,   509,   610,-32768,
   307,   328,-32768,   320,   310,-32768,-32768,-32768,-32768,    25,
   345,   365,   366,   332,   341,   351,   352,   330,   386,   347,
   350,-32768,-32768,   367,   353,   354,-32768,   410,   417,-32768
};

static const short yypgoto[] = {-32768,
-32768,   384,-32768,   266,-32768,-32768,-32768,-32768,   355,   230,
-32768,-32768,-32768,-32768,-32768,   -48,-32768,  -121,-32768,   190,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   383,  -134,
   -90,-32768,   -79,   -20,   356,   429,   218,     0,   -47,-32768,
-32768,-32768,   242,-32768,-32768,   287
};


#define	YYLAST		667


static const short yytable[] = {    46,
   161,   120,   128,    85,   147,   156,   192,   189,    58,    59,
    60,    82,   158,    66,    71,    72,    73,    74,    75,    76,
   108,   109,   110,   111,   181,    94,   171,   -93,    95,   198,
   112,     2,    99,   230,   -94,   231,   216,    86,    87,   121,
    52,   282,   113,     2,  -131,   118,   155,    88,   119,    86,
    87,   283,   166,   169,   148,   114,   115,    79,    67,    88,
   199,   239,   270,   192,   159,  -131,   224,   225,   232,   226,
   233,     3,     4,    89,   116,   227,    54,    55,     7,   138,
     8,     9,    10,    11,   163,    89,    56,    57,    90,   183,
   121,    15,    16,    17,   184,   125,   126,   167,    67,    82,
    90,   121,    18,    19,   102,    20,   113,   172,   173,   174,
   175,   176,    96,    26,    27,    28,   110,   195,    98,   114,
   115,    29,   224,   225,   112,   161,   245,   246,    33,    68,
    69,   227,   221,    85,   192,   122,   113,    97,   116,   197,
   123,    30,    31,    32,   104,   124,   125,   126,   161,   114,
   115,   183,     1,     2,     3,     4,   124,   125,   126,     5,
     6,     7,   212,     8,     9,    10,    11,   105,   116,    12,
    13,    14,   118,   252,    15,    16,    17,   218,   121,   220,
   222,   256,   133,   229,   100,    18,    19,   112,    20,    21,
    22,    23,    24,    25,   118,   -87,    26,    27,    28,   113,
   -87,   101,   102,    14,    29,   138,   134,    30,    31,    32,
   139,    33,   114,   115,   140,   140,   220,   142,   152,   141,
   209,    21,    22,    23,    24,    25,   242,   143,   183,   153,
   243,   116,   138,   255,   125,   126,   265,   259,   210,    30,
    31,    32,   138,   211,   154,   207,   266,   234,   267,   235,
   268,   269,    53,    51,     3,     4,   138,   272,   264,    54,
    55,     7,   170,     8,     9,    10,    11,   164,   177,    56,
    57,   165,   178,   179,    15,    16,    17,   182,   189,   193,
   190,   191,   194,   196,   200,    18,    19,   202,    20,   203,
   206,   223,   213,   237,   214,   215,    26,    27,    28,   108,
   109,   110,   111,   217,    29,    61,   219,     3,     4,   112,
   238,    33,    54,    55,     7,   236,     8,     9,    10,    11,
   240,   113,    56,    57,     2,   253,   248,    15,    16,    17,
   108,   109,   110,   111,   114,   115,   241,   261,    18,    19,
   112,    20,   257,   258,   262,   108,   109,   110,   111,    26,
    27,    28,   113,   116,   263,   112,   274,    29,   254,   275,
   108,   109,   110,   111,    33,   114,   115,   113,   278,   281,
   112,   280,   284,   285,   286,   108,   109,   110,   111,   288,
   114,   115,   113,   287,   116,   112,   289,   290,   291,   279,
   108,   109,   110,   111,   292,   114,   115,   113,   293,   116,
   112,   294,   295,   135,   296,   108,   109,   110,   111,   299,
   114,   115,   113,   297,   116,   112,   300,   103,   136,   205,
   108,   109,   110,   111,   107,   114,   115,   113,    44,   116,
   112,   244,   151,   137,   250,   108,   109,   110,   111,   204,
   114,   115,   113,   157,   116,   112,   271,     0,   144,   247,
     0,   108,   109,   110,   111,   114,   115,   113,     0,   116,
     0,   112,  -127,   162,     0,  -127,  -127,  -127,  -127,     0,
   114,   115,     0,   113,   116,  -127,     0,     0,   168,     0,
   108,   109,   110,   111,     0,     0,   114,   115,     0,   116,
   112,     0,     0,   273,     0,     0,   108,   109,   110,   111,
  -127,  -127,   113,     0,    51,   116,   112,     0,   145,     0,
     0,   108,   109,   110,   111,   114,   115,     0,   113,  -127,
     0,   112,  -108,     0,     0,  -108,  -108,  -108,  -108,     0,
     0,   114,   115,   113,   116,  -108,     2,   146,     0,     0,
     0,     0,    86,    87,     0,     0,   114,   115,     0,     0,
   116,     0,    88,   276,   108,   109,   110,   111,     0,     0,
  -108,  -108,     0,     0,   112,   116,     0,     0,   277,     0,
   108,   109,   110,   111,     0,     0,   113,     0,    89,  -108,
   112,     0,     0,   180,     0,     0,   260,     0,     0,   114,
   115,     0,   113,    90,     0,     0,     0,   108,   109,   110,
   111,   108,   109,   110,   249,   114,   115,   112,   116,     0,
     0,   112,   108,   109,   110,   111,     0,     0,     0,   113,
     0,     0,   112,   113,   116,     0,     0,     0,     0,     0,
     0,   251,   114,   115,   113,     0,   114,   115,     0,     0,
     0,     0,     0,     0,     0,     0,     0,   114,   115,     0,
     0,   116,     0,     0,     0,   116,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   116
};

static const short yycheck[] = {     0,
    91,    49,    51,    24,     9,    85,   128,     7,     9,    10,
    11,     7,     9,     7,    15,    16,    17,    18,    19,    20,
     3,     4,     5,     6,    42,    26,   106,     0,    29,    28,
    13,     8,    33,    56,     0,    58,   171,    14,    15,     7,
    52,    17,    25,     8,    42,    63,    42,    24,    49,    14,
    15,    27,     7,     7,    59,    38,    39,     7,    52,    24,
    59,   196,    62,   185,    61,    63,     4,     5,    56,     7,
    58,     9,    10,    50,    57,    13,    14,    15,    16,    56,
    18,    19,    20,    21,    67,    50,    24,    25,    65,    57,
     7,    29,    30,    31,    62,    63,    64,    52,    52,     7,
    65,     7,    40,    41,    25,    43,    25,   108,   109,   110,
   111,   112,     7,    51,    52,    53,     5,   138,    25,    38,
    39,    59,     4,     5,    13,   216,   206,   207,    66,    22,
    23,    13,   180,   154,   256,    52,    25,     7,    57,   140,
    57,    62,    63,    64,    55,    62,    63,    64,   239,    38,
    39,    57,     7,     8,     9,    10,    62,    63,    64,    14,
    15,    16,   163,    18,    19,    20,    21,    12,    57,    24,
    25,    26,    63,    42,    29,    30,    31,   178,     7,   180,
   181,   230,    56,   184,     7,    40,    41,    13,    43,    44,
    45,    46,    47,    48,    63,    56,    51,    52,    53,    25,
    61,    24,    25,    26,    59,    56,     7,    62,    63,    64,
    61,    66,    38,    39,    56,    56,   217,    60,    28,    61,
    61,    44,    45,    46,    47,    48,    56,    60,    57,    56,
    60,    57,    56,    62,    63,    64,    60,   238,    56,    62,
    63,    64,    56,    61,    56,    59,   247,    56,   249,    58,
   251,   252,     7,    42,     9,    10,    56,   258,    58,    14,
    15,    16,    55,    18,    19,    20,    21,    62,     7,    24,
    25,    63,    59,    52,    29,    30,    31,     5,     7,    52,
     7,     7,    60,    57,     9,    40,    41,    56,    43,     7,
    57,    52,    60,    63,    60,    60,    51,    52,    53,     3,
     4,     5,     6,    60,    59,     7,    58,     9,    10,    13,
    56,    66,    14,    15,    16,    62,    18,    19,    20,    21,
    52,    25,    24,    25,     8,    62,     9,    29,    30,    31,
     3,     4,     5,     6,    38,    39,    52,    60,    40,    41,
    13,    43,    62,    62,     9,     3,     4,     5,     6,    51,
    52,    53,    25,    57,    59,    13,    28,    59,    62,    52,
     3,     4,     5,     6,    66,    38,    39,    25,    62,    60,
    13,    52,    28,     9,     9,     3,     4,     5,     6,    39,
    38,    39,    25,    52,    57,    13,    36,    36,    59,    62,
     3,     4,     5,     6,     9,    38,    39,    25,    52,    57,
    13,    52,    36,    61,    52,     3,     4,     5,     6,     0,
    38,    39,    25,    60,    57,    13,     0,    34,    61,   154,
     3,     4,     5,     6,    42,    38,    39,    25,     0,    57,
    13,   202,    78,    61,   217,     3,     4,     5,     6,   153,
    38,    39,    25,    88,    57,    13,   257,    -1,    61,   208,
    -1,     3,     4,     5,     6,    38,    39,    25,    -1,    57,
    -1,    13,     0,    61,    -1,     3,     4,     5,     6,    -1,
    38,    39,    -1,    25,    57,    13,    -1,    -1,    61,    -1,
     3,     4,     5,     6,    -1,    -1,    38,    39,    -1,    57,
    13,    -1,    -1,    61,    -1,    -1,     3,     4,     5,     6,
    38,    39,    25,    -1,    42,    57,    13,    -1,    60,    -1,
    -1,     3,     4,     5,     6,    38,    39,    -1,    25,    57,
    -1,    13,     0,    -1,    -1,     3,     4,     5,     6,    -1,
    -1,    38,    39,    25,    57,    13,     8,    60,    -1,    -1,
    -1,    -1,    14,    15,    -1,    -1,    38,    39,    -1,    -1,
    57,    -1,    24,    60,     3,     4,     5,     6,    -1,    -1,
    38,    39,    -1,    -1,    13,    57,    -1,    -1,    60,    -1,
     3,     4,     5,     6,    -1,    -1,    25,    -1,    50,    57,
    13,    -1,    -1,    32,    -1,    -1,    58,    -1,    -1,    38,
    39,    -1,    25,    65,    -1,    -1,    -1,     3,     4,     5,
     6,     3,     4,     5,    37,    38,    39,    13,    57,    -1,
    -1,    13,     3,     4,     5,     6,    -1,    -1,    -1,    25,
    -1,    -1,    13,    25,    57,    -1,    -1,    -1,    -1,    -1,
    -1,    37,    38,    39,    25,    -1,    38,    39,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    -1,
    -1,    57,    -1,    -1,    -1,    57,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    57
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
#line 1603 "sslparser.cpp"

  switch (yyn) {

case 1:
#line 216 "sslparser.y"
{
			the_asgn = yyvsp[0].regtransfer;
		;
    break;}
case 2:
#line 219 "sslparser.y"
{
			the_asgn = new Assign(
				new Terminal(opNil),
				yyvsp[0].exp);
		;
    break;}
case 7:
#line 235 "sslparser.y"
{
			Dict.fetchExecCycle = yyvsp[0].rtlist;
		;
    break;}
case 14:
#line 260 "sslparser.y"
{ Dict.fixupParams(); ;
    break;}
case 17:
#line 273 "sslparser.y"
{
			// Note: the below copies the list of strings!
			Dict.DetParamMap[yyvsp[-4].str].params = *yyvsp[-1].parmlist;
			Dict.DetParamMap[yyvsp[-4].str].kind = PARAM_VARIANT;
			//delete $4;
		;
    break;}
case 18:
#line 287 "sslparser.y"
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
#line 304 "sslparser.y"
{ yyval.parmlist = yyvsp[-1].parmlist; ;
    break;}
case 20:
#line 305 "sslparser.y"
{ yyval.parmlist = new std::list<std::string>(); ;
    break;}
case 21:
#line 309 "sslparser.y"
{
					bFloat = false;
				;
    break;}
case 23:
#line 312 "sslparser.y"
{
					bFloat = true;
				;
    break;}
case 27:
#line 322 "sslparser.y"
{
				if (Dict.RegMap.find(yyvsp[-2].str) != Dict.RegMap.end())
					yyerror("Name reglist decared twice\n");
				Dict.RegMap[yyvsp[-2].str] = yyvsp[0].num;
			;
    break;}
case 28:
#line 327 "sslparser.y"
{
				if (Dict.RegMap.find(yyvsp[-5].str) != Dict.RegMap.end())
					yyerror("Name reglist declared twice\n");
				Dict.addRegister( yyvsp[-5].str, yyvsp[0].num, yyvsp[-3].num, bFloat);
			;
    break;}
case 29:
#line 332 "sslparser.y"
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
				if ((Dict.RegMap.find(yyvsp[-2].str) == Dict.RegMap.end()) || (Dict.RegMap.find(yyvsp[0].str) == Dict.RegMap.end()))
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
#line 366 "sslparser.y"
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
#line 388 "sslparser.y"
{
				if ((int)yyvsp[-8].strlist->size() != (yyvsp[0].num - yyvsp[-2].num + 1)) {
					std::cerr << "size of register array does not match mapping to r[" << yyvsp[-2].num << ".." << yyvsp[0].num << "]\n";
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
#line 402 "sslparser.y"
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
#line 414 "sslparser.y"
{
				yyvsp[-2].strlist->push_back(yyvsp[0].str);
			;
    break;}
case 34:
#line 417 "sslparser.y"
{
				yyval.strlist = new std::list<std::string>;
				yyval.strlist->push_back(yyvsp[0].str);
			;
    break;}
case 35:
#line 426 "sslparser.y"
{
				// Note: $2 is a list of strings
				Dict.FlagFuncs[yyvsp[-5].str] = new FlagDef(listStrToExp(yyvsp[-4].parmlist), yyvsp[-1].rtlist);
			;
    break;}
case 36:
#line 433 "sslparser.y"
{
				if (ConstTable.find(yyvsp[-2].str) != ConstTable.end())
					yyerror("Constant declared twice");
				ConstTable[std::string(yyvsp[-2].str)] = yyvsp[0].num;
			;
    break;}
case 37:
#line 439 "sslparser.y"
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
#line 453 "sslparser.y"
{
			TableDict[yyvsp[-2].str] = yyvsp[0].tab;
		;
    break;}
case 39:
#line 459 "sslparser.y"
{
			yyval.tab = new Table(*yyvsp[0].namelist);
			//delete $1;
		;
    break;}
case 40:
#line 464 "sslparser.y"
{
			yyval.tab = new OpTable(*yyvsp[0].namelist);
			//delete $1;
		;
    break;}
case 41:
#line 468 "sslparser.y"
{
			yyval.tab = new ExprTable(*yyvsp[0].exprlist);
			//delete $1;
		;
    break;}
case 42:
#line 475 "sslparser.y"
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
#line 485 "sslparser.y"
{
			yyval.namelist = yyvsp[0].namelist;
		;
    break;}
case 44:
#line 491 "sslparser.y"
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
#line 499 "sslparser.y"
{
			yyvsp[-3].namelist->push_back("");
		;
    break;}
case 46:
#line 502 "sslparser.y"
{
			yyval.namelist = yyvsp[0].namelist;
		;
    break;}
case 47:
#line 508 "sslparser.y"
{
			yyval.namelist = yyvsp[-1].namelist;
		;
    break;}
case 48:
#line 511 "sslparser.y"
{
			yyval.namelist = yyvsp[0].namelist;
		;
    break;}
case 49:
#line 517 "sslparser.y"
{
			yyval.namelist = new std::deque<std::string>;
			yyval.namelist->push_back("");
			yyval.namelist->push_back(yyvsp[-1].str);
		;
    break;}
case 50:
#line 522 "sslparser.y"
{
			yyval.namelist = new std::deque<std::string>(1, yyvsp[-1].str);
		;
    break;}
case 51:
#line 525 "sslparser.y"
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
#line 540 "sslparser.y"
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
#line 559 "sslparser.y"
{
			yyval.str = yyvsp[0].str;
		;
    break;}
case 54:
#line 563 "sslparser.y"
{
			yyval.str = yyvsp[0].str;
		;
    break;}
case 55:
#line 567 "sslparser.y"
{
			yyval.str = yyvsp[0].str;
		;
    break;}
case 56:
#line 574 "sslparser.y"
{
			yyval.namelist = yyvsp[-1].namelist;
		;
    break;}
case 57:
#line 581 "sslparser.y"
{
			yyval.namelist = yyvsp[-4].namelist;
			yyval.namelist->push_back(yyvsp[-1].str);
		;
    break;}
case 58:
#line 585 "sslparser.y"
{
			yyval.namelist = new std::deque<std::string>;
			yyval.namelist->push_back(yyvsp[-1].str);
		;
    break;}
case 59:
#line 593 "sslparser.y"
{
			yyval.exprlist = yyvsp[-1].exprlist;
		;
    break;}
case 60:
#line 600 "sslparser.y"
{
			yyval.exprlist = yyvsp[-4].exprlist;
			yyval.exprlist->push_back(yyvsp[-1].exp);
		;
    break;}
case 61:
#line 604 "sslparser.y"
{
			yyval.exprlist = new std::deque<Exp*>;
			yyval.exprlist->push_back(yyvsp[-1].exp);
		;
    break;}
case 62:
#line 612 "sslparser.y"
{
			yyvsp[0].insel->getrefmap(indexrefmap);
		//	   $3			$4
		;
    break;}
case 63:
#line 615 "sslparser.y"
{
			// This function expands the tables and saves the expanded RTLs
			// to the dictionary
			expandTables(yyvsp[-3].insel, yyvsp[-1].parmlist, yyvsp[0].rtlist, Dict);
		;
    break;}
case 64:
#line 623 "sslparser.y"
{
			yyval.insel = yyvsp[0].insel;
		;
    break;}
case 65:
#line 626 "sslparser.y"
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
#line 650 "sslparser.y"
{
			yyval.insel = new InsNameElem(yyvsp[0].str);
		;
    break;}
case 67:
#line 653 "sslparser.y"
{
			yyval.insel = yyvsp[0].insel;
		;
    break;}
case 68:
#line 656 "sslparser.y"
{
			yyval.insel = yyvsp[-1].insel;
			yyval.insel->append(yyvsp[0].insel);
		;
    break;}
case 69:
#line 663 "sslparser.y"
{
			yyval.insel = new InsOptionElem(yyvsp[-1].str);
		;
    break;}
case 70:
#line 666 "sslparser.y"
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
#line 679 "sslparser.y"
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
#line 688 "sslparser.y"
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
#line 699 "sslparser.y"
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
#line 708 "sslparser.y"
{
			yyval.insel = new InsNameElem(yyvsp[-1].str);
		;
    break;}
case 75:
#line 714 "sslparser.y"
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
#line 723 "sslparser.y"
{
			yyval.rtlist = new RTL(STMT_ASSIGN);
			if (yyvsp[0].regtransfer != NULL)
				yyval.rtlist->appendStmt(yyvsp[0].regtransfer);
		;
    break;}
case 77:
#line 731 "sslparser.y"
{
			yyval.regtransfer = yyvsp[0].regtransfer;
		;
    break;}
case 78:
#line 737 "sslparser.y"
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
#line 754 "sslparser.y"
{
			yyval.regtransfer = 0;
		;
    break;}
case 80:
#line 758 "sslparser.y"
{
			yyval.regtransfer = 0;
		;
    break;}
case 81:
#line 761 "sslparser.y"
{
		yyval.regtransfer = NULL;
	;
    break;}
case 82:
#line 767 "sslparser.y"
{
			// Not sure why the below is commented out (MVE)
/*			Location* pFlag = Location::regOf(Dict.RegMap[$3]);
			$1->push_back(pFlag);
			$$ = $1;
*/			yyval.explist = 0;
		;
    break;}
case 83:
#line 774 "sslparser.y"
{
/*			std::list<Exp*>* tmp = new std::list<Exp*>;
			Unary* pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
			tmp->push_back(pFlag);
			$$ = tmp;
*/			yyval.explist = 0;
		;
    break;}
case 84:
#line 785 "sslparser.y"
{
			assert(yyvsp[0].str != 0);
			yyvsp[-2].parmlist->push_back(yyvsp[0].str);
			yyval.parmlist = yyvsp[-2].parmlist;
		;
    break;}
case 85:
#line 791 "sslparser.y"
{
			yyval.parmlist = new std::list<std::string>;
			yyval.parmlist->push_back(yyvsp[0].str);
		;
    break;}
case 86:
#line 795 "sslparser.y"
{
			yyval.parmlist = new std::list<std::string>;
		;
    break;}
case 87:
#line 800 "sslparser.y"
{
			Dict.ParamSet.insert(yyvsp[0].str);		// Not sure if we need this set
			yyval.str = yyvsp[0].str;
		;
    break;}
case 88:
#line 806 "sslparser.y"
{
			yyval.explist->push_back(yyvsp[0].exp);
		;
    break;}
case 89:
#line 810 "sslparser.y"
{
			yyval.explist = new std::list<Exp*>;
			yyval.explist->push_back(yyvsp[0].exp);
		;
    break;}
case 90:
#line 815 "sslparser.y"
{
			yyval.explist = new std::list<Exp*>;
		;
    break;}
case 91:
#line 823 "sslparser.y"
{
			Assign* a = new Assign(yyvsp[-5].typ, yyvsp[-2].exp, yyvsp[0].exp);
			a->setGuard(yyvsp[-4].exp);
			yyval.regtransfer = a;
		;
    break;}
case 92:
#line 830 "sslparser.y"
{
			// update the size of any generated RT's
			yyval.regtransfer = new Assign(yyvsp[-3].typ, yyvsp[-2].exp, yyvsp[0].exp);
		;
    break;}
case 93:
#line 836 "sslparser.y"
{
			yyval.regtransfer = new Assign(
				new Terminal(opNil),
				new Terminal(opFpush));
		;
    break;}
case 94:
#line 841 "sslparser.y"
{
			yyval.regtransfer = new Assign(
				new Terminal(opNil),
				new Terminal(opFpop));
		;
    break;}
case 95:
#line 848 "sslparser.y"
{
			yyval.regtransfer = new Assign(yyvsp[-1].typ, NULL, yyvsp[0].exp);
		;
    break;}
case 96:
#line 854 "sslparser.y"
{
			yyval.exp = new Const(yyvsp[0].num);
		;
    break;}
case 97:
#line 858 "sslparser.y"
{
			yyval.exp = new Const(yyvsp[0].dbl);
		;
    break;}
case 98:
#line 862 "sslparser.y"
{
			yyval.exp = yyvsp[-1].exp;
		;
    break;}
case 99:
#line 866 "sslparser.y"
{
			yyval.exp = yyvsp[0].exp;
		;
    break;}
case 100:
#line 870 "sslparser.y"
{
			yyval.exp = new Ternary(opTern, yyvsp[-5].exp, yyvsp[-3].exp, yyvsp[-1].exp);
		;
    break;}
case 101:
#line 885 "sslparser.y"
{
			yyval.exp = new Unary(opAddrOf, yyvsp[-1].exp);
		;
    break;}
case 102:
#line 891 "sslparser.y"
{
			yyval.exp = new Ternary(strToOper(yyvsp[-6].str), new Const(yyvsp[-5].num), new Const(yyvsp[-3].num), yyvsp[-1].exp);
		;
    break;}
case 103:
#line 896 "sslparser.y"
{
			yyval.exp = new Unary(opFtrunc, yyvsp[-1].exp);
		;
    break;}
case 104:
#line 901 "sslparser.y"
{
			yyval.exp = new Unary(opFabs, yyvsp[-1].exp);
		;
    break;}
case 105:
#line 906 "sslparser.y"
{
			yyval.exp = new Terminal(opFpush);
		;
    break;}
case 106:
#line 909 "sslparser.y"
{
			yyval.exp = new Terminal(opFpop);
		;
    break;}
case 107:
#line 914 "sslparser.y"
{
			yyval.exp = new Unary(strToOper(yyvsp[-2].str), yyvsp[-1].exp);
		;
    break;}
case 108:
#line 920 "sslparser.y"
{
			std::ostringstream o;
			if (indexrefmap.find(yyvsp[-1].str) == indexrefmap.end()) {
				o << "index " << yyvsp[-1].str << " not declared for use.\n";
				yyerror(STR(o));
			} else if (TableDict.find(yyvsp[-2].str) == TableDict.end()) {
				o << "table " << yyvsp[-2].str << " not declared for use.\n";
				yyerror(STR(o));
			} else if (TableDict[yyvsp[-2].str]->getType() != EXPRTABLE) {
				o << "table " << yyvsp[-2].str << " is not an expression table but appears to be used as one.\n";
				yyerror(STR(o));
			} else if ((int)((ExprTable*)TableDict[yyvsp[-2].str])->expressions.size() < indexrefmap[yyvsp[-1].str]->ntokens()) {
				o << "table " << yyvsp[-2].str << " (" << ((ExprTable*)TableDict[yyvsp[-2].str])->expressions.size() <<
					") is too small to use " << yyvsp[-1].str << " (" << indexrefmap[yyvsp[-1].str]->ntokens() << ") as an index.\n";
				yyerror(STR(o));
			}
			// $1 is a map from string to Table*; $2 is a map from string to
			// InsNameElem*
			yyval.exp = new Binary(opExpTable, new Const(yyvsp[-2].str), new Const(yyvsp[-1].str));
		;
    break;}
case 109:
#line 944 "sslparser.y"
{
		std::ostringstream o;
		if (Dict.ParamSet.find(yyvsp[-2].str) != Dict.ParamSet.end() ) {
			if (Dict.DetParamMap.find(yyvsp[-2].str) != Dict.DetParamMap.end()) {
				ParamEntry& param = Dict.DetParamMap[yyvsp[-2].str];
				if (yyvsp[-1].explist->size() != param.funcParams.size() ) {
					o << yyvsp[-2].str << " requires " << param.funcParams.size() << " parameters, but received " << yyvsp[-1].explist->size() << ".\n";
					yyerror(STR(o));
				} else {
					// Everything checks out. *phew* 
					// Note: the below may not be right! (MVE)
					yyval.exp = new Binary(opFlagDef,
							new Const(yyvsp[-2].str),
							listExpToExp(yyvsp[-1].explist));
					//delete $2;			// Delete the list of char*s
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
#line 970 "sslparser.y"
{
			yyval.exp = makeSuccessor(yyvsp[-1].exp);
		;
    break;}
case 111:
#line 976 "sslparser.y"
{
			yyval.exp = new Unary(opSignExt, yyvsp[-1].exp);
		;
    break;}
case 112:
#line 985 "sslparser.y"
{
			// size casts and the opSize operator were generally deprecated,
			// but now opSize is used to transmit the size of operands that
			// could be memOfs from the decoder to type analysis
			if (yyvsp[0].num == STD_SIZE)
				yyval.exp = yyvsp[-1].exp;
			else
				yyval.exp = new Binary(opSize, new Const(yyvsp[0].num), yyvsp[-1].exp);
		;
    break;}
case 113:
#line 995 "sslparser.y"
{
			yyval.exp = new Unary(opNot, yyvsp[0].exp);
		;
    break;}
case 114:
#line 999 "sslparser.y"
{
			yyval.exp = new Unary(opLNot, yyvsp[0].exp);
		;
    break;}
case 115:
#line 1003 "sslparser.y"
{
			yyval.exp = new Unary(opFNeg, yyvsp[0].exp);
		;
    break;}
case 116:
#line 1007 "sslparser.y"
{
			yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
		;
    break;}
case 117:
#line 1011 "sslparser.y"
{
			yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
		;
    break;}
case 118:
#line 1015 "sslparser.y"
{
			yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
		;
    break;}
case 119:
#line 1019 "sslparser.y"
{
			yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
		;
    break;}
case 120:
#line 1023 "sslparser.y"
{
			yyval.exp = new Binary(strToOper(yyvsp[-1].str), yyvsp[-2].exp, yyvsp[0].exp);
		;
    break;}
case 121:
#line 1030 "sslparser.y"
{
			std::ostringstream o;
			if (indexrefmap.find(yyvsp[-2].str) == indexrefmap.end()) {
				o << "index " << yyvsp[-2].str << " not declared for use.\n";
				yyerror(STR(o));
			} else if (TableDict.find(yyvsp[-3].str) == TableDict.end()) {
				o << "table " << yyvsp[-3].str << " not declared for use.\n";
				yyerror(STR(o));
			} else if (TableDict[yyvsp[-3].str]->getType() != OPTABLE) {
				o << "table " << yyvsp[-3].str << " is not an operator table but appears to be used as one.\n";
				yyerror(STR(o));
			} else if ((int)TableDict[yyvsp[-3].str]->records.size() < indexrefmap[yyvsp[-2].str]->ntokens()) {
				o << "table " << yyvsp[-3].str << " is too small to use with " << yyvsp[-2].str << " as an index.\n";
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
#line 1053 "sslparser.y"
{
			yyval.exp = yyvsp[0].exp;
		;
    break;}
case 123:
#line 1064 "sslparser.y"
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
					yyval.exp = new Unary(opMachFtr,	 // Machine specific feature
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
#line 1089 "sslparser.y"
{
			yyval.exp = Location::regOf(yyvsp[-1].exp);
		;
    break;}
case 125:
#line 1093 "sslparser.y"
{
			int regNum;
			sscanf(yyvsp[0].str, "r%d", &regNum);
			yyval.exp = Location::regOf(regNum);
		;
    break;}
case 126:
#line 1099 "sslparser.y"
{
			yyval.exp = Location::memOf(yyvsp[-1].exp);
		;
    break;}
case 127:
#line 1103 "sslparser.y"
{
		// This is a mixture of the param: PARM {} match
		// and the value_op: NAME {} match
			Exp* s;
			std::set<std::string>::iterator it = Dict.ParamSet.find(yyvsp[0].str);
			if (it != Dict.ParamSet.end()) {
				s = new Location(opParam, new Const(yyvsp[0].str), NULL);
			} else if (ConstTable.find(yyvsp[0].str) != ConstTable.end()) {
				s = new Const(ConstTable[yyvsp[0].str]);
			} else {
				std::ostringstream ost;
				ost << "`" << yyvsp[0].str << "' is not a constant, definition or a";
				ost << " parameter of this instruction\n";
				yyerror(STR(ost));
				s = new Const(0);
			}
			yyval.exp = s;
		;
    break;}
case 128:
#line 1122 "sslparser.y"
{
			yyval.exp = new Ternary(opAt, yyvsp[-6].exp, yyvsp[-3].exp, yyvsp[-1].exp);
		;
    break;}
case 129:
#line 1126 "sslparser.y"
{
			yyval.exp = Location::tempOf(new Const(yyvsp[0].str));
		;
    break;}
case 130:
#line 1131 "sslparser.y"
{
			yyval.exp = new Unary(opPostVar, yyvsp[-1].exp);
		;
    break;}
case 131:
#line 1134 "sslparser.y"
{
			yyval.exp = makeSuccessor(yyvsp[-1].exp);
		;
    break;}
case 132:
#line 1140 "sslparser.y"
{
			yyval.num = yyvsp[-1].num;
		;
    break;}
case 133:
#line 1146 "sslparser.y"
{
			Dict.bigEndian = (strcmp(yyvsp[0].str, "BIG") == 0);
		;
    break;}
case 134:
#line 1151 "sslparser.y"
{
			yyval.str = yyvsp[0].str;
		;
    break;}
case 135:
#line 1154 "sslparser.y"
{
			yyval.str = yyvsp[0].str;
		;
    break;}
case 136:
#line 1160 "sslparser.y"
{
			char c = yyvsp[0].str[1];
			if (c == '*') yyval.typ = new SizeType(0); // MVE: should remove these
			else if (isdigit(c)) {
				int size;
				// Skip star (hence +1)
				sscanf(yyvsp[0].str+1, "%d", &size);
				yyval.typ = new SizeType(size);
			} else {
				int size;
				// Skip star and letter
				sscanf(yyvsp[0].str+2, "%d", &size);
				if (size == 0) size = STD_SIZE;
				switch (c) {
					case 'i': yyval.typ = new IntegerType(size, 1); break;
					case 'j': yyval.typ = new IntegerType(size, 0); break;
					case 'u': yyval.typ = new IntegerType(size, -1); break;
					case 'f': yyval.typ = new FloatType(size); break;
					case 'c': yyval.typ = new CharType; break;
					default:
						std::cerr << "Unexpected char " << c << " in assign type\n";
						yyval.typ = new IntegerType;
				}
			}
		;
    break;}
case 140:
#line 1200 "sslparser.y"
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
#line 1203 "sslparser.y"


/*==============================================================================
 * FUNCTION:		SSLParser::SSLParser
 * OVERVIEW:		Constructor for an existing stream.
 * PARAMETERS:		The stream, whether or not to debug
 * RETURNS:			<nothing>
 *============================================================================*/
SSLParser::SSLParser(std::istream &in, bool trace) : sslFile("input"), bFloat(false)
{
	theScanner = new SSLScanner(in, trace);
	if (trace) yydebug = 1; else yydebug=0;
}

/*==============================================================================
 * FUNCTION:		SSLParser::parseExp
 * OVERVIEW:		Parses an assignment from a string.
 * PARAMETERS:		the string
 * RETURNS:			an Assignment or NULL.
 *============================================================================*/
Statement* SSLParser::parseExp(const char *str) {
	std::istringstream ss(str);
	SSLParser p(ss, false);		// Second arg true for debugging
	RTLInstDict d;
	p.yyparse(d);
	return p.the_asgn;
}

/*==============================================================================
 * FUNCTION:		SSLParser::~SSLParser
 * OVERVIEW:		Destructor.
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
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
 * FUNCTION:		SSLParser::yyerror
 * OVERVIEW:		Display an error message and exit.
 * PARAMETERS:		msg - an error message
 * RETURNS:			<nothing>
 *============================================================================*/
void SSLParser::yyerror(char* msg)
{
	std::cerr << sslFile << ":" << theScanner->theLine << ": " << msg << std::endl;
}

/*==============================================================================
 * FUNCTION:		SSLParser::yylex
 * OVERVIEW:		The scanner driver than returns the next token.
 * PARAMETERS:		<none>
 * RETURNS:			the next token
 *============================================================================*/
int SSLParser::yylex()
{
	int token = theScanner->yylex(yylval);
	return token;
}

/*==============================================================================
 * FUNCTION:		SSLParser::strToOper
 * OVERVIEW:		Convert a string operator (e.g. "+f") to an OPER (opFPlus)
 * NOTE:			An attempt is made to make this moderately efficient, else
 *					we might have a skip chain of string comparisons
 * NOTE:			This is a member of SSLParser so we can call yyerror and
 *					have line number etc printed out
 * PARAMETERS:		s: pointer to the operator C string
 * RETURNS:			An OPER, or -1 if not found (enum opWild)
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
			return opNot;		// Bit inversion
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
 * FUNCTION:		listExpToExp
 * OVERVIEW:		Convert a list of actual parameters in the form of a
 *					  STL list of Exps into one expression (using opList)
 * NOTE:			The expressions in the list are not cloned; they are
 *					  simply copied to the new opList
 * PARAMETERS:		le: the list of expressions
 * RETURNS:			The opList Expression
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
	*cur = new Terminal(opNil);			// Terminate the chain
	return e;
}

/*==============================================================================
 * FUNCTION:		listStrToExp
 * OVERVIEW:		Convert a list of formal parameters in the form of a
 *					  STL list of strings into one expression (using opList)
 * PARAMETERS:		ls - the list of strings
 * RETURNS:			The opList expression
 *============================================================================*/
Exp* listStrToExp(std::list<std::string>* ls) {
	Exp* e;
	Exp** cur = &e;
	for (std::list<std::string>::iterator it = ls->begin(); it != ls->end(); it++) {
		*cur = new Binary(opList);
		// *it is a string. Convert it to a parameter
		((Binary*)*cur)->setSubExp1(new Location(opParam, new Const((char*)(*it).c_str()), NULL));
		cur = &(*cur)->refSubExp2();
	}
	*cur = new Terminal(opNil);			 // Terminate the chain
	return e;
}

/*==============================================================================
 * FUNCTION:		SSLParser::expandTables
 * OVERVIEW:		Expand tables in an RTL and save to dictionary
 * NOTE:			This may generate many entries
 * PARAMETERS:		iname: Parser object representing the instruction name
 *					params: Parser object representing the instruction params
 *					o_rtlist: Original rtlist object (before expanding)
 *					Dict: Ref to the dictionary that will contain the results
 *					  of the parse
 * RETURNS:			<nothing>
 *============================================================================*/
static bool now = false;
static Exp* srchExpr = new Binary(opExpTable,
	new Terminal(opWild),
	new Terminal(opWild));
static Exp* srchOp = new Ternary(opOpTable,
	new Terminal(opWild),
	new Terminal(opWild),
	new Terminal(opWild));
void SSLParser::expandTables(InsNameElem* iname, std::list<std::string>* params, RTL* o_rtlist, RTLInstDict& Dict)
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
		for (int j=0; j < n; j++) {
			Statement* s = rtl->elementAt(j);
			std::list<Exp*> le;
			// Expression tables
			assert(s->getKind() == STMT_ASSIGN);
			if (((Assign*)s)->searchAll(srchExpr, le)) {
				std::list<Exp*>::iterator it;
				for (it = le.begin(); it != le.end(); it++) {
					char* tbl = ((Const*)((Binary*)*it)->getSubExp1())->getStr();
					char* idx = ((Const*)((Binary*)*it)->getSubExp2())->getStr();
					Exp* repl =((ExprTable*)(TableDict[tbl]))->expressions[indexrefmap[idx]->getvalue()];
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
				Exp* e2 = b->getSubExp2();	// This should be an opList too
				assert(b->getOper() == opList);
				e2 = ((Binary*)e2)->getSubExp1();
				const char* ops = ((OpTable*)(TableDict[tbl]))->records[indexrefmap[idx]->getvalue()].c_str();
				Exp* repl = new Binary(strToOper(ops), e1->clone(),
				e2->clone());
				s->searchAndReplace(res, repl);
			}
		}
 
		if (Dict.appendToDict(nam, *params, *rtl) != 0) {
			o << "Pattern " << iname->getinspattern() << " conflicts with an earlier declaration of " << nam << ".\n";
			yyerror(STR(o));
		}
	}
	indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
}

/*==============================================================================
 * FUNCTION:		SSLParser::makeSuccessor
 * OVERVIEW:		Make the successor of the given expression, e.g. given
 *					  r[2], return succ( r[2] ) (using opSuccessor)
 *					We can't do the successor operation here, because the
 *					  parameters are not yet instantiated (still of the form
 *					  param(rd)). Actual successor done in Exp::fixSuccessor()
 * NOTE:			The given expression should be of the form	r[const]
 * NOTE:			The parameter expresion is copied (not cloned) in the result
 * PARAMETERS:		The expression to find the successor of
 * RETURNS:			The modified expression
 *============================================================================*/
Exp* SSLParser::makeSuccessor(Exp* e) {
	return new Unary(opSuccessor, e);
}
