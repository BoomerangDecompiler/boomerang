#define YY_AnsiCParser_h_included

/*  A Bison++ parser, made from ansi-c.y  */

 /* with Bison++ version bison++ Version 1.21-7, adapted from GNU bison by coetmeur@icdc.fr
  */


#line 1 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 77 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 89 "ansi-c-parser.cpp"
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
private:        \
    AnsiCScanner *theScanner; \
public: \
    std::list<Signature*> signatures; \
    std::list<Symbol*> symbols; \
    std::list<SymbolRef*> refs;\
    virtual ~AnsiCParser();
#line 37 "ansi-c.y"

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


#line 120 "ansi-c.y"
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
   std::list<TypeIdent*> *type_ident_list;
   SymbolMods *mods;
   CustomOptions *custom_options;
   callconv cc;
} yy_AnsiCParser_stype;
#define YY_AnsiCParser_STYPE yy_AnsiCParser_stype
#line 136 "ansi-c.y"

#include "ansi-c-scanner.h"

#line 77 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 121 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 236 "ansi-c-parser.cpp"

#line 121 "/home/38/binary/u1.luna.tools/lib/bison.cc"
/*  YY_AnsiCParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 125 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 245 "ansi-c-parser.cpp"

#line 125 "/home/38/binary/u1.luna.tools/lib/bison.cc"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 127 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 252 "ansi-c-parser.cpp"

#line 127 "/home/38/binary/u1.luna.tools/lib/bison.cc"
/* YY_AnsiCParser_DEBUG */
#endif


#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 132 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 262 "ansi-c-parser.cpp"

#line 132 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 240 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 375 "ansi-c-parser.cpp"
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
#define	REGOF	273
#define	MEMOF	274
#define	CUSTOM	275
#define	PREFER	276
#define	WITHSTACK	277
#define	PTR_OP	278
#define	INC_OP	279
#define	DEC_OP	280
#define	LEFT_OP	281
#define	RIGHT_OP	282
#define	LE_OP	283
#define	GE_OP	284
#define	EQ_OP	285
#define	NE_OP	286
#define	AND_OP	287
#define	OR_OP	288
#define	MUL_ASSIGN	289
#define	DIV_ASSIGN	290
#define	MOD_ASSIGN	291
#define	ADD_ASSIGN	292
#define	SUB_ASSIGN	293
#define	LEFT_ASSIGN	294
#define	RIGHT_ASSIGN	295
#define	AND_ASSIGN	296
#define	XOR_ASSIGN	297
#define	OR_ASSIGN	298
#define	TYPE_NAME	299
#define	TYPEDEF	300
#define	EXTERN	301
#define	STATIC	302
#define	AUTO	303
#define	REGISTER	304
#define	CHAR	305
#define	SHORT	306
#define	INT	307
#define	LONG	308
#define	SIGNED	309
#define	UNSIGNED	310
#define	FLOAT	311
#define	DOUBLE	312
#define	CONST	313
#define	VOLATILE	314
#define	VOID	315
#define	STRUCT	316
#define	UNION	317
#define	ENUM	318
#define	ELLIPSIS	319
#define	CASE	320
#define	DEFAULT	321
#define	IF	322
#define	ELSE	323
#define	SWITCH	324
#define	WHILE	325
#define	DO	326
#define	FOR	327
#define	GOTO	328
#define	CONTINUE	329
#define	BREAK	330
#define	RETURN	331


#line 240 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 284 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 499 "ansi-c-parser.cpp"
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
static const int REGOF;
static const int MEMOF;
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


#line 284 "/home/38/binary/u1.luna.tools/lib/bison.cc"
 /* decl const */
#else
enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

/* #line 287 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 582 "ansi-c-parser.cpp"
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
	,REGOF=273
	,MEMOF=274
	,CUSTOM=275
	,PREFER=276
	,WITHSTACK=277
	,PTR_OP=278
	,INC_OP=279
	,DEC_OP=280
	,LEFT_OP=281
	,RIGHT_OP=282
	,LE_OP=283
	,GE_OP=284
	,EQ_OP=285
	,NE_OP=286
	,AND_OP=287
	,OR_OP=288
	,MUL_ASSIGN=289
	,DIV_ASSIGN=290
	,MOD_ASSIGN=291
	,ADD_ASSIGN=292
	,SUB_ASSIGN=293
	,LEFT_ASSIGN=294
	,RIGHT_ASSIGN=295
	,AND_ASSIGN=296
	,XOR_ASSIGN=297
	,OR_ASSIGN=298
	,TYPE_NAME=299
	,TYPEDEF=300
	,EXTERN=301
	,STATIC=302
	,AUTO=303
	,REGISTER=304
	,CHAR=305
	,SHORT=306
	,INT=307
	,LONG=308
	,SIGNED=309
	,UNSIGNED=310
	,FLOAT=311
	,DOUBLE=312
	,CONST=313
	,VOLATILE=314
	,VOID=315
	,STRUCT=316
	,UNION=317
	,ENUM=318
	,ELLIPSIS=319
	,CASE=320
	,DEFAULT=321
	,IF=322
	,ELSE=323
	,SWITCH=324
	,WHILE=325
	,DO=326
	,FOR=327
	,GOTO=328
	,CONTINUE=329
	,BREAK=330
	,RETURN=331


#line 287 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 318 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 693 "ansi-c-parser.cpp"
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
const int YY_AnsiCParser_CLASS::REGOF=273;
const int YY_AnsiCParser_CLASS::MEMOF=274;
const int YY_AnsiCParser_CLASS::CUSTOM=275;
const int YY_AnsiCParser_CLASS::PREFER=276;
const int YY_AnsiCParser_CLASS::WITHSTACK=277;
const int YY_AnsiCParser_CLASS::PTR_OP=278;
const int YY_AnsiCParser_CLASS::INC_OP=279;
const int YY_AnsiCParser_CLASS::DEC_OP=280;
const int YY_AnsiCParser_CLASS::LEFT_OP=281;
const int YY_AnsiCParser_CLASS::RIGHT_OP=282;
const int YY_AnsiCParser_CLASS::LE_OP=283;
const int YY_AnsiCParser_CLASS::GE_OP=284;
const int YY_AnsiCParser_CLASS::EQ_OP=285;
const int YY_AnsiCParser_CLASS::NE_OP=286;
const int YY_AnsiCParser_CLASS::AND_OP=287;
const int YY_AnsiCParser_CLASS::OR_OP=288;
const int YY_AnsiCParser_CLASS::MUL_ASSIGN=289;
const int YY_AnsiCParser_CLASS::DIV_ASSIGN=290;
const int YY_AnsiCParser_CLASS::MOD_ASSIGN=291;
const int YY_AnsiCParser_CLASS::ADD_ASSIGN=292;
const int YY_AnsiCParser_CLASS::SUB_ASSIGN=293;
const int YY_AnsiCParser_CLASS::LEFT_ASSIGN=294;
const int YY_AnsiCParser_CLASS::RIGHT_ASSIGN=295;
const int YY_AnsiCParser_CLASS::AND_ASSIGN=296;
const int YY_AnsiCParser_CLASS::XOR_ASSIGN=297;
const int YY_AnsiCParser_CLASS::OR_ASSIGN=298;
const int YY_AnsiCParser_CLASS::TYPE_NAME=299;
const int YY_AnsiCParser_CLASS::TYPEDEF=300;
const int YY_AnsiCParser_CLASS::EXTERN=301;
const int YY_AnsiCParser_CLASS::STATIC=302;
const int YY_AnsiCParser_CLASS::AUTO=303;
const int YY_AnsiCParser_CLASS::REGISTER=304;
const int YY_AnsiCParser_CLASS::CHAR=305;
const int YY_AnsiCParser_CLASS::SHORT=306;
const int YY_AnsiCParser_CLASS::INT=307;
const int YY_AnsiCParser_CLASS::LONG=308;
const int YY_AnsiCParser_CLASS::SIGNED=309;
const int YY_AnsiCParser_CLASS::UNSIGNED=310;
const int YY_AnsiCParser_CLASS::FLOAT=311;
const int YY_AnsiCParser_CLASS::DOUBLE=312;
const int YY_AnsiCParser_CLASS::CONST=313;
const int YY_AnsiCParser_CLASS::VOLATILE=314;
const int YY_AnsiCParser_CLASS::VOID=315;
const int YY_AnsiCParser_CLASS::STRUCT=316;
const int YY_AnsiCParser_CLASS::UNION=317;
const int YY_AnsiCParser_CLASS::ENUM=318;
const int YY_AnsiCParser_CLASS::ELLIPSIS=319;
const int YY_AnsiCParser_CLASS::CASE=320;
const int YY_AnsiCParser_CLASS::DEFAULT=321;
const int YY_AnsiCParser_CLASS::IF=322;
const int YY_AnsiCParser_CLASS::ELSE=323;
const int YY_AnsiCParser_CLASS::SWITCH=324;
const int YY_AnsiCParser_CLASS::WHILE=325;
const int YY_AnsiCParser_CLASS::DO=326;
const int YY_AnsiCParser_CLASS::FOR=327;
const int YY_AnsiCParser_CLASS::GOTO=328;
const int YY_AnsiCParser_CLASS::CONTINUE=329;
const int YY_AnsiCParser_CLASS::BREAK=330;
const int YY_AnsiCParser_CLASS::RETURN=331;


#line 318 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 329 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 784 "ansi-c-parser.cpp"


#define	YYFINAL		149
#define	YYFLAG		-32768
#define	YYNTBASE	89

#define YYTRANSLATE(x) ((unsigned)(x) <= 331 ? yytranslate[x] : 109)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    82,
    84,    83,    80,    77,    81,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    78,    85,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    88,     2,    79,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    86,     2,    87,     2,     2,     2,     2,     2,
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
    76
};

#if YY_AnsiCParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    12,    14,    16,    18,
    22,    24,    25,    29,    31,    33,    34,    38,    40,    44,
    48,    52,    56,    58,    60,    69,    71,    75,    86,    93,
   100,   103,   111,   116,   122,   129,   134,   138,   143,   146,
   149,   150,   153,   157,   158,   162,   165,   170,   174,   177,
   181,   185,   188,   190,   192,   194,   197,   200,   203,   205,
   208,   212,   214,   216,   218,   221,   226,   230,   232,   235,
   238
};

static const short yyrhs[] = {    90,
     0,    91,    90,     0,     0,    98,     0,    99,     0,   102,
     0,   101,     0,    16,     0,    17,     0,    11,    77,    93,
     0,    11,     0,     0,    95,    77,    94,     0,    95,     0,
    60,     0,     0,    96,    78,    97,     0,    97,     0,    18,
    11,    79,     0,    19,    96,    79,     0,    96,    80,    96,
     0,    96,    81,    96,     0,    11,     0,   106,     0,   108,
    82,    83,     9,    84,    82,    94,    84,     0,    64,     0,
    45,   106,    85,     0,    45,   108,    82,    83,     9,    84,
    82,    94,    84,    85,     0,    45,   106,    82,    94,    84,
    85,     0,    61,     9,    86,   107,    87,    85,     0,   100,
    85,     0,   100,    21,   106,    82,    93,    84,    85,     0,
   106,    82,    94,    84,     0,    92,   106,    82,    94,    84,
     0,    20,   104,   106,    82,    94,    84,     0,    15,    11,
     9,    85,     0,    11,   106,    85,     0,    11,   103,   100,
    85,     0,    13,   103,     0,    14,   103,     0,     0,    96,
    78,     0,    22,    11,    84,     0,     0,    88,    11,    79,
     0,    88,    79,     0,   105,    88,    11,    79,     0,   105,
    88,    79,     0,   108,     9,     0,   108,     9,   105,     0,
   106,    85,   107,     0,   106,    85,     0,    50,     0,    51,
     0,    52,     0,    55,    50,     0,    55,    51,     0,    55,
    52,     0,    53,     0,    53,    53,     0,    55,    53,    53,
     0,    56,     0,    57,     0,    60,     0,   108,    83,     0,
   108,    88,    11,    79,     0,   108,    88,    79,     0,     9,
     0,    58,   108,     0,    61,     9,     0,    61,    86,   107,
    87,     0
};

#endif

#if YY_AnsiCParser_DEBUG != 0
static const short yyrline[] = { 0,
   157,   161,   163,   167,   169,   171,   173,   177,   180,   184,
   188,   192,   197,   201,   205,   207,   211,   215,   220,   223,
   226,   229,   232,   237,   253,   267,   271,   273,   287,   301,
   313,   317,   329,   343,   358,   377,   383,   389,   397,   401,
   405,   409,   412,   415,   419,   422,   425,   428,   433,   438,
   446,   450,   456,   458,   460,   462,   464,   466,   468,   470,
   472,   474,   476,   478,   480,   482,   486,   490,   495,   497,
   503
};

static const char * const yytname[] = {   "$","error","$illegal.","PREINCLUDE",
"PREDEFINE","PREIF","PREIFDEF","PREENDIF","PRELINE","IDENTIFIER","STRING_LITERAL",
"CONSTANT","SIZEOF","NODECODE","INCOMPLETE","SYMBOLREF","CDECL","PASCAL","REGOF",
"MEMOF","CUSTOM","PREFER","WITHSTACK","PTR_OP","INC_OP","DEC_OP","LEFT_OP","RIGHT_OP",
"LE_OP","GE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","MUL_ASSIGN","DIV_ASSIGN","MOD_ASSIGN",
"ADD_ASSIGN","SUB_ASSIGN","LEFT_ASSIGN","RIGHT_ASSIGN","AND_ASSIGN","XOR_ASSIGN",
"OR_ASSIGN","TYPE_NAME","TYPEDEF","EXTERN","STATIC","AUTO","REGISTER","CHAR",
"SHORT","INT","LONG","SIGNED","UNSIGNED","FLOAT","DOUBLE","CONST","VOLATILE",
"VOID","STRUCT","UNION","ENUM","ELLIPSIS","CASE","DEFAULT","IF","ELSE","SWITCH",
"WHILE","DO","FOR","GOTO","CONTINUE","BREAK","RETURN","','","':'","']'","'+'",
"'-'","'('","'*'","')'","';'","'{'","'}'","'['","translation_unit","decls","decl",
"convention","num_list","param_list","param_exp","exp","param","type_decl","func_decl",
"signature","symbol_ref_decl","symbol_decl","symbol_mods","custom_options","array_modifier",
"type_ident","type_ident_list","type",""
};
#endif

static const short yyr1[] = {     0,
    89,    90,    90,    91,    91,    91,    91,    92,    92,    93,
    93,    93,    94,    94,    94,    94,    95,    95,    96,    96,
    96,    96,    96,    97,    97,    97,    98,    98,    98,    98,
    99,    99,   100,   100,   100,   101,   102,   102,   103,   103,
   103,   104,   104,   104,   105,   105,   105,   105,   106,   106,
   107,   107,   108,   108,   108,   108,   108,   108,   108,   108,
   108,   108,   108,   108,   108,   108,   108,   108,   108,   108,
   108
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     3,
     1,     0,     3,     1,     1,     0,     3,     1,     3,     3,
     3,     3,     1,     1,     8,     1,     3,    10,     6,     6,
     2,     7,     4,     5,     6,     4,     3,     4,     2,     2,
     0,     2,     3,     0,     3,     2,     4,     3,     2,     3,
     3,     2,     1,     1,     1,     2,     2,     2,     1,     2,
     3,     1,     1,     1,     2,     4,     3,     1,     2,     2,
     4
};

static const short yydefact[] = {     3,
    68,    41,     0,     8,     9,    44,     0,    53,    54,    55,
    59,     0,    62,    63,     0,    64,     0,     1,     3,     0,
     4,     5,     0,     7,     6,     0,     0,    41,    41,     0,
     0,     0,     0,    23,     0,     0,     0,     0,     0,     0,
     0,    60,    56,    57,    58,     0,    69,    70,     0,     2,
     0,     0,    31,    16,    49,    65,     0,    39,    40,    70,
     0,    37,     0,     0,     0,     0,    42,     0,     0,     0,
    16,    27,     0,    61,     0,     0,     0,    16,     0,    64,
    26,     0,    14,     0,    18,    24,     0,     0,    50,     0,
    67,    38,    36,    19,    20,    43,    21,    22,    16,     0,
     0,     0,    52,    71,     0,    12,    33,    16,     0,     0,
     0,    46,     0,    66,     0,     0,     0,     0,    51,    34,
    11,     0,    13,    17,     0,    45,     0,    48,    35,    29,
     0,    30,    12,     0,     0,    47,    16,    10,    32,     0,
     0,    16,     0,     0,    28,    25,     0,     0,     0
};

static const short yydefgoto[] = {   147,
    18,    19,    20,   122,    82,    83,    84,    85,    21,    22,
    23,    24,    25,    31,    39,    89,    86,    77,    27
};

static const short yypact[] = {   123,
-32768,   157,     8,-32768,-32768,     7,   173,-32768,-32768,-32768,
   -41,     5,-32768,-32768,   173,-32768,    -3,-32768,   123,   173,
-32768,-32768,   -18,-32768,-32768,   -55,    -2,    36,    36,     1,
   145,   -53,    25,-32768,    33,    19,    35,   -65,   173,   -37,
    -8,-32768,-32768,-32768,-32768,   -17,   -60,   -33,   173,-32768,
   -16,   173,-32768,    95,   -15,-32768,     3,-32768,-32768,-32768,
   -13,-32768,     4,    12,   -11,    10,-32768,    19,    19,    11,
    95,-32768,    13,-32768,   173,    14,    16,    95,    18,    23,
-32768,    24,    32,   -38,-32768,-32768,    -4,     6,    22,    37,
-32768,-32768,-32768,-32768,-32768,-32768,   -21,   -21,    95,    27,
   103,    28,   173,-32768,    42,   110,-32768,    95,    67,    46,
    54,-32768,     9,-32768,    52,    56,    53,    57,-32768,-32768,
    72,    60,-32768,-32768,   148,-32768,    79,-32768,-32768,-32768,
    78,-32768,   110,    82,    80,-32768,    95,-32768,-32768,    81,
    85,    95,    87,    93,-32768,-32768,   185,   186,-32768
};

static const short yypgoto[] = {-32768,
   168,-32768,-32768,    55,   -47,-32768,    29,    83,-32768,-32768,
   158,-32768,-32768,    34,-32768,-32768,     2,   -64,    -7
};


#define	YYLAST		234


static const short yytable[] = {    41,
    55,    26,    52,    32,    55,    48,    55,    47,    40,    60,
   102,    42,    67,    90,    68,    69,   111,    34,    33,   127,
    26,    51,    56,   100,    35,    36,    54,    57,    37,    34,
   105,    62,    26,    63,    38,    74,    35,    36,   119,   109,
    70,    68,    69,    64,    71,    66,    87,    72,    28,    29,
    76,   115,    75,    79,    43,    44,    45,    46,    68,    69,
   123,    58,    59,    87,    65,    78,    53,    95,    68,    69,
    87,    92,    88,    73,    56,     1,    76,   110,    56,    57,
    56,    91,    49,    57,   112,    57,    49,   128,    93,   141,
    94,    87,    99,    96,   144,   101,    97,    98,   103,   106,
    87,    87,   104,     1,    76,    34,   -15,   107,   108,   113,
   116,   117,    35,    36,   118,   114,     8,     9,    10,    11,
   121,    12,    13,    14,    15,   120,    16,    30,   125,    87,
    81,     1,   126,     2,    87,   129,   131,     3,     4,     5,
   130,   132,     6,   134,     8,     9,    10,    11,   133,    12,
    13,    14,    15,     1,    80,    30,   135,   136,    81,   137,
     4,     5,   142,   140,     6,     1,   139,     7,   143,    28,
    29,   145,     8,     9,    10,    11,   146,    12,    13,    14,
    15,     1,    16,    17,   148,   149,    50,   138,    61,     0,
     0,   124,     0,     0,     8,     9,    10,    11,     0,    12,
    13,    14,    15,     0,    16,    30,     8,     9,    10,    11,
     0,    12,    13,    14,    15,     0,    16,    30,     0,     0,
     0,     0,     8,     9,    10,    11,     0,    12,    13,    14,
    15,     0,    16,    30
};

static const short yycheck[] = {     7,
     9,     0,    21,     2,     9,     9,     9,    15,     7,     9,
    75,    53,    78,    11,    80,    81,    11,    11,    11,    11,
    19,    20,    83,    71,    18,    19,    82,    88,    22,    11,
    78,    85,    31,     9,     6,    53,    18,    19,   103,    78,
    39,    80,    81,    11,    82,    11,    54,    85,    13,    14,
    49,    99,    86,    52,    50,    51,    52,    53,    80,    81,
   108,    28,    29,    71,    36,    82,    85,    79,    80,    81,
    78,    85,    88,    82,    83,     9,    75,    82,    83,    88,
    83,    79,    86,    88,    79,    88,    86,    79,    85,   137,
    79,    99,    82,    84,   142,    83,    68,    69,    85,    82,
   108,   109,    87,     9,   103,    11,    84,    84,    77,    88,
    84,     9,    18,    19,    87,    79,    50,    51,    52,    53,
    11,    55,    56,    57,    58,    84,    60,    61,    83,   137,
    64,     9,    79,    11,   142,    84,    84,    15,    16,    17,
    85,    85,    20,    84,    50,    51,    52,    53,    77,    55,
    56,    57,    58,     9,    60,    61,     9,    79,    64,    82,
    16,    17,    82,    84,    20,     9,    85,    45,    84,    13,
    14,    85,    50,    51,    52,    53,    84,    55,    56,    57,
    58,     9,    60,    61,     0,     0,    19,   133,    31,    -1,
    -1,   109,    -1,    -1,    50,    51,    52,    53,    -1,    55,
    56,    57,    58,    -1,    60,    61,    50,    51,    52,    53,
    -1,    55,    56,    57,    58,    -1,    60,    61,    -1,    -1,
    -1,    -1,    50,    51,    52,    53,    -1,    55,    56,    57,
    58,    -1,    60,    61
};

#line 329 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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


/* #line 783 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 1484 "ansi-c-parser.cpp"

  switch (yyn) {

case 1:
#line 158 "ansi-c.y"
{ ;
    break;}
case 2:
#line 162 "ansi-c.y"
{ ;
    break;}
case 3:
#line 164 "ansi-c.y"
{ ;
    break;}
case 4:
#line 168 "ansi-c.y"
{ ;
    break;}
case 5:
#line 170 "ansi-c.y"
{ ;
    break;}
case 6:
#line 172 "ansi-c.y"
{ ;
    break;}
case 7:
#line 174 "ansi-c.y"
{ ;
    break;}
case 8:
#line 179 "ansi-c.y"
{ yyval.cc = CONV_C; ;
    break;}
case 9:
#line 181 "ansi-c.y"
{ yyval.cc = CONV_PASCAL; ;
    break;}
case 10:
#line 185 "ansi-c.y"
{ yyval.num_list = yyvsp[0].num_list;
          yyval.num_list->push_front(yyvsp[-2].ival);
        ;
    break;}
case 11:
#line 189 "ansi-c.y"
{ yyval.num_list = new std::list<int>();
          yyval.num_list->push_back(yyvsp[0].ival);
        ;
    break;}
case 12:
#line 193 "ansi-c.y"
{ yyval.num_list = new std::list<int>();
        ;
    break;}
case 13:
#line 198 "ansi-c.y"
{ yyval.param_list = yyvsp[0].param_list;
            yyval.param_list->push_front(yyvsp[-2].param);
          ;
    break;}
case 14:
#line 202 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>(); 
            yyval.param_list->push_back(yyvsp[0].param);
          ;
    break;}
case 15:
#line 206 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 16:
#line 208 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 17:
#line 212 "ansi-c.y"
{ yyval.param = yyvsp[0].param;
      yyval.param->setExp(yyvsp[-2].exp);
    ;
    break;}
case 18:
#line 216 "ansi-c.y"
{ yyval.param = yyvsp[0].param;
    ;
    break;}
case 19:
#line 221 "ansi-c.y"
{ yyval.exp = Location::regOf(yyvsp[-1].ival);
    ;
    break;}
case 20:
#line 224 "ansi-c.y"
{ yyval.exp = Location::memOf(yyvsp[-1].exp);
    ;
    break;}
case 21:
#line 227 "ansi-c.y"
{ yyval.exp = new Binary(opPlus, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 22:
#line 230 "ansi-c.y"
{ yyval.exp = new Binary(opMinus, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 23:
#line 233 "ansi-c.y"
{ yyval.exp = new Const(yyvsp[0].ival);
    ;
    break;}
case 24:
#line 238 "ansi-c.y"
{  if (yyvsp[0].type_ident->ty->isArray() || 
            (yyvsp[0].type_ident->ty->isNamed() && 
             ((NamedType*)yyvsp[0].type_ident->ty)->resolvesTo() &&
             ((NamedType*)yyvsp[0].type_ident->ty)->resolvesTo()->isArray())) {
            /* C has complex semantics for passing arrays.. seeing as 
             * we're supposedly parsing C, then we should deal with this.
             * When you pass an array in C it is understood that you are
             * passing that array "by reference".  As all parameters in
             * our internal representation are passed "by value", we alter
             * the type here to be a pointer to an array.
             */
            yyvsp[0].type_ident->ty = new PointerType(yyvsp[0].type_ident->ty);
        }
        yyval.param = new Parameter(yyvsp[0].type_ident->ty, yyvsp[0].type_ident->nam.c_str()); 
     ;
    break;}
case 25:
#line 254 "ansi-c.y"
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
case 26:
#line 268 "ansi-c.y"
{ yyval.param = new Parameter(new VoidType, "..."); ;
    break;}
case 27:
#line 272 "ansi-c.y"
{ Type::addNamedType(yyvsp[-1].type_ident->nam.c_str(), yyvsp[-1].type_ident->ty); ;
    break;}
case 28:
#line 274 "ansi-c.y"
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
case 29:
#line 288 "ansi-c.y"
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
case 30:
#line 302 "ansi-c.y"
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
case 31:
#line 314 "ansi-c.y"
{
           signatures.push_back(yyvsp[-1].sig);
         ;
    break;}
case 32:
#line 318 "ansi-c.y"
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
case 33:
#line 330 "ansi-c.y"
{ Signature *sig = Signature::instantiate(plat, cc, yyvsp[-3].type_ident->nam.c_str()); 
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
case 34:
#line 344 "ansi-c.y"
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
case 35:
#line 359 "ansi-c.y"
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
case 36:
#line 378 "ansi-c.y"
{ SymbolRef *ref = new SymbolRef(yyvsp[-2].ival, yyvsp[-1].str);
              refs.push_back(ref);
            ;
    break;}
case 37:
#line 384 "ansi-c.y"
{ Symbol *sym = new Symbol(yyvsp[-2].ival);
             sym->nam = yyvsp[-1].type_ident->nam;
             sym->ty = yyvsp[-1].type_ident->ty;
             symbols.push_back(sym);
           ;
    break;}
case 38:
#line 390 "ansi-c.y"
{ Symbol *sym = new Symbol(yyvsp[-3].ival);
             sym->sig = yyvsp[-1].sig;
             sym->mods = yyvsp[-2].mods;
             symbols.push_back(sym);
           ;
    break;}
case 39:
#line 398 "ansi-c.y"
{ yyval.mods = yyvsp[0].mods;
             yyval.mods->noDecode = true;
           ;
    break;}
case 40:
#line 402 "ansi-c.y"
{ yyval.mods = yyvsp[0].mods;
             yyval.mods->incomplete = true;
           ;
    break;}
case 41:
#line 406 "ansi-c.y"
{ yyval.mods = new SymbolMods(); ;
    break;}
case 42:
#line 410 "ansi-c.y"
{ yyval.custom_options = new CustomOptions(); yyval.custom_options->exp = yyvsp[-1].exp;
           ;
    break;}
case 43:
#line 413 "ansi-c.y"
{ yyval.custom_options = new CustomOptions(); yyval.custom_options->sp = yyvsp[-1].ival;
           ;
    break;}
case 44:
#line 416 "ansi-c.y"
{ yyval.custom_options = new CustomOptions(); ;
    break;}
case 45:
#line 420 "ansi-c.y"
{ yyval.type = new ArrayType(NULL, yyvsp[-1].ival);
          ;
    break;}
case 46:
#line 423 "ansi-c.y"
{ yyval.type = new ArrayType(NULL);
          ;
    break;}
case 47:
#line 426 "ansi-c.y"
{ yyval.type = new ArrayType(yyvsp[-3].type, yyvsp[-1].ival);
          ;
    break;}
case 48:
#line 429 "ansi-c.y"
{ yyval.type = new ArrayType(yyvsp[-2].type);
          ;
    break;}
case 49:
#line 434 "ansi-c.y"
{ yyval.type_ident = new TypeIdent();
            yyval.type_ident->ty = yyvsp[-1].type;
            yyval.type_ident->nam = yyvsp[0].str;
          ;
    break;}
case 50:
#line 439 "ansi-c.y"
{ yyval.type_ident = new TypeIdent();
            ((ArrayType*)yyvsp[0].type)->fixBaseType(yyvsp[-2].type);
            yyval.type_ident->ty = yyvsp[0].type;
            yyval.type_ident->nam = yyvsp[-1].str;
          ;
    break;}
case 51:
#line 447 "ansi-c.y"
{ yyval.type_ident_list = yyvsp[0].type_ident_list;
            yyval.type_ident_list->push_front(yyvsp[-2].type_ident);
          ;
    break;}
case 52:
#line 451 "ansi-c.y"
{ yyval.type_ident_list = new std::list<TypeIdent*>(); 
            yyval.type_ident_list->push_back(yyvsp[-1].type_ident);
          ;
    break;}
case 53:
#line 457 "ansi-c.y"
{ yyval.type = new CharType(); ;
    break;}
case 54:
#line 459 "ansi-c.y"
{ yyval.type = new IntegerType(16); ;
    break;}
case 55:
#line 461 "ansi-c.y"
{ yyval.type = new IntegerType(); ;
    break;}
case 56:
#line 463 "ansi-c.y"
{ yyval.type = new IntegerType(8, false); ;
    break;}
case 57:
#line 465 "ansi-c.y"
{ yyval.type = new IntegerType(16, false); ;
    break;}
case 58:
#line 467 "ansi-c.y"
{ yyval.type = new IntegerType(32, false); ;
    break;}
case 59:
#line 469 "ansi-c.y"
{ yyval.type = new IntegerType(); ;
    break;}
case 60:
#line 471 "ansi-c.y"
{ yyval.type = new IntegerType(64); ;
    break;}
case 61:
#line 473 "ansi-c.y"
{ yyval.type = new IntegerType(64, false); ;
    break;}
case 62:
#line 475 "ansi-c.y"
{ yyval.type = new FloatType(32); ;
    break;}
case 63:
#line 477 "ansi-c.y"
{ yyval.type = new FloatType(64); ;
    break;}
case 64:
#line 479 "ansi-c.y"
{ yyval.type = new VoidType(); ;
    break;}
case 65:
#line 481 "ansi-c.y"
{ yyval.type = new PointerType(yyvsp[-1].type); ;
    break;}
case 66:
#line 483 "ansi-c.y"
{ // This isn't C, but it makes defining pointers to arrays easier
      yyval.type = new ArrayType(yyvsp[-3].type, yyvsp[-1].ival); 
    ;
    break;}
case 67:
#line 487 "ansi-c.y"
{ // This isn't C, but it makes defining pointers to arrays easier
      yyval.type = new ArrayType(yyvsp[-2].type); 
    ;
    break;}
case 68:
#line 491 "ansi-c.y"
{ //$$ = Type::getNamedType($1); 
      //if ($$ == NULL)
      yyval.type = new NamedType(yyvsp[0].str);
    ;
    break;}
case 69:
#line 496 "ansi-c.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 70:
#line 498 "ansi-c.y"
{
      char tmp[1024];
      sprintf(tmp, "struct %s", yyvsp[0].str);
      yyval.type = new NamedType(tmp);
    ;
    break;}
case 71:
#line 504 "ansi-c.y"
{ CompoundType *t = new CompoundType(); 
      for (std::list<TypeIdent*>::iterator it = yyvsp[-1].type_ident_list->begin();
           it != yyvsp[-1].type_ident_list->end(); it++) {
          t->addType((*it)->ty, (*it)->nam.c_str());
      }
      yyval.type = t;
    ;
    break;}
}

#line 783 "/home/38/binary/u1.luna.tools/lib/bison.cc"
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

/* #line 982 "/home/38/binary/u1.luna.tools/lib/bison.cc" */
#line 2149 "ansi-c-parser.cpp"
#line 513 "ansi-c.y"

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


