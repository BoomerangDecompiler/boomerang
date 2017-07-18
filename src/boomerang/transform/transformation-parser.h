#pragma once

#ifndef YY_TransformationParser_h_included
#define YY_TransformationParser_h_included

// #line 1 "/usr/local/lib/bison.h"
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
#include <cstdio>

/* #line 14 "/usr/local/lib/bison.h" */
// #line 21 "transformation-parser.h"
#define YY_TransformationParser_DEBUG    1
#define YY_TransformationParser_PARSE_PARAM
#define YY_TransformationParser_CONSTRUCTOR_PARAM \
    std::istream& in, bool trace
#define YY_TransformationParser_CONSTRUCTOR_INIT
#define YY_TransformationParser_CONSTRUCTOR_CODE       \
    theScanner = new TransformationScanner(in, trace); \
    if (trace) { yydebug = 1; }                           \
    else { yydebug = 0; }

#include <list>
#include <string>
#include "boomerang/type/type.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/signature.h"
#include "boomerang/include/transformer.h"
#include "generic.h"

class TransformationScanner;

struct yy_TransformationParser_stype
{
    int     ival;
    QString str;
    Type    *type;
    Exp     *exp;
};

#define YY_TransformationParser_STYPE    yy_TransformationParser_stype

// #line 14 "/usr/local/lib/bison.h"
/* %{ and %header{ and %union, during decl */
#ifndef YY_TransformationParser_COMPATIBILITY
#define  YY_TransformationParser_COMPATIBILITY    0
#endif

/* use no goto to be clean in C++ */
#ifndef YY_TransformationParser_USE_GOTO
#define YY_TransformationParser_USE_GOTO    0
#endif

#ifndef YY_TransformationParser_DEBUG
#endif
#ifndef YY_TransformationParser_LSP_NEEDED
#endif
/* DEFAULT LTYPE*/
#ifdef YY_TransformationParser_LSP_NEEDED
#ifndef YY_TransformationParser_LTYPE
typedef
    struct yyltype
{
    int  timestamp;
    int  first_line;
    int  first_column;
    int  last_line;
    int  last_column;
    char *text;
}

yyltype;

#define YY_TransformationParser_LTYPE    yyltype
#endif
#endif
/* DEFAULT STYPE*/
#ifndef YY_TransformationParser_STYPE
#define YY_TransformationParser_STYPE    int
#endif
/* DEFAULT MISCELANEOUS */
#ifndef YY_TransformationParser_PARSE
#define YY_TransformationParser_PARSE         yyparse
#endif
#ifndef YY_TransformationParser_LEX
#define YY_TransformationParser_LEX           yylex
#endif
#ifndef YY_TransformationParser_LVAL
#define YY_TransformationParser_LVAL          yylval
#endif
#ifndef YY_TransformationParser_CHAR
#define YY_TransformationParser_CHAR          yychar
#endif
#ifndef YY_TransformationParser_NERRS
#define YY_TransformationParser_NERRS         yynerrs
#endif
#ifndef YY_TransformationParser_DEBUG_FLAG
#define YY_TransformationParser_DEBUG_FLAG    yydebug
#endif
#ifndef YY_TransformationParser_ERROR
#define YY_TransformationParser_ERROR         yyerror
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
#define YY_TransformationParser_PARSE_PARAM    void
#endif
#endif

/* TOKEN C */
#ifndef YY_USE_CLASS
#else

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
#define YY_TransformationParser_USE_CONST_TOKEN    0
/* yes enum is more compatible with flex,  */
/* so by default we use it */
#endif
#if YY_TransformationParser_USE_CONST_TOKEN != 0
#ifndef YY_TransformationParser_ENUM_TOKEN
#define YY_TransformationParser_ENUM_TOKEN    yy_TransformationParser_enum_token
#endif
#endif

class TransformationParser
{
public:
    enum YY_TransformationParser_ENUM_TOKEN
    {
        YY_TransformationParser_NULL_TOKEN = 0

/* #line 185 "/usr/local/lib/bison.h" */
// #line 354 "transformation-parser.h"
        , SIZEOF                           = 258
        , KIND                             = 259
        , POINTER                          = 260
        , COMPOUND                         = 261
        , ARRAY                            = 262
        , TYPE                             = 263
        , FUNC                             = 264
        , WHERE                            = 265
        , BECOMES                          = 266
        , REGOF                            = 267
        , MEMOF                            = 268
        , ADDROF                           = 269
        , CONSTANT                         = 270
        , IDENTIFIER                       = 271
        , STRING_LITERAL                   = 272
        , PTR_OP                           = 273
        , INC_OP                           = 274
        , DEC_OP                           = 275
        , LEFT_OP                          = 276
        , RIGHT_OP                         = 277
        , LE_OP                            = 278
        , GE_OP                            = 279
        , EQ_OP                            = 280
        , NE_OP                            = 281
        , AND_OP                           = 282
        , OR_OP                            = 283
        , MUL_ASSIGN                       = 284
        , DIV_ASSIGN                       = 285
        , MOD_ASSIGN                       = 286
        , ADD_ASSIGN                       = 287
        , SUB_ASSIGN                       = 288
        , LEFT_ASSIGN                      = 289
        , RIGHT_ASSIGN                     = 290
        , AND_ASSIGN                       = 291
        , XOR_ASSIGN                       = 292
        , OR_ASSIGN                        = 293
        , TYPE_NAME                        = 294
        , STRUCT                           = 295
        , UNION                            = 296
        , ENUM                             = 297
        , ELLIPSIS                         = 298
        , BOOL_TRUE                        = 299
        , BOOL_FALSE                       = 300


// #line 185 "/usr/local/lib/bison.h"
        /* enum token */
    };  /* end of enum declaration */

public:
    int YY_TransformationParser_PARSE(YY_TransformationParser_PARSE_PARAM);
    virtual void YY_TransformationParser_ERROR(char *msg) YY_TransformationParser_ERROR_BODY;

#ifdef YY_TransformationParser_PURE
#ifdef YY_TransformationParser_LSP_NEEDED
    virtual int YY_TransformationParser_LEX(YY_TransformationParser_STYPE *YY_TransformationParser_LVAL, YY_TransformationParser_LTYPE *yylloc) YY_TransformationParser_LEX_BODY;

#else
    virtual int YY_TransformationParser_LEX(YY_TransformationParser_STYPE *YY_TransformationParser_LVAL) YY_TransformationParser_LEX_BODY;
#endif
#else
    virtual int YY_TransformationParser_LEX() YY_TransformationParser_LEX_BODY;

    YY_TransformationParser_STYPE YY_TransformationParser_LVAL;
#ifdef YY_TransformationParser_LSP_NEEDED
    YY_TransformationParser_LTYPE yylloc;
#endif
    int YY_TransformationParser_NERRS;
    int YY_TransformationParser_CHAR;
#endif
#if YY_TransformationParser_DEBUG != 0

public:
    int YY_TransformationParser_DEBUG_FLAG; /*  nonzero means print parse trace    */
#endif

public:
    TransformationParser(YY_TransformationParser_CONSTRUCTOR_PARAM);

public:

private:
    TransformationScanner *theScanner;

public:
    virtual ~TransformationParser();
};

/* other declare folow */
#endif


#if YY_TransformationParser_COMPATIBILITY != 0
/* backward compatibility */
#ifndef YYSTYPE
#define YYSTYPE    YY_TransformationParser_STYPE
#endif

#ifndef YYLTYPE
#define YYLTYPE    YY_TransformationParser_LTYPE
#endif
#ifndef YYDEBUG
#ifdef YY_TransformationParser_DEBUG
#define YYDEBUG    YY_TransformationParser_DEBUG
#endif
#endif

#endif
/* END */

/* #line 236 "/usr/local/lib/bison.h" */
// #line 454 "transformation-parser.h"
#endif
