#define YY_AnsiCParser_h_included

/*  A Bison++ parser, made from ansi-c.y  */

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
#line 89 "ansi-c-parser.cpp"
#define YY_AnsiCParser_DEBUG  1
#define YY_AnsiCParser_PARSE_PARAM  \
    const char *sigstr
#define YY_AnsiCParser_CONSTRUCTOR_PARAM  \
    std::istream &in, bool trace
#define YY_AnsiCParser_CONSTRUCTOR_INIT 
#define YY_AnsiCParser_CONSTRUCTOR_CODE  \
    theScanner = new AnsiCScanner(in, trace); \
    if (trace) yydebug = 1; else yydebug = 0;
#define YY_AnsiCParser_MEMBERS  \
private:        \
    AnsiCScanner *theScanner; \
public: \
    std::list<Signature*> signatures;
#line 35 "ansi-c.y"

  #include <list>
  #include <string>
  #include "exp.h"
  #include "type.h"
  #include "cfg.h"
  #include "proc.h"
  #include "signature.h"
  class AnsiCScanner;


#line 61 "ansi-c.y"
typedef union {
   int ival;
   char *str;
   Type *type;
   std::list<Parameter*> *param_list;
   Parameter *param;
   Exp *exp;
   Signature *signature;
} yy_AnsiCParser_stype;
#define YY_AnsiCParser_STYPE yy_AnsiCParser_stype
#line 71 "ansi-c.y"

#include "ansi-c-scanner.h"

#line 77 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* %{ and %header{ and %union, during decl */
#define YY_AnsiCParser_BISON 1
#ifndef YY_AnsiCParser_COMPATIBILITY
#ifndef YY_USE_CLASS
#define  YY_AnsiCParser_COMPATIBILITY 1
#else
#define  YY_AnsiCParser_COMPATIBILITY 0
#endif
#endif

#if YY_AnsiCParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YYLTYPE
#ifndef YY_AnsiCParser_LTYPE
#define YY_AnsiCParser_LTYPE YYLTYPE
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_AnsiCParser_STYPE 
#define YY_AnsiCParser_STYPE YYSTYPE
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_AnsiCParser_DEBUG
#define  YY_AnsiCParser_DEBUG YYDEBUG
#endif
#endif
#ifdef YY_AnsiCParser_STYPE
#ifndef yystype
#define yystype YY_AnsiCParser_STYPE
#endif
#endif
/* use goto to be compatible */
#ifndef YY_AnsiCParser_USE_GOTO
#define YY_AnsiCParser_USE_GOTO 1
#endif
#endif

/* use no goto to be clean in C++ */
#ifndef YY_AnsiCParser_USE_GOTO
#define YY_AnsiCParser_USE_GOTO 0
#endif

#ifndef YY_AnsiCParser_PURE

/* #line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 178 "ansi-c-parser.cpp"

#line 121 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/*  YY_AnsiCParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 187 "ansi-c-parser.cpp"

#line 125 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 194 "ansi-c-parser.cpp"

#line 127 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
/* YY_AnsiCParser_DEBUG */
#endif


#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 204 "ansi-c-parser.cpp"

#line 132 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* YY_AnsiCParser_LSP_NEEDED*/
#endif



/* DEFAULT LTYPE*/
#ifdef YY_AnsiCParser_LSP_NEEDED
#ifndef YY_AnsiCParser_LTYPE
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

#define YY_AnsiCParser_LTYPE yyltype
#endif
#endif
/* DEFAULT STYPE*/
      /* We used to use `unsigned long' as YY_AnsiCParser_STYPE on MSDOS,
	 but it seems better to be consistent.
	 Most programs should declare their own type anyway.  */

#ifndef YY_AnsiCParser_STYPE
#define YY_AnsiCParser_STYPE int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_AnsiCParser_PARSE
#define YY_AnsiCParser_PARSE yyparse
#endif
#ifndef YY_AnsiCParser_LEX
#define YY_AnsiCParser_LEX yylex
#endif
#ifndef YY_AnsiCParser_LVAL
#define YY_AnsiCParser_LVAL yylval
#endif
#ifndef YY_AnsiCParser_LLOC
#define YY_AnsiCParser_LLOC yylloc
#endif
#ifndef YY_AnsiCParser_CHAR
#define YY_AnsiCParser_CHAR yychar
#endif
#ifndef YY_AnsiCParser_NERRS
#define YY_AnsiCParser_NERRS yynerrs
#endif
#ifndef YY_AnsiCParser_DEBUG_FLAG
#define YY_AnsiCParser_DEBUG_FLAG yydebug
#endif
#ifndef YY_AnsiCParser_ERROR
#define YY_AnsiCParser_ERROR yyerror
#endif
#ifndef YY_AnsiCParser_PARSE_PARAM
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
#define YY_AnsiCParser_PARSE_PARAM
#ifndef YY_AnsiCParser_PARSE_PARAM_DEF
#define YY_AnsiCParser_PARSE_PARAM_DEF
#endif
#endif
#endif
#endif
#ifndef YY_AnsiCParser_PARSE_PARAM
#define YY_AnsiCParser_PARSE_PARAM void
#endif
#endif
#if YY_AnsiCParser_COMPATIBILITY != 0
/* backward compatibility */
#ifdef YY_AnsiCParser_LTYPE
#ifndef YYLTYPE
#define YYLTYPE YY_AnsiCParser_LTYPE
#else
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
#endif
#endif
#ifndef YYSTYPE
#define YYSTYPE YY_AnsiCParser_STYPE
#else
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
#endif
#ifdef YY_AnsiCParser_PURE
#ifndef YYPURE
#define YYPURE YY_AnsiCParser_PURE
#endif
#endif
#ifdef YY_AnsiCParser_DEBUG
#ifndef YYDEBUG
#define YYDEBUG YY_AnsiCParser_DEBUG 
#endif
#endif
#ifndef YY_AnsiCParser_ERROR_VERBOSE
#ifdef YYERROR_VERBOSE
#define YY_AnsiCParser_ERROR_VERBOSE YYERROR_VERBOSE
#endif
#endif
#ifndef YY_AnsiCParser_LSP_NEEDED
#ifdef YYLSP_NEEDED
#define YY_AnsiCParser_LSP_NEEDED YYLSP_NEEDED
#endif
#endif
#endif
#ifndef YY_USE_CLASS
/* TOKEN C */

/* #line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 317 "ansi-c-parser.cpp"
#define	PREINCLUDE	258
#define	PREDEFINE	259
#define	PREIF	260
#define	PREIFDEF	261
#define	PREENDIF	262
#define	PRELINE	263
#define	IDENTIFIER	264
#define	STRING_LITERAL	265
#define	CONSTANT	266
#define	SIZEOF	267
#define	PTR_OP	268
#define	INC_OP	269
#define	DEC_OP	270
#define	LEFT_OP	271
#define	RIGHT_OP	272
#define	LE_OP	273
#define	GE_OP	274
#define	EQ_OP	275
#define	NE_OP	276
#define	AND_OP	277
#define	OR_OP	278
#define	MUL_ASSIGN	279
#define	DIV_ASSIGN	280
#define	MOD_ASSIGN	281
#define	ADD_ASSIGN	282
#define	SUB_ASSIGN	283
#define	LEFT_ASSIGN	284
#define	RIGHT_ASSIGN	285
#define	AND_ASSIGN	286
#define	XOR_ASSIGN	287
#define	OR_ASSIGN	288
#define	TYPE_NAME	289
#define	TYPEDEF	290
#define	EXTERN	291
#define	STATIC	292
#define	AUTO	293
#define	REGISTER	294
#define	CHAR	295
#define	SHORT	296
#define	INT	297
#define	LONG	298
#define	SIGNED	299
#define	UNSIGNED	300
#define	FLOAT	301
#define	DOUBLE	302
#define	CONST	303
#define	VOLATILE	304
#define	VOID	305
#define	STRUCT	306
#define	UNION	307
#define	ENUM	308
#define	ELLIPSIS	309
#define	CASE	310
#define	DEFAULT	311
#define	IF	312
#define	ELSE	313
#define	SWITCH	314
#define	WHILE	315
#define	DO	316
#define	FOR	317
#define	GOTO	318
#define	CONTINUE	319
#define	BREAK	320
#define	RETURN	321


#line 240 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* #defines tokens */
#else
/* CLASS */
#ifndef YY_AnsiCParser_CLASS
#define YY_AnsiCParser_CLASS AnsiCParser
#endif
#ifndef YY_AnsiCParser_INHERIT
#define YY_AnsiCParser_INHERIT
#endif
#ifndef YY_AnsiCParser_MEMBERS
#define YY_AnsiCParser_MEMBERS 
#endif
#ifndef YY_AnsiCParser_LEX_BODY
#define YY_AnsiCParser_LEX_BODY  
#endif
#ifndef YY_AnsiCParser_ERROR_BODY
#define YY_AnsiCParser_ERROR_BODY  
#endif
#ifndef YY_AnsiCParser_CONSTRUCTOR_PARAM
#define YY_AnsiCParser_CONSTRUCTOR_PARAM
#endif
#ifndef YY_AnsiCParser_CONSTRUCTOR_CODE
#define YY_AnsiCParser_CONSTRUCTOR_CODE
#endif
#ifndef YY_AnsiCParser_CONSTRUCTOR_INIT
#define YY_AnsiCParser_CONSTRUCTOR_INIT
#endif
/* choose between enum and const */
#ifndef YY_AnsiCParser_USE_CONST_TOKEN
#define YY_AnsiCParser_USE_CONST_TOKEN 0
/* yes enum is more compatible with flex,  */
/* so by default we use it */
#endif
#if YY_AnsiCParser_USE_CONST_TOKEN != 0
#ifndef YY_AnsiCParser_ENUM_TOKEN
#define YY_AnsiCParser_ENUM_TOKEN yy_AnsiCParser_enum_token
#endif
#endif

class YY_AnsiCParser_CLASS YY_AnsiCParser_INHERIT
{
public:
#if YY_AnsiCParser_USE_CONST_TOKEN != 0
/* static const int token ... */

/* #line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 431 "ansi-c-parser.cpp"
static const int PREINCLUDE;
static const int PREDEFINE;
static const int PREIF;
static const int PREIFDEF;
static const int PREENDIF;
static const int PRELINE;
static const int IDENTIFIER;
static const int STRING_LITERAL;
static const int CONSTANT;
static const int SIZEOF;
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
static const int TYPEDEF;
static const int EXTERN;
static const int STATIC;
static const int AUTO;
static const int REGISTER;
static const int CHAR;
static const int SHORT;
static const int INT;
static const int LONG;
static const int SIGNED;
static const int UNSIGNED;
static const int FLOAT;
static const int DOUBLE;
static const int CONST;
static const int VOLATILE;
static const int VOID;
static const int STRUCT;
static const int UNION;
static const int ENUM;
static const int ELLIPSIS;
static const int CASE;
static const int DEFAULT;
static const int IF;
static const int ELSE;
static const int SWITCH;
static const int WHILE;
static const int DO;
static const int FOR;
static const int GOTO;
static const int CONTINUE;
static const int BREAK;
static const int RETURN;


#line 284 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* decl const */
#else
enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

/* #line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 504 "ansi-c-parser.cpp"
	,PREINCLUDE=258
	,PREDEFINE=259
	,PREIF=260
	,PREIFDEF=261
	,PREENDIF=262
	,PRELINE=263
	,IDENTIFIER=264
	,STRING_LITERAL=265
	,CONSTANT=266
	,SIZEOF=267
	,PTR_OP=268
	,INC_OP=269
	,DEC_OP=270
	,LEFT_OP=271
	,RIGHT_OP=272
	,LE_OP=273
	,GE_OP=274
	,EQ_OP=275
	,NE_OP=276
	,AND_OP=277
	,OR_OP=278
	,MUL_ASSIGN=279
	,DIV_ASSIGN=280
	,MOD_ASSIGN=281
	,ADD_ASSIGN=282
	,SUB_ASSIGN=283
	,LEFT_ASSIGN=284
	,RIGHT_ASSIGN=285
	,AND_ASSIGN=286
	,XOR_ASSIGN=287
	,OR_ASSIGN=288
	,TYPE_NAME=289
	,TYPEDEF=290
	,EXTERN=291
	,STATIC=292
	,AUTO=293
	,REGISTER=294
	,CHAR=295
	,SHORT=296
	,INT=297
	,LONG=298
	,SIGNED=299
	,UNSIGNED=300
	,FLOAT=301
	,DOUBLE=302
	,CONST=303
	,VOLATILE=304
	,VOID=305
	,STRUCT=306
	,UNION=307
	,ENUM=308
	,ELLIPSIS=309
	,CASE=310
	,DEFAULT=311
	,IF=312
	,ELSE=313
	,SWITCH=314
	,WHILE=315
	,DO=316
	,FOR=317
	,GOTO=318
	,CONTINUE=319
	,BREAK=320
	,RETURN=321


#line 287 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_AnsiCParser_PARSE (YY_AnsiCParser_PARSE_PARAM);
 virtual void YY_AnsiCParser_ERROR(char *msg) YY_AnsiCParser_ERROR_BODY;
#ifdef YY_AnsiCParser_PURE
#ifdef YY_AnsiCParser_LSP_NEEDED
 virtual int  YY_AnsiCParser_LEX (YY_AnsiCParser_STYPE *YY_AnsiCParser_LVAL,YY_AnsiCParser_LTYPE *YY_AnsiCParser_LLOC) YY_AnsiCParser_LEX_BODY;
#else
 virtual int  YY_AnsiCParser_LEX (YY_AnsiCParser_STYPE *YY_AnsiCParser_LVAL) YY_AnsiCParser_LEX_BODY;
#endif
#else
 virtual int YY_AnsiCParser_LEX() YY_AnsiCParser_LEX_BODY;
 YY_AnsiCParser_STYPE YY_AnsiCParser_LVAL;
#ifdef YY_AnsiCParser_LSP_NEEDED
 YY_AnsiCParser_LTYPE YY_AnsiCParser_LLOC;
#endif
 int   YY_AnsiCParser_NERRS;
 int    YY_AnsiCParser_CHAR;
#endif
#if YY_AnsiCParser_DEBUG != 0
 int YY_AnsiCParser_DEBUG_FLAG;   /*  nonzero means print parse trace     */
#endif
public:
 YY_AnsiCParser_CLASS(YY_AnsiCParser_CONSTRUCTOR_PARAM);
public:
 YY_AnsiCParser_MEMBERS 
};
/* other declare folow */
#if YY_AnsiCParser_USE_CONST_TOKEN != 0

/* #line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 605 "ansi-c-parser.cpp"
const int YY_AnsiCParser_CLASS::PREINCLUDE=258;
const int YY_AnsiCParser_CLASS::PREDEFINE=259;
const int YY_AnsiCParser_CLASS::PREIF=260;
const int YY_AnsiCParser_CLASS::PREIFDEF=261;
const int YY_AnsiCParser_CLASS::PREENDIF=262;
const int YY_AnsiCParser_CLASS::PRELINE=263;
const int YY_AnsiCParser_CLASS::IDENTIFIER=264;
const int YY_AnsiCParser_CLASS::STRING_LITERAL=265;
const int YY_AnsiCParser_CLASS::CONSTANT=266;
const int YY_AnsiCParser_CLASS::SIZEOF=267;
const int YY_AnsiCParser_CLASS::PTR_OP=268;
const int YY_AnsiCParser_CLASS::INC_OP=269;
const int YY_AnsiCParser_CLASS::DEC_OP=270;
const int YY_AnsiCParser_CLASS::LEFT_OP=271;
const int YY_AnsiCParser_CLASS::RIGHT_OP=272;
const int YY_AnsiCParser_CLASS::LE_OP=273;
const int YY_AnsiCParser_CLASS::GE_OP=274;
const int YY_AnsiCParser_CLASS::EQ_OP=275;
const int YY_AnsiCParser_CLASS::NE_OP=276;
const int YY_AnsiCParser_CLASS::AND_OP=277;
const int YY_AnsiCParser_CLASS::OR_OP=278;
const int YY_AnsiCParser_CLASS::MUL_ASSIGN=279;
const int YY_AnsiCParser_CLASS::DIV_ASSIGN=280;
const int YY_AnsiCParser_CLASS::MOD_ASSIGN=281;
const int YY_AnsiCParser_CLASS::ADD_ASSIGN=282;
const int YY_AnsiCParser_CLASS::SUB_ASSIGN=283;
const int YY_AnsiCParser_CLASS::LEFT_ASSIGN=284;
const int YY_AnsiCParser_CLASS::RIGHT_ASSIGN=285;
const int YY_AnsiCParser_CLASS::AND_ASSIGN=286;
const int YY_AnsiCParser_CLASS::XOR_ASSIGN=287;
const int YY_AnsiCParser_CLASS::OR_ASSIGN=288;
const int YY_AnsiCParser_CLASS::TYPE_NAME=289;
const int YY_AnsiCParser_CLASS::TYPEDEF=290;
const int YY_AnsiCParser_CLASS::EXTERN=291;
const int YY_AnsiCParser_CLASS::STATIC=292;
const int YY_AnsiCParser_CLASS::AUTO=293;
const int YY_AnsiCParser_CLASS::REGISTER=294;
const int YY_AnsiCParser_CLASS::CHAR=295;
const int YY_AnsiCParser_CLASS::SHORT=296;
const int YY_AnsiCParser_CLASS::INT=297;
const int YY_AnsiCParser_CLASS::LONG=298;
const int YY_AnsiCParser_CLASS::SIGNED=299;
const int YY_AnsiCParser_CLASS::UNSIGNED=300;
const int YY_AnsiCParser_CLASS::FLOAT=301;
const int YY_AnsiCParser_CLASS::DOUBLE=302;
const int YY_AnsiCParser_CLASS::CONST=303;
const int YY_AnsiCParser_CLASS::VOLATILE=304;
const int YY_AnsiCParser_CLASS::VOID=305;
const int YY_AnsiCParser_CLASS::STRUCT=306;
const int YY_AnsiCParser_CLASS::UNION=307;
const int YY_AnsiCParser_CLASS::ENUM=308;
const int YY_AnsiCParser_CLASS::ELLIPSIS=309;
const int YY_AnsiCParser_CLASS::CASE=310;
const int YY_AnsiCParser_CLASS::DEFAULT=311;
const int YY_AnsiCParser_CLASS::IF=312;
const int YY_AnsiCParser_CLASS::ELSE=313;
const int YY_AnsiCParser_CLASS::SWITCH=314;
const int YY_AnsiCParser_CLASS::WHILE=315;
const int YY_AnsiCParser_CLASS::DO=316;
const int YY_AnsiCParser_CLASS::FOR=317;
const int YY_AnsiCParser_CLASS::GOTO=318;
const int YY_AnsiCParser_CLASS::CONTINUE=319;
const int YY_AnsiCParser_CLASS::BREAK=320;
const int YY_AnsiCParser_CLASS::RETURN=321;


#line 318 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
 /* const YY_AnsiCParser_CLASS::token */
#endif
/*apres const  */
YY_AnsiCParser_CLASS::YY_AnsiCParser_CLASS(YY_AnsiCParser_CONSTRUCTOR_PARAM) YY_AnsiCParser_CONSTRUCTOR_INIT
{
#if YY_AnsiCParser_DEBUG != 0
YY_AnsiCParser_DEBUG_FLAG=0;
#endif
YY_AnsiCParser_CONSTRUCTOR_CODE;
};
#endif

/* #line 329 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 686 "ansi-c-parser.cpp"


#define	YYFINAL		45
#define	YYFLAG		-32768
#define	YYNTBASE	72

#define YYTRANSLATE(x) ((unsigned)(x) <= 321 ? yytranslate[x] : 80)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    68,
    70,    69,     2,    67,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    71,     2,
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
    46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
    56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
    66
};

#if YY_AnsiCParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    14,    16,    18,    19,
    22,    31,    33,    38,    45,    47,    49,    51,    54,    56,
    58,    60,    62,    65,    67
};

static const short yyrhs[] = {    73,
     0,    74,    73,     0,     0,    77,     0,    78,     0,    76,
    67,    75,     0,    76,     0,    50,     0,     0,    79,     9,
     0,    79,    68,    69,     9,    70,    68,    75,    70,     0,
    54,     0,    35,    79,     9,    71,     0,    79,     9,    68,
    75,    70,    71,     0,    40,     0,    41,     0,    42,     0,
    45,    42,     0,    43,     0,    46,     0,    47,     0,    50,
     0,    79,    69,     0,     9,     0,    48,    79,     0
};

#endif

#if YY_AnsiCParser_DEBUG != 0
static const short yyrline[] = { 0,
    82,    86,    88,    92,    94,    98,   102,   106,   108,   112,
   114,   128,   132,   136,   152,   154,   156,   158,   160,   162,
   164,   166,   168,   170,   175
};

static const char * const yytname[] = {   "$","error","$illegal.","PREINCLUDE",
"PREDEFINE","PREIF","PREIFDEF","PREENDIF","PRELINE","IDENTIFIER","STRING_LITERAL",
"CONSTANT","SIZEOF","PTR_OP","INC_OP","DEC_OP","LEFT_OP","RIGHT_OP","LE_OP",
"GE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","MUL_ASSIGN","DIV_ASSIGN","MOD_ASSIGN",
"ADD_ASSIGN","SUB_ASSIGN","LEFT_ASSIGN","RIGHT_ASSIGN","AND_ASSIGN","XOR_ASSIGN",
"OR_ASSIGN","TYPE_NAME","TYPEDEF","EXTERN","STATIC","AUTO","REGISTER","CHAR",
"SHORT","INT","LONG","SIGNED","UNSIGNED","FLOAT","DOUBLE","CONST","VOLATILE",
"VOID","STRUCT","UNION","ENUM","ELLIPSIS","CASE","DEFAULT","IF","ELSE","SWITCH",
"WHILE","DO","FOR","GOTO","CONTINUE","BREAK","RETURN","','","'('","'*'","')'",
"';'","translation_unit","decls","decl","param_list","param","type_decl","func_decl",
"type",""
};
#endif

static const short yyr1[] = {     0,
    72,    73,    73,    74,    74,    75,    75,    75,    75,    76,
    76,    76,    77,    78,    79,    79,    79,    79,    79,    79,
    79,    79,    79,    79,    79
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     3,     1,     1,     0,     2,
     8,     1,     4,     6,     1,     1,     1,     2,     1,     1,
     1,     1,     2,     1,     2
};

static const short yydefact[] = {     3,
    24,     0,    15,    16,    17,    19,     0,    20,    21,     0,
    22,     1,     3,     4,     5,     0,     0,    18,    25,     2,
     0,    23,     0,     9,    13,    22,    12,     0,     7,     0,
     0,     9,    10,     0,    14,     6,     0,     0,     0,     9,
     0,    11,     0,     0,     0
};

static const short yydefgoto[] = {    43,
    12,    13,    28,    29,    14,    15,    30
};

static const short yypact[] = {    23,
-32768,    34,-32768,-32768,-32768,-32768,   -35,-32768,-32768,    34,
-32768,-32768,    23,-32768,-32768,    -8,    -7,-32768,   -61,-32768,
   -59,-32768,   -60,    -6,-32768,   -58,-32768,   -55,   -57,    -9,
   -53,    -6,-32768,   -50,-32768,-32768,     7,   -49,   -48,    -6,
   -47,-32768,    22,    24,-32768
};

static const short yypgoto[] = {-32768,
    12,-32768,   -27,-32768,-32768,-32768,     4
};


#define	YYLAST		84


static const short yytable[] = {    33,
    21,    23,     1,    16,    36,    17,    18,    22,    24,    32,
    25,    -8,    41,    19,    31,    38,    16,    35,    37,    40,
    39,    44,    42,    45,    20,     0,     0,     0,     0,     0,
     0,     1,     0,     3,     4,     5,     6,     0,     7,     8,
     9,    10,     1,    26,     0,     0,     0,    27,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     2,    34,    22,
    22,    22,     3,     4,     5,     6,     0,     7,     8,     9,
    10,     0,    11,     3,     4,     5,     6,     0,     7,     8,
     9,    10,     0,    11
};

static const short yycheck[] = {     9,
     9,     9,     9,     0,    32,     2,    42,    69,    68,    67,
    71,    70,    40,    10,    70,     9,    13,    71,    69,    68,
    70,     0,    70,     0,    13,    -1,    -1,    -1,    -1,    -1,
    -1,     9,    -1,    40,    41,    42,    43,    -1,    45,    46,
    47,    48,     9,    50,    -1,    -1,    -1,    54,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    35,    68,    69,
    69,    69,    40,    41,    42,    43,    -1,    45,    46,    47,
    48,    -1,    50,    40,    41,    42,    43,    -1,    45,    46,
    47,    48,    -1,    50
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

#if YY_AnsiCParser_USE_GOTO != 0
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
#define yyclearin       (YY_AnsiCParser_CHAR = YYEMPTY)
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
  if (YY_AnsiCParser_CHAR == YYEMPTY && yylen == 1)                               \
    { YY_AnsiCParser_CHAR = (token), YY_AnsiCParser_LVAL = (value);                 \
      yychar1 = YYTRANSLATE (YY_AnsiCParser_CHAR);                                \
      YYPOPSTACK;                                               \
      YYGOTO(yybackup);                                            \
    }                                                           \
  else                                                          \
    { YY_AnsiCParser_ERROR ("syntax error: cannot back up"); YYERROR; }   \
while (0)

#define YYTERROR        1
#define YYERRCODE       256

#ifndef YY_AnsiCParser_PURE
/* UNPURE */
#define YYLEX           YY_AnsiCParser_LEX()
#ifndef YY_USE_CLASS
/* If nonreentrant, and not class , generate the variables here */
int     YY_AnsiCParser_CHAR;                      /*  the lookahead symbol        */
YY_AnsiCParser_STYPE      YY_AnsiCParser_LVAL;              /*  the semantic value of the */
				/*  lookahead symbol    */
int YY_AnsiCParser_NERRS;                 /*  number of parse errors so far */
#ifdef YY_AnsiCParser_LSP_NEEDED
YY_AnsiCParser_LTYPE YY_AnsiCParser_LLOC;   /*  location data for the lookahead     */
			/*  symbol                              */
#endif
#endif


#else
/* PURE */
#ifdef YY_AnsiCParser_LSP_NEEDED
#define YYLEX           YY_AnsiCParser_LEX(&YY_AnsiCParser_LVAL, &YY_AnsiCParser_LLOC)
#else
#define YYLEX           YY_AnsiCParser_LEX(&YY_AnsiCParser_LVAL)
#endif
#endif
#ifndef YY_USE_CLASS
#if YY_AnsiCParser_DEBUG != 0
int YY_AnsiCParser_DEBUG_FLAG;                    /*  nonzero means print parse trace     */
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
 YY_AnsiCParser_CLASS::
#endif
     YY_AnsiCParser_PARSE(YY_AnsiCParser_PARSE_PARAM)
#ifndef __STDC__
#ifndef __cplusplus
#ifndef YY_USE_CLASS
/* parameter definition without protypes */
YY_AnsiCParser_PARSE_PARAM_DEF
#endif
#endif
#endif
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YY_AnsiCParser_STYPE *yyvsp;
  int yyerrstatus;      /*  number of tokens to shift before error messages enabled */
  int yychar1=0;          /*  lookahead token as an internal (translated) token number */

  short yyssa[YYINITDEPTH];     /*  the state stack                     */
  YY_AnsiCParser_STYPE yyvsa[YYINITDEPTH];        /*  the semantic value stack            */

  short *yyss = yyssa;          /*  refer to the stacks thru separate pointers */
  YY_AnsiCParser_STYPE *yyvs = yyvsa;     /*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YY_AnsiCParser_LSP_NEEDED
  YY_AnsiCParser_LTYPE yylsa[YYINITDEPTH];        /*  the location stack                  */
  YY_AnsiCParser_LTYPE *yyls = yylsa;
  YY_AnsiCParser_LTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YY_AnsiCParser_PURE
  int YY_AnsiCParser_CHAR;
  YY_AnsiCParser_STYPE YY_AnsiCParser_LVAL;
  int YY_AnsiCParser_NERRS;
#ifdef YY_AnsiCParser_LSP_NEEDED
  YY_AnsiCParser_LTYPE YY_AnsiCParser_LLOC;
#endif
#endif

  YY_AnsiCParser_STYPE yyval;             /*  the variable used to return         */
				/*  semantic values from the action     */
				/*  routines                            */

  int yylen;
/* start loop, in which YYGOTO may be used. */
YYBEGINGOTO

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
    fprintf(stderr, "Starting parse\n");
#endif
  yystate = 0;
  yyerrstatus = 0;
  YY_AnsiCParser_NERRS = 0;
  YY_AnsiCParser_CHAR = YYEMPTY;          /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YY_AnsiCParser_LSP_NEEDED
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
      YY_AnsiCParser_STYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YY_AnsiCParser_LSP_NEEDED
      YY_AnsiCParser_LTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YY_AnsiCParser_LSP_NEEDED
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
#ifdef YY_AnsiCParser_LSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  YY_AnsiCParser_ERROR("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YY_AnsiCParser_STYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YY_AnsiCParser_LSP_NEEDED
      yyls = (YY_AnsiCParser_LTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YY_AnsiCParser_LSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YY_AnsiCParser_DEBUG != 0
      if (YY_AnsiCParser_DEBUG_FLAG)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
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

  if (YY_AnsiCParser_CHAR == YYEMPTY)
    {
#if YY_AnsiCParser_DEBUG != 0
      if (YY_AnsiCParser_DEBUG_FLAG)
	fprintf(stderr, "Reading a token: ");
#endif
      YY_AnsiCParser_CHAR = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (YY_AnsiCParser_CHAR <= 0)           /* This means end of input. */
    {
      yychar1 = 0;
      YY_AnsiCParser_CHAR = YYEOF;                /* Don't call YYLEX any more */

#if YY_AnsiCParser_DEBUG != 0
      if (YY_AnsiCParser_DEBUG_FLAG)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(YY_AnsiCParser_CHAR);

#if YY_AnsiCParser_DEBUG != 0
      if (YY_AnsiCParser_DEBUG_FLAG)
	{
	  fprintf (stderr, "Next token is %d (%s", YY_AnsiCParser_CHAR, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, YY_AnsiCParser_CHAR, YY_AnsiCParser_LVAL);
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

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting token %d (%s), ", YY_AnsiCParser_CHAR, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (YY_AnsiCParser_CHAR != YYEOF)
    YY_AnsiCParser_CHAR = YYEMPTY;

  *++yyvsp = YY_AnsiCParser_LVAL;
#ifdef YY_AnsiCParser_LSP_NEEDED
  *++yylsp = YY_AnsiCParser_LLOC;
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

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
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
#line 1292 "ansi-c-parser.cpp"

  switch (yyn) {

case 1:
#line 83 "ansi-c.y"
{ ;
    break;}
case 2:
#line 87 "ansi-c.y"
{ ;
    break;}
case 3:
#line 89 "ansi-c.y"
{ ;
    break;}
case 4:
#line 93 "ansi-c.y"
{ ;
    break;}
case 5:
#line 95 "ansi-c.y"
{ ;
    break;}
case 6:
#line 99 "ansi-c.y"
{ yyval.param_list = yyvsp[0].param_list;
            yyval.param_list->push_front(yyvsp[-2].param);
          ;
    break;}
case 7:
#line 103 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>(); 
            yyval.param_list->push_back(yyvsp[0].param);
          ;
    break;}
case 8:
#line 107 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 9:
#line 109 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 10:
#line 113 "ansi-c.y"
{ yyval.param = new Parameter(yyvsp[-1].type, yyvsp[0].str); ;
    break;}
case 11:
#line 115 "ansi-c.y"
{ Signature *sig = Signature::instantiate(sigstr, NULL);
       sig->addReturn(yyvsp[-7].type);
       for (std::list<Parameter*>::iterator it = yyvsp[-1].param_list->begin();
            it != yyvsp[-1].param_list->end(); it++)
           if (std::string((*it)->getName()) != "...")
               sig->addParameter(*it);
           else {
               sig->addEllipsis();
               delete *it;
           }
       delete yyvsp[-1].param_list;
       yyval.param = new Parameter(new PointerType(new FuncType(sig)), yyvsp[-4].str); 
     ;
    break;}
case 12:
#line 129 "ansi-c.y"
{ yyval.param = new Parameter(new VoidType, "..."); ;
    break;}
case 13:
#line 133 "ansi-c.y"
{ Type::addNamedType(yyvsp[-1].str, yyvsp[-2].type); ;
    break;}
case 14:
#line 137 "ansi-c.y"
{ Signature *sig = Signature::instantiate(sigstr, yyvsp[-4].str); 
           sig->addReturn(yyvsp[-5].type);
           for (std::list<Parameter*>::iterator it = yyvsp[-2].param_list->begin();
                it != yyvsp[-2].param_list->end(); it++)
               if (std::string((*it)->getName()) != "...")
                   sig->addParameter(*it);
               else {
                   sig->addEllipsis();
                   delete *it;
               }
           delete yyvsp[-2].param_list;
           signatures.push_back(sig);
         ;
    break;}
case 15:
#line 153 "ansi-c.y"
{ yyval.type = new CharType(); ;
    break;}
case 16:
#line 155 "ansi-c.y"
{ yyval.type = new IntegerType(16); ;
    break;}
case 17:
#line 157 "ansi-c.y"
{ yyval.type = new IntegerType(); ;
    break;}
case 18:
#line 159 "ansi-c.y"
{ yyval.type = new IntegerType(32, false); ;
    break;}
case 19:
#line 161 "ansi-c.y"
{ yyval.type = new IntegerType(); ;
    break;}
case 20:
#line 163 "ansi-c.y"
{ yyval.type = new FloatType(32); ;
    break;}
case 21:
#line 165 "ansi-c.y"
{ yyval.type = new FloatType(64); ;
    break;}
case 22:
#line 167 "ansi-c.y"
{ yyval.type = new VoidType(); ;
    break;}
case 23:
#line 169 "ansi-c.y"
{ yyval.type = new PointerType(yyvsp[-1].type); ;
    break;}
case 24:
#line 171 "ansi-c.y"
{ yyval.type = Type::getNamedType(yyvsp[0].str); 
      if (yyval.type == NULL)
          yyval.type = new NamedType(yyvsp[0].str);
    ;
    break;}
case 25:
#line 176 "ansi-c.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
}

#line 783 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc"
   /* the action file gets copied in in place of this dollarsign  */
  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YY_AnsiCParser_LSP_NEEDED
  yylsp -= yylen;
#endif

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YY_AnsiCParser_LSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = YY_AnsiCParser_LLOC.first_line;
      yylsp->first_column = YY_AnsiCParser_LLOC.first_column;
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
      ++YY_AnsiCParser_NERRS;

#ifdef YY_AnsiCParser_ERROR_VERBOSE
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
	      YY_AnsiCParser_ERROR(msg);
	      free(msg);
	    }
	  else
	    YY_AnsiCParser_ERROR ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YY_AnsiCParser_ERROR_VERBOSE */
	YY_AnsiCParser_ERROR("parse error");
    }

  YYGOTO(yyerrlab1);
YYLABEL(yyerrlab1)   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (YY_AnsiCParser_CHAR == YYEOF)
	YYABORT;

#if YY_AnsiCParser_DEBUG != 0
      if (YY_AnsiCParser_DEBUG_FLAG)
	fprintf(stderr, "Discarding token %d (%s).\n", YY_AnsiCParser_CHAR, yytname[yychar1]);
#endif

      YY_AnsiCParser_CHAR = YYEMPTY;
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
#ifdef YY_AnsiCParser_LSP_NEEDED
  yylsp--;
#endif

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
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

#if YY_AnsiCParser_DEBUG != 0
  if (YY_AnsiCParser_DEBUG_FLAG)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = YY_AnsiCParser_LVAL;
#ifdef YY_AnsiCParser_LSP_NEEDED
  *++yylsp = YY_AnsiCParser_LLOC;
#endif

  yystate = yyn;
  YYGOTO(yynewstate);
/* end loop, in which YYGOTO may be used. */
  YYENDGOTO
}

/* END */

/* #line 982 "/home/02/binary/u1.luna.tools/bison++/lib/bison.cc" */
#line 1631 "ansi-c-parser.cpp"
#line 181 "ansi-c.y"

#include <stdio.h>

int AnsiCParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}

void AnsiCParser::yyerror(char *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", theScanner->column, "^", theScanner->column, s);
}



