/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.3"
%defines
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace {::SSL2}
%define api.prefix {SSL2}

%code requires {

#include "InsNameElem.h"

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/RTL.h"


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

}

%define api.token.prefix {TOK_}
%token
  END  0    "end of file"
;

// keywords
%token KW_ENDIANNESS KW_BIG KW_LITTLE
%token KW_COVERS KW_SHARES
%token KW_FPUSH KW_FPOP
%token KW_FLOAT KW_INTEGER KW_FLAGS KW_INSTRUCTION

// identifiers
%token <QString> IDENT REG_IDENT TEMP STR_LITERAL
%token <int>     INT_LITERAL
%token <double>  FLOAT_LITERAL

// operators
%token BIT_OR BIT_AND XOR
%token EQUAL NEQ LESS GTR LESSEQ GTREQ ULESS UGTR ULESSEQ UGTREQ
%token AND OR NOT LNOT FNEG
%token SHL SHR SAR ROL ROR RLC RRC
%token PLUS MINUS MOD MULT DIV SMOD SMULT SDIV POW
%token FMUL FDIV
%token FPLUS FMINUS

// function calls
%token ADDROF SUCCESSOR
%token <QString> CONV_FUNC TRUNC_FUNC FABS_FUNC TRANSCEND
%token <QString> NAME_CALL NAME_LOOKUP

// other tokens
%token MEMOF REGOF
%token <int> REG_NUM
%token THEN INDEX ASSIGN TO DOT COLON AT UNDERSCORE QUESTION COMMA SEMICOLON DOLLAR QUOTE
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
%left PLUS MINUS FPLUS FMINUS
%left MULT DIV MOD SMULT SDIV SMOD FMUL FDIV
%right NOT LNOT
%nonassoc AT

%type <SharedExp>    exp location exp_term
%type <Statement *>  statement
%type <Assign *>     assignment
%type <SharedType>   assigntype
%type <SharedRTL>    rtl nonempty_rtl rtl_part
%type <std::shared_ptr<Table>> table_expr
%type <std::shared_ptr<InsNameElem>> instr_name instr_name_elem
%type <std::shared_ptr<std::deque<QString>>> str_list strtable_expr str_array
%type <std::shared_ptr<std::list<QString>>> paramlist nonempty_paramlist
%type <std::shared_ptr<std::list<SharedExp>>> arglist nonempty_arglist


%printer { yyoutput << $$; } <*>;
%printer { yyoutput << $$.toStdString(); } <QString>;
%printer { yyoutput << $$->toString().toStdString(); } <SharedExp>;
%printer { yyoutput << $$->toString().toStdString(); } <Statement *>;
%printer { yyoutput << $$->toString().toStdString(); } <Assign *>;
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
%start specification;

specification:
    specification parts SEMICOLON
  | parts SEMICOLON
  ;

parts:
    endianness_def
  | constant_def
  | reg_def
  | flagfunc_def
  | table_assign
  | instr_def
  ;

endianness_def:
    KW_ENDIANNESS KW_BIG      { drv.m_dict->m_endianness = Endian::Big; }
  | KW_ENDIANNESS KW_LITTLE   { drv.m_dict->m_endianness = Endian::Little; }
  ;

constant_def:
    IDENT ASSIGN INT_LITERAL {
        if (drv.ConstTable.find($1) != drv.ConstTable.end()) {
            throw SSL2::parser::syntax_error(drv.location, "Constant already defined.");
        }

        drv.ConstTable[$1] = $3;
    }
  ;

exp:
    exp FMUL exp        { $$ = Binary::get(opFMult,     $1, $3); }
  | exp FDIV  exp       { $$ = Binary::get(opFDiv,      $1, $3); }
  | exp FPLUS exp       { $$ = Binary::get(opFPlus,     $1, $3); }
  | exp FMINUS exp      { $$ = Binary::get(opFMinus,    $1, $3); }
  | exp POW exp         { $$ = Binary::get(opPow,       $1, $3); }
  | exp MOD exp         { $$ = Binary::get(opMod,       $1, $3); }
  | exp MULT exp        { $$ = Binary::get(opMult,      $1, $3); }
  | exp DIV exp         { $$ = Binary::get(opDiv,       $1, $3); }
  | exp SMULT exp       { $$ = Binary::get(opMults,     $1, $3); }
  | exp SDIV exp        { $$ = Binary::get(opDivs,      $1, $3); }
  | exp SMOD exp        { $$ = Binary::get(opMods,      $1, $3); }
  | exp PLUS exp        { $$ = Binary::get(opPlus,      $1, $3); }
  | exp MINUS exp       { $$ = Binary::get(opMinus,     $1, $3); }
  | exp SHL exp         { $$ = Binary::get(opShL,       $1, $3); }
  | exp SHR exp         { $$ = Binary::get(opShR,       $1, $3); }
  | exp SAR exp         { $$ = Binary::get(opShRA,      $1, $3); }
  | exp ROL exp         { $$ = Binary::get(opRotL,      $1, $3); }
  | exp ROR exp         { $$ = Binary::get(opRotR,      $1, $3); }
  | exp RLC exp         { $$ = Binary::get(opRotLC,     $1, $3); }
  | exp RRC exp         { $$ = Binary::get(opRotRC,     $1, $3); }
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
  | NOT exp             { $$ = Unary::get(opBitNot, $2); }
  | LNOT exp            { $$ = Unary::get(opLNot,   $2); }
  | FNEG exp            { $$ = Unary::get(opFNeg,   $2); }
  | exp_term { $$ = std::move($1); }
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
  ;

location:
    REG_IDENT {
        if (!drv.m_dict->getRegDB()->isRegDefined($1)) {
            throw SSL2::parser::syntax_error(drv.location, "Register is undefined.");
        }

        const RegNum regNum = drv.m_dict->getRegDB()->getRegNumByName($1);
        if (regNum == RegNumSpecial) {
            const OPER op = strToTerm($1);
            if (op != opInvalid) {
                $$ = Terminal::get(op);
            }
            else {
                // Machine specific feature
                $$ = Unary::get(opMachFtr, Const::get($1));
            }
        }
        else {
            $$ = Location::regOf(regNum);
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
            throw SSL2::parser::syntax_error(drv.location, "Undeclared identifier.");
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

reg_def:
    KW_INTEGER { drv.m_regType = RegType::Int;   } reg_def_part
  | KW_FLOAT   { drv.m_regType = RegType::Float; } reg_def_part
  | KW_FLAGS   { drv.m_regType = RegType::Flags; } reg_def_part
  ;

reg_def_part:
    // example: %eax[32] -> 24
    REG_IDENT LBRACKET INT_LITERAL RBRACKET INDEX INT_LITERAL {
        if ($3 <= 0) {
            throw SSL2::parser::syntax_error(drv.location, "Register size must be positive.");
        }
        else if (drv.m_dict->getRegDB()->isRegDefined($1)) {
            throw SSL2::parser::syntax_error(drv.location, "Register already defined.");
        }
        else if (!drv.m_dict->getRegDB()->createReg(drv.m_regType, $6, $1, $3)) {
            throw SSL2::parser::syntax_error(drv.location, "Cannot create register.");
        }
    }
    // example: %foo[32] -> 10 COVERS %bar..%baz
  | REG_IDENT LBRACKET INT_LITERAL RBRACKET INDEX INT_LITERAL KW_COVERS REG_IDENT TO REG_IDENT {
        if ($3 <= 0) {
            throw SSL2::parser::syntax_error(drv.location, "Register size must be positive.");
        }
        else if (drv.m_dict->getRegDB()->isRegDefined($1)) {
            throw SSL2::parser::syntax_error(drv.location, "Register already defined.");
        }
        else if ($6 != RegNumSpecial && drv.m_dict->getRegDB()->isRegNumDefined($6)) {
            throw SSL2::parser::syntax_error(drv.location, "Register index already defined.");
        }
        else if (!drv.m_dict->getRegDB()->isRegDefined($8) ||
                 !drv.m_dict->getRegDB()->isRegDefined($10)) {
            throw SSL2::parser::syntax_error(drv.location, "Undefined COVERS range.");
        }

        int bitSum = 0; // sum of all bits of all covered registers
        const RegNum rangeStart = drv.m_dict->getRegDB()->getRegNumByName($8);
        const RegNum rangeEnd   = drv.m_dict->getRegDB()->getRegNumByName($10);

        // range inclusive!
        for (RegNum i = rangeStart; i <= rangeEnd; i++) {
            if (drv.m_dict->getRegDB()->getRegNameByNum(i) == "") {
                throw SSL2::parser::syntax_error(drv.location, "Not all registers in range defined.");
            }
            bitSum += drv.m_dict->getRegDB()->getRegSizeByNum(i);
        }

        if (bitSum != $3) {
            throw SSL2::parser::syntax_error(drv.location, "Register size does not match size of covered registers.");
        }

        if (!drv.m_dict->getRegDB()->createReg(drv.m_regType, $6, $1, $3)) {
            throw SSL2::parser::syntax_error(drv.location, "Cannot create register.");
        }

        if ($6 != RegNumSpecial) {
            bitSum = 0;
            for (int i = rangeStart; i <= rangeEnd; i++) {
                drv.m_dict->getRegDB()->createRegRelation($1,
                    drv.m_dict->getRegDB()->getRegNameByNum(i), bitSum);
                bitSum += drv.m_dict->getRegDB()->getRegSizeByNum(i);
            }
        }
    }
    // example: %ah[8] -> 10 SHARES %ax@[8..15]
  | REG_IDENT LBRACKET INT_LITERAL RBRACKET INDEX INT_LITERAL KW_SHARES REG_IDENT AT LBRACKET INT_LITERAL TO INT_LITERAL RBRACKET {
        if ($3 <= 0) {
            throw SSL2::parser::syntax_error(drv.location, "Register size must be positive.");
        }
        else if (drv.m_dict->getRegDB()->isRegDefined($1)) {
            throw SSL2::parser::syntax_error(drv.location, "Register already defined.");
        }
        else if ($6 != RegNumSpecial && drv.m_dict->getRegDB()->isRegNumDefined($6)) {
            throw SSL2::parser::syntax_error(drv.location, "Register index already defined.");
        }
        else if (drv.m_dict->getRegDB()->getRegNumByName($8) == RegNumSpecial) {
            QString msg = QString("Shared register '%1' not defined.").arg($8);
            throw SSL2::parser::syntax_error(drv.location, msg.toStdString());
        }
        else if ($3 != ($13 - $11) + 1) {
            throw SSL2::parser::syntax_error(drv.location, "Register size does not equal shared range");
        }

        const int tgtRegSize = drv.m_dict->getRegDB()->getRegSizeByNum(drv.m_dict->getRegDB()->getRegNumByName($8));
        if ($11 < 0 || $13 >= tgtRegSize) {
            throw SSL2::parser::syntax_error(drv.location, "Range extends over target register.");
        }
        else if (!drv.m_dict->getRegDB()->createReg(drv.m_regType, $6, $1, $3)) {
            throw SSL2::parser::syntax_error(drv.location, "Cannot create register.");
        }
        else if ($6 != RegNumSpecial) {
            drv.m_dict->getRegDB()->createRegRelation($8, $1, $11);
        }
    }
  ;

flagfunc_def:
    // example: *def* of ADDFLAGS(...) { ... }
    NAME_CALL LPAREN paramlist RPAREN {
        drv.m_dict->m_definedParams.insert($3->begin(), $3->end());
    } LBRACE rtl RBRACE {
        if (drv.m_dict->m_flagFuncs.find($1) != drv.m_dict->m_flagFuncs.end()) {
            throw SSL2::parser::syntax_error(drv.location, "Flag function already defined.");
        }
        drv.m_dict->m_flagFuncs.insert($1);
        drv.m_dict->m_definedParams.clear();
    }
  ;

paramlist: // comma separated list of identifiers
    %empty              { $$.reset(new std::list<QString>()); }
  | nonempty_paramlist  { $$ = std::move($1); }
  ;

nonempty_paramlist:
    IDENT {
        $$.reset(new std::list<QString>({ $1 }));
    }
  | nonempty_paramlist COMMA IDENT  {
        $1->push_back($3);
        $$ = std::move($1);
    }
  ;

rtl:
    // I think %empty should do too, but this is more explicit
    UNDERSCORE {
        $$.reset(new RTL(Address::ZERO));
    }
  | nonempty_rtl { $$ = std::move($1); }
  ;

nonempty_rtl:
    statement               { $$.reset(new RTL(Address::ZERO, { $1 })); }
  | rtl_part                { $$ = std::move($1); }
  | nonempty_rtl statement  { $1->append($2); $$ = std::move($1); }
  | nonempty_rtl rtl_part   { $1->append($2->getStatements()); $$ = std::move($1); }
  ;

rtl_part:
    KW_FPUSH {
        $$.reset(new RTL(Address::ZERO, {
            new Assign(FloatType::get(80), Location::tempOf(Const::get(const_cast<char *>("tmpD9"))), Location::regOf(REG_PENT_ST7)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST7), Location::regOf(REG_PENT_ST6)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST6), Location::regOf(REG_PENT_ST5)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST5), Location::regOf(REG_PENT_ST4)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST4), Location::regOf(REG_PENT_ST3)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST3), Location::regOf(REG_PENT_ST2)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST2), Location::regOf(REG_PENT_ST1)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST1), Location::regOf(REG_PENT_ST0)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST0), Location::tempOf(Const::get(const_cast<char *>("tmpD9"))))
        }));
    }
  | KW_FPOP {
        $$.reset(new RTL(Address::ZERO, {
            new Assign(FloatType::get(80), Location::tempOf(Const::get(const_cast<char *>("tmpD9"))), Location::regOf(REG_PENT_ST0)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST0), Location::regOf(REG_PENT_ST1)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST1), Location::regOf(REG_PENT_ST2)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST2), Location::regOf(REG_PENT_ST3)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST3), Location::regOf(REG_PENT_ST4)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST4), Location::regOf(REG_PENT_ST5)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST5), Location::regOf(REG_PENT_ST6)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST6), Location::regOf(REG_PENT_ST7)),
            new Assign(FloatType::get(80), Location::regOf(REG_PENT_ST7), Location::tempOf(Const::get(const_cast<char *>("tmpD9")))),
        }));
    }
  ;

statement:
    assignment { $$ = $1; }
    // example: *use* of ADDFLAGS(...)
  | NAME_CALL LPAREN arglist RPAREN {
        if (drv.m_dict->m_flagFuncs.find($1) == drv.m_dict->m_flagFuncs.end()) {
            throw SSL2::parser::syntax_error(drv.location, "Flag function not defined.");
        }
        const bool isFloat = $1 == "SETFFLAGS";
        $$ = new Assign(Terminal::get(isFloat ? opFflags : opFlags),
                        Binary::get(opFlagCall, Const::get($1), listExpToExp($3.get())));
    }
  ;

assignment:
    // example *32* %eax := 0
    assigntype location ASSIGN exp {
        $$ = new Assign($1, $2, $4);
    }
    // exampe *32* %CF=0 => %eax := %ecx
  | assigntype exp THEN location ASSIGN exp {
        $$ = new Assign($1, $4, $6);
        $$->setGuard($2);
    }
  ;

assigntype:
    // example: *i32*
    ASSIGNTYPE {
        const QString typeStr = $1.mid(1, $1.length()-2);
        // we know this is not empty because of lexer rules
        const int offset = typeStr[0].isNumber() ? 0 : 1;
        bool converted = false;
        const int size = typeStr.mid(offset).toInt(&converted, 10);

        if (!converted || size <= 0) {
            throw SSL2::parser::syntax_error(drv.location, "Assigntype size too small or too large.");
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
            throw SSL2::parser::syntax_error(drv.location, "Table already defined.");
        }
        drv.TableDict[$1] = $3;
    }
  ;

table_expr:
    strtable_expr   { $$.reset(new Table(*$1)); }
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
        else {
            $$.reset(new std::deque<QString>(drv.TableDict[$1]->getRecords()));
        }
    }
  ;

str_array:
    str_array COMMA STR_LITERAL {
        $1->push_back($3);
        $$ = std::move($1);
    }
  | STR_LITERAL {
        $$.reset(new std::deque<QString>({ $1 }));
    }
  ;

instr_def:
    KW_INSTRUCTION instr_name {
        $2->getRefMap(drv.indexrefmap);
    } LPAREN paramlist RPAREN LBRACE {
        drv.m_dict->m_definedParams.insert($5->begin(), $5->end());
    } rtl RBRACE {
        // This function expands the tables and saves the expanded RTLs to the dictionary
        drv.expandTables($2, $5, $9, drv.m_dict);
        drv.m_dict->m_definedParams.clear();
    }
  ;

// example: PUSH.reg32
instr_name:
    instr_name_elem                { $$ = std::move($1); }
  | instr_name DOT instr_name_elem { $1->append($3); $$ = std::move($1); }
  ;

instr_name_elem:
    // example: "foo"
    STR_LITERAL {
        $$.reset(new InsNameElem($1));
    }
    // example: FOO[IDX] where FOO is some kind of pre-defined string table
  | NAME_LOOKUP LBRACKET IDENT RBRACKET {
        if (drv.TableDict.find($1) == drv.TableDict.end()) {
            throw SSL2::parser::syntax_error(drv.location, "Table has not been declared.");
        }

        $$.reset(new InsListElem($3, drv.TableDict[$1], $3));
    }
  ;

%%

void SSL2::parser::error(const location_type& l, const std::string& m)
{
    std::cerr << l << ": " << m << '\n';
}
