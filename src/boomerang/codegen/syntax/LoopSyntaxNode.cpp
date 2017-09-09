#include "LoopSyntaxNode.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Exp.h"


LoopSyntaxNode::LoopSyntaxNode(SyntaxNode *body, SharedExp cond, bool postTested)
    : m_cond(cond)
    , m_body(body)
    , m_postTested(postTested)
{
}


LoopSyntaxNode::~LoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


SyntaxNode *LoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    return root->findNodeFor(m_bb->getOutEdge(1));
}


int LoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void LoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // we can always ignore gotos at the end of the body.
    if ((m_body->getNumOutEdges() == 1) && m_body->endsWithGoto()) {
        LOG_VERBOSE("successor: ignoring goto at end of body of pretested loop");
        assert(m_body->getOutEdge(root, 0)->startsWith(this));

        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nBody = m_body->clone();
        nBody->ignoreGoto();
        n = n->replace(m_body, nBody);
        successors.push_back(n);
    }

    m_body->addSuccessors(root, successors);
}


SyntaxNode *LoopSyntaxNode::clone()
{
    LoopSyntaxNode *b = new LoopSyntaxNode(m_body->clone(), m_cond->clone(), m_postTested);
    b->m_correspond = this;
    b->m_bb         = m_bb;

    return b;
}


SyntaxNode *LoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_body->getCorrespond() == from) {
        assert(to);
        m_body = to;
    }
    else {
        m_body = m_body->replace(from, to);
    }

    return this;
}


SyntaxNode *LoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    SyntaxNode *n = m_body->findNodeFor(bb);
    if (m_postTested && n == m_body) {
        return this;
    }
    return n;
}


void LoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop " << (m_postTested ? "post" : "pre") << "-tested ";
    os << m_cond << " \"];" << '\n';
    m_body->printAST(root, os);
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_body->getNumber() << ";" << '\n';
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << getOutEdge(root, 0)->getNumber() << " [style=dotted];" << '\n';
}


InfiniteLoopSyntaxNode::InfiniteLoopSyntaxNode()
    : m_body(nullptr)
{
}


InfiniteLoopSyntaxNode::~InfiniteLoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


int InfiniteLoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void InfiniteLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // we can always ignore gotos at the end of the body.
    if ((m_body->getNumOutEdges() == 1) && m_body->endsWithGoto()) {
        LOG_VERBOSE("Successor: ignoring goto at end of body of infinite loop");
        assert(m_body->getOutEdge(root, 0) == this);

        SyntaxNode *n = root->clone();
        n->setDepth(root->getDepth() + 1);
        SyntaxNode *nBody = m_body->clone();
        nBody->ignoreGoto();
        n = n->replace(m_body, nBody);
        successors.push_back(n);
    }

    m_body->addSuccessors(root, successors);
}


SyntaxNode *InfiniteLoopSyntaxNode::clone()
{
    InfiniteLoopSyntaxNode *b = new InfiniteLoopSyntaxNode();

    b->m_correspond = this;
    b->m_bb        = m_bb;
    b->m_body        = m_body->clone();
    return b;
}


SyntaxNode *InfiniteLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
{
    assert(m_correspond != from);

    if (m_body->getCorrespond() == from) {
        assert(to);
        m_body = to;
    }
    else {
        m_body = m_body->replace(from, to);
    }

    return this;
}


SyntaxNode *InfiniteLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    SyntaxNode *n = m_body->findNodeFor(bb);

    if (n == m_body) {
        return this;
    }

    return n;
}


void InfiniteLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop infinite\"];" << '\n';

    if (m_body) {
        m_body->printAST(root, os);
    }

    if (m_body) {
        os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
        os << " -> " << m_body->getNumber() << ";" << '\n';
    }
}

SyntaxNode* LoopSyntaxNode::getEnclosingLoop(SyntaxNode* base, SyntaxNode* cur)
{
    if (this == base) {
        return cur;
    }

    return m_body->getEnclosingLoop(base, this);
}

