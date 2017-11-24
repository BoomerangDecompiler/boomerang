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


#include "boomerang/codegen/syntax/SyntaxNode.h"
#include "boomerang/db/exp/ExpHelp.h"


/**
 * Syntax node for loops
 */
class LoopSyntaxNode : public SyntaxNode
{
public:
    LoopSyntaxNode(SyntaxNode *body = nullptr, SharedExp cond = nullptr, bool postTested = false);
    LoopSyntaxNode(const LoopSyntaxNode& other) = delete;
    LoopSyntaxNode(LoopSyntaxNode&& other) = delete;

    virtual ~LoopSyntaxNode() override;

    LoopSyntaxNode& operator=(const LoopSyntaxNode& other) = delete;
    LoopSyntaxNode& operator=(LoopSyntaxNode& other) = delete;

public:
    /// \copydoc SyntaxNode::isBranch
    virtual bool isBranch() const override { return false; }

    /// \copydoc SyntaxNode::isGoto
    virtual bool isGoto() const override { return false; }

    /// \copydoc SyntaxNode::endsWithGoto
    virtual bool endsWithGoto() const override { return false; }

    /// \copydoc SyntaxNode::getNumOutEdges
    virtual size_t getNumOutEdges() const override { return isInfinite() ? 0 : 1; }

    /// \copydoc SyntaxNode::getOutEdge
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

    /// \copydoc SyntaxNode::getEnclosingLoop
    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) override;

    /// \copydoc SyntaxNode::clone
    virtual SyntaxNode *clone() override;

    /// \copydoc SyntaxNode::replace
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    /// Update the condition of this loop.
    void setCond(SharedExp e)   { m_cond = e; }

    /// Update the body of this loop.
    void setBody(SyntaxNode *n) { m_body = n; }

    /// \copydoc SyntaxNode::findNodeFor
    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;

    /// \copydoc SyntaxNode::printAST
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;

    /// \copydoc SyntaxNode::evaluate
    virtual int evaluate(SyntaxNode *root) override;

    /// \copydoc SyntaxNode::addSuccessors
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

    /// \returns true if the loop has a condition
    inline bool hasCond() const { return m_cond != nullptr; }

    inline bool isInfinite()   const { return !hasCond(); }
    inline bool isPreTested()  const { return hasCond() && !m_postTested; }
    inline bool isPostTested() const { return hasCond() && m_postTested; }

protected:
    SharedExp m_cond;   ///< Condition of the loop header, or nullptr if this loop is infinite
    SyntaxNode *m_body; ///< Loop body
    bool m_postTested;  ///< Is this loop a pre-tested or a post-tested loop?
};
