/*
 * Copyright (C) 2004, Trent Waddington
 */
/***************************************************************************/ /**
  * \file    xmlprogparser.cpp
  * \brief   Implementation of the XMLProgParser and related classes.
  ******************************************************************************/
#include "xmlprogparser.h"
#include "type.h"
#include "module.h"
#include "prog.h"
#include "proc.h"
#include "rtl.h"
#include "statement.h"
#include "sigenum.h"
#include "signature.h"
#include "boomerang.h"
#include "log.h"
#include "basicblock.h"
#include "frontend.h"

#include <QtCore/QFile>
#include <QtCore/QDebug>
#include <QtCore/QXmlStreamReader>
#include <cstdio>
#include <cstring>

enum xmlElement {
    e_prog,
    e_procs,
    e_global,
    e_cluster,
    e_libproc,
    e_userproc,
    e_local,
    e_symbol,
    e_secondexp,
    e_proven_true,
    e_callee,
    e_caller,
    e_defines,
    e_signature,
    e_param,
    e_return,
    e_rettype,
    e_prefreturn,
    e_prefparam,
    e_cfg,
    e_bb,
    e_inedge,
    e_outedge,
    e_livein,
    e_order,
    e_revorder,
    e_rtl,
    e_stmt,
    e_assign,
    e_assignment,
    e_phiassign,
    e_lhs,
    e_rhs,
    e_callstmt,
    e_dest,
    e_argument,
    e_returnexp,
    e_returntype,
    e_returnstmt,
    e_returns,
    e_modifieds,
    e_gotostmt,
    e_branchstmt,
    e_cond,
    e_casestmt,
    e_boolasgn,
    e_type,
    e_exp,
    e_voidtype,
    e_integertype,
    e_pointertype,
    e_chartype,
    e_namedtype,
    e_arraytype,
    e_basetype,
    e_sizetype,
    e_location,
    e_unary,
    e_binary,
    e_ternary,
    e_const,
    e_terminal,
    e_typedexp,
    e_refexp,
    e_def,
    e_subexp1,
    e_subexp2,
    e_subexp3,
    e_unknown = -1
};

#define TAG(x) &XMLProgParser::start_##x, &XMLProgParser::addToContext_##x

_tag XMLProgParser::tags[] = {{"prog", TAG(prog)},
                              {"procs", TAG(procs)},
                              {"global", TAG(global)},
                              {"cluster", TAG(cluster)},
                              {"libproc", TAG(libproc)},
                              {"userproc", TAG(userproc)},
                              {"local", TAG(local)},
                              {"symbol", TAG(symbol)},
                              {"secondexp", TAG(secondexp)},
                              {"proven_true", TAG(proven_true)},
                              {"callee", TAG(callee)},
                              {"caller", TAG(caller)},
                              {"defines", TAG(defines)},
                              {"signature", TAG(signature)},
                              {"param", TAG(param)},
                              {"return", TAG(return )},
                              {"rettype", TAG(rettype)},
                              {"prefreturn", TAG(prefreturn)},
                              {"prefparam", TAG(prefparam)},
                              {"cfg", TAG(cfg)},
                              {"bb", TAG(bb)},
                              {"inedge", TAG(inedge)},
                              {"outedge", TAG(outedge)},
                              {"livein", TAG(livein)},
                              {"order", TAG(order)},
                              {"revorder", TAG(revorder)},
                              {"rtl", TAG(rtl)},
                              {"stmt", TAG(stmt)},
                              {"assign", TAG(assign)},
                              {"assignment", TAG(assignment)},
                              {"phiassign", TAG(phiassign)},
                              {"lhs", TAG(lhs)},
                              {"rhs", TAG(rhs)},
                              {"callstmt", TAG(callstmt)},
                              {"dest", TAG(dest)},
                              {"argument", TAG(argument)},
                              {"returnexp", TAG(returnexp)},
                              {"returntype", TAG(returntype)},
                              {"returnstmt", TAG(returnstmt)},
                              {"returns", TAG(returns)},
                              {"modifieds", TAG(modifieds)},
                              {"gotostmt", TAG(gotostmt)},
                              {"branchstmt", TAG(branchstmt)},
                              {"cond", TAG(cond)},
                              {"casestmt", TAG(casestmt)},
                              {"boolasgn", TAG(boolasgn)},
                              {"type", TAG(type)},
                              {"exp", TAG(exp)},
                              {"voidtype", TAG(voidtype)},
                              {"integertype", TAG(integertype)},
                              {"pointertype", TAG(pointertype)},
                              {"chartype", TAG(chartype)},
                              {"namedtype", TAG(namedtype)},
                              {"arraytype", TAG(arraytype)},
                              {"basetype", TAG(basetype)},
                              {"sizetype", TAG(sizetype)},
                              {"location", TAG(location)},
                              {"unary", TAG(unary)},
                              {"binary", TAG(binary)},
                              {"ternary", TAG(ternary)},
                              {"const", TAG(const)},
                              {"terminal", TAG(terminal)},
                              {"typedexp", TAG(typedexp)},
                              {"refexp", TAG(refexp)},
                              {"def", TAG(def)},
                              {"subexp1", TAG(subexp1)},
                              {"subexp2", TAG(subexp2)},
                              {"subexp3", TAG(subexp3)},
                              {nullptr, nullptr, nullptr}};

class Context {
  public:
    int tag;
    int n;
    QString str;
    Prog *prog;
    Global *global;
    Module *cluster;
    Function *proc;
    Signature *signature;
    Cfg *cfg;
    BasicBlock *bb;
    RTL *rtl;
    Instruction *stmt;
    Parameter *param;
    // ImplicitParameter *implicitParam;
    Return *ret;
    SharedType type;
    Exp *exp, *symbol;
    std::list<Function *> procs;

    Context(int tag)
        : tag(tag), prog(nullptr), proc(nullptr), signature(nullptr), cfg(nullptr), bb(nullptr), rtl(nullptr),
          stmt(nullptr), param(nullptr), /*implicitParam(nullptr),*/ ret(nullptr), exp(nullptr) {}
};

void XMLProgParser::handleElementStart(QXmlStreamReader &strm) {
    QStringRef name = strm.name();
    QXmlStreamAttributes attrs = strm.attributes();
    for (int i = 0; tags[i].tag; i++)
        if (name == tags[i].tag) {
            // LOG_STREAM() << "got tag: " << tags[i].tag << "\n";
            stack.push_front(new Context(i));
            (this->*tags[i].start_proc)(attrs);
            return;
        }
    qWarning() << "got unknown tag: " << name << "\n";
    stack.push_front(new Context(e_unknown));
}

void XMLProgParser::handleElementEnd(const QXmlStreamReader & /*el*/) {
    // LOG_STREAM() << "end tag: " << el << " tos: " << stack.front()->tag << "\n";
    std::list<Context *>::iterator it = stack.begin();
    if (it != stack.end()) {
        it++;
        if (it != stack.end()) {
            if ((*it)->tag != e_unknown) {
                // LOG_STREAM() << " second: " << (*it)->tag << "\n";
                (this->*tags[(*it)->tag].end_proc)(*it, stack.front()->tag);
            }
            stack.erase(stack.begin());
        }
    }
}

void XMLProgParser::addId(const QXmlStreamAttributes &attr, void *x) {
    QStringRef val = attr.value(QLatin1Literal("id"));
    if (!val.isEmpty()) {
        // LOG_STREAM() << "map id " << val << " to " << std::hex << (int)x << std::dec << "\n";
        idToX[val.toInt()] = x;
    }
}
void XMLProgParser::addType(const QXmlStreamAttributes &attr, SharedType x) {
    QStringRef val = attr.value(QLatin1Literal("id"));
    if (!val.isEmpty()) {
        // LOG_STREAM() << "map id " << val << " to " << std::hex << (int)x << std::dec << "\n";
        idToType[val.toInt()] = x;
    }
}

void *XMLProgParser::findId(const QStringRef &id) {
    if (id.isNull() || id.isEmpty())
        return nullptr;
    int n = id.toInt();
    if (n == 0)
        return nullptr;
    auto it = idToX.find(n);
    if (it == idToX.end()) {
        qCritical() << "findId could not find \"" << id << "\"\n";
        assert(false);
        return nullptr;
    }
    return it->second;
}
SharedType XMLProgParser::findType(const QStringRef &id) {
    if (id.isNull() || id.isEmpty())
        return nullptr;
    int n = id.toInt();
    if (n == 0)
        return nullptr;
    auto it = idToType.find(n);
    if (it == idToType.end()) {
        qCritical() << "findId could not find type for \"" << id << "\"\n";
        assert(false);
        return nullptr;
    }
    return it->second;
}
void XMLProgParser::start_prog(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        return;
    }
    QStringRef name = attr.value(QLatin1Literal("name"));
    Context *ctx = stack.front();
    ctx->prog = new Prog(name.toString());
    addId(attr, ctx->prog);
    if (!name.isEmpty())
        ctx->prog->setName(qPrintable(name.toString()));
    name = attr.value(QLatin1Literal("path"));
    if (!name.isEmpty())
        ctx->prog->m_path = name.toString();
    QStringRef iNumberedProc = attr.value(QLatin1Literal("iNumberedProc"));
    ctx->prog->m_iNumberedProc = iNumberedProc.toInt();
}

void XMLProgParser::addToContext_prog(Context *c, int e) {
    Context *ctx = stack.front();
    if (phase == 1) {
        switch (e) {
        case e_libproc:
        case e_userproc:
            Boomerang::get()->alertLoad(ctx->proc);
            break;
        }
        return;
    }
    switch (e) {
    case e_libproc:
        ctx->proc->setProg(c->prog);
        ctx->proc->setParent(ctx->cluster);
        break;
    case e_userproc:
        ctx->proc->setProg(c->prog);
        ctx->cluster->getFunctionList().push_back(ctx->proc);
        ctx->cluster->setLocationMap(ctx->proc->getNativeAddress(),stack.front()->proc);
        break;
    case e_procs: {
        Module * current_m = ctx->cluster;
        auto func_list(current_m->getFunctionList());
        for (auto &elem : ctx->procs) {
            func_list.push_back(elem);
            current_m->setLocationMap(elem->getNativeAddress(),elem);
            Boomerang::get()->alertLoad(elem);
        }
    }
        break;
    case e_cluster:
        c->prog->m_rootCluster = ctx->cluster;
        break;
    case e_global:
        c->prog->globals.insert(ctx->global);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context prog\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context prog\n";
        break;
    }
}

void XMLProgParser::start_procs(const QXmlStreamAttributes & /*attr*/) {
    if (phase == 1) {
        return;
    }
    stack.front()->procs.clear();
}

void XMLProgParser::addToContext_procs(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_libproc:
        c->procs.push_back(stack.front()->proc);
        break;
    case e_userproc:
        c->procs.push_back(stack.front()->proc);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context procs\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context procs\n";
        break;
    }
}

void XMLProgParser::start_global(const QXmlStreamAttributes &attr) {
    Context *ctx = stack.front();
    if (phase == 1) {
        return;
    }
    ctx->global = new Global();
    addId(attr, ctx->global);
    QStringRef name = attr.value(QLatin1Literal("name"));
    if (!name.isEmpty())
        ctx->global->nam = name.toString();
    QStringRef uaddr = attr.value(QLatin1Literal("uaddr"));
    if (!uaddr.isEmpty())
        ctx->global->uaddr = ADDRESS::g(uaddr.toInt(nullptr, 16));
}

void XMLProgParser::addToContext_global(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_type:
        c->global->type = stack.front()->type;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context global\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context global\n";
        break;
    }
}

void XMLProgParser::start_cluster(const QXmlStreamAttributes &attr) {
    Context *ctx = stack.front();
    if (phase == 1) {
        ctx->cluster = (Module *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    ctx->cluster = new Module();
    addId(attr, ctx->cluster);
    QStringRef name = attr.value(QLatin1Literal("name"));
    if (!name.isEmpty())
        ctx->cluster->setName(name.toString());
}

void XMLProgParser::addToContext_cluster(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_cluster:
        c->cluster->addChild(stack.front()->cluster);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context cluster\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context cluster\n";
        break;
    }
}

void XMLProgParser::start_libproc(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->proc = (Function *)findId(attr.value(QLatin1Literal("id")));
        Function *p = (Function *)findId(attr.value(QLatin1Literal("firstCaller")));
        if (p)
            stack.front()->proc->m_firstCaller = p;
        Module *c = (Module *)findId(attr.value(QLatin1Literal("cluster")));
        if (c)
            stack.front()->proc->Parent = c;
        return;
    }
    stack.front()->proc = new LibProc();
    addId(attr, stack.front()->proc);
    QStringRef address = attr.value(QLatin1Literal("address"));
    if (!address.isEmpty())
        stack.front()->proc->address = ADDRESS::g(address.toInt());
    address = attr.value(QLatin1Literal("firstCallerAddress"));
    if (!address.isEmpty())
        stack.front()->proc->m_firstCallerAddr = ADDRESS::g(address.toInt());
}

void XMLProgParser::addToContext_libproc(Context *c, int e) {
    if (phase == 1) {
        switch (e) {
        case e_caller:
            CallStatement *call = dynamic_cast<CallStatement *>(stack.front()->stmt);
            assert(call);
            c->proc->addCaller(call);
            break;
        }
        return;
    }
    switch (e) {
    case e_signature:
        c->proc->setSignature(stack.front()->signature);
        break;
    case e_proven_true:
        c->proc->setProvenTrue(stack.front()->exp);
        break;
    case e_caller:
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context libproc\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context libproc\n";
        break;
    }
}

void XMLProgParser::start_userproc(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->proc = (Function *)findId(attr.value(QLatin1Literal("id")));
        UserProc *u = dynamic_cast<UserProc *>(stack.front()->proc);
        assert(u);
        Function *p = (Function *)findId(attr.value(QLatin1Literal("firstCaller")));
        if (p)
            u->m_firstCaller = p;
        Module *c = (Module *)findId(attr.value(QLatin1Literal("cluster")));
        if (c)
            u->Parent = c;
        ReturnStatement *r = (ReturnStatement *)findId(attr.value(QLatin1Literal("retstmt")));
        if (r)
            u->theReturnStatement = r;
        return;
    }
    UserProc *proc = new UserProc();
    stack.front()->proc = proc;
    addId(attr, proc);

    QStringRef address = attr.value(QLatin1Literal("address"));
    if (!address.isEmpty())
        proc->address = ADDRESS::g(address.toInt());
    address = attr.value(QLatin1Literal("status"));
    if (!address.isEmpty())
        proc->status = (ProcStatus)address.toInt();
    address = attr.value(QLatin1Literal("firstCallerAddress"));
    if (!address.isEmpty())
        proc->m_firstCallerAddr = ADDRESS::g(address.toInt());
}

void XMLProgParser::addToContext_userproc(Context *c, int e) {
    UserProc *userproc = dynamic_cast<UserProc *>(c->proc);
    assert(userproc);
    if (phase == 1) {
        switch (e) {
        case e_caller: {
            CallStatement *call = dynamic_cast<CallStatement *>(stack.front()->stmt);
            assert(call);
            c->proc->addCaller(call);
            break;
        }
        case e_callee:
            userproc->addCallee(stack.front()->proc);
            break;
        }
        return;
    }
    switch (e) {
    case e_signature:
        c->proc->setSignature(stack.front()->signature);
        break;
    case e_proven_true:
        c->proc->setProvenTrue(stack.front()->exp);
        break;
    case e_caller:
        break;
    case e_callee:
        break;
    case e_cfg:
        userproc->setCFG(stack.front()->cfg);
        break;
    case e_local:
        userproc->locals[stack.front()->str] = stack.front()->type;
        break;
    case e_symbol:
        userproc->mapSymbolTo(stack.front()->exp, stack.front()->symbol);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context userproc\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context userproc\n";
        break;
    }
}

void XMLProgParser::start_local(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        return;
    }
    stack.front()->str = attr.value(QLatin1Literal("name")).toString();
}

void XMLProgParser::addToContext_local(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_type:
        c->type = stack.front()->type;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context local\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context local\n";
        break;
    }
}

void XMLProgParser::start_symbol(const QXmlStreamAttributes & /*attr*/) {
    if (phase == 1) {
        return;
    }
}

void XMLProgParser::addToContext_symbol(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_exp:
        c->exp = stack.front()->exp;
        break;
    case e_secondexp:
        c->symbol = stack.front()->exp;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context local\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context local\n";
        break;
    }
}

void XMLProgParser::start_proven_true(const QXmlStreamAttributes & /*attr*/) {}

void XMLProgParser::addToContext_proven_true(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_callee(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->proc = (Function *)findId(attr.value(QLatin1Literal("proc")));
    }
}

void XMLProgParser::addToContext_callee(Context * /*c*/, int /*e*/) {}

void XMLProgParser::start_caller(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("call")));
    }
}

void XMLProgParser::addToContext_caller(Context * /*c*/, int /*e*/) {}

void XMLProgParser::start_signature(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->signature = (Signature *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    QStringRef plat = attr.value(QLatin1Literal("platform"));
    QStringRef convention = attr.value(QLatin1Literal("convention"));
    QStringRef name = attr.value(QLatin1Literal("name"));

    Signature *sig;
    // TODO: use platforms loaded from plugins
    if (!plat.isEmpty() && !convention.isEmpty()) {
        platform p;
        callconv c;
        if (plat == "pentium")
            p = PLAT_PENTIUM;
        else if (plat == "sparc")
            p = PLAT_SPARC;
        else if (plat == "ppc")
            p = PLAT_PPC;
        else if (plat == "st20")
            p = PLAT_ST20;
        else {
            qCritical() << "unknown platform: " << plat << "\n";
            assert(false);
            p = PLAT_PENTIUM;
        }
        if (convention == "stdc")
            c = CONV_C;
        else if (convention == "pascal")
            c = CONV_PASCAL;
        else if (convention == "thiscall")
            c = CONV_THISCALL;
        else {
            qCritical() << "unknown convention: " << convention << "\n";
            assert(false);
            c = CONV_C;
        }
        sig = Signature::instantiate(p, c, qPrintable(name.toString()));
    } else
        sig = new Signature(qPrintable(name.toString()));
    sig->params.clear();
    // sig->implicitParams.clear();
    sig->returns.clear();
    stack.front()->signature = sig;
    addId(attr, sig);
    QStringRef n = attr.value(QLatin1Literal("ellipsis"));
    if (!n.isEmpty())
        sig->ellipsis = n.toInt() > 0;
    n = attr.value(QLatin1Literal("preferedName"));
    if (!n.isEmpty())
        sig->preferedName = n.toString();
}

void XMLProgParser::addToContext_signature(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_param:
        c->signature->appendParameter(stack.front()->param);
        break;
    case e_return:
        c->signature->appendReturn(stack.front()->ret);
        break;
    case e_rettype:
        c->signature->setRetType(stack.front()->type);
        break;
    case e_prefparam:
        c->signature->preferedParams.push_back(stack.front()->n);
        break;
    case e_prefreturn:
        c->signature->preferedReturn = stack.front()->type;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context signature\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context signature\n";
        break;
    }
}

void XMLProgParser::start_param(const QXmlStreamAttributes &attr) {
    Context *ctx = stack.front();
    if (phase == 1) {
        ctx->param = (Parameter *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    ctx->param = new Parameter;
    addId(attr, ctx->param);
    QStringRef n = attr.value(QLatin1Literal("name"));
    if (!n.isEmpty())
        ctx->param->name(n.toString());
}

void XMLProgParser::addToContext_param(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_type:
        c->param->setType(stack.front()->type);
        break;
    case e_exp:
        c->param->setExp(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context param\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context param\n";
        break;
    }
}

void XMLProgParser::start_prefreturn(const QXmlStreamAttributes &) {
    if (phase == 1) {
        return;
    }
}

void XMLProgParser::addToContext_prefreturn(Context *c, int /*e*/) {
    if (phase == 1) {
        return;
    }
    c->type = stack.front()->type;
}

void XMLProgParser::start_prefparam(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        return;
    }
    QStringRef n = attr.value(QLatin1Literal("index"));
    assert(!n.isEmpty());
    stack.front()->n = n.toInt();
}

void XMLProgParser::addToContext_prefparam(Context * /*c*/, int e) {
    if (phase == 1) {
        return;
    }
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context prefparam\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context prefparam\n";
    //    break;
    //      }
}

void XMLProgParser::start_return(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->ret = (Return *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->ret = new Return();
    addId(attr, stack.front()->ret);
}

void XMLProgParser::addToContext_return(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_type:
        c->ret->type = stack.front()->type;
        break;
    case e_exp:
        c->ret->exp = stack.front()->exp;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context return\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context return\n";
        break;
    }
}

void XMLProgParser::start_rettype(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_rettype(Context *c, int /*e*/) { c->type = stack.front()->type; }

void XMLProgParser::start_cfg(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->cfg = (Cfg *)findId(attr.value(QLatin1Literal("id")));
        BasicBlock *entryBB = (BasicBlock *)findId(attr.value(QLatin1Literal("entryBB")));
        if (entryBB)
            stack.front()->cfg->setEntryBB(entryBB);
        BasicBlock *exitBB = (BasicBlock *)findId(attr.value(QLatin1Literal("exitBB")));
        if (exitBB)
            stack.front()->cfg->setExitBB(exitBB);
        return;
    }
    Cfg *cfg = new Cfg();
    stack.front()->cfg = cfg;
    addId(attr, cfg);

    QStringRef str = attr.value(QLatin1Literal("wellformed"));
    if (!str.isEmpty())
        cfg->WellFormed = str.toInt() > 0;
    str = attr.value(QLatin1Literal("lastLabel"));
    if (!str.isEmpty())
        cfg->lastLabel = str.toInt();
}

void XMLProgParser::addToContext_cfg(Context *c, int e) {
    if (phase == 1) {
        switch (e) {
        case e_order:
            c->cfg->Ordering.push_back(stack.front()->bb);
            break;
        case e_revorder:
            c->cfg->revOrdering.push_back(stack.front()->bb);
            break;
        }
        return;
    }
    switch (e) {
    case e_bb:
        c->cfg->addBB(stack.front()->bb);
        break;
    case e_order:
        break;
    case e_revorder:
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context cfg\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context cfg\n";
        break;
    }
}

void XMLProgParser::start_bb(const QXmlStreamAttributes &attr) {
    BasicBlock *bb;
    if (phase == 1) {
        bb = stack.front()->bb = (BasicBlock *)findId(attr.value(QLatin1Literal("id")));
        // note the rediculous duplication here
        BasicBlock *h = (BasicBlock *)findId(attr.value(QLatin1Literal("immPDom")));
        if (h)
            bb->ImmPDom = h;
        h = (BasicBlock *)findId(attr.value(QLatin1Literal("loopHead")));
        if (h)
            bb->LoopHead = h;
        h = (BasicBlock *)findId(attr.value(QLatin1Literal("caseHead")));
        if (h)
            bb->CaseHead = h;
        h = (BasicBlock *)findId(attr.value(QLatin1Literal("condFollow")));
        if (h)
            bb->CondFollow = h;
        h = (BasicBlock *)findId(attr.value(QLatin1Literal("loopFollow")));
        if (h)
            bb->LoopFollow = h;
        h = (BasicBlock *)findId(attr.value(QLatin1Literal("latchNode")));
        if (h)
            bb->LatchNode = h;
        return;
    }
    bb = new BasicBlock(stack.front()->proc); //TODO: verify that this is a valid proc for this bb
    stack.front()->bb = bb;
    addId(attr, bb);

    QStringRef str = attr.value(QLatin1Literal("nodeType"));
    if (!str.isEmpty())
        bb->NodeType = (BBTYPE)str.toInt();
    str = attr.value(QLatin1Literal("labelNum"));
    if (!str.isEmpty())
        bb->LabelNum = str.toInt();
    str = attr.value(QLatin1Literal("labelneeded"));
    if (!str.isEmpty())
        bb->LabelNeeded = str.toInt() > 0;
    str = attr.value(QLatin1Literal("incomplete"));
    if (!str.isEmpty())
        bb->Incomplete = str.toInt() > 0;
    str = attr.value(QLatin1Literal("jumpreqd"));
    if (!str.isEmpty())
        bb->JumpReqd = str.toInt() > 0;
    str = attr.value(QLatin1Literal("m_traversed"));
    if (!str.isEmpty())
        bb->TraversedMarker = str.toInt() > 0;
    str = attr.value(QLatin1Literal("DFTfirst"));
    if (!str.isEmpty())
        bb->DFTfirst = str.toInt();
    str = attr.value(QLatin1Literal("DFTlast"));
    if (!str.isEmpty())
        bb->DFTlast = str.toInt();
    str = attr.value(QLatin1Literal("DFTrevfirst"));
    if (!str.isEmpty())
        bb->DFTrevfirst = str.toInt();
    str = attr.value(QLatin1Literal("DFTrevlast"));
    if (!str.isEmpty())
        bb->DFTrevlast = str.toInt();
    str = attr.value(QLatin1Literal("structType"));
    if (!str.isEmpty())
        bb->StructType = (SBBTYPE)str.toInt();
    str = attr.value(QLatin1Literal("loopCondType"));
    if (!str.isEmpty())
        bb->LoopCondType = (SBBTYPE)str.toInt();
    str = attr.value(QLatin1Literal("ord"));
    if (!str.isEmpty())
        bb->Ord = str.toInt();
    str = attr.value(QLatin1Literal("revOrd"));
    if (!str.isEmpty())
        bb->RevOrd = str.toInt();
    str = attr.value(QLatin1Literal("inEdgesVisited"));
    if (!str.isEmpty())
        bb->InEdgesVisited = str.toInt();
    str = attr.value(QLatin1Literal("numForwardInEdges"));
    if (!str.isEmpty())
        bb->NumForwardInEdges = str.toInt();
    str = attr.value(QLatin1Literal("loopStamp1"));
    if (!str.isEmpty())
        bb->LoopStamps[0] = str.toInt();
    str = attr.value(QLatin1Literal("loopStamp2"));
    if (!str.isEmpty())
        bb->LoopStamps[1] = str.toInt();
    str = attr.value(QLatin1Literal("revLoopStamp1"));
    if (!str.isEmpty())
        bb->RevLoopStamps[0] = str.toInt();
    str = attr.value(QLatin1Literal("revLoopStamp2"));
    if (!str.isEmpty())
        bb->RevLoopStamps[1] = str.toInt();
    str = attr.value(QLatin1Literal("traversed"));
    if (!str.isEmpty())
        bb->Traversed = (travType)str.toInt();
    str = attr.value(QLatin1Literal("hllLabel"));
    if (!str.isEmpty())
        bb->HllLabel = str.toInt() > 0;
    str = attr.value(QLatin1Literal("labelStr"));
    if (!str.isEmpty())
        bb->LabelStr = str.toString();
    str = attr.value(QLatin1Literal("indentLevel"));
    if (!str.isEmpty())
        bb->IndentLevel = str.toInt();
    str = attr.value(QLatin1Literal("sType"));
    if (!str.isEmpty())
        bb->StructuringType = (structType)str.toInt();
    str = attr.value(QLatin1Literal("usType"));
    if (!str.isEmpty())
        bb->UnstructuredType = (unstructType)str.toInt();
    str = attr.value(QLatin1Literal("lType"));
    if (!str.isEmpty())
        bb->LoopHeaderType = (LoopType)str.toInt();
    str = attr.value(QLatin1Literal("cType"));
    if (!str.isEmpty())
        bb->ConditionHeaderType = (CondType)str.toInt();
}

void XMLProgParser::addToContext_bb(Context *c, int e) {
    if (phase == 1) {
        switch (e) {
        case e_inedge:
            c->bb->addInEdge(stack.front()->bb);
            break;
        case e_outedge:
            c->bb->addOutEdge(stack.front()->bb);
            break;
        }
        return;
    }
    switch (e) {
    case e_inedge:
        break;
    case e_outedge:
        break;
    case e_livein:
        c->bb->addLiveIn((Location *)stack.front()->exp);
        break;
    case e_rtl:
        c->bb->addRTL(stack.front()->rtl);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context bb\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context bb\n";
        break;
    }
}

void XMLProgParser::start_inedge(const QXmlStreamAttributes &attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock *)findId(attr.value(QLatin1Literal("bb")));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_inedge(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context inedge\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context inedge\n";
    //    break;
    //      }
}

void XMLProgParser::start_outedge(const QXmlStreamAttributes &attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock *)findId(attr.value(QLatin1Literal("bb")));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_outedge(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context outedge\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context outedge\n";
    //    break;
    //      }
}

void XMLProgParser::start_livein(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_livein(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_order(const QXmlStreamAttributes &attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock *)findId(attr.value(QLatin1Literal("bb")));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_order(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context order\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context order\n";
    //    break;
    //      }
}

void XMLProgParser::start_revorder(const QXmlStreamAttributes &attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock *)findId(attr.value(QLatin1Literal("bb")));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_revorder(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context revOrder\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context order\n";
    //    break;
    //      }
}

void XMLProgParser::start_rtl(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->rtl = (RTL *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->rtl = new RTL();
    addId(attr, stack.front()->rtl);
    QStringRef a = attr.value(QLatin1Literal("addr"));
    if (!a.isEmpty())
        stack.front()->rtl->nativeAddr = ADDRESS::g(a.toInt());
}

void XMLProgParser::addToContext_rtl(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_stmt:
        c->rtl->appendStmt(stack.front()->stmt);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context rtl\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context rtl\n";
        break;
    }
}

void XMLProgParser::start_stmt(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_stmt(Context *c, int /*e*/) { c->stmt = stack.front()->stmt; }

void XMLProgParser::start_assign(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        //        Statement *parent = (Statement*)findId(attr.value(QLatin1Literal("parent")));
        //        if (parent)
        //            stack.front()->stmt->parent = parent;
        return;
    }
    stack.front()->stmt = new Assign();
    addId(attr, stack.front()->stmt);
    QStringRef n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        stack.front()->stmt->Number = n.toInt();
}

void XMLProgParser::start_assignment(const QXmlStreamAttributes &) {}

void XMLProgParser::start_phiassign(const QXmlStreamAttributes &) {
    // FIXME: TBC
}

void XMLProgParser::addToContext_assign(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    Assign *assign = dynamic_cast<Assign *>(c->stmt);
    assert(assign);
    switch (e) {
    case e_lhs:
        assign->setLeft(stack.front()->exp);
        break;
    case e_rhs:
        assign->setRight(stack.front()->exp);
        break;
    case e_type:
        assert(stack.front()->type);
        assign->setType(stack.front()->type);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context assign\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context assign\n";
        break;
    }
}

void XMLProgParser::start_callstmt(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        //        Statement *s = (Statement*)findId(attr.value(QLatin1Literal("parent")));
        //        if (s)
        //            ((Statement*)stack.front()->stmt)->parent = s;
        return;
    }
    CallStatement *call = new CallStatement();
    stack.front()->stmt = call;
    addId(attr, call);
    QStringRef n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        call->Number = n.toInt();
    n = attr.value(QLatin1Literal("computed"));
    if (!n.isEmpty())
        call->m_isComputed = n.toInt() > 0;
    n = attr.value(QLatin1Literal("returnAftercall"));
    if (!n.isEmpty())
        call->returnAfterCall = n.toInt() > 0;
}

void XMLProgParser::addToContext_callstmt(Context *c, int e) {
    CallStatement *call = dynamic_cast<CallStatement *>(c->stmt);
    assert(call);
    if (phase == 1) {
        switch (e) {
        case e_dest:
            if (stack.front()->proc)
                call->setDestProc(stack.front()->proc);
            break;
        }
        return;
    }
    // TODO: analyze the code to understand the intended use of returnExp
    // Exp* returnExp = nullptr;
    switch (e) {
    case e_dest:
        call->setDest(stack.front()->exp);
        break;
    case e_argument:
        call->appendArgument((Assignment *)stack.front()->stmt);
        break;
    case e_returnexp:
        // Assume that the corresponding return type will appear next
        // returnExp = stack.front()->exp;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context callstmt\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context callstmt\n";
        break;
    }
}

void XMLProgParser::start_dest(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        Function *p = (Function *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->proc = p;
        return;
    }
}

void XMLProgParser::addToContext_dest(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_returnstmt(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        //      Statement *s = (Statement*)findId(attr.value(QLatin1Literal("parent")));
        //        if (s)
        //            ((Statement*)stack.front()->stmt)->parent = s;
        return;
    }
    ReturnStatement *ret = new ReturnStatement();
    stack.front()->stmt = ret;
    addId(attr, ret);
    QStringRef n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        ret->Number = n.toInt();
    n = attr.value(QLatin1Literal("retAddr"));
    if (!n.isEmpty())
        ret->retAddr = ADDRESS::g(n.toInt());
}

void XMLProgParser::addToContext_returnstmt(Context *c, int e) {
    ReturnStatement *ret = dynamic_cast<ReturnStatement *>(c->stmt);
    assert(ret);
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_modifieds:
        ret->modifieds.append((Assignment *)stack.front()->stmt);
        break;
    case e_returns:
        ret->returns.append((Assignment *)stack.front()->stmt);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context returnstmt\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context returnstmt\n";
        break;
    }
}

void XMLProgParser::start_returns(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_returns(Context * /*c*/, int /*e*/) {}

void XMLProgParser::start_modifieds(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_modifieds(Context * /*c*/, int /*e*/) {}

void XMLProgParser::start_gotostmt(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        //        Statement *s = (Statement*)findId(attr.value(QLatin1Literal("parent")));
        //        if (s)
        //            ((Statement*)stack.front()->stmt)->parent = s;
        return;
    }
    GotoStatement *branch = new GotoStatement();
    stack.front()->stmt = branch;
    addId(attr, branch);
    QStringRef n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        branch->Number = n.toInt();
    n = attr.value(QLatin1Literal("computed"));
    if (!n.isEmpty())
        branch->m_isComputed = n.toInt() > 0;
}

void XMLProgParser::addToContext_gotostmt(Context *c, int e) {
    GotoStatement *branch = dynamic_cast<GotoStatement *>(c->stmt);
    assert(branch);
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_dest:
        branch->setDest(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context gotostmt\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context gotostmt\n";
        break;
    }
}

void XMLProgParser::start_branchstmt(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        return;
    }
    BranchStatement *branch = new BranchStatement();
    stack.front()->stmt = branch;
    addId(attr, branch);
    QStringRef n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        branch->Number = n.toInt();
    n = attr.value(QLatin1Literal("computed"));
    if (!n.isEmpty())
        branch->m_isComputed = n.toInt() > 0;
    n = attr.value(QLatin1Literal("jtcond"));
    if (!n.isEmpty())
        branch->jtCond = (BRANCH_TYPE)n.toInt();
    n = attr.value(QLatin1Literal("float"));
    if (!n.isEmpty())
        branch->bFloat = n.toInt() > 0;
}

void XMLProgParser::addToContext_branchstmt(Context *c, int e) {
    BranchStatement *branch = dynamic_cast<BranchStatement *>(c->stmt);
    assert(branch);
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_cond:
        branch->setCondExpr(stack.front()->exp);
        break;
    case e_dest:
        branch->setDest(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context branchstmt\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context branchstmt\n";
        break;
    }
}

void XMLProgParser::start_casestmt(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        return;
    }
    CaseStatement *cas = new CaseStatement();
    stack.front()->stmt = cas;
    addId(attr, cas);
    QStringRef n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        cas->Number = n.toInt();
    n = attr.value(QLatin1Literal("computed"));
    if (!n.isEmpty())
        cas->m_isComputed = n.toInt() > 0;
}

void XMLProgParser::addToContext_casestmt(Context *c, int e) {
    CaseStatement *cas = dynamic_cast<CaseStatement *>(c->stmt);
    assert(cas);
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_dest:
        cas->setDest(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context casestmt\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context casestmt\n";
        break;
    }
}

void XMLProgParser::start_boolasgn(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            stack.front()->stmt->setProc(p);
        return;
    }
    QStringRef n = attr.value(QLatin1Literal("size"));
    assert(!n.isEmpty());
    BoolAssign *boo = new BoolAssign(n.toInt());
    stack.front()->stmt = boo;
    addId(attr, boo);
    n = attr.value(QLatin1Literal("number"));
    if (!n.isEmpty())
        boo->Number = n.toInt();
    n = attr.value(QLatin1Literal("jtcond"));
    if (!n.isEmpty())
        boo->jtCond = (BRANCH_TYPE)n.toInt();
    n = attr.value(QLatin1Literal("float"));
    if (!n.isEmpty())
        boo->bFloat = n.toInt() > 0;
}

void XMLProgParser::addToContext_boolasgn(Context *c, int e) {
    BoolAssign *boo = dynamic_cast<BoolAssign *>(c->stmt);
    assert(boo);
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_cond:
        boo->pCond = stack.front()->exp;
        break;
    case e_lhs:
        boo->lhs = stack.front()->exp;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context boolasgn\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context boolasgn\n";
        break;
    }
}

void XMLProgParser::start_type(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_type(Context *c, int /*e*/) { c->type = stack.front()->type; }

void XMLProgParser::start_basetype(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_basetype(Context *c, int /*e*/) { c->type = stack.front()->type; }

void XMLProgParser::start_sizetype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    auto ty = SizeType::get();
    stack.front()->type = ty;
    addType(attr, ty);
    QStringRef n = attr.value(QLatin1Literal("size"));
    if (!n.isEmpty())
        ty->size = n.toInt();
}

void XMLProgParser::addToContext_sizetype(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context SizeType\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context SizeType\n";
    //    break;
    //      }
}

void XMLProgParser::start_exp(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_exp(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_secondexp(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_secondexp(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_defines(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_defines(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_lhs(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_lhs(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_rhs(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_rhs(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_argument(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_argument(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_returnexp(const QXmlStreamAttributes &) {}

void XMLProgParser::start_returntype(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_returnexp(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::addToContext_returntype(Context *c, int /*e*/) { c->type = stack.front()->type; }

void XMLProgParser::start_cond(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_cond(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_subexp1(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_subexp1(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_subexp2(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_subexp2(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_subexp3(const QXmlStreamAttributes &) {}

void XMLProgParser::addToContext_subexp3(Context *c, int /*e*/) { c->exp = stack.front()->exp; }

void XMLProgParser::start_voidtype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->type = VoidType::get();
    addType(attr, stack.front()->type);
}

void XMLProgParser::addToContext_voidtype(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context voidType\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context voidType\n";
    //    break;
    //      }
}

void XMLProgParser::start_integertype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    auto ty = IntegerType::get(STD_SIZE);
    stack.front()->type = ty;
    addType(attr, ty);
    QStringRef n = attr.value(QLatin1Literal("size"));
    if (!n.isEmpty())
        ty->size = n.toInt();
    n = attr.value(QLatin1Literal("signedness"));
    if (!n.isEmpty())
        ty->signedness = n.toInt();
}

void XMLProgParser::addToContext_integertype(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context integerType\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context integerType\n";
    //    break;
    //      }
}

void XMLProgParser::start_pointertype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->type = PointerType::get(nullptr);
    addType(attr, stack.front()->type);
}

void XMLProgParser::addToContext_pointertype(Context *c, int /*e*/) {
    auto p = std::dynamic_pointer_cast<PointerType>(c->type);
    assert(p);
    if (phase == 1) {
        return;
    }
    p->setPointsTo(stack.front()->type);
}

void XMLProgParser::start_chartype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->type = CharType::get();
    addType(attr, stack.front()->type);
}

void XMLProgParser::addToContext_chartype(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context charType\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context charType\n";
    //    break;
    //      }
}

void XMLProgParser::start_namedtype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->type = NamedType::get(attr.value(QLatin1Literal("name")).toString());
    addType(attr, stack.front()->type);
}

void XMLProgParser::addToContext_namedtype(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context namedType\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context namedType\n";
    //    break;
    //      }
}

void XMLProgParser::start_arraytype(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->type = findType(attr.value(QLatin1Literal("id")));
        return;
    }
    auto a = ArrayType::get(VoidType::get());
    stack.front()->type = a;
    addType(attr, a);
    QStringRef len = attr.value(QLatin1Literal("length"));
    if (!len.isEmpty())
        a->Length = len.toInt();
}

void XMLProgParser::addToContext_arraytype(Context *c, int e) {
    auto a = std::dynamic_pointer_cast<ArrayType>(c->type);
    assert(a);
    switch (e) {
    case e_basetype:
        a->BaseType = stack.front()->type;
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context arrayType\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context arrayType\n";
        break;
    }
}

void XMLProgParser::start_location(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        UserProc *p = (UserProc *)findId(attr.value(QLatin1Literal("proc")));
        if (p)
            ((Location *)stack.front()->exp)->setProc(p);
        return;
    }
    OPER op = (OPER)operFromString(attr.value(QLatin1Literal("op")));
    assert(op != -1);
    stack.front()->exp = new Location(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_location(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    Location *l = dynamic_cast<Location *>(c->exp);
    assert(l);
    switch (e) {
    case e_subexp1:
        c->exp->setSubExp1(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context unary\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context location\n";
        break;
    }
}

void XMLProgParser::start_unary(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    OPER op = (OPER)operFromString(attr.value(QLatin1Literal("op")));
    assert(op != -1);
    stack.front()->exp = new Unary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_unary(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_subexp1:
        c->exp->setSubExp1(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context unary\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context unary\n";
        break;
    }
}

void XMLProgParser::start_binary(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    OPER op = (OPER)operFromString(attr.value(QLatin1Literal("op")));
    assert(op != -1);
    stack.front()->exp = new Binary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_binary(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_subexp1:
        c->exp->setSubExp1(stack.front()->exp);
        break;
    case e_subexp2:
        c->exp->setSubExp2(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context binary\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context binary\n";
        break;
    }
}

void XMLProgParser::start_ternary(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    OPER op = (OPER)operFromString(attr.value(QLatin1Literal("op")));
    assert(op != -1);
    stack.front()->exp = new Ternary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_ternary(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch (e) {
    case e_subexp1:
        c->exp->setSubExp1(stack.front()->exp);
        break;
    case e_subexp2:
        c->exp->setSubExp2(stack.front()->exp);
        break;
    case e_subexp3:
        c->exp->setSubExp3(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context ternary\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context ternary\n";
        break;
    }
}

void XMLProgParser::start_const(const QXmlStreamAttributes &attr) {
    Context *ctx = stack.front();
    if (phase == 1) {
        ctx->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    QStringRef value = attr.value(QLatin1Literal("value"));
    QStringRef opstring = attr.value(QLatin1Literal("op"));
    assert(!value.isEmpty());
    assert(!opstring.isEmpty());
    // LOG_STREAM() << "got value=" << value << " opstring=" << opstring << "\n";
    OPER op = (OPER)operFromString(opstring);
    assert(op != -1);
    switch (op) {
    case opIntConst:
        ctx->exp = new Const(value.toInt());
        addId(attr, stack.front()->exp);
        break;
    case opStrConst:
        ctx->exp = Const::get(value.toString());
        addId(attr, stack.front()->exp);
        break;
    case opFltConst:
        stack.front()->exp = new Const(value.toFloat());
        addId(attr, stack.front()->exp);
        break;
    default:
        LOG << "unknown Const op " << op << "\n";
        assert(false);
    }
    // LOG_STREAM() << "end of start const\n";
}

void XMLProgParser::addToContext_const(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context const\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context const\n";
    //    break;
    //      }
}

void XMLProgParser::start_terminal(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    OPER op = (OPER)operFromString(attr.value(QLatin1Literal("op")));
    assert(op != -1);
    stack.front()->exp = new Terminal(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_terminal(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context terminal\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context terminal\n";
    //    break;
    //      }
}

void XMLProgParser::start_typedexp(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        return;
    }
    stack.front()->exp = new TypedExp();
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_typedexp(Context *c, int e) {
    TypedExp *t = dynamic_cast<TypedExp *>(c->exp);
    assert(t);
    switch (e) {
    case e_type:
        t->type = stack.front()->type;
        break;
    case e_subexp1:
        c->exp->setSubExp1(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context typedexp\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context typedexp\n";
        break;
    }
}

void XMLProgParser::start_refexp(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp *)findId(attr.value(QLatin1Literal("id")));
        RefExp *r = dynamic_cast<RefExp *>(stack.front()->exp);
        assert(r);
        r->def = (Instruction *)findId(attr.value(QLatin1Literal("def")));
        return;
    }
    stack.front()->exp = new RefExp();
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_refexp(Context *c, int e) {
    switch (e) {
    case e_subexp1:
        c->exp->setSubExp1(stack.front()->exp);
        break;
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context refexp\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context refexp\n";
        break;
    }
}

void XMLProgParser::start_def(const QXmlStreamAttributes &attr) {
    if (phase == 1) {
        stack.front()->stmt = (Instruction *)findId(attr.value(QLatin1Literal("stmt")));
        return;
    }
}

void XMLProgParser::addToContext_def(Context * /*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        LOG_STREAM() << "unknown tag " << e << " in context def\n";
    else
        LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context def\n";
    //        break;
    //      }
}

Prog *XMLProgParser::parse(const QString &filename) {

    if (!QFile::exists(filename))
        return nullptr;

    stack.clear();
    Prog *prog = nullptr;
    for (phase = 0; phase < 2; phase++) {
        parseFile(filename);
        if (stack.front()->prog) {
            prog = stack.front()->prog;
            parseChildren(prog->getRootCluster());
        }
    }
    if (prog == nullptr)
        return nullptr;
    // FrontEnd *pFE = FrontEnd::Load(prog->getPath(), prog);        // Path is usually empty!?
    FrontEnd *pFE = FrontEnd::Load(prog->getPathAndName(), prog);
    prog->setFrontEnd(pFE);
    return prog;
}
void XMLProgParser::parseFile(const QString &filename) {
    QFile src(filename);
    if (!src.exists() || !src.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << " File " << filename << " cannot be opened";
        return;
    }
    QXmlStreamReader xml_stream(&src);
    while (!xml_stream.atEnd() && !xml_stream.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml_stream.readNext();
        switch (token) {
        case QXmlStreamReader::StartDocument:
            break;
        case QXmlStreamReader::StartElement:
            handleElementStart(xml_stream);
            break;
        case QXmlStreamReader::EndElement:
            handleElementEnd(xml_stream);
            break;
        default:
            break;
        }
    }
    /* Error handling. */
    if (xml_stream.hasError()) {
        qWarning() << "Xml parse failed: " << xml_stream.errorString();
    }
}

void XMLProgParser::parseChildren(Module *c) {
    QString path = c->makeDirs();
    for (auto &elem : c->Children) {
        QString d = path + "/" + elem->getName() + ".xml";
        parseFile(d);
        parseChildren(elem);
    }
}

extern const char *operStrings[];

int XMLProgParser::operFromString(const QStringRef &s) {
    for (int i = 0; i < opNumOf; i++)
        if (s == operStrings[i])
            return i;
    return -1;
}

// out << \" *(\w+) *= *\\"" *<< *(.*) *<< *"\\"";
// out.writeAttribute("\1",\2);

void XMLProgParser::persistToXML(QXmlStreamWriter &out, Module *c) {
    out.writeStartElement("module");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(c).m_value));
    out.writeAttribute("name", c->Name);
    for (auto p : *c) {
        QXmlStreamWriter wrt(c->getStream().device());
        wrt.writeStartDocument();
        wrt.writeStartElement("procs");
        persistToXML(wrt, p);
        wrt.writeEndElement();
        wrt.writeEndElement();
    }
    for (auto &elem : c->Children) {
        persistToXML(out, elem);
    }
    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, Global *g) {
    out.writeStartElement("global");
    out.writeAttribute("name", g->nam);
    out.writeAttribute("uaddr", QString::number(g->uaddr.m_value, 16));
    out.writeStartElement("type");
    persistToXML(out, g->type);
    out.writeEndElement();
    out.writeEndElement();
}

void XMLProgParser::persistToXML(Prog *prog) {
    prog->m_rootCluster->openStreams("xml");
    QTextStream &os = prog->m_rootCluster->getStream();
    QXmlStreamWriter wrt(os.device());
    wrt.writeStartDocument();
    if (prog->m_rootCluster->getUpstream())
        wrt.writeStartElement("procs");

    wrt.writeStartElement("prog");
    wrt.writeAttribute("path", prog->getPath());
    wrt.writeAttribute("name", prog->getName());
    wrt.writeAttribute("iNumberedProc", QString::number(prog->m_iNumberedProc));
    wrt.writeEndElement();
    for (auto const &elem : prog->globals)
        persistToXML(wrt, elem);
    persistToXML(wrt, prog->m_rootCluster);
    if (prog->m_rootCluster->getUpstream())
        wrt.writeEndElement();
    wrt.writeEndElement();
    prog->m_rootCluster->closeStreams();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, LibProc *proc) {
    out.writeStartElement("libproc");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(proc).m_value));
    out.writeAttribute("address", QString::number(proc->address.m_value));
    out.writeAttribute("firstCallerAddress", QString::number(proc->m_firstCallerAddr.m_value));
    if (proc->m_firstCaller)
        out.writeAttribute("firstCaller", QString::number(ADDRESS::host_ptr(proc->m_firstCaller).m_value));
    if (proc->Parent)
        out.writeAttribute("cluster", QString::number(ADDRESS::host_ptr(proc->Parent).m_value));

    persistToXML(out, proc->signature);

    for (auto const &elem : proc->callerSet) {
        out.writeStartElement("caller");
        out.writeAttribute("call", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }
    for (auto &elem : proc->provenTrue) {
        out.writeStartElement("proven_true");
        persistToXML(out, elem.first);
        persistToXML(out, elem.second);
        out.writeEndElement();
    }
    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, UserProc *proc) {
    out.writeStartElement("userproc");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(proc).m_value));
    out.writeAttribute("address", QString::number(proc->address.m_value));
    out.writeAttribute("status", QString::number((int)proc->status));
    out.writeAttribute("firstCallerAddress", QString::number(proc->m_firstCallerAddr.m_value));
    if (proc->m_firstCaller)
        out.writeAttribute("firstCaller", QString::number(ADDRESS::host_ptr(proc->m_firstCaller).m_value));
    if (proc->Parent)
        out.writeAttribute("cluster", QString::number(ADDRESS::host_ptr(proc->Parent).m_value));
    if (proc->theReturnStatement)
        out.writeAttribute("retstmt", QString::number(ADDRESS::host_ptr(proc->theReturnStatement).m_value));

    persistToXML(out, proc->signature);

    for (auto const &elem : proc->callerSet) {
        out.writeStartElement("caller");
        out.writeAttribute("call", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }
    for (auto &elem : proc->provenTrue) {
        out.writeStartElement("proven_true");
        persistToXML(out, elem.first);
        persistToXML(out, elem.second);
        out.writeEndElement();
    }

    for (auto &elem : proc->locals) {
        out.writeStartElement("local");
        out.writeAttribute("name", (elem).first);
        out.writeStartElement("type");
        persistToXML(out, (elem).second);
        out.writeEndElement();
        out.writeEndElement();
    }

    for (auto &elem : proc->symbolMap) {
        out.writeStartElement("symbol");
        out.writeStartElement("exp");
        persistToXML(out, (elem).first);
        out.writeEndElement();
        out.writeStartElement("secondexp");
        persistToXML(out, (elem).second);
        out.writeEndElement();
        out.writeEndElement();
    }

    for (auto &elem : proc->calleeList) {
        out.writeStartElement("callee");
        out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }

    persistToXML(out, proc->cfg);

    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, Function *proc) {
    if (proc->isLib())
        persistToXML(out, (LibProc *)proc);
    else
        persistToXML(out, (UserProc *)proc);
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, Signature *sig) {
    out.writeStartElement("signature");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(sig).m_value));
    out.writeAttribute("name", sig->name);
    out.writeAttribute("ellipsis", QString::number((int)sig->ellipsis));
    out.writeAttribute("preferedName", sig->preferedName);
    if (sig->getPlatform() != PLAT_GENERIC)
        out.writeAttribute("platform", sig->platformName(sig->getPlatform()));
    if (sig->getConvention() != CONV_NONE)
        out.writeAttribute("convention", sig->conventionName(sig->getConvention()));

    for (auto &elem : sig->params) {
        out.writeStartElement("param");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeAttribute("name", elem->name());
        out.writeStartElement("type");
        persistToXML(out, elem->getType());
        out.writeEndElement();
        out.writeStartElement("exp");
        persistToXML(out, elem->getExp());
        out.writeEndElement();
        out.writeEndElement();
    }
    for (auto &elem : sig->returns) {
        out.writeStartElement("return");
        out.writeStartElement("type");
        persistToXML(out, (elem)->type);
        out.writeEndElement();
        out.writeStartElement("exp");
        persistToXML(out, (elem)->exp);
        out.writeEndElement();
        out.writeEndElement();
    }
    if (sig->rettype) {
        out.writeStartElement("rettype");
        persistToXML(out, sig->rettype);
        out.writeEndElement();
    }
    if (sig->preferedReturn) {
        out.writeStartElement("prefreturn");
        persistToXML(out, sig->preferedReturn);
        out.writeEndElement();
    }
    for (auto &elem : sig->preferedParams) {
        out.writeStartElement("prefparam");
        out.writeAttribute("index", QString::number(elem));
        out.writeEndElement();
    }
    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, const SharedType &ty) {
    auto v = ty->as<VoidType>();
    if (v) {
        out.writeStartElement("voidtype");
        out.writeAttribute("index", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeEndElement();
        return;
    }
    auto f = ty->as<FuncType>();
    if (f) {
        out.writeStartElement("functype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        persistToXML(out, f->signature);
        out.writeEndElement();
        return;
    }
    auto i = ty->as<IntegerType>();
    if (i) {
        out.writeStartElement("integertype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeAttribute("size", QString::number(i->size));
        out.writeAttribute("signedness", QString::number(i->signedness));
        out.writeEndElement();
        return;
    }
    auto fl = ty->as<FloatType>();
    if (fl) {
        out.writeStartElement("floattype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeAttribute("size", QString::number(fl->size));
        out.writeEndElement();
        return;
    }
    auto b = ty->as<BooleanType>();
    if (b) {
        out.writeStartElement("booleantype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeEndElement();
        return;
    }
    auto c = ty->as<CharType>();
    if (c) {
        out.writeStartElement("chartype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeEndElement();
        return;
    }
    const PointerType *p = dynamic_cast<const PointerType *>(ty.get());
    if (p) {
        out.writeStartElement("pointertype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        persistToXML(out, p->points_to);
        out.writeEndElement();
        return;
    }
    auto a = ty->as<ArrayType>();
    if (a) {
        out.writeStartElement("arraytype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeAttribute("length", QString::number((int)a->Length));
        out.writeStartElement("basetype");
        persistToXML(out, a->BaseType);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    auto n = ty->asNamed();
    if (n) {
        out.writeStartElement("namedtype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeAttribute("name", n->name);
        out.writeEndElement();
        return;
    }
    auto co = ty->as<CompoundType>();
    if (co) {
        out.writeStartElement("compoundtype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        for (unsigned i = 0; i < co->names.size(); i++) {
            out.writeStartElement("member");
            out.writeAttribute("name", co->names[i]);
            persistToXML(out, co->types[i]);
            out.writeEndElement();
        }
        out.writeEndElement();
        return;
    }
    auto sz = ty->as<SizeType>();
    if (sz) {
        out.writeStartElement("sizetype");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(ty.get()).m_value));
        out.writeAttribute("size", QString::number(sz->getSize()));
        out.writeEndElement();
        return;
    }
    LOG_STREAM() << "unknown type in persistToXML\n";
    assert(false);
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, const Exp *e) {
    const TypeVal *t = dynamic_cast<const TypeVal *>(e);
    const char *op_name = e->getOperName();
    if (t) {
        out.writeStartElement("typeval");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("type");
        persistToXML(out, t->val);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const Terminal *te = dynamic_cast<const Terminal *>(e);
    if (te) {
        out.writeStartElement("terminal");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeEndElement();
        return;
    }
    const Const *c = dynamic_cast<const Const *>(e);
    if (c) {
        out.writeStartElement("const");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeAttribute("conscript", QString::number(c->conscript));
        if (c->op == opIntConst)
            out.writeAttribute("value", QString::number(c->u.i));
        else if (c->op == opFuncConst)
            out.writeAttribute("value", QString::number(c->u.a.m_value));
        else if (c->op == opFltConst)
            out.writeAttribute("value", QString::number(c->u.d));
        else if (c->op == opStrConst)
            out.writeAttribute("value", c->strin);
        else {
            // TODO
            // QWord ll;
            // Proc* pp;
            assert(false);
        }
        out.writeEndElement();
        return;
    }
    const Location *l = dynamic_cast<const Location *>(e);
    if (l) {
        out.writeStartElement("location");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        if (l->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(l->proc).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, l->subExp1);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const RefExp *r = dynamic_cast<const RefExp *>(e);
    if (r) {
        out.writeStartElement("refexp");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        if (r->def)
            out.writeAttribute("def", QString::number(ADDRESS::host_ptr(r->def).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, r->subExp1);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const FlagDef *f = dynamic_cast<const FlagDef *>(e);
    if (f) {
        out.writeStartElement("flagdef");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        if (f->rtl)
            out.writeAttribute("rtl", QString::number(ADDRESS::host_ptr(f->rtl.get()).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, f->subExp1);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const TypedExp *ty = dynamic_cast<const TypedExp *>(e);
    if (ty) {
        out.writeStartElement("typedexp");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, ty->subExp1);
        out.writeEndElement();
        out.writeStartElement("type");
        persistToXML(out, ty->type);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const Ternary *tn = dynamic_cast<const Ternary *>(e);
    if (tn) {
        out.writeStartElement("ternary");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, tn->subExp1);
        out.writeEndElement();
        out.writeStartElement("subexp2");
        persistToXML(out, tn->subExp2);
        out.writeEndElement();
        out.writeStartElement("subexp3");
        persistToXML(out, tn->subExp3);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const Binary *b = dynamic_cast<const Binary *>(e);
    if (b) {
        out.writeStartElement("binary");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, b->subExp1);
        out.writeEndElement();
        out.writeStartElement("subexp2");
        persistToXML(out, b->subExp2);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    const Unary *u = dynamic_cast<const Unary *>(e);
    if (u) {
        out.writeStartElement("unary");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(e).m_value));
        out.writeAttribute("op", op_name);
        out.writeStartElement("subexp1");
        persistToXML(out, u->subExp1);
        out.writeEndElement();
        out.writeEndElement();
        return;
    }
    LOG_STREAM() << "unknown exp in persistToXML\n";
    assert(false);
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, Cfg *cfg) {
    out.writeStartElement("cfg");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(cfg).m_value));
    out.writeAttribute("wellformed", QString::number(cfg->WellFormed));
    out.writeAttribute("lastLabel", QString::number(cfg->lastLabel));
    out.writeAttribute("entryBB", QString::number(ADDRESS::host_ptr(cfg->entryBB).m_value));
    out.writeAttribute("exitBB", QString::number(ADDRESS::host_ptr(cfg->exitBB).m_value));

    for (auto &elem : cfg->m_listBB)
        persistToXML(out, elem);

    for (auto &elem : cfg->Ordering) {
        out.writeStartElement("order");
        out.writeAttribute("bb", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }

    for (auto &elem : cfg->revOrdering) {
        out.writeStartElement("revorder");
        out.writeAttribute("bb", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }

    // TODO
    // MAPBB m_mapBB;
    // std::set<CallStatement*> callSites;
    // std::vector<PBB> BBs;               // Pointers to BBs from indices
    // std::map<PBB, int> indices;           // Indices from pointers to BBs
    // more
    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, const BasicBlock *bb) {
    out.writeStartElement("order");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(bb).m_value));
    out.writeAttribute("nodeType", QString::number((int)bb->NodeType));
    out.writeAttribute("labelNum", QString::number(bb->LabelNum));
    out.writeAttribute("labelneeded", QString::number(bb->LabelNeeded));
    out.writeAttribute("incomplete", QString::number(bb->Incomplete));
    out.writeAttribute("jumpreqd", QString::number(bb->JumpReqd));
    out.writeAttribute("m_traversed", QString::number(bb->TraversedMarker));
    out.writeAttribute("jumpreqd", QString::number(bb->JumpReqd));
    out.writeAttribute("DFTfirst", QString::number(bb->DFTfirst));
    out.writeAttribute("DFTlast", QString::number(bb->DFTlast));
    out.writeAttribute("DFTrevfirst", QString::number(bb->DFTrevfirst));
    out.writeAttribute("DFTrevlast", QString::number(bb->DFTrevlast));
    out.writeAttribute("structType", QString::number(bb->StructType));
    out.writeAttribute("loopCondType", QString::number(bb->LoopCondType));
    out.writeAttribute("ord", QString::number(bb->Ord));
    out.writeAttribute("revOrd", QString::number(bb->RevOrd));
    out.writeAttribute("inEdgesVisited", QString::number(bb->InEdgesVisited));
    out.writeAttribute("numForwardInEdges", QString::number(bb->NumForwardInEdges));
    out.writeAttribute("loopStamp1", QString::number(bb->LoopStamps[0]));
    out.writeAttribute("loopStamp2", QString::number(bb->LoopStamps[1]));
    out.writeAttribute("revLoopStamp1", QString::number(bb->RevLoopStamps[0]));
    out.writeAttribute("revLoopStamp2", QString::number(bb->RevLoopStamps[1]));
    out.writeAttribute("traversed", QString::number((int)bb->Traversed));
    out.writeAttribute("hllLabel", QString::number((int)bb->HllLabel));
    if (!bb->LabelStr.isEmpty())
        out.writeAttribute("labelStr", bb->LabelStr);
    out.writeAttribute("indentLevel", QString::number(bb->IndentLevel));
    // note the rediculous duplication here
    if (bb->ImmPDom)
        out.writeAttribute("immPDom", QString::number(ADDRESS::host_ptr(bb->ImmPDom).m_value));
    if (bb->LoopHead)
        out.writeAttribute("loopHead", QString::number(ADDRESS::host_ptr(bb->LoopHead).m_value));
    if (bb->CaseHead)
        out.writeAttribute("caseHead", QString::number(ADDRESS::host_ptr(bb->CaseHead).m_value));
    if (bb->CondFollow)
        out.writeAttribute("condFollow", QString::number(ADDRESS::host_ptr(bb->CondFollow).m_value));
    if (bb->LoopFollow)
        out.writeAttribute("loopFollow", QString::number(ADDRESS::host_ptr(bb->LoopFollow).m_value));
    if (bb->LatchNode)
        out.writeAttribute("latchNode", QString::number(ADDRESS::host_ptr(bb->LatchNode).m_value));
    out.writeAttribute("sType", QString::number((int)bb->StructuringType));
    out.writeAttribute("usType", QString::number((int)bb->UnstructuredType));
    out.writeAttribute("lType", QString::number((int)bb->LoopHeaderType));
    out.writeAttribute("cType", QString::number((int)bb->ConditionHeaderType));

    for (auto &elem : bb->InEdges) {
        out.writeStartElement("inedge");
        out.writeAttribute("bb", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }
    for (auto &elem : bb->OutEdges) {
        out.writeStartElement("outedge");
        out.writeAttribute("bb", QString::number(ADDRESS::host_ptr(elem).m_value));
        out.writeEndElement();
    }

    LocationSet::iterator it;
    for (it = bb->LiveIn.begin(); it != bb->LiveIn.end(); it++) {
        out.writeStartElement("livein");
        persistToXML(out, *it);
        out.writeEndElement();
    }

    if (bb->ListOfRTLs) {
        for (auto &elem : *bb->ListOfRTLs)
            persistToXML(out, elem);
    }
    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, const RTL *rtl) {
    out.writeStartElement("rtl");
    out.writeAttribute("id", QString::number(ADDRESS::host_ptr(rtl).m_value));
    out.writeAttribute("addr", QString::number(rtl->nativeAddr.m_value));
    for (auto const &elem : *rtl) {
        out.writeStartElement("stmt");
        persistToXML(out, elem);
        out.writeEndElement();
    }
    out.writeEndElement();
}

void XMLProgParser::persistToXML(QXmlStreamWriter &out, const Instruction *stmt) {
    const BoolAssign *b = dynamic_cast<const BoolAssign *>(stmt);
    if (b) {
        out.writeStartElement("boolasgn");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(b->Number));
        //        if (b->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(b->parent).m_value));
        if (b->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(b->proc).m_value));

        out.writeAttribute("jtcond", QString::number(b->jtCond));
        out.writeAttribute("float", QString::number((int)b->bFloat));
        out.writeAttribute("size", QString::number(b->Size));

        if (b->pCond) {
            out.writeStartElement("cond");
            persistToXML(out, b->pCond);
            out.writeEndElement();
        }
        out.writeEndElement();
        return;
    }
    const ReturnStatement *r = dynamic_cast<const ReturnStatement *>(stmt);
    if (r) {
        out.writeStartElement("returnstmt");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(r->Number));
        //        if (r->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(r->parent).m_value));
        if (r->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(r->proc).m_value));
        out.writeAttribute("retAddr", QString::number(r->retAddr.m_value));

        for (auto const &elem : r->modifieds) {
            out.writeStartElement("modifieds");
            persistToXML(out, elem);
            out.writeEndElement();
        }
        for (auto const &elem : r->returns) {
            out.writeStartElement("returns");
            persistToXML(out, elem);
            out.writeEndElement();
        }

        out.writeEndElement();
        return;
    }
    const CallStatement *c = dynamic_cast<const CallStatement *>(stmt);
    if (c) {
        out.writeStartElement("callstmt");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(c->Number));
        out.writeAttribute("computed", QString::number(c->m_isComputed));
        //        if (c->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(c->parent).m_value));
        if (c->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(c->proc).m_value));
        out.writeAttribute("returnAfterCall", QString::number((int)c->returnAfterCall));

        if (c->pDest) {
            out.writeStartElement("dest");
            if (c->procDest)
                out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(c->procDest).m_value));
            persistToXML(out, c->pDest);
            out.writeEndElement();
        }

        for (auto const &elem : c->arguments) {
            out.writeStartElement("argument");
            persistToXML(out, elem);
            out.writeEndElement();
        }

        for (auto const &elem : c->defines) {
            out.writeStartElement("defines");
            persistToXML(out, elem);
            out.writeEndElement();
        }

        out.writeEndElement();
        return;
    }
    const CaseStatement *ca = dynamic_cast<const CaseStatement *>(stmt);
    if (ca) {
        out.writeStartElement("casestmt");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(ca->Number));
        out.writeAttribute("computed", QString::number(ca->m_isComputed));

        //        if (ca->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(ca->parent).m_value));
        if (ca->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(ca->proc).m_value));

        if (ca->pDest) {
            out.writeStartElement("dest");
            persistToXML(out, ca->pDest);
            out.writeEndElement();
        }
        // TODO
        // SWITCH_INFO* pSwitchInfo;   // Ptr to struct with info about the switch
        out.writeEndElement();
        return;
    }
    const BranchStatement *br = dynamic_cast<const BranchStatement *>(stmt);
    if (br) {
        out.writeStartElement("branchstmt");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(br->Number));
        out.writeAttribute("computed", QString::number(br->m_isComputed));
        out.writeAttribute("jtcond", QString::number(br->jtCond));
        out.writeAttribute("float", QString::number(br->bFloat));
        //        if (br->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(br->parent).m_value));
        if (br->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(br->proc).m_value));

        if (br->pDest) {
            out.writeStartElement("dest");
            persistToXML(out, br->pDest);
            out.writeEndElement();
        }
        if (br->pCond) {
            out.writeStartElement("cond");
            persistToXML(out, br->pCond);
            out.writeEndElement();
        }
        out.writeEndElement();
        return;
    }
    const GotoStatement *g = dynamic_cast<const GotoStatement *>(stmt);
    if (g) {
        out.writeStartElement("gotostmt");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(g->Number));
        out.writeAttribute("computed", QString::number(g->m_isComputed));
        //        if (g->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(g->parent).m_value));
        if (g->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(g->proc).m_value));
        if (g->pDest) {
            out.writeStartElement("dest");
            persistToXML(out, g->pDest);
            out.writeEndElement();
        }
        out.writeEndElement();
        return;
    }
    const PhiAssign *p = dynamic_cast<const PhiAssign *>(stmt);
    if (p) {
        out.writeStartElement("phiassign");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(p->Number));
        //        if (p->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(p->parent).m_value));
        if (p->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(p->proc).m_value));

        out.writeStartElement("lhs");
        persistToXML(out, p->lhs);
        out.writeEndElement();
        for (auto it = p->cbegin(); it != p->cend(); p++) {
            out.writeStartElement("def");
            out.writeAttribute("stmt", QString::number(ADDRESS::host_ptr(it->second.def()).m_value));
            out.writeAttribute("exp", QString::number(ADDRESS::host_ptr(it->second.e).m_value));
            out.writeEndElement();
        }
        out.writeEndElement();
        return;
    }
    const Assign *a = dynamic_cast<const Assign *>(stmt);
    if (a) {
        out.writeStartElement("assign");
        out.writeAttribute("id", QString::number(ADDRESS::host_ptr(stmt).m_value));
        out.writeAttribute("number", QString::number(a->Number));
        //        if (a->parent)
        //            out.writeAttribute("parent",QString::number(ADDRESS::host_ptr(a->parent).m_value));
        if (a->proc)
            out.writeAttribute("proc", QString::number(ADDRESS::host_ptr(a->proc).m_value));

        out.writeStartElement("lhs");
        persistToXML(out, a->lhs);
        out.writeEndElement();
        out.writeStartElement("rhs");
        persistToXML(out, a->rhs);
        out.writeEndElement();
        if (a->type) {
            out.writeStartElement("type");
            persistToXML(out, a->type);
            out.writeEndElement();
        }
        if (a->guard) {
            out.writeStartElement("guard");
            persistToXML(out, a->guard);
            out.writeEndElement();
        }
        out.writeEndElement();
        return;
    }
    LOG_STREAM() << "unknown stmt in persistToXML\n";
    assert(false);
}

void XMLProgParser::addToContext_assignment(Context *c, int /*e*/) { c->stmt = stack.front()->stmt; }

void XMLProgParser::addToContext_phiassign(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    PhiAssign *pa = dynamic_cast<PhiAssign *>(c->stmt);
    assert(pa);
    switch (e) {
    case e_lhs:
        pa->setLeft(stack.front()->exp);
        break;
    // FIXME: More required
    default:
        if (e == e_unknown)
            LOG_STREAM() << "unknown tag " << e << " in context assign\n";
        else
            LOG_STREAM() << "need to handle tag " << tags[e].tag << " in context assign\n";
        break;
    }
}
