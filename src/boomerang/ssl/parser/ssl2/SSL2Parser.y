%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert

%code requires {

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/RTL.h"

#include "boomerang/ssl/parser/InsNameElem.h"

class SSL2ParserDriver;

}

// The parsing context.
%param { SSL2ParserDriver& drv }
%locations
%define parse.trace
%define parse.error verbose
%code {

#include "boomerang/ssl/RTLInstDict.h"
#include "SSL2ParserDriver.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/ssl/type/CharType.h"
#include "boomerang/util/Util.h"

extern OPER strToTerm(const QString &name);                // Convert string to a Terminal (if possible)
extern SharedExp listExpToExp(std::list<SharedExp>* le);   // Convert a STL list of Exp* to opList
extern SharedExp listStrToExp(std::list<std::string>* ls); // Convert a STL list of strings to opList

}

%define api.token.prefix {TOK_}
%token
  END  0    "end of file"
;

// keywords
%token ENDIANNESS BIG LITTLE
%token COVERS SHARES
%token FPUSH FPOP
%token TOK_FLOAT TOK_INTEGER

// identifiers
%token <QString> IDENT REG_IDENT TEMP
%token <int>     INT_LITERAL
%token <double>  FLOAT_LITERAL

// operators
%token BIT_OR BIT_AND XOR
%token EQUAL NEQ LESS GTR LESSEQ GTREQ ULESS UGTR ULESSEQ UGTREQ
%token AND OR NOT LNOT FNEG
%token SHL SHR SAR ROL ROR RLC RRC
%token PLUS MINUS MOD MULT DIV SMOD SMULT SDIV POW
%token FMUL FDMUL FQMUL FDIV FDDIV FQDIV
%token FPLUS FDPLUS FQPLUS FMINUS FDMINUS FQMINUS

// function calls
%token ADDROF SUCCESSOR
%token <QString> CONV_FUNC TRUNC_FUNC FABS_FUNC TRANSCEND
%token <QString> NAME_CALL NAME_LOOKUP

// other tokens
%token MEMOF REGOF
%token <int> REG_NUM
%token THEN INDEX ASSIGN TO DOT COLON AT UNDERSCORE QUESTION COMMA SEMICOLON DOLLAR QUOTE DQUOTE
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token <QString> ASSIGNTYPE

// precedence
%left COMMA
%right ASSIGN
%left OR
%left AND
%left BIT_OR
%left XOR
%left BIT_AND
%left EQUAL NEQ
%left LESS GTR LESSEQ GTREQ ULESS UGTR ULESSEQ UGTREQ
%left SHL SHR SAR ROL ROR RLC RRC
%left PLUS FPLUS FDPLUS FQPLUS MINUS FMINUS FDMINUS FQMINUS
%left MULT DIV MOD SMULT SDIV SMOD FMUL FDMUL FQMUL FDIV FDDIV FQDIV
%right NOT LNOT
%right CAST_OP
%nonassoc AT

%type <SharedExp>    exp location exp_term
%type <Statement *>  statement
%type <Assign *>     assignment
%type <SharedType>   assigntype
%type <int>          cast
%type <SharedRTL>    rtl nonempty_rtl
%type <QString>      bin_oper str
%type <std::shared_ptr<Table>> table_expr
%type <std::shared_ptr<InsNameElem>> instr_name instr_name_elem
%type <std::shared_ptr<std::deque<QString>>> str_list strtable_expr str_array opstr_expr opstr_array
%type <std::shared_ptr<std::deque<SharedExp>>> exprstr_expr exprstr_array
%type <std::shared_ptr<std::list<QString>>> paramlist nonempty_paramlist
%type <std::shared_ptr<std::list<SharedExp>>> arglist nonempty_arglist


%printer { yyoutput << $$; } <*>;
%printer { yyoutput << $$.toStdString(); } <QString>;
%printer { yyoutput << $$->toString().toStdString(); } <SharedExp>;
%printer { yyoutput << $$->prints().toStdString(); } <Statement *>;
%printer { yyoutput << $$->prints().toStdString(); } <Assign *>;
%printer {
    yyoutput << "{ ";
    bool first = true;

    for (auto &elem : *$$) {
        if (first) {
            first = false;
        }
        else {
            yyoutput << ", ";
        }
        yyoutput << elem.toStdString();
    }
    yyoutput << " }";
} <std::shared_ptr<std::deque<QString>>>


%%
%start specorasgn;

specorasgn:
    specification
  | assignment { drv.the_asgn = $1; }
  | exp { drv.the_asgn = new Assign(Terminal::get(opNil), $1); }
  ;

specification:
    specification parts SEMICOLON
  | parts SEMICOLON
  ;

parts:
    endianness_def
  | constant_def
  | reglist
  | flagfunc_def
  | table_assign
  | instr_def
  ;

endianness_def:
    ENDIANNESS BIG      { drv.m_dict->m_endianness = Endian::Big; }
  | ENDIANNESS LITTLE   { drv.m_dict->m_endianness = Endian::Little; }
  ;

constant_def:
    IDENT ASSIGN INT_LITERAL {
        if (drv.ConstTable.find($1) != drv.ConstTable.end()) {
            throw yy::parser::syntax_error(drv.location, "Constant already defined.");
        }
/*        SharedExp rhs = $3->simplify();
        if (!rhs->isIntConst()) {
            throw yy::parser::syntax_error(drv.location, "Cannot assign a non-constant value to a constant.");
        }*/

        drv.ConstTable[$1] = $3;
    }
  ;

exp:
    exp FMUL exp        { $$ = Binary::get(opFMult,     $1, $3); }
  | exp FDMUL exp       { $$ = Binary::get(opFMultd,    $1, $3); }
  | exp FQMUL exp       { $$ = Binary::get(opFMultq,    $1, $3); }
  | exp FDIV  exp       { $$ = Binary::get(opFDiv,      $1, $3); }
  | exp FDDIV exp       { $$ = Binary::get(opFDivd,     $1, $3); }
  | exp FQDIV exp       { $$ = Binary::get(opFDivq,     $1, $3); }
  | exp FPLUS exp       { $$ = Binary::get(opFPlus,     $1, $3); }
  | exp FDPLUS exp      { $$ = Binary::get(opFPlusd,    $1, $3); }
  | exp FQPLUS exp      { $$ = Binary::get(opFPlusq,    $1, $3); }
  | exp FMINUS exp      { $$ = Binary::get(opFMinus,    $1, $3); }
  | exp FDMINUS exp     { $$ = Binary::get(opFMinusd,   $1, $3); }
  | exp FQMINUS exp     { $$ = Binary::get(opFMinusq,   $1, $3); }
  | exp POW exp         { $$ = Binary::get(opPow,       $1, $3); }
  | exp MOD exp         { $$ = Binary::get(opMod,       $1, $3); }
  | exp MULT exp        { $$ = Binary::get(opMult,      $1, $3); }
  | exp DIV exp         { $$ = Binary::get(opDiv,       $1, $3); }
  | exp SMULT exp       { $$ = Binary::get(opMults,     $1, $3); }
  | exp SDIV exp        { $$ = Binary::get(opDivs,      $1, $3); }
  | exp SMOD exp        { $$ = Binary::get(opMods,      $1, $3); }
  | exp PLUS exp        { $$ = Binary::get(opPlus,      $1, $3); }
  | exp MINUS exp       { $$ = Binary::get(opMinus,     $1, $3); }
  | exp SHL exp         { $$ = Binary::get(opShiftL,    $1, $3); }
  | exp SHR exp         { $$ = Binary::get(opShiftR,    $1, $3); }
  | exp SAR exp         { $$ = Binary::get(opShiftRA,   $1, $3); }
  | exp ROL exp         { $$ = Binary::get(opRotateL,   $1, $3); }
  | exp ROR exp         { $$ = Binary::get(opRotateR,   $1, $3); }
  | exp RLC exp         { $$ = Binary::get(opRotateLC,  $1, $3); }
  | exp RRC exp         { $$ = Binary::get(opRotateRC,  $1, $3); }
  | exp BIT_OR exp      { $$ = Binary::get(opBitOr,     $1, $3); }
  | exp BIT_AND exp     { $$ = Binary::get(opBitAnd,    $1, $3); }
  | exp XOR exp         { $$ = Binary::get(opBitXor,    $1, $3); }
  | exp EQUAL exp       { $$ = Binary::get(opEquals,    $1, $3); }
  | exp NEQ exp         { $$ = Binary::get(opNotEqual,  $1, $3); }
  | exp LESS exp        { $$ = Binary::get(opLess,      $1, $3); }
  | exp LESSEQ exp      { $$ = Binary::get(opLessEq,    $1, $3); }
  | exp GTR exp         { $$ = Binary::get(opGtr,       $1, $3); }
  | exp GTREQ exp       { $$ = Binary::get(opGtrEq,     $1, $3); }
  | exp ULESS exp       { $$ = Binary::get(opLessUns,   $1, $3); }
  | exp UGTR exp        { $$ = Binary::get(opGtrUns,    $1, $3); }
  | exp ULESSEQ exp     { $$ = Binary::get(opLessEqUns, $1, $3); }
  | exp UGTREQ exp      { $$ = Binary::get(opGtrEqUns,  $1, $3); }
  | exp AND exp         { $$ = Binary::get(opAnd,       $1, $3); }
  | exp OR exp          { $$ = Binary::get(opOr,        $1, $3); }
  | NOT exp             { $$ = Unary::get(opNot,   $2); }
  | LNOT exp            { $$ = Unary::get(opLNot,  $2); }
  | FNEG exp            { $$ = Unary::get(opFNeg,  $2); }
  | exp cast %prec CAST_OP {
        if ($2 == STD_SIZE) {
            $$ = std::move($1);
        }
        else {
            $$ = Binary::get(opSize, Const::get($2), $1);
        }
    }
  | exp_term { $$ = std::move($1); }
  ;

cast:
    LBRACE INT_LITERAL RBRACE { $$ = $2; }
  ;

exp_term:
    INT_LITERAL     { $$ = Const::get($1); }
  | FLOAT_LITERAL   { $$ = Const::get($1); }
  | LPAREN exp RPAREN { $$ = std::move($2); }
  | location        { $$ = std::move($1); }
  | LBRACKET exp QUESTION exp COLON exp RBRACKET { $$ = Ternary::get(opTern, $2, $4, $6); }
  | ADDROF LPAREN exp RPAREN { $$ = Unary::get(opAddrOf, $3); }
  | CONV_FUNC LPAREN INT_LITERAL COMMA INT_LITERAL COMMA exp RPAREN {
        $$ = Ternary::get(drv.strToOper($1), Const::get($3), Const::get($5), $7);
    }
  | TRUNC_FUNC LPAREN exp RPAREN { $$ = Unary::get(opFtrunc, $3); }
  | FABS_FUNC LPAREN exp RPAREN  { $$ = Unary::get(opFabs, $3); }
  | TRANSCEND LPAREN exp RPAREN  { $$ = Unary::get(drv.strToOper($1), $3); }
  | NAME_LOOKUP LBRACKET IDENT RBRACKET {
        /* example: *Use* of COND[idx] */
        if (drv.indexrefmap.find($3) == drv.indexrefmap.end()) {
            throw yy::parser::syntax_error(drv.location, "Index not declared for use.");
        }
        else if (drv.TableDict.find($1) == drv.TableDict.end()) {
            throw yy::parser::syntax_error(drv.location, "Table not declared for use.");
        }
        else if (drv.TableDict[$1]->getType() != EXPRTABLE) {
            throw yy::parser::syntax_error(drv.location, "Table is not an expression table.");
        }
        else if (std::static_pointer_cast<ExprTable>(drv.TableDict[$1])->expressions.size() !=
                 drv.indexrefmap[$3]->getNumTokens()) {
            throw yy::parser::syntax_error(drv.location, "Table size does not match index size.");
        }

        $$ = Binary::get(opExpTable, Const::get($1), Const::get($3));
    }
  ;

location:
    REG_IDENT {
        const bool isFlag = $1.contains("flags");
        auto it = drv.m_dict->m_regIDs.find($1);
        if (it == drv.m_dict->m_regIDs.end() && !isFlag) {
            throw yy::parser::syntax_error(drv.location, "Register is undefined.");
        }
        else if (isFlag || it->second == -1) {
            // A special register, e.g. %npc or %CF. Return a Terminal for it
            const OPER op = strToTerm($1);
            if (op) {
                $$ = Terminal::get(op);
            }
            else {
                // Machine specific feature
                $$ = Unary::get(opMachFtr, Const::get($1));
            }
        }
        else {
            $$ = Location::regOf(it->second);
        }
    }
  | REGOF LBRACKET exp RBRACKET { $$ = Location::regOf($3); }
  | REG_NUM                     { $$ = Location::regOf($1); }
  | MEMOF LBRACKET exp RBRACKET { $$ = Location::memOf($3); }
  | exp AT LBRACKET exp COLON exp RBRACKET { $$ = Ternary::get(opAt, $1, $4, $6); }
  | TEMP                        { $$ = Location::tempOf(Const::get($1)); }
  | SUCCESSOR LPAREN exp RPAREN { $$ = drv.makeSuccessor($3); }
  | IDENT {
        if (drv.m_dict->m_definedParams.find($1) != drv.m_dict->m_definedParams.end()) {
            $$ = Location::param($1);
        }
        else if (drv.ConstTable.find($1) != drv.ConstTable.end()) {
            $$ = Const::get(drv.ConstTable[$1]);
        }
        else {
            throw yy::parser::syntax_error(drv.location, "Undeclared identifier.");
        }
    }
  ;

arglist:
    %empty              { $$.reset(new std::list<SharedExp>()); }
  | nonempty_arglist    { $$ = std::move($1); }
  ;

nonempty_arglist:
    exp                         { $$.reset(new std::list<SharedExp>({ $1 })); }
  | nonempty_arglist COMMA exp  { $1->push_back($3); $$ = std::move($1); }
  ;

reglist:
    TOK_INTEGER { drv.bFloat = false; } a_reglist
  | TOK_FLOAT   { drv.bFloat = true;  } a_reglist
  ;

a_reglist:
    REG_IDENT LBRACKET INT_LITERAL RBRACKET INDEX INT_LITERAL {
        if ($3 <= 0) {
            throw yy::parser::syntax_error(drv.location, "Register size must be positive.");
        }
        else if (drv.m_dict->m_regIDs.find($1) != drv.m_dict->m_regIDs.end()) {
            throw yy::parser::syntax_error(drv.location, "Register already defined.");
        }
        drv.m_dict->addRegister($1, $6, $3, drv.bFloat);
    }
  | REG_IDENT LBRACKET INT_LITERAL RBRACKET INDEX INT_LITERAL COVERS REG_IDENT TO REG_IDENT {
        if ($3 <= 0) {
            throw yy::parser::syntax_error(drv.location, "Register size must be positive.");
        }
        else if (drv.m_dict->m_regIDs.find($1) != drv.m_dict->m_regIDs.end()) {
            throw yy::parser::syntax_error(drv.location, "Register already defined.");
        }
        else if ($6 != -1 && drv.m_dict->m_regInfo.find($6) != drv.m_dict->m_regInfo.end()) {
            throw yy::parser::syntax_error(drv.location, "Register index already defined.");
        }
        else if (drv.m_dict->m_regIDs.find($8)  == drv.m_dict->m_regIDs.end() ||
                 drv.m_dict->m_regIDs.find($10) == drv.m_dict->m_regIDs.end()) {
            throw yy::parser::syntax_error(drv.location, "Undefined COVERS range.");
        }

        int bitSum = 0; // sum of all bits of all covered registers
        const int rangeStart = drv.m_dict->m_regIDs[$8];
        const int rangeEnd   = drv.m_dict->m_regIDs[$10];

        // range inclusive!
        for (int i = rangeStart; i <= rangeEnd; i++) {
            if (drv.m_dict->getRegNameByID(i) == "") {
                throw yy::parser::syntax_error(drv.location, "Not all registers in range defined.");
            }
            bitSum += drv.m_dict->getRegSizeByID(i);
        }

        if (bitSum != $3) {
            throw yy::parser::syntax_error(drv.location, "Register size does not match size of covered registers.");
        }

        drv.m_dict->addRegister($1, $6, $3, drv.bFloat);
        if ($6 != -1) {
            drv.m_dict->m_regInfo[$6].setName($1);
            drv.m_dict->m_regInfo[$6].setSize($3);
            drv.m_dict->m_regInfo[$6].setMappedIndex(drv.m_dict->getRegIDByName($8));
            drv.m_dict->m_regInfo[$6].setMappedOffset(0);
            drv.m_dict->m_regInfo[$6].setIsFloat(drv.bFloat);
        }
    }
  | REG_IDENT LBRACKET INT_LITERAL RBRACKET INDEX INT_LITERAL SHARES REG_IDENT AT LBRACKET INT_LITERAL TO INT_LITERAL RBRACKET {
        if ($3 <= 0) {
            throw yy::parser::syntax_error(drv.location, "Register size must be positive.");
        }
        else if (drv.m_dict->m_regIDs.find($1) != drv.m_dict->m_regIDs.end()) {
            throw yy::parser::syntax_error(drv.location, "Register already defined.");
        }
        else if ($6 != -1 && drv.m_dict->m_regInfo.find($6) != drv.m_dict->m_regInfo.end()) {
            throw yy::parser::syntax_error(drv.location, "Register index already defined.");
        }
        else if (drv.m_dict->getRegIDByName($8) == -1) {
            QString msg = QString("Shared register '%1' not defined.").arg($8);
            throw yy::parser::syntax_error(drv.location, msg.toStdString());
        }
        else if ($3 != ($13 - $11) + 1) {
            throw yy::parser::syntax_error(drv.location, "Register size does not equal shared range");
        }

        const int tgtRegSize = drv.m_dict->getRegSizeByID(drv.m_dict->getRegIDByName($8));
        if ($11 < 0 || $13 >= tgtRegSize) {
            throw yy::parser::syntax_error(drv.location, "Range extends over target register.");
        }

        drv.m_dict->addRegister($1, $6, $3, drv.bFloat);
        if ($6 != -1) {
            drv.m_dict->m_regInfo[$6].setName($1);
            drv.m_dict->m_regInfo[$6].setSize($3);
            drv.m_dict->m_regInfo[$6].setMappedIndex(drv.m_dict->getRegIDByName($8));
            drv.m_dict->m_regInfo[$6].setMappedOffset($11);
            drv.m_dict->m_regInfo[$6].setIsFloat(drv.bFloat);
        }
    }
  ;

flagfunc_def:
    NAME_CALL LPAREN paramlist RPAREN {
        drv.m_dict->m_definedParams.insert($3->begin(), $3->end());
    } LBRACE rtl RBRACE {
        if (drv.m_dict->m_flagFuncs.find($1) != drv.m_dict->m_flagFuncs.end()) {
            throw yy::parser::syntax_error(drv.location, "Flag function already defined.");
        }
        drv.m_dict->m_flagFuncs.insert($1);
        drv.m_dict->m_definedParams.clear();
    }
  ;

paramlist:
    %empty              { $$.reset(new std::list<QString>()); }
  | nonempty_paramlist  { $$ = std::move($1); }
  ;

nonempty_paramlist: /* comma separated list of identifiers */
    IDENT                           {
        $$.reset(new std::list<QString>({ $1 }));
    }
  | nonempty_paramlist COMMA IDENT  {
        $1->push_back($3);
        $$ = std::move($1);
    }
  ;

rtl:
    UNDERSCORE { /* I think %empty should do too */
        $$.reset(new RTL(Address::ZERO));
    }
  | nonempty_rtl { $$ = std::move($1); }
  ;

nonempty_rtl:
    statement               { $$.reset(new RTL(Address::ZERO, { $1 })); }
  | nonempty_rtl statement  { $1->append($2); $$ = std::move($1); }
  ;

statement:
    assignment { $$ = $1; }
  | NAME_CALL LPAREN arglist RPAREN { /* example: ADDFLAGS(...) */
        if (drv.m_dict->m_flagFuncs.find($1) == drv.m_dict->m_flagFuncs.end()) {
            throw yy::parser::syntax_error(drv.location, "Flag function not defined.");
        }
        const bool isFloat = $1 == "SETFFLAGS";
        $$ = new Assign(Terminal::get(isFloat ? opFflags : opFlags),
                        Binary::get(opFlagCall, Const::get($1), listExpToExp($3.get())));
    }
  | FPUSH   { $$ = new Assign(Terminal::get(opNil), Terminal::get(opFpush)); }
  | FPOP    { $$ = new Assign(Terminal::get(opNil), Terminal::get(opFpop)); }
  ;

assignment:
    assigntype location ASSIGN exp {
        $$ = new Assign($1, $2, $4);
    }
  | assigntype exp THEN location ASSIGN exp {
        $$ = new Assign($1, $4, $6);
        $$->setGuard($2);
    }
  ;

assigntype:
    ASSIGNTYPE {
        const QString typeStr = $1.mid(1, $1.length()-2);
        // we know this is not empty because of lexer rules
        const int offset = typeStr[0].isNumber() ? 0 : 1;
        bool converted = false;
        const int size = typeStr.mid(offset).toInt(&converted, 10);

        if (!converted || size <= 0) {
            throw yy::parser::syntax_error(drv.location, "Assigntype size too small or too large.");
        }
        else if (typeStr[0].isNumber()) {
            $$ = SizeType::get(size);
        }
        else {
            switch (typeStr[0].toLatin1()) {
            case 'i': $$ = IntegerType::get(size, Sign::Signed); break;
            case 'j': $$ = IntegerType::get(size, Sign::Unknown); break;
            case 'u': $$ = IntegerType::get(size, Sign::Unsigned); break;
            case 'f': $$ = FloatType::get(size); break;
            case 'c': $$ = CharType::get(); break;
            default: assert(false);
            }
        }
    }
  ;

table_assign:
    IDENT ASSIGN table_expr {
        if (drv.TableDict.find($1) != drv.TableDict.end()) {
            throw yy::parser::syntax_error(drv.location, "Table already defined.");
        }
        drv.TableDict[$1] = $3;
    }
  ;

table_expr:
    strtable_expr   { $$.reset(new Table(*$1)); }
  | opstr_expr      { $$.reset(new OpTable(*$1)); }
  | exprstr_expr    { $$.reset(new ExprTable(*$1)); }
  ;

strtable_expr:
    strtable_expr DOT str_list {
        // cross-product of two str_expr's
        $$.reset(new std::deque<QString>());
        for (auto i : *$1) {
            for (auto j : *$3) {
                $$->push_back(i + j);
            }
        }
    }
  | str_list { $$ = std::move($1); }
  ;

str_list:
    LBRACE str_array RBRACE { $$ = std::move($2); }
  | IDENT {
        // shortcut: use .rm32 instead of . "rm32"
        if (drv.TableDict.find($1) == drv.TableDict.end()) {
            $$.reset(new std::deque<QString>({ $1 }));
        }
        else if (drv.TableDict[$1]->getType() == NAMETABLE) {
            $$.reset(new std::deque<QString>(drv.TableDict[$1]->getRecords()));
        }
        else {
            throw yy::parser::syntax_error(drv.location, "Table is not a NAMETABLE.");
        }
    }
  ;

str_array:
    str_array COMMA str {
        $1->push_back($3);
        $$ = std::move($1);
    }
  | str {
        $$.reset(new std::deque<QString>({ $1 }));
    }
  ;

str:
    DQUOTE DQUOTE       { $$ = QString(); }
  | DQUOTE IDENT DQUOTE { $$ = std::move($2); }
  ;

opstr_expr:
    LBRACE opstr_array RBRACE { $$ = std::move($2); }
  ;

opstr_array:
    DQUOTE bin_oper DQUOTE                   { $$.reset(new std::deque<QString>({ $2 })); }
  | opstr_array COMMA DQUOTE bin_oper DQUOTE { $1->push_back($4); $$ = std::move($1); }
  ;

bin_oper:
    RLC     { $$ = "rlc"; }
  | RRC     { $$ = "rrc"; }
  | ROL     { $$ = "rl";  }
  | ROR     { $$ = "ror"; }
  | SHR     { $$ = ">>";  }
  | SAR     { $$ = ">>A"; }
  | SHL     { $$ = "<<";  }
  | BIT_OR  { $$ = "|"; }
  | BIT_AND { $$ = "&"; }
  | XOR     { $$ = "^"; }
  | MOD     { $$ = "%"; }
  | MULT    { $$ = "*"; }
  | DIV     { $$ = "/"; }
  | SMULT   { $$ = "*!"; }
  | SDIV    { $$ = "/!"; }
  | SMOD    { $$ = "%!"; }
  | PLUS    { $$ = "+"; }
  | MINUS   { $$ = "-"; }
  | FMUL    { $$ = "*f"; }
  | FDMUL   { $$ = "*fd"; }
  | FQMUL   { $$ = "*fq"; }
  | FDIV    { $$ = "/f"; }
  | FDDIV   { $$ = "/fd"; }
  | FQDIV   { $$ = "/fq"; }
  | FPLUS   { $$ = "+f";  }
  | FDPLUS  { $$ = "+fd"; }
  | FQPLUS  { $$ = "+fq"; }
  | FMINUS  { $$ = "-f"; }
  | FDMINUS { $$ = "-fd"; }
  | FQMINUS { $$ = "-fq"; }
  | POW     { $$ = "pow"; }
  ;

exprstr_expr:
    LBRACE exprstr_array RBRACE { $$ = std::move($2); }
  ;

exprstr_array:
    exprstr_array COMMA DQUOTE exp DQUOTE {
        $$ = std::move($1);
        $$->push_back($4);
    }
  | DQUOTE exp DQUOTE {
        $$.reset(new std::deque<SharedExp>({ $2 }));
    }
  ;

instr_def:
    instr_name {
        $1->getRefMap(drv.indexrefmap);
    } paramlist {
        drv.m_dict->m_definedParams.insert($3->begin(), $3->end());
    } rtl {
        // This function expands the tables and saves the expanded RTLs to the dictionary
        drv.expandTables($1, $3, $5, drv.m_dict);
        drv.m_dict->m_definedParams.clear();
    }
  ;

instr_name:
    instr_name_elem                { $$ = std::move($1); }
  | instr_name DOT instr_name_elem { $1->append($3); $$ = std::move($1); }
  ;

instr_name_elem:
    IDENT {
        $$.reset(new InsNameElem($1));
    }
  | NAME_LOOKUP LBRACKET INT_LITERAL RBRACKET {
        if (drv.TableDict.find($1) == drv.TableDict.end()) {
            throw yy::parser::syntax_error(drv.location, "Table has not been declared.");
        }
        else if (!Util::inRange($3, 0, (int)drv.TableDict[$1]->getRecords().size())) {
            throw yy::parser::syntax_error(drv.location, "Can't get element of table.");
        }
        else {
            $$.reset(new InsNameElem(drv.TableDict[$1]->getRecords()[$3]));
        }
    }
  | NAME_LOOKUP LBRACKET IDENT RBRACKET {
        if (drv.TableDict.find($1) == drv.TableDict.end()) {
            throw yy::parser::syntax_error(drv.location, "Table has not been declared.");
        }
        else {
            $$.reset(new InsListElem($3, drv.TableDict[$1], $3));
        }
    }
  ;

%%

void yy::parser::error(const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
