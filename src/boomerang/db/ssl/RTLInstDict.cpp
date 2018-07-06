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


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/ssl/sslparser.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Binary.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/db/RTL.h"
#include "boomerang/util/Log.h"


TableEntry::TableEntry()
    : m_rtl(Address::INVALID)
{
}


TableEntry::TableEntry(const std::list<QString>& params, const RTL& rtl)
    : m_rtl(rtl)
{
    std::copy(params.begin(), params.end(), std::back_inserter(m_params));
}


int TableEntry::appendRTL(const std::list<QString>& params, const RTL& rtl)
{
    if (!std::equal(m_params.begin(), m_params.end(), params.begin())) {
        return -1;
    }

    m_rtl.append(rtl.getStatements());
    return 0;
}


int RTLInstDict::insert(const QString& name, std::list<QString>& params, const RTL& rtl)
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


bool RTLInstDict::readSSLFile(const QString& SSLFileName)
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

    addRegister("%CTI", -1, 1, false);
    addRegister("%NEXT", -1, 32, false);

    theParser.yyparse(*this);

    fixupParams();

    if (m_verboseOutput) {
        QTextStream q_cout(stdout);
        q_cout << "\n=======Expanded RTL template dictionary=======\n";
        print(q_cout);
        q_cout << "\n==============================================\n\n";
    }

    return true;
}


void RTLInstDict::addRegister(const QString& name, int id, int size, bool flt)
{
    RegMap[name] = id;

    if (id == -1) {
        SpecialRegMap.insert(std::make_pair(name, Register(name, size, flt)));
    }
    else {
        DetRegMap.insert(std::make_pair(id, Register(name, size, flt)));
    }
}


void RTLInstDict::print(QTextStream& os /*= std::cout*/)
{
    for (auto& elem : idict) {
        // print the instruction name
        os << (elem).first << "  ";

        // print the parameters
        const std::list<QString>& params((elem).second.m_params);
        int i = params.size();

        for (auto s = params.begin(); s != params.end(); ++s, i--) {
            os << *s << (i != 1 ? "," : "");
        }

        os << "\n";

        // print the RTL
        RTL& rtlist = (elem).second.m_rtl;
        rtlist.print(os);
        os << "\n";
    }
}


void RTLInstDict::fixupParams()
{
    for (ParamEntry& param : DetParamMap) {
        param.m_mark = 0;
    }

    int mark = 1;

    for (auto iter = DetParamMap.begin(); iter != DetParamMap.end(); ++iter) {
        if (iter.value().m_kind == PARAM_VARIANT) {
            std::list<QString> funcParams;
            bool               haveCount = false;
            fixupParamsSub(iter.key(), funcParams, haveCount, mark++);
        }
    }
}


void RTLInstDict::fixupParamsSub(const QString& s, std::list<QString>& funcParams, bool& haveCount, int mark)
{
    ParamEntry& param = DetParamMap[s];

    if (param.m_params.empty()) {
        LOG_ERROR("Error in SSL File: Variant operand %1 has no branches. Well that's really useful...", s);
        return;
    }

    if (param.m_mark == mark) {
        return; /* Already seen this round. May indicate a cycle, but may not */
    }

    param.m_mark = mark;

    for (const QString& name : param.m_params) {
        ParamEntry& sub = DetParamMap[name];

        if (sub.m_kind == PARAM_VARIANT) {
            fixupParamsSub(name, funcParams, haveCount, mark);

            if (!haveCount) { /* Empty branch? */
                continue;
            }
        }
        else if (!haveCount) {
            haveCount = true;
            char buf[10];

            for (unsigned int i = 1; i <= sub.m_funcParams.size(); i++) {
                sprintf(buf, "__lp%u", i);
                funcParams.push_back(buf);
            }
        }

        if (funcParams.size() != sub.m_funcParams.size()) {
            LOG_ERROR("Error in SSL File: Variant operand %1 does not have a fixed number of functional parameters:", s);
            LOG_ERROR("Expected %1 parameters, but branch %2 has %3 parameters.", funcParams.size(), name, sub.m_funcParams.size());
        }
        else if ((funcParams != sub.m_funcParams) && (sub.m_asgn != nullptr)) {
            /* Rename so all the parameter names match */
            for (auto i = funcParams.begin(), j = sub.m_funcParams.begin(); i != funcParams.end(); i++, j++) {
                Location  paramLoc(opParam, Const::get(*j), nullptr); // Location::param(j->c_str())
                SharedExp replace = Location::param(*i);
                sub.m_asgn->searchAndReplace(paramLoc, replace);
            }

            sub.m_funcParams = funcParams;
        }
    }

    param.m_funcParams = funcParams;
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


bool RTLInstDict::partialType(Exp *exp, Type& ty)
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


std::unique_ptr<RTL> RTLInstDict::instantiateRTL(const QString& name, Address natPC,
                                                    const std::vector<SharedExp>& actuals)
{
    // TODO try to retrieve fast instruction mappings
    // before trying the verbose instructions
    auto dict_entry = idict.find(name);
    if (dict_entry == idict.end()) {
        return nullptr; // instruction not found
    }

    TableEntry& entry(dict_entry->second);
    return instantiateRTL(entry.m_rtl, natPC, entry.m_params, actuals);
}


std::unique_ptr<RTL> RTLInstDict::instantiateRTL(RTL& existingRTL, Address natPC,
                                             std::list<QString>& params,
                                             const std::vector<SharedExp>& actuals)
{
    assert(params.size() == actuals.size());

    // Get a deep copy of the template RTL
    std::unique_ptr<RTL> newList(new RTL(existingRTL));
    newList->setAddress(natPC);

    // Iterate through each Statement of the new list of stmts
    for (Statement *ss : *newList) {
        // Search for the formals and replace them with the actuals
        auto param = params.begin();
        std::vector<SharedExp>::const_iterator actual = actuals.begin();

        for ( ; param != params.end(); ++param, ++actual) {
            /* Simple parameter - just construct the formal to search for */
            Location formal(opParam, Const::get(*param), nullptr); // Location::param(param->c_str());
            ss->searchAndReplace(formal, *actual);
            // delete formal;
        }

        ss->fixSuccessor();

        if (m_verboseOutput) {
            QTextStream q_cout(stdout);
            q_cout << "            " << ss << "\n";
        }
    }

    transformPostVars(*newList.get(), true);

    // Perform simplifications, e.g. *1 in Pentium addressing modes
    for (Statement *s : *newList) {
        s->simplify();
    }

    return newList;
}


/* Small struct for transformPostVars */
struct transPost
{
    bool       used; // If the base expression (e.g. r[0]) is used
    // Important because if not, we don't have to make any
    // substitutions at all
    bool       isNew; // Not sure (MVE)
    SharedExp  tmp;   // The temp to replace r[0]' with
    SharedExp  post;  // The whole postvar expression. e.g. r[0]'
    SharedExp  base;  // The base expression (e.g. r[0])
    SharedType type;  // The type of the temporary (needed for the final assign)
};


void RTLInstDict::transformPostVars(RTL& rts, bool optimise)
{
    // Map from var (could be any expression really) to details
    std::map<SharedExp, transPost, lessExpStar> vars;
    int tmpcount = 1; // For making temp names unique
                      // Exp* matchParam(1,idParam);    // ? Was never used anyway

    // First pass: Scan for post-variables and usages of their referents
    for (Statement *rt : rts) {
        // ss appears to be a list of expressions to be searched
        // It is either the LHS and RHS of an assignment, or it's the parameters of a flag call
        SharedExp ss;

        if (rt->isAssign()) {
            Assign    *rt_asgn = static_cast<Assign *>(rt);
            SharedExp lhs = rt_asgn->getLeft();
            SharedExp rhs = rt_asgn->getRight();

            // Look for assignments to post-variables
            if (lhs && lhs->isPostVar()) {
                if (vars.find(lhs) == vars.end()) {
                    // Add a record in the map for this postvar
                    transPost& el = vars[lhs];
                    el.used = false;
                    el.type = rt_asgn->getType();

                    // Constuct a temporary. We should probably be smarter and actually check
                    // that it's not otherwise used here.
                    QString tmpname = QString("%1%2post").arg(el.type->getTempName()).arg((tmpcount++));
                    el.tmp = Location::tempOf(Const::get(tmpname));

                    // Keep a copy of the referrent. For example, if the lhs is r[0]', base is r[0]
                    el.base  = lhs->getSubExp1();
                    el.post  = lhs; // The whole post-var, e.g. r[0]'
                    el.isNew = true;

                    // The emulator generator sets optimise false
                    // I think this forces always generating the temps (MVE)
                    if (!optimise) {
                        el.used  = true;
                        el.isNew = false;
                    }
                }
            }

            // For an assignment, the two expressions to search are the left and right hand sides (could just put the
            // whole assignment on, I suppose)
            assert(lhs != nullptr);
            assert(rhs != nullptr);
            ss = Binary::get(opList, lhs->clone(), Binary::get(opList, rhs->clone(), Terminal::get(opNil)));
        }
        else if (rt->isFlagAssign()) {
            Assign *rt_asgn = static_cast<Assign *>(rt);
            // Exp *lhs = rt_asgn->getLeft();
            auto rhs = rt_asgn->getRight();
            // An opFlagCall is assumed to be a Binary with a string and an opList of parameters
            ss = rhs->getSubExp2();
            assert(false); // was ss = (Binary *)((Binary *)rt)->getSubExp2();
        }

        /* Look for usages of post-variables' referents
         * Trickier than you'd think, as we need to make sure to skip over the post-variables themselves. ie match
         * r[0] but not r[0]'
         * Note: back with SemStrs, we could use a match expression which was a wildcard prepended to the base
         * expression; this would match either the base (r[0]) or the post-var (r[0]').
         * Can't really use this with Exps, so we search twice; once for the base, and once for the post, and if we
         * get more with the former, then we have a use of the base (consider r[0] + r[0]')
         */
        for (auto& var : vars) {
            if (var.second.isNew) {
                // Make sure we don't match a var in its defining statement
                var.second.isNew = false;
                continue;
            }

            for (auto cur = ss; !cur->isNil(); cur = cur->getSubExp2()) {
                if (var.second.used) {
                    break; // Don't bother; already know it's used
                }

                SharedExp s = cur->getSubExp1();

                if (!s) {
                    continue;
                }

                if (*s == *var.second.base) {
                    var.second.used = true;
                    break;
                }

                std::list<SharedExp> res1, res2;
                s->searchAll(*var.second.base, res1);
                s->searchAll(*var.second.post, res2);

                // Each match of a post will also match the base.
                // But if there is a bare (non-post) use of the base, there will be a result in res1 that is not in res2
                if (res1.size() > res2.size()) {
                    var.second.used = true;
                }
            }
        }
    }

    // Second pass: Replace post-variables with temporaries where needed
    for (Statement *rt : rts) {
        for (auto& var : vars) {
            if (var.second.used) {
                rt->searchAndReplace(*var.first, var.second.tmp);
            }
            else {
                rt->searchAndReplace(*var.first, var.second.base);
            }
        }
    }

    // Finally: Append assignments where needed from temps to base vars
    // Example: esp' = esp-4; m[esp'] = modrm; FLAG(esp)
    // all the esp' are replaced with say tmp1, you need a "esp = tmp1" at the end to actually make the change
    for (auto& var : vars) {
        if (var.second.used) {
            Assign *te = new Assign(var.second.type, var.second.base->clone(), var.second.tmp);
            rts.push_back(te);
        }
    }
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
    AliasMap.clear();
    fastMap.clear();
    idict.clear();
    fetchExecCycle = nullptr;
}
