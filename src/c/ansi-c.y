/*==============================================================================
 * FILE:	   ansi-c.y
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
	platform plat, callconv cc

%define CONSTRUCTOR_PARAM \
	std::istream &in, bool trace

%define CONSTRUCTOR_INIT

%define CONSTRUCTOR_CODE \
	theScanner = new AnsiCScanner(in, trace); \
	if (trace) yydebug = 1; else yydebug = 0;

%define MEMBERS \
private:		\
	AnsiCScanner *theScanner; \
public: \
	std::list<Signature*> signatures; \
	std::list<Symbol*> symbols; \
	std::list<SymbolRef*> refs;\
	virtual ~AnsiCParser();


%header{
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

%}
%token PREINCLUDE PREDEFINE PREIF PREIFDEF PREENDIF PRELINE
%token<str> IDENTIFIER STRING_LITERAL
%token<ival> CONSTANT 
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

%union {
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
}

%{
#include "ansi-c-scanner.h"
%}

%type<type> type
%type<param> param
%type<param> param_exp
%type<exp> exp
%type<bound> optional_bound;
%type<custom_options> custom_options
%type<param_list> param_list;
%type<num_list> num_list;
%type<type_ident> type_ident;
%type<type_ident_list> type_ident_list;
%type<sig> signature;
%type<mods> symbol_mods;
%type<type> array_modifier;
%type<cc> convention;

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
	| symbol_decl
	{ }
	| symbol_ref_decl
	{ }
	;

convention:
		CDECL
		{ $$ = CONV_C; }
		| PASCAL
		{ $$ = CONV_PASCAL; }
		| THISCALL
		{ $$ = CONV_THISCALL; }
		;

num_list: CONSTANT ',' num_list 
		{ $$ = $3;
		  $$->push_front($1);
		}
		| CONSTANT
		{ $$ = new std::list<int>();
		  $$->push_back($1);
		}
		| /* empty */
		{ $$ = new std::list<int>();
		}
		;

param_list: param_exp ',' param_list 
		  { $$ = $3;
			$$->push_front($1);
		  }
		  | param_exp
		  { $$ = new std::list<Parameter*>(); 
			$$->push_back($1);
		  }
		  | VOID
		  { $$ = new std::list<Parameter*>()}
		  | /* empty */
		  { $$ = new std::list<Parameter*>()}
		  ;

param_exp: exp ':' param
	{ $$ = $3;
	  $$->setExp($1);
	}
	| param
	{ $$ = $1;
	}
	;

exp: REGOF CONSTANT ']'
	{ $$ = Location::regOf($2);
	}
	| MEMOF exp ']'
	{ $$ = Location::memOf($2);
	}
	| exp '+' exp
	{ $$ = new Binary(opPlus, $1, $3);
	}
	| exp '-' exp
	{ $$ = new Binary(opMinus, $1, $3);
	}
	| CONSTANT
	{ $$ = new Const($1);
	}
	;
	
optional_bound: MAXBOUND IDENTIFIER ')'
    { $$ = new Bound(0, $2); }
    | /* */
    { }
    ;

param: type_ident optional_bound
	 {	if ($1->ty->isArray() || 
			($1->ty->isNamed() && 
			 ((NamedType*)$1->ty)->resolvesTo() &&
			 ((NamedType*)$1->ty)->resolvesTo()->isArray())) {
			/* C has complex semantics for passing arrays.. seeing as 
			 * we're supposedly parsing C, then we should deal with this.
			 * When you pass an array in C it is understood that you are
			 * passing that array "by reference".  As all parameters in
			 * our internal representation are passed "by value", we alter
			 * the type here to be a pointer to an array.
			 */
			$1->ty = new PointerType($1->ty);
		}
		$$ = new Parameter($1->ty, $1->nam.c_str());
		if ($2) {
		   switch($2->kind) {
		     case 0: $$->setBoundMax($2->nam.c_str());
		   }
		}
	 }
	 | type '(' '*' IDENTIFIER ')' '(' param_list ')'
	 { Signature *sig = Signature::instantiate(plat, cc, NULL);
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

type_decl: TYPEDEF type_ident ';'
		 { Type::addNamedType($2->nam.c_str(), $2->ty); }
		 | TYPEDEF type '(' '*' IDENTIFIER ')' '(' param_list ')' ';'
		 { Signature *sig = Signature::instantiate(plat, cc, NULL);
		   sig->addReturn($2);
		   for (std::list<Parameter*>::iterator it = $8->begin();
				it != $8->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete $8;
		   Type::addNamedType($5, new PointerType(new FuncType(sig))); 
		 }
		 | TYPEDEF type_ident '(' param_list ')' ';'
		 { Signature *sig = Signature::instantiate(plat, cc, $2->nam.c_str());
		   sig->addReturn($2->ty);
		   for (std::list<Parameter*>::iterator it = $4->begin();
				it != $4->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete $4;
		   Type::addNamedType($2->nam.c_str(), new FuncType(sig)); 
		 } 
		 | STRUCT IDENTIFIER '{' type_ident_list '}' ';'
		 { CompoundType *t = new CompoundType(); 
		   for (std::list<TypeIdent*>::iterator it = $4->begin();
				   it != $4->end(); it++) {
			  t->addType((*it)->ty, (*it)->nam.c_str());
		   }
		   char tmp[1024];
		   sprintf(tmp, "struct %s", $2);
		   Type::addNamedType(tmp, t); 
		 }
		 ;

func_decl: signature ';'
		 {
		   signatures.push_back($1);
		 }
		 | signature PREFER type_ident '(' num_list ')' ';'
		 {
		   $1->setPreferedReturn($3->ty);
		   $1->setPreferedName($3->nam.c_str());
		   for (std::list<int>::iterator it = $5->begin();
				it != $5->end(); it++)
			   $1->addPreferedParameter(*it - 1);
		   delete $5;
		   signatures.push_back($1);
		 }
		 ;

signature: type_ident '(' param_list ')'
		 { 
		   /* Use the passed calling convention (cc) */
		   Signature *sig = Signature::instantiate(plat, cc, $1->nam.c_str()); 
		   sig->addReturn($1->ty);
		   for (std::list<Parameter*>::iterator it = $3->begin();
				it != $3->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete $3;
		   $$ = sig;
		 }
		 | convention type_ident '(' param_list ')'
		 { Signature *sig = Signature::instantiate(plat, $1,
			  $2->nam.c_str()); 
		   sig->addReturn($2->ty);
		   for (std::list<Parameter*>::iterator it = $4->begin();
				it != $4->end(); it++)
			   if (std::string((*it)->getName()) != "...")
				   sig->addParameter(*it);
			   else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete $4;
		   $$ = sig;
		 }
		 | CUSTOM custom_options type_ident '(' param_list ')'
		 { CustomSignature *sig = new CustomSignature($3->nam.c_str()); 
		   if ($2->exp)
			   sig->addReturn($3->ty, $2->exp);
		   if ($2->sp)
			   sig->setSP($2->sp);
		   for (std::list<Parameter*>::iterator it = $5->begin();
				it != $5->end(); it++)
			   if (std::string((*it)->getName()) != "...") {
				   sig->addParameter(*it);
			   } else {
				   sig->addEllipsis();
				   delete *it;
			   }
		   delete $5;
		   $$ = sig;
		 }
		 ;

symbol_ref_decl: SYMBOLREF CONSTANT IDENTIFIER ';'
			{ SymbolRef *ref = new SymbolRef($2, $3);
			  refs.push_back(ref);
			}
			;

symbol_decl: CONSTANT type_ident ';'
		   { Symbol *sym = new Symbol($1);
			 sym->nam = $2->nam;
			 sym->ty = $2->ty;
			 symbols.push_back(sym);
		   }
			 /* Note: in practice, a function signature needs either a
			   "symbolmods" (__nodecode or __incomplete), or a calling
			   convention (__cdecl, __pascal, __thiscall, etc).
			   This is because of the one-symbol lookahead limitation;
			   the parser can't distinguish 123 int foo from 123 int foo() */
		   | CONSTANT symbol_mods signature ';'
		   { Symbol *sym = new Symbol($1);
			 sym->sig = $3;
			 sym->mods = $2;
			 symbols.push_back(sym);
		   }
		   ; 

symbol_mods: NODECODE symbol_mods
		   { $$ = $2;
			 $$->noDecode = true;
		   }
		   | INCOMPLETE symbol_mods
		   { $$ = $2;
			 $$->incomplete = true;
		   } 
		   | /* empty */
		   { $$ = new SymbolMods(); }
		   ;

custom_options: exp ':'
		   { $$ = new CustomOptions(); $$->exp = $1;
		   }
		   | WITHSTACK CONSTANT ')'
		   { $$ = new CustomOptions(); $$->sp = $2;
		   }
		   | /* */
		   { $$ = new CustomOptions(); }
		   ;

array_modifier: '[' CONSTANT ']'
		  { $$ = new ArrayType(NULL, $2);
		  }
		  | '[' ']'
		  { $$ = new ArrayType(NULL);
		  } 
		  | array_modifier '[' CONSTANT ']'
		  { $$ = new ArrayType($1, $3);
		  }
		  | array_modifier '[' ']'
		  { $$ = new ArrayType($1);
		  }
		  ;

type_ident: type IDENTIFIER
		  { $$ = new TypeIdent();
			$$->ty = $1;
			$$->nam = $2;
		  }
		  | type IDENTIFIER array_modifier
		  { $$ = new TypeIdent();
			((ArrayType*)$3)->fixBaseType($1);
			$$->ty = $3;
			$$->nam = $2;
		  }
		  ;

type_ident_list: type_ident ';' type_ident_list 
		  { $$ = $3;
			$$->push_front($1);
		  }
		  | type_ident ';'
		  { $$ = new std::list<TypeIdent*>(); 
			$$->push_back($1);
		  }
		  ;

type: CHAR 
	{ $$ = new CharType(); }
	| SHORT 
	{ $$ = new IntegerType(16, 1); }
	| INT 
	{ $$ = new IntegerType(32, 1); }
	| UNSIGNED CHAR
	{ $$ = new IntegerType(8, 0); }
	| UNSIGNED SHORT
	{ $$ = new IntegerType(16, 0); }
	| UNSIGNED INT 
	{ $$ = new IntegerType(32, 0); }
	| UNSIGNED LONG 
	{ $$ = new IntegerType(32, 0); }
	| UNSIGNED 
	{ $$ = new IntegerType(32, 0); }
	| LONG 
	{ $$ = new IntegerType(32, 1); }
	| LONG LONG
	{ $$ = new IntegerType(64, 1); }
	| UNSIGNED LONG LONG
	{ $$ = new IntegerType(64, 0); }
	| FLOAT 
	{ $$ = new FloatType(32); }
	| DOUBLE 
	{ $$ = new FloatType(64); }
	| VOID
	{ $$ = new VoidType(); }
	| type '*'
	{ $$ = new PointerType($1); }
	| type '[' CONSTANT ']'
	{ // This isn't C, but it makes defining pointers to arrays easier
	  $$ = new ArrayType($1, $3); 
	}
	| type '[' ']'
	{ // This isn't C, but it makes defining pointers to arrays easier
	  $$ = new ArrayType($1); 
	}
	| IDENTIFIER
	{ //$$ = Type::getNamedType($1); 
	  //if ($$ == NULL)
	  $$ = new NamedType($1);
	}
	| CONST type
	{ $$ = $2; }
	| STRUCT IDENTIFIER 
	{
	  char tmp[1024];
	  sprintf(tmp, "struct %s", $2);
	  $$ = new NamedType(tmp);
	}
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
		printf("\n%s", theScanner->lineBuf);
	printf("\n%*s\n%*s on line %i\n", theScanner->column, "^", theScanner->column, s, theScanner->theLine);
}

AnsiCParser::~AnsiCParser()
{
	// Suppress warnings from gcc about lack of virtual destructor
}


