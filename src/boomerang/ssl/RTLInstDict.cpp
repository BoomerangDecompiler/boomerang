#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RTLInstDict.h"

#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/parser/SSL2ParserDriver.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/BranchStatement.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/CaseStatement.h"
#include "boomerang/ssl/statements/GotoStatement.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/log/Log.h"


RTLInstDict::RTLInstDict(bool verboseOutput)
    : m_verboseOutput(verboseOutput)
    , m_endianness(Endian::Little)
{
}


RTLInstDict::~RTLInstDict()
{
}


int RTLInstDict::insert(const QString &name, std::list<QString> &params, const RTL &rtl)
{
    QString opcode = name.toUpper();

    opcode.remove(".");

    if (m_instructions.find({ opcode, params.size() }) == m_instructions.end()) {
        std::pair<QString, int> key{ opcode, params.size() };
        m_instructions.emplace(key, TableEntry(params, rtl));
    }
    else {
        return m_instructions[{ opcode, params.size() }].appendRTL(params, rtl);
    }

    return 0;
}


bool RTLInstDict::readSSLFile(const QString &sslFileName)
{
    LOG_MSG("Loading machine specifications from '%1'...", sslFileName);
    // emptying the rtl dictionary
    m_instructions.clear();

    // Clear all state
    reset();

    SSL2ParserDriver drv(this);

    if (drv.parse(sslFileName.toStdString()) != 0) {
        return false;
    }

    if (m_verboseOutput) {
        QString s;
        OStream os(&s);
        print(os);

        LOG_VERBOSE("");
        LOG_VERBOSE("=======Expanded RTL template dictionary=======");
        LOG_VERBOSE("%1", s);
        LOG_VERBOSE("==============================================");
        LOG_VERBOSE("");
    }

    return true;
}


void RTLInstDict::print(OStream &os /*= std::cout*/)
{
    for (auto &elem : m_instructions) {
        // print the instruction name
        os << (elem).first.first << "  ";

        // print the parameters
        const std::list<QString> &params((elem).second.m_params);
        int i = params.size();

        for (auto s = params.begin(); s != params.end(); ++s, i--) {
            os << *s << (i != 1 ? "," : "");
        }

        os << "\n";

        // print the RTL
        RTL &rtlist = (elem).second.m_rtl;
        rtlist.print(os);
        os << "\n";
    }
}


std::unique_ptr<RTL> RTLInstDict::instantiateRTL(const QString &name, Address natPC,
                                                 const std::vector<SharedExp> &args)
{
    // TODO try to retrieve fast instruction mappings
    // before trying the verbose instructions
    auto dict_entry = m_instructions.find({ name, args.size() });
    if (dict_entry == m_instructions.end()) {
        LOG_ERROR("Cannot instantiate instruction '%1' at address %2: "
                  "No instruction template takes %3 arguments",
                  name, natPC, args.size());
        return nullptr; // instruction not found
    }

    TableEntry &entry(dict_entry->second);
    return instantiateRTL(entry.m_rtl, natPC, entry.m_params, args);
}


std::unique_ptr<RTL> RTLInstDict::instantiateRTL(const RTL &existingRTL, Address natPC,
                                                 const std::list<QString> &params,
                                                 const std::vector<SharedExp> &args)
{
    assert(params.size() == args.size());

    // Get a deep copy of the template RTL
    std::unique_ptr<RTL> newList(new RTL(existingRTL));
    newList->setAddress(natPC);

    // Iterate through each Statement of the new list of stmts
    for (SharedStmt ss : *newList) {
        // Search for the formals and replace them with the actual arguments
        auto arg = args.begin();

        for (QString paramName : params) {
            /* Simple parameter - just construct the formal to search for */
            Location param(opParam, Const::get(paramName), nullptr);
            ss->searchAndReplace(param, *arg);
            ++arg;
        }
        assert(arg == args.end());
        fixSuccessorForStmt(ss);

        if (m_verboseOutput) {
            LOG_MSG("            %1", ss);
        }
    }

    // Perform simplifications, e.g. *1 in x86 addressing modes
    for (SharedStmt &s : *newList) {
        s->simplify();

        // Fixup for goto, case, branch, and call
        if (s->isGoto()) {
            std::shared_ptr<GotoStatement> jump = s->as<GotoStatement>();

            if (jump->getDest()->isIntConst()) {
                jump->setIsComputed(false);
            }
            else {
                SharedExp dest = jump->getDest();
                s.reset();
                std::shared_ptr<CaseStatement> caseStmt(new CaseStatement);
                caseStmt->setDest(dest);
                caseStmt->setIsComputed(true);
                s = caseStmt;
            }
        }
        else if (s->isBranch()) {
            std::shared_ptr<BranchStatement> branch = s->as<BranchStatement>();
            branch->setIsComputed(!branch->getDest()->isIntConst());
        }
        else if (s->isCall()) {
            std::shared_ptr<CallStatement> call = s->as<CallStatement>();
            if (call->getDest()) {
                call->setDest(call->getDest()->simplify());
                call->setIsComputed(!call->getDest()->isIntConst());
            }
            else {
                call->setIsComputed(true);
            }
        }
    }

    return newList;
}


void RTLInstDict::fixSuccessorForStmt(const SharedStmt &stmt)
{
    if (!stmt->isAssign()) {
        return;
    }

    std::shared_ptr<Assign> asgn = stmt->as<Assign>();
    asgn->setLeft(asgn->getLeft()->fixSuccessor());
    asgn->setRight(asgn->getRight()->fixSuccessor());
}


void RTLInstDict::reset()
{
    m_regDB.clear();

    m_definedParams.clear();
    m_flagFuncs.clear();
    m_instructions.clear();
}


RegDB *RTLInstDict::getRegDB()
{
    return &m_regDB;
}


const RegDB *RTLInstDict::getRegDB() const
{
    return &m_regDB;
}
