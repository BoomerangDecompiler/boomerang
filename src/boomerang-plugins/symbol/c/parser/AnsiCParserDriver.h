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


#include "AnsiCParser.hpp"

#include "boomerang/core/BoomerangAPI.h"


// Tell Flex the lexer's prototype ...
#define YY_DECL AnsiC::parser::symbol_type AnsiClex(AnsiCParserDriver &drv)
// ... and declare it for the parser's sake.
YY_DECL;


/**
 * Class for parsing an SSL specification file.
 */
class AnsiCParserDriver
{
public:
    AnsiCParserDriver();

    /// Parse the file with name. return 0 on success.
    int parse(const QString &fileName, Machine machine, CallConv cc);

public:
    // The token's location used by the scanner.
    AnsiC::location location;

    Machine plat = Machine::INVALID;
    CallConv cc  = CallConv::INVALID;

    std::list<std::shared_ptr<Signature>> signatures;
    std::list<std::shared_ptr<Symbol>> symbols;
    std::list<std::shared_ptr<SymbolRef>> refs;

private:
    // Handling the scanner.
    bool scanBegin();
    void scanEnd();

private:
    std::string file;    ///< The name of the file being parsed.
    bool trace_parsing;  ///< Whether to generate parser debug traces.
    bool trace_scanning; ///< Whether to generate scanner debug traces.
};
