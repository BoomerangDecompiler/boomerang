#ifndef YY_AnsiCParser_h_included
#define YY_AnsiCParser_h_included

#line 1 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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

/* #line 14 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
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
    std::list<Signature*> signatures; \
    std::list<Symbol*> symbols; \
    std::list<SymbolRef*> refs;
#line 36 "ansi-c.y"

  #include <list>
  #include <string>
  #include "exp.h"
  #include "type.h"
  #include "cfg.h"
  #include "proc.h"
  #include "signature.h"
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

  class SymbolRef {
  public:
      ADDRESS addr;
      std::string nam;

      SymbolRef(ADDRESS a, const char *nam) : addr(a), nam(nam) { }
  };


#line 102 "ansi-c.y"
typedef union {
   int ival;
   char *str;
   Type *type;
   std::list<Parameter*> *param_list;
   Parameter *param;
   Exp *exp;
   Signature *sig;
   TypeIdent *type_ident;
   std::list<TypeIdent*> *type_ident_list;
   SymbolMods *mods;
} yy_AnsiCParser_stype;
#define YY_AnsiCParser_STYPE yy_AnsiCParser_stype

#line 14 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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

/* #line 63 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 153 "ansi-c-parser.h"

#line 63 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* YY_AnsiCParser_PURE */
#endif

/* #line 65 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 160 "ansi-c-parser.h"

#line 65 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* prefix */
#ifndef YY_AnsiCParser_DEBUG

/* #line 67 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 167 "ansi-c-parser.h"

#line 67 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
/* YY_AnsiCParser_DEBUG */
#endif
#ifndef YY_AnsiCParser_LSP_NEEDED

/* #line 70 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 175 "ansi-c-parser.h"

#line 70 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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


/* #line 143 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 253 "ansi-c-parser.h"
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
#define	PTR_OP	272
#define	INC_OP	273
#define	DEC_OP	274
#define	LEFT_OP	275
#define	RIGHT_OP	276
#define	LE_OP	277
#define	GE_OP	278
#define	EQ_OP	279
#define	NE_OP	280
#define	AND_OP	281
#define	OR_OP	282
#define	MUL_ASSIGN	283
#define	DIV_ASSIGN	284
#define	MOD_ASSIGN	285
#define	ADD_ASSIGN	286
#define	SUB_ASSIGN	287
#define	LEFT_ASSIGN	288
#define	RIGHT_ASSIGN	289
#define	AND_ASSIGN	290
#define	XOR_ASSIGN	291
#define	OR_ASSIGN	292
#define	TYPE_NAME	293
#define	TYPEDEF	294
#define	EXTERN	295
#define	STATIC	296
#define	AUTO	297
#define	REGISTER	298
#define	CHAR	299
#define	SHORT	300
#define	INT	301
#define	LONG	302
#define	SIGNED	303
#define	UNSIGNED	304
#define	FLOAT	305
#define	DOUBLE	306
#define	CONST	307
#define	VOLATILE	308
#define	VOID	309
#define	STRUCT	310
#define	UNION	311
#define	ENUM	312
#define	ELLIPSIS	313
#define	CASE	314
#define	DEFAULT	315
#define	IF	316
#define	ELSE	317
#define	SWITCH	318
#define	WHILE	319
#define	DO	320
#define	FOR	321
#define	GOTO	322
#define	CONTINUE	323
#define	BREAK	324
#define	RETURN	325


#line 143 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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

/* #line 182 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 366 "ansi-c-parser.h"
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


#line 182 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
 /* decl const */
#else
enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

/* #line 185 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 443 "ansi-c-parser.h"
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
	,PTR_OP=272
	,INC_OP=273
	,DEC_OP=274
	,LEFT_OP=275
	,RIGHT_OP=276
	,LE_OP=277
	,GE_OP=278
	,EQ_OP=279
	,NE_OP=280
	,AND_OP=281
	,OR_OP=282
	,MUL_ASSIGN=283
	,DIV_ASSIGN=284
	,MOD_ASSIGN=285
	,ADD_ASSIGN=286
	,SUB_ASSIGN=287
	,LEFT_ASSIGN=288
	,RIGHT_ASSIGN=289
	,AND_ASSIGN=290
	,XOR_ASSIGN=291
	,OR_ASSIGN=292
	,TYPE_NAME=293
	,TYPEDEF=294
	,EXTERN=295
	,STATIC=296
	,AUTO=297
	,REGISTER=298
	,CHAR=299
	,SHORT=300
	,INT=301
	,LONG=302
	,SIGNED=303
	,UNSIGNED=304
	,FLOAT=305
	,DOUBLE=306
	,CONST=307
	,VOLATILE=308
	,VOID=309
	,STRUCT=310
	,UNION=311
	,ENUM=312
	,ELLIPSIS=313
	,CASE=314
	,DEFAULT=315
	,IF=316
	,ELSE=317
	,SWITCH=318
	,WHILE=319
	,DO=320
	,FOR=321
	,GOTO=322
	,CONTINUE=323
	,BREAK=324
	,RETURN=325


#line 185 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h"
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

/* #line 236 "/home/02/binary/u1.luna.tools/bison++/lib/bison.h" */
#line 568 "ansi-c-parser.h"
#endif
