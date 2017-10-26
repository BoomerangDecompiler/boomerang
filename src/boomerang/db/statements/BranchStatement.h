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


#include "boomerang/db/statements/GotoStatement.h"

/**
 * BranchStatement has a condition Exp in addition to the destination of the jump.
 */
class BranchStatement : public GotoStatement
{
public:
    /**
     * \fn        BranchStatement::BranchStatement
     * \brief     Constructor.
     */
    BranchStatement();

    /**
     * \fn        BranchStatement::~BranchStatement
     * \brief     Destructor
     */
    virtual ~BranchStatement() override;

    // Make a deep copy, and make the copy a derived object if needed.

    /**
     * \fn        BranchStatement::clone
     * \brief     Deep copy clone
     * \returns   Pointer to a new Instruction, a clone of this BranchStatement
     */
    virtual Statement *clone() const override;

    /// Accept a visitor to this Statement
    /// visit this stmt
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    // Set and return the BRANCH_TYPE of this jcond as well as whether the
    // floating point condition codes are used.

    /**
     * \fn    BranchStatement::setCondType
     * \brief Sets the BRANCH_TYPE of this jcond as well as the flag
     *        indicating whether or not the floating point condition codes
     *        are used.
     * \param cond - the BRANCH_TYPE
     * \param usesFloat - this condional jump checks the floating point condition codes
     */
    void setCondType(BranchType cond, bool usesFloat = false);

    BranchType getCond() const { return m_jumpType; }
    bool isFloat() const { return m_isFloat; }
    void setFloat(bool b) { m_isFloat = b; }

    // Set and return the Exp representing the HL condition

    /**
     * \fn      BranchStatement::getCondExpr
     * \brief   Return the SemStr expression containing the HL condition.
     * \returns ptr to an expression
     */
    SharedExp getCondExpr() const;

    /**
     * \fn          BranchStatement::setCondExpr
     * \brief       Set the SemStr expression containing the HL condition.
     * \param       pe - Pointer to Exp to set
     */
    void setCondExpr(SharedExp pe);

    BasicBlock *getFallBB() const;
    BasicBlock *getTakenBB() const;

    /// not that if you set the taken BB or fixed dest first,
    /// you will not be able to set the fall BB
    void setFallBB(BasicBlock *bb);
    void setTakenBB(BasicBlock *bb);

    /**
     * \fn        BranchStatement::print
     * \brief     Write a text representation to the given stream
     * \param     os stream
     * \param     html produce html encoded representation
     */
    void print(QTextStream& os, bool html = false) const override;

    // general search
    bool search(const Exp& search, SharedExp& result) const override;

    /**
     * \fn    BranchStatement::searchAndReplace
     * \brief Replace all instances of search with replace.
     * \param search - a location to search for
     * \param replace - the expression with which to replace it
     * \param cc - ignored
     * \returns True if any change
     */
    bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    // Searches for all instances of a given subexpression within this
    // expression and adds them to a given list in reverse nesting order.

    /**
     * \brief   Find all instances of the search expression
     * \param   search - a location to search for
     * \param   result - a list which will have any matching exprs
     *          appended to it
     * \returns true if there were any matches
     */
    bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    /// code generation
    virtual void generateCode(ICodeGenerator *generator, const BasicBlock *parentBB) override;

    /// dataflow analysis
    bool usesExp(const Exp& e) const override;

    /// simplify all the uses/defs in this Statememt
    void simplify() override;

    /// Generate constraints
    void genConstraints(LocationSet& cons) override;

    /// Data flow based type analysis
    void dfaTypeAnalysis(bool& ch) override;

private:
    BranchType m_jumpType; ///< The condition for jumping
    SharedExp m_cond;      ///< The Exp representation of the high level condition: e.g., r[8] == 5
    bool m_isFloat;        ///< True if uses floating point CC
    // jtCond seems to be mainly needed for the Pentium weirdness.
    // Perhaps m_isFloat, m_jumpType, and m_size could one day be merged into a type
};
