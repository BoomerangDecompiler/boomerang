#pragma once

#define YY_USE_CLASS
#include <stdio.h>
#define YY_AnsiCParser_DEBUG  1

#include <list>
#include <memory>
#include <string>
#include "exp.h"
#include "type.h"
#include "cfg.h"
#include "proc.h"
#include "signature.h"
#include "util.h"

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

    Symbol(ADDRESS a) : addr(a), nam(""), ty(nullptr), sig(nullptr),
        mods(nullptr) { }
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

    CustomOptions() : exp(nullptr), sp(0) { }
};

class SymbolRef {
public:
    ADDRESS addr;
    std::string nam;

    SymbolRef(ADDRESS a, const char *_nam) : addr(a), nam(_nam) { }
};

class Bound {
public:
    int kind;
    std::string nam;

    Bound(int _kind, const char *_nam) : kind(_kind), nam(_nam) { }
};


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

/* prefix */
class AnsiCParser
{
public:
    enum YY_AnsiCParser_ENUM_TOKEN { YY_AnsiCParser_NULL_TOKEN=0

                                     /* //#line 185 "/usr/local/lib/bison.h" */
                                     //#line 485 "ansi-c-parser.h"
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


                                     //#line 185 "/usr/local/lib/bison.h"
                                     /* enum token */
                                   }; /* end of enum declaration */
public:
    int yyparse(platform plat, callconv cc);
    virtual void yyerror(const char *msg);
    virtual int yylex() ;
    yy_AnsiCParser_stype yylval;
    int yynerrs;
    int yychar;
#if YY_AnsiCParser_DEBUG != 0
public:
    int yydebug;    /*  nonzero means print parse trace    */
#endif
public:
    AnsiCParser(std::istream &in, bool trace);
public:
private:
    AnsiCScanner *theScanner;
public:
    std::list<Signature*> signatures;
    std::list<Symbol*> symbols;
    std::list<SymbolRef*> refs;
    virtual ~AnsiCParser();
};
