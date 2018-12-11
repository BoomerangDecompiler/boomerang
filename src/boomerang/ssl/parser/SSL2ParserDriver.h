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


#include "SSL2Parser.hpp"

#include "boomerang/ssl/Register.h"

#include <map>
#include <string>


// Tell Flex the lexer's prototype ...
#define YY_DECL SSL2::parser::symbol_type SSL2lex(SSL2ParserDriver &drv)
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

public:
    OPER strToOper(const QString &s);

public:
    RTLInstDict *m_dict;
    SharedExp result;

    // The token's location used by the scanner.
    SSL2::location location;

    SharedExp makeSuccessor(SharedExp e);
    bool expandTables(const std::shared_ptr<InsNameElem> &iname,
                      const std::shared_ptr<std::list<QString>> &params, SharedRTL o_rtlist,
                      RTLInstDict *dict);

    /// Maps SSL constants to their values.
    std::map<QString, int> ConstTable;

    /// maps index names to instruction name-elements
    std::map<QString, std::shared_ptr<InsNameElem>> indexrefmap;

    /// Maps table names to Tables.
    std::map<QString, std::shared_ptr<Table>> TableDict;

    RegType m_regType = RegType::Invalid; ///< Type of the last/current register encountered

private:
    // Handling the scanner.
    bool scanBegin();
    void scanEnd();

private:
    std::string file;    ///< The name of the file being parsed.
    bool trace_parsing;  ///< Whether to generate parser debug traces.
    bool trace_scanning; ///< Whether to generate scanner debug traces.
};
