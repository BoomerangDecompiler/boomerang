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


#include "Proc.h"


/**
 * Class for library procedures called by the program (like printf).
 */
class BOOMERANG_API LibProc : public Function
{
public:
    /**
     * Constructor with name, native address.
     * \param        address Address of entry point of function
     * \param        name    Name of function
     * \param        module  Module that contains this library function
     */
    LibProc(Address address, const QString &name, Module *module);
    LibProc(const LibProc &) = delete;
    LibProc(LibProc &&)      = default;

    ~LibProc() override = default;

    LibProc &operator=(const LibProc &) = delete;
    LibProc &operator=(LibProc &&) = default;

public:
    /// \copydoc Function::isLib
    bool isLib() const override;

    /// \copydoc Function::isNoReturn
    bool isNoReturn() const override;

    /// \copydoc Function::getProven
    SharedExp getProven(SharedExp left) override;

    /// \copydoc Function::getPremised
    /// Get the RHS that is premised for left
    SharedExp getPremised(SharedExp) override;

    /// \copydoc Function::isPreserved
    bool isPreserved(SharedExp e) override;
};
