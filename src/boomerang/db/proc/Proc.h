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


#include "boomerang/core/BoomerangAPI.h"
#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/Address.h"

#include <QString>

#include <set>


class Module;
class CallStatement;
class Prog;
class Signature;


/**
 * Interface for the procedure classes, which are used to store information
 * about variables in the procedure such as parameters and locals.
 */
class BOOMERANG_API Function
{
public:
    typedef std::set<std::shared_ptr<CallStatement>> CallerSet;

public:
    /**
     * \param address   Address of entry point of procedure
     * \param signature The Signature for this Proc
     * \param module    The Module this procedure belongs to
     */
    Function(Address address, const std::shared_ptr<Signature> &signature, Module *module);
    Function(const Function &) = delete;
    Function(Function &&)      = default;

    virtual ~Function();

    Function &operator=(const Function &) = delete;
    Function &operator=(Function &&) = default;

public:
    /// Get the name of this procedure.
    QString getName() const;

    /// Rename this procedure.
    void setName(const QString &name);

    /// Get the address of the entry point of this procedure.
    Address getEntryAddress() const;

    /// Set the entry address of this procedure
    void setEntryAddress(Address addr);

    virtual bool isLib() const { return false; } ///< Return true if this is a library proc
    virtual bool isNoReturn() const = 0;         ///< Return true if this procedure doesn't return

    /// Get the program this procedure belongs to.
    Prog *getProg();
    const Prog *getProg() const;

    Module *getModule() { return m_module; }
    const Module *getModule() const { return m_module; }

    void setModule(Module *module);
    void removeFromModule();

    std::shared_ptr<Signature> getSignature() const { return m_signature; }
    void setSignature(std::shared_ptr<Signature> sig);

    /// \returns the call statements that call this function.
    const CallerSet &getCallers() const { return m_callers; }
    CallerSet &getCallers() { return m_callers; }

    /// Add to the set of callers
    void addCaller(const std::shared_ptr<CallStatement> &caller) { m_callers.insert(caller); }
    void removeCaller(const std::shared_ptr<CallStatement> &caller) { m_callers.erase(caller); }

    void removeParameterFromSignature(SharedExp e);

    /// Rename the first parameter named \p oldName to \p newName.
    void renameParameter(const QString &oldName, const QString &newName);

    /// Get the RHS, if any, that is proven for left
    virtual SharedExp getProven(SharedExp left) = 0;

    /// Get the RHS, if any, that is premised for left
    virtual SharedExp getPremised(SharedExp left) = 0;

    /// \returns true iff \p e is preserved by this proc
    virtual bool isPreserved(SharedExp e) = 0;

protected:
    Prog *m_prog           = nullptr;          ///< Program containing this function.
    Module *m_module       = nullptr;          ///< Module containing this function.
    Address m_entryAddress = Address::INVALID; ///< Entry address of this function.

    /**
     * The formal signature of this procedure.
     * This information is determined either by the common.hs file (for a library function) or by
     * analysis.
     * \note This belongs in the CALL, because the same procedure can have different
     * signatures if it happens to have varargs. Temporarily here till it can be permanently
     * moved.
     */
    std::shared_ptr<Signature> m_signature;

    /// Set of callers (CallStatements that call this procedure).
    CallerSet m_callers;
};
