/*==============================================================================
 * FILE:       ansi-c.y
 * OVERVIEW:   Parser for ANSI C.
 *
 *  This doesn't actually work yet.
 *============================================================================*/
/*
 * $Revision$
 * 10 Apr 02 - Trent: Created
 */

%{
  #include <list>
  #include "exp.h"
  #include "stmt.h"
  #include "type.h"
  #include "decl.h"
  #include "func.h"
%}
%token PREINCLUDE PREDEFINE PREIF PREIFDEF PREENDIF PRELINE
%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%union {
   Func *func;
   Type *type;
   Decl *decl;
   list<Decl *> *decls;
   Stmt *stmt;
   list<Stmt *> *stmts;
   BlockStmt *block;
}

%type<func> function_definition
%type<type> declaration_specifiers 
%type<decl> declarator 
%type<decls> declaration_list 
%type<decls> declaration
%type<block> compound_statement
%type<stmts> statement_list
%type<decl> declaration
%type<stmt> statement

%start translation_unit
%%

primary_expression
	: IDENTIFIER
          { $$ = new Const(idVar, $1); }
	| CONSTANT
          { $$ = new Const(idInt, $1); }
	| STRING_LITERAL
          { $$ = new Const(idString, $1); }
	| '(' expression ')'
          { $$ = $2; }
	;

postfix_expression
	: primary_expression
          { $$ = $1; }
	| postfix_expression '[' expression ']'
          { $$ = Binary(idArrayOf, $1, $3); }
	| postfix_expression '(' ')'
          { $$ = Unary(idFunc, NULL); }
	| postfix_expression '(' argument_expression_list ')'
          { $$ = Unary(idFunc, $3); }
	| postfix_expression '.' IDENTIFIER
          { $$ = Binary(idMember, $1, $3); }
	| postfix_expression PTR_OP IDENTIFIER
          { $$ = Binary(idMemberThru, $1, $3); }
	| postfix_expression INC_OP
          { $$ = Unary(idPostInc, $1); }
	| postfix_expression DEC_OP
          { $$ = Unary(idPostDec, $1); }
	;

argument_expression_list
	: assignment_expression
          {}
	| argument_expression_list ',' assignment_expression
          {}
	;

unary_expression
	: postfix_expression
          {}
	| INC_OP unary_expression
          {}
	| DEC_OP unary_expression
          {}
	| unary_operator cast_expression
          {}
	| SIZEOF unary_expression
          {}
	| SIZEOF '(' type_name ')'
          {}
	;

unary_operator
	: '&'
          {}
	| '*'
          {}
	| '+'
          {}
	| '-'
          {}
	| '~'
          {}
	| '!'
          {}
	;

cast_expression
	: unary_expression
          {}
	| '(' type_name ')' cast_expression
          {}
	;

multiplicative_expression
	: cast_expression
          {}
	| multiplicative_expression '*' cast_expression
          {}
	| multiplicative_expression '/' cast_expression
          {}
	| multiplicative_expression '%' cast_expression
          {}
	;

additive_expression
	: multiplicative_expression
          {}
	| additive_expression '+' multiplicative_expression
          {}
	| additive_expression '-' multiplicative_expression
          {}
	;

shift_expression
	: additive_expression
          {}
	| shift_expression LEFT_OP additive_expression
          {}
	| shift_expression RIGHT_OP additive_expression
          {}
	;

relational_expression
	: shift_expression
          {}
	| relational_expression '<' shift_expression
          {}
	| relational_expression '>' shift_expression
          {}
	| relational_expression LE_OP shift_expression
          {}
	| relational_expression GE_OP shift_expression
          {}
	;

equality_expression
	: relational_expression
          {}
	| equality_expression EQ_OP relational_expression
          {}
	| equality_expression NE_OP relational_expression
          {}
	;

and_expression
	: equality_expression
          {}
	| and_expression '&' equality_expression
          {}
	;

exclusive_or_expression
	: and_expression
          {}
	| exclusive_or_expression '^' and_expression
          {}
	;

inclusive_or_expression
	: exclusive_or_expression
          {}
	| inclusive_or_expression '|' exclusive_or_expression
          {}
	;

logical_and_expression
	: inclusive_or_expression
          {}
	| logical_and_expression AND_OP inclusive_or_expression
          {}
	;

logical_or_expression
	: logical_and_expression
          {}
	| logical_or_expression OR_OP logical_and_expression
          {}
	;

conditional_expression
	: logical_or_expression
          {}
	| logical_or_expression '?' expression ':' conditional_expression
          {}
	;

assignment_expression
	: conditional_expression
          {}
	| unary_expression assignment_operator assignment_expression
          {}
	;

assignment_operator
	: '='
          {}
	| MUL_ASSIGN
          {}
	| DIV_ASSIGN
          {}
	| MOD_ASSIGN
          {}
	| ADD_ASSIGN
          {}
	| SUB_ASSIGN
          {}
	| LEFT_ASSIGN
          {}
	| RIGHT_ASSIGN
          {}
	| AND_ASSIGN
          {}
	| XOR_ASSIGN
          {}
	| OR_ASSIGN
          {}
	;

expression
	: assignment_expression
          {}
	| expression ',' assignment_expression
          {}
	;

constant_expression
	: conditional_expression
          {}
	;

declaration
	: declaration_specifiers ';'
          { $$ = new list<Decl *>;
            $$->push_back(new Decl($1));
          }
	| declaration_specifiers init_declarator_list ';'
          { $$ = new list<Decl *>;
            for(list<Decl *>::iterator d = $2->begin(); d != $2->end(); d++) {
               (*d)->setBaseType($1);
               $$->push_bash(*d);
            }
          }
	;

declaration_specifiers
	: storage_class_specifier
          { $$ = $1;}
	| storage_class_specifier declaration_specifiers
          { $$ = $1;
            $$->addToType($2);
          }
	| type_specifier
          { $$ = $1; }
	| type_specifier declaration_specifiers
          { $$ = $1;
            $$->addToType($2);
          }
	| type_qualifier
          { $$ = $1; }
	| type_qualifier declaration_specifiers
          { $$ = $1;
            $$->addToType($2);
          }
	;

init_declarator_list
	: init_declarator
          { $$ = new list<Decl *>;
            $$->push_back($1); 
          }
	| init_declarator_list ',' init_declarator
          { $$ = $1;
            $$->push_back($3);
          }
	;

init_declarator
	: declarator
          { $$ = $1; }
	| declarator '=' initializer
          { $$ = $1; $$->setInitVal($3); }
	;

storage_class_specifier
	: TYPEDEF
          { $$ = new Type(Type::C_TYPEDEF); }
	| EXTERN
          { $$ = new Type(Type::C_EXTERN); }
	| STATIC
          { $$ = new Type(Type::C_STATIC); }
	| AUTO
          { $$ = new Type(Type::C_AUTO); }
	| REGISTER
          { $$ = new Type(Type::C_REGISTER); }
	;

type_specifier
	: VOID
          { $$ = new Type(Type::C_VOID); }
	| CHAR
          { $$ = new Type(Type::C_CHAR); }
	| SHORT
          { $$ = new Type(Type::C_SHORT); }
	| INT
          { $$ = new Type(Type::C_INT); }
	| LONG
          { $$ = new Type(Type::C_LONG); }
	| FLOAT
          { $$ = new Type(Type::C_FLOAT); }
	| DOUBLE
          { $$ = new Type(Type::C_DOUBLE); }
	| SIGNED
          { $$ = new Type(Type::C_SIGNED); }
	| UNSIGNED
          { $$ = new Type(Type::C_UNSIGNED); }
	| struct_or_union_specifier
          { $$ = $1; }
	| enum_specifier
          { $$ = $1; }
	| TYPE_NAME
          { $$ = $1; }
	;

struct_or_union_specifier
	: struct_or_union IDENTIFIER '{' struct_declaration_list '}'
          { $$ = $1; $$->setName($2); $$->setFields($4); }
	| struct_or_union '{' struct_declaration_list '}'
          { $$ = $1; $$->setFields($3); }
	| struct_or_union IDENTIFIER
          { $$ = $1; $$->setName($2); }
	;

struct_or_union
	: STRUCT
          { $$ = new StructType(); }
	| UNION
          { $$ = new UnionType(); }
	;

struct_declaration_list
	: struct_declaration
          { $$ = new list<Decl *>;
            $$->merge($1);
          }
	| struct_declaration_list struct_declaration
          { $$ = $1;
            $$->merge($2);
          }
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
          { $$ = new list<Decl *>;
            for(list<Decl *>::iterator d = $2->begin(); d != $2->end(); d++) {
               (*d)->setBaseType($1);
               $$->push_bash(*d);
            }
          }
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
          { $$ = $1; $$->addToType($2); }
	| type_specifier
          { $$ = $1; }
	| type_qualifier specifier_qualifier_list
          { $$ = $1; $$->addToType($2); }
	| type_qualifier
          { $$ = $1; }
	;

struct_declarator_list
	: struct_declarator
          { $$ = new list<Decl *>;
            $$->push_back($1);
          }
	| struct_declarator_list ',' struct_declarator
          { $$ = $1;
            $$->push_back($3);
          }
	;

struct_declarator
	: declarator
          { $$ = $1; }
	| ':' constant_expression
          { $$ = new Type(); $$->setBitLength($2); }
	| declarator ':' constant_expression
          { $$ = $1; $$->setBitLength($3); }
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
          { $$ = new EnumType($3); }
	| ENUM IDENTIFIER '{' enumerator_list '}'
          { $$ = new EnumType($2, $4); }
	| ENUM IDENTIFIER
          { $$ = new EnumType($1); }
	;

enumerator_list
	: enumerator
          { $$ = new list<Decl *>;
            $$->push_back($1);
          }
	| enumerator_list ',' enumerator
          { $$ = $1; 
            $$->push_back($3);
          }
	;

enumerator
	: IDENTIFIER
          { $$ = new Decl($1); }
	| IDENTIFIER '=' constant_expression
          { $$ = new Decl($1); $$->setInitVal($3); }
	;

type_qualifier
	: CONST
          { $$ = new Type(Type::C_CONST); }
	| VOLATILE
          { $$ = new Type(Type::C_VOLATILE); }
	;

declarator
	: pointer direct_declarator
          { $$ = $1; $$->addToDecl($2); }
	| direct_declarator
          { $$ = $1; }
	;

direct_declarator
	: IDENTIFIER
          { $$ = new Decl($1); }
	| '(' declarator ')'
          { $$ = new Decl($1); }
	| direct_declarator '[' constant_expression ']'
          { $$ = new ArrayDecl($1, $3); }
	| direct_declarator '[' ']'
          { $$ = new ArrayDecl($1); }
	| direct_declarator '(' parameter_type_list ')'
          { $$ = new FuncDecl($1, $3); }
	| direct_declarator '(' identifier_list ')'
          { $$ = new FuncDecl($1, $3); }
	| direct_declarator '(' ')'
          { $$ = new FuncDecl($1); }
	;

pointer
	: '*'
          { $$ = new PointerType(); }
	| '*' type_qualifier_list
          { $$ = new PointerType($1); } 
	| '*' pointer
          { $$ = new PointerDecl($1); }
	| '*' type_qualifier_list pointer
          { $$ = new PointerDecl($1, $2); }
	;

type_qualifier_list
	: type_qualifier
          { $$ = new list<Type *>;
            $$->push_back($1); 
          }
	| type_qualifier_list type_qualifier
          { $$ = $1;  $$->push_back($2); }
	;


parameter_type_list
	: parameter_list
          { $$ = $1; }
	| parameter_list ',' ELLIPSIS
          { $$ = $1; }
	;

parameter_list
	: parameter_declaration
          { $$ = new list<Decl*>;
            $$->push_back($1); 
          }
	| parameter_list ',' parameter_declaration
          { $$ = $1;
            $1->push_back($3);
          }
	;

parameter_declaration
	: declaration_specifiers declarator
          { $$ = $1; $$->setBaseType($1); }
	| declaration_specifiers abstract_declarator
          { $$ = $1; $$->setBaseType($1); }
	| declaration_specifiers
          { $$ = new Decl(); $$->setBaseType($1); }
	;

identifier_list
	: IDENTIFIER
          { $$ = new list<Exp *>;
            $$->push_back(new UnaryExp(idIdent, $1));
          }
	| identifier_list ',' IDENTIFIER
          { $$ = $1;
            $$->push_back(new UnaryExp(idIdent, $3)); 
          }
	;

type_name
	: specifier_qualifier_list
          {}
	| specifier_qualifier_list abstract_declarator
          {}
	;

abstract_declarator
	: pointer
          { $$ = $1; }
	| direct_abstract_declarator
          { $$ = $1; }
	| pointer direct_abstract_declarator
          { $$ = $1; $$->setTo($2); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
          {}
	| '[' ']'
          {}
	| '[' constant_expression ']'
          {}
	| direct_abstract_declarator '[' ']'
          {}
	| direct_abstract_declarator '[' constant_expression ']'
          {}
	| '(' ')'
          {}
	| '(' parameter_type_list ')'
          {}
	| direct_abstract_declarator '(' ')'
          {}
	| direct_abstract_declarator '(' parameter_type_list ')'
          {}
	;

initializer
	: assignment_expression
          {}
	| '{' initializer_list '}'
          {}
	| '{' initializer_list ',' '}'
          {}
	;

initializer_list
	: initializer
          {}
	| initializer_list ',' initializer
          {}
	;

statement
	: labeled_statement
          { $$ = $1; }
	| compound_statement
          { $$ = $1; }
	| expression_statement
          { $$ = $1; }
	| selection_statement
          { $$ = $1; }
	| iteration_statement
          { $$ = $1; }
	| jump_statement
          { $$ = $1; }
	;

labeled_statement
	: IDENTIFIER ':' statement
          {}
	| CASE constant_expression ':' statement
          {}
	| DEFAULT ':' statement
          {}
	;

compound_statement
	: '{' '}'
          { $$ = new BlockStmt(); }
	| '{' statement_list '}'
          { $$ = new BlockStmt($2); }
	| '{' declaration_list '}'
          { $$ = new BlockStmt($2); }
	| '{' declaration_list statement_list '}'
          { $$ = new BlockStmt($2, $3); }
	;

declaration_list
	: declaration
          { $$ = new list<Decl *>;
            $$->merge($1);
          }
	| declaration_list declaration
          { $$ = $1;
            $$->merge($2);
          }
	;

statement_list
	: statement
          { $$ = new list<Stmt *>;
            $$->push_back($1);
          }
	| statement_list statement
          { $$ = $1;
            $$->push_back($2); 
          }
	;

expression_statement
	: ';'
          {}
	| expression ';'
          {}
	;

selection_statement
	: IF '(' expression ')' statement
          {}
	| IF '(' expression ')' statement ELSE statement
          {}
	| SWITCH '(' expression ')' statement
          {}
	;

iteration_statement
	: WHILE '(' expression ')' statement
          {}
	| DO statement WHILE '(' expression ')' ';'
          {}
	| FOR '(' expression_statement expression_statement ')' statement
          {}
	| FOR '(' expression_statement expression_statement expression ')' statement
          {}
	;

jump_statement
	: GOTO IDENTIFIER ';'
          {}
	| CONTINUE ';'
          {}
	| BREAK ';'
          {}
	| RETURN ';'
          {}
	| RETURN expression ';'
          {}
	;

translation_unit
	: external_declaration
          {}
	| translation_unit external_declaration
          {}
	;

external_declaration
	: function_definition
          {}
	| declaration
          {}
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
          { $$ = new Func($1, $2, $3, $4); }
	| declaration_specifiers declarator compound_statement
          { $$ = new Func($1, $2, $3); }
	| declarator declaration_list compound_statement
          { $$ = new Func($1, $2, $3); }
	| declarator compound_statement
          { $$ = new Func($1, $2); }
	;

%%
#include <stdio.h>

extern char yytext[];
extern int column;

void yyerror(char *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}



