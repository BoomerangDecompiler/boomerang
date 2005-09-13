#ifndef YY_TransformationParser_h_included
#define YY_TransformationParser_h_included

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
#line 21 "transformation-parser.h"
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

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_TransformationParser_STYPE 
#define YY_TransformationParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_TransformationParser_DEBUG
#define  YY_TransformationParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/usr/local/lib/bison.h" */
#line 114 "transformation-parser.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_TransformationParser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 121 "transformation-parser.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_TransformationParser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 128 "transformation-parser.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_TransformationParser_DEBUG */
#endif
#ifndef YY_TransformationParser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 136 "transformation-parser.h"

#line 70 "/usr/local/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_TransformationParser_PURE
extern YY_TransformationParser_STYPE YY_TransformationParser_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 214 "transformation-parser.h"
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


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/usr/local/lib/bison.h" */
#line 302 "transformation-parser.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_TransformationParser_ENUM_TOKEN { YY_TransformationParser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 354 "transformation-parser.h"
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


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_TransformationParser_PARSE(YY_TransformationParser_PARSE_PARAM);
 virtual void YY_TransformationParser_ERROR(char *msg) YY_TransformationParser_ERROR_BODY;
#ifdef YY_TransformationParser_PURE
#ifdef YY_TransformationParser_LSP_NEEDED
 virtual int  YY_TransformationParser_LEX(YY_TransformationParser_STYPE *YY_TransformationParser_LVAL,YY_TransformationParser_LTYPE *YY_TransformationParser_LLOC) YY_TransformationParser_LEX_BODY;
#else
 virtual int  YY_TransformationParser_LEX(YY_TransformationParser_STYPE *YY_TransformationParser_LVAL) YY_TransformationParser_LEX_BODY;
#endif
#else
 virtual int YY_TransformationParser_LEX() YY_TransformationParser_LEX_BODY;
 YY_TransformationParser_STYPE YY_TransformationParser_LVAL;
#ifdef YY_TransformationParser_LSP_NEEDED
 YY_TransformationParser_LTYPE YY_TransformationParser_LLOC;
#endif
 int YY_TransformationParser_NERRS;
 int YY_TransformationParser_CHAR;
#endif
#if YY_TransformationParser_DEBUG != 0
public:
 int YY_TransformationParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_TransformationParser_CLASS(YY_TransformationParser_CONSTRUCTOR_PARAM);
public:
 YY_TransformationParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_TransformationParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_TransformationParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_TransformationParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_TransformationParser_DEBUG 
#define YYDEBUG YY_TransformationParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 454 "transformation-parser.h"
#endif
