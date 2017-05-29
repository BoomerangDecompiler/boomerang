/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 1998, David Ung
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   sslparser.y
 * OVERVIEW:   Defines a parser class that reads an SSL specification and
 *			   returns the list of SSL instruction and table definitions.
 *============================================================================*/

/* $Revision$
 * Updates:
 * Shane Sendall (original C version) Dec 1997
 * Doug Simon (C++ version) Jan 1998
 * 29 Apr 02 - Mike: Mods for boomerang
 * 03 May 02 - Mike: Commented
 * 08 May 02 - Mike: ParamMap -> ParamSet
 * 15 May 02 - Mike: Fixed strToOper: *f was coming out as /f, << as =
 * 16 Jul 02 - Mike: Fixed code in expandTables processing opOpTables: was
 *				doing replacements on results of searchAll
 * 09 Dec 02 - Mike: Added succ() syntax (for SPARC LDD and STD)
 * 29 Sep 03 - Mike: Parse %DF correctly
 * 22 Jun 04 - Mike: TEMP can be a location now (location was var_op)
 * 31 Oct 04 - Mike: srchExpr and srchOp are statics now; saves creating and deleting these expressions for every
 *				opcode. Seems to prevent a lot of memory churn, and may prevent (for now) the mystery
 *				test/sparc/switch_gcc problem (which goes away when you try to gdb it)
 * 30 Aug 04 - Mike: added init_sslparser() for garbage collection safety
 */

%name SSLParser		// the parser class name will be SSLParser

// stuff to go in sslparser.h
%header{
#include "config.h"
#ifdef HAVE_LIBGC
#include "gc.h"
#else
#define NO_GARBAGE_COLLECTOR
#endif
#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include <sstream>
#include "types.h"
#include "rtl.h"
#include "table.h"
#include "insnameelem.h"
#include "util.h"			// E.g. str()
#include "statement.h"

#ifdef _WIN32
#include <malloc.h>
#endif

class SSLScanner;
%}

//*============================================================================
//* Declaration of semantic value type. Must be first thing in
//* sslparser.cc (or at least before including sslscanner.h which needs
//* to know about this type).
//*============================================================================
%union {
        Exp*			exp;
        char*			str;
        int				num;
        double			dbl;
        Statement*		regtransfer;
        Type*			typ;

        Table*			tab;
        InsNameElem*	insel;
        std::list<std::string>*	  parmlist;
        std::list<std::string>*	  strlist;
        std::deque<Exp*>*	 exprlist;
        std::deque<std::string>*  namelist;
        std::list<Exp*>*	 explist;
        RTL*			rtlist;
}

// Other stuff to go in sslparser.cc
%{
#include "sslscanner.h"
OPER strToTerm(char* s);		// Convert string to a Terminal (if possible)
Exp* listExpToExp(std::list<Exp*>* le);	 // Convert a STL list of Exp* to opList
Exp* listStrToExp(std::list<std::string>* ls);// Convert a STL list of strings to opList
%}

%define DEBUG 1

// %define INHERIT : public gc	// This is how to force the parser class to be declared as derived from class gc

%define PARSE_PARAM \
        RTLInstDict& Dict

%define CONSTRUCTOR_PARAM \
        const std::string& sslFile, \
        bool trace

%define CONSTRUCTOR_INIT : \
   sslFile(sslFile), bFloat(false)

%define CONSTRUCTOR_CODE \
        std::fstream *fin = new std::fstream(sslFile.c_str(), std::ios::in); \
        theScanner = NULL; \
        if (!*fin) { \
                std::cerr << "can't open `" << sslFile << "' for reading\n"; \
        return; \
        } \
        theScanner = new SSLScanner(*fin, trace); \
        if (trace) yydebug = 1;

%define MEMBERS \
public: \
                SSLParser(std::istream &in, bool trace); \
                virtual ~SSLParser(); \
OPER	strToOper(const char*s); /* Convert string to an operator */ \
static	Statement* parseExp(const char *str); /* Parse an expression or assignment from a string */ \
/* The code for expanding tables and saving to the dictionary */ \
void	expandTables(InsNameElem* iname, std::list<std::string>* params, RTL* o_rtlist, RTLInstDict& Dict); \
Exp*	makeSuccessor(Exp* e);	/* Get successor (of register expression) */ \
\
        /* \
         * The scanner. \
         */ \
        SSLScanner* theScanner; \
protected: \
\
        /* \
         * The file from which the SSL spec is read. \
         */ \
        std::string sslFile; \
\
        /* \
         * Result for parsing an assignment. \
         */ \
        Statement *the_asgn; \
\
        /* \
         * Maps SSL constants to their values. \
         */ \
        std::map<std::string,int> ConstTable; \
\
        /* \
         * maps index names to instruction name-elements \
         */ \
        std::map<std::string, InsNameElem*> indexrefmap; \
\
        /* \
         * Maps table names to Table's.\
         */ \
        std::map<std::string, Table*> TableDict; \
\
        /* \
         * True when FLOAT keyword seen; false when INTEGER keyword seen \
         * (in @REGISTER section) \
         */ \
        bool bFloat;

/*==============================================================================
 * Declaration of token types, associativity and precedence
 *============================================================================*/

%token <str> COND_OP BIT_OP ARITH_OP LOG_OP
%token <str> NAME ASSIGNTYPE
%token <str> REG_ID REG_NUM COND_TNAME DECOR
%token <str> FARITH_OP FPUSH FPOP
%token <str> TEMP SHARES CONV_FUNC TRUNC_FUNC TRANSCEND FABS_FUNC
%token <str> BIG LITTLE
%token <str> NAME_CALL NAME_LOOKUP

%token		 ENDIANNESS COVERS INDEX
%token		 SHARES NOT LNOT FNEG THEN LOOKUP_RDC BOGUS
%token		 ASSIGN TO COLON S_E AT ADDR REG_IDX EQUATE
%token		 MEM_IDX TOK_INTEGER TOK_FLOAT FAST OPERAND
%token		 FETCHEXEC CAST_OP FLAGMACRO SUCCESSOR

%token <num> NUM
%token <dbl> FLOATNUM		// I'd prefer type double here!

%token

%left LOG_OP
%right COND_OP
%left BIT_OP
%left ARITH_OP
%left FARITH_OP
%right NOT LNOT FCHS
%left CAST_OP
%left LOOKUP_RDC
%left S_E				// Sign extend. Note it effectively has low precedence, because being a post operator,
                                                // the whole expression is already parsed, and hence is sign extended.
                                                // Another reason why ! is deprecated!
%nonassoc AT

%type <exp> exp location exp_term
%type <str> bin_oper param
%type <regtransfer> rt assign_rt
%type <typ> assigntype
%type <num> cast
%type <tab> table_expr
%type <insel> name_contract instr_name instr_elem
%type <strlist> reg_table
%type <parmlist> list_parameter func_parameter
%type <namelist> str_term str_expr str_array name_expand opstr_expr opstr_array
%type <explist> flag_list
%type <exprlist> exprstr_expr exprstr_array
%type <explist> list_actualparameter
%type <rtlist> rt_list
%type <str> esize

%%

specorasgn:
                assign_rt {
                        the_asgn = $1;
                }
        |	exp {
                        the_asgn = new Assign(
                                new Terminal(opNil),
                                $1);
                }
        |	specification
        ;

specification:
                specification parts ';'
        |	parts ';'
        ;

parts:
                instr

        |	FETCHEXEC rt_list {
                        Dict.fetchExecCycle = $2;
                }

                // Name := value
        |	constants

        |	table_assign

                // Optional one-line section declaring endianness
        |	endianness

                // Optional section describing faster versions of instructions (e.g. that don't inplement the full
                // specifications, but if they work, will be much faster)
        |	fastlist

                // Definitions of registers (with overlaps, etc)
        |	reglist

                // Declaration of "flag functions". These describe the detailed flag setting semantics for insructions
        |	flag_fnc

                // Addressing modes (or instruction operands) (optional)
        |	OPERAND operandlist { Dict.fixupParams(); }

        ;

operandlist:
                operandlist ',' operand
        |	operand
        ;

operand:
                // In the .tex documentation, this is the first, or variant kind
                // Example: reg_or_imm := { imode, rmode };
                //$1	$2	 $3		  $4		$5
                param EQUATE '{' list_parameter '}' {
                        // Note: the below copies the list of strings!
                        Dict.DetParamMap[$1].params = *$4;
                        Dict.DetParamMap[$1].kind = PARAM_VARIANT;
                        //delete $4;
                }

                // In the documentation, these are the second and third kinds
                // The third kind is described as the functional, or lambda, form
                // In terms of DetParamMap[].kind, they are PARAM_EXP unless there
                // actually are parameters in square brackets, in which case it is
                // PARAM_LAMBDA
                // Example: indexA	rs1, rs2 *i32* r[rs1] + r[rs2]
                //$1	   $2			  $3		   $4	   $5
        |	param list_parameter func_parameter assigntype exp {
                        std::map<std::string, InsNameElem*> m;
                        ParamEntry &param = Dict.DetParamMap[$1];
                        Statement* asgn = new Assign($4, new Terminal(opNil), $5);
                        // Note: The below 2 copy lists of strings (to be deleted below!)
                        param.params = *$2;
                        param.funcParams = *$3;
                        param.asgn = asgn;
                        param.kind = PARAM_ASGN;

                        if( param.funcParams.size() != 0 )
                                param.kind = PARAM_LAMBDA;
                        //delete $2;
                        //delete $3;
                }
        ;

func_parameter: '[' list_parameter ']' { $$ = $2; }
                |	{ $$ = new std::list<std::string>(); }
                ;

reglist:
                                TOK_INTEGER {
                                        bFloat = false;
                                } a_reglists
                        |	TOK_FLOAT {
                                        bFloat = true;
                                } a_reglists
                        ;

a_reglists:		a_reglists ',' a_reglist
                        |	a_reglist
                        ;

a_reglist:
                        REG_ID INDEX NUM {
                                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                                        yyerror("Name reglist decared twice\n");
                                Dict.RegMap[$1] = $3;
                        }
                |	REG_ID '[' NUM ']' INDEX NUM {
                                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                                        yyerror("Name reglist declared twice\n");
                                Dict.addRegister( $1, $6, $3, bFloat);
                        }
                |	REG_ID '[' NUM ']' INDEX NUM COVERS REG_ID TO REG_ID {
                                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                                        yyerror("Name reglist declared twice\n");
                                Dict.RegMap[$1] = $6;
                                // Now for detailed Reg information
                                if (Dict.DetRegMap.find($6) != Dict.DetRegMap.end())
                                        yyerror("Index used for more than one register\n");
                                Dict.DetRegMap[$6].s_name($1);
                                Dict.DetRegMap[$6].s_size($3);
                                Dict.DetRegMap[$6].s_address(NULL);
                                // check range is legitimate for size. 8,10
                                if ((Dict.RegMap.find($8) == Dict.RegMap.end()) || (Dict.RegMap.find($10) == Dict.RegMap.end()))
                                        yyerror("Undefined range\n");
                                else {
                                        int bitsize = Dict.DetRegMap[Dict.RegMap[$10]].g_size();
                                        for (int i = Dict.RegMap[$8]; i != Dict.RegMap[$10]; i++) {
                                                if (Dict.DetRegMap.find(i) == Dict.DetRegMap.end()) {
                                                        yyerror("Not all registers in range defined\n");
                                                        break;
                                                }
                                                bitsize += Dict.DetRegMap[i].g_size();
                                                if (bitsize > $3) {
                                                        yyerror("Range exceeds size of register\n");
                                                        break;
                                                }
                                        }
                                if (bitsize < $3)
                                        yyerror("Register size is exceeds registers in range\n");
                                        // copy information
                                }
                                Dict.DetRegMap[$6].s_mappedIndex(Dict.RegMap[$8]);
                                Dict.DetRegMap[$6].s_mappedOffset(0);
                                Dict.DetRegMap[$6].s_float(bFloat);
                        }
                |	REG_ID '[' NUM ']' INDEX NUM SHARES REG_ID AT '[' NUM TO NUM ']' {
                                if (Dict.RegMap.find($1) != Dict.RegMap.end())
                                        yyerror("Name reglist declared twice\n");
                                Dict.RegMap[$1] = $6;
                                // Now for detailed Reg information
                                if (Dict.DetRegMap.find($6) != Dict.DetRegMap.end())
                                        yyerror("Index used for more than one register\n");
                                Dict.DetRegMap[$6].s_name($1);
                                Dict.DetRegMap[$6].s_size($3);
                                Dict.DetRegMap[$6].s_address(NULL);
                                // Do checks
                                if ($3 != ($13 - $11) + 1)
                                        yyerror("Size does not equal range\n");
                                        if (Dict.RegMap.find($8) != Dict.RegMap.end()) {
                                                if ($13 >= Dict.DetRegMap[Dict.RegMap[$8]].g_size())
                                                        yyerror("Range extends over target register\n");
                                        } else
                                                yyerror("Shared index not yet defined\n");
                                Dict.DetRegMap[$6].s_mappedIndex(Dict.RegMap[$8]);
                                Dict.DetRegMap[$6].s_mappedOffset($11);
                                Dict.DetRegMap[$6].s_float(bFloat);
                        }
                |	'[' reg_table ']' '[' NUM ']' INDEX NUM TO NUM {
                                if ((int)$2->size() != ($10 - $8 + 1)) {
                                        std::cerr << "size of register array does not match mapping to r[" << $8 << ".." << $10 << "]\n";
                                        exit(1);
                                } else {
                                        std::list<std::string>::iterator loc = $2->begin();
                                        for (int x = $8; x <= $10; x++, loc++) {
                                                if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                                                        yyerror("Name reglist declared twice\n");
                                                Dict.addRegister( loc->c_str(), x, $5, bFloat);
                                        }
                                        //delete $2;
                                }
                        }
                |	'[' reg_table ']' '[' NUM ']' INDEX NUM {
                                std::list<std::string>::iterator loc = $2->begin();
                                for (; loc != $2->end(); loc++) {
                                        if (Dict.RegMap.find(*loc) != Dict.RegMap.end())
                                                yyerror("Name reglist declared twice\n");
                                        Dict.addRegister(loc->c_str(), $8, $5, bFloat);
                                }
                                //delete $2;
                        }
                ;

reg_table:
                        reg_table ',' REG_ID {
                                $1->push_back($3);
                        }
                |	REG_ID {
                                $$ = new std::list<std::string>;
                                $$->push_back($1);
                        }
                ;

// Flag definitions
flag_fnc:
                        // $1		   $2		 $3	 $4	   $5	 $6
                        NAME_CALL list_parameter ')' '{' rt_list '}' {
                                // Note: $2 is a list of strings
                                Dict.FlagFuncs[$1] = new FlagDef(listStrToExp($2), $5);
                        }
                ;

constants:
                        NAME EQUATE NUM {
                                if (ConstTable.find($1) != ConstTable.end())
                                        yyerror("Constant declared twice");
                                ConstTable[std::string($1)] = $3;
                        }

                |	NAME EQUATE NUM ARITH_OP NUM {
                                if (ConstTable.find($1) != ConstTable.end())
                                        yyerror("Constant declared twice");
                                else if ($4 == std::string("-"))
                                        ConstTable[std::string($1)] = $3 - $5;
                                else if ($4 == std::string("+"))
                                        ConstTable[std::string($1)] = $3 + $5;
                                else
                                        yyerror("Constant expression must be NUM + NUM or NUM - NUM");
                        }
                ;


table_assign:
                NAME EQUATE table_expr {
                        TableDict[$1] = $3;
                }
        ;

table_expr:
                str_expr {
                        $$ = new Table(*$1);
                        //delete $1;
                }
                // Example: OP2 := { "<<",	">>",  ">>A" };
        |	opstr_expr {
                        $$ = new OpTable(*$1);
                        //delete $1;
                }
        |	exprstr_expr {
                        $$ = new ExprTable(*$1);
                        //delete $1;
                }
        ;

str_expr:
                str_expr str_term {
                        // cross-product of two str_expr's
                        std::deque<std::string>::iterator i, j;
                        $$ = new std::deque<std::string>;
                        for (i = $1->begin(); i != $1->end(); i++)
                                for (j = $2->begin(); j != $2->end(); j++)
                                        $$->push_back((*i) + (*j));
                        //delete $1;
                        //delete $2;
                }
        |	str_term {
                        $$ = $1;
                }
        ;

str_array:
                str_array ',' str_expr {
                        // want to append $3 to $1
                        // The following causes a massive warning message about mixing signed and unsigned
                        $1->insert($1->end(), $3->begin(), $3->end());
                        //delete $3;
                        $$ = $1;
                }
        |	str_array ',' '"' '"' {
                        $1->push_back("");
                }
        |	str_expr {
                        $$ = $1;
                }
        ;

str_term:
                '{' str_array '}' {
                        $$ = $2;
                }
        |	name_expand {
                        $$ = $1;
                }
        ;

name_expand:
                '\'' NAME '\'' {
                        $$ = new std::deque<std::string>;
                        $$->push_back("");
                        $$->push_back($2);
                }
        |	'"' NAME '"' {
                        $$ = new std::deque<std::string>(1, $2);
                }
        |	'$' NAME {
                        std::ostringstream o;
                        // expand $2 from table of names
                        if (TableDict.find($2) != TableDict.end())
                                if (TableDict[$2]->getType() == NAMETABLE)
                                        $$ = new std::deque<std::string>(TableDict[$2]->Records);
                                else {
                                        o << "name " << $2 << " is not a NAMETABLE.\n";
                                        yyerror(STR(o));
                                }
                        else {
                                o << "could not dereference name " << $2 << "\n";
                                yyerror(STR(o));
                        }
                }
        |	NAME {
                        // try and expand $1 from table of names. if fail, expand using '"' NAME '"' rule
                        if (TableDict.find($1) != TableDict.end())
                                if (TableDict[$1]->getType() == NAMETABLE)
                                        $$ = new std::deque<std::string>(TableDict[$1]->Records);
                                else {
                                        std::ostringstream o;
                                        o << "name " << $1 << " is not a NAMETABLE.\n";
                                        yyerror(STR(o));
                                }
                        else {
                                $$ = new std::deque<std::string>;
                                $$->push_back($1);
                        }
                }
        ;

bin_oper:
                BIT_OP {
                        $$ = $1;
                }

        |	ARITH_OP {
                        $$ = $1;
                }

        |	FARITH_OP {
                        $$ = $1;
                }
        ;

                // Example: OP2 := { "<<",	">>",  ">>A" };
opstr_expr:
                '{' opstr_array '}' {
                        $$ = $2;
                }
        ;

opstr_array:
                //	$1		$2	$3	  $4	 $5
                opstr_array ',' '"' bin_oper '"' {
                        $$ = $1;
                        $$->push_back($4);
                }
        |	'"' bin_oper '"' {
                        $$ = new std::deque<std::string>;
                        $$->push_back($2);
                }
        ;

                // Example: COND1_C := { "~%ZF", "%ZF", "~(%ZF | (%NF ^ %OF))", ...
exprstr_expr:
                '{' exprstr_array '}' {
                        $$ = $2;
                }
        ;

exprstr_array:
                // $1		  $2  $3  $4  $5
                exprstr_array ',' '"' exp '"' {
                        $$ = $1;
                        $$->push_back($4);
                }
        |	'"' exp '"' {
                        $$ = new std::deque<Exp*>;
                        $$->push_back($2);
                }
        ;

instr:
                //	$1
                instr_name {
                        $1->getrefmap(indexrefmap);
                //	   $3			$4
                } list_parameter rt_list {
                        // This function expands the tables and saves the expanded RTLs to the dictionary
                        expandTables($1, $3, $4, Dict);
                }
        ;

instr_name:
                instr_elem {
                        $$ = $1;
                }
        |	instr_name DECOR {
                        std::string::size_type i;
                        InsNameElem *temp;
                        std::string nm = $2;

                        if (nm[0] == '^')
                                nm.replace(0, 1, "");

                        // remove all " and _, from the decoration
                        while ((i = nm.find("\"")) != nm.npos)
                                nm.replace(i,1,"");
                        // replace all '.' with '_'s from the decoration
                        while ((i = nm.find(".")) != nm.npos)
                                nm.replace(i,1,"_");
                        while ((i = nm.find("_")) != nm.npos)
                                nm.replace(i,1,"");

                        temp = new InsNameElem(nm.c_str());
                        $$ = $1;
                        $$->append(temp);
                }
        ;

instr_elem:
                NAME {
                        $$ = new InsNameElem($1);
                }
        |	name_contract {
                        $$ = $1;
                }
        |	instr_elem name_contract {
                        $$ = $1;
                        $$->append($2);
                }
        ;

name_contract:
                '\'' NAME '\'' {
                        $$ = new InsOptionElem($2);
                }
        |	NAME_LOOKUP NUM ']' {
                        std::ostringstream o;
                        if (TableDict.find($1) == TableDict.end()) {
                                o << "Table " << $1 << " has not been declared.\n";
                                yyerror(STR(o));
                        } else if (($2 < 0) || ($2 >= (int)TableDict[$1]->Records.size())) {
                                o << "Can't get element " << $2 << " of table " << $1 << ".\n";
                                yyerror(STR(o));
                        } else
                                $$ = new InsNameElem(TableDict[$1]->Records[$2].c_str());
                }

                // Example: ARITH[IDX]	where ARITH := { "ADD", "SUB", ...};
        |	NAME_LOOKUP NAME ']' {
                        std::ostringstream o;
                        if (TableDict.find($1) == TableDict.end()) {
                                o << "Table " << $1 << " has not been declared.\n";
                                yyerror(STR(o));
                        } else
                                $$ = new InsListElem($1, TableDict[$1], $2);
                }

        |	'$' NAME_LOOKUP NUM ']' {
                        std::ostringstream o;
                        if (TableDict.find($2) == TableDict.end()) {
                                o << "Table " << $2 << " has not been declared.\n";
                                yyerror(STR(o));
                        } else if (($3 < 0) || ($3 >= (int)TableDict[$2]->Records.size())) {
                                o << "Can't get element " << $3 << " of table " << $2 << ".\n";
                                yyerror(STR(o));
                        } else
                                $$ = new InsNameElem(TableDict[$2]->Records[$3].c_str());
                }
        |	'$' NAME_LOOKUP NAME ']' {
                        std::ostringstream o;
                        if (TableDict.find($2) == TableDict.end()) {
                                o << "Table " << $2 << " has not been declared.\n";
                                yyerror(STR(o));
                        } else
                                $$ = new InsListElem($2, TableDict[$2], $3);
                }

        |	'"' NAME '"' {
                        $$ = new InsNameElem($2);
                }
        ;

rt_list:
                rt_list rt {
                        // append any automatically generated register transfers and clear the list they were stored in.
                        // Do nothing for a NOP (i.e. $2 = 0)
                        if ($2 != NULL) {
                                $1->appendStmt($2);
                        }
                        $$ = $1;
                }

        |	rt {
                        $$ = std::make_shared<RTL>(ADDRESS::g(0L)); // WARN: the code here was RTL(STMT_ASSIGN), which is not right, since RTL parameter is an address
                        if ($1 != NULL)
                                $$->appendStmt($1);
                }
        ;

rt:
                assign_rt {
                        $$ = $1;
                }

                // Example: ADDFLAGS(r[tmp], reg_or_imm, r[rd])
                // $1			  $2		   $3
        |	NAME_CALL list_actualparameter ')' {
                        std::ostringstream o;
                        if (Dict.FlagFuncs.find($1) != Dict.FlagFuncs.end()) {
                                // Note: SETFFLAGS assigns to the floating point flags. All others to the integer flags
                                bool bFloat = strcmp($1, "SETFFLAGS") == 0;
                                OPER op = bFloat ? opFflags : opFlags;
                                $$ = new Assign(
                                        new Terminal(op),
                                        new Binary(opFlagCall,
                                                new Const($1),
                                                listExpToExp($2)));
                        } else {
                                o << $1 << " is not declared as a flag function.\n";
                                yyerror(STR(o));
                        }
                }
        |	FLAGMACRO flag_list ')' {
                        $$ = 0;
                }
                // E.g. undefineflags() (but we don't handle this yet... flags are changed, but not in a way we support)
        |	FLAGMACRO ')' {
                        $$ = 0;
                }
        |  '_' {
                $$ = NULL;
        }
        ;

flag_list:
                flag_list ',' REG_ID {
                        // Not sure why the below is commented out (MVE)
/*			Location* pFlag = Location::regOf(Dict.RegMap[$3]);
                        $1->push_back(pFlag);
                        $$ = $1;
*/			$$ = 0;
                }
        |	REG_ID {
/*			std::list<Exp*>* tmp = new std::list<Exp*>;
                        Unary* pFlag = new Unary(opIdRegOf, Dict.RegMap[$1]);
                        tmp->push_back(pFlag);
                        $$ = tmp;
*/			$$ = 0;
                }
        ;

                // Note: this list is a list of strings (other code needs this)
list_parameter:
                list_parameter ',' param {
                        assert($3 != 0);
                        $1->push_back($3);
                        $$ = $1;
                }

        |	param {
                        $$ = new std::list<std::string>;
                        $$->push_back($1);
                }
        |	{
                        $$ = new std::list<std::string>;
                }
        ;

param:	NAME {
                        Dict.ParamSet.insert($1);		// MVE: Likely wrong. Likely supposed to be OPERAND params only
                        $$ = $1;
                }

list_actualparameter:
                list_actualparameter ',' exp {
                        $$->push_back($3);
                }

        |	exp {
                        $$ = new std::list<Exp*>;
                        $$->push_back($1);
                }

        |	{
                        $$ = new std::list<Exp*>;
                }
        ;

assign_rt:
                // Size	  guard =>	  lhs	  :=    rhs
                //	$1	   $2		   $4			$6
                assigntype exp THEN location EQUATE exp {
                        Assign* a = new Assign($1, $4, $6);
                        a->setGuard($2);
                        $$ = a;
                }
                // Size		lhs		:=	 rhs
                // $1		$2		$3	 $4
        |	assigntype location EQUATE exp {
                        // update the size of any generated RT's
                        $$ = new Assign($1, $2, $4);
                }

                // FPUSH and FPOP are special "transfers" with just a Terminal
        |	FPUSH {
                        $$ = new Assign(
                                new Terminal(opNil),
                                new Terminal(opFpush));
                }
        |	FPOP {
                        $$ = new Assign(
                                new Terminal(opNil),
                                new Terminal(opFpop));
                }
                // Just a RHS? Is this used? Note: flag calls are handled at the rt: level
                //	$1		$2
        |	assigntype exp {
                        $$ = new Assign($1, NULL, $2);
                }
        ;

exp_term:
                NUM {
                        $$ = new Const($1);
                }

        |	FLOATNUM {
                        $$ = new Const($1);
                }

        |	'(' exp ')' {
                        $$ = $2;
                }

        |	location {
                        $$ = $1;
                }

        |	'[' exp '?' exp COLON exp ']' {
                        $$ = new Ternary(opTern, $2, $4, $6);
                }

        // Address-of, for LEA type instructions
        |	ADDR exp ')' {
                        $$ = new Unary(opAddrOf, $2);
                }

        // Conversion functions, e.g. fsize(32, 80, modrm). Args are FROMsize, TOsize, EXPression
        |	CONV_FUNC NUM ',' NUM ',' exp ')' {
                        $$ = new Ternary(strToOper($1), new Const($2), new Const($4), $6);
                }

        // Truncation function: ftrunc(3.01) == 3.00
        |	TRUNC_FUNC exp ')' {
                        $$ = new Unary(opFtrunc, $2);
                }

        // fabs function: fabs(-3.01) == 3.01
        |	FABS_FUNC exp ')' {
                        $$ = new Unary(opFabs, $2);
                }

        // FPUSH and FPOP
        |	FPUSH {
                        $$ = new Terminal(opFpush);
                }
        |	FPOP {
                        $$ = new Terminal(opFpop);
                }

        // Transcendental functions
        |	TRANSCEND exp ')' {
                        $$ = new Unary(strToOper($1), $2);
                }

                // Example: *Use* of COND[idx]
                //	 $1		 $2
        |	NAME_LOOKUP NAME ']' {
                        std::ostringstream o;
                        if (indexrefmap.find($2) == indexrefmap.end()) {
                                o << "index " << $2 << " not declared for use.\n";
                                yyerror(STR(o));
                        } else if (TableDict.find($1) == TableDict.end()) {
                                o << "table " << $1 << " not declared for use.\n";
                                yyerror(STR(o));
                        } else if (TableDict[$1]->getType() != EXPRTABLE) {
                                o << "table " << $1 << " is not an expression table but appears to be used as one.\n";
                                yyerror(STR(o));
                        } else if (((ExprTable*)TableDict[$1])->expressions.size() < indexrefmap[$2]->ntokens()) {
                                o << "table " << $1 << " (" << ((ExprTable*)TableDict[$1])->expressions.size() <<
                                        ") is too small to use " << $2 << " (" << indexrefmap[$2]->ntokens() << ") as an index.\n";
                                yyerror(STR(o));
                        }
                        // $1 is a map from string to Table*; $2 is a map from string to InsNameElem*
                        $$ = new Binary(opExpTable, new Const($1), new Const($2));
                }

                // This is a "lambda" function-like parameter
                // $1 is the "function" name, and $2 is a list of Exp* for the actual params.
                // I believe only PA/RISC uses these so far.
        |	NAME_CALL list_actualparameter ')' {
                std::ostringstream o;
                if (Dict.ParamSet.find($1) != Dict.ParamSet.end() ) {
                        if (Dict.DetParamMap.find($1) != Dict.DetParamMap.end()) {
                                ParamEntry& param = Dict.DetParamMap[$1];
                                if ($2->size() != param.funcParams.size() ) {
                                        o << $1 << " requires " << param.funcParams.size() << " parameters, but received " << $2->size()
                                                << ".\n";
                                        yyerror(STR(o));
                                } else {
                                        // Everything checks out. *phew*
                                        // Note: the below may not be right! (MVE)
                                        $$ = new Binary(opFlagDef,
                                                        new Const($1),
                                                        listExpToExp($2));
                                        //delete $2;			// Delete the list of char*s
                                }
                        } else {
                                o << $1 << " is not defined as a OPERAND function.\n";
                                yyerror(STR(o));
                        }
                } else {
                        o << "Unrecognized name " << $1 << " in lambda call.\n";
                }
        }

        |		SUCCESSOR exp ')' {
                        $$ = makeSuccessor($2);
                }
        ;

exp:
                exp S_E {
                        $$ = new Unary(opSignExt, $1);
                }

                // "%prec CAST_OP" just says that this operator has the precedence of the dummy terminal CAST_OP
                // It's a "precedence modifier" (see "Context-Dependent Precedence" in the Bison documantation)
          // $1	 $2
        |	exp cast %prec CAST_OP {
                        // size casts and the opSize operator were generally deprecated, but now opSize is used to transmit
                        // the size of operands that could be memOfs from the decoder to type analysis
                        if ($2 == STD_SIZE)
                                $$ = $1;
                        else
                                $$ = new Binary(opSize, new Const($2), $1);
                }

        |	NOT exp {
                        $$ = new Unary(opNot, $2);
                }

        |	LNOT exp {
                        $$ = new Unary(opLNot, $2);
                }

        |	FNEG exp {
                        $$ = new Unary(opFNeg, $2);
                }

        |	exp FARITH_OP exp {
                        $$ = new Binary(strToOper($2), $1, $3);
                }

        |	exp ARITH_OP exp {
                        $$ = new Binary(strToOper($2), $1, $3);
                }

        |	exp BIT_OP exp {
                        $$ = new Binary(strToOper($2), $1, $3);
                }

        |	exp COND_OP exp {
                        $$ = new Binary(strToOper($2), $1, $3);
                }

        |	exp LOG_OP exp {
                        $$ = new Binary(strToOper($2), $1, $3);
                }

                // See comment above re "%prec LOOKUP_RDC"
                // Example: OP1[IDX] where OP1 := {	 "&",  "|", "^", ...};
                //$1	 $2		 $3	 $4	   $5
        |	exp NAME_LOOKUP NAME ']' exp_term %prec LOOKUP_RDC {
                        std::ostringstream o;
                        if (indexrefmap.find($3) == indexrefmap.end()) {
                                o << "index " << $3 << " not declared for use.\n";
                                yyerror(STR(o));
                        } else if (TableDict.find($2) == TableDict.end()) {
                                o << "table " << $2 << " not declared for use.\n";
                                yyerror(STR(o));
                        } else if (TableDict[$2]->getType() != OPTABLE) {
                                o << "table " << $2 << " is not an operator table but appears to be used as one.\n";
                                yyerror(STR(o));
                        } else if (TableDict[$2]->Records.size() < indexrefmap[$3]->ntokens()) {
                                o << "table " << $2 << " is too small to use with " << $3 << " as an index.\n";
                                yyerror(STR(o));
                        }
                        $$ = new Ternary(opOpTable, new Const($2), new Const($3),
                                new Binary(opList,
                                        $1,
                                        new Binary(opList,
                                                $5,
                                                new Terminal(opNil))));
                }

        |	exp_term {
                        $$ = $1;
                }
        ;

location:
                // This is for constant register numbers. Often, these are special, in the sense that the register mapping
                // is -1. If so, the equivalent of a special register is generated, i.e. a Terminal or opMachFtr
                // (machine specific feature) representing that register.
                REG_ID {
                        bool isFlag = strstr($1, "flags") != 0;
                        std::map<std::string, int>::const_iterator it = Dict.RegMap.find($1);
                        if (it == Dict.RegMap.end() && !isFlag) {
                                std::ostringstream ost;
                                ost << "register `" << $1 << "' is undefined";
                                yyerror(STR(ost));
                        } else if (isFlag || it->second == -1) {
                                // A special register, e.g. %npc or %CF. Return a Terminal for it
                                OPER op = strToTerm($1);
                                if (op) {
                                        $$ = new Terminal(op);
                                } else {
                                        $$ = new Unary(opMachFtr,	 // Machine specific feature
                                                        new Const($1));
                                }
                        }
                        else {
                                // A register with a constant reg nmber, e.g. %g2.  In this case, we want to return r[const 2]
                                $$ = Location::regOf(it->second);
                        }
                }

        |	REG_IDX exp ']' {
                        $$ = Location::regOf($2);
                }

        |	REG_NUM {
                        int regNum;
                        sscanf($1, "r%d", &regNum);
                        $$ = Location::regOf(regNum);
                }

        |	MEM_IDX exp ']' {
                        $$ = Location::memOf($2);
                }

        |	NAME {
                // This is a mixture of the param: PARM {} match and the value_op: NAME {} match
                        Exp* s;
                        std::set<std::string>::iterator it = Dict.ParamSet.find($1);
                        if (it != Dict.ParamSet.end()) {
                                s = new Location(opParam, new Const($1), NULL);
                        } else if (ConstTable.find($1) != ConstTable.end()) {
                                s = new Const(ConstTable[$1]);
                        } else {
                                std::ostringstream ost;
                                ost << "`" << $1 << "' is not a constant, definition or a parameter of this instruction\n";
                                yyerror(STR(ost));
                                s = new Const(0);
                        }
                        $$ = s;
                }

        |	   exp AT '[' exp COLON exp ']' {
                        $$ = new Ternary(opAt, $1, $4, $6);
                }

        |	TEMP {
                        $$ = Location::tempOf(new Const($1));
                }

                // This indicates a post-instruction marker (var tick)
        |	   location '\'' {
                        $$ = new Unary(opPostVar, $1);
                }
        |		SUCCESSOR exp ')' {
                        $$ = makeSuccessor($2);
                }
        ;

cast:
                '{' NUM '}' {
                        $$ = $2;
                }
        ;

endianness:
                ENDIANNESS esize {
                        Dict.bigEndian = (strcmp($2, "BIG") == 0);
                }

esize:
                BIG {
                        $$ = $1;
                }
        |	LITTLE {
                        $$ = $1;
                }
        ;

assigntype:
                ASSIGNTYPE {
                        char c = $1[1];
                        if (c == '*') $$ = new SizeType(0); // MVE: should remove these
                        else if (isdigit(c)) {
                                int size;
                                // Skip star (hence +1)
                                sscanf($1+1, "%d", &size);
                                $$ = new SizeType(size);
                        } else {
                                int size;
                                // Skip star and letter
                                sscanf($1+2, "%d", &size);
                                if (size == 0) size = STD_SIZE;
                                switch (c) {
                                        case 'i': $$ = new IntegerType(size, 1); break;
                                        case 'j': $$ = new IntegerType(size, 0); break;
                                        case 'u': $$ = new IntegerType(size, -1); break;
                                        case 'f': $$ = new FloatType(size); break;
                                        case 'c': $$ = new CharType; break;
                                        default:
                                                std::cerr << "Unexpected char " << c << " in assign type\n";
                                                $$ = new IntegerType;
                                }
                        }
                }

// Section for indicating which instructions to substitute when using -f (fast but not quite as exact instruction
// mapping)
fastlist:
                FAST fastentries
        ;

fastentries:
                fastentries ',' fastentry

        |	fastentry

        ;

fastentry:
                NAME INDEX NAME {
                        Dict.fastMap[std::string($1)] = std::string($3);
                }
%%
