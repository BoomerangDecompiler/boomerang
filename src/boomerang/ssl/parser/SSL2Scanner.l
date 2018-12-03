/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */

%{ /* -*- C++ -*- */

#include "SSL2ParserDriver.h"
#include "SSL2Parser.hpp"

#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/util/log/Log.h"

#include <cerrno>
#include <climits>
#include <cstdlib>

#undef NULL
#define NULL nullptr

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

identifier      [a-zA-Z][a-zA-Z_0-9]*
reg_id          %[a-zA-Z][a-zA-Z_0-9]*
int_literal     [\+\-]?[0-9]+
hex_literal     0x[0-9A-Fa-f]+
blank           [ \t]
comment         \#.*
asgntype        \*[ijufc]?[1-9][0-9]*\*

%{
#undef NULL
#define NULL nullptr

// Code run each time a pattern is matched.
#define YY_USER_ACTION  loc.columns(yyleng);
%}

%%

%{
#undef NULL
#define NULL nullptr

// A handy shortcut to the location held by the driver.
yy::location& loc = drv.location;
// Code run each time yylex is called.
loc.step();

%}


"INTEGER"       return yy::parser::make_TOK_INTEGER(loc);
"FLOAT"         return yy::parser::make_TOK_FLOAT(loc);
"ENDIANNESS"    return yy::parser::make_ENDIANNESS(loc);
"BIG"           return yy::parser::make_BIG(loc);
"LITTLE"        return yy::parser::make_LITTLE(loc);
"COVERS"        return yy::parser::make_COVERS(loc);
"SHARES"        return yy::parser::make_SHARES(loc);
"FPUSH"         return yy::parser::make_FPUSH(loc);
"FPOP"          return yy::parser::make_FPOP(loc);
"|"|"or"        return yy::parser::make_BIT_OR(loc);
"&"|"and"       return yy::parser::make_BIT_AND(loc);
"^"|"xor"       return yy::parser::make_XOR(loc);
"="             return yy::parser::make_EQUAL(loc);
"~="            return yy::parser::make_NEQ(loc);
"<"             return yy::parser::make_LESS(loc);
">"             return yy::parser::make_GTR(loc);
"<="            return yy::parser::make_LESSEQ(loc);
">="            return yy::parser::make_GTREQ(loc);
"<u"            return yy::parser::make_ULESS(loc);
">u"            return yy::parser::make_UGTR(loc);
"<=u"           return yy::parser::make_ULESSEQ(loc);
">=u"           return yy::parser::make_UGTREQ(loc);
"<<"            return yy::parser::make_SHL(loc);
">>"            return yy::parser::make_SHR(loc);
">>A"           return yy::parser::make_SAR(loc);
"rl"            return yy::parser::make_ROL(loc);
"rr"            return yy::parser::make_ROR(loc);
"rlc"           return yy::parser::make_RLC(loc);
"rrc"           return yy::parser::make_RRC(loc);
"%"             return yy::parser::make_MOD(loc);
"*"             return yy::parser::make_MULT(loc);
"/"             return yy::parser::make_DIV(loc);
"%!"            return yy::parser::make_SMOD(loc);
"*!"            return yy::parser::make_SMULT(loc);
"/!"            return yy::parser::make_SDIV(loc);
"*f"            return yy::parser::make_FMUL(loc);
"/f"            return yy::parser::make_FDIV(loc);
"+f"            return yy::parser::make_FPLUS(loc);
"-f"            return yy::parser::make_FMINUS(loc);
"pow"           return yy::parser::make_POW(loc);
"+"             return yy::parser::make_PLUS(loc);
"-"             return yy::parser::make_MINUS(loc);
"~"             return yy::parser::make_NOT(loc);
"L~"            return yy::parser::make_LNOT(loc);
"~f"            return yy::parser::make_FNEG(loc);
"=>"            return yy::parser::make_THEN(loc);
"->"            return yy::parser::make_INDEX(loc);
":="            return yy::parser::make_ASSIGN(loc);
".."            return yy::parser::make_TO(loc);
"."             return yy::parser::make_DOT(loc);
":"             return yy::parser::make_COLON(loc);
"@"             return yy::parser::make_AT(loc);
"_"             return yy::parser::make_UNDERSCORE(loc);
"?"             return yy::parser::make_QUESTION(loc);
","             return yy::parser::make_COMMA(loc);
"("             return yy::parser::make_LPAREN(loc);
")"             return yy::parser::make_RPAREN(loc);
"["             return yy::parser::make_LBRACKET(loc);
"]"             return yy::parser::make_RBRACKET(loc);
"{"             return yy::parser::make_LBRACE(loc);
"}"             return yy::parser::make_RBRACE(loc);
";"             return yy::parser::make_SEMICOLON(loc);
"$"             return yy::parser::make_DOLLAR(loc);
"'"             return yy::parser::make_QUOTE(loc);
"\""            return yy::parser::make_DQUOTE(loc);
"addr"/"("      return yy::parser::make_ADDROF(loc);
"succ"/"("      return yy::parser::make_SUCCESSOR(loc);
"m"/"["         return yy::parser::make_MEMOF(loc);
"r"/"["         return yy::parser::make_REGOF(loc);
"r"[0-9]+ {
                return yy::parser::make_REG_NUM(strtol(yytext+1, nullptr, 10), loc);
}
{asgntype} {
                const char *result = yytext;
                return yy::parser::make_ASSIGNTYPE(result, loc);
}
("fsize"|"itof"|"ftoi"|"fround"|"truncu"|"truncs"|"zfill"|"sgnex")/"(" {
                const char *result = yytext;
                return yy::parser::make_CONV_FUNC(result, loc);
}
"ftrunc"/"(" {
                const char *result = yytext;
                return yy::parser::make_TRUNC_FUNC(result, loc);
}
"fabs"/"(" {
                const char *result = yytext;
                return yy::parser::make_FABS_FUNC(result, loc);
}
("sin"|"cos"|"tan"|"arctan"|"log2"|"loge"|"log10"|"execute"|"sqrt")/"(" {
                const char *result = yytext;
                return yy::parser::make_TRANSCEND(result, loc);
}
"tmp"[a-zA-Z0-9_]* {
                const char *result = yytext;
                return yy::parser::make_TEMP(result, loc);
}
"%"[A-Za-z][A-Za-z0-9]* {
                const char *result = yytext;
                return yy::parser::make_REG_IDENT(result, loc);
}
(_?[A-Za-z][A-Za-z0-9_]*)/"(" {
                const char *result = yytext;
                return yy::parser::make_NAME_CALL(result, loc);
}
(_?[A-Za-z][A-Za-z0-9_]*)/"[" {
                const char *result = yytext;
                return yy::parser::make_NAME_LOOKUP(result, loc);
}
_?[A-Za-z][A-Za-z0-9_]* {
                const char *result = yytext;
                return yy::parser::make_IDENT(result, loc);
}
-?[0-9]+"."[0-9]+ {
                return yy::parser::make_FLOAT_LITERAL(atof(yytext), loc);
}
-?[0-9]+ {
                return yy::parser::make_INT_LITERAL(strtol(yytext, nullptr, 10), loc);
}
0x[A-F0-9]+ {
                return yy::parser::make_INT_LITERAL(strtol(yytext, nullptr, 16), loc);
}
-?"2**"[0-9]+    {
                // a power of two
                const int sign = (yytext[0] == '-' ? -1 : 1);
                const char *start = yytext + (sign == -1 ? 4:3);
                return yy::parser::make_INT_LITERAL((1 << strtol(start, nullptr, 10)) * sign, loc);
}

{blank}+   loc.step();
{comment}  loc.step();
[\n]+      loc.lines(yyleng); loc.step();

<<EOF>>    return yy::parser::make_END(loc);

. {
    throw yy::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
}

%%

bool SSL2ParserDriver::scanBegin()
{
    yy_flex_debug = trace_scanning;
    if (file.empty()) {
        return false;
    }
    else if (!(yyin = fopen(file.c_str(), "r"))) {
        LOG_ERROR("Cannot open SSL file: %1", strerror(errno));
        return false;
    }

    return true;
}


void SSL2ParserDriver::scanEnd()
{
    fclose(yyin);
}
