#define YY_AnsiCParser_h_included

/*  A Bison++ parser, made from ansi-c.y  */

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
#line 85 "c/ansi-c-parser.cpp"
#define YY_AnsiCParser_DEBUG  1
#define YY_AnsiCParser_PARSE_PARAM  \
	platform plat, callconv cc
#define YY_AnsiCParser_CONSTRUCTOR_PARAM  \
	std::istream &in, bool trace
#define YY_AnsiCParser_CONSTRUCTOR_INIT 
#define YY_AnsiCParser_CONSTRUCTOR_CODE  \
	theScanner = new AnsiCScanner(in, trace); \
	if (trace) yydebug = 1; else yydebug = 0;
#define YY_AnsiCParser_MEMBERS  \
private:		\
	AnsiCScanner *theScanner; \
public: \
	std::list<Signature*> signatures; \
	std::list<Symbol*> symbols; \
	std::list<SymbolRef*> refs;\
	virtual ~AnsiCParser();
#line 37 "c/ansi-c.y"

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

  class AnsiCScanner;

  class TypeIdent {
  public:
	  Type *ty;
	  std::string nam;
  };

  class SymbolMods;

  class Symbol {
  public:
	  ADDRESS addr;
	  std::string nam;
	  Type *ty;
	  Signature *sig;
	  SymbolMods *mods;

	  Symbol(ADDRESS a) : addr(a), nam(""), ty(NULL), sig(NULL), 
						  mods(NULL) { }
  };
	
  class SymbolMods {
  public:
	  bool noDecode;
	  bool incomplete;

	  SymbolMods() : noDecode(false), incomplete(false) { }
  };

  class CustomOptions {
  public:
	  Exp *exp;
	  int sp;

	  CustomOptions() : exp(NULL), sp(0) { }
  };

  class SymbolRef {
  public:
	  ADDRESS addr;
	  std::string nam;

	  SymbolRef(ADDRESS a, const char *nam) : addr(a), nam(nam) { }
  };
  
  class Bound {
  public:
      int kind;
      std::string nam;
      
      Bound(int kind, const char *nam) : kind(kind), nam(nam) { }
  };


#line 129 "c/ansi-c.y"
typedef union {
   int ival;
   char *str;
   Type *type;
   std::list<Parameter*> *param_list;
   std::list<int> *num_list;
   Parameter *param;
   Exp *exp;
   Signature *sig;
   TypeIdent *type_ident;
   Bound *bound;
   std::list<TypeIdent*> *type_ident_list;
   SymbolMods *mods;
   CustomOptions *custom_options;
   callconv cc;
} yy_AnsiCParser_stype;
#define YY_AnsiCParser_STYPE yy_AnsiCParser_stype
#line 146 "c/ansi-c.y"

#include "ansi-c-scanner.h"

#line 73 "/usr/local/lib/bison.cc"
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

/* #line 117 "/usr/local/lib/bison.cc" */
#line 241 "c/ansi-c-parser.cpp"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_AnsiCParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 250 "c/ansi-c-parser.cpp"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 257 "c/ansi-c-parser.cpp"

#line 123 "/usr/local/lib/bison.cc"
/* YY_AnsiCParser_DEBUG */
#endif


#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 267 "c/ansi-c-parser.cpp"

#line 128 "/usr/local/lib/bison.cc"
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

/* #line 236 "/usr/local/lib/bison.cc" */
#line 380 "c/ansi-c-parser.cpp"
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
#define	NODECODE	268
#define	INCOMPLETE	269
#define	SYMBOLREF	270
#define	CDECL	271
#define	PASCAL	272
#define	THISCALL	273
#define	REGOF	274
#define	MEMOF	275
#define	MAXBOUND	276
#define	CUSTOM	277
#define	PREFER	278
#define	WITHSTACK	279
#define	PTR_OP	280
#define	INC_OP	281
#define	DEC_OP	282
#define	LEFT_OP	283
#define	RIGHT_OP	284
#define	LE_OP	285
#define	GE_OP	286
#define	EQ_OP	287
#define	NE_OP	288
#define	AND_OP	289
#define	OR_OP	290
#define	MUL_ASSIGN	291
#define	DIV_ASSIGN	292
#define	MOD_ASSIGN	293
#define	ADD_ASSIGN	294
#define	SUB_ASSIGN	295
#define	LEFT_ASSIGN	296
#define	RIGHT_ASSIGN	297
#define	AND_ASSIGN	298
#define	XOR_ASSIGN	299
#define	OR_ASSIGN	300
#define	TYPE_NAME	301
#define	TYPEDEF	302
#define	EXTERN	303
#define	STATIC	304
#define	AUTO	305
#define	REGISTER	306
#define	CHAR	307
#define	SHORT	308
#define	INT	309
#define	LONG	310
#define	SIGNED	311
#define	UNSIGNED	312
#define	FLOAT	313
#define	DOUBLE	314
#define	CONST	315
#define	VOLATILE	316
#define	VOID	317
#define	STRUCT	318
#define	UNION	319
#define	ENUM	320
#define	ELLIPSIS	321
#define	CASE	322
#define	DEFAULT	323
#define	IF	324
#define	ELSE	325
#define	SWITCH	326
#define	WHILE	327
#define	DO	328
#define	FOR	329
#define	GOTO	330
#define	CONTINUE	331
#define	BREAK	332
#define	RETURN	333


#line 236 "/usr/local/lib/bison.cc"
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

/* #line 280 "/usr/local/lib/bison.cc" */
#line 506 "c/ansi-c-parser.cpp"
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
static const int NODECODE;
static const int INCOMPLETE;
static const int SYMBOLREF;
static const int CDECL;
static const int PASCAL;
static const int THISCALL;
static const int REGOF;
static const int MEMOF;
static const int MAXBOUND;
static const int CUSTOM;
static const int PREFER;
static const int WITHSTACK;
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


#line 280 "/usr/local/lib/bison.cc"
 /* decl const */
#else
enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

/* #line 283 "/usr/local/lib/bison.cc" */
#line 591 "c/ansi-c-parser.cpp"
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
	,NODECODE=268
	,INCOMPLETE=269
	,SYMBOLREF=270
	,CDECL=271
	,PASCAL=272
	,THISCALL=273
	,REGOF=274
	,MEMOF=275
	,MAXBOUND=276
	,CUSTOM=277
	,PREFER=278
	,WITHSTACK=279
	,PTR_OP=280
	,INC_OP=281
	,DEC_OP=282
	,LEFT_OP=283
	,RIGHT_OP=284
	,LE_OP=285
	,GE_OP=286
	,EQ_OP=287
	,NE_OP=288
	,AND_OP=289
	,OR_OP=290
	,MUL_ASSIGN=291
	,DIV_ASSIGN=292
	,MOD_ASSIGN=293
	,ADD_ASSIGN=294
	,SUB_ASSIGN=295
	,LEFT_ASSIGN=296
	,RIGHT_ASSIGN=297
	,AND_ASSIGN=298
	,XOR_ASSIGN=299
	,OR_ASSIGN=300
	,TYPE_NAME=301
	,TYPEDEF=302
	,EXTERN=303
	,STATIC=304
	,AUTO=305
	,REGISTER=306
	,CHAR=307
	,SHORT=308
	,INT=309
	,LONG=310
	,SIGNED=311
	,UNSIGNED=312
	,FLOAT=313
	,DOUBLE=314
	,CONST=315
	,VOLATILE=316
	,VOID=317
	,STRUCT=318
	,UNION=319
	,ENUM=320
	,ELLIPSIS=321
	,CASE=322
	,DEFAULT=323
	,IF=324
	,ELSE=325
	,SWITCH=326
	,WHILE=327
	,DO=328
	,FOR=329
	,GOTO=330
	,CONTINUE=331
	,BREAK=332
	,RETURN=333


#line 283 "/usr/local/lib/bison.cc"
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

/* #line 314 "/usr/local/lib/bison.cc" */
#line 704 "c/ansi-c-parser.cpp"
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
const int YY_AnsiCParser_CLASS::NODECODE=268;
const int YY_AnsiCParser_CLASS::INCOMPLETE=269;
const int YY_AnsiCParser_CLASS::SYMBOLREF=270;
const int YY_AnsiCParser_CLASS::CDECL=271;
const int YY_AnsiCParser_CLASS::PASCAL=272;
const int YY_AnsiCParser_CLASS::THISCALL=273;
const int YY_AnsiCParser_CLASS::REGOF=274;
const int YY_AnsiCParser_CLASS::MEMOF=275;
const int YY_AnsiCParser_CLASS::MAXBOUND=276;
const int YY_AnsiCParser_CLASS::CUSTOM=277;
const int YY_AnsiCParser_CLASS::PREFER=278;
const int YY_AnsiCParser_CLASS::WITHSTACK=279;
const int YY_AnsiCParser_CLASS::PTR_OP=280;
const int YY_AnsiCParser_CLASS::INC_OP=281;
const int YY_AnsiCParser_CLASS::DEC_OP=282;
const int YY_AnsiCParser_CLASS::LEFT_OP=283;
const int YY_AnsiCParser_CLASS::RIGHT_OP=284;
const int YY_AnsiCParser_CLASS::LE_OP=285;
const int YY_AnsiCParser_CLASS::GE_OP=286;
const int YY_AnsiCParser_CLASS::EQ_OP=287;
const int YY_AnsiCParser_CLASS::NE_OP=288;
const int YY_AnsiCParser_CLASS::AND_OP=289;
const int YY_AnsiCParser_CLASS::OR_OP=290;
const int YY_AnsiCParser_CLASS::MUL_ASSIGN=291;
const int YY_AnsiCParser_CLASS::DIV_ASSIGN=292;
const int YY_AnsiCParser_CLASS::MOD_ASSIGN=293;
const int YY_AnsiCParser_CLASS::ADD_ASSIGN=294;
const int YY_AnsiCParser_CLASS::SUB_ASSIGN=295;
const int YY_AnsiCParser_CLASS::LEFT_ASSIGN=296;
const int YY_AnsiCParser_CLASS::RIGHT_ASSIGN=297;
const int YY_AnsiCParser_CLASS::AND_ASSIGN=298;
const int YY_AnsiCParser_CLASS::XOR_ASSIGN=299;
const int YY_AnsiCParser_CLASS::OR_ASSIGN=300;
const int YY_AnsiCParser_CLASS::TYPE_NAME=301;
const int YY_AnsiCParser_CLASS::TYPEDEF=302;
const int YY_AnsiCParser_CLASS::EXTERN=303;
const int YY_AnsiCParser_CLASS::STATIC=304;
const int YY_AnsiCParser_CLASS::AUTO=305;
const int YY_AnsiCParser_CLASS::REGISTER=306;
const int YY_AnsiCParser_CLASS::CHAR=307;
const int YY_AnsiCParser_CLASS::SHORT=308;
const int YY_AnsiCParser_CLASS::INT=309;
const int YY_AnsiCParser_CLASS::LONG=310;
const int YY_AnsiCParser_CLASS::SIGNED=311;
const int YY_AnsiCParser_CLASS::UNSIGNED=312;
const int YY_AnsiCParser_CLASS::FLOAT=313;
const int YY_AnsiCParser_CLASS::DOUBLE=314;
const int YY_AnsiCParser_CLASS::CONST=315;
const int YY_AnsiCParser_CLASS::VOLATILE=316;
const int YY_AnsiCParser_CLASS::VOID=317;
const int YY_AnsiCParser_CLASS::STRUCT=318;
const int YY_AnsiCParser_CLASS::UNION=319;
const int YY_AnsiCParser_CLASS::ENUM=320;
const int YY_AnsiCParser_CLASS::ELLIPSIS=321;
const int YY_AnsiCParser_CLASS::CASE=322;
const int YY_AnsiCParser_CLASS::DEFAULT=323;
const int YY_AnsiCParser_CLASS::IF=324;
const int YY_AnsiCParser_CLASS::ELSE=325;
const int YY_AnsiCParser_CLASS::SWITCH=326;
const int YY_AnsiCParser_CLASS::WHILE=327;
const int YY_AnsiCParser_CLASS::DO=328;
const int YY_AnsiCParser_CLASS::FOR=329;
const int YY_AnsiCParser_CLASS::GOTO=330;
const int YY_AnsiCParser_CLASS::CONTINUE=331;
const int YY_AnsiCParser_CLASS::BREAK=332;
const int YY_AnsiCParser_CLASS::RETURN=333;


#line 314 "/usr/local/lib/bison.cc"
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

/* #line 325 "/usr/local/lib/bison.cc" */
#line 797 "c/ansi-c-parser.cpp"


#define	YYFINAL		154
#define	YYFLAG		-32768
#define	YYNTBASE	91

#define YYTRANSLATE(x) ((unsigned)(x) <= 333 ? yytranslate[x] : 112)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    85,
    84,    86,    82,    79,    83,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    80,    87,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    90,     2,    81,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    88,     2,    89,     2,     2,     2,     2,     2,
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
    66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
    76,    77,    78
};

#if YY_AnsiCParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    12,    14,    16,    18,
    20,    24,    26,    27,    31,    33,    35,    36,    40,    42,
    46,    50,    54,    58,    60,    64,    65,    68,    77,    79,
    83,    94,   101,   108,   111,   119,   124,   130,   137,   142,
   146,   151,   154,   157,   158,   161,   165,   166,   170,   173,
   178,   182,   185,   189,   193,   196,   198,   200,   202,   205,
   208,   211,   214,   216,   218,   221,   225,   227,   229,   231,
   234,   239,   243,   245,   248,   251
};

static const short yyrhs[] = {    92,
     0,    93,    92,     0,     0,   101,     0,   102,     0,   105,
     0,   104,     0,    16,     0,    17,     0,    18,     0,    11,
    79,    95,     0,    11,     0,     0,    97,    79,    96,     0,
    97,     0,    62,     0,     0,    98,    80,   100,     0,   100,
     0,    19,    11,    81,     0,    20,    98,    81,     0,    98,
    82,    98,     0,    98,    83,    98,     0,    11,     0,    21,
     9,    84,     0,     0,   109,    99,     0,   111,    85,    86,
     9,    84,    85,    96,    84,     0,    66,     0,    47,   109,
    87,     0,    47,   111,    85,    86,     9,    84,    85,    96,
    84,    87,     0,    47,   109,    85,    96,    84,    87,     0,
    63,     9,    88,   110,    89,    87,     0,   103,    87,     0,
   103,    23,   109,    85,    95,    84,    87,     0,   109,    85,
    96,    84,     0,    94,   109,    85,    96,    84,     0,    22,
   107,   109,    85,    96,    84,     0,    15,    11,     9,    87,
     0,    11,   109,    87,     0,    11,   106,   103,    87,     0,
    13,   106,     0,    14,   106,     0,     0,    98,    80,     0,
    24,    11,    84,     0,     0,    90,    11,    81,     0,    90,
    81,     0,   108,    90,    11,    81,     0,   108,    90,    81,
     0,   111,     9,     0,   111,     9,   108,     0,   109,    87,
   110,     0,   109,    87,     0,    52,     0,    53,     0,    54,
     0,    57,    52,     0,    57,    53,     0,    57,    54,     0,
    57,    55,     0,    57,     0,    55,     0,    55,    55,     0,
    57,    55,    55,     0,    58,     0,    59,     0,    62,     0,
   111,    86,     0,   111,    90,    11,    81,     0,   111,    90,
    81,     0,     9,     0,    60,   111,     0,    63,     9,     0,
    63,    88,   110,    89,     0
};

#endif

#if YY_AnsiCParser_DEBUG != 0
static const short yyrline[] = { 0,
   168,   172,   174,   178,   180,   182,   184,   188,   191,   193,
   197,   201,   205,   210,   214,   218,   220,   224,   228,   233,
   236,   239,   242,   245,   250,   252,   256,   277,   291,   295,
   297,   311,   325,   337,   341,   353,   369,   384,   403,   409,
   420,   428,   432,   436,   440,   443,   446,   450,   453,   456,
   459,   464,   469,   477,   481,   487,   489,   491,   493,   495,
   497,   499,   501,   503,   505,   507,   509,   511,   513,   515,
   517,   521,   525,   530,   532,   538
};

static const char * const yytname[] = {   "$","error","$illegal.","PREINCLUDE",
"PREDEFINE","PREIF","PREIFDEF","PREENDIF","PRELINE","IDENTIFIER","STRING_LITERAL",
"CONSTANT","SIZEOF","NODECODE","INCOMPLETE","SYMBOLREF","CDECL","PASCAL","THISCALL",
"REGOF","MEMOF","MAXBOUND","CUSTOM","PREFER","WITHSTACK","PTR_OP","INC_OP","DEC_OP",
"LEFT_OP","RIGHT_OP","LE_OP","GE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","MUL_ASSIGN",
"DIV_ASSIGN","MOD_ASSIGN","ADD_ASSIGN","SUB_ASSIGN","LEFT_ASSIGN","RIGHT_ASSIGN",
"AND_ASSIGN","XOR_ASSIGN","OR_ASSIGN","TYPE_NAME","TYPEDEF","EXTERN","STATIC",
"AUTO","REGISTER","CHAR","SHORT","INT","LONG","SIGNED","UNSIGNED","FLOAT","DOUBLE",
"CONST","VOLATILE","VOID","STRUCT","UNION","ENUM","ELLIPSIS","CASE","DEFAULT",
"IF","ELSE","SWITCH","WHILE","DO","FOR","GOTO","CONTINUE","BREAK","RETURN","','",
"':'","']'","'+'","'-'","')'","'('","'*'","';'","'{'","'}'","'['","translation_unit",
"decls","decl","convention","num_list","param_list","param_exp","exp","optional_bound",
"param","type_decl","func_decl","signature","symbol_ref_decl","symbol_decl",
"symbol_mods","custom_options","array_modifier","type_ident","type_ident_list",
"type",""
};
#endif

static const short yyr1[] = {     0,
    91,    92,    92,    93,    93,    93,    93,    94,    94,    94,
    95,    95,    95,    96,    96,    96,    96,    97,    97,    98,
    98,    98,    98,    98,    99,    99,   100,   100,   100,   101,
   101,   101,   101,   102,   102,   103,   103,   103,   104,   105,
   105,   106,   106,   106,   107,   107,   107,   108,   108,   108,
   108,   109,   109,   110,   110,   111,   111,   111,   111,   111,
   111,   111,   111,   111,   111,   111,   111,   111,   111,   111,
   111,   111,   111,   111,   111,   111
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
     3,     1,     0,     3,     1,     1,     0,     3,     1,     3,
     3,     3,     3,     1,     3,     0,     2,     8,     1,     3,
    10,     6,     6,     2,     7,     4,     5,     6,     4,     3,
     4,     2,     2,     0,     2,     3,     0,     3,     2,     4,
     3,     2,     3,     3,     2,     1,     1,     1,     2,     2,
     2,     2,     1,     1,     2,     3,     1,     1,     1,     2,
     4,     3,     1,     2,     2,     4
};

static const short yydefact[] = {     3,
    73,    44,     0,     8,     9,    10,    47,     0,    56,    57,
    58,    64,    63,    67,    68,     0,    69,     0,     1,     3,
     0,     4,     5,     0,     7,     6,     0,     0,    44,    44,
     0,     0,     0,     0,    24,     0,     0,     0,     0,     0,
     0,     0,    65,    59,    60,    61,    62,    74,    75,     0,
     2,     0,     0,    34,    17,    52,    70,     0,    42,    43,
    75,     0,    40,     0,     0,     0,     0,    45,     0,     0,
     0,    17,    30,     0,    66,     0,     0,     0,    17,     0,
    69,    29,     0,    15,     0,    19,    26,     0,     0,    53,
     0,    72,    41,    39,    20,    21,    46,    22,    23,    17,
     0,     0,     0,    55,    76,     0,    13,    36,    17,     0,
     0,    27,     0,     0,    49,     0,    71,     0,     0,     0,
     0,    54,    37,    12,     0,    14,    18,     0,     0,    48,
     0,    51,    38,    32,     0,    33,    13,     0,    25,     0,
    50,    17,    11,    35,     0,     0,    17,     0,     0,    31,
    28,     0,     0,     0
};

static const short yydefgoto[] = {   152,
    19,    20,    21,   125,    83,    84,    85,   112,    86,    22,
    23,    24,    25,    26,    32,    40,    90,    87,    78,    28
};

static const short yypact[] = {   120,
-32768,   132,    16,-32768,-32768,-32768,    12,   159,-32768,-32768,
-32768,   -48,   -39,-32768,-32768,   159,-32768,     2,-32768,   120,
   159,-32768,-32768,   -17,-32768,-32768,   -51,    -4,    39,    39,
     3,   147,   -44,    48,-32768,    52,     9,    55,   -43,   159,
   -29,    -7,-32768,-32768,-32768,-32768,    14,   -60,   -21,   159,
-32768,   -13,   159,-32768,    95,   -22,-32768,     6,-32768,-32768,
-32768,   -12,-32768,   -11,    15,   -33,   -10,-32768,     9,     9,
    -1,    95,-32768,    17,-32768,   159,    20,    10,    95,    23,
    11,-32768,    25,    21,   -38,-32768,    89,    -5,     8,    22,
    30,-32768,-32768,-32768,-32768,-32768,-32768,   -23,   -23,    95,
    29,   111,    36,   159,-32768,    44,   121,-32768,    95,    64,
   124,-32768,    54,    62,-32768,    13,-32768,    60,    72,    67,
    73,-32768,-32768,    83,    82,-32768,-32768,    86,   162,-32768,
   100,-32768,-32768,-32768,    91,-32768,   121,   101,-32768,   109,
-32768,    95,-32768,-32768,   112,   114,    95,   116,   131,-32768,
-32768,   196,   208,-32768
};

static const short yypgoto[] = {-32768,
   200,-32768,-32768,    87,   -54,-32768,    28,-32768,   113,-32768,
-32768,   193,-32768,-32768,    32,-32768,-32768,     1,   -66,    -8
};


#define	YYLAST		225


static const short yytable[] = {    42,
    27,    56,    33,    56,    56,    53,    43,    48,    41,   103,
    49,    61,    44,    45,    46,    47,    91,   101,   114,    35,
    27,    52,    35,   131,   106,    57,    34,    36,    37,    58,
    36,    37,    27,    55,    39,    38,    68,   122,    69,    70,
    71,   110,    63,    69,    70,   118,    88,    96,    69,    70,
    77,    29,    30,    80,   126,    72,    64,    73,    69,    70,
    59,    60,    65,    88,    66,    67,    76,    89,    75,    54,
    88,    79,     1,    97,    93,    94,    77,    74,    57,   113,
    57,    57,    58,   100,    58,    58,    92,   146,   115,    50,
    50,    88,   149,   132,   -16,    95,    98,    99,   105,   109,
    88,    88,   102,     1,    77,    35,   104,   107,   108,   111,
   117,   116,   119,    36,    37,     9,    10,    11,    12,   120,
    13,    14,    15,    16,   121,    17,    31,   123,     1,    82,
     2,   124,   128,    88,     3,     4,     5,     6,    88,   129,
     1,     7,   130,   133,    29,    30,     9,    10,    11,    12,
   135,    13,    14,    15,    16,     1,    81,    31,   134,   136,
    82,   137,     4,     5,     6,   138,     8,     1,     7,   139,
   140,     9,    10,    11,    12,   142,    13,    14,    15,    16,
   141,    17,    18,     9,    10,    11,    12,   144,    13,    14,
    15,    16,   145,    17,    31,   153,   147,   148,     9,    10,
    11,    12,   150,    13,    14,    15,    16,   154,    17,    31,
     9,    10,    11,    12,   151,    13,    14,    15,    16,    51,
    17,    31,   127,   143,    62
};

static const short yycheck[] = {     8,
     0,     9,     2,     9,     9,    23,    55,    16,     8,    76,
     9,     9,    52,    53,    54,    55,    11,    72,    11,    11,
    20,    21,    11,    11,    79,    86,    11,    19,    20,    90,
    19,    20,    32,    85,     7,    24,    80,   104,    82,    83,
    40,    80,    87,    82,    83,   100,    55,    81,    82,    83,
    50,    13,    14,    53,   109,    85,     9,    87,    82,    83,
    29,    30,    11,    72,    37,    11,    88,    90,    55,    87,
    79,    85,     9,    84,    87,    87,    76,    85,    86,    85,
    86,    86,    90,    85,    90,    90,    81,   142,    81,    88,
    88,   100,   147,    81,    84,    81,    69,    70,    89,    79,
   109,   110,    86,     9,   104,    11,    87,    85,    84,    21,
    81,    90,    84,    19,    20,    52,    53,    54,    55,     9,
    57,    58,    59,    60,    89,    62,    63,    84,     9,    66,
    11,    11,     9,   142,    15,    16,    17,    18,   147,    86,
     9,    22,    81,    84,    13,    14,    52,    53,    54,    55,
    84,    57,    58,    59,    60,     9,    62,    63,    87,    87,
    66,    79,    16,    17,    18,    84,    47,     9,    22,    84,
     9,    52,    53,    54,    55,    85,    57,    58,    59,    60,
    81,    62,    63,    52,    53,    54,    55,    87,    57,    58,
    59,    60,    84,    62,    63,     0,    85,    84,    52,    53,
    54,    55,    87,    57,    58,    59,    60,     0,    62,    63,
    52,    53,    54,    55,    84,    57,    58,    59,    60,    20,
    62,    63,   110,   137,    32
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
/* ALLOCA SIMULATION */
/* __HAVE_NO_ALLOCA */
#ifdef __HAVE_NO_ALLOCA
int __alloca_free_ptr(char *ptr,char *ref)
{if(ptr!=ref) free(ptr);
 return 0;}

#define __ALLOCA_alloca(size) malloc(size)
#define __ALLOCA_free(ptr,ref) __alloca_free_ptr((char *)ptr,(char *)ref)

#ifdef YY_AnsiCParser_LSP_NEEDED
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
#define yyclearin       (YY_AnsiCParser_CHAR = YYEMPTY)
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
	  __ALLOCA_return(2);
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) __ALLOCA_alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      __ALLOCA_free(yyss1,yyssa);
      yyvs = (YY_AnsiCParser_STYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
      __ALLOCA_free(yyvs1,yyvsa);
#ifdef YY_AnsiCParser_LSP_NEEDED
      yyls = (YY_AnsiCParser_LTYPE *) __ALLOCA_alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      __ALLOCA_free(yyls1,yylsa);
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


/* #line 811 "/usr/local/lib/bison.cc" */
#line 1531 "c/ansi-c-parser.cpp"

  switch (yyn) {

case 1:
#line 169 "c/ansi-c.y"
{ ;
    break;}
case 2:
#line 173 "c/ansi-c.y"
{ ;
    break;}
case 3:
#line 175 "c/ansi-c.y"
{ ;
    break;}
case 4:
#line 179 "c/ansi-c.y"
{ ;
    break;}
case 5:
#line 181 "c/ansi-c.y"
{ ;
    break;}
case 6:
#line 183 "c/ansi-c.y"
{ ;
    break;}
case 7:
#line 185 "c/ansi-c.y"
{ ;
    break;}
case 8:
#line 190 "c/ansi-c.y"
{ yyval.cc = CONV_C; ;
    break;}
case 9:
#line 192 "c/ansi-c.y"
{ yyval.cc = CONV_PASCAL; ;
    break;}
case 10:
#line 194 "c/ansi-c.y"
{ yyval.cc = CONV_THISCALL; ;
    break;}
case 11:
#line 198 "c/ansi-c.y"
{ yyval.num_list = yyvsp[0].num_list;
		  yyval.num_list->push_front(yyvsp[-2].ival);
		;
    break;}
case 12:
#line 202 "c/ansi-c.y"
{ yyval.num_list = new std::list<int>();
		  yyval.num_list->push_back(yyvsp[0].ival);
		;
    break;}
case 13:
#line 206 "c/ansi-c.y"
{ yyval.num_list = new std::list<int>();
		;
    break;}
case 14:
#line 211 "c/ansi-c.y"
{ yyval.param_list = yyvsp[0].param_list;
			yyval.param_list->push_front(yyvsp[-2].param);
		  ;
    break;}
case 15:
#line 215 "c/ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>(); 
			yyval.param_list->push_back(yyvsp[0].param);
		  ;
    break;}
case 16:
#line 219 "c/ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 17:
#line 221 "c/ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 18:
#line 225 "c/ansi-c.y"
{ yyval.param = yyvsp[0].param;
	  yyval.param->setExp(yyvsp[-2].exp);
	;
    break;}
case 19:
#line 229 "c/ansi-c.y"
{ yyval.param = yyvsp[0].param;
	;
    break;}
case 20:
#line 234 "c/ansi-c.y"
{ yyval.exp = Location::regOf(yyvsp[-1].ival);
	;
    break;}
case 21:
#line 237 "c/ansi-c.y"
{ yyval.exp = Location::memOf(yyvsp[-1].exp);
	;
    break;}
case 22:
#line 240 "c/ansi-c.y"
{ yyval.exp = new Binary(opPlus, yyvsp[-2].exp, yyvsp[0].exp);
	;
    break;}
case 23:
#line 243 "c/ansi-c.y"
{ yyval.exp = new Binary(opMinus, yyvsp[-2].exp, yyvsp[0].exp);
	;
    break;}
case 24:
#line 246 "c/ansi-c.y"
{ yyval.exp = new Const(yyvsp[0].ival);
	;
    break;}
case 25:
#line 251 "c/ansi-c.y"
{ yyval.bound = new Bound(0, yyvsp[-1].str); ;
    break;}
case 26:
#line 253 "c/ansi-c.y"
{ ;
    break;}
case 27:
#line 257 "c/ansi-c.y"
{	if (yyvsp[-1].type_ident->ty->isArray() || 
			(yyvsp[-1].type_ident->ty->isNamed() && 
			 ((NamedType*)yyvsp[-1].type_ident->ty)->resolvesTo() &&
			 ((NamedType*)yyvsp[-1].type_ident->ty)->resolvesTo()->isArray())) {
			/* C has complex semantics for passing arrays.. seeing as 
			 * we're supposedly parsing C, then we should deal with this.
			 * When you pass an array in C it is understood that you are
			 * passing that array "by reference".  As all parameters in
			 * our internal representation are passed "by value", we alter
			 * the type here to be a pointer to an array.
			 */
			yyvsp[-1].type_ident->ty = new PointerType(yyvsp[-1].type_ident->ty);
		}
		yyval.param = new Parameter(yyvsp[-1].type_ident->ty, yyvsp[-1].type_ident->nam.c_str());
		if (yyvsp[0].bound) {
		   switch(yyvsp[0].bound->kind) {
		     case 0: yyval.param->setBoundMax(yyvsp[0].bound->nam.c_str());
		   }
		}
	 ;
    break;}
case 28:
#line 278 "c/ansi-c.y"
{ Signature *sig = Signature::instantiate(plat, cc, NULL);
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
case 29:
#line 292 "c/ansi-c.y"
{ yyval.param = new Parameter(new VoidType, "..."); ;
    break;}
case 30:
#line 296 "c/ansi-c.y"
{ Type::addNamedType(yyvsp[-1].type_ident->nam.c_str(), yyvsp[-1].type_ident->ty); ;
    break;}
case 31:
#line 298 "c/ansi-c.y"
{ Signature *sig = Signature::instantiate(plat, cc, NULL);
		   sig->addReturn(yyvsp[-8].type);
		   for (std::list<Parameter*>::iterator it = yyvsp[-2].param_list->begin();
				it != yyvsp[-2].param_list->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete yyvsp[-2].param_list;
		   Type::addNamedType(yyvsp[-5].str, new PointerType(new FuncType(sig))); 
		 ;
    break;}
case 32:
#line 312 "c/ansi-c.y"
{ Signature *sig = Signature::instantiate(plat, cc, yyvsp[-4].type_ident->nam.c_str());
		   sig->addReturn(yyvsp[-4].type_ident->ty);
		   for (std::list<Parameter*>::iterator it = yyvsp[-2].param_list->begin();
				it != yyvsp[-2].param_list->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete yyvsp[-2].param_list;
		   Type::addNamedType(yyvsp[-4].type_ident->nam.c_str(), new FuncType(sig)); 
		 ;
    break;}
case 33:
#line 326 "c/ansi-c.y"
{ CompoundType *t = new CompoundType(); 
		   for (std::list<TypeIdent*>::iterator it = yyvsp[-2].type_ident_list->begin();
				   it != yyvsp[-2].type_ident_list->end(); it++) {
			  t->addType((*it)->ty, (*it)->nam.c_str());
		   }
		   char tmp[1024];
		   sprintf(tmp, "struct %s", yyvsp[-4].str);
		   Type::addNamedType(tmp, t); 
		 ;
    break;}
case 34:
#line 338 "c/ansi-c.y"
{
		   signatures.push_back(yyvsp[-1].sig);
		 ;
    break;}
case 35:
#line 342 "c/ansi-c.y"
{
		   yyvsp[-6].sig->setPreferedReturn(yyvsp[-4].type_ident->ty);
		   yyvsp[-6].sig->setPreferedName(yyvsp[-4].type_ident->nam.c_str());
		   for (std::list<int>::iterator it = yyvsp[-2].num_list->begin();
				it != yyvsp[-2].num_list->end(); it++)
			   yyvsp[-6].sig->addPreferedParameter(*it - 1);
		   delete yyvsp[-2].num_list;
		   signatures.push_back(yyvsp[-6].sig);
		 ;
    break;}
case 36:
#line 354 "c/ansi-c.y"
{ 
		   /* Use the passed calling convention (cc) */
		   Signature *sig = Signature::instantiate(plat, cc, yyvsp[-3].type_ident->nam.c_str()); 
		   sig->addReturn(yyvsp[-3].type_ident->ty);
		   for (std::list<Parameter*>::iterator it = yyvsp[-1].param_list->begin();
				it != yyvsp[-1].param_list->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete yyvsp[-1].param_list;
		   yyval.sig = sig;
		 ;
    break;}
case 37:
#line 370 "c/ansi-c.y"
{ Signature *sig = Signature::instantiate(plat, yyvsp[-4].cc,
			  yyvsp[-3].type_ident->nam.c_str()); 
		   sig->addReturn(yyvsp[-3].type_ident->ty);
		   for (std::list<Parameter*>::iterator it = yyvsp[-1].param_list->begin();
				it != yyvsp[-1].param_list->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete yyvsp[-1].param_list;
		   yyval.sig = sig;
		 ;
    break;}
case 38:
#line 385 "c/ansi-c.y"
{ CustomSignature *sig = new CustomSignature(yyvsp[-3].type_ident->nam.c_str()); 
		   if (yyvsp[-4].custom_options->exp)
			   sig->addReturn(yyvsp[-3].type_ident->ty, yyvsp[-4].custom_options->exp);
		   if (yyvsp[-4].custom_options->sp)
			   sig->setSP(yyvsp[-4].custom_options->sp);
		   for (std::list<Parameter*>::iterator it = yyvsp[-1].param_list->begin();
				it != yyvsp[-1].param_list->end(); it++)
			   if (std::string((*it)->getName()) != "...") {
				   sig->addParameter(*it);
			   } else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete yyvsp[-1].param_list;
		   yyval.sig = sig;
		 ;
    break;}
case 39:
#line 404 "c/ansi-c.y"
{ SymbolRef *ref = new SymbolRef(yyvsp[-2].ival, yyvsp[-1].str);
			  refs.push_back(ref);
			;
    break;}
case 40:
#line 410 "c/ansi-c.y"
{ Symbol *sym = new Symbol(yyvsp[-2].ival);
			 sym->nam = yyvsp[-1].type_ident->nam;
			 sym->ty = yyvsp[-1].type_ident->ty;
			 symbols.push_back(sym);
		   ;
    break;}
case 41:
#line 421 "c/ansi-c.y"
{ Symbol *sym = new Symbol(yyvsp[-3].ival);
			 sym->sig = yyvsp[-1].sig;
			 sym->mods = yyvsp[-2].mods;
			 symbols.push_back(sym);
		   ;
    break;}
case 42:
#line 429 "c/ansi-c.y"
{ yyval.mods = yyvsp[0].mods;
			 yyval.mods->noDecode = true;
		   ;
    break;}
case 43:
#line 433 "c/ansi-c.y"
{ yyval.mods = yyvsp[0].mods;
			 yyval.mods->incomplete = true;
		   ;
    break;}
case 44:
#line 437 "c/ansi-c.y"
{ yyval.mods = new SymbolMods(); ;
    break;}
case 45:
#line 441 "c/ansi-c.y"
{ yyval.custom_options = new CustomOptions(); yyval.custom_options->exp = yyvsp[-1].exp;
		   ;
    break;}
case 46:
#line 444 "c/ansi-c.y"
{ yyval.custom_options = new CustomOptions(); yyval.custom_options->sp = yyvsp[-1].ival;
		   ;
    break;}
case 47:
#line 447 "c/ansi-c.y"
{ yyval.custom_options = new CustomOptions(); ;
    break;}
case 48:
#line 451 "c/ansi-c.y"
{ yyval.type = new ArrayType(NULL, yyvsp[-1].ival);
		  ;
    break;}
case 49:
#line 454 "c/ansi-c.y"
{ yyval.type = new ArrayType(NULL);
		  ;
    break;}
case 50:
#line 457 "c/ansi-c.y"
{ yyval.type = new ArrayType(yyvsp[-3].type, yyvsp[-1].ival);
		  ;
    break;}
case 51:
#line 460 "c/ansi-c.y"
{ yyval.type = new ArrayType(yyvsp[-2].type);
		  ;
    break;}
case 52:
#line 465 "c/ansi-c.y"
{ yyval.type_ident = new TypeIdent();
			yyval.type_ident->ty = yyvsp[-1].type;
			yyval.type_ident->nam = yyvsp[0].str;
		  ;
    break;}
case 53:
#line 470 "c/ansi-c.y"
{ yyval.type_ident = new TypeIdent();
			((ArrayType*)yyvsp[0].type)->fixBaseType(yyvsp[-2].type);
			yyval.type_ident->ty = yyvsp[0].type;
			yyval.type_ident->nam = yyvsp[-1].str;
		  ;
    break;}
case 54:
#line 478 "c/ansi-c.y"
{ yyval.type_ident_list = yyvsp[0].type_ident_list;
			yyval.type_ident_list->push_front(yyvsp[-2].type_ident);
		  ;
    break;}
case 55:
#line 482 "c/ansi-c.y"
{ yyval.type_ident_list = new std::list<TypeIdent*>(); 
			yyval.type_ident_list->push_back(yyvsp[-1].type_ident);
		  ;
    break;}
case 56:
#line 488 "c/ansi-c.y"
{ yyval.type = new CharType(); ;
    break;}
case 57:
#line 490 "c/ansi-c.y"
{ yyval.type = new IntegerType(16, 1); ;
    break;}
case 58:
#line 492 "c/ansi-c.y"
{ yyval.type = new IntegerType(32, 1); ;
    break;}
case 59:
#line 494 "c/ansi-c.y"
{ yyval.type = new IntegerType(8, 0); ;
    break;}
case 60:
#line 496 "c/ansi-c.y"
{ yyval.type = new IntegerType(16, 0); ;
    break;}
case 61:
#line 498 "c/ansi-c.y"
{ yyval.type = new IntegerType(32, 0); ;
    break;}
case 62:
#line 500 "c/ansi-c.y"
{ yyval.type = new IntegerType(32, 0); ;
    break;}
case 63:
#line 502 "c/ansi-c.y"
{ yyval.type = new IntegerType(32, 0); ;
    break;}
case 64:
#line 504 "c/ansi-c.y"
{ yyval.type = new IntegerType(32, 1); ;
    break;}
case 65:
#line 506 "c/ansi-c.y"
{ yyval.type = new IntegerType(64, 1); ;
    break;}
case 66:
#line 508 "c/ansi-c.y"
{ yyval.type = new IntegerType(64, 0); ;
    break;}
case 67:
#line 510 "c/ansi-c.y"
{ yyval.type = new FloatType(32); ;
    break;}
case 68:
#line 512 "c/ansi-c.y"
{ yyval.type = new FloatType(64); ;
    break;}
case 69:
#line 514 "c/ansi-c.y"
{ yyval.type = new VoidType(); ;
    break;}
case 70:
#line 516 "c/ansi-c.y"
{ yyval.type = new PointerType(yyvsp[-1].type); ;
    break;}
case 71:
#line 518 "c/ansi-c.y"
{ // This isn't C, but it makes defining pointers to arrays easier
	  yyval.type = new ArrayType(yyvsp[-3].type, yyvsp[-1].ival); 
	;
    break;}
case 72:
#line 522 "c/ansi-c.y"
{ // This isn't C, but it makes defining pointers to arrays easier
	  yyval.type = new ArrayType(yyvsp[-2].type); 
	;
    break;}
case 73:
#line 526 "c/ansi-c.y"
{ //$$ = Type::getNamedType($1); 
	  //if ($$ == NULL)
	  yyval.type = new NamedType(yyvsp[0].str);
	;
    break;}
case 74:
#line 531 "c/ansi-c.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 75:
#line 533 "c/ansi-c.y"
{
	  char tmp[1024];
	  sprintf(tmp, "struct %s", yyvsp[0].str);
	  yyval.type = new NamedType(tmp);
	;
    break;}
case 76:
#line 539 "c/ansi-c.y"
{ CompoundType *t = new CompoundType(); 
	  for (std::list<TypeIdent*>::iterator it = yyvsp[-1].type_ident_list->begin();
		   it != yyvsp[-1].type_ident_list->end(); it++) {
		  t->addType((*it)->ty, (*it)->nam.c_str());
	  }
	  yyval.type = t;
	;
    break;}
}

#line 811 "/usr/local/lib/bison.cc"
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

/* #line 1010 "/usr/local/lib/bison.cc" */
#line 2223 "c/ansi-c-parser.cpp"
#line 548 "c/ansi-c.y"

#include <stdio.h>

int AnsiCParser::yylex()
{
	int token = theScanner->yylex(yylval);
	return token;
}

void AnsiCParser::yyerror(char *s)
{
	fflush(stdout);
		printf("\n%s", theScanner->lineBuf);
	printf("\n%*s\n%*s on line %i\n", theScanner->column, "^", theScanner->column, s, theScanner->theLine);
}

AnsiCParser::~AnsiCParser()
{
	// Suppress warnings from gcc about lack of virtual destructor
}


