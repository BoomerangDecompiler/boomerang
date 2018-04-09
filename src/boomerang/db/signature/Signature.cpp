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


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/MIPSSignature.h"
#include "boomerang/db/signature/PentiumSignature.h"
#include "boomerang/db/signature/PPCSignature.h"
#include "boomerang/db/signature/SparcSignature.h"
#include "boomerang/db/signature/ST20Signature.h"
#include "boomerang/db/signature/Win32Signature.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/type/type/Type.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/PointerType.h"

#include <cassert>
#include <string>
#include <cstring>
#include <sstream>


QString Signature::getPlatformName(Platform plat)
{
    switch (plat)
    {
    case Platform::PENTIUM:
        return "pentium";

    case Platform::SPARC:
        return "sparc";

    case Platform::M68K:
        return "m68k";

    case Platform::PARISC:
        return "parisc";

    case Platform::PPC:
        return "ppc";

    case Platform::MIPS:
        return "mips";

    case Platform::ST20:
        return "st20";

    default:
        return "???";
    }
}


QString Signature::getConventionName(CallConv cc)
{
    switch (cc)
    {
    case CallConv::C:
        return "stdc";

    case CallConv::Pascal:
        return "pascal";

    case CallConv::ThisCall:
        return "thiscall";

    case CallConv::FastCall:
        return "fastcall";

    default:
        return "??";
    }
}


Signature::Signature(const QString& name)
    : m_ellipsis(false)
    , m_unknown(true)
    , m_forced(false)
    , m_preferredReturn(nullptr)
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

    n->m_ellipsis        = m_ellipsis;
    n->m_preferredName   = m_preferredName;
    n->m_preferredReturn = m_preferredReturn ? m_preferredReturn->clone() : nullptr;
    n->m_preferredParams = m_preferredParams;
    n->m_unknown         = m_unknown;
    n->m_sigFile         = m_sigFile;
    return n;
}


bool Signature::operator==(const Signature& other) const
{
    // if (name != other.name) return false;        // MVE: should the name be significant? I'm thinking no
    if (m_params.size() != other.m_params.size()) {
        return false;
    }

    // Only care about the first return location (at present)
    for (auto it1 = m_params.begin(), it2 = other.m_params.begin(); it1 != m_params.end(); it1++, it2++) {
        if (!(**it1 == **it2)) {
            return false;
        }
    }

    if (m_returns.size() != other.m_returns.size()) {
        return false;
    }

    for (auto rr1 = m_returns.begin(), rr2 = other.m_returns.begin(); rr1 != m_returns.end(); ++rr1, ++rr2) {
        if (!(**rr1 == **rr2)) {
            return false;
        }
    }

    return true;
}


QString Signature::getName() const
{
    return m_name;
}


void Signature::setName(const QString& name)
{
    m_name = name;
}


void Signature::addParameter(const SharedExp& e, SharedType ty)
{
    addParameter(nullptr, e, ty);
}


void Signature::addParameter(const QString& name, const SharedExp& e,
                             SharedType type, const QString& boundMax)
{
    if (e == nullptr) {
        // Else get infinite mutual recursion with the below proc
        LOG_FATAL("No expression for parameter %1 %2",
                  type ? type->getCtype() : "<notype>",
                  !name.isNull() ? qPrintable(name) : "<noname>");
    }

    QString s;
    QString new_name = name;

    if (name.isNull()) {
        size_t n  = m_params.size() + 1;
        bool   ok = false;

        while (!ok) {
            s  = QString("param%1").arg(n);
            ok = true;

            for (auto& elem : m_params) {
                if (s == elem->getName()) {
                    ok = false;
                    break;
                }
            }

            n++;
        }

        new_name = s;
    }

    addParameter(std::make_shared<Parameter>(type, new_name, e, boundMax));
    // addImplicitParametersFor(p);
}


void Signature::addParameter(std::shared_ptr<Parameter> param)
{
    SharedType ty   = param->getType();
    QString    name = param->getName();
    SharedExp  e    = param->getExp();

    if (name.isEmpty()) {
        name = QString::null;
    }

    if ((ty == nullptr) || (e == nullptr) || name.isNull()) {
        addParameter(name, e, ty, param->getBoundMax());
    }
    else {
        m_params.push_back(param);
    }
}


void Signature::removeParameter(const SharedExp& e)
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
    if (n < static_cast<int>(m_params.size())) {
        // truncate
        m_params.erase(m_params.begin() + n, m_params.end());
    }
    else {
        for (int i = static_cast<int>(m_params.size()); i < n; i++) {
            addParameter(nullptr, VoidType::get());
        }
    }
}


const QString& Signature::getParamName(int n) const
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
    if (!Util::inRange(n, 0, static_cast<int>(m_params.size()))) {
        return QString::null;
    }

    QString s = m_params[n]->getBoundMax();

    if (s.isEmpty()) {
        return QString::null;
    }

    return s;
}


void Signature::setParamType(int n, SharedType ty)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    m_params[n]->setType(ty);
}


void Signature::setParamType(const QString& name, SharedType ty)
{
    int idx = findParam(name);

    if (idx == -1) {
        LOG_WARN("Could not set type for unknown parameter %1", name);
        return;
    }

    m_params[idx]->setType(ty);
}


void Signature::setParamType(const SharedExp& e, SharedType ty)
{
    int idx = findParam(e);

    if (idx == -1) {
        LOG_WARN("Could not set type for unknown parameter expression %1", e);
        return;
    }

    m_params[idx]->setType(ty);
}


void Signature::setParamName(int n, const QString& name)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    m_params[n]->setName(name);
}


void Signature::setParamExp(int n, SharedExp e)
{
    assert(Util::inRange(n, 0, static_cast<int>(m_params.size())));
    m_params[n]->setExp(e);
}


int Signature::findParam(const SharedExp& e) const
{
    for (int i = 0; i < getNumParams(); i++) {
        if (*getParamExp(i) == *e) {
            return i;
        }
    }

    return -1;
}


void Signature::renameParam(const QString& oldName, const QString& newName)
{
    for (int i = 0; i < getNumParams(); i++) {
        if (m_params[i]->getName() == oldName) {
            m_params[i]->setName(newName);
            break;
        }
    }
}


int Signature::findParam(const QString& name) const
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
    for (unsigned i = 0; i < getNumReturns(); i++) {
        if (*m_returns[i]->getExp() == *exp) {
            return static_cast<int>(i);
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
        return std::shared_ptr<Signature>(new CallingConvention::Win32Signature(*this));
    }

    if (CallingConvention::StdC::PentiumSignature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::PentiumSignature(*this));
    }

    if (CallingConvention::StdC::SparcSignature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::SparcSignature(*this));
    }

    if (CallingConvention::StdC::PPCSignature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::PPCSignature(*this));
    }

    if (CallingConvention::StdC::ST20Signature::qualified(p, *this)) {
        return std::shared_ptr<Signature>(new CallingConvention::StdC::ST20Signature(*this));
    }

    return shared_from_this();
}


std::shared_ptr<Signature> Signature::instantiate(Platform plat, CallConv cc, const QString& name)
{
    switch (plat)
    {
    case Platform::PENTIUM:

        if (cc == CallConv::Pascal) {
            // For now, assume the only pascal calling convention pentium signatures will be Windows
            return std::make_shared<CallingConvention::Win32Signature>(name);
        }
        else if (cc == CallConv::ThisCall) {
            return std::make_shared<CallingConvention::Win32TcSignature>(name);
        }
        else {
            return std::make_shared<CallingConvention::StdC::PentiumSignature>(name);
        }

    case Platform::SPARC:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        assert(cc == CallConv::C);
        return std::make_shared<CallingConvention::StdC::SparcSignature>(name);

    case Platform::PPC:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        return std::make_shared<CallingConvention::StdC::PPCSignature>(name);

    case Platform::ST20:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        return std::make_shared<CallingConvention::StdC::ST20Signature>(name);

    case Platform::MIPS:

        if (cc == CallConv::Pascal) {
            cc = CallConv::C;
        }

        return std::make_shared<CallingConvention::StdC::MIPSSignature>(name);

    // insert other conventions here
    default:
        LOG_ERROR("Unknown signature: %1 %2", getConventionName(cc), getPlatformName(plat));
        return nullptr;
    }
}


void Signature::print(QTextStream& out, bool /*html*/) const
{
    if (isForced()) {
        out << "*forced* ";
    }

    if (!m_returns.empty()) {
        out << "{ ";
        unsigned n = 0;

        for (const std::shared_ptr<Return>& rr : m_returns) {
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
        out << m_params[i]->getType()->getCtype() << " " << m_params[i]->getName() << " " << m_params[i]->getExp();

        if (i != m_params.size() - 1) {
            out << ", ";
        }
    }

    out << ")";
}


char *Signature::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    tgt += "\n";

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void Signature::getABIDefines(Prog *prog, StatementList& defs)
{
    if (defs.size() > 0) {
        return; // Do only once
    }

    switch (prog->getMachine())
    {
    case Machine::PENTIUM:
        defs.append(new ImplicitAssign(Location::regOf(24))); // eax
        defs.append(new ImplicitAssign(Location::regOf(25))); // ecx
        defs.append(new ImplicitAssign(Location::regOf(26))); // edx
        break;

    case Machine::SPARC:

        for (int r = 8; r <= 13; ++r) {
            defs.append(new ImplicitAssign(Location::regOf(r))); // %o0-o5
        }

        defs.append(new ImplicitAssign(Location::regOf(1)));     // %g1
        break;

    case Machine::PPC:

        for (int r = 3; r <= 12; ++r) {
            defs.append(new ImplicitAssign(Location::regOf(r))); // r3-r12
        }

        break;

    case Machine::ST20:
        defs.append(new ImplicitAssign(Location::regOf(0))); // A
        defs.append(new ImplicitAssign(Location::regOf(1))); // B
        defs.append(new ImplicitAssign(Location::regOf(2))); // C
        break;

    default:
        break;
    }
}


int Signature::getStackRegister() const
{
    return -1;
}


int Signature::getStackRegister(Prog *prog)
{
    switch (prog->getMachine())
    {
    case Machine::SPARC:
        return 14;

    case Machine::PENTIUM:
        return 28;

    case Machine::PPC:
        return 1;

    case Machine::ST20:
        return 3;

    case Machine::MIPS:
        return 29;

    default:
        return -1;
    }
}


bool Signature::isStackLocal(Prog *prog, SharedExp e) const
{
    // e must be m[...]
    if (e->isSubscript()) {
        return isStackLocal(prog, e->getSubExp1());
    }

    if (!e->isMemOf()) {
        return false;
    }

    SharedExp addr = e->getSubExp1();
    return isAddrOfStackLocal(prog, addr);
}


bool Signature::isAddrOfStackLocal(Prog *prog, const SharedExp& e) const
{
    OPER op = e->getOper();

    if (op == opAddrOf) {
        return isStackLocal(prog, e->getSubExp1());
    }

    // e must be sp -/+ K or just sp
    static SharedExp sp = Location::regOf(getStackRegister(prog));

    if ((op != opMinus) && (op != opPlus)) {
        // Matches if e is sp or sp{0} or sp{-}
        return(*e == *sp ||
               (e->isSubscript() && e->access<RefExp>()->isImplicitDef() && *e->getSubExp1() == *sp));
    }

    if ((op == opMinus) && !isLocalOffsetNegative()) {
        return false;
    }

    if ((op == opPlus) && !isLocalOffsetPositive()) {
        return false;
    }

    SharedExp sub1 = e->getSubExp1();
    SharedExp sub2 = e->getSubExp2();

    // e must be <sub1> +- K
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


bool Signature::returnCompare(const Assignment& a, const Assignment& b) const
{
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}


bool Signature::argumentCompare(const Assignment& a, const Assignment& b) const
{
    return *a.getLeft() < *b.getLeft(); // Default: sort by expression only, no explicit ordering
}
