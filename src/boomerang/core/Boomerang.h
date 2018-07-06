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


#include "boomerang/core/Settings.h"

#include <memory>
#include <set>


class QString;
class Function;
class UserProc;
class Project;
class IWatcher;


/**
 * Controls the loading, decoding, decompilation and code generation for a program.
 * This is the main class of the decompiler.
 */
class Boomerang
{
private:
    /**
     * Initializes the Boomerang object.
     * The default settings are:
     * - All options disabled
     * - Infinite propagations
     * - A maximum memory depth of 99
     * - The path to the executable is "./"
     * - The output directory is "./output/"
     * - Main log stream is output on stderr
     */
    Boomerang();
    Boomerang(const Boomerang& other) = delete;
    Boomerang(Boomerang&& other) = default;

    virtual ~Boomerang() = default;

    Boomerang& operator=(const Boomerang& other) = delete;
    Boomerang& operator=(Boomerang&& other) = default;

public:
    /// \returns The global boomerang object. It will be created if it does not already exist.
    static Boomerang *get();
    static void destroy();

public:

};
