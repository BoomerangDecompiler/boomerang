#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Signature.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/ProcCFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/PPCSignature.h"
#include "boomerang/db/signature/PentiumSignature.h"
#include "boomerang/db/signature/SPARCSignature.h"
#include "boomerang/db/signature/ST20Signature.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/signature/Win32Signature.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/ssl/type/PointerType.h"
#include "boomerang/ssl/type/SizeType.h"
#include "boomerang/ssl/type/Type.h"
#include "boomerang/ssl/type/VoidType.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"

#include <cassert>
#include <cstring>
#include <sstream>
#include <string>


Signature::Signature(const QString &name)
    : m_ellipsis(false)
    , m_unknown(true)
    , m_forced(false)
{
    if (name == nullptr) {
        m_name = "<ANON>";
    }
    else {
        m_name = name;
    }
}


Signature::~Signature()
{
}


std::shared_ptr<Signature> Signature::clone() const
{
    auto n = std::make_shared<Signature>(m_name);

    Util::clone(m_params, n->m_params);
    Util::clone(m_returns, n->m_returns);

    n->m_ellipsis      = m_ellipsis;
    n->m_preferredName = m_preferredName;
    n->m_unknown       = m_unknown;
    n->m_sigFile       = m_sigFile;
    return n;
}


bool Signature::operator==(const Signature &other) const
{
    if (m_name != other.m_name) {
        return false;
    }

    if (m_params.size() != other.m_params.size() || m_returns.size() != other.m_returns.size()) {
        return false;
    }

    return std::equal(m_params.begin(), m_params.end(), other.m_params.begin(),
                      [](const std::shared_ptr<Parameter> &param,
                         const std::shared_ptr<Parameter> &otherParam) {
                          return *param == *otherParam;
                      }) &&
           std::equal(m_returns.begin(), m_returns.end(), other.m_returns.begin(),
                      [](const std::shared_ptr<Return> &ret,
                         const std::shared_ptr<Return> &otherRet) { return *ret == *otherRet; });
}


bool Signature::operator<(const Signature &other) const
{
    if (m_name != other.m_name) {
        return m_name < other.m_name;
    }

    if (m_params.size() != other.m_params.size()) {
        return m_params.size() < other.m_params.size();
    }
    else if (m_returns.size() != m_params.size()) {
        return m_returns.size() < other.m_returns.size();
    }

    for (size_t i = 0; i < m_params.size(); ++i) {
        if (*m_params[i] != *other.m_params[i]) {
            return *m_params[i] < *other.m_params[i];
        }
    }

    for (size_t i = 0; i < m_returns.size(); ++i) {
        if (*m_returns[i] != *other.m_returns[i]) {
            return *m_returns[i] < *other.m_returns[i];
        }
    }

    return false; // equal
}


QString Signature::getName() const
{
    return m_name;
}


void Signature::setName(const QString &name)
{
    m_name = name;
}


void Signature::addParameter(const SharedExp &e, SharedType ty)
{
    addParameter("", e, ty);
}


void Signature::addParameter(const QString &name, const SharedExp &e, SharedType type,
                             const QString &boundMax)
{
    if (e == nullptr) {
        LOG_FATAL("No expression for parameter %1 %2", type ? type->getCtype() : "<notype>",
                  !name.isEmpty() ? qPrintable(name) : "<noname>");
    }

    QString newName = name;

    if (newName.isEmpty()) {
        size_t n = 0;

        // try param0, param1 etc. until no collision
        do {
            const QString s = QString("param%1").arg(n++);

            if (!std::any_of(m_params.begin(), m_params.end(),
                             [&s](const std::shared_ptr<Parameter> &param) {
                                 return param->getName() == s;
                             })) {
                newName = s;
            }
        } while (newName.isEmpty());
    }

    addParameter(std::make_shared<Parameter>(type, newName, e, boundMax));
    // addImplicitParametersFor(p);
}


void Signature::addParameter(std::shared_ptr<Parameter> param)
{
    SharedType ty = param->getType();
    QString name  = param->getName();
    SharedExp e   = param->getExp();

    if ((ty == nullptr) || (e == nullptr) || name.isEmpty()) {
        addParameter(name, e, ty, param->getBoundMax());
    }
    else {
        m_params.push_back(param);
    }
}


void Signature::removeParameter(const SharedExp &e)
{
    int i = findParam(e);

    if (i != -1) {
        removeParameter(i);
    }
}


void Signature::removeParameter(int i)
{
    const int n = m_params.size();
    if (!Util::inRange(i, 0, n)) {
        return;
    }

    m_params.erase(m_params.begin() + i);
}


void Signature::setNumParams(int n)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size() + 1)));
    m_params.erase(m_params.begin() + n, m_params.end());
}


const QString &Signature::getParamName(int n) const
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    return m_params[n]->getName();
}


SharedExp Signature::getParamExp(int n) const
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    return m_params[n]->getExp();
}


SharedType Signature::getParamType(int n) const
{
    // assert(n < (int)params.size() || ellipsis);
    // With recursion, parameters not set yet. Hack for now:
    if (!Util::inRange(n, 0, static_cast<int>(m_params.size()))) {
        return nullptr;
    }

    return m_params[n]->getType();
}


QString Signature::getParamBoundMax(int n) const
{
    if (Util::inRange(n, 0, static_cast<int>(m_params.size()))) {
        return m_params[n]->getBoundMax();
    }
    else {
        return "";
    }
}


void Signature::setParamType(int n, SharedType ty)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    m_params[n]->setType(ty);
}


void Signature::setParamType(const QString &name, SharedType ty)
{
    int idx = findParam(name);

    if (idx == -1) {
        LOG_WARN("Could not set type for unknown parameter %1", name);
        return;
    }

    m_params[idx]->setType(ty);
}


void Signature::setParamType(const SharedExp &e, SharedType ty)
{
    int idx = findParam(e);

    if (idx == -1) {
        LOG_WARN("Could not set type for unknown parameter expression '%1' of '%2'", e, getName());
        return;
    }

    m_params[idx]->setType(ty);
}


void Signature::setParamName(int n, const QString &name)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    m_params[n]->setName(name);
}


void Signature::setParamExp(int n, SharedExp e)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    m_params[n]->setExp(e);
}


int Signature::findParam(const SharedExp &e) const
{
    for (int i = 0; i < getNumParams(); i++) {
        if (*getParamExp(i) == *e) {
            return i;
        }
    }

    return -1;
}


bool Signature::renameParam(const QString &oldName, const QString &newName)
{
    for (int i = 0; i < getNumParams(); i++) {
        if (m_params[i]->getName() == oldName) {
            m_params[i]->setName(newName);
            return true;
        }
    }

    return false;
}


int Signature::findParam(const QString &name) const
{
    for (int i = 0; i < getNumParams(); i++) {
        if (getParamName(i) == name) {
            return i;
        }
    }

    return -1;
}


int Signature::findReturn(SharedConstExp exp) const
{
    if (!exp) {
        return -1;
    }

    for (int i = 0; i < getNumReturns(); i++) {
        if (*m_returns[i]->getExp() == *exp) {
            return i;
        }
    }

    return -1;
}


void Signature::addReturn(SharedType type, SharedExp exp)
{
    assert(exp);
    m_returns.emplace_back(std::make_shared<Return>(type, exp));
}


void Signature::addReturn(SharedExp exp)
{
    addReturn(PointerType::get(VoidType::get()), exp);
}


SharedConstExp Signature::getReturnExp(int n) const
{
    assert(Util::inRange(n, 0, static_cast<int>(m_returns.size())));
    return m_returns[n]->getExp();
}


SharedExp Signature::getReturnExp(int n)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_returns.size())));
    return m_returns[n]->getExp();
}


SharedExp Signature::getArgumentExp(int n) const
{
    return getParamExp(n);
}


SharedConstType Signature::getReturnType(int n) const
{
    assert(Util::inRange(n, 0, static_cast<int>(m_returns.size())));
    return m_returns[n]->getType();
}


SharedType Signature::getReturnType(int n)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_returns.size())));
    return m_returns[n]->getType();
}


std::shared_ptr<Signature> Signature::promote(UserProc *p)
{
    // FIXME: the whole promotion idea needs a redesign...
    if (CallingConvention::Win32Signature::qualified(p, *this)) {
        return std::make_shared<CallingConvention::Win32Signature>(*this);
    }

    if (CallingConvention::StdC::PentiumSignature::qualified(p, *this)) {
        return std::make_shared<CallingConvention::StdC::PentiumSignature>(*this);
    }

    if (CallingConvention::StdC::SPARCSignature::qualified(p, *this)) {
        return std::make_shared<CallingConvention::StdC::SPARCSignature>(*this);
    }

    if (CallingConvention::StdC::PPCSignature::qualified(p, *this)) {
        return std::make_shared<CallingConvention::StdC::PPCSignature>(*this);
    }

    if (CallingConvention::StdC::ST20Signature::qualified(p, *this)) {
        return std::make_shared<CallingConvention::StdC::ST20Signature>(*this);
    }

    return shared_from_this();
}


std::unique_ptr<Signature> Signature::instantiate(Machine machine, CallConv cc, const QString &name)
{
    switch (machine) {
    case Machine::X86:
        if (cc == CallConv::Pascal) {
            // For now, assume the only pascal calling convention Pentium signatures will be Windows
            return std::make_unique<CallingConvention::Win32Signature>(name);
        }
        else if (cc == CallConv::ThisCall) {
            return std::make_unique<CallingConvention::Win32TcSignature>(name);
        }
        else {
            return std::make_unique<CallingConvention::StdC::PentiumSignature>(name);
        }

    case Machine::SPARC: return std::make_unique<CallingConvention::StdC::SPARCSignature>(name);

    case Machine::PPC: return std::make_unique<CallingConvention::StdC::PPCSignature>(name);

    case Machine::ST20: return std::make_unique<CallingConvention::StdC::ST20Signature>(name);

    // insert other conventions here
    default:
        LOG_WARN("Unknown signature: %1 (CallConv: %2)", name, Util::getCallConvName(cc));
        return std::make_unique<Signature>(name);
    }
}


void Signature::print(OStream &out, bool /*html*/) const
{
    if (isForced()) {
        out << "*forced* ";
    }

    if (!m_returns.empty()) {
        out << "{ ";
        unsigned n = 0;

        for (const std::shared_ptr<Return> &rr : m_returns) {
            out << rr->getType()->getCtype() << " " << rr->getExp();

            if (n != m_returns.size() - 1) {
                out << ",";
            }

            out << " ";
            n++;
        }

        out << "} ";
    }
    else {
        out << "void ";
    }

    out << m_name << "(";

    for (unsigned int i = 0; i < m_params.size(); i++) {
        out << m_params[i]->getType()->getCtype() << " " << m_params[i]->getName() << " "
            << m_params[i]->getExp();

        if (i != m_params.size() - 1) {
            out << ", ";
        }
    }

    out << ")";
}


bool Signature::getABIDefines(Machine machine, StatementList &defs)
{
    if (machine == Machine::INVALID || !defs.empty()) {
        return false; // Do only once
    }

    switch (machine) {
    case Machine::X86:
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_PENT_EAX))); // eax
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_PENT_ECX))); // ecx
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_PENT_EDX))); // edx
        return true;

    case Machine::SPARC:
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_O0))); // %o0-o5
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_O1)));
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_O2)));
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_O3)));
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_O4)));
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_O5)));
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_SPARC_G1))); // %g1
        return true;

    case Machine::PPC:

        for (int r = REG_PPC_G3; r <= REG_PPC_G12; ++r) {
            defs.append(std::make_shared<ImplicitAssign>(Location::regOf(r))); // r3-r12
        }

        break;

    case Machine::ST20:
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_ST20_A))); // A
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_ST20_B))); // B
        defs.append(std::make_shared<ImplicitAssign>(Location::regOf(REG_ST20_C))); // C
        return true;

    default: break;
    }

    return true;
}


RegNum Signature::getStackRegister() const
{
    return RegNumSpecial;
}


bool Signature::isStackLocal(RegNum spIndex, SharedConstExp e) const
{
    // e must be m[...]
    if (e->isSubscript()) {
        return isStackLocal(spIndex, e->getSubExp1());
    }
    else if (!e->isMemOf()) {
        return false;
    }

    return isAddrOfStackLocal(spIndex, e->getSubExp1());
}


bool Signature::isAddrOfStackLocal(RegNum spIndex, const SharedConstExp &e) const
{
    if (e->isAddrOf()) {
        return isStackLocal(spIndex, e->getSubExp1());
    }

    // e must be sp -/+ K or just sp
    SharedConstExp sp = Location::regOf(spIndex);

    const OPER op = e->getOper();
    if ((op != opMinus) && (op != opPlus)) {
        // Matches if e is sp or sp{0} or sp{-}
        return *e == *sp || (e->isSubscript() && e->access<RefExp>()->isImplicitDef() &&
                             *e->getSubExp1() == *sp);
    }

    // We may have weird expressions like sp + -4
    // which is the address of a stack local on x86
    SharedConstExp exp2 = e->clone()->simplify();
    if (!isOpCompatStackLocal(exp2->getOper())) {
        return false;
    }

    SharedConstExp sub1 = exp2->getSubExp1();
    SharedConstExp sub2 = exp2->getSubExp2();

    // exp2 must be <sub1> +- K
    if (!sub2->isIntConst()) {
        return false;
    }

    // first operand must be sp or sp{0} or sp{-}
    if (sub1->isSubscript()) {
        if (!sub1->access<RefExp>()->isImplicitDef()) {
            return false;
        }

        sub1 = sub1->getSubExp1();
    }

    return *sub1 == *sp;
}


bool Signature::isOpCompatStackLocal(OPER op) const
{
    if (op == opMinus) {
        return isLocalOffsetNegative();
    }

    if (op == opPlus) {
        return isLocalOffsetPositive();
    }

    return false;
}


bool Signature::returnCompare(const Assignment &a, const Assignment &b) const
{
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}


bool Signature::argumentCompare(const Assignment &a, const Assignment &b) const
{
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}
