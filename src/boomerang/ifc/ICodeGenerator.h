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


#include "boomerang/ssl/exp/ExpHelp.h"


class Assign;
class Function;
class Module;
class OStream;
class Prog;
class Project;
class StatementList;
class UserProc;

class QString;


/// Base class for generating high-level code from SSL statements.
/// Concrete implementations of this class provide specific language
/// bindings.
class BOOMERANG_API ICodeGenerator
{
public:
    ICodeGenerator(Project *) { }
    virtual ~ICodeGenerator() = default;

public:
    /**
     * Generate code for a module or function, or all modules.
     * \param program The program to generate code for.
     * \param module The module to generate code for, or nullptr to generate code for all modules.
     * \param proc The function to generate code for, or nullptr to generate code for all procedures
     *             in a module.
     * \param intermixRTL Set this to true to intermix code with underlying
     * intermediate representation. Currently not implemented.
     */
    virtual void generateCode(const Prog *program, Module *module = nullptr,
                              UserProc *proc = nullptr, bool intermixRTL = false) = 0;
};
