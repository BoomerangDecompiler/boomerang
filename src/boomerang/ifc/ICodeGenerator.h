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
class Prog;
class StatementList;
class UserProc;

class QString;
class OStream;


/**
 * Base class for generating high-level code from statements.
 *
 * This class is provides methods which are generic of procedural
 * languages like C, Pascal, Fortran etc. Included in the base class
 * is the follow and goto sets which are used during code generation.
 * Concrete implementations of this class provide specific language
 * bindings for a single procedure in the program.
 */
class ICodeGenerator
{
public:
    ICodeGenerator() = default;
    virtual ~ICodeGenerator() = default;

public:
    /// Generate code for \p program to \p os.
    virtual void generateCode(const Prog *program, OStream& os) = 0;

    /**
     * Generate code for a module or function, or all modules.
     * \param program The program to generate code for.
     * \param module The module to generate code for, or nullptr to generate code for all modules.
     * \param proc The function to generate code for, or nullptr to generate code for all procedures in a module.
     * \param intermixRTL Set this to true to intermix code with underlying intermediate representation.
     *                    Currently not implemented.
     */
    virtual void generateCode(const Prog *program, Module *module = nullptr, UserProc *proc = nullptr, bool intermixRTL = false) = 0;

public:
    /*
     * Functions to add new code, pure virtual.
     * DEPRECATED
     */
    // sequential statements

    /// Add an assignment statement at the current position.
    virtual void addAssignmentStatement(Assign *s) = 0;

    /**
     * Adds a call to the function \p proc.
     *
     * \param dest           The Proc the call is to.
     * \param name           The name the Proc has.
     * \param args           The arguments to the call.
     * \param results        The variable that will receive the return value of the function.
     *
     * \todo                Remove the \p name parameter and use Proc::getName()
     * \todo                Add assignment for when the function returns a struct.
     */
    virtual void addCallStatement(Function *dest, const QString& name,
                                  const StatementList& args, const StatementList& results) = 0;

    /**
     * Adds an indirect call to \a exp.
     * \see AddCallStatement
     * \param results UNUSED
     * \todo Add the use of \a results like AddCallStatement.
     */
    virtual void addIndCallStatement(const SharedExp& exp, const StatementList& args,
                                     const StatementList& results) = 0;

    /**
     * Adds a return statement and returns the first expression in \a rets.
     * \todo This should be returning a struct if more than one real return value.
     */
    virtual void addReturnStatement(const StatementList *rets) = 0;

    /// Removes unused labels from the code.
    virtual void removeUnusedLabels() = 0;
};
