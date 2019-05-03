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
%option prefix="SSL2"

identifier      [a-zA-Z][a-zA-Z_0-9]*
reg_id          %[a-zA-Z][a-zA-Z_0-9]*
int_literal     [\+\-]?[0-9]+
hex_literal     0x[0-9A-Fa-f]+
str_literal     \"[^\"]*\"
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
SSL2::location& loc = drv.location;
// Code run each time yylex is called.
loc.step();

%}


"INTEGER"       return SSL2::parser::make_KW_INTEGER(loc);
"FLOAT"         return SSL2::parser::make_KW_FLOAT(loc);
"FLAGS"         return SSL2::parser::make_KW_FLAGS(loc);
"INSTRUCTION"   return SSL2::parser::make_KW_INSTRUCTION(loc);
"ENDIANNESS"    return SSL2::parser::make_KW_ENDIANNESS(loc);
"BIG"           return SSL2::parser::make_KW_BIG(loc);
"LITTLE"        return SSL2::parser::make_KW_LITTLE(loc);
"COVERS"        return SSL2::parser::make_KW_COVERS(loc);
"SHARES"        return SSL2::parser::make_KW_SHARES(loc);
"FPUSH"         return SSL2::parser::make_KW_FPUSH(loc);
"FPOP"          return SSL2::parser::make_KW_FPOP(loc);
"ret"           return SSL2::parser::make_KW_RET(loc);

"|"             return SSL2::parser::make_BIT_OR(loc);
"&"             return SSL2::parser::make_BIT_AND(loc);
"^"             return SSL2::parser::make_XOR(loc);
"||"            return SSL2::parser::make_OR(loc);
"&&"            return SSL2::parser::make_AND(loc);
"="             return SSL2::parser::make_EQUAL(loc);
"~="            return SSL2::parser::make_NEQ(loc);
"<"             return SSL2::parser::make_LESS(loc);
">"             return SSL2::parser::make_GTR(loc);
"<="            return SSL2::parser::make_LESSEQ(loc);
">="            return SSL2::parser::make_GTREQ(loc);
"<u"            return SSL2::parser::make_ULESS(loc);
">u"            return SSL2::parser::make_UGTR(loc);
"<=u"           return SSL2::parser::make_ULESSEQ(loc);
">=u"           return SSL2::parser::make_UGTREQ(loc);
"<<"            return SSL2::parser::make_SHL(loc);
">>"            return SSL2::parser::make_SHR(loc);
">>A"           return SSL2::parser::make_SAR(loc);
"rl"            return SSL2::parser::make_ROL(loc);
"rr"            return SSL2::parser::make_ROR(loc);
"rlc"           return SSL2::parser::make_RLC(loc);
"rrc"           return SSL2::parser::make_RRC(loc);
"%"             return SSL2::parser::make_MOD(loc);
"*"             return SSL2::parser::make_MULT(loc);
"/"             return SSL2::parser::make_DIV(loc);
"%!"            return SSL2::parser::make_SMOD(loc);
"*!"            return SSL2::parser::make_SMULT(loc);
"/!"            return SSL2::parser::make_SDIV(loc);
"*f"            return SSL2::parser::make_FMUL(loc);
"/f"            return SSL2::parser::make_FDIV(loc);
"+f"            return SSL2::parser::make_FPLUS(loc);
"-f"            return SSL2::parser::make_FMINUS(loc);
"pow"           return SSL2::parser::make_POW(loc);
"+"             return SSL2::parser::make_PLUS(loc);
"-"             return SSL2::parser::make_MINUS(loc);
"~"             return SSL2::parser::make_NOT(loc);
"L~"            return SSL2::parser::make_LNOT(loc);
"~f"            return SSL2::parser::make_FNEG(loc);
"=>"            return SSL2::parser::make_THEN(loc);
"->"            return SSL2::parser::make_INDEX(loc);
":="            return SSL2::parser::make_ASSIGN(loc);
".."            return SSL2::parser::make_TO(loc);
"."             return SSL2::parser::make_DOT(loc);
":"             return SSL2::parser::make_COLON(loc);
"@"             return SSL2::parser::make_AT(loc);
"_"             return SSL2::parser::make_UNDERSCORE(loc);
"?"             return SSL2::parser::make_QUESTION(loc);
","             return SSL2::parser::make_COMMA(loc);
"("             return SSL2::parser::make_LPAREN(loc);
")"             return SSL2::parser::make_RPAREN(loc);
"["             return SSL2::parser::make_LBRACKET(loc);
"]"             return SSL2::parser::make_RBRACKET(loc);
"{"             return SSL2::parser::make_LBRACE(loc);
"}"             return SSL2::parser::make_RBRACE(loc);
";"             return SSL2::parser::make_SEMICOLON(loc);
"$"             return SSL2::parser::make_DOLLAR(loc);
"'"             return SSL2::parser::make_QUOTE(loc);
"addr"/"("      return SSL2::parser::make_ADDROF(loc);
"succ"/"("      return SSL2::parser::make_SUCCESSOR(loc);
"m"/"["         return SSL2::parser::make_MEMOF(loc);
"r"/"["         return SSL2::parser::make_REGOF(loc);
"r"[0-9]+ {
                return SSL2::parser::make_REG_NUM(strtol(yytext+1, nullptr, 10), loc);
}
{asgntype} {
                const char *result = yytext;
                return SSL2::parser::make_ASSIGNTYPE(result, loc);
}
("fsize"|"itof"|"ftoi"|"fround"|"truncu"|"truncs"|"zfill"|"sgnex")/"(" {
                const char *result = yytext;
                return SSL2::parser::make_CONV_FUNC(result, loc);
}
"ftrunc"/"(" {
                const char *result = yytext;
                return SSL2::parser::make_TRUNC_FUNC(result, loc);
}
"fabs"/"(" {
                const char *result = yytext;
                return SSL2::parser::make_FABS_FUNC(result, loc);
}
("sin"|"cos"|"tan"|"arctan"|"log2"|"loge"|"log10"|"execute"|"sqrt")/"(" {
                const char *result = yytext;
                return SSL2::parser::make_TRANSCEND(result, loc);
}
"tmp"[a-zA-Z0-9_]* {
                const char *result = yytext;
                return SSL2::parser::make_TEMP(result, loc);
}
"%"[A-Za-z][A-Za-z0-9]* {
                const char *result = yytext;
                return SSL2::parser::make_REG_IDENT(result, loc);
}
(_?[A-Za-z][A-Za-z0-9_]*)/"(" {
                const char *result = yytext;
                return SSL2::parser::make_NAME_CALL(result, loc);
}
(_?[A-Za-z][A-Za-z0-9_]*)/"[" {
                const char *result = yytext;
                return SSL2::parser::make_NAME_LOOKUP(result, loc);
}
_?[A-Za-z][A-Za-z0-9_]* {
                const char *result = yytext;
                return SSL2::parser::make_IDENT(result, loc);
}
{str_literal} {
                size_t len = strlen(yytext);
                yytext[len-1] = 0;
                const char *result = yytext + 1;
                return SSL2::parser::make_STR_LITERAL(result, loc);
}
-?[0-9]+"."[0-9]+ {
                return SSL2::parser::make_FLOAT_LITERAL(atof(yytext), loc);
}
-?[0-9]+ {
                return SSL2::parser::make_INT_LITERAL(strtol(yytext, nullptr, 10), loc);
}
0x[A-F0-9]+ {
                return SSL2::parser::make_INT_LITERAL(strtol(yytext, nullptr, 16), loc);
}
-?"2**"[0-9]+    {
                // a power of two
                const int sign = (yytext[0] == '-' ? -1 : 1);
                const char *start = yytext + (sign == -1 ? 4:3);
                return SSL2::parser::make_INT_LITERAL((1 << strtol(start, nullptr, 10)) * sign, loc);
}

{blank}+   loc.step();
{comment}  loc.step();
[\n]+      loc.lines(yyleng); loc.step();

<<EOF>>    return SSL2::parser::make_END(loc);

. {
    throw SSL2::parser::syntax_error(loc, "invalid character: " + std::string(yytext));
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
    yylex_destroy();
}
