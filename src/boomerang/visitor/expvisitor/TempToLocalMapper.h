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


#include "boomerang/ssl/type/Type.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class UserProc;


/**
 * Maps temporary variables in IR to local variables (e.g. tmpf -> local0)
 */
class TempToLocalMapper : public ExpVisitor
{
public:
    TempToLocalMapper(UserProc *proc);
    virtual ~TempToLocalMapper() = default;

public:
    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

private:
    /**
     * Given the name of a temporary variable, return its Type.
     * The type of a variable is determined by the value of the fourth character.
     * Types are mapped as follows:
     *  - 'f' -> f32
     *  - 'd' -> f64
     *  - 'F' -> f80
     *  - 'D' -> f128
     *  - 'l' -> j64
     *  - 'h' -> j16
     *  - 'b' -> j8
     * Otherwise, this function returns j32.
     *
     * \param   name reference to a string (e.g. "tmp", "tmpd")
     * \returns Ptr to a new Type object
     */
    SharedType getTempType(const QString &name);

private:
    UserProc *m_proc; ///< Proc object for storing the symbols
};
