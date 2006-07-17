#ifndef YY_SSLParser_h_included
#define YY_SSLParser_h_included

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
#line 21 "sslparser.h"
#line 41 "sslparser.y"

#include "gc.h"
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
#include "statement.h"

#ifdef WIN32
#include <malloc.h>
#endif

class SSLScanner;

#line 68 "sslparser.y"
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

#line 14 "/usr/local/lib/bison.h"
 /* %{ and %header{ and %union, during decl */
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
/* WARNING obsolete !!! user defined YYLTYPE not reported into generated header */
/* use %define LTYPE */
#endif
#endif
#ifdef YYSTYPE
#ifndef YY_SSLParser_STYPE 
#define YY_SSLParser_STYPE YYSTYPE
/* WARNING obsolete !!! user defined YYSTYPE not reported into generated header */
/* use %define STYPE */
#endif
#endif
#ifdef YYDEBUG
#ifndef YY_SSLParser_DEBUG
#define  YY_SSLParser_DEBUG YYDEBUG
/* WARNING obsolete !!! user defined YYDEBUG not reported into generated header */
/* use %define DEBUG */
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

/* #line 63 "/usr/local/lib/bison.h" */
#line 179 "sslparser.h"

#line 63 "/usr/local/lib/bison.h"
/* YY_SSLParser_PURE */
#endif

/* #line 65 "/usr/local/lib/bison.h" */
#line 186 "sslparser.h"

#line 65 "/usr/local/lib/bison.h"
/* prefix */
#ifndef YY_SSLParser_DEBUG

/* #line 67 "/usr/local/lib/bison.h" */
#line 193 "sslparser.h"

#line 67 "/usr/local/lib/bison.h"
/* YY_SSLParser_DEBUG */
#endif
#ifndef YY_SSLParser_LSP_NEEDED

/* #line 70 "/usr/local/lib/bison.h" */
#line 201 "sslparser.h"

#line 70 "/usr/local/lib/bison.h"
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

/* TOKEN C */
#ifndef YY_USE_CLASS

#ifndef YY_SSLParser_PURE
extern YY_SSLParser_STYPE YY_SSLParser_LVAL;
#endif


/* #line 143 "/usr/local/lib/bison.h" */
#line 279 "sslparser.h"
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


#line 143 "/usr/local/lib/bison.h"
 /* #defines token */
/* after #define tokens, before const tokens S5*/
#else
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

/* #line 182 "/usr/local/lib/bison.h" */
#line 376 "sslparser.h"
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


#line 182 "/usr/local/lib/bison.h"
 /* decl const */
#else
enum YY_SSLParser_ENUM_TOKEN { YY_SSLParser_NULL_TOKEN=0

/* #line 185 "/usr/local/lib/bison.h" */
#line 437 "sslparser.h"
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


#line 185 "/usr/local/lib/bison.h"
 /* enum token */
     }; /* end of enum declaration */
#endif
public:
 int YY_SSLParser_PARSE(YY_SSLParser_PARSE_PARAM);
 virtual void YY_SSLParser_ERROR(char *msg) YY_SSLParser_ERROR_BODY;
#ifdef YY_SSLParser_PURE
#ifdef YY_SSLParser_LSP_NEEDED
 virtual int  YY_SSLParser_LEX(YY_SSLParser_STYPE *YY_SSLParser_LVAL,YY_SSLParser_LTYPE *YY_SSLParser_LLOC) YY_SSLParser_LEX_BODY;
#else
 virtual int  YY_SSLParser_LEX(YY_SSLParser_STYPE *YY_SSLParser_LVAL) YY_SSLParser_LEX_BODY;
#endif
#else
 virtual int YY_SSLParser_LEX() YY_SSLParser_LEX_BODY;
 YY_SSLParser_STYPE YY_SSLParser_LVAL;
#ifdef YY_SSLParser_LSP_NEEDED
 YY_SSLParser_LTYPE YY_SSLParser_LLOC;
#endif
 int YY_SSLParser_NERRS;
 int YY_SSLParser_CHAR;
#endif
#if YY_SSLParser_DEBUG != 0
public:
 int YY_SSLParser_DEBUG_FLAG;	/*  nonzero means print parse trace	*/
#endif
public:
 YY_SSLParser_CLASS(YY_SSLParser_CONSTRUCTOR_PARAM);
public:
 YY_SSLParser_MEMBERS 
};
/* other declare folow */
#endif


#if YY_SSLParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE YY_SSLParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE YY_SSLParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_SSLParser_DEBUG 
#define YYDEBUG YY_SSLParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
#line 546 "sslparser.h"
#endif
