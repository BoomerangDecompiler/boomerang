#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "RegDB.h"

#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/type/IntegerType.h"
#include "boomerang/util/ByteUtil.h"
#include "boomerang/util/log/Log.h"

#include <stack>


RegDB::RegDB()
{
}


RegDB::~RegDB()
{
}


void RegDB::clear()
{
    m_regIDs.clear();
    m_regInfo.clear();
    m_specialRegInfo.clear();
}


bool RegDB::isRegDefined(const QString &regName) const
{
    return m_regIDs.find(regName) != m_regIDs.end();
}


bool RegDB::isRegIdxDefined(int regID) const
{
    return m_regInfo.find(regID) != m_regInfo.end();
}


const Register *RegDB::getRegByID(int regID) const
{
    const auto it = m_regInfo.find(regID);
    return it != m_regInfo.end() ? &it->second : nullptr;
}


const Register *RegDB::getRegByName(const QString &name) const
{
    const RegID id = getRegIDByName(name);
    if (id == RegIDSpecial) {
        const auto it = m_specialRegInfo.find(name);
        return it != m_specialRegInfo.end() ? &it->second : nullptr;
    }
    else {
        return getRegByID(id);
    }
}


RegID RegDB::getRegIDByName(const QString &name) const
{
    const auto it = m_regIDs.find(name);
    return it != m_regIDs.end() ? it->second : RegIDSpecial;
}


QString RegDB::getRegNameByID(RegID regID) const
{
    const auto it = m_regInfo.find(regID);
    return it != m_regInfo.end() ? it->second.getName() : "";
}


int RegDB::getRegSizeByID(RegID regID) const
{
    const auto iter = m_regInfo.find(regID);
    return iter != m_regInfo.end() ? iter->second.getSize() : 32;
}


bool RegDB::createReg(RegType regType, RegID id, const QString &name, int size)
{
    if (name.isEmpty() || size <= 0 || regType == RegType::Invalid) {
        return false;
    }

    const auto &[_, inserted] = m_regIDs.insert({ name, id });
    Q_UNUSED(_);

    if (!inserted) {
        // a register with the same name (or alias) already exists
        return false;
    }

    if (id == RegIDSpecial) {
        // otherwise would have been caught above
        assert(m_specialRegInfo.find(name) == m_specialRegInfo.end());
        m_specialRegInfo.insert({ name, Register(regType, name, size) });
        return true;
    }

    const auto it = m_regInfo.find(id);
    if (it != m_regInfo.end()) {
        // register alias: only name can be different
        const Register &reg = it->second;
        if (regType != reg.getRegType() || size != reg.getSize()) {
            m_regIDs.erase(name);
            return false;
        }
    }
    else {
        m_regInfo.insert({ id, Register(regType, name, size) });
    }

    return true;
}


bool RegDB::createRegRelation(const QString &parent, const QString &child, int offsetInParent)
{
    if (parent == child) {
        return false;
    }
    else if (!isRegDefined(parent) || !isRegDefined(child)) {
        return false;
    }
    else if (getRegIDByName(parent) == RegIDSpecial) {
        // parent is a special register -> fail
        return false;
    }
    else if (m_parent.find(child) != m_parent.end() ||
             m_offsetInParent.find(child) != m_offsetInParent.end() ||
             (m_children.find(parent) != m_children.end() &&
              m_children.at(parent).find(offsetInParent) != m_children.at(parent).end())) {
        // relation already exists
        return false;
    }

    m_parent[child]                    = parent;
    m_offsetInParent[child]            = offsetInParent;
    m_children[parent][offsetInParent] = child;
    return true;
}


std::unique_ptr<RTL> RegDB::processOverlappedRegs(Assignment *stmt,
                                                  const std::set<RegID> &usedRegs) const
{
    assert(stmt != nullptr);
    SharedConstExp lhs = stmt->getLeft();
    if (!lhs->isRegOfConst()) {
        // lhs must be a consant value regof, otherwise we can't look up the ID
        return nullptr;
    }

    const RegID myID = lhs->access<Const, 1>()->getInt();
    if (myID == RegIDSpecial || !isRegIdxDefined(myID)) {
        return nullptr;
    }

    std::unique_ptr<RTL> result = std::make_unique<RTL>(Address::ZERO);

    try {
        // first process the effects of assignment "up" the register forest
        // e.g. the effects on %eax when assigning to %ah
        {
            const Register *base  = getRegByID(myID);
            const Register *child = base;
            int offsetInParent    = 0;

            while (child != nullptr) {
                const bool hasParent = m_parent.find(child->getName()) != m_parent.end();
                if (!hasParent) {
                    break; // reached top of tree
                }
                const Register *parent = getRegByName(m_parent.at(child->getName()));
                assert(parent != nullptr);

                offsetInParent += m_offsetInParent.at(child->getName());
                const RegID parentID = getRegIDByName(parent->getName());

                // is the parent actually used? if not, then skip
                if (usedRegs.find(parentID) != usedRegs.end()) {
                    Assignment *overlapAsgn = emitOverlappedStmt(stmt, parent, base,
                                                                 offsetInParent);
                    if (overlapAsgn) {
                        result->append(overlapAsgn);
                    }
                }
                child = parent; // up one level
            }
        }

        // now process the effects of assignment "down" the register tree
        // e.g. the effects on %ah when assigning to %eax
        {
            const Register *base = getRegByID(myID);
            std::stack<std::pair<const Register *, int>> toVisit({ { base, 0 } });

            while (!toVisit.empty()) {
                const auto &[current, offset] = toVisit.top();
                toVisit.pop();

                if (current != base) {
                    const RegID currentID = getRegIDByName(current->getName());

                    if (m_offsetInParent.find(current->getName()) != m_offsetInParent.end()) {
                        // is the parent actually used? if not, then skip
                        if (usedRegs.find(currentID) != usedRegs.end()) {
                            Assignment *overlapAsgn = emitOverlappedStmt(stmt, current, base,
                                                                         offset);
                            if (overlapAsgn) {
                                result->append(overlapAsgn);
                            }
                        }
                    }
                }

                if (m_children.find(current->getName()) != m_children.end()) {
                    // recurse to children if there are any
                    for (const auto &[childOffset, child] : m_children.at(current->getName())) {
                        toVisit.push({ getRegByName(child), offset + childOffset });
                    }
                }
            }
        }
    }
    catch (std::out_of_range &) {
        assert(false);
    }

    return result;
}


Assignment *RegDB::emitOverlappedStmt(const Assignment *original, const Register *lhs,
                                      const Register *rhs, int offsetInParent) const
{
    const RegID lhsID = getRegIDByName(lhs->getName());
    const RegID rhsID = getRegIDByName(rhs->getName());

    if (lhsID == RegIDSpecial || rhsID == RegIDSpecial) {
        return nullptr;
    }
    assert(lhsID != rhsID);

    Assign *result = nullptr;
    if (lhs->getSize() <= rhs->getSize()) {
        // emit lhs = rhs@[offset:(offset + lhs->size -1)]
        result = new Assign(IntegerType::get(lhs->getSize()), Location::regOf(lhsID),
                            Ternary::get(opAt, Location::regOf(rhsID), Const::get(offsetInParent),
                                         Const::get(offsetInParent + lhs->getSize() - 1)));
    }
    else {
        const unsigned int mask = ~(Util::getLowerBitMask(rhs->getSize()) << offsetInParent);

        // emit lhs := (lhs & mask) | (zfill(rhs) << offset)
        result = new Assign(
            IntegerType::get(lhs->getSize()), Location::regOf(lhsID),
            Binary::get(
                opBitOr, Binary::get(opBitAnd, Location::regOf(lhsID), Const::get(mask)),
                Binary::get(opShiftL,
                            Ternary::get(opZfill, Const::get(rhs->getSize()),
                                         Const::get(lhs->getSize()), Location::regOf(rhsID)),
                            Const::get(offsetInParent))));
    }

    if (original->isAssign() && static_cast<const Assign *>(original)->getGuard()) {
        result->setGuard(static_cast<const Assign *>(original)->getGuard()->clone());
    }

    result->simplify();
    return result;
}
