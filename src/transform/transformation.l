/*==============================================================================
 * FILE:       transformation.l
 * OVERVIEW:   Scanner for transformation language.
 *============================================================================*/
/*
 * $Revision$
 * 17 Apr 04 - Trent: Created
 */
%name TransformationScanner


D			[0-9]
/* ALlow ? and @ in identifiers, so we can use mangled names */
L			[a-zA-Z_?@]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%header{
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "transformation-parser.h"
// For some reason, MSVC 5.00 complains about use of undefined type RTL a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#include "rtl.h"
#endif
#define MAX_LINE 1024      // the longest line
%}

%define CONSTRUCTOR_PARAM std::istream &ins, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace), in(ins), column(0)
%define CONSTRUCTOR_CODE { *lineBuf = 0; }
%define INPUT_CODE if (in.eof()) {  \
    result = 0; \
} else { \
    in.read(buffer, max_size); \
    result=in.gcount(); \
} \
return result;
%define MEMBERS \
public: \
    int theLine;        /* the current line number */\
    char lineBuf[MAX_LINE]; /* the current line */ \
    bool traceLines;    /* echo each lines as it is scanned */ \
    std::istream &in; \
    int column; \
private: \
    void count(); \
    void comment();

%define LEX_PARAM YY_TransformationParser_STYPE &yylval

%%

"/*"			{ comment(); }

"sizeof"		{ count(); return TransformationParser::SIZEOF; }
"where"		        { count(); return TransformationParser::WHERE; }
"becomes"		{ count(); return TransformationParser::BECOMES; }
"kind("                 { count(); return TransformationParser::KIND; }
"type("                 { count(); return TransformationParser::TYPE; }
"pointer("              { count(); return TransformationParser::POINTER; }
"compound"              { count(); return TransformationParser::COMPOUND; }
"array"                 { count(); return TransformationParser::ARRAY; }
"r["                    { count(); return TransformationParser::REGOF; }
"m["                    { count(); return TransformationParser::MEMOF; }
"a["                    { count(); return TransformationParser::ADDROF; }
"true"                  { count(); return TransformationParser::BOOL_TRUE; }
"false"                 { count(); return TransformationParser::BOOL_FALSE; }
{L}({L}|{D})*\(         { count(); yylval.str = strdup(yytext); yylval.str[strlen(yylval.str)-1] = 0; return TransformationParser::FUNC; }

{L}({L}|{D})*		{ count(); yylval.str = strdup(yytext); return TransformationParser::IDENTIFIER; }

0[xX]{H}+{IS}?		{ count(); sscanf(yytext, "0x%x", &yylval.ival); return TransformationParser::CONSTANT; }
0{D}+{IS}?		{ count(); return TransformationParser::CONSTANT; }
{D}+{IS}?		{ count(); sscanf(yytext, "%d", &yylval.ival); return TransformationParser::CONSTANT; }
L?'(\\.|[^\\'])+'	{ count(); return TransformationParser::CONSTANT; }

{D}+{E}{FS}?		{ count(); return TransformationParser::CONSTANT; }
{D}*"."{D}+({E})?{FS}?	{ count(); return TransformationParser::CONSTANT; }
{D}+"."{D}*({E})?{FS}?	{ count(); return TransformationParser::CONSTANT; }

L?\"(\\.|[^\\"])*\"	{ count(); yylval.str = strdup(yytext); return TransformationParser::STRING_LITERAL; }

"..."			{ count(); return TransformationParser::ELLIPSIS; }
">>="			{ count(); return TransformationParser::RIGHT_ASSIGN; }
"<<="			{ count(); return TransformationParser::LEFT_ASSIGN; }
"+="			{ count(); return TransformationParser::ADD_ASSIGN; }
"-="			{ count(); return TransformationParser::SUB_ASSIGN; }
"*="			{ count(); return TransformationParser::MUL_ASSIGN; }
"/="			{ count(); return TransformationParser::DIV_ASSIGN; }
"%="			{ count(); return TransformationParser::MOD_ASSIGN; }
"&="			{ count(); return TransformationParser::AND_ASSIGN; }
"^="			{ count(); return TransformationParser::XOR_ASSIGN; }
"|="			{ count(); return TransformationParser::OR_ASSIGN; }
">>"			{ count(); return TransformationParser::RIGHT_OP; }
"<<"			{ count(); return TransformationParser::LEFT_OP; }
"++"			{ count(); return TransformationParser::INC_OP; }
"--"			{ count(); return TransformationParser::DEC_OP; }
"->"			{ count(); return TransformationParser::PTR_OP; }
"&&"			{ count(); return TransformationParser::AND_OP; }
"||"			{ count(); return TransformationParser::OR_OP; }
"<="			{ count(); return TransformationParser::LE_OP; }
">="			{ count(); return TransformationParser::GE_OP; }
"=="			{ count(); return TransformationParser::EQ_OP; }
"!="			{ count(); return TransformationParser::NE_OP; }
";"			{ count(); return ';'; }
"{"		        { count(); return '{'; }
"}"		        { count(); return '}'; }
","			{ count(); return ','; }
":"			{ count(); return ':'; }
"="			{ count(); return '='; }
"("			{ count(); return '('; }
")"			{ count(); return ')'; }
"["		        { count(); return '['; }
"]"		        { count(); return ']'; }
"."			{ count(); return '.'; }
"&"			{ count(); return '&'; }
"!"			{ count(); return '!'; }
"~"			{ count(); return '~'; }
"-"			{ count(); return '-'; }
"+"			{ count(); return '+'; }
"*"			{ count(); return '*'; }
"/"			{ count(); return '/'; }
"%"			{ count(); return '%'; }
"<"			{ count(); return '<'; }
">"			{ count(); return '>'; }
"^"			{ count(); return '^'; }
"|"			{ count(); return '|'; }
"?"			{ count(); return '?'; }

[ \t\v\n\f]		{ count(); }
.			{ /* ignore bad characters */ }

%%

void TransformationScanner::comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		/*putchar(c)*/;

	if ((c1 = input()) != '/' && c != 0)
	{
		/*unput(c1);*/
		goto loop;
	}

	if (c != 0)
		/*putchar(c1)*/;
}

void TransformationScanner::count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

        strcat(lineBuf, yytext);
        if (strchr(lineBuf, '\n')) {
            char buf[MAX_LINE];
            strcpy(buf, lineBuf);
            strcpy(lineBuf, strchr(buf, '\n')+1);
            theLine++;
        }
}



