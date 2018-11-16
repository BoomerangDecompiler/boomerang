#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


# include "SSL2Parser.hpp"

# include <map>
# include <string>


// Tell Flex the lexer's prototype ...
# define YY_DECL \
yy::parser::symbol_type yylex (SSL2ParserDriver& drv)
// ... and declare it for the parser's sake.
YY_DECL;


class RTLInstDict;


/**
 * Class for parsing an SSL specification file.
 */
class SSL2ParserDriver
{
public:
    SSL2ParserDriver(RTLInstDict *dict);

    /// Parse the file with name eturn 0 on success.
    int parse(const std::string &fileName);

    static BOOMERANG_API Statement *parseExp(const char *str, bool verboseOutput);

public:
    OPER strToOper(const QString &s);


public:
    RTLInstDict *m_dict;
    SharedExp result;

    // The token's location used by the scanner.
    yy::location location;

    SharedExp makeSuccessor(SharedExp e);
    bool expandTables(const std::shared_ptr<InsNameElem> &iname,
                      const std::shared_ptr<std::list<QString>> &params,
                      SharedRTL o_rtlist, RTLInstDict *dict);

    /// Result for parsing an assignment.
    Statement *the_asgn;

    /// Maps SSL constants to their values.
    std::map<QString, int> ConstTable;

    /// maps index names to instruction name-elements
    std::map<QString, std::shared_ptr<InsNameElem>> indexrefmap;

    /// Maps table names to Tables.
    std::map<QString, std::shared_ptr<Table>> TableDict;

    /// True when FLOAT keyword seen; false when INTEGER keyword seen (in @REGISTER section)
    bool bFloat;

private:
    // Handling the scanner.
    bool scanBegin();
    void scanEnd();

private:
    std::string file;     ///< The name of the file being parsed.
    bool trace_parsing;   ///< Whether to generate parser debug traces.
    bool trace_scanning;  ///< Whether to generate scanner debug traces.
};
