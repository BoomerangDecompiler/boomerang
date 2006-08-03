#ifndef YY_AnsiCParser_h_included
#define YY_AnsiCParser_h_included

#line 1 "/usr/local/lib/bison.h"
/* before anything */
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
#endif
#include <stdio.h>

/* #line 14 "/usr/local/lib/bison.h" */
#line 21 "ansi-c-parser.h"
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
  
  class Bound {
  public:
      int kind;
      std::string nam;
      
      Bound(int kind, const char *nam) : kind(kind), nam(nam) { }
  };


#line 129 "ansi-c.y"
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

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_AnsiCParser_STYPE 
#define YY_AnsiCParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_AnsiCParser_DEBUG
#define  YY_AnsiCParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/usr/local/lib/bison.h" */
#line 179 "ansi-c-parser.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_AnsiCParser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 186 "ansi-c-parser.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 193 "ansi-c-parser.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_AnsiCParser_DEBUG */
#endif
#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 201 "ansi-c-parser.h"

#line 70 "/usr/local/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_AnsiCParser_PURE
extern YY_AnsiCParser_STYPE YY_AnsiCParser_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 279 "ansi-c-parser.h"
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


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/usr/local/lib/bison.h" */
#line 400 "ansi-c-parser.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 485 "ansi-c-parser.h"
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


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_AnsiCParser_PARSE(YY_AnsiCParser_PARSE_PARAM);
 virtual void YY_AnsiCParser_ERROR(char *msg) YY_AnsiCParser_ERROR_BODY;
#ifdef YY_AnsiCParser_PURE
#ifdef YY_AnsiCParser_LSP_NEEDED
 virtual int  YY_AnsiCParser_LEX(YY_AnsiCParser_STYPE *YY_AnsiCParser_LVAL,YY_AnsiCParser_LTYPE *YY_AnsiCParser_LLOC) YY_AnsiCParser_LEX_BODY;
#else
 virtual int  YY_AnsiCParser_LEX(YY_AnsiCParser_STYPE *YY_AnsiCParser_LVAL) YY_AnsiCParser_LEX_BODY;
#endif
#else
 virtual int YY_AnsiCParser_LEX() YY_AnsiCParser_LEX_BODY;
 YY_AnsiCParser_STYPE YY_AnsiCParser_LVAL;
#ifdef YY_AnsiCParser_LSP_NEEDED
 YY_AnsiCParser_LTYPE YY_AnsiCParser_LLOC;
#endif
 int YY_AnsiCParser_NERRS;
 int YY_AnsiCParser_CHAR;
#endif
#if YY_AnsiCParser_DEBUG != 0
public:
 int YY_AnsiCParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_AnsiCParser_CLASS(YY_AnsiCParser_CONSTRUCTOR_PARAM);
public:
 YY_AnsiCParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_AnsiCParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_AnsiCParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_AnsiCParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_AnsiCParser_DEBUG 
#define YYDEBUG YY_AnsiCParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 618 "ansi-c-parser.h"
#endif
