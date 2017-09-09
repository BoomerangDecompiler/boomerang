#pragma once

#include "boomerang/codegen/syntax/SyntaxNode.h"
#include "boomerang/db/exp/ExpHelp.h"


class PretestedLoopSyntaxNode : public SyntaxNode
{
public:
    PretestedLoopSyntaxNode();
    virtual ~PretestedLoopSyntaxNode();
    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 1; }
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

    virtual bool endsWithGoto() const override { return false; }
    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) override;

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    void setCond(SharedExp e) { m_cond = e; }
    void setBody(SyntaxNode *n) { m_body = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_body;
    SharedExp m_cond;
};


class PostTestedLoopSyntaxNode : public SyntaxNode
{
public:
    PostTestedLoopSyntaxNode();
    virtual ~PostTestedLoopSyntaxNode();

    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 1; }
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
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

    void setCond(SharedExp e) { m_cond = e; }
    void setBody(SyntaxNode *n) { m_body = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_body;
    SharedExp m_cond;
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
