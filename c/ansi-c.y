/*==============================================================================
 * FILE:       ansi-c.y
 * OVERVIEW:   Parser for ANSI C.
 *
 *============================================================================*/
/*
 * $Revision$
 * 10 Apr 02 - Trent: Created
 * 03 Dec 02 - Trent: reduced to just parse types and signatures
 */
%name AnsiCParser

%define DEBUG 1

%define PARSE_PARAM \
    const char *sigstr

%define CONSTRUCTOR_PARAM \
    std::istream &in, bool trace

%define CONSTRUCTOR_INIT

%define CONSTRUCTOR_CODE \
    theScanner = new AnsiCScanner(in, trace); \
    if (trace) yydebug = 1; else yydebug = 0;

%define MEMBERS \
private:        \
    AnsiCScanner *theScanner; \
public: \
    std::list<Signature*> signatures;
    


%header{
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

%}
%token PREINCLUDE PREDEFINE PREIF PREIFDEF PREENDIF PRELINE
%token<str> IDENTIFIER STRING_LITERAL
%token<ival> CONSTANT 
%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%union {
   int ival;
   char *str;
   Type *type;
   std::list<Parameter*> *param_list;
   Parameter *param;
   Exp *exp;
   Signature *signature;
   TypeIdent *type_ident;
   std::list<TypeIdent*> *type_ident_list;
}

%{
#include "ansi-c-scanner.h"
%}

%type<type> type
%type<param> param
%type<param_list> param_list;
%type<type_ident> type_ident;
%type<type_ident_list> type_ident_list;

%start translation_unit
%%

translation_unit: decls 
        { }
	;

decls: decl decls
     { }
     | /* empty */
     { }
     ;

decl: type_decl
    { }
    | func_decl
    { }
    ;

param_list: param ',' param_list 
          { $$ = $3;
            $$->push_front($1);
          }
          | param
          { $$ = new std::list<Parameter*>(); 
            $$->push_back($1);
          }
          | VOID
          { $$ = new std::list<Parameter*>()}
          | /* empty */
          { $$ = new std::list<Parameter*>()}
          ;

param: type IDENTIFIER
     { $$ = new Parameter($1, $2); }
     | type '(' '*' IDENTIFIER ')' '(' param_list ')'
     { Signature *sig = Signature::instantiate(sigstr, NULL);
       sig->addReturn($1);
       for (std::list<Parameter*>::iterator it = $7->begin();
            it != $7->end(); it++)
           if (std::string((*it)->getName()) != "...")
               sig->addParameter(*it);
           else {
               sig->addEllipsis();
               delete *it;
           }
       delete $7;
       $$ = new Parameter(new PointerType(new FuncType(sig)), $4); 
     }
     | ELLIPSIS
     { $$ = new Parameter(new VoidType, "..."); }
     ;

type_decl: TYPEDEF type IDENTIFIER ';'
         { Type::addNamedType($3, $2); }
         ;

func_decl: type IDENTIFIER '(' param_list ')' ';'
         { Signature *sig = Signature::instantiate(sigstr, $2); 
           sig->addReturn($1);
           for (std::list<Parameter*>::iterator it = $4->begin();
                it != $4->end(); it++)
               if (std::string((*it)->getName()) != "...")
                   sig->addParameter(*it);
               else {
                   sig->addEllipsis();
                   delete *it;
               }
           delete $4;
           signatures.push_back(sig);
         }
         ;

type_ident: type IDENTIFIER ';'
          { $$ = new TypeIdent();
            $$->ty = $1;
            $$->nam = $2;
          }

type_ident_list: type_ident type_ident_list 
          { $$ = $2;
            $$->push_front($1);
          }
          | type_ident
          { $$ = new std::list<TypeIdent*>(); 
            $$->push_back($1);
          }
          ;

type: CHAR 
    { $$ = new CharType(); }
    | SHORT 
    { $$ = new IntegerType(16); }
    | INT 
    { $$ = new IntegerType(); }
    | UNSIGNED INT 
    { $$ = new IntegerType(32, false); }
    | LONG 
    { $$ = new IntegerType(); }
    | FLOAT 
    { $$ = new FloatType(32); }
    | DOUBLE 
    { $$ = new FloatType(64); }
    | VOID
    { $$ = new VoidType(); }
    | type '*'
    { $$ = new PointerType($1); }
    | IDENTIFIER
    { $$ = Type::getNamedType($1); 
      if ($$ == NULL)
          $$ = new NamedType($1);
    }
    | CONST type
    { $$ = $2; }
    | STRUCT '{' type_ident_list '}'
    { CompoundType *t = new CompoundType(); 
      for (std::list<TypeIdent*>::iterator it = $3->begin();
           it != $3->end(); it++) {
          t->addType((*it)->ty, (*it)->nam.c_str());
      }
      $$ = t;
    }
    ;

%%
#include <stdio.h>

int AnsiCParser::yylex()
{
    int token = theScanner->yylex(yylval);
    return token;
}

void AnsiCParser::yyerror(char *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", theScanner->column, "^", theScanner->column, s);
}



