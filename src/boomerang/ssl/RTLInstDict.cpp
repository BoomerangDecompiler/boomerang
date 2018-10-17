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
#include "boomerang/ssl/parser/SSLParser.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/type/FloatType.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/log/Log.h"


TableEntry::TableEntry()
    : m_rtl(Address::INVALID)
{
}


TableEntry::TableEntry(const std::list<QString> &params, const RTL &rtl)
    : m_rtl(rtl)
{
    std::copy(params.begin(), params.end(), std::back_inserter(m_params));
}


int TableEntry::appendRTL(const std::list<QString> &params, const RTL &rtl)
{
    if (!std::equal(m_params.begin(), m_params.end(), params.begin())) {
        return -1;
    }

    m_rtl.append(rtl.getStatements());
    return 0;
}


RTLInstDict::RTLInstDict(bool verboseOutput)
    : m_verboseOutput(verboseOutput)
    , m_bigEndian(Endian::Little)
{
}


int RTLInstDict::insert(const QString &name, std::list<QString> &params, const RTL &rtl)
{
    QString opcode = name.toUpper();

    opcode.remove(".");

    if (idict.find(opcode) == idict.end()) {
        idict.emplace(opcode, TableEntry(params, rtl));
    }
    else {
        return idict[opcode].appendRTL(params, rtl);
    }

    return 0;
}


bool RTLInstDict::readSSLFile(const QString &SSLFileName)
{
    // emptying the rtl dictionary
    idict.clear();

    // Clear all state
    reset();

    // Attempt to Parse the SSL file
    SSLParser theParser(qPrintable(SSLFileName),
#ifdef DEBUG_SSLPARSER
                        true
#else
                        false
#endif
    );

    if (theParser.theScanner == nullptr) {
        return false;
    }

    theParser.yyparse(*this);

    if (m_verboseOutput) {
        OStream q_cout(stdout);
        q_cout << "\n=======Expanded RTL template dictionary=======\n";
        print(q_cout);
        q_cout << "\n==============================================\n\n";
    }

    return true;
}


void RTLInstDict::addRegister(const QString &name, int id, int size, bool flt)
{
    RegMap[name] = id;

    if (id == -1) {
        SpecialRegMap.insert(std::make_pair(name, Register(name, size, flt)));
    }
    else {
        DetRegMap.insert(std::make_pair(id, Register(name, size, flt)));
    }
}


void RTLInstDict::print(OStream &os /*= std::cout*/)
{
    for (auto &elem : idict) {
        // print the instruction name
        os << (elem).first << "  ";

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


std::pair<QString, unsigned> RTLInstDict::getSignature(const char *name)
{
    // Take the argument, convert it to upper case and remove any _'s and .'s
    QString hlpr(name);

    hlpr = hlpr.replace(".", "").toUpper();
    // Look up the dictionary
    std::map<QString, TableEntry>::iterator it = idict.find(hlpr);

    if (it == idict.end()) {
        LOG_ERROR("No entry for '%1' in RTL dictionary", name);
        it = idict.find("NOP");

        if (it == idict.end()) {
            LOG_ERROR("No entry for 'NOP' in RTL dictionary");
            return { hlpr, 0 }; // At least, don't cause segfault
        }
    }

    return { hlpr, (it->second).m_params.size() };
}


bool RTLInstDict::partialType(Exp *exp, Type &ty)
{
    if (exp->isSizeCast()) {
        ty = *IntegerType::get(exp->access<Const, 1>()->getInt());
        return true;
    }

    if (exp->isFltConst()) {
        ty = *FloatType::get(64);
        return true;
    }

    return false;
}


std::unique_ptr<RTL> RTLInstDict::instantiateRTL(const QString &name, Address natPC,
                                                 const std::vector<SharedExp> &actuals)
{
    // TODO try to retrieve fast instruction mappings
    // before trying the verbose instructions
    auto dict_entry = idict.find(name);
    if (dict_entry == idict.end()) {
        return nullptr; // instruction not found
    }

    TableEntry &entry(dict_entry->second);
    std::unique_ptr<RTL> rtl = instantiateRTL(entry.m_rtl, natPC, entry.m_params, actuals);
    if (rtl) {
        return rtl;
    }
    else {
        LOG_ERROR("Cannot instantiate instruction '%1' at address %2: "
                  "Instruction has %3 parameters, but got %4 arguments",
                  name, natPC, entry.m_params.size(), actuals.size());
        return nullptr;
    }
}


QString RTLInstDict::getRegNameByID(int regID) const
{
    for (const std::pair<QString, int> &elem : RegMap) {
        if (elem.second == regID) {
            return elem.first;
        }
    }

    return QString("");
}


int RTLInstDict::getRegIDByName(const QString& regName) const
{
    const auto iter = RegMap.find(regName);
    return iter != RegMap.end() ? iter->second : -1;
}


int RTLInstDict::getRegSizeByID(int regID) const
{
    const auto iter = DetRegMap.find(regID);
    return iter != DetRegMap.end() ? iter->second.getSize() : 32;
}


void RTLInstDict::setEndian(Endian endian)
{
    m_bigEndian = endian;
}


std::unique_ptr<RTL> RTLInstDict::instantiateRTL(RTL &existingRTL, Address natPC,
                                                 std::list<QString> &params,
                                                 const std::vector<SharedExp> &actuals)
{
    if (params.size() != actuals.size()) {
        return nullptr;
    }

    // Get a deep copy of the template RTL
    std::unique_ptr<RTL> newList(new RTL(existingRTL));
    newList->setAddress(natPC);

    // Iterate through each Statement of the new list of stmts
    for (Statement *ss : *newList) {
        // Search for the formals and replace them with the actuals
        auto param                                    = params.begin();
        std::vector<SharedExp>::const_iterator actual = actuals.begin();

        for (; param != params.end(); ++param, ++actual) {
            /* Simple parameter - just construct the formal to search for */
            Location formal(opParam, Const::get(*param),
                            nullptr); // Location::param(param->c_str());
            ss->searchAndReplace(formal, *actual);
            // delete formal;
        }

        ss->fixSuccessor();

        if (m_verboseOutput) {
            OStream q_cout(stdout);
            q_cout << "            " << ss << "\n";
        }
    }

    // Perform simplifications, e.g. *1 in Pentium addressing modes
    for (Statement *s : *newList) {
        s->simplify();
    }

    return newList;
}


void RTLInstDict::reset()
{
    RegMap.clear();
    DetRegMap.clear();
    SpecialRegMap.clear();
    ParamSet.clear();
    DetParamMap.clear();
    FlagFuncs.clear();
    DefMap.clear();
    fastMap.clear();
    idict.clear();
}
