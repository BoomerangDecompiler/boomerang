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


#include "boomerang/db/statements/Statement.h"
#include "boomerang/db/exp/Const.h"


/**
 * GotoStatement has just one member variable, an expression representing the
 * jump's destination (an integer constant for direct jumps; an expression
 * for register jumps). An instance of this class will never represent a
 * return or computed call as these are distinguised by the decoder and are
 * instantiated as CallStatements and ReturnStatements respecitvely.
 * This class also represents unconditional jumps with a fixed offset
 * (e.g BN, Ba on SPARC).
 */
class GotoStatement : public Statement
{
public:
    GotoStatement();

    /// Construct a jump to a fixed address \p jumpDest
    GotoStatement(Address jumpDest);
    GotoStatement(const GotoStatement& other) = default;
    GotoStatement(GotoStatement&& other) = default;

    virtual ~GotoStatement() override;

    GotoStatement& operator=(const GotoStatement& other) = default;
    GotoStatement& operator=(GotoStatement&& other) = default;

public:
    /// \copydoc Statement::clone
    virtual Statement *clone() const override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtVisitor *visitor) const override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtPartModifier *modifier) override;

    /// Set the destination of this jump to be a given fixed address.
    void setDest(Address addr);

    /// Set the destination of this jump to be an expression (computed jump)
    void setDest(SharedExp pd);

    /// Returns the destination of this CTI.
    virtual SharedExp getDest();
    virtual const SharedExp getDest() const;

    /// \returns Fixed destination address or Address::INVALID if there isn't one.
    /// For dynamic CTIs, returns Address::INVALID.
    Address getFixedDest() const;

    /**
     * Adjust the destination of this CTI by a given amount (i.e. a constant offset).
     * Causes an error if this destination is not a fixed destination.
     * \param delta the amount to add to the destination (can be negative)
     */
    void adjustFixedDest(int delta);

    /**
     * Sets the fact that this call is computed.
     * \todo This should really be removed, once CaseStatement
     * and HLNwayCall are implemented properly
     */
    void setIsComputed(bool computed = true);

    /**
     * Returns whether or not this call is computed.
     * \todo This should really be removed, once CaseStatement
     * and HLNwayCall are implemented properly
     * \returns this call is computed
     */
    bool isComputed() const;

    /// \copydoc Statement::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Statement::search
    virtual bool search(const Exp& pattern, SharedExp& result) const override;

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAndReplace(const Exp& pattern, SharedExp replace, bool cc = false) override;


    // code generation
    virtual void generateCode(ICodeGenerator *, const BasicBlock *) override;

    // simplify all the uses/defs in this Statement
    virtual void simplify() override;

    // Statement virtual functions
    virtual bool isDefinition() const override { return false; }
    virtual bool usesExp(const Exp&) const override;

protected:
    /// NOTE: This should be removed, once CaseStatement and HLNwayCall are implemented
    /// properly.
    std::shared_ptr<Const> constDest() { return std::static_pointer_cast<Const>(m_dest); }
    const std::shared_ptr<const Const> constDest() const { return std::static_pointer_cast<const Const>(m_dest); }

protected:
    /// Destination of a jump or call. This is the absolute destination
    /// for both static and dynamic CTIs.
    SharedExp m_dest;
    bool m_isComputed; ///< True if this is a CTI with a computed destination address.
};
