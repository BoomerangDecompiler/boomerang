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
#line 85 "ansi-c-parser.cpp"
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
#line 232 "ansi-c-parser.cpp"

#line 117 "/usr/local/lib/bison.cc"
/*  YY_AnsiCParser_PURE */
#endif

/* section apres lecture def, avant lecture grammaire S2 */

/* #line 121 "/usr/local/lib/bison.cc" */
#line 241 "ansi-c-parser.cpp"

#line 121 "/usr/local/lib/bison.cc"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 123 "/usr/local/lib/bison.cc" */
#line 248 "ansi-c-parser.cpp"

#line 123 "/usr/local/lib/bison.cc"
/* YY_AnsiCParser_DEBUG */
#endif


#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 128 "/usr/local/lib/bison.cc" */
#line 258 "ansi-c-parser.cpp"

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
#line 371 "ansi-c-parser.cpp"
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
#define	CUSTOM	276
#define	PREFER	277
#define	WITHSTACK	278
#define	PTR_OP	279
#define	INC_OP	280
#define	DEC_OP	281
#define	LEFT_OP	282
#define	RIGHT_OP	283
#define	LE_OP	284
#define	GE_OP	285
#define	EQ_OP	286
#define	NE_OP	287
#define	AND_OP	288
#define	OR_OP	289
#define	MUL_ASSIGN	290
#define	DIV_ASSIGN	291
#define	MOD_ASSIGN	292
#define	ADD_ASSIGN	293
#define	SUB_ASSIGN	294
#define	LEFT_ASSIGN	295
#define	RIGHT_ASSIGN	296
#define	AND_ASSIGN	297
#define	XOR_ASSIGN	298
#define	OR_ASSIGN	299
#define	TYPE_NAME	300
#define	TYPEDEF	301
#define	EXTERN	302
#define	STATIC	303
#define	AUTO	304
#define	REGISTER	305
#define	CHAR	306
#define	SHORT	307
#define	INT	308
#define	LONG	309
#define	SIGNED	310
#define	UNSIGNED	311
#define	FLOAT	312
#define	DOUBLE	313
#define	CONST	314
#define	VOLATILE	315
#define	VOID	316
#define	STRUCT	317
#define	UNION	318
#define	ENUM	319
#define	ELLIPSIS	320
#define	CASE	321
#define	DEFAULT	322
#define	IF	323
#define	ELSE	324
#define	SWITCH	325
#define	WHILE	326
#define	DO	327
#define	FOR	328
#define	GOTO	329
#define	CONTINUE	330
#define	BREAK	331
#define	RETURN	332


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
#line 496 "ansi-c-parser.cpp"
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
#line 580 "ansi-c-parser.cpp"
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
	,CUSTOM=276
	,PREFER=277
	,WITHSTACK=278
	,PTR_OP=279
	,INC_OP=280
	,DEC_OP=281
	,LEFT_OP=282
	,RIGHT_OP=283
	,LE_OP=284
	,GE_OP=285
	,EQ_OP=286
	,NE_OP=287
	,AND_OP=288
	,OR_OP=289
	,MUL_ASSIGN=290
	,DIV_ASSIGN=291
	,MOD_ASSIGN=292
	,ADD_ASSIGN=293
	,SUB_ASSIGN=294
	,LEFT_ASSIGN=295
	,RIGHT_ASSIGN=296
	,AND_ASSIGN=297
	,XOR_ASSIGN=298
	,OR_ASSIGN=299
	,TYPE_NAME=300
	,TYPEDEF=301
	,EXTERN=302
	,STATIC=303
	,AUTO=304
	,REGISTER=305
	,CHAR=306
	,SHORT=307
	,INT=308
	,LONG=309
	,SIGNED=310
	,UNSIGNED=311
	,FLOAT=312
	,DOUBLE=313
	,CONST=314
	,VOLATILE=315
	,VOID=316
	,STRUCT=317
	,UNION=318
	,ENUM=319
	,ELLIPSIS=320
	,CASE=321
	,DEFAULT=322
	,IF=323
	,ELSE=324
	,SWITCH=325
	,WHILE=326
	,DO=327
	,FOR=328
	,GOTO=329
	,CONTINUE=330
	,BREAK=331
	,RETURN=332


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
#line 692 "ansi-c-parser.cpp"
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
const int YY_AnsiCParser_CLASS::CUSTOM=276;
const int YY_AnsiCParser_CLASS::PREFER=277;
const int YY_AnsiCParser_CLASS::WITHSTACK=278;
const int YY_AnsiCParser_CLASS::PTR_OP=279;
const int YY_AnsiCParser_CLASS::INC_OP=280;
const int YY_AnsiCParser_CLASS::DEC_OP=281;
const int YY_AnsiCParser_CLASS::LEFT_OP=282;
const int YY_AnsiCParser_CLASS::RIGHT_OP=283;
const int YY_AnsiCParser_CLASS::LE_OP=284;
const int YY_AnsiCParser_CLASS::GE_OP=285;
const int YY_AnsiCParser_CLASS::EQ_OP=286;
const int YY_AnsiCParser_CLASS::NE_OP=287;
const int YY_AnsiCParser_CLASS::AND_OP=288;
const int YY_AnsiCParser_CLASS::OR_OP=289;
const int YY_AnsiCParser_CLASS::MUL_ASSIGN=290;
const int YY_AnsiCParser_CLASS::DIV_ASSIGN=291;
const int YY_AnsiCParser_CLASS::MOD_ASSIGN=292;
const int YY_AnsiCParser_CLASS::ADD_ASSIGN=293;
const int YY_AnsiCParser_CLASS::SUB_ASSIGN=294;
const int YY_AnsiCParser_CLASS::LEFT_ASSIGN=295;
const int YY_AnsiCParser_CLASS::RIGHT_ASSIGN=296;
const int YY_AnsiCParser_CLASS::AND_ASSIGN=297;
const int YY_AnsiCParser_CLASS::XOR_ASSIGN=298;
const int YY_AnsiCParser_CLASS::OR_ASSIGN=299;
const int YY_AnsiCParser_CLASS::TYPE_NAME=300;
const int YY_AnsiCParser_CLASS::TYPEDEF=301;
const int YY_AnsiCParser_CLASS::EXTERN=302;
const int YY_AnsiCParser_CLASS::STATIC=303;
const int YY_AnsiCParser_CLASS::AUTO=304;
const int YY_AnsiCParser_CLASS::REGISTER=305;
const int YY_AnsiCParser_CLASS::CHAR=306;
const int YY_AnsiCParser_CLASS::SHORT=307;
const int YY_AnsiCParser_CLASS::INT=308;
const int YY_AnsiCParser_CLASS::LONG=309;
const int YY_AnsiCParser_CLASS::SIGNED=310;
const int YY_AnsiCParser_CLASS::UNSIGNED=311;
const int YY_AnsiCParser_CLASS::FLOAT=312;
const int YY_AnsiCParser_CLASS::DOUBLE=313;
const int YY_AnsiCParser_CLASS::CONST=314;
const int YY_AnsiCParser_CLASS::VOLATILE=315;
const int YY_AnsiCParser_CLASS::VOID=316;
const int YY_AnsiCParser_CLASS::STRUCT=317;
const int YY_AnsiCParser_CLASS::UNION=318;
const int YY_AnsiCParser_CLASS::ENUM=319;
const int YY_AnsiCParser_CLASS::ELLIPSIS=320;
const int YY_AnsiCParser_CLASS::CASE=321;
const int YY_AnsiCParser_CLASS::DEFAULT=322;
const int YY_AnsiCParser_CLASS::IF=323;
const int YY_AnsiCParser_CLASS::ELSE=324;
const int YY_AnsiCParser_CLASS::SWITCH=325;
const int YY_AnsiCParser_CLASS::WHILE=326;
const int YY_AnsiCParser_CLASS::DO=327;
const int YY_AnsiCParser_CLASS::FOR=328;
const int YY_AnsiCParser_CLASS::GOTO=329;
const int YY_AnsiCParser_CLASS::CONTINUE=330;
const int YY_AnsiCParser_CLASS::BREAK=331;
const int YY_AnsiCParser_CLASS::RETURN=332;


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
#line 784 "ansi-c-parser.cpp"


#define	YYFINAL		150
#define	YYFLAG		-32768
#define	YYNTBASE	90

#define YYTRANSLATE(x) ((unsigned)(x) <= 332 ? yytranslate[x] : 110)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    83,
    85,    84,    81,    78,    82,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    79,    86,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    89,     2,    80,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    87,     2,    88,     2,     2,     2,     2,     2,
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
    76,    77
};

#if YY_AnsiCParser_DEBUG != 0
static const short yyprhs[] = {     0,
     0,     2,     5,     6,     8,    10,    12,    14,    16,    18,
    20,    24,    26,    27,    31,    33,    35,    36,    40,    42,
    46,    50,    54,    58,    60,    62,    71,    73,    77,    88,
    95,   102,   105,   113,   118,   124,   131,   136,   140,   145,
   148,   151,   152,   155,   159,   160,   164,   167,   172,   176,
   179,   183,   187,   190,   192,   194,   196,   199,   202,   205,
   207,   210,   214,   216,   218,   220,   223,   228,   232,   234,
   237,   240
};

static const short yyrhs[] = {    91,
     0,    92,    91,     0,     0,    99,     0,   100,     0,   103,
     0,   102,     0,    16,     0,    17,     0,    18,     0,    11,
    78,    94,     0,    11,     0,     0,    96,    78,    95,     0,
    96,     0,    61,     0,     0,    97,    79,    98,     0,    98,
     0,    19,    11,    80,     0,    20,    97,    80,     0,    97,
    81,    97,     0,    97,    82,    97,     0,    11,     0,   107,
     0,   109,    83,    84,     9,    85,    83,    95,    85,     0,
    65,     0,    46,   107,    86,     0,    46,   109,    83,    84,
     9,    85,    83,    95,    85,    86,     0,    46,   107,    83,
    95,    85,    86,     0,    62,     9,    87,   108,    88,    86,
     0,   101,    86,     0,   101,    22,   107,    83,    94,    85,
    86,     0,   107,    83,    95,    85,     0,    93,   107,    83,
    95,    85,     0,    21,   105,   107,    83,    95,    85,     0,
    15,    11,     9,    86,     0,    11,   107,    86,     0,    11,
   104,   101,    86,     0,    13,   104,     0,    14,   104,     0,
     0,    97,    79,     0,    23,    11,    85,     0,     0,    89,
    11,    80,     0,    89,    80,     0,   106,    89,    11,    80,
     0,   106,    89,    80,     0,   109,     9,     0,   109,     9,
   106,     0,   107,    86,   108,     0,   107,    86,     0,    51,
     0,    52,     0,    53,     0,    56,    51,     0,    56,    52,
     0,    56,    53,     0,    54,     0,    54,    54,     0,    56,
    54,    54,     0,    57,     0,    58,     0,    61,     0,   109,
    84,     0,   109,    89,    11,    80,     0,   109,    89,    80,
     0,     9,     0,    59,   109,     0,    62,     9,     0,    62,
    87,   108,    88,     0
};

#endif

#if YY_AnsiCParser_DEBUG != 0
static const short yyrline[] = { 0,
   157,   161,   163,   167,   169,   171,   173,   177,   180,   182,
   186,   190,   194,   199,   203,   207,   209,   213,   217,   222,
   225,   228,   231,   234,   239,   255,   269,   273,   275,   289,
   303,   315,   319,   331,   345,   360,   379,   385,   391,   399,
   403,   407,   411,   414,   417,   421,   424,   427,   430,   435,
   440,   448,   452,   458,   460,   462,   464,   466,   468,   470,
   472,   474,   476,   478,   480,   482,   484,   488,   492,   497,
   499,   505
};

static const char * const yytname[] = {   "$","error","$illegal.","PREINCLUDE",
"PREDEFINE","PREIF","PREIFDEF","PREENDIF","PRELINE","IDENTIFIER","STRING_LITERAL",
"CONSTANT","SIZEOF","NODECODE","INCOMPLETE","SYMBOLREF","CDECL","PASCAL","THISCALL",
"REGOF","MEMOF","CUSTOM","PREFER","WITHSTACK","PTR_OP","INC_OP","DEC_OP","LEFT_OP",
"RIGHT_OP","LE_OP","GE_OP","EQ_OP","NE_OP","AND_OP","OR_OP","MUL_ASSIGN","DIV_ASSIGN",
"MOD_ASSIGN","ADD_ASSIGN","SUB_ASSIGN","LEFT_ASSIGN","RIGHT_ASSIGN","AND_ASSIGN",
"XOR_ASSIGN","OR_ASSIGN","TYPE_NAME","TYPEDEF","EXTERN","STATIC","AUTO","REGISTER",
"CHAR","SHORT","INT","LONG","SIGNED","UNSIGNED","FLOAT","DOUBLE","CONST","VOLATILE",
"VOID","STRUCT","UNION","ENUM","ELLIPSIS","CASE","DEFAULT","IF","ELSE","SWITCH",
"WHILE","DO","FOR","GOTO","CONTINUE","BREAK","RETURN","','","':'","']'","'+'",
"'-'","'('","'*'","')'","';'","'{'","'}'","'['","translation_unit","decls","decl",
"convention","num_list","param_list","param_exp","exp","param","type_decl","func_decl",
"signature","symbol_ref_decl","symbol_decl","symbol_mods","custom_options","array_modifier",
"type_ident","type_ident_list","type",""
};
#endif

static const short yyr1[] = {     0,
    90,    91,    91,    92,    92,    92,    92,    93,    93,    93,
    94,    94,    94,    95,    95,    95,    95,    96,    96,    97,
    97,    97,    97,    97,    98,    98,    98,    99,    99,    99,
    99,   100,   100,   101,   101,   101,   102,   103,   103,   104,
   104,   104,   105,   105,   105,   106,   106,   106,   106,   107,
   107,   108,   108,   109,   109,   109,   109,   109,   109,   109,
   109,   109,   109,   109,   109,   109,   109,   109,   109,   109,
   109,   109
};

static const short yyr2[] = {     0,
     1,     2,     0,     1,     1,     1,     1,     1,     1,     1,
     3,     1,     0,     3,     1,     1,     0,     3,     1,     3,
     3,     3,     3,     1,     1,     8,     1,     3,    10,     6,
     6,     2,     7,     4,     5,     6,     4,     3,     4,     2,
     2,     0,     2,     3,     0,     3,     2,     4,     3,     2,
     3,     3,     2,     1,     1,     1,     2,     2,     2,     1,
     2,     3,     1,     1,     1,     2,     4,     3,     1,     2,
     2,     4
};

static const short yydefact[] = {     3,
    69,    42,     0,     8,     9,    10,    45,     0,    54,    55,
    56,    60,     0,    63,    64,     0,    65,     0,     1,     3,
     0,     4,     5,     0,     7,     6,     0,     0,    42,    42,
     0,     0,     0,     0,    24,     0,     0,     0,     0,     0,
     0,     0,    61,    57,    58,    59,     0,    70,    71,     0,
     2,     0,     0,    32,    17,    50,    66,     0,    40,    41,
    71,     0,    38,     0,     0,     0,     0,    43,     0,     0,
     0,    17,    28,     0,    62,     0,     0,     0,    17,     0,
    65,    27,     0,    15,     0,    19,    25,     0,     0,    51,
     0,    68,    39,    37,    20,    21,    44,    22,    23,    17,
     0,     0,     0,    53,    72,     0,    13,    34,    17,     0,
     0,     0,    47,     0,    67,     0,     0,     0,     0,    52,
    35,    12,     0,    14,    18,     0,    46,     0,    49,    36,
    30,     0,    31,    13,     0,     0,    48,    17,    11,    33,
     0,     0,    17,     0,     0,    29,    26,     0,     0,     0
};

static const short yydefgoto[] = {   148,
    19,    20,    21,   123,    83,    84,    85,    86,    22,    23,
    24,    25,    26,    32,    40,    90,    87,    78,    28
};

static const short yypact[] = {   111,
-32768,   162,     4,-32768,-32768,-32768,     7,   174,-32768,-32768,
-32768,   -41,   -17,-32768,-32768,   174,-32768,    -4,-32768,   111,
   174,-32768,-32768,   -20,-32768,-32768,   -60,    -2,    25,    25,
     2,   138,   -54,    35,-32768,    49,     9,    52,   -36,   174,
   -24,    -5,-32768,-32768,-32768,-32768,    16,   -72,     6,   174,
-32768,    -9,   174,-32768,    56,    10,-32768,     5,-32768,-32768,
-32768,    11,-32768,    12,    20,   -32,    18,-32768,     9,     9,
    21,    56,-32768,    22,-32768,   174,    30,    23,    56,    36,
    38,-32768,    39,    13,   -26,-32768,-32768,    -3,     8,    42,
    45,-32768,-32768,-32768,-32768,-32768,-32768,   -13,   -13,    56,
    48,   125,    54,   174,-32768,    51,   126,-32768,    56,    87,
    66,    71,-32768,    14,-32768,    68,    72,    75,    80,-32768,
-32768,    83,    89,-32768,-32768,   168,-32768,    98,-32768,-32768,
-32768,    96,-32768,   126,    94,    97,-32768,    56,-32768,-32768,
   101,   100,    56,    95,   102,-32768,-32768,   186,   188,-32768
};

static const short yypgoto[] = {-32768,
   173,-32768,-32768,    64,   -48,-32768,     3,    91,-32768,-32768,
   170,-32768,-32768,    28,-32768,-32768,     1,   -62,    -8
};


#define	YYLAST		236


static const short yytable[] = {    42,
    27,    53,    33,    56,    49,    56,    56,    48,    41,    39,
    61,    57,    43,   103,    34,    91,    58,    35,   112,    35,
    27,    52,    55,   101,   128,    36,    37,    36,    37,    38,
   106,    63,    27,    44,    45,    46,    47,    29,    30,    66,
    71,   120,    68,    64,    69,    70,    88,    96,    69,    70,
    77,   116,   110,    80,    69,    70,    59,    60,    72,    65,
   124,    73,    67,    88,     1,    54,    35,    69,    70,    75,
    88,    98,    99,    79,    36,    37,    77,    74,    57,   111,
    57,    57,    50,    58,    92,    58,    58,   113,    50,   142,
   109,    88,    76,   129,   145,     1,    93,    94,    89,    95,
    88,    88,    97,   100,    77,   102,     9,    10,    11,    12,
   105,    13,    14,    15,    16,   104,    81,    31,   107,     1,
    82,     2,   -16,   108,   115,     3,     4,     5,     6,    88,
   114,     7,   117,   118,    88,   121,   122,     9,    10,    11,
    12,   119,    13,    14,    15,    16,     1,    17,    31,   126,
   127,    82,   130,     4,     5,     6,     8,   131,     7,   132,
   134,     9,    10,    11,    12,   133,    13,    14,    15,    16,
     1,    17,    18,   135,    29,    30,   136,   137,   138,   140,
   146,   141,     1,   143,   144,   149,   147,   150,     9,    10,
    11,    12,    51,    13,    14,    15,    16,   139,    17,    31,
   125,    62,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     9,    10,    11,    12,     0,    13,    14,    15,
    16,     0,    17,    31,     9,    10,    11,    12,     0,    13,
    14,    15,    16,     0,    17,    31
};

static const short yycheck[] = {     8,
     0,    22,     2,     9,     9,     9,     9,    16,     8,     7,
     9,    84,    54,    76,    11,    11,    89,    11,    11,    11,
    20,    21,    83,    72,    11,    19,    20,    19,    20,    23,
    79,    86,    32,    51,    52,    53,    54,    13,    14,    37,
    40,   104,    79,     9,    81,    82,    55,    80,    81,    82,
    50,   100,    79,    53,    81,    82,    29,    30,    83,    11,
   109,    86,    11,    72,     9,    86,    11,    81,    82,    54,
    79,    69,    70,    83,    19,    20,    76,    83,    84,    83,
    84,    84,    87,    89,    80,    89,    89,    80,    87,   138,
    78,   100,    87,    80,   143,     9,    86,    86,    89,    80,
   109,   110,    85,    83,   104,    84,    51,    52,    53,    54,
    88,    56,    57,    58,    59,    86,    61,    62,    83,     9,
    65,    11,    85,    85,    80,    15,    16,    17,    18,   138,
    89,    21,    85,     9,   143,    85,    11,    51,    52,    53,
    54,    88,    56,    57,    58,    59,     9,    61,    62,    84,
    80,    65,    85,    16,    17,    18,    46,    86,    21,    85,
    78,    51,    52,    53,    54,    86,    56,    57,    58,    59,
     9,    61,    62,    85,    13,    14,     9,    80,    83,    86,
    86,    85,     9,    83,    85,     0,    85,     0,    51,    52,
    53,    54,    20,    56,    57,    58,    59,   134,    61,    62,
   110,    32,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    51,    52,    53,    54,    -1,    56,    57,    58,
    59,    -1,    61,    62,    51,    52,    53,    54,    -1,    56,
    57,    58,    59,    -1,    61,    62
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
#line 1516 "ansi-c-parser.cpp"

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
#line 183 "ansi-c.y"
{ yyval.cc = CONV_THISCALL; ;
    break;}
case 11:
#line 187 "ansi-c.y"
{ yyval.num_list = yyvsp[0].num_list;
          yyval.num_list->push_front(yyvsp[-2].ival);
        ;
    break;}
case 12:
#line 191 "ansi-c.y"
{ yyval.num_list = new std::list<int>();
          yyval.num_list->push_back(yyvsp[0].ival);
        ;
    break;}
case 13:
#line 195 "ansi-c.y"
{ yyval.num_list = new std::list<int>();
        ;
    break;}
case 14:
#line 200 "ansi-c.y"
{ yyval.param_list = yyvsp[0].param_list;
            yyval.param_list->push_front(yyvsp[-2].param);
          ;
    break;}
case 15:
#line 204 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>(); 
            yyval.param_list->push_back(yyvsp[0].param);
          ;
    break;}
case 16:
#line 208 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 17:
#line 210 "ansi-c.y"
{ yyval.param_list = new std::list<Parameter*>();
    break;}
case 18:
#line 214 "ansi-c.y"
{ yyval.param = yyvsp[0].param;
      yyval.param->setExp(yyvsp[-2].exp);
    ;
    break;}
case 19:
#line 218 "ansi-c.y"
{ yyval.param = yyvsp[0].param;
    ;
    break;}
case 20:
#line 223 "ansi-c.y"
{ yyval.exp = Location::regOf(yyvsp[-1].ival);
    ;
    break;}
case 21:
#line 226 "ansi-c.y"
{ yyval.exp = Location::memOf(yyvsp[-1].exp);
    ;
    break;}
case 22:
#line 229 "ansi-c.y"
{ yyval.exp = new Binary(opPlus, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 23:
#line 232 "ansi-c.y"
{ yyval.exp = new Binary(opMinus, yyvsp[-2].exp, yyvsp[0].exp);
    ;
    break;}
case 24:
#line 235 "ansi-c.y"
{ yyval.exp = new Const(yyvsp[0].ival);
    ;
    break;}
case 25:
#line 240 "ansi-c.y"
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
case 26:
#line 256 "ansi-c.y"
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
case 27:
#line 270 "ansi-c.y"
{ yyval.param = new Parameter(new VoidType, "..."); ;
    break;}
case 28:
#line 274 "ansi-c.y"
{ Type::addNamedType(yyvsp[-1].type_ident->nam.c_str(), yyvsp[-1].type_ident->ty); ;
    break;}
case 29:
#line 276 "ansi-c.y"
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
case 30:
#line 290 "ansi-c.y"
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
case 31:
#line 304 "ansi-c.y"
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
case 32:
#line 316 "ansi-c.y"
{
           signatures.push_back(yyvsp[-1].sig);
         ;
    break;}
case 33:
#line 320 "ansi-c.y"
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
case 34:
#line 332 "ansi-c.y"
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
case 35:
#line 346 "ansi-c.y"
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
case 36:
#line 361 "ansi-c.y"
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
case 37:
#line 380 "ansi-c.y"
{ SymbolRef *ref = new SymbolRef(yyvsp[-2].ival, yyvsp[-1].str);
              refs.push_back(ref);
            ;
    break;}
case 38:
#line 386 "ansi-c.y"
{ Symbol *sym = new Symbol(yyvsp[-2].ival);
             sym->nam = yyvsp[-1].type_ident->nam;
             sym->ty = yyvsp[-1].type_ident->ty;
             symbols.push_back(sym);
           ;
    break;}
case 39:
#line 392 "ansi-c.y"
{ Symbol *sym = new Symbol(yyvsp[-3].ival);
             sym->sig = yyvsp[-1].sig;
             sym->mods = yyvsp[-2].mods;
             symbols.push_back(sym);
           ;
    break;}
case 40:
#line 400 "ansi-c.y"
{ yyval.mods = yyvsp[0].mods;
             yyval.mods->noDecode = true;
           ;
    break;}
case 41:
#line 404 "ansi-c.y"
{ yyval.mods = yyvsp[0].mods;
             yyval.mods->incomplete = true;
           ;
    break;}
case 42:
#line 408 "ansi-c.y"
{ yyval.mods = new SymbolMods(); ;
    break;}
case 43:
#line 412 "ansi-c.y"
{ yyval.custom_options = new CustomOptions(); yyval.custom_options->exp = yyvsp[-1].exp;
           ;
    break;}
case 44:
#line 415 "ansi-c.y"
{ yyval.custom_options = new CustomOptions(); yyval.custom_options->sp = yyvsp[-1].ival;
           ;
    break;}
case 45:
#line 418 "ansi-c.y"
{ yyval.custom_options = new CustomOptions(); ;
    break;}
case 46:
#line 422 "ansi-c.y"
{ yyval.type = new ArrayType(NULL, yyvsp[-1].ival);
          ;
    break;}
case 47:
#line 425 "ansi-c.y"
{ yyval.type = new ArrayType(NULL);
          ;
    break;}
case 48:
#line 428 "ansi-c.y"
{ yyval.type = new ArrayType(yyvsp[-3].type, yyvsp[-1].ival);
          ;
    break;}
case 49:
#line 431 "ansi-c.y"
{ yyval.type = new ArrayType(yyvsp[-2].type);
          ;
    break;}
case 50:
#line 436 "ansi-c.y"
{ yyval.type_ident = new TypeIdent();
            yyval.type_ident->ty = yyvsp[-1].type;
            yyval.type_ident->nam = yyvsp[0].str;
          ;
    break;}
case 51:
#line 441 "ansi-c.y"
{ yyval.type_ident = new TypeIdent();
            ((ArrayType*)yyvsp[0].type)->fixBaseType(yyvsp[-2].type);
            yyval.type_ident->ty = yyvsp[0].type;
            yyval.type_ident->nam = yyvsp[-1].str;
          ;
    break;}
case 52:
#line 449 "ansi-c.y"
{ yyval.type_ident_list = yyvsp[0].type_ident_list;
            yyval.type_ident_list->push_front(yyvsp[-2].type_ident);
          ;
    break;}
case 53:
#line 453 "ansi-c.y"
{ yyval.type_ident_list = new std::list<TypeIdent*>(); 
            yyval.type_ident_list->push_back(yyvsp[-1].type_ident);
          ;
    break;}
case 54:
#line 459 "ansi-c.y"
{ yyval.type = new CharType(); ;
    break;}
case 55:
#line 461 "ansi-c.y"
{ yyval.type = new IntegerType(16); ;
    break;}
case 56:
#line 463 "ansi-c.y"
{ yyval.type = new IntegerType(); ;
    break;}
case 57:
#line 465 "ansi-c.y"
{ yyval.type = new IntegerType(8, false); ;
    break;}
case 58:
#line 467 "ansi-c.y"
{ yyval.type = new IntegerType(16, false); ;
    break;}
case 59:
#line 469 "ansi-c.y"
{ yyval.type = new IntegerType(32, false); ;
    break;}
case 60:
#line 471 "ansi-c.y"
{ yyval.type = new IntegerType(); ;
    break;}
case 61:
#line 473 "ansi-c.y"
{ yyval.type = new IntegerType(64); ;
    break;}
case 62:
#line 475 "ansi-c.y"
{ yyval.type = new IntegerType(64, false); ;
    break;}
case 63:
#line 477 "ansi-c.y"
{ yyval.type = new FloatType(32); ;
    break;}
case 64:
#line 479 "ansi-c.y"
{ yyval.type = new FloatType(64); ;
    break;}
case 65:
#line 481 "ansi-c.y"
{ yyval.type = new VoidType(); ;
    break;}
case 66:
#line 483 "ansi-c.y"
{ yyval.type = new PointerType(yyvsp[-1].type); ;
    break;}
case 67:
#line 485 "ansi-c.y"
{ // This isn't C, but it makes defining pointers to arrays easier
      yyval.type = new ArrayType(yyvsp[-3].type, yyvsp[-1].ival); 
    ;
    break;}
case 68:
#line 489 "ansi-c.y"
{ // This isn't C, but it makes defining pointers to arrays easier
      yyval.type = new ArrayType(yyvsp[-2].type); 
    ;
    break;}
case 69:
#line 493 "ansi-c.y"
{ //$$ = Type::getNamedType($1); 
      //if ($$ == NULL)
      yyval.type = new NamedType(yyvsp[0].str);
    ;
    break;}
case 70:
#line 498 "ansi-c.y"
{ yyval.type = yyvsp[0].type; ;
    break;}
case 71:
#line 500 "ansi-c.y"
{
      char tmp[1024];
      sprintf(tmp, "struct %s", yyvsp[0].str);
      yyval.type = new NamedType(tmp);
    ;
    break;}
case 72:
#line 506 "ansi-c.y"
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
#line 2185 "ansi-c-parser.cpp"
#line 515 "ansi-c.y"

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


