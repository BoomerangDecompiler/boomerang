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


#include "boomerang/ssl/statements/GotoStatement.h"


enum class SwitchType : char
{
    Invalid = 0,
    a       = 'a',
    A       = 'A',
    o       = 'o',
    O       = 'O',
    r       = 'r',
    R       = 'R',
    H       = 'H',
    F       = 'F', // Fortran style
};

/**
 * CaseStatement is derived from GotoStatement. In addition to the destination
 * of the jump, it has a switch variable Exp.
 */
struct SwitchInfo
{
    SharedExp switchExp;   ///< Expression to switch on, e.g. v[7]
    SwitchType switchType; ///< Switch type: 'A', 'O', 'R', 'H', or 'F' etc
    int lowerBound;        ///< Lower bound of the switch variable
    int upperBound;        ///< Upper bound for the switch variable
    Address tableAddr;     ///< Native address of the table, or ptr to array of values for form F
    int numTableEntries;   ///< Number of entries in the table (form H only)
    int offsetFromJumpTbl = 0; ///< Distance from jump to table (form R only)
};


class BOOMERANG_API CaseStatement : public GotoStatement
{
public:
    CaseStatement();
    CaseStatement(const CaseStatement &other) = default;
    CaseStatement(CaseStatement &&other)      = default;

    virtual ~CaseStatement() override;

    CaseStatement &operator=(const CaseStatement &other) = default;
    CaseStatement &operator=(CaseStatement &&other) = default;

public:
    /// \copydoc GotoStatement::clone
    virtual Statement *clone() const override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtVisitor *visitor) const override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc GotoStatement::accept
    virtual bool accept(StmtPartModifier *modifier) override;

    /// \copydoc GotoStatement::print
    virtual void print(OStream &os) const override;

    /// \copydoc GotoStatement::searchAndReplace
    virtual bool searchAndReplace(const Exp &search, SharedExp replace, bool cc = false) override;

    /// \copydoc GotoStatement::searchAll
    virtual bool searchAll(const Exp &search, std::list<SharedExp> &result) const override;

    /// \copydoc GotoStatement::simplify
    virtual void simplify() override;

    /// Get information about this switch statement
    SwitchInfo *getSwitchInfo();
    const SwitchInfo *getSwitchInfo() const;

    void setSwitchInfo(std::unique_ptr<SwitchInfo> psi);

private:
    std::unique_ptr<SwitchInfo> m_switchInfo; ///< Ptr to struct with information about the switch
};
