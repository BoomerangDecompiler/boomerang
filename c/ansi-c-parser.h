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
#line 36 "ansi-c.y"

  #include <list>
  #include <string>
  #include "exp.h"
  #include "type.h"
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
#line 111 "ansi-c-parser.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_AnsiCParser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 118 "ansi-c-parser.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 125 "ansi-c-parser.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_AnsiCParser_DEBUG */
#endif
#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 133 "ansi-c-parser.h"

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
#line 211 "ansi-c-parser.h"
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
#line 320 "ansi-c-parser.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 393 "ansi-c-parser.h"
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
#line 514 "ansi-c-parser.h"
#endif
