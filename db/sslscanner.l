/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:          sslscanner.l
 * OVERVIEW:      Defines a scanner to parse the tokens of a Semantic
 *                Specification Language specification.
 *============================================================================*/
 
/* 
 * Shane Sendall (original C version) Dec 1997
 * Doug Simon (C++ version) Jan 1998
 * 29 Apr 02 - Mike: Mods for boomerang; removed &~, |~, and ^~ operators
 * 09 May 02 - Mike: ASSIGNSIZE is an num now
 */

%name SSLScanner
 
%header{
#include "sslparser.h"
#define MAX_LINE 1024      // the longest SSL spec line
%}

// stuff to go in sslscanner.cc
%{
#include "sslscanner.h"
%}

%define CONSTRUCTOR_PARAM std::istream &ins, bool trace
%define CONSTRUCTOR_INIT : theLine(1), traceLines(trace), in(ins)
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
    std::istream &in;

%define LEX_PARAM YY_SSLParser_STYPE &yylval 

%%

"INTEGER"    |
"integer"    { return SSLParser::TOK_INTEGER; }
"FLOAT"      |
"float"      { return SSLParser::TOK_FLOAT; }
"OPERAND"    { return SSLParser::OPERAND; }
"ENDIANNESS" |
"endianness" {
               yylval.str = strdup(yytext);
               return SSLParser::ENDIANNESS;
             }
"BIG"        |
"big"        {
               yylval.str = strdup(yytext);
               return SSLParser::BIG;
             }
"LITTLE"     |
"little"     {
                yylval.str = strdup(yytext);
                return SSLParser::LITTLE;
             }
"COVERS"     { return SSLParser::COVERS; }
"SHARES"     { return SSLParser::SHARES; }
"FAST"       { return SSLParser::FAST; }
"FETCHEXEC"  { return SSLParser::FETCHEXEC; }

"*"[0-9]+"*" {
               yylval.num = strtol(yytext+1, 0, 10);
               return SSLParser::ASSIGNSIZE;
             }

"or"    |
"and"        {
               yylval.str = strdup(yytext);
               return SSLParser::LOG_OP;
             }
"="     |
"~="    |
"<"     |
">"     |
"<="    |
">="    |
"<u"    |
">u"    |
"<=u"   |
">=u"        {
               yylval.str = strdup(yytext);
               return SSLParser::COND_OP;
             }

"rlc"   |
"rrc"   |
"rl"    |
"rr"    |
">>"    |
"<<"    |
">>A"   |
"|"     |
"&"     |
"^"     {
        yylval.str = strdup(yytext);
        return SSLParser::BIT_OP;

      }
"%"     |
"*"     |
"/"     |
"*!"    |
"/!"    |
"%!"    |
"+"     |
"-"   {
        yylval.str = strdup(yytext);
        return SSLParser::ARITH_OP;
      }
"*f"    |
"*fd"   |
"*fq"   |
"*fsd"  |
"*fdq"  |
"/f"    |
"/fd"   |
"/fq"   |
"+f"    |
"+fd"   |
"+fq"   |
"-f"    |
"-fd"   |
"-fq" {
        yylval.str = strdup(yytext);
        return SSLParser::FARITH_OP;
      }
"~"   {
        return SSLParser::NOT;
      }
"=>"  {
        return SSLParser::THEN;
      }
"->"  {
        return SSLParser::INDEX;
       }
":="   {
        return SSLParser::EQUATE;
      }
"::="  {
        yylval.str = strdup(yytext);
        return SSLParser::ASSIGN;
      }
".."  {
        return SSLParser::TO;
      }
":"   {
        return SSLParser::COLON;
      }
"!"   {
        return SSLParser::S_E;
      }
"@"   {
        return SSLParser::AT;
      }
"addr("     {
                return SSLParser::ADDR;
            }
"fsize("  |
"itof("   |
"ftoi("   |
"fround(" |
"truncu(" |
"truncs(" |
"zfill("  |
"sgnex("     {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::CONV_FUNC;
            }
"defineflags(" |
"undefineflags(" {
                return SSLParser::FLAGMACRO;
                }
"FPOP"      {   return SSLParser::FPOP; }
"FPUSH"     {   return SSLParser::FPUSH; }
"sin("      |
"cos("      |
"tan("      |
"arctan("   |
"log2("     |
"loge("     |
"log10("    |
"execute("  |
"sqrt("     {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::TRANSCEND;
            }
"tmp"[a-zA-Z0-9_]*  {
                yylval.str = strdup(yytext);
                return SSLParser::TEMP;
            }

"r["        {   return SSLParser::REG_IDX; }
"m["        {   return SSLParser::MEM_IDX; }
"_"         {   return yytext[0];   }
"%"[A-Za-z][A-Za-z0-9]* {
                yylval.str = strdup(yytext);
                return SSLParser::REG_ID;
            }
_?[A-Za-z][A-Za-z0-9_]*"(" {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::NAME_CALL;
            }
_?[A-Za-z][A-Za-z0-9_]*"[" {
                yylval.str = strdup(yytext);
                yylval.str[strlen(yylval.str)-1] = '\0';
                return SSLParser::NAME_LOOKUP;
            }
_?[A-Za-z][A-Za-z0-9_]* {
                yylval.str = strdup(yytext);
                return SSLParser::NAME;
            }
"."[A-Za-z][A-Za-z.0-9]* |
"^"\"[A-Za-z][A-Za-z]*\" {
                    yylval.str = strdup(yytext);
                    return SSLParser::DECOR;
            }
[?,{}()'"]  {   return yytext[0];   }
"]"         {   return yytext[0];   }
"["         {   return yytext[0];   }
";"         {   return yytext[0];   }
"$"         {   return yytext[0];   }
-?[0-9]+"."[0-9]+ {
        yylval.dbl = atof(yytext);
        return SSLParser::FLOATNUM;
     }
-?[0-9]+ {
        yylval.num = strtol(yytext,0,10);
        return SSLParser::NUM;
     }

0x[A-F0-9]+ {
        yylval.num = strtol(yytext,0,16);
        return SSLParser::NUM;
     }

-?"2**"[0-9]+   {
                    // a power of two
                    int sign = (yytext[0] == '-' ? -1 : 1);
                    char* start = yytext + (sign == -1 ? 4:3);

                        // David - changed to << for more efficient and 
                        // not depend on the <cmath> anymore.
                    yylval.num = 1 << (strtol(start,0,10) * sign);
                        // yylval.num = (int)pow(2,strtol(start,0,10)) * sign;
                    return SSLParser::NUM;
                }

[ \t]+          ;
#.*$            ;   /* comment */
.               ;
\n.*            {   // funky print-next-line-for-debug token
                    if (yyleng > MAX_LINE)
                    {
                        std::cerr << theLine + 1 << ": line too long.\n";
                        exit(1);
                    }
                    // saves the next line
                    strcpy(lineBuf, yytext+1);
                    theLine++;
                    // reparse from char 1
                    // give back all but the \n to rescan
                    yyless(1);

                    // echo the line if line tracing is turned on
                    if (traceLines)
                    std::cerr << lineBuf << std::endl;

                    // Quell a warning
                    yy_last_accepting_state = 0;
                    yy_last_accepting_cpos = 0;
                }
%%
