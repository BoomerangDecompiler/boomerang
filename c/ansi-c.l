/*==============================================================================
 * FILE:       ansi-c.l
 * OVERVIEW:   Scanner for ANSI C.
 *============================================================================*/
/*
 * $Revision$
 * 10 Apr 02 - Trent: Created
 */
%name AnsiCScanner


D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%header{
#include <stdio.h>
#include "ansi-c-parser.h"
#define MAX_LINE 1024      // the longest line
%}

%define CONSTRUCTOR_PARAM std::istream &ins, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace), in(ins), column(0)
%define CONSTRUCTOR_CODE
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
    void comment(); \
    int check_type();

%define LEX_PARAM YY_AnsiCParser_STYPE &yylval

%%

^#include               { count(); return AnsiCParser::PREINCLUDE; }
^#define                { count(); return AnsiCParser::PREDEFINE; }
^#if                    { count(); return AnsiCParser::PREIF; }
^#ifdef                 { count(); return AnsiCParser::PREIFDEF; }
^#endif                 { count(); return AnsiCParser::PREENDIF; }
^#line                  { count(); return AnsiCParser::PRELINE; }

"/*"			{ comment(); }

"auto"			{ count(); return AnsiCParser::AUTO; }
"break"			{ count(); return AnsiCParser::BREAK; }
"case"			{ count(); return AnsiCParser::CASE; }
"char"			{ count(); return AnsiCParser::CHAR; }
"const"			{ count(); return AnsiCParser::CONST; }
"continue"		{ count(); return AnsiCParser::CONTINUE; }
"default"		{ count(); return AnsiCParser::DEFAULT; }
"do"			{ count(); return AnsiCParser::DO; }
"double"		{ count(); return AnsiCParser::DOUBLE; }
"else"			{ count(); return AnsiCParser::ELSE; }
"enum"			{ count(); return AnsiCParser::ENUM; }
"extern"		{ count(); return AnsiCParser::EXTERN; }
"float"			{ count(); return AnsiCParser::FLOAT; }
"for"			{ count(); return AnsiCParser::FOR; }
"goto"			{ count(); return AnsiCParser::GOTO; }
"if"			{ count(); return AnsiCParser::IF; }
"int"			{ count(); return AnsiCParser::INT; }
"long"			{ count(); return AnsiCParser::LONG; }
"register"		{ count(); return AnsiCParser::REGISTER; }
"return"		{ count(); return AnsiCParser::RETURN; }
"short"			{ count(); return AnsiCParser::SHORT; }
"signed"		{ count(); return AnsiCParser::SIGNED; }
"sizeof"		{ count(); return AnsiCParser::SIZEOF; }
"static"		{ count(); return AnsiCParser::STATIC; }
"struct"		{ count(); return AnsiCParser::STRUCT; }
"switch"		{ count(); return AnsiCParser::SWITCH; }
"typedef"		{ count(); return AnsiCParser::TYPEDEF; }
"union"			{ count(); return AnsiCParser::UNION; }
"unsigned"		{ count(); return AnsiCParser::UNSIGNED; }
"void"			{ count(); return AnsiCParser::VOID; }
"volatile"		{ count(); return AnsiCParser::VOLATILE; }
"while"			{ count(); return AnsiCParser::WHILE; }

{L}({L}|{D})*		{ count(); yylval.str = strdup(yytext); return check_type(); }

0[xX]{H}+{IS}?		{ count(); return AnsiCParser::CONSTANT; }
0{D}+{IS}?		{ count(); return AnsiCParser::CONSTANT; }
{D}+{IS}?		{ count(); return AnsiCParser::CONSTANT; }
L?'(\\.|[^\\'])+'	{ count(); return AnsiCParser::CONSTANT; }

{D}+{E}{FS}?		{ count(); return AnsiCParser::CONSTANT; }
{D}*"."{D}+({E})?{FS}?	{ count(); return AnsiCParser::CONSTANT; }
{D}+"."{D}*({E})?{FS}?	{ count(); return AnsiCParser::CONSTANT; }

L?\"(\\.|[^\\"])*\"	{ count(); yylval.str = strdup(yytext); return AnsiCParser::STRING_LITERAL; }

"..."			{ count(); return AnsiCParser::ELLIPSIS; }
">>="			{ count(); return AnsiCParser::RIGHT_ASSIGN; }
"<<="			{ count(); return AnsiCParser::LEFT_ASSIGN; }
"+="			{ count(); return AnsiCParser::ADD_ASSIGN; }
"-="			{ count(); return AnsiCParser::SUB_ASSIGN; }
"*="			{ count(); return AnsiCParser::MUL_ASSIGN; }
"/="			{ count(); return AnsiCParser::DIV_ASSIGN; }
"%="			{ count(); return AnsiCParser::MOD_ASSIGN; }
"&="			{ count(); return AnsiCParser::AND_ASSIGN; }
"^="			{ count(); return AnsiCParser::XOR_ASSIGN; }
"|="			{ count(); return AnsiCParser::OR_ASSIGN; }
">>"			{ count(); return AnsiCParser::RIGHT_OP; }
"<<"			{ count(); return AnsiCParser::LEFT_OP; }
"++"			{ count(); return AnsiCParser::INC_OP; }
"--"			{ count(); return AnsiCParser::DEC_OP; }
"->"			{ count(); return AnsiCParser::PTR_OP; }
"&&"			{ count(); return AnsiCParser::AND_OP; }
"||"			{ count(); return AnsiCParser::OR_OP; }
"<="			{ count(); return AnsiCParser::LE_OP; }
">="			{ count(); return AnsiCParser::GE_OP; }
"=="			{ count(); return AnsiCParser::EQ_OP; }
"!="			{ count(); return AnsiCParser::NE_OP; }
";"			{ count(); return ';'; }
("{"|"<%")		{ count(); return '{'; }
("}"|"%>")		{ count(); return '}'; }
","			{ count(); return ','; }
":"			{ count(); return ':'; }
"="			{ count(); return '='; }
"("			{ count(); return '('; }
")"			{ count(); return ')'; }
("["|"<:")		{ count(); return '['; }
("]"|":>")		{ count(); return ']'; }
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

void AnsiCScanner::comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
		putchar(c);

	if ((c1 = input()) != '/' && c != 0)
	{
		unput(c1);
		goto loop;
	}

	if (c != 0)
		putchar(c1);
}

void AnsiCScanner::count()
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	//ECHO;
}


int AnsiCScanner::check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return AnsiCParser::TYPE_NAME;
*
*	return AnsiCParser::IDENTIFIER;
*/

/*
*	it actually will only return IDENTIFIER
*/
	return AnsiCParser::IDENTIFIER;
}



