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
#include "boomerang/util/Address.h"

#include <vector>


class IDecoder;
class Project;
class UserProc;
class QString;


/// Disassembles a binary file into Functions and BasicBlocks and lifts them to IRFragments.
class BOOMERANG_API IFrontEnd
{
public:
    IFrontEnd(Project *) {}
    virtual ~IFrontEnd() = default;

public:
    virtual bool initialize(Project *project) = 0;

    virtual IDecoder *getDecoder()             = 0;
    virtual const IDecoder *getDecoder() const = 0;

public:
    /// Disassemble all entry points.
    /// \returns true for a good decode (no invalid instructions)
    [[nodiscard]] virtual bool disassembleEntryPoints() = 0;

    /// Disassemble all functions until there are no un-disassembled functions left.
    /// \returns true if decoded successfully.
    [[nodiscard]] virtual bool disassembleAll() = 0;

    /// Disassemble the funnction beginning at address \p addr.
    /// Creates the function if it does not yet exist.
    /// \returns true iff decoded successfully.
    [[nodiscard]] virtual bool disassembleFunctionAtAddr(Address addr) = 0;

    /// Disassemble a single procedure (or a fragment thereof), starting at \p addr.
    /// \param proc the procedure object
    /// \param addr the entry address of \p proc
    /// \returns true for a good decode (no illegal instructions)
    [[nodiscard]] virtual bool disassembleProc(UserProc *proc, Address addr) = 0;

    /// Lift all instructions for a proc.
    /// \returns true on success, false on failure
    [[nodiscard]] virtual bool liftProc(UserProc *proc) = 0;

public:
    /// \returns the address of "main", or Address::INVALID if not found
    virtual Address findMainEntryPoint(bool &gotMain) = 0;

    /// Returns a list of all available entrypoints.
    virtual std::vector<Address> findEntryPoints() = 0;

    /// Determines whether the proc with name \p procName returns or not (like abort)
    virtual bool isNoReturnCallDest(const QString &procName) const = 0;

    /// Add a "hint" that an instruction at \p addr references a named global
    virtual void addRefHint(Address addr, const QString &name) = 0;
};
