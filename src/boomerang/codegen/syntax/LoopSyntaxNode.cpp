#include "LoopSyntaxNode.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/util/Log.h"
#include "boomerang/db/exp/Exp.h"


PretestedLoopSyntaxNode::PretestedLoopSyntaxNode()
    : m_body(nullptr)
    , m_cond(nullptr)
{
}


PretestedLoopSyntaxNode::~PretestedLoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


SyntaxNode *PretestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    return root->findNodeFor(m_bb->getOutEdge(1));
}


int PretestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void PretestedLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
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


SyntaxNode *PretestedLoopSyntaxNode::clone()
{
    PretestedLoopSyntaxNode *b = new PretestedLoopSyntaxNode();

    b->m_correspond = this;
    b->m_bb        = m_bb;
    b->m_cond         = m_cond->clone();
    b->m_body        = m_body->clone();
    return b;
}


SyntaxNode *PretestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
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


SyntaxNode *PretestedLoopSyntaxNode::findNodeFor(BasicBlock *bb)
{
    if (m_bb == bb) {
        return this;
    }

    return m_body->findNodeFor(bb);
}


void PretestedLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop pretested ";
    os << m_cond << " \"];" << '\n';
    m_body->printAST(root, os);
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << m_body->getNumber() << ";" << '\n';
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << " -> " << getOutEdge(root, 0)->getNumber() << " [style=dotted];" << '\n';
}


PostTestedLoopSyntaxNode::PostTestedLoopSyntaxNode()
    : m_body(nullptr)
    , m_cond(nullptr)
{
}


PostTestedLoopSyntaxNode::~PostTestedLoopSyntaxNode()
{
    if (m_body) {
        delete m_body;
    }
}


SyntaxNode *PostTestedLoopSyntaxNode::getOutEdge(SyntaxNode *root, size_t)
{
    return root->findNodeFor(m_bb->getOutEdge(1));
}


int PostTestedLoopSyntaxNode::evaluate(SyntaxNode *root)
{
    int n = 1;

    n += m_body->evaluate(root);
    return n;
}


void PostTestedLoopSyntaxNode::addSuccessors(SyntaxNode *root, std::vector<SyntaxNode *>& successors)
{
    // we can always ignore gotos at the end of the body.
    if ((m_body->getNumOutEdges() == 1) && m_body->endsWithGoto()) {
        LOG_VERBOSE("successor: ignoring goto at end of body of posttested loop");
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


SyntaxNode *PostTestedLoopSyntaxNode::clone()
{
    PostTestedLoopSyntaxNode *b = new PostTestedLoopSyntaxNode();

    b->m_correspond = this;
    b->m_bb        = m_bb;
    b->m_cond         = m_cond->clone();
    b->m_body        = m_body->clone();
    return b;
}


SyntaxNode *PostTestedLoopSyntaxNode::replace(SyntaxNode *from, SyntaxNode *to)
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


SyntaxNode *PostTestedLoopSyntaxNode::findNodeFor(BasicBlock *bb)
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


void PostTestedLoopSyntaxNode::printAST(SyntaxNode *root, QTextStream& os)
{
    os << qSetFieldWidth(4) << m_nodeID << qSetFieldWidth(0) << " ";
    os << "[label=\"loop posttested ";
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

SyntaxNode* PretestedLoopSyntaxNode::getEnclosingLoop(SyntaxNode* base, SyntaxNode* cur)
{
    if (this == base) {
        return cur;
    }

    return m_body->getEnclosingLoop(base, this);
}

