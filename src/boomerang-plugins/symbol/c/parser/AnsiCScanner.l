/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

%{ /* -*- C++ -*- */

#include "AnsiCParserDriver.h"
#include "AnsiCParser.hpp"

#include "boomerang/util/log/Log.h"

// Work around an incompatibility in flex (at least versions
// 2.5.31 through 2.5.33): it generates code that does
// not conform to C89.  See Debian bug 333231
// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
#undef yywrap
#define yywrap() 1

// Pacify warnings in yy_init_buffer (observed with Flex 2.6.4)
// and GCC 7.3.0.
#if defined __GNUC__ && 7 <= __GNUC__
# pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
%}

%option noyywrap nounput batch debug noinput
%option prefix="AnsiC"

D           [0-9]
/* Allow ? and @ in identifiers, so we can use mangled names */
L           [a-zA-Z_?@]
H           [a-fA-F0-9]
E           [Ee][+-]?{D}+
FS          (f|F|l|L)
IS          (u|U|l|L)*

%x C_COMMENT

%{
// Code run each time a pattern is matched.
#define YY_USER_ACTION  loc.columns(yyleng);
%}

%%

%{
// A handy shortcut to the location held by the driver.
AnsiC::location& loc = drv.location;
// Code run each time yylex is called.
loc.step();

%}


"char"              { return AnsiC::parser::make_KW_CHAR(loc); }
"const"             { return AnsiC::parser::make_KW_CONST(loc); }
"double"            { return AnsiC::parser::make_KW_DOUBLE(loc); }
"enum"              { return AnsiC::parser::make_KW_ENUM(loc); }
"float"             { return AnsiC::parser::make_KW_FLOAT(loc); }
"int"               { return AnsiC::parser::make_KW_INT(loc); }
"long"              { return AnsiC::parser::make_KW_LONG(loc); }
"short"             { return AnsiC::parser::make_KW_SHORT(loc); }
"signed"            { return AnsiC::parser::make_KW_SIGNED(loc); }
"struct"            { return AnsiC::parser::make_KW_STRUCT(loc); }
"typedef"           { return AnsiC::parser::make_KW_TYPEDEF(loc); }
"union"             { return AnsiC::parser::make_KW_UNION(loc); }
"unsigned"          { return AnsiC::parser::make_KW_UNSIGNED(loc); }
"void"              { return AnsiC::parser::make_KW_VOID(loc); }

"PREFER"            { return AnsiC::parser::make_KW_PREFER(loc); }

"__nodecode"        { return AnsiC::parser::make_NODECODE(loc); }
"__incomplete"      { return AnsiC::parser::make_INCOMPLETE(loc); }
"__cdecl"           { return AnsiC::parser::make_CDECL(loc); }
"__pascal"          { return AnsiC::parser::make_PASCAL(loc); }
"__stdcall"         { return AnsiC::parser::make_PASCAL(loc); }
"__custom"          { return AnsiC::parser::make_CUSTOM(loc); }
"__thiscall"        { return AnsiC::parser::make_THISCALL(loc);}
"__withstack("      { return AnsiC::parser::make_WITHSTACK(loc); }
"__symref"          { return AnsiC::parser::make_SYMBOLREF(loc); }
"r["                { return AnsiC::parser::make_REGOF(loc); }
"m["                { return AnsiC::parser::make_MEMOF(loc); }
"@max("             { return AnsiC::parser::make_MAXBOUND(loc); }

"..."           { return AnsiC::parser::make_ELLIPSIS(loc); }
">>="           { return AnsiC::parser::make_RIGHT_ASSIGN(loc); }
"<<="           { return AnsiC::parser::make_LEFT_ASSIGN(loc); }
"+="            { return AnsiC::parser::make_ADD_ASSIGN(loc); }
"-="            { return AnsiC::parser::make_SUB_ASSIGN(loc); }
"*="            { return AnsiC::parser::make_MUL_ASSIGN(loc); }
"/="            { return AnsiC::parser::make_DIV_ASSIGN(loc); }
"%="            { return AnsiC::parser::make_MOD_ASSIGN(loc); }
"&="            { return AnsiC::parser::make_AND_ASSIGN(loc); }
"^="            { return AnsiC::parser::make_XOR_ASSIGN(loc); }
"|="            { return AnsiC::parser::make_OR_ASSIGN(loc); }
">>"            { return AnsiC::parser::make_RIGHT_OP(loc); }
"<<"            { return AnsiC::parser::make_LEFT_OP(loc); }
"++"            { return AnsiC::parser::make_INC_OP(loc); }
"--"            { return AnsiC::parser::make_DEC_OP(loc); }
"->"            { return AnsiC::parser::make_PTR_OP(loc); }
"&&"            { return AnsiC::parser::make_AND_OP(loc); }
"||"            { return AnsiC::parser::make_OR_OP(loc); }
"<="            { return AnsiC::parser::make_LE_OP(loc); }
">="            { return AnsiC::parser::make_GE_OP(loc); }
"=="            { return AnsiC::parser::make_EQ_OP(loc); }
"!="            { return AnsiC::parser::make_NE_OP(loc); }
";"             { return AnsiC::parser::make_SEMICOLON(loc); }
"{"             { return AnsiC::parser::make_LBRACE(loc); }
"}"             { return AnsiC::parser::make_RBRACE(loc); }
","             { return AnsiC::parser::make_COMMA(loc); }
":"             { return AnsiC::parser::make_COLON(loc); }
"="             { return AnsiC::parser::make_ASSIGN_OP(loc); }
"("             { return AnsiC::parser::make_LPAREN(loc); }
")"             { return AnsiC::parser::make_RPAREN(loc); }
"["             { return AnsiC::parser::make_LBRACKET(loc); }
"]"             { return AnsiC::parser::make_RBRACKET(loc); }
"."             { return AnsiC::parser::make_DOT(loc); }
"&"             { return AnsiC::parser::make_AND(loc); }
"!"             { return AnsiC::parser::make_NOT(loc); }
"~"             { return AnsiC::parser::make_BIT_NOT(loc); }
"-"             { return AnsiC::parser::make_MINUS(loc); }
"+"             { return AnsiC::parser::make_PLUS(loc); }
"*"             { return AnsiC::parser::make_STAR(loc); }
"/"             { return AnsiC::parser::make_DIV(loc); }
"%"             { return AnsiC::parser::make_MOD(loc); }
"<"             { return AnsiC::parser::make_LESS(loc); }
">"             { return AnsiC::parser::make_GTR(loc); }
"^"             { return AnsiC::parser::make_XOR(loc); }
"|"             { return AnsiC::parser::make_BIT_OR(loc); }
"?"             { return AnsiC::parser::make_QUESTION(loc); }


{L}({L}|{D})*       { return AnsiC::parser::make_IDENTIFIER(yytext, loc); }
"0"[xX]{H}+{IS}?    { return AnsiC::parser::make_CONSTANT(strtol(yytext, nullptr, 16), loc); }
"0"{D}+{IS}?        { return AnsiC::parser::make_CONSTANT(strtol(yytext, nullptr,  8), loc); }
{D}+{IS}?           { return AnsiC::parser::make_CONSTANT(strtol(yytext, nullptr, 10), loc); }
"L"?"'"(\\.|[^\\'])+"'" { return AnsiC::parser::make_STRING_LITERAL(yytext, loc); }
{D}+{E}{FS}?            { return AnsiC::parser::make_CONSTANT(strtol(yytext, nullptr, 10), loc); }
{D}*"."{D}+({E})?{FS}?  { return AnsiC::parser::make_CONSTANT(strtof(yytext, nullptr), loc); }
{D}+"."{D}*({E})?{FS}?  { return AnsiC::parser::make_CONSTANT(strtof(yytext, nullptr), loc); }
"L"?\"(.|[^\"])*\"    { return AnsiC::parser::make_STRING_LITERAL(yytext, loc); }


"/*"            { loc.step(); BEGIN(C_COMMENT); }
<C_COMMENT>"*/" { loc.step(); BEGIN(INITIAL); }
<C_COMMENT>\n   { loc.lines(yyleng); loc.step(); }
<C_COMMENT>.    { loc.step(); }
"//".*          { loc.step(); }
[\n]+           { loc.lines(yyleng); loc.step(); }
[ \t\v\f]+      { loc.step(); }

<<EOF>>         { return AnsiC::parser::make_END(loc); }

. {
    throw AnsiC::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
}

%%

bool AnsiCParserDriver::scanBegin()
{
    AnsiC_flex_debug = trace_scanning;
    if (file.empty()) {
        return false;
    }
    else if (!(AnsiCin = fopen(file.c_str(), "r"))) {
        LOG_ERROR("Cannot open SSL file: %1", strerror(errno));
        return false;
    }

    return true;
}


void AnsiCParserDriver::scanEnd()
{
    fclose(AnsiCin);
}
