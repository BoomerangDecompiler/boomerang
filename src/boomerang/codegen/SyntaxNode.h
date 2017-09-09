#pragma once

#include <vector>
#include <cassert>
#include <memory>

#include "boomerang/util/Types.h"
#include "boomerang/type/Type.h"
#include "boomerang/db/Managed.h"

class BasicBlock;


class SyntaxNode
{
public:
    SyntaxNode();
    virtual ~SyntaxNode();

    virtual bool isBlock() const { return false; }
    virtual bool isGoto() const;
    virtual bool isBranch() const;

    virtual void ignoreGoto() {}

    virtual int getNumber() const { return m_nodeID; }

    BasicBlock *getBB() const { return m_bb; }
    void setBB(BasicBlock *bb) { m_bb = bb; }

    virtual size_t getNumOutEdges() const = 0;
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) = 0;
    virtual bool endsWithGoto() const = 0;

    virtual bool startsWith(SyntaxNode *node) const { return this == node; }

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) = 0;

    int getScore();

    void addToScore(int n) { m_score = getScore() + n; }
    void setDepth(int n) { m_depth = n; }
    int getDepth() const { return m_depth; }

    virtual SyntaxNode *clone() = 0;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) = 0;

    SyntaxNode *getCorrespond() const { return m_correspond; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb)          = 0;
    virtual void printAST(SyntaxNode *root, QTextStream& os) = 0;
    virtual int evaluate(SyntaxNode *root) = 0;

    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
    { Q_UNUSED(root); Q_UNUSED(successors); }

protected:
    BasicBlock *m_bb;
    int m_nodeID;
    int m_score;
    SyntaxNode *m_correspond; ///< corresponding node in previous state
    bool m_isGoto;
    int m_depth;
};


class BlockSyntaxNode : public SyntaxNode
{
public:
    BlockSyntaxNode();
    virtual ~BlockSyntaxNode();

    virtual bool isBlock() const override { return m_bb == nullptr; }

    virtual void ignoreGoto() override
    {
        if (m_bb) {
            m_isGoto = false;
        }
        else if (statements.size() > 0) {
            statements[statements.size() - 1]->ignoreGoto();
        }
    }

    size_t getNumStatements() const { return m_bb ? 0 : statements.size(); }

    SyntaxNode *getStatement(size_t n)
    {
        assert(m_bb == nullptr);
        assert(n < statements.size());
        return statements[n];
    }

    void prependStatement(SyntaxNode *n)
    {
        assert(m_bb == nullptr);
        statements.resize(statements.size() + 1);

        for (size_t i = statements.size() - 1; i > 0; i--) {
            statements[i] = statements[i - 1];
        }

        statements[0] = n;
    }

    void addStatement(SyntaxNode *n)
    {
        assert(m_bb == nullptr);
        statements.push_back(n);
    }

    void setStatement(size_t i, SyntaxNode *n)
    {
        assert(m_bb == nullptr);
        statements[i] = n;
    }

    virtual size_t getNumOutEdges() const override;
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t n) override;

    virtual bool endsWithGoto() const override
    {
        if (m_bb) {
            return isGoto();
        }

        bool last = false;

        if (statements.size() > 0) {
            last = statements[statements.size() - 1]->endsWithGoto();
        }

        return last;
    }

    virtual bool startsWith(SyntaxNode *node) const override
    {
        return this == node || (statements.size() > 0 && statements[0]->startsWith(node));
    }

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) override
    {
        if (this == base) {
            return cur;
        }

        for (unsigned i = 0; i < statements.size(); i++) {
            SyntaxNode *n = statements[i]->getEnclosingLoop(base, cur);

            if (n) {
                return n;
            }
        }

        return nullptr;
    }

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

private:
    std::vector<SyntaxNode *> statements;
};


class IfThenSyntaxNode : public SyntaxNode
{
public:
    IfThenSyntaxNode();
    virtual ~IfThenSyntaxNode();

    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 1; }
    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t) override;
    virtual bool endsWithGoto() const override { return false; }

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr)  override
    {
        if (this == base) {
            return cur;
        }

        return m_then->getEnclosingLoop(base, cur);
    }

    void setCond(SharedExp e) { m_cond = e; }
    SharedExp getCond() const { return m_cond; }
    void setThen(SyntaxNode *n) { m_then = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_then;
    SharedExp m_cond;
};


class IfThenElseSyntaxNode : public SyntaxNode
{
public:
    IfThenElseSyntaxNode();
    virtual ~IfThenElseSyntaxNode();

    virtual bool isGoto() const override { return false; }
    virtual bool isBranch() const override { return false; }

    virtual size_t getNumOutEdges() const override { return 1; }

    virtual SyntaxNode *getOutEdge(SyntaxNode *root, size_t)  override
    {
        SyntaxNode *o = m_then->getOutEdge(root, 0);

        assert(o == m_else->getOutEdge(root, 0));
        return o;
    }

    virtual bool endsWithGoto() const override { return false; }

    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr)  override
    {
        if (this == base) {
            return cur;
        }

        SyntaxNode *n = m_then->getEnclosingLoop(base, cur);
        return n ? n : m_else->getEnclosingLoop(base, cur);
    }

    virtual SyntaxNode *clone() override;
    virtual SyntaxNode *replace(SyntaxNode *from, SyntaxNode *to) override;

    void setCond(SharedExp e) { m_cond = e; }
    void setThen(SyntaxNode *n) { m_then = n; }
    void setElse(SyntaxNode *n) { m_else = n; }

    virtual SyntaxNode *findNodeFor(BasicBlock *bb) override;
    virtual void printAST(SyntaxNode *root, QTextStream& os) override;
    virtual int evaluate(SyntaxNode *root) override;
    virtual void addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors) override;

protected:
    SyntaxNode *m_then;
    SyntaxNode *m_else;
    SharedExp m_cond;
};


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
    virtual SyntaxNode *getEnclosingLoop(SyntaxNode *base, SyntaxNode *cur = nullptr) override
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
