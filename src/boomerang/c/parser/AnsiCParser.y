/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace {::AnsiC}
%name-prefix "AnsiC"

%code requires {

#include "boomerang/ssl/type/Type.h"
#include "boomerang/util/Address.h"
#include "boomerang/db/signature/Signature.h"

class AnsiCParserDriver;



  class TypeIdent {
  public:
      SharedType ty;
      QString name;
  };

  class SymbolMods;

  class Symbol {
  public:
      Address addr;
      QString name;
      SharedType ty;
      std::shared_ptr<Signature> sig;
      SymbolMods *mods;

      Symbol(Address a)
        : addr(a), name(""), ty(NULL), sig(NULL),
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
      SharedExp exp;
      int sp;

      CustomOptions() : exp(NULL), sp(0) { }
  };

  class SymbolRef {
  public:
      Address addr;
      QString name;

      SymbolRef(Address a, const QString &_name) : addr(a), name(_name) { }
  };

  class Bound {
  public:
      int kind;
      QString name;

      Bound(int kind, const QString &_name) : kind(kind), name(_name) { }
  };

}

// The parsing context.
%param { AnsiCParserDriver& drv }
%locations
%define parse.trace
%define parse.error verbose
%code {

#include "boomerang/c/parser/AnsiCParserDriver.h"
#include "boomerang/db/signature/CustomSignature.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/NamedType.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/FuncType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/ssl/type/FloatType.h"

}

%define api.token.prefix {TOK_}
%token
  END  0    "end of file"
;

%token PREINCLUDE PREDEFINE PREIF PREIFDEF PREENDIF PRELINE
%token<QString> IDENTIFIER STRING_LITERAL
%token<int> CONSTANT
%token SIZEOF
%token NODECODE
%token INCOMPLETE
%token SYMBOLREF
%token CDECL PASCAL THISCALL
%token REGOF
%token MEMOF
%token MAXBOUND
%token CUSTOM PREFER
%token WITHSTACK
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token SEMICOLON COMMA COLON ASSIGN_OP DOT AND NOT BIT_NOT MINUS PLUS STAR DIV MOD
%token LESS GTR XOR BIT_OR QUESTION
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET


%type<SharedType> type
%type<std::shared_ptr<Parameter>> param
%type<std::shared_ptr<Parameter>> param_exp
%type<SharedExp> exp
%type<Bound *> optional_bound;
%type<CustomOptions *> custom_options
%type<std::list<std::shared_ptr<Parameter>> *> param_list;
%type<std::list<int> *> num_list;
%type<TypeIdent *> type_ident;
%type<std::list<TypeIdent*> *> type_ident_list;
%type<std::shared_ptr<Signature>> signature;
%type<SymbolMods *> symbol_mods;
%type<SharedType> array_modifier;
%type<CallConv> convention;

%start translation_unit

%%

translation_unit:
    decls
  ;

decls:
    decl decls
  | %empty
  ;

decl:
    type_decl
  | func_decl
  | symbol_decl
  | symbol_ref_decl
  ;

type_decl:
    TYPEDEF type_ident SEMICOLON {
        Type::addNamedType($2->name, $2->ty);
    }
  | TYPEDEF type LPAREN STAR IDENTIFIER RPAREN LPAREN param_list RPAREN SEMICOLON {
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, drv.cc, NULL);
        sig->addReturn($2);

        for (std::shared_ptr<Parameter> &param : *$8) {
            if (param->getName() != "...") {
                sig->addParameter(param);
            }
            else {
                sig->setHasEllipsis(true);
            }
        }

        delete $8;
        Type::addNamedType($5, PointerType::get(FuncType::get(sig)));
    }
  | TYPEDEF type_ident LPAREN param_list RPAREN SEMICOLON  {
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, drv.cc, $2->name);
        sig->addReturn($2->ty);

        for (std::shared_ptr<Parameter> &param : *$4) {
            if (param->getName() != "...")
                sig->addParameter(param);
            else {
                sig->setHasEllipsis(true);
            }
        }

        delete $4;
        Type::addNamedType($2->name, FuncType::get(sig));
    }
  | STRUCT IDENTIFIER LBRACE type_ident_list RBRACE SEMICOLON {
        std::shared_ptr<CompoundType> ty = CompoundType::get();
        for (TypeIdent *ti : *$4) {
            ty->addMember(ti->ty, ti->name);
        }

        Type::addNamedType(QString("struct ") + $2, ty);
    }
  ;

type_ident:
    type IDENTIFIER {
        $$ = new TypeIdent();
        $$->ty = $1;
        $$->name = $2;
    }
  | type IDENTIFIER array_modifier {
        $$ = new TypeIdent();
        $3->as<ArrayType>()->fixBaseType($1);
        $$->ty = $3;
        $$->name = $2;
    }
  ;

type:
    CHAR                { $$ = CharType::get(); }
  | SHORT               { $$ = IntegerType::get(16, Sign::Signed);   }
  | INT                 { $$ = IntegerType::get(32, Sign::Signed);   }
  | UNSIGNED CHAR       { $$ = IntegerType::get( 8, Sign::Unsigned); }
  | UNSIGNED SHORT      { $$ = IntegerType::get(16, Sign::Unsigned); }
  | UNSIGNED INT        { $$ = IntegerType::get(32, Sign::Unsigned); }
  | UNSIGNED LONG       { $$ = IntegerType::get(32, Sign::Unsigned); }
  | UNSIGNED            { $$ = IntegerType::get(32, Sign::Unsigned); }
  | LONG                { $$ = IntegerType::get(32, Sign::Signed);   }
  | LONG LONG           { $$ = IntegerType::get(64, Sign::Signed);   }
  | UNSIGNED LONG LONG  { $$ = IntegerType::get(64, Sign::Unsigned); }
  | FLOAT               { $$ = FloatType::get(32); }
  | DOUBLE              { $$ = FloatType::get(64); }
  | VOID                { $$ = VoidType::get(); }
  | type STAR           { $$ = PointerType::get($1); }
  | type LBRACKET CONSTANT RBRACKET {
        // This isn't C, but it makes defining pointers to arrays easier
        $$ = ArrayType::get($1, $3);
    }
  | type LBRACKET RBRACKET {
        // This isn't C, but it makes defining pointers to arrays easier
        $$ = ArrayType::get($1);
    }
  | IDENTIFIER {

        $$ = NamedType::get($1);
    }
  | CONST type { $$ = $2; }
  | STRUCT IDENTIFIER {
        $$ = NamedType::get(QString("struct ") + $2);
    }
  | STRUCT LBRACE type_ident_list RBRACE {
        std::shared_ptr<CompoundType> ty = CompoundType::get();
        for (TypeIdent *ti : *$3) {
            ty->addMember(ti->ty, ti->name);
        }
        $$ = ty;
    }
  ;

type_ident_list:
    type_ident SEMICOLON type_ident_list  { $$ = $3; $$->push_front($1); }
  | type_ident SEMICOLON                  { $$ = new std::list<TypeIdent *>(); $$->push_back($1); }
  ;

array_modifier:
    LBRACKET CONSTANT RBRACKET                  { $$ = ArrayType::get(NULL, $2); }
  | LBRACKET RBRACKET                           { $$ = ArrayType::get(NULL); }
  | array_modifier LBRACKET CONSTANT RBRACKET   { $$ = ArrayType::get($1, $3); }
  | array_modifier LBRACKET RBRACKET            { $$ = ArrayType::get($1); }
  ;

param_list:
    param_exp COMMA param_list    { $$ = $3;  $$->push_front($1); }
  | param_exp                     { $$ = new std::list<std::shared_ptr<Parameter>>(); $$->push_back($1); }
  | VOID                          { $$ = new std::list<std::shared_ptr<Parameter>>(); }
  | %empty                        { $$ = new std::list<std::shared_ptr<Parameter>>(); }
  ;

param_exp:
    exp COLON param     { $$ = $3; $$->setExp($1); }
  | param               { $$ = $1; }
  ;

exp:
    REGOF CONSTANT RBRACE   { $$ = Location::regOf($2); }
  | MEMOF exp RBRACE        { $$ = Location::memOf($2);  }
  | exp PLUS exp            { $$ = Binary::get(opPlus, $1, $3); }
  | exp MINUS exp           { $$ = Binary::get(opMinus, $1, $3); }
  | CONSTANT                { $$ = Const::get($1); }
  ;

param:
    type_ident optional_bound {
        if ($1->ty->resolvesToArray()) {
            /* C has complex semantics for passing arrays.. seeing as
             * we're supposedly parsing C, then we should deal with this.
             * When you pass an array in C it is understood that you are
             * passing that array "by reference". As all parameters in
             * our internal representation are passed "by value", we alter
             * the type here to be a pointer to an array.
             */
            $1->ty = PointerType::get($1->ty);
        }

        $$.reset(new Parameter($1->ty, $1->name));
        if ($2) {
            switch($2->kind) {
                case 0: $$->setBoundMax($2->name);
            }
        }
     }
  | type LPAREN STAR IDENTIFIER RPAREN LPAREN param_list RPAREN {
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, drv.cc, NULL);
        sig->addReturn($1);

        for (std::shared_ptr<Parameter> &param : *$7) {
            if (param->getName() != "...") {
                sig->addParameter(param);
            }
            else {
                sig->setHasEllipsis(true);
            }
        }

        delete $7;
        $$.reset(new Parameter(PointerType::get(FuncType::get(sig)), $4));
    }
  | ELLIPSIS { $$.reset(new Parameter(VoidType::get(), "...")); }
  ;

optional_bound:
    MAXBOUND IDENTIFIER RPAREN  { $$ = new Bound(0, $2); }
 |  %empty                      { $$ = nullptr; }
 ;

func_decl:
    signature SEMICOLON {
        drv.signatures.push_back($1);
    }
  | signature PREFER type_ident LPAREN num_list RPAREN SEMICOLON {
        $1->setPreferredName($3->name);

        delete $5;
        drv.signatures.push_back($1);
    }
  ;

signature:
    type_ident LPAREN param_list RPAREN {
        /* Use the passed calling convention (cc) */
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, drv.cc, $1->name);
        sig->addReturn($1->ty);

        for (std::shared_ptr<Parameter> &param : *$3) {
            if (param->getName() != "...") {
                sig->addParameter(param);
            }
            else {
                sig->setHasEllipsis(true);
            }
        }

        delete $3;
        $$ = sig;
    }
  | convention type_ident LPAREN param_list RPAREN {
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, $1, $2->name);
        sig->addReturn($2->ty);

        for (std::shared_ptr<Parameter> &param : *$4) {
            if (param->getName() != "...")
                sig->addParameter(param);
            else {
                sig->setHasEllipsis(true);
            }
        }

        delete $4;
        $$ = sig;
    }
  | CUSTOM custom_options type_ident LPAREN param_list RPAREN {
        std::shared_ptr<CustomSignature> sig = std::make_shared<CustomSignature>($3->name);
        if ($2->exp) {
            sig->addReturn($3->ty, $2->exp);
        }

        if ($2->sp) {
            sig->setSP($2->sp);
        }

        for (std::shared_ptr<Parameter> &param : *$5) {
            if (param->getName() != "...") {
                sig->addParameter(param);
            }
            else {
                sig->setHasEllipsis(true);
            }
        }

        delete $5;
        $$ = sig;
    }
  ;

convention:
    CDECL       { $$ = CallConv::C; }
  | PASCAL      { $$ = CallConv::Pascal; }
  | THISCALL    { $$ = CallConv::ThisCall; }
  ;

num_list:
    CONSTANT COMMA num_list   { $$ = $3;  $$->push_front($1); }
  | CONSTANT                  { $$ = new std::list<int>(); $$->push_back($1); }
  | %empty                    { $$ = new std::list<int>(); }
  ;

custom_options:
    exp COLON                   { $$ = new CustomOptions(); $$->exp = $1; }
  | WITHSTACK CONSTANT RPAREN   { $$ = new CustomOptions(); $$->sp = $2; }
  | %empty                      { $$ = new CustomOptions(); }
  ;

symbol_decl:
    CONSTANT type_ident SEMICOLON {
        Symbol *sym = new Symbol(Address($1));
        sym->name = $2->name;
        sym->ty = $2->ty;
        drv.symbols.push_back(sym);
    }
    // Note: in practice, a function signature needs either a "symbolmods"
    // (__nodecode or __incomplete), or a calling convention
    // (__cdecl, __pascal, __thiscall, etc). This is because of the one-symbol
    // lookahead limitation; the parser can't distinguish 123 int foo from 123 int foo()
  | CONSTANT symbol_mods signature SEMICOLON {
        Symbol *sym = new Symbol(Address($1));
        sym->sig = $3;
        sym->mods = $2;
        drv.symbols.push_back(sym);
    }
  ;

symbol_mods:
    NODECODE symbol_mods   { $$ = $2; $$->noDecode = true; }
  | INCOMPLETE symbol_mods { $$ = $2; $$->incomplete = true; }
  | %empty                 { $$ = new SymbolMods(); }
  ;

symbol_ref_decl:
    SYMBOLREF CONSTANT IDENTIFIER SEMICOLON {
        SymbolRef *ref = new SymbolRef(Address($2), $3);
        drv.refs.push_back(ref);
    }
  ;

%%

void AnsiC::parser::error(const AnsiC::parser::location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
