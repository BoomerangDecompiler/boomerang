#pragma once

#include "boomerang/codegen/syntax/SyntaxNode.h"
#include "boomerang/db/exp/ExpHelp.h"


class LoopSyntaxNode : public SyntaxNode
{
public:
    LoopSyntaxNode(SyntaxNode* body = nullptr, SharedExp cond = nullptr, bool postTested = false);
    virtual ~LoopSyntaxNode();

public:
    /// \copydoc SyntaxNode::isBranch
    virtual bool isBranch() const override { return false; }

    /// \copydoc SyntaxNode::isGoto
    virtual bool isGoto() const override { return false; }

    /// \copydoc SyntaxNode::endsWithGoto
    virtual bool endsWithGoto() const override { return false; }

    /// \copydoc SyntaxNode::getNumOutEdges
    virtual size_t getNumOutEdges() const override { return 1; }

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

protected:
    SharedExp m_cond;
    SyntaxNode *m_body;
    bool m_postTested = false; ///< Is this loop a pre-tested or a post-tested loop?
};


class InfiniteLoopSyntaxNode : public SyntaxNode
{
public:
    InfiniteLoopSyntaxNode();
    virtual ~InfiniteLoopSyntaxNode();

    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 0; }
    virtual SyntaxNode *getOutEdge(SyntaxNode *, size_t)  override { return nullptr; }
    virtual bool endsWithGoto() const override { return false; }
    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr)  override
    {
        if (this == base) {
            return cur;
        }

        return m_body->getEnclosingLoop(base, this);
    }

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    void setBody(SyntaxNode *n) { m_body = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_body;
};
