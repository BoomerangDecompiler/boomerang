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
#include "boomerang/frontend/DefaultFrontEnd.h"


/**
 * Contains routines to manage the decoding of PPC binaries.
 */
class BOOMERANG_PLUGIN_API PPCFrontEnd : public DefaultFrontEnd
{
public:
    /// \copydoc IFrontEnd::IFrontEnd
    PPCFrontEnd(Project *project);
    PPCFrontEnd(const PPCFrontEnd &other) = delete;
    PPCFrontEnd(PPCFrontEnd &&other)      = default;

    /// \copydoc IFrontEnd::~IFrontEnd
    virtual ~PPCFrontEnd() override = default;

    PPCFrontEnd &operator=(const PPCFrontEnd &other) = delete;
    PPCFrontEnd &operator=(PPCFrontEnd &&other) = default;

public:
    /// \copydoc IFrontEnd::processProc
    virtual bool processProc(UserProc *proc, Address entryAddr) override;

    /// \copydoc IFrontEnd::getMainEntryPoint
    virtual Address findMainEntryPoint(bool &gotMain) override;
};
