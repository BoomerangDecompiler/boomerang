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


#include "boomerang/ssl/RegDB.h"
#include "boomerang/ssl/Register.h"
#include "boomerang/ssl/TableEntry.h"
#include "boomerang/ssl/parser/SSL2Parser.hpp"
#include "boomerang/util/ByteUtil.h"

#include <map>
#include <set>
#include <vector>


class Exp;
class Statement;
class Type;
class OStream;


using SharedExp = std::shared_ptr<Exp>;


/**
 * The RTLInstDict represents a dictionary that maps unique instruction names to the
 * parameters they take and a template of their semantics (as an RTL).
 * These instruction semantics templates are populated by \ref readSSLFile;
 * concrete instruction semantics are instantiated via \ref instantiateRTL.
 */
class BOOMERANG_API RTLInstDict
{
    friend class SSL2ParserDriver;
    friend class SSL2::parser;

public:
    RTLInstDict(bool verboseOutput = false);
    RTLInstDict(const RTLInstDict &) = delete;
    RTLInstDict(RTLInstDict &&)      = default;

    ~RTLInstDict();

    RTLInstDict &operator=(const RTLInstDict &) = delete;
    RTLInstDict &operator=(RTLInstDict &&) = default;

public:
    /**
     * Read and parse the SSL file, and initialise the expanded instruction dictionary
     * (this object). This also reads and sets up the register map and flag functions.
     *
     * \param sslFileName the name of the file containing the SSL specification.
     * \returns           true if the file was read successfully.
     */
    bool readSSLFile(const QString &sslFileName);

    /**
     * Returns a new RTL containing the semantics of the instruction with name \p name.
     *
     * \param name    the name of the instruction (must correspond to one defined in the SSL file).
     * \param pc      address at which the named instruction is located
     * \param args    the actual values of the instruction parameters
     */
    std::unique_ptr<RTL> instantiateRTL(const QString &name, Address pc,
                                        const std::vector<SharedExp> &args);

    RegDB *getRegDB();
    const RegDB *getRegDB() const;

private:
    /// Reset the object to "undo" a readSSLFile()
    void reset();

    /**
     * Returns an instance of a register transfer list for the parameterized rtlist with the given
     * formals replaced with the arguments given as the third parameter.
     *
     * \param   rtls    a register transfer list
     * \param   pc      address at which the named instruction is located
     * \param   params  a list of formal parameters
     * \param   args    the actual parameter values
     * \returns the instantiated list of Exps
     */
    std::unique_ptr<RTL> instantiateRTL(const RTL &rtls, Address pc,
                                        const std::list<QString> &params,
                                        const std::vector<SharedExp> &args);

    /**
     * Appends one RTL to the dictionary, or adds it to idict if an
     * entry does not already exist.
     *
     * \param name       name of the instruction to add to
     * \param parameters list of formal parameters (as strings) for the RTL to add
     * \param rtl        reference to the RTL to add
     * \returns zero for success, non-zero for failure
     */
    int insert(const QString &name, std::list<QString> &parameters, const RTL &rtl);

    /// Print a textual representation of the dictionary.
    void print(OStream &os);

    /// Replace opSuccessor by real semantics in \p stmt.
    void fixSuccessorForStmt(const SharedStmt &stmt);

private:
    /// Print messages when reading an SSL file or when instantiaing an instruction
    bool m_verboseOutput;

    /// Endianness of the source machine
    Endian m_endianness;

    RegDB m_regDB;

    /// During parsing, this contains the parameters of the currently parsed
    /// flag function or instruction.
    std::set<QString> m_definedParams;

    /// All names of defined flag functions
    std::set<QString> m_flagFuncs;

    /// The actual instruction dictionary.
    std::map<std::pair<QString, int>, TableEntry> m_instructions;
};
