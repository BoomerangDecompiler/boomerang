#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DataIntervalMap.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/type/ArrayType.h"
#include "boomerang/ssl/type/CompoundType.h"
#include "boomerang/util/Util.h"
#include "boomerang/util/log/Log.h"


DataIntervalMap::DataIntervalMap(UserProc *userProc)
    : m_proc(userProc)
{
}


bool DataIntervalMap::isClear(Address addr, unsigned size) const
{
    VariableMap::const_iterator it1, it2;
    std::tie(it1, it2) = m_varMap.equalRange(addr, addr + size);

    if ((it1 == m_varMap.end()) && (it2 == m_varMap.end())) {
        return true; // no variable blocking
    }
    else if (it2 == m_varMap.end()) {
        return false;
    }

    for (VariableMap::const_iterator it = it1; it != it2; ++it) {
        // since we don't know the length of unbounded arrays (yet),
        // it is possible that the array ends before \p addr.
        // Therefore, allow these intervals to be marked as "clear"
        if (!it->second.type->isArray() || !it->second.type->as<ArrayType>()->isUnbounded()) {
            return false;
        }
    }

    return true; // only unbounded arrays "blocking"
}


bool DataIntervalMap::isClear(Address lower, Address upper) const
{
    assert(upper >= lower);
    return isClear(lower, (upper - lower).value());
}


bool DataIntervalMap::isClear(const Interval<Address> interval) const
{
    return isClear(interval.lower(), interval.upper());
}


const TypedVariable *DataIntervalMap::find(Address addr) const
{
    const_iterator it = find_it(addr);

    return it != m_varMap.end() ? (&it->second) : nullptr;
}


DataIntervalMap::const_iterator DataIntervalMap::find_it(Address addr) const
{
    return m_varMap.find(addr);
}


DataIntervalMap::iterator DataIntervalMap::insertItem(Address baseAddr, QString name,
                                                      SharedType type, bool forced)
{
    if (name.isEmpty()) {
        name = "<noname>";
    }

    const Interval<Address> newTypeRange(baseAddr, baseAddr + type->getSize() / 8);

    iterator it1, it2;
    std::tie(it1, it2) = m_varMap.equalRange(newTypeRange);

    if ((it1 == end()) && (it2 == end())) {
        // not overlapped by any variable -> just insert directly
        return m_varMap.insert(newTypeRange, TypedVariable(baseAddr, name, type));
    }

    // try to adjust types such that the type can be inserted
    while (it1 != it2) {
        TypedVariable &var = it1->second;

        if (it1->first.containsInterval(newTypeRange)) {
            if (newTypeRange.containsInterval(it1->first)) {
                // both types are of equal size
                bool changed;
                it1->second.type = it1->second.type->meetWith(type, changed);
                return it1;
            }
            else {
                // new type may be part of an existing type
                insertComponentType(&var, baseAddr, name, type, forced);
                return it1;
            }
        }
        else if (newTypeRange.containsInterval(it1->first)) {
            // the new type is a larger/derived type which contains the old type
            return replaceComponents(baseAddr, name, type, forced);
        }
        else {
            // the new and the old type do not completely overlap
            // -> fail or insert anyway if forced
            if (forced) {
                clearRange(newTypeRange);
                assert(isClear(newTypeRange));
                return m_varMap.insert(newTypeRange, TypedVariable(baseAddr, name, type));
            }
            else {
                LOG_ERROR("TYPE ERROR: Cannot insert variable of type %1 at address %2",
                          type->getCtype(), baseAddr);
                LOG_ERROR("TYPE ERROR: because it conflicts with variable of type %1 at address %2",
                          var.type->getCtype(), var.baseAddr);
                return end();
            }
        }
    }

    return end();
}


void DataIntervalMap::insertComponentType(TypedVariable *existingVar, Address addr,
                                          const QString & /*name*/, SharedType type,
                                          bool /*forced*/)
{
    assert(existingVar);
    assert(existingVar->baseAddr <= addr);

    if (existingVar->type->resolvesToCompound()) {
        const uint64 bitOffset = (addr - existingVar->baseAddr).value() * 8;
        SharedType memberType  = existingVar->type->as<CompoundType>()->getMemberTypeByOffset(
            bitOffset);

        if (!memberType || !memberType->isCompatibleWith(*type)) {
            LOG_ERROR("TYPE ERROR: At address %1 type %2 is not compatible with existing structure "
                      "type %3",
                      addr, type->getCtype(), existingVar->type->getCtype());
        }

        // types are compatible
        bool ch;
        memberType = memberType->meetWith(type, ch);
        existingVar->type->as<CompoundType>()->setMemberTypeByOffset(bitOffset, memberType);
    }
    else if (existingVar->type->resolvesToArray()) {
        SharedType baseType = existingVar->type->as<ArrayType>()->getBaseType();
        assert(baseType);

        if (!baseType->isCompatibleWith(*type)) {
            LOG_ERROR("TYPE ERROR: At address %1 type %2 is not compatible with existing array "
                      "member type %3",
                      addr, type->getCtype(), baseType->getCtype());
            return;
        }

        // types are compatible -> change it
        bool ch;
        existingVar->type->as<ArrayType>()->setBaseType(baseType->meetWith(type, ch));
    }
    else {
        LOG_ERROR("TYPE ERROR: Existing type at address %1 is not structure or array type",
                  existingVar->baseAddr);
    }
}


DataIntervalMap::iterator DataIntervalMap::replaceComponents(Address addr, const QString &name,
                                                             SharedType ty, bool /*forced*/)
{
    // This is the byte address just past the type to be inserted
    const Address endAddr = addr + ty->getSize() / 8;

    VariableMap::const_iterator it1, it2;

    // First check that the new entry will be compatible with everything it will overlap
    if (ty->resolvesToCompound()) {
        std::tie(it1, it2) = m_varMap.equalRange(addr, endAddr);

        for (VariableMap::const_iterator it = it1; it != it2; ++it) {
            unsigned bitOffset = (it->second.baseAddr - addr).value() * 8;

            SharedType memberType = ty->as<CompoundType>()->getMemberTypeByOffset(bitOffset);

            if (!memberType->isCompatibleWith(*it->second.type, true)) {
                LOG_ERROR("TYPE ERROR: At address %1 struct type %2 is not compatible with "
                          "existing type %3",
                          addr, ty->getCtype(), it->second.type->getCtype());
                return end();
            }

            LOG_VERBOSE("%1", this->toString());
            LOG_VERBOSE("%1 %2", memberType->getCtype(), it->second.type->getCtype());

            bool ch;
            memberType = it->second.type->meetWith(memberType, ch);
            ty->as<CompoundType>()->setMemberTypeByOffset(bitOffset, memberType);
        }
    }
    else if (ty->resolvesToArray()) {
        SharedType memberType = ty->as<ArrayType>()->getBaseType();
        std::tie(it1, it2)    = m_varMap.equalRange(addr, endAddr);

        for (VariableMap::const_iterator it = it1; it != it2; ++it) {
            if (memberType->isCompatibleWith(*it->second.type, true)) {
                bool ch;
                memberType = memberType->meetWith(it->second.type, ch);
                ty->as<ArrayType>()->setBaseType(memberType);
            }
            else {
                LOG_ERROR("TYPE ERROR: At address %1 array type %2 is not compatible with existing "
                          "type %3",
                          addr, ty->getCtype(), it->second.type->getCtype());
                return end();
            }
        }
    }
    else {
        // Just make sure it doesn't overlap anything
        if (!isClear(addr, (ty->getSize() + 7) / 8)) {
            LOG_ERROR("TYPE ERROR: at address %1, overlapping type %2 "
                      "does not resolve to compound or array",
                      addr, ty->getCtype());
            return end();
        }
    }

    // The compound or array type is compatible. Remove the items that it will overlap with
    std::tie(it1, it2) = m_varMap.equalRange(addr, endAddr);

    // Check for existing locals that need to be updated
    if (ty->resolvesToCompound() || ty->resolvesToArray()) {
        SharedExp rsp = Location::regOf(m_proc->getSignature()->getStackRegister());
        auto rsp0     = RefExp::get(rsp, m_proc->getCFG()->findTheImplicitAssign(rsp)); // sp{0}

        for (VariableMap::const_iterator it = it1; it != it2; ++it) {
            // Check if there is an existing local here
            SharedExp locl = Location::memOf(
                Binary::get(opPlus, rsp0->clone(), Const::get(it->second.baseAddr.native())));
            locl->simplifyArith(); // Convert m[sp{0} + -4] to m[sp{0} - 4]
            SharedType elemTy;
            int bitOffset = (it->second.baseAddr - addr).value() / 8;

            if (ty->resolvesToCompound()) {
                elemTy = ty->as<CompoundType>()->getMemberTypeByOffset(bitOffset);
            }
            else {
                elemTy = ty->as<ArrayType>()->getBaseType();
            }

            QString locName = m_proc->findLocal(locl, elemTy);

            if (!locName.isEmpty() && ty->resolvesToCompound()) {
                auto c = ty->as<CompoundType>();
                // want s.m where s is the new compound object and m is the member at offset
                // bitOffset
                QString memName = c->getMemberNameByOffset(bitOffset);
                SharedExp s = Location::memOf(Binary::get(opPlus, rsp0->clone(), Const::get(addr)));
                s->simplifyArith();
                SharedExp memberExp = Binary::get(opMemberAccess, s, Const::get(memName));
                m_proc->mapSymbolTo(locl, memberExp);
            }
            else {
                // FIXME: to be completed
            }
        }
    }

    m_varMap.eraseAll(Interval<Address>(addr, addr + ty->getSizeInBytes()));

    return m_varMap.insert(addr, addr + ty->getSizeInBytes(), TypedVariable(addr, name, ty));
}


void DataIntervalMap::checkMatching(TypedVariable *pdie, Address addr, const QString & /*name*/,
                                    SharedType ty, bool /*forced*/)
{
    if (pdie->type->isCompatibleWith(*ty)) {
        // Just merge the types and exit
        bool ch    = false;
        pdie->type = pdie->type->meetWith(ty, ch);
        return;
    }

    LOG_MSG("TYPE DIFFERENCE (could be OK): At address %1 existing type %2 but added type %3", addr,
            pdie->type->getCtype(), ty->getCtype());
}


void DataIntervalMap::deleteItem(Address addr)
{
    m_varMap.eraseAll(addr);
}


QString DataIntervalMap::toString()
{
    QString tgt;
    OStream ost(&tgt);

    for (const auto &varPair : m_varMap) {
        const Interval<Address> &varRange = varPair.first;
        const TypedVariable &var          = varPair.second;
        ost << varRange.lower() << "-" << varRange.upper() << " " << var.name << " "
            << var.type->getCtype() << "\n";
    }

    return tgt;
}


void DataIntervalMap::clearRange(const Interval<Address> &interval)
{
    iterator it, it2;

    std::tie(it, it2) = m_varMap.equalRange(interval);

    while (it != it2) {
        TypedVariable &var = it->second;
        const Interval<Address> typeRange(var.baseAddr, var.baseAddr + 8 * var.size);

        if (var.type->resolvesToArray() && (var.baseAddr < interval.lower()) &&
            var.type->as<ArrayType>()->isUnbounded()) {
            // unbounded array -> just adjust the range of the array
            // to not overlap with the interval
            uint64 newSize = interval.lower().value() - var.baseAddr.value();

            // we have a whole number as number of elements;
            newSize %= var.type->as<ArrayType>()->getBaseType()->getSize();

            LOG_VERBOSE("Adjusting size of unbounded array at address %1 to %2 bytes", var.baseAddr,
                        newSize / 8);
            var.size = newSize / 8;
            var.type->as<ArrayType>()->setLength(
                8 * var.size / var.type->as<ArrayType>()->getBaseType()->getSize());

            it = std::next(it);
        }
        else {
            // type is in the way -> just delete it
            it = m_varMap.erase(it);
        }
    }
}
