#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SyntaxNode.h"


#include "boomerang/util/Log.h"
#include "boomerang/core/Boomerang.h"

#include "boomerang/db/Prog.h"
#include "boomerang/db/CFG.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/exp/Unary.h"

#include <iomanip> // For setfill etc
#include <cstring>
#include <cstdlib>

static int nodecount = 1000;

void PRINT_BEFORE_AFTER(SyntaxNode *root, SyntaxNode *n)
{
    QFile tgt("before.dot");
    if (!tgt.open(QFile::WriteOnly)) {
        return;
    }

    QTextStream of(&tgt);
    of << "digraph before {" << '\n';
    root->printAST(root, of);
    of << "}" << '\n';
    QFile tgt2("after.dot");

    if (!tgt2.open(QFile::WriteOnly)) {
        return;
    }

    QTextStream of1(&tgt2);
    of1 << "digraph after {" << '\n';
    n->printAST(n, of1);
    of1 << "}" << '\n';
}


SyntaxNode::SyntaxNode()
    : m_bb(nullptr)
    , m_score(-1)
    , m_correspond(nullptr)
    , m_isGoto(true)
{
    m_nodeID = nodecount++;
}


SyntaxNode::~SyntaxNode()
{
}


int SyntaxNode::getScore()
{
    if (m_score == -1) {
        m_score = evaluate(this);
    }

    return m_score;
}


bool SyntaxNode::isGoto() const
{
    return m_bb && m_bb->getType() == BBType::Oneway && m_isGoto;
}


bool SyntaxNode::isBranch() const
{
    return m_bb && m_bb->getType() == BBType::Twoway;
}
