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

struct TypeIdent
{
    SharedType ty;
    QString name;
};

struct SymbolMods
{
    bool noDecode = false;
    bool incomplete = false;
};

struct Symbol
{
    Address addr;
    QString name;
    SharedType ty = nullptr;
    std::shared_ptr<Signature> sig = nullptr;

    SymbolMods mods;

    Symbol(Address a)
        : addr(a)
        , name("")
    {}
};

struct CustomOptions
{
    SharedExp exp = nullptr;
    int sp = 0;
};

struct SymbolRef
{
    Address addr;
    QString name;

    SymbolRef(Address a, const QString &_name)
        : addr(a)
        , name(_name)
    {}
};

struct Bound
{
    int kind;
    QString name;

    Bound(int _kind, const QString &_name)
        : kind(_kind)
        , name(_name)
    {}
};

}

// The parsing context.
%param { AnsiCParserDriver& drv }
%locations
%define parse.trace
%define parse.error verbose
%code {

#include "AnsiCParserDriver.h"

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

// keywords
%token KW_TYPEDEF
%token KW_CHAR KW_SHORT KW_INT KW_LONG KW_SIGNED KW_UNSIGNED KW_FLOAT KW_DOUBLE KW_CONST KW_VOID
%token KW_STRUCT KW_UNION KW_ENUM
%token KW_PREFER

%token KW_NODECODE
%token KW_INCOMPLETE
%token KW_SYMBOLREF
%token KW_WITHSTACK
%token KW_CDECL KW_PASCAL KW_THISCALL KW_CUSTOM

%token REGOF MEMOF MAXBOUND

%token ELLIPSIS
%token PLUS MINUS

%token SEMICOLON COMMA COLON STAR
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET

%token<QString> IDENTIFIER STRING_LITERAL
%token<int> CONSTANT


%type<SharedType> type
%type<std::shared_ptr<Parameter>> param param_exp
%type<SharedExp> exp
%type<std::shared_ptr<Bound>> optional_bound;
%type<std::shared_ptr<CustomOptions>> custom_options
%type<std::shared_ptr<std::list<std::shared_ptr<Parameter>>>> param_list;
%type<std::shared_ptr<std::list<int>>> num_list;
%type<std::shared_ptr<TypeIdent>> type_ident;
%type<std::shared_ptr<std::list<std::shared_ptr<TypeIdent>>>> type_ident_list;
%type<std::shared_ptr<Signature>> signature;
%type<SymbolMods> symbol_mods;
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
    KW_TYPEDEF type_ident SEMICOLON {
        Type::addNamedType($2->name, $2->ty);
    }
  | KW_TYPEDEF type LPAREN STAR IDENTIFIER RPAREN LPAREN param_list RPAREN SEMICOLON {
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

        Type::addNamedType($5, PointerType::get(FuncType::get(sig)));
    }
  | KW_TYPEDEF type_ident LPAREN param_list RPAREN SEMICOLON  {
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, drv.cc, $2->name);
        sig->addReturn($2->ty);

        for (std::shared_ptr<Parameter> &param : *$4) {
            if (param->getName() != "...")
                sig->addParameter(param);
            else {
                sig->setHasEllipsis(true);
            }
        }

        Type::addNamedType($2->name, FuncType::get(sig));
    }
  | KW_STRUCT IDENTIFIER LBRACE type_ident_list RBRACE SEMICOLON {
        std::shared_ptr<CompoundType> ty = CompoundType::get();
        for (std::shared_ptr<TypeIdent> &ti : *$4) {
            ty->addMember(ti->ty, ti->name);
        }

        Type::addNamedType(QString("struct ") + $2, ty);
    }
  ;

type_ident:
    type IDENTIFIER {
        $$.reset(new TypeIdent);
        $$->ty = $1;
        $$->name = $2;
    }
  | type IDENTIFIER array_modifier {
        std::shared_ptr<ArrayType> arrayTy = $3->as<ArrayType>();
        SharedType baseType = arrayTy->getBaseType();

        while (baseType && baseType->isArray()) {
            arrayTy = baseType->as<ArrayType>();
            baseType = arrayTy->getBaseType();
        }

        assert(baseType == nullptr);
        arrayTy->setBaseType($1);

        $$.reset(new TypeIdent);
        $$->ty = $3;
        $$->name = $2;
    }
  ;

type:
    KW_CHAR                             { $$ = CharType::get(); }
  | KW_UNSIGNED KW_CHAR                 { $$ = IntegerType::get( 8, Sign::Unsigned); }
  | KW_SHORT                            { $$ = IntegerType::get(16, Sign::Signed);   }
  | KW_UNSIGNED KW_SHORT                { $$ = IntegerType::get(16, Sign::Unsigned); }
  | KW_INT                              { $$ = IntegerType::get(32, Sign::Signed);   }
  | KW_UNSIGNED KW_INT                  { $$ = IntegerType::get(32, Sign::Unsigned); }
  | KW_UNSIGNED                         { $$ = IntegerType::get(32, Sign::Unsigned); }
  | KW_LONG                             { $$ = IntegerType::get(32, Sign::Signed);   }
  | KW_LONG KW_INT                      { $$ = IntegerType::get(32, Sign::Signed);   }
  | KW_UNSIGNED KW_LONG                 { $$ = IntegerType::get(32, Sign::Unsigned); }
  | KW_UNSIGNED KW_LONG KW_INT          { $$ = IntegerType::get(32, Sign::Unsigned); }
  | KW_LONG KW_LONG                     { $$ = IntegerType::get(64, Sign::Signed);   }
  | KW_LONG KW_LONG KW_INT              { $$ = IntegerType::get(64, Sign::Signed);   }
  | KW_UNSIGNED KW_LONG KW_LONG         { $$ = IntegerType::get(64, Sign::Unsigned); }
  | KW_UNSIGNED KW_LONG KW_LONG KW_INT  { $$ = IntegerType::get(64, Sign::Unsigned); }
  | KW_FLOAT                            { $$ = FloatType::get(32); }
  | KW_DOUBLE                           { $$ = FloatType::get(64); }
  | KW_VOID                             { $$ = VoidType::get(); }
  | type STAR                           { $$ = PointerType::get($1); }
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
  | KW_CONST type { $$ = $2; }
  | KW_STRUCT IDENTIFIER {
        $$ = NamedType::get(QString("struct ") + $2);
    }
  | KW_STRUCT LBRACE type_ident_list RBRACE {
        std::shared_ptr<CompoundType> ty = CompoundType::get();
        for (std::shared_ptr<TypeIdent> &ti : *$3) {
            ty->addMember(ti->ty, ti->name);
        }
        $$ = ty;
    }
  ;

type_ident_list:
    type_ident SEMICOLON type_ident_list  { $$ = $3; $$->push_front($1); }
  | type_ident SEMICOLON {
        $$.reset(new std::list<std::shared_ptr<TypeIdent>>());
        $$->push_back($1);
    }
  ;

array_modifier:
    LBRACKET CONSTANT RBRACKET                  { $$ = ArrayType::get(NULL, $2); }
  | LBRACKET RBRACKET                           { $$ = ArrayType::get(NULL); }
  | array_modifier LBRACKET CONSTANT RBRACKET   { $$ = ArrayType::get($1, $3); }
  | array_modifier LBRACKET RBRACKET            { $$ = ArrayType::get($1); }
  ;

param_list:
    param_exp COMMA param_list    { $$ = $3;  $$->push_front($1); }
  | param_exp                     { $$.reset(new std::list<std::shared_ptr<Parameter>>()); $$->push_back($1); }
  | KW_VOID                       { $$.reset(new std::list<std::shared_ptr<Parameter>>()); }
  | %empty                        { $$.reset(new std::list<std::shared_ptr<Parameter>>()); }
  ;

param_exp:
    exp COLON param     { $$ = $3; $$->setExp($1); }
  | param               { $$ = $1; }
  ;

exp:
    REGOF LBRACKET CONSTANT RBRACKET { $$ = Location::regOf($3); }
  | MEMOF LBRACKET exp RBRACKET      { $$ = Location::memOf($3);  }
  | exp PLUS exp                     { $$ = Binary::get(opPlus, $1, $3); }
  | exp MINUS exp                    { $$ = Binary::get(opMinus, $1, $3); }
  | CONSTANT                         { $$ = Const::get($1); }
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

        $$.reset(new Parameter(PointerType::get(FuncType::get(sig)), $4));
    }
  | ELLIPSIS { $$.reset(new Parameter(VoidType::get(), "...")); }
  ;

optional_bound:
    MAXBOUND LPAREN IDENTIFIER RPAREN  { $$.reset(new Bound(0, $3)); }
 |  %empty                             { $$ = nullptr; }
 ;

func_decl:
    signature SEMICOLON {
        drv.signatures.push_back($1);
    }
  | signature KW_PREFER type_ident LPAREN num_list RPAREN SEMICOLON {
        $1->setPreferredName($3->name);
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

        $$ = sig;
    }
  | convention type_ident LPAREN param_list RPAREN {
        std::shared_ptr<Signature> sig = Signature::instantiate(drv.plat, $1, $2->name);
        sig->addReturn($2->ty);

        for (std::shared_ptr<Parameter> &param : *$4) {
            if (param->getName() != "...") {
                sig->addParameter(param);
            }
            else {
                sig->setHasEllipsis(true);
            }
        }

        $$ = sig;
    }
  | KW_CUSTOM custom_options type_ident LPAREN param_list RPAREN {
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

        $$ = sig;
    }
  ;

convention:
    KW_CDECL       { $$ = CallConv::C; }
  | KW_PASCAL      { $$ = CallConv::Pascal; }
  | KW_THISCALL    { $$ = CallConv::ThisCall; }
  ;

num_list:
    CONSTANT COMMA num_list   { $$ = $3;  $$->push_front($1); }
  | CONSTANT                  { $$.reset(new std::list<int>()); $$->push_back($1); }
  | %empty                    { $$.reset(new std::list<int>()); }
  ;

custom_options:
    exp COLON                           { $$.reset(new CustomOptions()); $$->exp = $1; }
  | KW_WITHSTACK LPAREN CONSTANT RPAREN { $$.reset(new CustomOptions()); $$->sp = $3; }
  | %empty                              { $$.reset(new CustomOptions()); }
  ;

symbol_decl:
    CONSTANT type_ident SEMICOLON {
        std::shared_ptr<Symbol> sym(new Symbol(Address($1)));
        sym->name = $2->name;
        sym->ty = $2->ty;
        drv.symbols.push_back(sym);
    }
    // Note: in practice, a function signature needs either a "symbolmods"
    // (__nodecode or __incomplete), or a calling convention
    // (__cdecl, __pascal, __thiscall, etc). This is because of the one-symbol
    // lookahead limitation; the parser can't distinguish 123 int foo from 123 int foo()
  | CONSTANT symbol_mods signature SEMICOLON {
        std::shared_ptr<Symbol> sym(new Symbol(Address($1)));
        sym->sig = $3;
        sym->mods = $2;
        drv.symbols.push_back(sym);
    }
  ;

symbol_mods:
    KW_NODECODE symbol_mods   { $$ = $2; $$.noDecode = true; }
  | KW_INCOMPLETE symbol_mods { $$ = $2; $$.incomplete = true; }
  | %empty                 { }
  ;

symbol_ref_decl:
    KW_SYMBOLREF CONSTANT IDENTIFIER SEMICOLON {
        std::shared_ptr<SymbolRef> ref(new SymbolRef(Address($2), $3));
        drv.refs.push_back(ref);
    }
  ;

%%

void AnsiC::parser::error(const AnsiC::parser::location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
