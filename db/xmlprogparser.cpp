/*
 * Copyright (C) 2004, Trent Waddington
 */
/***************************************************************************//**
 * \file    xmlprogparser.cpp
 * \brief   Implementation of the XMLProgParser and related classes.
 ******************************************************************************/
#include <cstdio>
#include <cstring>
extern "C" {
#include "expat.h"
}
#include "type.h"
#include "cluster.h"
#include "prog.h"
#include "proc.h"
#include "rtl.h"
#include "statement.h"
#include "sigenum.h"
#include "signature.h"
#include "xmlprogparser.h"
#include "boomerang.h"
#include "log.h"
#include "frontend.h"

typedef enum { e_prog, e_procs, e_global, e_cluster, e_libproc, e_userproc, e_local, e_symbol, e_secondexp,
               e_proven_true, e_callee, e_caller, e_defines,
               e_signature, e_param, e_return, e_rettype, e_prefreturn, e_prefparam,
               e_cfg, e_bb, e_inedge, e_outedge, e_livein, e_order, e_revorder,
               e_rtl, e_stmt, e_assign, e_assignment, e_phiassign, e_lhs, e_rhs,
               e_callstmt, e_dest, e_argument, e_returnexp, e_returntype,
               e_returnstmt, e_returns, e_modifieds,
               e_gotostmt, e_branchstmt, e_cond,
               e_casestmt,
               e_boolasgn,
               e_type, e_exp,
               e_voidtype, e_integertype, e_pointertype, e_chartype, e_namedtype, e_arraytype, e_basetype, e_sizetype,
               e_location, e_unary, e_binary, e_ternary, e_const, e_terminal, e_typedexp, e_refexp, e_def,
               e_subexp1, e_subexp2, e_subexp3, e_unknown = -1 } xmlElement;

#define TAG(x) &XMLProgParser::start_ ## x, &XMLProgParser::addToContext_ ## x

_tag XMLProgParser::tags[] = {
    { "prog", TAG(prog) },
    { "procs", TAG(procs) },
    { "global", TAG(global) },
    { "cluster", TAG(cluster) },
    { "libproc", TAG(libproc) },
    { "userproc", TAG(userproc) },
    { "local", TAG(local) },
    { "symbol", TAG(symbol) },
    { "secondexp", TAG(secondexp) },
    { "proven_true", TAG(proven_true) },
    { "callee", TAG(callee) },
    { "caller", TAG(caller) },
    { "defines", TAG(defines) },
    { "signature", TAG(signature) },
    { "param",    TAG(param) },
    { "return", TAG(return) },
    { "rettype", TAG(rettype) },
    { "prefreturn", TAG(prefreturn) },
    { "prefparam", TAG(prefparam) },
    { "cfg", TAG(cfg) },
    { "bb", TAG(bb) },
    { "inedge", TAG(inedge) },
    { "outedge", TAG(outedge) },
    { "livein", TAG(livein) },
    { "order", TAG(order) },
    { "revorder", TAG(revorder) },
    { "rtl", TAG(rtl) },
    { "stmt", TAG(stmt) },
    { "assign", TAG(assign) },
    { "assignment", TAG(assignment) },
    { "phiassign", TAG(phiassign) },
    { "lhs", TAG(lhs) },
    { "rhs", TAG(rhs) },
    { "callstmt", TAG(callstmt) },
    { "dest", TAG(dest) },
    { "argument", TAG(argument) },
    { "returnexp", TAG(returnexp) },
    { "returntype", TAG(returntype) },
    { "returnstmt", TAG(returnstmt) },
    { "returns", TAG(returns) },
    { "modifieds", TAG(modifieds) },
    { "gotostmt", TAG(gotostmt) },
    { "branchstmt", TAG(branchstmt) },
    { "cond", TAG(cond) },
    { "casestmt", TAG(casestmt) },
    { "boolasgn", TAG(boolasgn) },
    { "type", TAG(type) },
    { "exp", TAG(exp) },
    { "voidtype", TAG(voidtype) },
    { "integertype", TAG(integertype) },
    { "pointertype", TAG(pointertype) },
    { "chartype", TAG(chartype) },
    { "namedtype", TAG(namedtype) },
    { "arraytype", TAG(arraytype) },
    { "basetype", TAG(basetype) },
    { "sizetype", TAG(sizetype) },
    { "location", TAG(location) },
    { "unary", TAG(unary) },
    { "binary", TAG(binary) },
    { "ternary", TAG(ternary) },
    { "const", TAG(const) },
    { "terminal", TAG(terminal) },
    { "typedexp", TAG(typedexp) },
    { "refexp", TAG(refexp) },
    { "def", TAG(def) },
    { "subexp1", TAG(subexp1) },
    { "subexp2", TAG(subexp2) },
    { "subexp3", TAG(subexp3) },
    { nullptr, nullptr, nullptr}
};

class Context {
public:
    int tag;
    int n;
    std::string str;
    Prog *prog;
    Global *global;
    Cluster *cluster;
    Proc *proc;
    Signature *signature;
    Cfg *cfg;
    BasicBlock *bb;
    RTL *rtl;
    Statement *stmt;
    Parameter *param;
    // ImplicitParameter *implicitParam;
    Return *ret;
    Type *type;
    Exp *exp, *symbol;
    std::list<Proc*> procs;

    Context(int tag) : tag(tag), prog(nullptr), proc(nullptr), signature(nullptr), cfg(nullptr), bb(nullptr), rtl(nullptr), stmt(nullptr), param(nullptr), /*implicitParam(nullptr),*/ ret(nullptr), type(nullptr), exp(nullptr) { }
};

static void XMLCALL
start(void *data, const char *el, const char **attr) {
    ((XMLProgParser*)data)->handleElementStart(el, attr);
}

static void XMLCALL end(void *data, const char *el) {
    ((XMLProgParser*)data)->handleElementEnd(el);
}

static void XMLCALL text(void */*data*/, const char *s, int len) {
    int mylen;
    char buf[1024];
    if (len == 1 && *s == '\n')
        return;
    mylen = len < 1024 ? len : 1023;
    memcpy(buf, s, mylen);
    buf[mylen] = 0;
    printf("error: text in document %i bytes (%s)\n", len, buf);
}

const char *XMLProgParser::getAttr(const char **attr, const char *name) {
    for (int i = 0; attr[i]; i += 2)
        if (!strcmp(attr[i], name))
            return attr[i+1];
    return nullptr;
}

void XMLProgParser::handleElementStart(const char *el, const char **attr) {
    for (int i = 0; tags[i].tag; i++)
        if (!strcmp(el, tags[i].tag)) {
            //std::cerr << "got tag: " << tags[i].tag << "\n";
            stack.push_front(new Context(i));
            (this->*tags[i].start_proc)(attr);
            return;
        }
    std::cerr << "got unknown tag: " << el << "\n";
    stack.push_front(new Context(e_unknown));
}

void XMLProgParser::handleElementEnd(const char */*el*/) {
    //std::cerr << "end tag: " << el << " tos: " << stack.front()->tag << "\n";
    std::list<Context*>::iterator it = stack.begin();
    if (it != stack.end()) {
        it++;
        if (it != stack.end()) {
            if ((*it)->tag != e_unknown) {
                //std::cerr << " second: " << (*it)->tag << "\n";
                (this->*tags[(*it)->tag].end_proc)(*it, stack.front()->tag);
            }
            stack.erase(stack.begin());
        }
    }
}

void XMLProgParser::addId(const char **attr, void *x) {
    const char *val = getAttr(attr, "id");
    if (val) {
        //std::cerr << "map id " << val << " to " << std::hex << (int)x << std::dec << "\n";
        idToX[atoi(val)] = x;
    }
}

void *XMLProgParser::findId(const char *id) {
    if (id == nullptr)
        return nullptr;
    int n = atoi(id);
    if (n == 0)
        return nullptr;
    std::map<int, void*>::iterator it = idToX.find(n);
    if (it == idToX.end()) {
        std::cerr << "findId could not find \"" << id << "\"\n";
        assert(false);
        return nullptr;
    }
    return (*it).second;
}

void XMLProgParser::start_prog(const char **attr) {
    if (phase == 1) {
        return;
    }
    stack.front()->prog = new Prog();
    addId(attr, stack.front()->prog);
    const char *name = getAttr(attr, "name");
    if (name)
        stack.front()->prog->setName(name);
    name = getAttr(attr, "path");
    if (name)
        stack.front()->prog->m_path = name;
    const char *iNumberedProc = getAttr(attr, "iNumberedProc");
    stack.front()->prog->m_iNumberedProc = atoi(iNumberedProc);
}

void XMLProgParser::addToContext_prog(Context *c, int e) {
    if (phase == 1) {
        switch(e) {
            case e_libproc:
            case e_userproc:
                Boomerang::get()->alert_load(stack.front()->proc);
                break;
        }
        return;
    }
    switch(e) {
        case e_libproc:
            stack.front()->proc->setProg(c->prog);
            c->prog->m_procs.push_back(stack.front()->proc);
            c->prog->m_procLabels[stack.front()->proc->getNativeAddress()] = stack.front()->proc;
            break;
        case e_userproc:
            stack.front()->proc->setProg(c->prog);
            c->prog->m_procs.push_back(stack.front()->proc);
            c->prog->m_procLabels[stack.front()->proc->getNativeAddress()] = stack.front()->proc;
            break;
        case e_procs:
            for (auto & elem : stack.front()->procs) {
                c->prog->m_procs.push_back(elem);
                c->prog->m_procLabels[(elem)->getNativeAddress()] = elem;
                Boomerang::get()->alert_load(elem);
            }
            break;
        case e_cluster:
            c->prog->m_rootCluster = stack.front()->cluster;
            break;
        case e_global:
            c->prog->globals.insert(stack.front()->global);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context prog\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context prog\n";
            break;
    }
}

void XMLProgParser::start_procs(const char **/*attr*/) {
    if (phase == 1) {
        return;
    }
    stack.front()->procs.clear();
}

void XMLProgParser::addToContext_procs(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_libproc:
            c->procs.push_back(stack.front()->proc);
            break;
        case e_userproc:
            c->procs.push_back(stack.front()->proc);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context procs\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context procs\n";
            break;
    }
}

void XMLProgParser::start_global(const char **attr) {
    if (phase == 1) {
        return;
    }
    stack.front()->global = new Global();
    addId(attr, stack.front()->global);
    const char *name = getAttr(attr, "name");
    if (name)
        stack.front()->global->nam = name;
    const char *uaddr = getAttr(attr, "uaddr");
    if (uaddr)
        stack.front()->global->uaddr = ADDRESS::g(atoi(uaddr));
}

void XMLProgParser::addToContext_global(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_type:
            c->global->type = stack.front()->type;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context global\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context global\n";
            break;
    }
}

void XMLProgParser::start_cluster(const char **attr) {
    if (phase == 1) {
        stack.front()->cluster = (Cluster*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->cluster = new Cluster();
    addId(attr, stack.front()->cluster);
    const char *name = getAttr(attr, "name");
    if (name)
        stack.front()->cluster->setName(name);
}

void XMLProgParser::addToContext_cluster(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_cluster:
            c->cluster->addChild(stack.front()->cluster);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context cluster\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context cluster\n";
            break;
    }
}

void XMLProgParser::start_libproc(const char **attr) {
    if (phase == 1) {
        stack.front()->proc = (Proc*)findId(getAttr(attr, "id"));
        Proc *p = (Proc*)findId(getAttr(attr, "firstCaller"));
        if (p)
            stack.front()->proc->m_firstCaller = p;
        Cluster *c = (Cluster*)findId(getAttr(attr, "cluster"));
        if (c)
            stack.front()->proc->cluster = c;
        return;
    }
    stack.front()->proc = new LibProc();
    addId(attr, stack.front()->proc);
    const char *address = getAttr(attr, "address");
    if (address)
        stack.front()->proc->address = ADDRESS::g(atoi(address));
    address = getAttr(attr, "firstCallerAddress");
    if (address)
        stack.front()->proc->m_firstCallerAddr = ADDRESS::g(atoi(address));
}

void XMLProgParser::addToContext_libproc(Context *c, int e) {
    if (phase == 1) {
        switch(e) {
            case e_caller:
                CallStatement *call = dynamic_cast<CallStatement*>(stack.front()->stmt);
                assert(call);
                c->proc->addCaller(call);
                break;
        }
        return;
    }
    switch(e) {
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
                std::cerr << "unknown tag " << e << " in context libproc\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context libproc\n";
            break;
    }
}

void XMLProgParser::start_userproc(const char **attr) {
    if (phase == 1) {
        stack.front()->proc = (Proc*)findId(getAttr(attr, "id"));
        UserProc *u = dynamic_cast<UserProc*>(stack.front()->proc);
        assert(u);
        Proc *p = (Proc*)findId(getAttr(attr, "firstCaller"));
        if (p)
            u->m_firstCaller = p;
        Cluster *c = (Cluster*)findId(getAttr(attr, "cluster"));
        if (c)
            u->cluster = c;
        ReturnStatement *r = (ReturnStatement*)findId(getAttr(attr, "retstmt"));
        if (r)
            u->theReturnStatement = r;
        return;
    }
    UserProc *proc = new UserProc();
    stack.front()->proc = proc;
    addId(attr, proc);

    const char *address = getAttr(attr, "address");
    if (address)
        proc->address = ADDRESS::g(atoi(address));
    address = getAttr(attr, "status");
    if (address)
        proc->status = (ProcStatus)atoi(address);
    address = getAttr(attr, "firstCallerAddress");
    if (address)
        proc->m_firstCallerAddr = ADDRESS::g(atoi(address));
}

void XMLProgParser::addToContext_userproc(Context *c, int e) {
    UserProc *userproc = dynamic_cast<UserProc*>(c->proc);
    assert(userproc);
    if (phase == 1) {
        switch(e) {
            case e_caller: {
                CallStatement *call = dynamic_cast<CallStatement*>(stack.front()->stmt);
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
    switch(e) {
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
            userproc->locals[stack.front()->str.c_str()] = stack.front()->type;
            break;
        case e_symbol:
            userproc->mapSymbolTo(stack.front()->exp, stack.front()->symbol);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context userproc\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context userproc\n";
            break;
    }
}

void XMLProgParser::start_local(const char **attr) {
    if (phase == 1) {
        return;
    }
    stack.front()->str = getAttr(attr, "name");
}

void XMLProgParser::addToContext_local(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_type:
            c->type = stack.front()->type;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context local\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context local\n";
            break;
    }
}

void XMLProgParser::start_symbol(const char **/*attr*/) {
    if (phase == 1) {
        return;
    }
}

void XMLProgParser::addToContext_symbol(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_exp:
            c->exp = stack.front()->exp;
            break;
        case e_secondexp:
            c->symbol = stack.front()->exp;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context local\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context local\n";
            break;
    }
}

void XMLProgParser::start_proven_true(const char **/*attr*/) {
}

void XMLProgParser::addToContext_proven_true(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_callee(const char **attr) {
    if (phase == 1) {
        stack.front()->proc = (Proc*)findId(getAttr(attr, "proc"));
    }
}

void XMLProgParser::addToContext_callee(Context */*c*/, int /*e*/) {
}

void XMLProgParser::start_caller(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "call"));
    }
}

void XMLProgParser::addToContext_caller(Context */*c*/, int /*e*/) {
}

void XMLProgParser::start_signature(const char **attr) {
    if (phase == 1) {
        stack.front()->signature = (Signature*)findId(getAttr(attr, "id"));
        return;
    }
    const char *plat = getAttr(attr, "platform");
    const char *convention = getAttr(attr, "convention");
    const char *name = getAttr(attr, "name");

    Signature *sig;
    if (plat && convention) {
        platform p;
        callconv c;
        if (!strcmp(plat, "pentium"))
            p = PLAT_PENTIUM;
        else if (!strcmp(plat, "sparc"))
            p = PLAT_SPARC;
        else if (!strcmp(plat, "ppc"))
            p = PLAT_PPC;
        else if (!strcmp(plat, "st20"))
            p = PLAT_ST20;
        else {
            std::cerr << "unknown platform: " << plat << "\n";
            assert(false);
            p = PLAT_PENTIUM;
        }
        if (!strcmp(convention, "stdc"))
            c = CONV_C;
        else if (!strcmp(convention, "pascal"))
            c = CONV_PASCAL;
        else if (!strcmp(convention, "thiscall"))
            c = CONV_THISCALL;
        else {
            std::cerr << "unknown convention: " << convention << "\n";
            assert(false);
            c = CONV_C;
        }
        sig = Signature::instantiate(p, c, name);
    } else
        sig = new Signature(name);
    sig->params.clear();
    // sig->implicitParams.clear();
    sig->returns.clear();
    stack.front()->signature = sig;
    addId(attr, sig);
    const char *n = getAttr(attr, "ellipsis");
    if (n)
        sig->ellipsis = atoi(n) > 0;
    n = getAttr(attr, "preferedName");
    if (n)
        sig->preferedName = n;
}

void XMLProgParser::addToContext_signature(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
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
                std::cerr << "unknown tag " << e << " in context signature\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context signature\n";
            break;
    }
}

void XMLProgParser::start_param(const char **attr) {
    if (phase == 1) {
        stack.front()->param = (Parameter*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->param = new Parameter;
    addId(attr, stack.front()->param);
    const char *n = getAttr(attr, "name");
    if (n)
        stack.front()->param->name(n);
}

void XMLProgParser::addToContext_param(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_type:
            c->param->setType(stack.front()->type);
            break;
        case e_exp:
            c->param->setExp(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context param\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context param\n";
            break;
    }
}

void XMLProgParser::start_prefreturn(const char **/*attr*/) {
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

void XMLProgParser::start_prefparam(const char **attr) {
    if (phase == 1) {
        return;
    }
    const char *n = getAttr(attr, "index");
    assert(n);
    stack.front()->n = atoi(n);
}

void XMLProgParser::addToContext_prefparam(Context */*c*/, int e) {
    if (phase == 1) {
        return;
    }
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context prefparam\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context prefparam\n";
    //    break;
    //      }
}

void XMLProgParser::start_return(const char **attr) {
    if (phase == 1) {
        stack.front()->ret = (Return*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->ret = new Return();
    addId(attr, stack.front()->ret);
}

void XMLProgParser::addToContext_return(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_type:
            c->ret->type = stack.front()->type;
            break;
        case e_exp:
            c->ret->exp = stack.front()->exp;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context return\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context return\n";
            break;
    }
}

void XMLProgParser::start_rettype(const char **/*attr*/) { }

void XMLProgParser::addToContext_rettype(Context *c, int /*e*/) {
    c->type = stack.front()->type;
}

void XMLProgParser::start_cfg(const char **attr) {
    if (phase == 1) {
        stack.front()->cfg = (Cfg*)findId(getAttr(attr, "id"));
        BasicBlock * entryBB = (BasicBlock *)findId(getAttr(attr, "entryBB"));
        if (entryBB)
            stack.front()->cfg->setEntryBB(entryBB);
        BasicBlock * exitBB = (BasicBlock *)findId(getAttr(attr, "exitBB"));
        if (exitBB)
            stack.front()->cfg->setExitBB(exitBB);
        return;
    }
    Cfg *cfg = new Cfg();
    stack.front()->cfg = cfg;
    addId(attr, cfg);

    const char *str = getAttr(attr, "wellformed");
    if (str)
        cfg->m_bWellFormed = atoi(str) > 0;
    str = getAttr(attr, "lastLabel");
    if (str)
        cfg->lastLabel = atoi(str);
}

void XMLProgParser::addToContext_cfg(Context *c, int e) {
    if (phase == 1) {
        switch(e) {
            case e_order:
                c->cfg->Ordering.push_back(stack.front()->bb);
                break;
            case e_revorder:
                c->cfg->revOrdering.push_back(stack.front()->bb);
                break;
        }
        return;
    }
    switch(e) {
        case e_bb:
            c->cfg->addBB(stack.front()->bb);
            break;
        case e_order:
            break;
        case e_revorder:
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context cfg\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context cfg\n";
            break;
    }
}

void XMLProgParser::start_bb(const char **attr) {
    BasicBlock * bb;
    if (phase == 1) {
        bb = stack.front()->bb = (BasicBlock*)findId(getAttr(attr, "id"));
        BasicBlock * h = (BasicBlock *)findId(getAttr(attr, "m_loopHead"));
        if (h)
            bb->m_loopHead = h;
        h = (BasicBlock *)findId(getAttr(attr, "m_caseHead"));
        if (h)
            bb->m_caseHead = h;
        h = (BasicBlock *)findId(getAttr(attr, "m_condFollow"));
        if (h)
            bb->m_condFollow = h;
        h = (BasicBlock *)findId(getAttr(attr, "m_loopFollow"));
        if (h)
            bb->m_loopFollow = h;
        h = (BasicBlock *)findId(getAttr(attr, "m_latchNode"));
        if (h)
            bb->m_latchNode = h;
        // note the rediculous duplication here
        h = (BasicBlock *)findId(getAttr(attr, "immPDom"));
        if (h)
            bb->immPDom = h;
        h = (BasicBlock *)findId(getAttr(attr, "loopHead"));
        if (h)
            bb->loopHead = h;
        h = (BasicBlock *)findId(getAttr(attr, "caseHead"));
        if (h)
            bb->caseHead = h;
        h = (BasicBlock *)findId(getAttr(attr, "condFollow"));
        if (h)
            bb->condFollow = h;
        h = (BasicBlock *)findId(getAttr(attr, "loopFollow"));
        if (h)
            bb->loopFollow = h;
        h = (BasicBlock *)findId(getAttr(attr, "latchNode"));
        if (h)
            bb->latchNode = h;
        return;
    }
    bb = new BasicBlock();
    stack.front()->bb = bb;
    addId(attr, bb);

    const char *str = getAttr(attr, "nodeType");
    if (str)
        bb->m_nodeType = (BBTYPE)atoi(str);
    str = getAttr(attr, "labelNum");
    if (str)
        bb->m_iLabelNum = atoi(str);
    str = getAttr(attr, "label");
    if (str)
        bb->m_labelStr = strdup(str);
    str = getAttr(attr, "labelneeded");
    if (str)
        bb->m_labelneeded = atoi(str) > 0;
    str = getAttr(attr, "incomplete");
    if (str)
        bb->m_bIncomplete = atoi(str) > 0;
    str = getAttr(attr, "jumpreqd");
    if (str)
        bb->m_bJumpReqd = atoi(str) > 0;
    str = getAttr(attr, "m_traversed");
    if (str)
        bb->m_iTraversed = atoi(str) > 0;
    str = getAttr(attr, "DFTfirst");
    if (str)
        bb->m_DFTfirst = atoi(str);
    str = getAttr(attr, "DFTlast");
    if (str)
        bb->m_DFTlast = atoi(str);
    str = getAttr(attr, "DFTrevfirst");
    if (str)
        bb->m_DFTrevfirst = atoi(str);
    str = getAttr(attr, "DFTrevlast");
    if (str)
        bb->m_DFTrevlast = atoi(str);
    str = getAttr(attr, "structType");
    if (str)
        bb->m_structType = (SBBTYPE)atoi(str);
    str = getAttr(attr, "loopCondType");
    if (str)
        bb->m_loopCondType = (SBBTYPE)atoi(str);
    str = getAttr(attr, "ord");
    if (str)
        bb->ord = atoi(str);
    str = getAttr(attr, "revOrd");
    if (str)
        bb->revOrd = atoi(str);
    str = getAttr(attr, "inEdgesVisited");
    if (str)
        bb->inEdgesVisited = atoi(str);
    str = getAttr(attr, "numForwardInEdges");
    if (str)
        bb->numForwardInEdges = atoi(str);
    str = getAttr(attr, "loopStamp1");
    if (str)
        bb->loopStamps[0] = atoi(str);
    str = getAttr(attr, "loopStamp2");
    if (str)
        bb->loopStamps[1] = atoi(str);
    str = getAttr(attr, "revLoopStamp1");
    if (str)
        bb->revLoopStamps[0] = atoi(str);
    str = getAttr(attr, "revLoopStamp2");
    if (str)
        bb->revLoopStamps[1] = atoi(str);
    str = getAttr(attr, "traversed");
    if (str)
        bb->traversed = (travType)atoi(str);
    str = getAttr(attr, "hllLabel");
    if (str)
        bb->hllLabel = atoi(str) > 0;
    str = getAttr(attr, "labelStr");
    if (str)
        bb->labelStr = strdup(str);
    str = getAttr(attr, "indentLevel");
    if (str)
        bb->indentLevel = atoi(str);
    str = getAttr(attr, "sType");
    if (str)
        bb->sType = (structType)atoi(str);
    str = getAttr(attr, "usType");
    if (str)
        bb->usType = (unstructType)atoi(str);
    str = getAttr(attr, "lType");
    if (str)
        bb->lType = (loopType)atoi(str);
    str = getAttr(attr, "cType");
    if (str)
        bb->cType = (condType)atoi(str);
}

void XMLProgParser::addToContext_bb(Context *c, int e) {
    if (phase == 1) {
        switch(e) {
            case e_inedge:
                c->bb->addInEdge(stack.front()->bb);
                break;
            case e_outedge:
                c->bb->addOutEdge(stack.front()->bb);
                break;
        }
        return;
    }
    switch(e) {
        case e_inedge:
            break;
        case e_outedge:
            break;
        case e_livein:
            c->bb->addLiveIn((Location*)stack.front()->exp);
            break;
        case e_rtl:
            c->bb->addRTL(stack.front()->rtl);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context bb\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context bb\n";
            break;
    }
}

void XMLProgParser::start_inedge(const char **attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock*)findId(getAttr(attr, "bb"));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_inedge(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context inedge\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context inedge\n";
    //    break;
    //      }
}

void XMLProgParser::start_outedge(const char **attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock*)findId(getAttr(attr, "bb"));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_outedge(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context outedge\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context outedge\n";
    //    break;
    //      }
}

void XMLProgParser::start_livein(const char **/*attr*/) { }

void XMLProgParser::addToContext_livein(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_order(const char **attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock *)findId(getAttr(attr, "bb"));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_order(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context order\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context order\n";
    //    break;
    //      }
}

void XMLProgParser::start_revorder(const char **attr) {
    if (phase == 1)
        stack.front()->bb = (BasicBlock *)findId(getAttr(attr, "bb"));
    else
        stack.front()->bb = nullptr;
}

void XMLProgParser::addToContext_revorder(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context revOrder\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context order\n";
    //    break;
    //      }
}

void XMLProgParser::start_rtl(const char **attr) {
    if (phase == 1) {
        stack.front()->rtl = (RTL*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->rtl = new RTL();
    addId(attr, stack.front()->rtl);
    const char *a = getAttr(attr, "addr");
    if (a)
        stack.front()->rtl->nativeAddr = ADDRESS::g(atoi(a));
}

void XMLProgParser::addToContext_rtl(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_stmt:
            c->rtl->appendStmt(stack.front()->stmt);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context rtl\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context rtl\n";
            break;
    }
}

void XMLProgParser::start_stmt(const char **/*attr*/) {
}

void XMLProgParser::addToContext_stmt(Context *c, int /*e*/) {
    c->stmt = stack.front()->stmt;
}

void XMLProgParser::start_assign(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            stack.front()->stmt->setProc(p);
//        Statement *parent = (Statement*)findId(getAttr(attr, "parent"));
//        if (parent)
//            stack.front()->stmt->parent = parent;
        return;
    }
    stack.front()->stmt = new Assign();
    addId(attr, stack.front()->stmt);
    const char *n = getAttr(attr, "number");
    if (n)
        stack.front()->stmt->number = atoi(n);
}

void XMLProgParser::start_assignment(const char **/*attr*/) {
}

void XMLProgParser::start_phiassign(const char **/*attr*/) {
    // FIXME: TBC
}

void XMLProgParser::addToContext_assign(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    Assign *assign = dynamic_cast<Assign*>(c->stmt);
    assert(assign);
    switch(e) {
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
                std::cerr << "unknown tag " << e << " in context assign\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context assign\n";
            break;
    }
}

void XMLProgParser::start_callstmt(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Statement*)stack.front()->stmt)->setProc(p);
//        Statement *s = (Statement*)findId(getAttr(attr, "parent"));
//        if (s)
//            ((Statement*)stack.front()->stmt)->parent = s;
        return;
    }
    CallStatement *call = new CallStatement();
    stack.front()->stmt = call;
    addId(attr, call);
    const char *n = getAttr(attr, "number");
    if (n)
        call->number = atoi(n);
    n = getAttr(attr, "computed");
    if (n)
        call->m_isComputed = atoi(n) > 0;
    n = getAttr(attr, "returnAftercall");
    if (n)
        call->returnAfterCall = atoi(n) > 0;
}

void XMLProgParser::addToContext_callstmt(Context *c, int e) {
    CallStatement *call = dynamic_cast<CallStatement*>(c->stmt);
    assert(call);
    if (phase == 1) {
        switch(e) {
            case e_dest:
                if (stack.front()->proc)
                    call->setDestProc(stack.front()->proc);
                break;
        }
        return;
    }
    // TODO: analyze the code to understand the intended use of returnExp
    //Exp* returnExp = nullptr;
    switch(e) {
        case e_dest:
            call->setDest(stack.front()->exp);
            break;
        case e_argument:
            call->appendArgument((Assignment*)stack.front()->stmt);
            break;
        case e_returnexp:
            // Assume that the corresponding return type will appear next
            //returnExp = stack.front()->exp;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context callstmt\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context callstmt\n";
            break;
    }
}

void XMLProgParser::start_dest(const char **attr) {
    if (phase == 1) {
        Proc *p = (Proc*)findId(getAttr(attr, "proc"));
        if (p)
            stack.front()->proc = p;
        return;
    }
}

void XMLProgParser::addToContext_dest(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_returnstmt(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Statement*)stack.front()->stmt)->setProc(p);
//      Statement *s = (Statement*)findId(getAttr(attr, "parent"));
//        if (s)
//            ((Statement*)stack.front()->stmt)->parent = s;
        return;
    }
    ReturnStatement *ret = new ReturnStatement();
    stack.front()->stmt = ret;
    addId(attr, ret);
    const char *n = getAttr(attr, "number");
    if (n)
        ret->number = atoi(n);
    n = getAttr(attr, "retAddr");
    if (n)
        ret->retAddr = ADDRESS::g(atoi(n));
}

void XMLProgParser::addToContext_returnstmt(Context *c, int e) {
    ReturnStatement *ret = dynamic_cast<ReturnStatement*>(c->stmt);
    assert(ret);
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_modifieds:
            ret->modifieds.append((Assignment*)stack.front()->stmt);
            break;
        case e_returns:
            ret->returns.append((Assignment*)stack.front()->stmt);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context returnstmt\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context returnstmt\n";
            break;
    }
}

void XMLProgParser::start_returns(const char **/*attr*/) {
}

void XMLProgParser::addToContext_returns(Context */*c*/, int /*e*/) {
}

void XMLProgParser::start_modifieds(const char **/*attr*/) {
}

void XMLProgParser::addToContext_modifieds(Context */*c*/, int /*e*/) {
}

void XMLProgParser::start_gotostmt(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Statement*)stack.front()->stmt)->setProc(p);
//        Statement *s = (Statement*)findId(getAttr(attr, "parent"));
//        if (s)
//            ((Statement*)stack.front()->stmt)->parent = s;
        return;
    }
    GotoStatement *branch = new GotoStatement();
    stack.front()->stmt = branch;
    addId(attr, branch);
    const char *n = getAttr(attr, "number");
    if (n)
        branch->number = atoi(n);
    n = getAttr(attr, "computed");
    if (n)
        branch->m_isComputed = atoi(n) > 0;
}

void XMLProgParser::addToContext_gotostmt(Context *c, int e) {
    GotoStatement *branch = dynamic_cast<GotoStatement*>(c->stmt);
    assert(branch);
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_dest:
            branch->setDest(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context gotostmt\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context gotostmt\n";
            break;
    }
}

void XMLProgParser::start_branchstmt(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Statement*)stack.front()->stmt)->setProc(p);
        return;
    }
    BranchStatement *branch = new BranchStatement();
    stack.front()->stmt = branch;
    addId(attr, branch);
    const char *n = getAttr(attr, "number");
    if (n)
        branch->number = atoi(n);
    n = getAttr(attr, "computed");
    if (n)
        branch->m_isComputed = atoi(n) > 0;
    n = getAttr(attr, "jtcond");
    if (n)
        branch->jtCond = (BRANCH_TYPE)atoi(n);
    n = getAttr(attr, "float");
    if (n)
        branch->bFloat = atoi(n) > 0;
}

void XMLProgParser::addToContext_branchstmt(Context *c, int e) {
    BranchStatement *branch = dynamic_cast<BranchStatement*>(c->stmt);
    assert(branch);
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_cond:
            branch->setCondExpr(stack.front()->exp);
            break;
        case e_dest:
            branch->setDest(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context branchstmt\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context branchstmt\n";
            break;
    }
}

void XMLProgParser::start_casestmt(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Statement*)stack.front()->stmt)->setProc(p);
        return;
    }
    CaseStatement *cas = new CaseStatement();
    stack.front()->stmt = cas;
    addId(attr, cas);
    const char *n = getAttr(attr, "number");
    if (n)
        cas->number = atoi(n);
    n = getAttr(attr, "computed");
    if (n)
        cas->m_isComputed = atoi(n) > 0;
}

void XMLProgParser::addToContext_casestmt(Context *c, int e) {
    CaseStatement *cas = dynamic_cast<CaseStatement*>(c->stmt);
    assert(cas);
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_dest:
            cas->setDest(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context casestmt\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context casestmt\n";
            break;
    }
}

void XMLProgParser::start_boolasgn(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Statement*)stack.front()->stmt)->setProc(p);
        return;
    }
    const char *n = getAttr(attr, "size");
    assert(n);
    BoolAssign *boo = new BoolAssign(atoi(n));
    stack.front()->stmt = boo;
    addId(attr, boo);
    n = getAttr(attr, "number");
    if (n)
        boo->number = atoi(n);
    n = getAttr(attr, "jtcond");
    if (n)
        boo->jtCond = (BRANCH_TYPE)atoi(n);
    n = getAttr(attr, "float");
    if (n)
        boo->bFloat = atoi(n) > 0;
}

void XMLProgParser::addToContext_boolasgn(Context *c, int e) {
    BoolAssign *boo = dynamic_cast<BoolAssign*>(c->stmt);
    assert(boo);
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_cond:
            boo->pCond = stack.front()->exp;
            break;
        case e_lhs:
            boo->lhs = stack.front()->exp;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context boolasgn\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context boolasgn\n";
            break;
    }
}

void XMLProgParser::start_type(const char **/*attr*/) {
}

void XMLProgParser::addToContext_type(Context *c, int /*e*/) {
    c->type = stack.front()->type;
}

void XMLProgParser::start_basetype(const char **/*attr*/) {
}

void XMLProgParser::addToContext_basetype(Context *c, int /*e*/) {
    c->type = stack.front()->type;
}

void XMLProgParser::start_sizetype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    SizeType *ty = new SizeType();
    stack.front()->type = ty;
    addId(attr, ty);
    const char *n = getAttr(attr, "size");
    if (n)
        ty->size = atoi(n);
}

void XMLProgParser::addToContext_sizetype(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context SizeType\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context SizeType\n";
    //    break;
    //      }
}

void XMLProgParser::start_exp(const char **/*attr*/) {
}

void XMLProgParser::addToContext_exp(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_secondexp(const char **/*attr*/) {
}

void XMLProgParser::addToContext_secondexp(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_defines(const char **/*attr*/) {
}

void XMLProgParser::addToContext_defines(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_lhs(const char **/*attr*/) {
}

void XMLProgParser::addToContext_lhs(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_rhs(const char **/*attr*/) {
}

void XMLProgParser::addToContext_rhs(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_argument(const char **/*attr*/) {
}

void XMLProgParser::addToContext_argument(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_returnexp(const char **/*attr*/) {
}

void XMLProgParser::start_returntype(const char **/*attr*/) {
}

void XMLProgParser::addToContext_returnexp(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::addToContext_returntype(Context *c, int /*e*/) {
    c->type = stack.front()->type;
}

void XMLProgParser::start_cond(const char **/*attr*/) {
}

void XMLProgParser::addToContext_cond(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_subexp1(const char **/*attr*/) {
}

void XMLProgParser::addToContext_subexp1(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_subexp2(const char **/*attr*/) {
}

void XMLProgParser::addToContext_subexp2(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_subexp3(const char **/*attr*/) {
}

void XMLProgParser::addToContext_subexp3(Context *c, int /*e*/) {
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_voidtype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->type = new VoidType();
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_voidtype(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context voidType\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context voidType\n";
    //    break;
    //      }
}

void XMLProgParser::start_integertype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    IntegerType *ty = IntegerType::get(STD_SIZE);
    stack.front()->type = ty;
    addId(attr, ty);
    const char *n = getAttr(attr, "size");
    if (n)
        ty->size = atoi(n);
    n = getAttr(attr, "signedness");
    if (n)
        ty->signedness = atoi(n);
}

void XMLProgParser::addToContext_integertype(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context integerType\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context integerType\n";
    //    break;
    //      }
}

void XMLProgParser::start_pointertype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->type = new PointerType(nullptr);
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_pointertype(Context *c, int /*e*/) {
    PointerType *p = dynamic_cast<PointerType*>(c->type);
    assert(p);
    if (phase == 1) {
        return;
    }
    p->setPointsTo(stack.front()->type);
}

void XMLProgParser::start_chartype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->type = new CharType();
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_chartype(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context charType\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context charType\n";
    //    break;
    //      }
}

void XMLProgParser::start_namedtype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->type = new NamedType(getAttr(attr, "name"));
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_namedtype(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context namedType\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context namedType\n";
    //    break;
    //      }
}

void XMLProgParser::start_arraytype(const char **attr) {
    if (phase == 1) {
        stack.front()->type = (Type*)findId(getAttr(attr, "id"));
        return;
    }
    ArrayType *a = new ArrayType();
    stack.front()->type = a;
    addId(attr, a);
    const char *len = getAttr(attr, "length");
    if (len)
        a->length = atoi(len);
}

void XMLProgParser::addToContext_arraytype(Context *c, int e) {
    ArrayType *a = dynamic_cast<ArrayType*>(c->type);
    assert(a);
    switch(e) {
        case e_basetype:
            a->base_type = stack.front()->type;
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context arrayType\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context arrayType\n";
            break;
    }
}

void XMLProgParser::start_location(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
        if (p)
            ((Location*)stack.front()->exp)->setProc(p);
        return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Location(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_location(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    Location *l = dynamic_cast<Location*>(c->exp);
    assert(l);
    switch(e) {
        case e_subexp1:
            c->exp->setSubExp1(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context unary\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context location\n";
            break;
    }
}

void XMLProgParser::start_unary(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Unary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_unary(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_subexp1:
            c->exp->setSubExp1(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context unary\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context unary\n";
            break;
    }
}

void XMLProgParser::start_binary(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Binary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_binary(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
        case e_subexp1:
            c->exp->setSubExp1(stack.front()->exp);
            break;
        case e_subexp2:
            c->exp->setSubExp2(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context binary\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context binary\n";
            break;
    }
}

void XMLProgParser::start_ternary(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Ternary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_ternary(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    switch(e) {
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
                std::cerr << "unknown tag " << e << " in context ternary\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context ternary\n";
            break;
    }
}

void XMLProgParser::start_const(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        return;
    }
    double d;
    const char *value = getAttr(attr, "value");
    const char *opstring = getAttr(attr, "op");
    assert(value);
    assert(opstring);
    //std::cerr << "got value=" << value << " opstring=" << opstring << "\n";
    OPER op = (OPER)operFromString(opstring);
    assert(op != -1);
    switch(op) {
        case opIntConst:
            stack.front()->exp = new Const(atoi(value));
            addId(attr, stack.front()->exp);
            break;
        case opStrConst:
            stack.front()->exp = new Const(strdup(value));
            addId(attr, stack.front()->exp);
            break;
        case opFltConst:
            sscanf(value, "%lf", &d);
            stack.front()->exp = new Const(d);
            addId(attr, stack.front()->exp);
            break;
        default:
            LOG << "unknown Const op " << op << "\n";
            assert(false);
    }
    //std::cerr << "end of start const\n";
}

void XMLProgParser::addToContext_const(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context const\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context const\n";
    //    break;
    //      }
}

void XMLProgParser::start_terminal(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Terminal(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_terminal(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context terminal\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context terminal\n";
    //    break;
    //      }
}

void XMLProgParser::start_typedexp(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        return;
    }
    stack.front()->exp = new TypedExp();
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_typedexp(Context *c, int e) {
    TypedExp *t = dynamic_cast<TypedExp*>(c->exp);
    assert(t);
    switch(e) {
        case e_type:
            t->type = stack.front()->type;
            break;
        case e_subexp1:
            c->exp->setSubExp1(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context typedexp\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context typedexp\n";
            break;
    }
}

void XMLProgParser::start_refexp(const char **attr) {
    if (phase == 1) {
        stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
        RefExp *r = dynamic_cast<RefExp*>(stack.front()->exp);
        assert(r);
        r->def = (Statement*)findId(getAttr(attr, "def"));
        return;
    }
    stack.front()->exp = new RefExp();
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_refexp(Context *c, int e) {
    switch(e) {
        case e_subexp1:
            c->exp->setSubExp1(stack.front()->exp);
            break;
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context refexp\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context refexp\n";
            break;
    }
}

void XMLProgParser::start_def(const char **attr) {
    if (phase == 1) {
        stack.front()->stmt = (Statement*)findId(getAttr(attr, "stmt"));
        return;
    }
}

void XMLProgParser::addToContext_def(Context */*c*/, int e) {
    //      switch(e) {
    //    default:
    if (e == e_unknown)
        std::cerr << "unknown tag " << e << " in context def\n";
    else
        std::cerr << "need to handle tag " << tags[e].tag << " in context def\n";
    //        break;
    //      }
}

Prog *XMLProgParser::parse(const std::string &filename) {
    FILE *f = fopen(filename.c_str(), "r");
    if (f == nullptr)
        return nullptr;
    fclose(f);

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
    //FrontEnd *pFE = FrontEnd::Load(prog->getPath(), prog);        // Path is usually empty!?
    FrontEnd *pFE = FrontEnd::Load(prog->getPathAndName(), prog);
    prog->setFrontEnd(pFE);
    return prog;
}

void XMLProgParser::parseFile(const std::string &filename) {
    FILE *f = fopen(filename.c_str(), "r");
    if (f == nullptr)
        return;
    XML_Parser p = XML_ParserCreate(nullptr);
    if (! p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        return;
    }

    XML_SetUserData(p, this);
    XML_SetElementHandler(p, start, end);
    XML_SetCharacterDataHandler(p, text);

    char Buff[8192];

    for (;;) {
        int done;
        int len;

        len = fread(Buff, 1, sizeof(Buff), f);
        if (ferror(f)) {
            fprintf(stderr, "Read error\n");
            fclose(f);
            return;
        }
        done = feof(f);

        if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
            if (XML_GetErrorCode(p) != XML_ERROR_NO_ELEMENTS)
                fprintf(stderr, "Parse error at line %ld of file %s:\n%s\n", XML_GetCurrentLineNumber(p), filename.c_str(),
                        XML_ErrorString(XML_GetErrorCode(p)));
            fclose(f);
            return;
        }

        if (done)
            break;
    }
    fclose(f);
}

void XMLProgParser::parseChildren(Cluster *c) {
    std::string path = c->makeDirs();
    for (auto & elem : c->children) {
        std::string d = path + "/" + elem->getName() + ".xml";
        parseFile(d.c_str());
        parseChildren(elem);
    }
}

extern char* operStrings[];

int XMLProgParser::operFromString(const char *s) {
    for (int i = 0; i < opNumOf; i++)
        if (!strcmp(s, operStrings[i]))
            return i;
    return -1;
}


void XMLProgParser::persistToXML(std::ostream &out, Cluster *c) {
    out << "<cluster id=\"" << ADDRESS::host_ptr(c) << "\" name=\"" << c->name << "\"";
    out << ">\n";
    for (auto & elem : c->children) {
        persistToXML(out, elem);
    }
    out << "</cluster>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Global *g) {
    out << "<global name=\"" << g->nam << "\" uaddr=\"" << g->uaddr << "\">\n";
    out << "<type>\n";
    persistToXML(out, g->type);
    out << "</type>\n";
    out << "</global>\n";
}

void XMLProgParser::persistToXML(Prog *prog) {
    prog->m_rootCluster->openStreams("xml");
    std::ofstream &os = prog->m_rootCluster->getStream();
    os << "<prog path=\"" << prog->getPath() << "\" name=\"" << prog->getName() << "\" iNumberedProc=\"" <<
          prog->m_iNumberedProc << "\">\n";
    for (auto const & elem : prog->globals)
        persistToXML(os, elem);
    persistToXML(os, prog->m_rootCluster);
    for (auto p : prog->m_procs) {

        persistToXML(p->getCluster()->getStream(), p);
    }
    os << "</prog>\n";
    os.close();
    prog->m_rootCluster->closeStreams();
}

void XMLProgParser::persistToXML(std::ostream &out, LibProc *proc) {
    out << "<libproc id=\"" << ADDRESS::host_ptr(proc) << "\" address=\"" << proc->address << "\"";
    out << " firstCallerAddress=\"" << proc->m_firstCallerAddr << "\"";
    if (proc->m_firstCaller)
        out << " firstCaller=\"" << ADDRESS::host_ptr(proc->m_firstCaller) << "\"";
    if (proc->cluster)
        out << " cluster=\"" << ADDRESS::host_ptr(proc->cluster) << "\"";
    out << ">\n";

    persistToXML(out, proc->signature);

    for (auto const & elem : proc->callerSet)
        out << "<caller call=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";
    for (auto & elem : proc->provenTrue) {
        out << "<proven_true>\n";
        persistToXML(out, elem.first);
        persistToXML(out, elem.second);
        out << "</proven_true>\n";
    }
    out << "</libproc>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, UserProc *proc) {
    out << "<userproc id=\"" << ADDRESS::host_ptr(proc) << "\"";
    out << " address=\"" << proc->address << "\"";
    out << " status=\"" << (int)proc->status << "\"";
    out << " firstCallerAddress=\"" << proc->m_firstCallerAddr << "\"";
    if (proc->m_firstCaller)
        out << " firstCaller=\"" << ADDRESS::host_ptr(proc->m_firstCaller) << "\"";
    if (proc->cluster)
        out << " cluster=\"" << ADDRESS::host_ptr(proc->cluster) << "\"";
    if (proc->theReturnStatement)
        out << " retstmt=\"" << ADDRESS::host_ptr(proc->theReturnStatement) << "\"";
    out << ">\n";

    persistToXML(out, proc->signature);

    for (auto const & elem : proc->callerSet)
        out << "<caller call=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";
    for (auto & elem : proc->provenTrue) {
        out << "<proven_true>\n";
        persistToXML(out, elem.first);
        persistToXML(out, elem.second);
        out << "</proven_true>\n";
    }

    for (auto & elem : proc->locals) {
        out << "<local name=\"" << (elem).first << "\">\n";
        out << "<type>\n";
        persistToXML(out, (elem).second);
        out << "</type>\n";
        out << "</local>\n";
    }

    for (auto & elem : proc->symbolMap) {
        out << "<symbol>\n";
        out << "<exp>\n";
        persistToXML(out, (elem).first);
        out << "</exp>\n";
        out << "<secondexp>\n";
        persistToXML(out, (elem).second);
        out << "</secondexp>\n";
        out << "</symbol>\n";
    }


    for (auto & elem : proc->calleeList)
        out << "<callee proc=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";

    persistToXML(out, proc->cfg);

    out << "</userproc>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Proc *proc) {
    if (proc->isLib())
        persistToXML(out, (LibProc*)proc);
    else
        persistToXML(out, (UserProc*)proc);
}

void XMLProgParser::persistToXML(std::ostream &out, Signature *sig) {
    out << "<signature id=\"" << ADDRESS::host_ptr(sig) << "\"";
    out << " name=\"" << sig->name.toStdString() << "\"";
    out << " ellipsis=\"" << (int)sig->ellipsis << "\"";
    out << " preferedName=\"" << sig->preferedName << "\"";
    if (sig->getPlatform() != PLAT_GENERIC)
        out << " platform=\"" << sig->platformName(sig->getPlatform()) << "\"";
    if (sig->getConvention() != CONV_NONE)
        out << " convention=\"" << sig->conventionName(sig->getConvention()) << "\"";
    out << ">\n";
    for (auto & elem : sig->params) {
        out << "<param id=\"" << ADDRESS::host_ptr(elem) << "\" name=\"" << elem->name() << "\">\n";
        out << "<type>\n";
        persistToXML(out, elem->getType());
        out << "</type>\n";
        out << "<exp>\n";
        persistToXML(out, elem->getExp());
        out << "</exp>\n";
        out << "</param>\n";
    }
    for (auto & elem : sig->returns) {
        out << "<return>\n";
        out << "<type>\n";
        persistToXML(out, (elem)->type);
        out << "</type>\n";
        out << "<exp>\n";
        persistToXML(out, (elem)->exp);
        out << "</exp>\n";
        out << "</return>\n";
    }
    if (sig->rettype) {
        out << "<rettype>\n";
        persistToXML(out, sig->rettype);
        out << "</rettype>\n";
    }
    if (sig->preferedReturn) {
        out << "<prefreturn>\n";
        persistToXML(out, sig->preferedReturn);
        out << "</prefreturn>\n";
    }
    for (auto & elem : sig->preferedParams)
        out << "<prefparam index=\"" << elem << "\"/>\n";
    out << "</signature>\n";
}


void XMLProgParser::persistToXML(std::ostream &out, const Type *ty) {
    const VoidType *v = dynamic_cast<const VoidType*>(ty);
    if (v) {
        out << "<voidtype id=\"" << ADDRESS::host_ptr(ty) << "\"/>\n";
        return;
    }
    const FuncType *f = dynamic_cast<const FuncType*>(ty);
    if (f) {
        out << "<functype id=\"" << ADDRESS::host_ptr(ty) << "\">\n";
        persistToXML(out, f->signature);
        out << "</functype>\n";
        return;
    }
    const IntegerType *i = dynamic_cast<const IntegerType*>(ty);
    if (i) {
        out << "<integertype id=\"" << ADDRESS::host_ptr(ty) << "\" size=\"" << i->size << "\" signedness=\"" << i->signedness <<
               "\"/>\n";
        return;
    }
    const FloatType *fl = dynamic_cast<const FloatType*>(ty);
    if (fl) {
        out << "<floattype id=\"" << ADDRESS::host_ptr(ty) << "\" size=\"" << fl->size << "\"/>\n";
        return;
    }
    const BooleanType *b = dynamic_cast<const BooleanType*>(ty);
    if (b) {
        out << "<booleantype id=\"" << ADDRESS::host_ptr(ty) << "\"/>\n";
        return;
    }
    const CharType *c = dynamic_cast<const CharType*>(ty);
    if (c) {
        out << "<chartype id=\"" << ADDRESS::host_ptr(ty) << "\"/>\n";
        return;
    }
    const PointerType *p = dynamic_cast<const PointerType*>(ty);
    if (p) {
        out << "<pointertype id=\"" << ADDRESS::host_ptr(ty) << "\">\n";
        persistToXML(out, p->points_to);
        out << "</pointertype>\n";
        return;
    }
    const ArrayType *a = dynamic_cast<const ArrayType*>(ty);
    if (a) {
        out << "<arraytype id=\"" << ADDRESS::host_ptr(ty) << "\" length=\"" << (int)a->length << "\">\n";
        out << "<basetype>\n";
        persistToXML(out, a->base_type);
        out << "</basetype>\n";
        out << "</arraytype>\n";
        return;
    }
    const NamedType *n = dynamic_cast<const NamedType*>(ty);
    if (n) {
        out << "<namedtype id=\"" << ADDRESS::host_ptr(ty) << "\" name=\"" << n->name << "\"/>\n";
        return;
    }
    const CompoundType *co = dynamic_cast<const CompoundType*>(ty);
    if (co) {
        out << "<compoundtype id=\"" << ADDRESS::host_ptr(ty) << "\">\n";
        for (unsigned i = 0; i < co->names.size(); i++) {
            out << "<member name=\"" << co->names[i] << "\">\n";
            persistToXML(out, co->types[i]);
            out << "</member>\n";
        }
        out << "</compoundtype>\n";
        return;
    }
    const SizeType *sz = dynamic_cast<const SizeType*>(ty);
    if (sz) {
        out << "<sizetype id=\"" << ADDRESS::host_ptr(ty) << "\" size=\"" << sz->getSize() << "\"/>\n";
        return;
    }
    std::cerr << "unknown type in persistToXML\n";
    assert(false);
}

void XMLProgParser::persistToXML(std::ostream &out, const Exp *e) {
    const TypeVal *t = dynamic_cast<const TypeVal*>(e);
    const char *op_name=e->getOperName();
    if (t) {
        out << "<typeval id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\">\n";
        out << "<type>\n";
        persistToXML(out, t->val);
        out << "</type>\n";
        out << "</typeval>\n";
        return;
    }
    const Terminal *te = dynamic_cast<const Terminal*>(e);
    if (te) {
        out << "<terminal id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\"/>\n";
        return;
    }
    const Const *c = dynamic_cast<const Const*>(e);
    if (c) {
        out << "<const id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\"";
        out << " conscript=\"" << c->conscript << "\"";
        if (c->op == opIntConst)
            out << " value=\"" << c->u.i << "\"";
        else if (c->op == opFuncConst)
            out << " value=\"" << c->u.a << "\"";
        else if (c->op == opFltConst)
            out << " value=\"" << c->u.d << "\"";
        else if (c->op == opStrConst)
            out << " value=\"" << c->u.p << "\"";
        else {
            // TODO
            // QWord ll;
            // Proc* pp;
            assert(false);
        }
        out << "/>\n";
        return;
    }
    const Location *l = dynamic_cast<const Location*>(e);
    if (l) {
        out << "<location id=\"" << ADDRESS::host_ptr(e) << "\"";
        if (l->proc)
            out << " proc=\"" << ADDRESS::host_ptr(l->proc) << "\"";
        out << " op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, l->subExp1);
        out << "</subexp1>\n";
        out << "</location>\n";
        return;
    }
    const RefExp *r = dynamic_cast<const RefExp*>(e);
    if (r) {
        out << "<refexp id=\"" << ADDRESS::host_ptr(e) << "\"";
        if (r->def)
            out << " def=\"" << ADDRESS::host_ptr(r->def) << "\"";
        out << " op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, r->subExp1);
        out << "</subexp1>\n";
        out << "</refexp>\n";
        return;
    }
    const FlagDef *f = dynamic_cast<const FlagDef*>(e);
    if (f) {
        out << "<flagdef id=\"" << ADDRESS::host_ptr(e) << "\"";
        if (f->rtl)
            out << " rtl=\"" << ADDRESS::host_ptr(f->rtl) << "\"";
        out << " op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, f->subExp1);
        out << "</subexp1>\n";
        out << "</flagdef>\n";
        return;
    }
    const TypedExp *ty = dynamic_cast<const TypedExp*>(e);
    if (ty) {
        out << "<typedexp id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, ty->subExp1);
        out << "</subexp1>\n";
        out << "<type>\n";
        persistToXML(out, ty->type);
        out << "</type>\n";
        out << "</typedexp>\n";
        return;
    }
    const Ternary *tn = dynamic_cast<const Ternary*>(e);
    if (tn) {
        out << "<ternary id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, tn->subExp1);
        out << "</subexp1>\n";
        out << "<subexp2>\n";
        persistToXML(out, tn->subExp2);
        out << "</subexp2>\n";
        out << "<subexp3>\n";
        persistToXML(out, tn->subExp3);
        out << "</subexp3>\n";
        out << "</ternary>\n";
        return;
    }
    const Binary *b = dynamic_cast<const Binary*>(e);
    if (b) {
        out << "<binary id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, b->subExp1);
        out << "</subexp1>\n";
        out << "<subexp2>\n";
        persistToXML(out, b->subExp2);
        out << "</subexp2>\n";
        out << "</binary>\n";
        return;
    }
    const Unary *u = dynamic_cast<const Unary*>(e);
    if (u) {
        out << "<unary id=\"" << ADDRESS::host_ptr(e) << "\" op=\"" << op_name << "\">\n";
        out << "<subexp1>\n";
        persistToXML(out, u->subExp1);
        out << "</subexp1>\n";
        out << "</unary>\n";
        return;
    }
    std::cerr << "unknown exp in persistToXML\n";
    assert(false);
}

void XMLProgParser::persistToXML(std::ostream &out, Cfg *cfg) {
    out << "<cfg id=\"" << ADDRESS::host_ptr(cfg) << "\" wellformed=\"" << (int)cfg->m_bWellFormed << "\" lastLabel=\"" <<
           cfg->lastLabel << "\"";
    out << " entryBB=\"" << ADDRESS::host_ptr(cfg->entryBB) << "\"";
    out << " exitBB=\"" << ADDRESS::host_ptr(cfg->exitBB) << "\"";
    out << ">\n";

    for (auto & elem : cfg->m_listBB)
        persistToXML(out, elem);

    for (auto & elem : cfg->Ordering)
        out << "<order bb=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";

    for (auto & elem : cfg->revOrdering)
        out << "<revorder bb=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";

    // TODO
    // MAPBB m_mapBB;
    // std::set<CallStatement*> callSites;
    // std::vector<PBB> BBs;               // Pointers to BBs from indices
    // std::map<PBB, int> indices;           // Indices from pointers to BBs
    // more
    out << "</cfg>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, const BasicBlock *bb) {
    out << "<bb id=\"" << ADDRESS::host_ptr(bb) << "\" nodeType=\"" << bb->m_nodeType << "\" labelNum=\""
        << bb->m_iLabelNum << "\" label=\"" << bb->m_labelStr << "\" labelneeded=\"" << (int)bb->m_labelneeded
        << "\"" << " incomplete=\"" << (int)bb->m_bIncomplete << "\" jumpreqd=\"" << (int)bb->m_bJumpReqd
        << "\" m_traversed=\"" << bb->m_iTraversed << "\"";
    out << " DFTfirst=\"" << bb->m_DFTfirst << "\" DFTlast=\"" << bb->m_DFTlast << "\" DFTrevfirst=\"" <<
           bb->m_DFTrevfirst << "\" DFTrevlast=\"" << bb->m_DFTrevlast << "\"";
    out << " structType=\"" << bb->m_structType << "\"";
    out << " loopCondType=\"" << bb->m_loopCondType << "\"";
    if (bb->m_loopHead)
        out << " m_loopHead=\"" << ADDRESS::host_ptr(bb->m_loopHead) << "\"";
    if (bb->m_caseHead)
        out << " m_caseHead=\"" << ADDRESS::host_ptr(bb->m_caseHead) << "\"";
    if (bb->m_condFollow)
        out << " m_condFollow=\"" << ADDRESS::host_ptr(bb->m_condFollow) << "\"";
    if (bb->m_loopFollow)
        out << " m_loopFollow=\"" << ADDRESS::host_ptr(bb->m_loopFollow) << "\"";
    if (bb->m_latchNode)
        out << " m_latchNode=\"" << ADDRESS::host_ptr(bb->m_latchNode) << "\"";
    out << " ord=\"" << bb->ord << "\"";
    out << " revOrd=\"" << bb->revOrd << "\"";
    out << " inEdgesVisited=\"" << bb->inEdgesVisited << "\"";
    out << " numForwardInEdges=\"" << bb->numForwardInEdges << "\"";
    out << " loopStamp1=\"" << bb->loopStamps[0] << "\"";
    out << " loopStamp2=\"" << bb->loopStamps[1] << "\"";
    out << " revLoopStamp1=\"" << bb->revLoopStamps[0] << "\"";
    out << " revLoopStamp2=\"" << bb->revLoopStamps[1] << "\"";
    out << " traversed=\"" << (int)bb->traversed << "\"";
    out << " hllLabel=\"" << (int)bb->hllLabel << "\"";
    if (bb->labelStr)
        out << " labelStr=\"" << bb->labelStr << "\"";
    out << " indentLevel=\"" << bb->indentLevel << "\"";
    // note the rediculous duplication here
    if (bb->immPDom)
        out << " immPDom=\"" << ADDRESS::host_ptr(bb->immPDom) << "\"";
    if (bb->loopHead)
        out << " loopHead=\"" << ADDRESS::host_ptr(bb->loopHead) << "\"";
    if (bb->caseHead)
        out << " caseHead=\"" << ADDRESS::host_ptr(bb->caseHead) << "\"";
    if (bb->condFollow)
        out << " condFollow=\"" << ADDRESS::host_ptr(bb->condFollow) << "\"";
    if (bb->loopFollow)
        out << " loopFollow=\"" << ADDRESS::host_ptr(bb->loopFollow) << "\"";
    if (bb->latchNode)
        out << " latchNode=\"" << ADDRESS::host_ptr(bb->latchNode) << "\"";
    out << " sType=\"" << (int)bb->sType << "\"";
    out << " usType=\"" << (int)bb->usType << "\"";
    out << " lType=\"" << (int)bb->lType << "\"";
    out << " cType=\"" << (int)bb->cType << "\"";
    out << ">\n";

    for (auto & elem : bb->m_InEdges)
        out << "<inedge bb=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";
    for (auto & elem : bb->m_OutEdges)
        out << "<outedge bb=\"" << ADDRESS::host_ptr(elem) << "\"/>\n";

    LocationSet::iterator it;
    for (it = bb->liveIn.begin(); it != bb->liveIn.end(); it++) {
        out << "<livein>\n";
        persistToXML(out, *it);
        out << "</livein>\n";
    }

    if (bb->m_pRtls) {
        for (auto & elem : *bb->m_pRtls)
            persistToXML(out, elem);
    }
    out << "</bb>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, const RTL *rtl) {
    out << "<rtl id=\"" << ADDRESS::host_ptr(rtl) << "\" addr=\"" << rtl->nativeAddr << "\">\n";
    for (auto const & elem : *rtl) {
        out << "<stmt>\n";
        persistToXML(out, elem);
        out << "</stmt>\n";
    }
    out << "</rtl>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, const Statement *stmt) {
    const BoolAssign *b = dynamic_cast<const BoolAssign*>(stmt);
    if (b) {
        out << "<boolasgn id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << b->number << "\"";
//        if (b->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(b->parent) << "\"";
        if (b->proc)
            out << " proc=\"" << ADDRESS::host_ptr(b->proc) << "\"";
        out << " jtcond=\"" << b->jtCond << "\"";
        out << " float=\"" << (int)b->bFloat << "\"";
        out << " size=\"" << b->size << "\"";
        out << ">\n";
        if (b->pCond) {
            out << "<cond>\n";
            persistToXML(out, b->pCond);
            out << "</cond>\n";
        }
        out << "</boolasgn>\n";
        return;
    }
    const ReturnStatement *r = dynamic_cast<const ReturnStatement*>(stmt);
    if (r) {
        out << "<returnstmt id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << r->number << "\"";
//        if (r->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(r->parent) << "\"";
        if (r->proc)
            out << " proc=\"" << ADDRESS::host_ptr(r->proc) << "\"";
        out << " retAddr=\"" << r->retAddr << "\"";
        out << ">\n";

        for (auto const & elem : r->modifieds) {
            out << "<modifieds>\n";
            persistToXML(out, elem);
            out << "</modifieds>\n";
        }
        for (auto const & elem : r->returns) {
            out << "<returns>\n";
            persistToXML(out, elem);
            out << "</returns>\n";
        }

        out << "</returnstmt>\n";
        return;
    }
    const CallStatement *c = dynamic_cast<const CallStatement*>(stmt);
    if (c) {
        out << "<callstmt id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << c->number
            << "\" computed=\"" << (int)c->m_isComputed << "\"";
//        if (c->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(c->parent) << "\"";
        if (c->proc)
            out << " proc=\"" << ADDRESS::host_ptr(c->proc) << "\"";
        out << " returnAfterCall=\"" << (int)c->returnAfterCall << "\"";
        out << ">\n";

        if (c->pDest) {
            out << "<dest";
            if (c->procDest)
                out << " proc=\"" << ADDRESS::host_ptr(c->procDest) << "\"";
            out << ">\n";
            persistToXML(out, c->pDest);
            out << "</dest>\n";
        }

        for (auto const & elem : c->arguments) {
            out << "<argument>\n";
            persistToXML(out, elem);
            out << "</argument>\n";
        }

        for (auto const & elem : c->defines) {
            out << "<defines>\n";
            persistToXML(out, elem);
            out << "</defines>\n";
        }

        out << "</callstmt>\n";
        return;
    }
    const CaseStatement *ca = dynamic_cast<const CaseStatement*>(stmt);
    if (ca) {
        out << "<casestmt id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << ca->number
            << "\" computed=\"" << (int)ca->m_isComputed << "\"";
//        if (ca->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(ca->parent) << "\"";
        if (ca->proc)
            out << " proc=\"" << ADDRESS::host_ptr(ca->proc) << "\"";
        out << ">\n";
        if (ca->pDest) {
            out << "<dest>\n";
            persistToXML(out, ca->pDest);
            out << "</dest>\n";
        }
        // TODO
        // SWITCH_INFO* pSwitchInfo;   // Ptr to struct with info about the switch
        out << "</casestmt>\n";
        return;
    }
    const BranchStatement *br = dynamic_cast<const BranchStatement*>(stmt);
    if (br) {
        out << "<branchstmt id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << br->number
            << "\" computed=\"" << (int)br->m_isComputed << "\""
            << " jtcond=\"" << br->jtCond << "\" float=\"" << (int)br->bFloat << "\"";
//        if (br->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(br->parent) << "\"";
        if (br->proc)
            out << " proc=\"" << ADDRESS::host_ptr(br->proc) << "\"";
        out << ">\n";
        if (br->pDest) {
            out << "<dest>\n";
            persistToXML(out, br->pDest);
            out << "</dest>\n";
        }
        if (br->pCond) {
            out << "<cond>\n";
            persistToXML(out, br->pCond);
            out << "</cond>\n";
        }
        out << "</branchstmt>\n";
        return;
    }
    const GotoStatement *g = dynamic_cast<const GotoStatement*>(stmt);
    if (g) {
        out << "<gotostmt id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << g->number << "\""
            << " computed=\"" << (int) g->m_isComputed << "\"";
//        if (g->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(g->parent) << "\"";
        if (g->proc)
            out << " proc=\"" << ADDRESS::host_ptr(g->proc) << "\"";
        out << ">\n";
        if (g->pDest) {
            out << "<dest>\n";
            persistToXML(out, g->pDest);
            out << "</dest>\n";
        }
        out << "</gotostmt>\n";
        return;
    }
    const PhiAssign *p = dynamic_cast<const PhiAssign*>(stmt);
    if (p) {
        out << "<phiassign id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << p->number << "\"";
//        if (p->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(p->parent) << "\"";
        if (p->proc)
            out << " proc=\"" << ADDRESS::host_ptr(p->proc) << "\"";
        out << ">\n";
        out << "<lhs>\n";
        persistToXML(out, p->lhs);
        out << "</lhs>\n";
        for (auto it = p->cbegin(); it != p->cend(); p++)
            out << "<def stmt=\"" << ADDRESS::host_ptr(it->second.def()) << "\" exp=\""
                << ADDRESS::host_ptr(it->second.e) << "\" />\n";
        out << "</phiassign>\n";
        return;
    }
    const Assign *a = dynamic_cast<const Assign*>(stmt);
    if (a) {
        out << "<assign id=\"" << ADDRESS::host_ptr(stmt) << "\" number=\"" << a->number << "\"";
//        if (a->parent)
//            out << " parent=\"" << ADDRESS::host_ptr(a->parent) << "\"";
        if (a->proc)
            out << " proc=\"" << ADDRESS::host_ptr(a->proc) << "\"";
        out << ">\n";
        out << "<lhs>\n";
        persistToXML(out, a->lhs);
        out << "</lhs>\n";
        out << "<rhs>\n";
        persistToXML(out, a->rhs);
        out << "</rhs>\n";
        if (a->type) {
            out << "<type>\n";
            persistToXML(out, a->type);
            out << "</type>\n";
        }
        if (a->guard) {
            out << "<guard>\n";
            persistToXML(out, a->guard);
            out << "/guard>\n";
        }
        out << "</assign>\n";
        return;
    }
    std::cerr << "unknown stmt in persistToXML\n";
    assert(false);
}

void XMLProgParser::addToContext_assignment(Context *c, int /*e*/) {
    c->stmt = stack.front()->stmt;
}

void XMLProgParser::addToContext_phiassign(Context *c, int e) {
    if (phase == 1) {
        return;
    }
    PhiAssign *pa = dynamic_cast<PhiAssign*>(c->stmt);
    assert(pa);
    switch(e) {
        case e_lhs:
            pa->setLeft(stack.front()->exp);
            break;
            // FIXME: More required
        default:
            if (e == e_unknown)
                std::cerr << "unknown tag " << e << " in context assign\n";
            else
                std::cerr << "need to handle tag " << tags[e].tag << " in context assign\n";
            break;
    }
}

