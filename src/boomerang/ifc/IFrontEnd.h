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
#include "boomerang/frontend/SigEnum.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Address.h"

#include <list>
#include <memory>
#include <vector>


class BasicBlock;
class CallStatement;
class DecodeResult;
class Exp;
class IDecoder;
class Project;

class Signature;
class Statement;
class UserProc;

class QString;

using SharedExp      = std::shared_ptr<Exp>;
using SharedConstExp = std::shared_ptr<const Exp>;
using RTLList        = std::list<std::unique_ptr<RTL>>;


/**
 * Decodes a binary file into Functions and BasicBlocks.
 */
class BOOMERANG_API IFrontEnd
{
public:
    IFrontEnd(Project *) {}
    virtual ~IFrontEnd() = default;

public:
    virtual bool initialize(Project *project) = 0;

    virtual IDecoder *getDecoder()             = 0;
    virtual const IDecoder *getDecoder() const = 0;

    /// Decode all undecoded procedures.
    /// \returns true for a good decode (no invalid instructions)
    virtual bool decodeEntryPointsRecursive(bool decodeMain = true) = 0;

    /// Decode all procs starting at a given address
    /// \returns true iff decoded successfully.
    virtual bool decodeRecursive(Address addr) = 0;

    /// Decode all undecoded functions.
    /// \returns true if decoded successfully.
    virtual bool decodeUndecoded() = 0;

    /// Decode a fragment of a procedure, e.g. for each destination of a switch statement
    /// \returns true iff decoded successfully.
    virtual bool decodeFragment(UserProc *proc, Address addr) = 0;

    /**
     * Process a procedure, given a native (source machine) address.
     * This is the main function for decoding a procedure.
     *
     * \param proc the procedure object
     * \param addr the entry address of \p proc
     *
     * \returns true for a good decode (no illegal instructions)
     */
    virtual bool processProc(UserProc *proc, Address addr) = 0;

    /// Lift all instructions for a proc.
    /// \returns true on success, false on failure
    [[nodiscard]] virtual bool liftProc(UserProc *proc) = 0;

public:
    /// Locate the entry address of "main", returning a native address
    virtual Address findMainEntryPoint(bool &gotMain) = 0;

    /// Returns a list of all available entrypoints.
    virtual std::vector<Address> findEntryPoints() = 0;

    /// Determines whether the proc with name \p procName returns or not (like abort)
    virtual bool isNoReturnCallDest(const QString &procName) const = 0;

    /// Add a "hint" that an instruction at \p addr references a named global
    virtual void addRefHint(Address addr, const QString &name) = 0;
};
