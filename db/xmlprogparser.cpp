#include <stdio.h>
extern "C" {
#include "expat.h"
}
#include "cluster.h"
#include "prog.h"
#include "proc.h"
#include "rtl.h"
#include "statement.h"
#include "sigenum.h"
#include "signature.h"
#include "xmlprogparser.h"
#include "boomerang.h"
#include "frontend.h"


typedef enum { e_prog, e_procs, e_global, e_cluster, e_libproc, e_userproc, e_local, e_symbol, e_secondexp,
	       e_proven, e_callee, e_caller, e_defines,
    	       e_signature, e_param, e_implicitparam, e_return, e_rettype, e_prefreturn, e_prefparam,
	       e_cfg, e_bb, e_inedge, e_outedge, e_livein, e_order, e_revorder,
	       e_rtl, e_stmt, e_assign, e_lhs, e_rhs, 
	       e_callstmt, e_dest, e_argument, e_implicitarg, e_returnexp,
	       e_returnstmt,
	       e_gotostmt, e_branchstmt, e_cond,
	       e_casestmt,
	       e_boolstmt,
	       e_type, e_exp, 
	       e_voidtype, e_integertype, e_pointertype, e_chartype, e_namedtype, e_arraytype, e_basetype,
	       e_location, e_unary, e_binary, e_ternary, e_const, e_terminal, e_typedexp, e_refexp, e_phiexp, e_def,
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
    { "proven", TAG(proven) },
    { "callee", TAG(callee) },
    { "caller", TAG(caller) },
    { "defines", TAG(defines) },
    { "signature", TAG(signature) },
    { "param",  TAG(param) },
    { "implicitparam", TAG(implicitparam) },
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
    { "lhs", TAG(lhs) },
    { "rhs", TAG(rhs) },
    { "callstmt", TAG(callstmt) },
    { "dest", TAG(dest) },
    { "argument", TAG(argument) },
    { "implicitarg", TAG(implicitarg) },
    { "returnexp", TAG(returnexp) },
    { "returnstmt", TAG(returnstmt) },
    { "gotostmt", TAG(gotostmt) },
    { "branchstmt", TAG(branchstmt) },
    { "cond", TAG(cond) },
    { "casestmt", TAG(casestmt) },
    { "boolstmt", TAG(boolstmt) },
    { "type", TAG(type) },
    { "exp", TAG(exp) },
    { "voidtype", TAG(voidtype) },
    { "integertype", TAG(integertype) },
    { "pointertype", TAG(pointertype) },
    { "chartype", TAG(chartype) },
    { "namedtype", TAG(namedtype) },
    { "arraytype", TAG(arraytype) },
    { "basetype", TAG(basetype) },
    { "location", TAG(location) },
    { "unary", TAG(unary) },
    { "binary", TAG(binary) },
    { "ternary", TAG(ternary) },
    { "const", TAG(const) },
    { "terminal", TAG(terminal) },
    { "typedexp", TAG(typedexp) },
    { "refexp", TAG(refexp) },
    { "phiexp", TAG(phiexp) },
    { "def", TAG(def) },
    { "subexp1", TAG(subexp1) },
    { "subexp2", TAG(subexp2) },
    { "subexp3", TAG(subexp3) },    
    { 0, 0, 0}
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
	ImplicitParameter *implicitParam;
	Return *ret;
	Type *type;
	Exp *exp, *symbol;
	std::list<Proc*> procs;

    	Context(int tag) : tag(tag), prog(NULL), proc(NULL), signature(NULL), cfg(NULL), bb(NULL), rtl(NULL), stmt(NULL), param(NULL), implicitParam(NULL), ret(NULL), type(NULL), exp(NULL) { } 
};

static void XMLCALL
start(void *data, const char *el, const char **attr)
{
  ((XMLProgParser*)data)->handleElementStart(el, attr);
}

static void XMLCALL
end(void *data, const char *el)
{
  ((XMLProgParser*)data)->handleElementEnd(el);
}

static void XMLCALL
text(void *data, const char *s, int len)
{
    int mylen;
    char buf[1024];
    if (len == 1 && *s == '\n')
	return;
    mylen = len < 1024 ? len : 1023;
    memcpy(buf, s, mylen);
    buf[mylen] = 0;
    printf("error: text in document %i bytes (%s)\n", len, buf);
}

const char *XMLProgParser::getAttr(const char **attr, const char *name)
{
  for (int i = 0; attr[i]; i += 2)
  	if (!strcmp(attr[i], name))
		return attr[i+1];
  return NULL;
}

void XMLProgParser::handleElementStart(const char *el, const char **attr)
{
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

void XMLProgParser::handleElementEnd(const char *el)
{
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

void XMLProgParser::addId(const char **attr, void *x)
{
    const char *val = getAttr(attr, "id");
    if (val) {
	//std::cerr << "map id " << val << " to " << std::hex << (int)x << std::dec << "\n";
	idToX[atoi(val)] = x;
    }
}

void *XMLProgParser::findId(const char *id)
{
    if (id == NULL)
	return NULL;
    int n = atoi(id);
    if (n == 0)
	return NULL;
    std::map<int, void*>::iterator it = idToX.find(n);
    if (it == idToX.end()) {
	std::cerr << "findId could not find \"" << id << "\"\n";
	assert(false);
	return NULL;
    }
    return (*it).second;
}

void XMLProgParser::start_prog(const char **attr)
{
    if (phase == 1) {
	return;
    }
    stack.front()->prog = new Prog();
    addId(attr, stack.front()->prog);
    const char *name = getAttr(attr, "name");
    if (name)
	stack.front()->prog->setName(name);
    const char *iNumberedProc = getAttr(attr, "iNumberedProc");
    stack.front()->prog->m_iNumberedProc = atoi(iNumberedProc);
}

void XMLProgParser::addToContext_prog(Context *c, int e)
{
    if (phase == 1) {
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
	    for (std::list<Proc*>::iterator it = stack.front()->procs.begin(); it != stack.front()->procs.end(); it++) {
		c->prog->m_procs.push_back(*it);
		c->prog->m_procLabels[(*it)->getNativeAddress()] = *it;
	    }
	    break;
	case e_cluster:
	    c->prog->m_rootCluster = stack.front()->cluster;
	    break;
	case e_global:
	    c->prog->globals.push_back(stack.front()->global);
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context prog\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context prog\n";
	    break;
    }
}

void XMLProgParser::start_procs(const char **attr)
{
    if (phase == 1) {
	return;
    }
    stack.front()->procs.clear();
}

void XMLProgParser::addToContext_procs(Context *c, int e)
{
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

void XMLProgParser::start_global(const char **attr)
{
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
	stack.front()->global->uaddr = atoi(uaddr);
}

void XMLProgParser::addToContext_global(Context *c, int e)
{
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

void XMLProgParser::start_cluster(const char **attr)
{
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

void XMLProgParser::addToContext_cluster(Context *c, int e)
{
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

void XMLProgParser::start_libproc(const char **attr)
{
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
	stack.front()->proc->address = atoi(address);
    address = getAttr(attr, "firstCallerAddress");
    if (address)
	stack.front()->proc->m_firstCallerAddr = atoi(address);
}

void XMLProgParser::addToContext_libproc(Context *c, int e)
{
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
	case e_proven:
	    c->proc->setProven(stack.front()->exp);
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

void XMLProgParser::start_userproc(const char **attr)
{
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
	proc->address = atoi(address);
    address = getAttr(attr, "decoded");
    if (address)
	proc->decoded = atoi(address) > 0;
    address = getAttr(attr, "analysed");
    if (address)
	proc->analysed = atoi(address) > 0;
    address = getAttr(attr, "decompileSeen");
    if (address)
	proc->decompileSeen = atoi(address) > 0;
    address = getAttr(attr, "decompiled");
    if (address)
	proc->decompiled = atoi(address) > 0;
    address = getAttr(attr, "isRecursive");
    if (address)
	proc->isRecursive = atoi(address) > 0;
    address = getAttr(attr, "firstCallerAddress");
    if (address)
	proc->m_firstCallerAddr = atoi(address);
}

void XMLProgParser::addToContext_userproc(Context *c, int e)
{
    UserProc *userproc = dynamic_cast<UserProc*>(c->proc);
    assert(userproc);
    if (phase == 1) {
	switch(e) {
	    case e_caller:
		{
		    CallStatement *call = dynamic_cast<CallStatement*>(stack.front()->stmt);
		    assert(call);
		    c->proc->addCaller(call);
		}
		break;
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
	case e_proven:
	    c->proc->setProven(stack.front()->exp);
	    break;
	case e_caller:
	    break;
	case e_callee:
	    break;
	case e_defines:
	    userproc->addDef(stack.front()->exp);
	    break;
	case e_cfg:
	    userproc->setCFG(stack.front()->cfg);
	    break;
	case e_local:
	    userproc->locals[stack.front()->str.c_str()] = stack.front()->type;
	    break;
	case e_symbol:
	    userproc->symbolMap[stack.front()->exp] = stack.front()->symbol;
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context userproc\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context userproc\n";
	    break;
    }
}

void XMLProgParser::start_local(const char **attr)
{
    if (phase == 1) {
	return;
    }
    stack.front()->str = getAttr(attr, "name");
}

void XMLProgParser::addToContext_local(Context *c, int e)
{
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

void XMLProgParser::start_symbol(const char **attr)
{
    if (phase == 1) {
	return;
    }
}

void XMLProgParser::addToContext_symbol(Context *c, int e)
{
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

void XMLProgParser::start_proven(const char **attr)
{
}

void XMLProgParser::addToContext_proven(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_callee(const char **attr)
{
    if (phase == 1) {
	stack.front()->proc = (Proc*)findId(getAttr(attr, "proc"));
    }
}

void XMLProgParser::addToContext_callee(Context *c, int e)
{
}

void XMLProgParser::start_caller(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "call"));
    }
}

void XMLProgParser::addToContext_caller(Context *c, int e)
{
}

void XMLProgParser::start_signature(const char **attr)
{
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
    sig->implicitParams.clear();
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

void XMLProgParser::addToContext_signature(Context *c, int e)
{
    if (phase == 1) {
	return;
    }
    switch(e) {
	case e_param:
	    c->signature->appendParameter(stack.front()->param);
	    break;
	case e_implicitparam:
	    c->signature->appendImplicitParameter(stack.front()->implicitParam);
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

void XMLProgParser::start_param(const char **attr)
{
    if (phase == 1) {
	stack.front()->param = (Parameter*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->param = new Parameter();
    addId(attr, stack.front()->param);
    const char *n = getAttr(attr, "name");
    if (n)
	stack.front()->param->setName(n);
}

void XMLProgParser::addToContext_param(Context *c, int e)
{
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

void XMLProgParser::start_implicitparam(const char **attr)
{
    if (phase == 1) {
	stack.front()->implicitParam = (ImplicitParameter*)findId(getAttr(attr, "id"));
	Parameter *parent = (Parameter*)findId(getAttr(attr, "parent"));
	if (parent)
	    stack.front()->implicitParam->setParent(parent);
	return;
    }
    stack.front()->implicitParam = new ImplicitParameter();
    addId(attr, stack.front()->implicitParam);
    const char *n = getAttr(attr, "name");
    if (n)
	stack.front()->implicitParam->setName(n);
}

void XMLProgParser::addToContext_implicitparam(Context *c, int e)
{
    if (phase == 1) {
	return;
    }
    switch(e) {
	case e_type:
	    c->implicitParam->setType(stack.front()->type);
	    break;
	case e_exp:
	    c->implicitParam->setExp(stack.front()->exp);
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context implicitParam\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context implicitParam\n";
	break;
    }
}

void XMLProgParser::start_prefreturn(const char **attr)
{
    if (phase == 1) {
	return;
    }
}

void XMLProgParser::addToContext_prefreturn(Context *c, int e)
{
    if (phase == 1) {
	return;
    }
    c->type = stack.front()->type;
}

void XMLProgParser::start_prefparam(const char **attr)
{
    if (phase == 1) {
	return;
    }
    const char *n = getAttr(attr, "index");
    assert(n);
    stack.front()->n = atoi(n);
}

void XMLProgParser::addToContext_prefparam(Context *c, int e)
{
    if (phase == 1) {
	return;
    }
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context prefparam\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context prefparam\n";
//	break;
//    }
}

void XMLProgParser::start_return(const char **attr)
{
    if (phase == 1) {
	stack.front()->ret = (Return*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->ret = new Return();
    addId(attr, stack.front()->ret);
}

void XMLProgParser::addToContext_return(Context *c, int e)
{
    if (phase == 1) {
	return;
    }
    switch(e) {
	case e_type:
	    c->ret->setType(stack.front()->type);
	    break;
	case e_exp:
	    c->ret->setExp(stack.front()->exp);
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context return\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context return\n";
	break;
    }
}

void XMLProgParser::start_rettype(const char **attr)
{
}

void XMLProgParser::addToContext_rettype(Context *c, int e)
{
    c->type = stack.front()->type;
}

void XMLProgParser::start_cfg(const char **attr)
{
    if (phase == 1) {
	stack.front()->cfg = (Cfg*)findId(getAttr(attr, "id"));
	PBB entryBB = (PBB)findId(getAttr(attr, "entryBB"));
	if (entryBB)
	    stack.front()->cfg->setEntryBB(entryBB);
	PBB exitBB = (PBB)findId(getAttr(attr, "exitBB"));
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

void XMLProgParser::addToContext_cfg(Context *c, int e)
{
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

void XMLProgParser::start_bb(const char **attr)
{
    PBB bb;
    if (phase == 1) {
	bb = stack.front()->bb = (BasicBlock*)findId(getAttr(attr, "id"));
	PBB h = (PBB)findId(getAttr(attr, "m_loopHead"));
	if (h)
	    bb->m_loopHead = h;
	h = (PBB)findId(getAttr(attr, "m_caseHead"));
	if (h)
	    bb->m_caseHead = h;
	h = (PBB)findId(getAttr(attr, "m_condFollow"));
	if (h)
	    bb->m_condFollow = h;
	h = (PBB)findId(getAttr(attr, "m_loopFollow"));
	if (h)
	    bb->m_loopFollow = h;
	h = (PBB)findId(getAttr(attr, "m_latchNode"));
	if (h)
	    bb->m_latchNode = h;
	// note the rediculous duplication here
	h = (PBB)findId(getAttr(attr, "immPDom"));
	if (h)
	    bb->immPDom = h;
	h = (PBB)findId(getAttr(attr, "loopHead"));
	if (h)
	    bb->loopHead = h;
	h = (PBB)findId(getAttr(attr, "caseHead"));
	if (h)
	    bb->caseHead = h;
	h = (PBB)findId(getAttr(attr, "condFollow"));
	if (h)
	    bb->condFollow = h;
	h = (PBB)findId(getAttr(attr, "loopFollow"));
	if (h)
	    bb->loopFollow = h;
	h = (PBB)findId(getAttr(attr, "latchNode"));
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

void XMLProgParser::addToContext_bb(Context *c, int e)
{
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

void XMLProgParser::start_inedge(const char **attr)
{
    if (phase == 1)
	stack.front()->bb = (BasicBlock*)findId(getAttr(attr, "bb"));
    else
	stack.front()->bb = NULL;
}

void XMLProgParser::addToContext_inedge(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context inedge\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context inedge\n";
//	break;
//    }
}

void XMLProgParser::start_outedge(const char **attr)
{
    if (phase == 1)
	stack.front()->bb = (BasicBlock*)findId(getAttr(attr, "bb"));
    else
	stack.front()->bb = NULL;
}

void XMLProgParser::addToContext_outedge(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context outedge\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context outedge\n";
//	break;
//    }
}

void XMLProgParser::start_livein(const char **attr)
{
}

void XMLProgParser::addToContext_livein(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_order(const char **attr)
{
    if (phase == 1)
	stack.front()->bb = (PBB)findId(getAttr(attr, "bb"));
    else 
	stack.front()->bb = NULL;
}

void XMLProgParser::addToContext_order(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context order\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context order\n";
//	break;
//    }
}

void XMLProgParser::start_revorder(const char **attr)
{
    if (phase == 1)
	stack.front()->bb = (PBB)findId(getAttr(attr, "bb"));
    else 
	stack.front()->bb = NULL;
}

void XMLProgParser::addToContext_revorder(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context revOrder\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context order\n";
//	break;
//    }
}

void XMLProgParser::start_rtl(const char **attr)
{
    if (phase == 1) {
	stack.front()->rtl = (RTL*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->rtl = new RTL();
    addId(attr, stack.front()->rtl);
    const char *a = getAttr(attr, "addr");
    if (a)
	stack.front()->rtl->nativeAddr = atoi(a);
}

void XMLProgParser::addToContext_rtl(Context *c, int e)
{
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

void XMLProgParser::start_stmt(const char **attr)
{
}

void XMLProgParser::addToContext_stmt(Context *c, int e)
{
    c->stmt = stack.front()->stmt;
}

void XMLProgParser::start_assign(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    stack.front()->stmt->setProc(p);
	Statement *parent = (Statement*)findId(getAttr(attr, "parent"));
	if (parent)
	    stack.front()->stmt->parent = parent;
	return;
    }
    stack.front()->stmt = new Assign();
    addId(attr, stack.front()->stmt);
    const char *n = getAttr(attr, "number");
    if (n)
	stack.front()->stmt->number = atoi(n);
}

void XMLProgParser::addToContext_assign(Context *c, int e)
{
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

void XMLProgParser::start_callstmt(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    ((Statement*)stack.front()->stmt)->setProc(p);
	Statement *s = (Statement*)findId(getAttr(attr, "parent"));
	if (s)
	    ((Statement*)stack.front()->stmt)->parent = s;
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
    n = getAttr(attr, "returnTypeSize");
    if (n)
	call->returnTypeSize = atoi(n);
    n = getAttr(attr, "returnAftercall");
    if (n)
	call->returnAfterCall = atoi(n) > 0;
}

void XMLProgParser::addToContext_callstmt(Context *c, int e)
{
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
    switch(e) {
	case e_dest:
	    call->setDest(stack.front()->exp);
	    break;
	case e_implicitarg:
	    call->appendImplicitArgument(stack.front()->exp);
	    break;
	case e_argument:
	    call->appendArgument(stack.front()->exp);
	    break;
	case e_returnexp:
	    call->addReturn(stack.front()->exp);
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context callstmt\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context callstmt\n";
	break;
    }
}

void XMLProgParser::start_dest(const char **attr)
{
    if (phase == 1) {
	Proc *p = (Proc*)findId(getAttr(attr, "proc"));
	if (p)
	    stack.front()->proc = p;
	return;
    }
}

void XMLProgParser::addToContext_dest(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_returnstmt(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    ((Statement*)stack.front()->stmt)->setProc(p);
	Statement *s = (Statement*)findId(getAttr(attr, "parent"));
	if (s)
	    ((Statement*)stack.front()->stmt)->parent = s;
	return;
    }
    ReturnStatement *ret = new ReturnStatement();
    stack.front()->stmt = ret;
    addId(attr, ret);
    const char *n = getAttr(attr, "number");
    if (n)
	ret->number = atoi(n);
    n = getAttr(attr, "bytesPopped");
    if (n)
	ret->nBytesPopped = atoi(n);
    n = getAttr(attr, "retAddr");
    if (n)
	ret->retAddr = atoi(n);
}

void XMLProgParser::addToContext_returnstmt(Context *c, int e)
{
    ReturnStatement *ret = dynamic_cast<ReturnStatement*>(c->stmt);
    assert(ret);
    if (phase == 1) {
	return;
    }
    switch(e) {
	case e_returnexp:
	    ret->returns.push_back(stack.front()->exp);
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context returnstmt\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context returnstmt\n";
	break;
    }
}

void XMLProgParser::start_gotostmt(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    ((Statement*)stack.front()->stmt)->setProc(p);
	Statement *s = (Statement*)findId(getAttr(attr, "parent"));
	if (s)
	    ((Statement*)stack.front()->stmt)->parent = s;
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

void XMLProgParser::addToContext_gotostmt(Context *c, int e)
{
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

void XMLProgParser::start_branchstmt(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    ((Statement*)stack.front()->stmt)->setProc(p);
	Statement *s = (Statement*)findId(getAttr(attr, "parent"));
	if (s)
	    ((Statement*)stack.front()->stmt)->parent = s;
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

void XMLProgParser::addToContext_branchstmt(Context *c, int e)
{
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

void XMLProgParser::start_casestmt(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    ((Statement*)stack.front()->stmt)->setProc(p);
	Statement *s = (Statement*)findId(getAttr(attr, "parent"));
	if (s)
	    ((Statement*)stack.front()->stmt)->parent = s;
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

void XMLProgParser::addToContext_casestmt(Context *c, int e)
{
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

void XMLProgParser::start_boolstmt(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "id"));
	UserProc *p = (UserProc*)findId(getAttr(attr, "proc"));
	if (p)
	    ((Statement*)stack.front()->stmt)->setProc(p);
	Statement *s = (Statement*)findId(getAttr(attr, "parent"));
	if (s)
	    ((Statement*)stack.front()->stmt)->parent = s;
	return;
    }
    const char *n = getAttr(attr, "size");
    assert(n);
    BoolStatement *boo = new BoolStatement(atoi(n));
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

void XMLProgParser::addToContext_boolstmt(Context *c, int e)
{
    BoolStatement *boo = dynamic_cast<BoolStatement*>(c->stmt);
    assert(boo);
    if (phase == 1) {
	return;
    }
    switch(e) {
	case e_cond:
	    boo->pCond = stack.front()->exp;
	    break;
	case e_dest:
	    boo->pDest = stack.front()->exp;
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context boolstmt\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context boolstmt\n";
	break;
    }
}

void XMLProgParser::start_type(const char **attr)
{
}

void XMLProgParser::addToContext_type(Context *c, int e)
{
    c->type = stack.front()->type;
}

void XMLProgParser::start_basetype(const char **attr)
{
}

void XMLProgParser::addToContext_basetype(Context *c, int e)
{
    c->type = stack.front()->type;
}

void XMLProgParser::start_exp(const char **attr)
{
}

void XMLProgParser::addToContext_exp(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_secondexp(const char **attr)
{
}

void XMLProgParser::addToContext_secondexp(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_defines(const char **attr)
{
}

void XMLProgParser::addToContext_defines(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_lhs(const char **attr)
{
}

void XMLProgParser::addToContext_lhs(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_rhs(const char **attr)
{
}

void XMLProgParser::addToContext_rhs(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_argument(const char **attr)
{
}

void XMLProgParser::addToContext_argument(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_implicitarg(const char **attr)
{
}

void XMLProgParser::addToContext_implicitarg(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_returnexp(const char **attr)
{
}

void XMLProgParser::addToContext_returnexp(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_cond(const char **attr)
{
}

void XMLProgParser::addToContext_cond(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_subexp1(const char **attr)
{
}

void XMLProgParser::addToContext_subexp1(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_subexp2(const char **attr)
{
}

void XMLProgParser::addToContext_subexp2(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_subexp3(const char **attr)
{
}

void XMLProgParser::addToContext_subexp3(Context *c, int e)
{
    c->exp = stack.front()->exp;
}

void XMLProgParser::start_voidtype(const char **attr)
{
    if (phase == 1) {
	stack.front()->type = (Type*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->type = new VoidType();
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_voidtype(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context voidType\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context voidType\n";
//	break;
//    }
}

void XMLProgParser::start_integertype(const char **attr)
{
    if (phase == 1) {
	stack.front()->type = (Type*)findId(getAttr(attr, "id"));
	return;
    }
    IntegerType *ty = new IntegerType();
    stack.front()->type = ty;
    addId(attr, ty);
    const char *n = getAttr(attr, "size");
    if (n)
	ty->size = atoi(n);
    n = getAttr(attr, "signd");
    if (n)
	ty->signd = atoi(n) > 0;
}

void XMLProgParser::addToContext_integertype(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context integerType\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context integerType\n";
//	break;
//    }
}

void XMLProgParser::start_pointertype(const char **attr)
{
    if (phase == 1) {
	stack.front()->type = (Type*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->type = new PointerType(NULL);
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_pointertype(Context *c, int e)
{
    PointerType *p = dynamic_cast<PointerType*>(c->type);
    assert(p);
    if (phase == 1) {
	return;
    }
    p->setPointsTo(stack.front()->type);
}

void XMLProgParser::start_chartype(const char **attr)
{
    if (phase == 1) {
	stack.front()->type = (Type*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->type = new CharType();
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_chartype(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context charType\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context charType\n";
//	break;
//    }
}

void XMLProgParser::start_namedtype(const char **attr)
{
    if (phase == 1) {
	stack.front()->type = (Type*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->type = new NamedType(getAttr(attr, "name"));
    addId(attr, stack.front()->type);
}

void XMLProgParser::addToContext_namedtype(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context namedType\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context namedType\n";
//	break;
//    }
}

void XMLProgParser::start_arraytype(const char **attr)
{
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

void XMLProgParser::addToContext_arraytype(Context *c, int e)
{
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

void XMLProgParser::start_location(const char **attr)
{
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

void XMLProgParser::addToContext_location(Context *c, int e)
{
    if (phase == 1) {
	return;
    }
    Location *l = dynamic_cast<Location*>(c->exp);
    assert(l);
    switch(e) {
	case e_type:
	    l->ty = stack.front()->type;
	    break;
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

void XMLProgParser::start_unary(const char **attr)
{
    if (phase == 1) {
	stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
	return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Unary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_unary(Context *c, int e)
{
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

void XMLProgParser::start_binary(const char **attr)
{
    if (phase == 1) {
	stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
	return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Binary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_binary(Context *c, int e)
{
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

void XMLProgParser::start_ternary(const char **attr)
{
    if (phase == 1) {
	stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
	return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Ternary(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_ternary(Context *c, int e)
{
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

void XMLProgParser::start_const(const char **attr)
{
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

void XMLProgParser::addToContext_const(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context const\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context const\n";
//	break;
//    }
}

void XMLProgParser::start_terminal(const char **attr)
{
    if (phase == 1) {
	stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
	return;
    }
    OPER op = (OPER)operFromString(getAttr(attr, "op"));
    assert(op != -1);
    stack.front()->exp = new Terminal(op);
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_terminal(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context terminal\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context terminal\n";
//	break;
//    }
}

void XMLProgParser::start_typedexp(const char **attr)
{
    if (phase == 1) {
	stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->exp = new TypedExp();
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_typedexp(Context *c, int e)
{
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

void XMLProgParser::start_refexp(const char **attr)
{
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

void XMLProgParser::addToContext_refexp(Context *c, int e)
{
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

void XMLProgParser::start_phiexp(const char **attr)
{
    if (phase == 1) {
	stack.front()->exp = (Exp*)findId(getAttr(attr, "id"));
	return;
    }
    stack.front()->exp = new PhiExp();
    addId(attr, stack.front()->exp);
}

void XMLProgParser::addToContext_phiexp(Context *c, int e)
{
    if (phase == 1) {
	switch(e) {
	    case e_def:
		PhiExp *phi = dynamic_cast<PhiExp*>(c->exp);
		assert(phi);
		phi->stmtVec.putAt(phi->stmtVec.size(), stack.front()->stmt);
		break;
	}
	return;
    }
    switch(e) {
	case e_subexp1:
	    c->exp->setSubExp1(stack.front()->exp);
	    break;
	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context phiexp\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context phiexp\n";
	    break;
    }
}

void XMLProgParser::start_def(const char **attr)
{
    if (phase == 1) {
	stack.front()->stmt = (Statement*)findId(getAttr(attr, "stmt"));
	return;
    }
}

void XMLProgParser::addToContext_def(Context *c, int e)
{
//    switch(e) {
//	default:
	    if (e == e_unknown)
		std::cerr << "unknown tag " << e << " in context def\n";
	    else 
		std::cerr << "need to handle tag " << tags[e].tag << " in context def\n";
//	    break;
//    }
}

Prog *XMLProgParser::parse(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (f == NULL)
      return NULL;
  fclose(f);

  stack.clear();
  Prog *prog = NULL;
  for (phase = 0; phase < 2; phase++) {
      parseFile(filename);
      if (stack.front()->prog) {
	  prog = stack.front()->prog;
	  parseChildren(prog->getRootCluster());
      }
  }
  if (prog == NULL)
      return NULL;
  FrontEnd *pFE = FrontEnd::Load(prog->getName());
  prog->setFrontEnd(pFE);
  prog->setBinaryFile(pFE->getBinaryFile());
  return prog;
}

void XMLProgParser::parseFile(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (f == NULL)
      return;
  XML_Parser p = XML_ParserCreate(NULL);
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
	    fprintf(stderr, "Parse error at line %d of file %s:\n%s\n", XML_GetCurrentLineNumber(p), filename, XML_ErrorString(XML_GetErrorCode(p)));
	fclose(f);
	return;
    }

    if (done)
	break;
  }
  fclose(f);
}

void XMLProgParser::parseChildren(Cluster *c)
{
    std::string path = c->makeDirs();
    for (unsigned i = 0; i < c->children.size(); i++) {
	std::string d = path + "/" + c->children[i]->getName() + ".xml";
	parseFile(d.c_str());
	parseChildren(c->children[i]);
    }
}

extern char* operStrings[];

int XMLProgParser::operFromString(const char *s)
{
    for (int i = 0; i < opNumOf; i++)
	if (!strcmp(s, operStrings[i]))
	    return i;
    return -1;
}


void XMLProgParser::persistToXML(std::ostream &out, Cluster *c)
{
    out << "<cluster id=\"" << (int)c << "\" name=\"" << c->name << "\"";
    out << ">\n";
    for (unsigned i = 0; i < c->children.size(); i++) {
	persistToXML(out, c->children[i]);
    }
    out << "</cluster>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Global *g)
{
    out << "<global name=\"" << g->nam << "\" uaddr=\"" << (int)g->uaddr << "\">\n";
    out << "<type>\n";
    persistToXML(out, g->type);
    out << "</type>\n";
    out << "</global>\n";
}

const char *Cluster::getOutPath(const char *ext)
{
    std::string basedir = makeDirs();
    return (basedir + "/" + name + "." + ext).c_str();
}

void Cluster::openStream(const char *ext)
{
    if (out.is_open())
	return;
    out.open(getOutPath(ext));
    if (!strcmp(ext, "xml")) {
	out << "<?xml version=\"1.0\"?>\n";
	if (parent != NULL)
	    out << "<procs>\n";
    }
}

void Cluster::openStreams(const char *ext)
{
    openStream(ext);
    for (unsigned i = 0; i < children.size(); i++)
	children[i]->openStreams(ext);
}

void Cluster::closeStreams()
{
    if (out.is_open()) {
	if (parent != NULL)
	    out << "</procs>\n";
	out.close();
    }
    for (unsigned i = 0; i < children.size(); i++)
	children[i]->closeStreams();
}

void XMLProgParser::persistToXML(Prog *prog)
{
    prog->m_rootCluster->openStreams("xml");
    std::ofstream &os = prog->m_rootCluster->getStream();
    os << "<prog name=\"" << prog->getName() << "\" iNumberedProc=\"" << prog->m_iNumberedProc << "\">\n";
    for (std::vector<Global*>::iterator it1 = prog->globals.begin(); it1 != prog->globals.end(); it1++)
	persistToXML(os, *it1);
    persistToXML(os, prog->m_rootCluster);
    for (std::list<Proc*>::iterator it = prog->m_procs.begin(); it != prog->m_procs.end(); it++) {
        Proc *p = *it;
	persistToXML(p->getCluster()->getStream(), p);
    }
    os << "</prog>\n";
    os.close();
    prog->m_rootCluster->closeStreams();
}

void XMLProgParser::persistToXML(std::ostream &out, LibProc *proc)
{
    out << "<libproc id=\"" << (int)proc << "\" address=\"" << (int)proc->address << "\"";
    out << " firstCallerAddress=\"" << proc->m_firstCallerAddr << "\"";
    if (proc->m_firstCaller)
	out << " firstCaller=\"" << (int)proc->m_firstCaller << "\"";
    if (proc->cluster)
	out << " cluster=\"" << (int)proc->cluster << "\"";
    out << ">\n";

    persistToXML(out, proc->signature);

    for (std::set<CallStatement*>::iterator it = proc->callerSet.begin();
      it != proc->callerSet.end(); it++)
	out << "<caller call=\"" << (int)(*it) << "\"/>\n";
    for (std::set<Exp*, lessExpStar>::iterator it = proc->proven.begin();
      it != proc->proven.end(); it++) {
	out << "<proven>\n";
	persistToXML(out, *it);
	out << "</proven>\n";
    }
    out << "</libproc>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, UserProc *proc)
{
    out << "<userproc id=\"" << (int)proc << "\"";
    out << " address=\"" << (int)proc->address << "\"";
    out << " decoded=\"" << (int)proc->decoded << "\"";
    out << " analysed=\"" << (int)proc->analysed << "\"";
    out << " decompileSeen=\"" << (int)proc->decompileSeen << "\"";
    out << " decompiled=\"" << (int)proc->decompiled << "\"";
    out << " isRecursive=\"" << (int)proc->isRecursive << "\"";
    out << " firstCallerAddress=\"" << proc->m_firstCallerAddr << "\"";
    if (proc->m_firstCaller)
	out << " firstCaller=\"" << (int)proc->m_firstCaller << "\"";
    if (proc->cluster)
	out << " cluster=\"" << (int)proc->cluster << "\"";
    if (proc->theReturnStatement)
	out << " retstmt=\"" << (int)proc->theReturnStatement << "\"";
    out << ">\n";

    persistToXML(out, proc->signature);

    for (std::set<CallStatement*>::iterator it = proc->callerSet.begin();
      it != proc->callerSet.end(); it++)
	out << "<caller call=\"" << (int)(*it) << "\"/>\n";
    for (std::set<Exp*, lessExpStar>::iterator it = proc->proven.begin();
      it != proc->proven.end(); it++) {
	out << "<proven>\n";
	persistToXML(out, *it);
	out << "</proven>\n";
    }

    for (std::map<std::string, Type*>::iterator it1 = proc->locals.begin(); it1 != proc->locals.end(); it1++) {
	out << "<local name=\"" << (*it1).first << "\">\n";
	out << "<type>\n";
	persistToXML(out, (*it1).second);
	out << "</type>\n";
	out << "</local>\n";
    }

    for (std::map<Exp*, Exp*, lessExpStar>::iterator it2 = proc->symbolMap.begin(); it2 != proc->symbolMap.end(); it2++) {
	out << "<symbol>\n";
	out << "<exp>\n";
	persistToXML(out, (*it2).first);
	out << "</exp>\n";
	out << "<secondexp>\n";
	persistToXML(out, (*it2).second);
	out << "</secondexp>\n";
	out << "</symbol>\n";
    }


    for (std::set<Proc*>::iterator it = proc->calleeSet.begin(); it != proc->calleeSet.end(); it++)
	out << "<callee proc=\"" << (int)(*it) << "\"/>\n";

    persistToXML(out, proc->cfg);

    out << "</userproc>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Proc *proc)
{
    if (proc->isLib())
	persistToXML(out, (LibProc*)proc);
    else
	persistToXML(out, (UserProc*)proc);
}

void XMLProgParser::persistToXML(std::ostream &out, Signature *sig)
{
    out << "<signature id=\"" << (int)sig << "\"";
    out << " name=\"" << sig->name << "\"";
    out << " ellipsis=\"" << (int)sig->ellipsis << "\"";
    out << " preferedName=\"" << sig->preferedName << "\"";
    if (sig->getPlatform() != PLAT_GENERIC)
	out << " platform=\"" << sig->platformName(sig->getPlatform()) << "\"";
    if (sig->getConvention() != CONV_NONE)
	out << " convention=\"" << sig->conventionName(sig->getConvention()) << "\"";
    out << ">\n";
    for (unsigned i = 0; i < sig->params.size(); i++) {
	out << "<param id=\"" << (int)sig->params[i] << "\" name=\"" << sig->params[i]->getName() << "\">\n";
	out << "<type>\n";
	persistToXML(out, sig->params[i]->getType());
	out << "</type>\n";
	out << "<exp>\n";
	persistToXML(out, sig->params[i]->getExp());
	out << "</exp>\n";
	out << "</param>\n";
    }
    for (unsigned i = 0; i < sig->implicitParams.size(); i++) {
	out << "<implicitparam id=\"" << (int)sig->implicitParams[i] << "\"";
	if (sig->implicitParams[i]->getParent())
	    out << " parent=\"" << (int)sig->implicitParams[i]->getParent() << "\"";
	out << " name=\"" << sig->implicitParams[i]->getName() << "\">\n";
	out << "<type>\n";
	persistToXML(out, sig->implicitParams[i]->getType());
	out << "</type>\n";
	out << "<exp>\n";
	persistToXML(out, sig->implicitParams[i]->getExp());
	out << "</exp>\n";
	out << "</implicitparam>\n";
    }
    for (unsigned i = 0; i < sig->returns.size(); i++) {
	out << "<return>\n";
	out << "<type>\n";
	persistToXML(out, sig->returns[i]->getType());
	out << "</type>\n";
	out << "<exp>\n";
	persistToXML(out, sig->returns[i]->getExp());
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
    for (unsigned i = 0; i < sig->preferedParams.size(); i++)
	out << "<prefparam index=\"" << sig->preferedParams[i] << "\"/>\n";
    out << "</signature>\n";
}


void XMLProgParser::persistToXML(std::ostream &out, Type *ty)
{
    VoidType *v = dynamic_cast<VoidType*>(ty);
    if (v) { 
	out << "<voidtype id=\"" << (int)ty << "\"/>\n";
	return;
    }
    FuncType *f = dynamic_cast<FuncType*>(ty);
    if (f) {
	out << "<functype id=\"" << (int)ty << "\">\n";
	persistToXML(out, f->signature);
	out << "</functype>\n";
	return;
    }
    IntegerType *i = dynamic_cast<IntegerType*>(ty);
    if (i) {
	out << "<integertype id=\"" << (int)ty << "\" size=\"" << i->size << "\" signd=\"" << i->signd << "\"/>\n";
	return;
    }
    FloatType *fl = dynamic_cast<FloatType*>(ty);
    if (fl) {
	out << "<floattype id=\"" << (int)ty << "\" size=\"" << fl->size << "\"/>\n";
	return;
    }
    BooleanType *b = dynamic_cast<BooleanType*>(ty);
    if (b) {
	out << "<booleantype id=\"" << (int)ty << "\"/>\n";
	return;
    }
    CharType *c = dynamic_cast<CharType*>(ty);
    if (c) {
	out << "<chartype id=\"" << (int)ty << "\"/>\n";
	return;
    }
    PointerType *p = dynamic_cast<PointerType*>(ty);
    if (p) {
	out << "<pointertype id=\"" << (int)ty << "\">\n";
	persistToXML(out, p->points_to);
	out << "</pointertype>\n";
	return;
    }
    ArrayType *a = dynamic_cast<ArrayType*>(ty);
    if (a) {
	out << "<arraytype id=\"" << (int)ty << "\" length=\"" << (int)a->length << "\">\n";
	out << "<basetype>\n";
	persistToXML(out, a->base_type);
	out << "</basetype>\n";
	out << "</arraytype>\n";
	return;
    }
    NamedType *n = dynamic_cast<NamedType*>(ty);
    if (n) {
	out << "<namedtype id=\"" << (int)ty << "\" name=\"" << n->name << "\"/>\n";
	return;
    }
    CompoundType *co = dynamic_cast<CompoundType*>(ty);
    if (co) {
	out << "<compoundtype id=\"" << (int)ty << "\">\n";
	for (unsigned i = 0; i < co->names.size(); i++) {
	    out << "<member name=\"" << co->names[i] << "\">\n";
	    persistToXML(out, co->types[i]);
	    out << "</member>\n";
	}
	out << "</compoundtype>\n";
	return;
    }    
    std::cerr << "unknown type in persistToXML\n";
    assert(false);
}

void XMLProgParser::persistToXML(std::ostream &out, Exp *e)
{
    TypeVal *t = dynamic_cast<TypeVal*>(e);
    if (t) {
	out << "<typeval id=\"" << (int)e << "\" op=\"" << operStrings[t->op] << "\">\n";
	out << "<type>\n";
	persistToXML(out, t->val);
	out << "</type>\n";
	out << "</typeval>\n";
	return;
    } 
    Terminal *te = dynamic_cast<Terminal*>(e);
    if (te) {
	out << "<terminal id=\"" << (int)e << "\" op=\"" << operStrings[te->op] << "\"/>\n";
	return;
    } 
    Const *c = dynamic_cast<Const*>(e);
    if (c) {
	out << "<const id=\"" << (int)e << "\" op=\"" << operStrings[c->op] << "\"";
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
    Location *l = dynamic_cast<Location*>(e);
    if (l) {
	out << "<location id=\"" << (int)e << "\"";
	if (l->proc)
	    out << " proc=\"" << (int)l->proc << "\"";
	out << " op=\"" << operStrings[l->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, l->subExp1);
	out << "</subexp1>\n";
	if (l->ty) {
	    out << "<type>\n";
	    persistToXML(out, l->ty);
	    out << "</type>\n";
	}
	out << "</location>\n";
	return;
    } 
    PhiExp *p = dynamic_cast<PhiExp*>(e);
    if (p) {
	out << "<phiexp id=\"" << (int)e << "\"";
	if (p->stmt)
	    out << " stmt=\"" << (int)p->stmt << "\"";
	out << " op=\"" << operStrings[p->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, p->subExp1);
	out << "</subexp1>\n";
	StatementVec::iterator it;
	for (it = p->stmtVec.begin(); it != p->stmtVec.end(); it++)
	    out << "<def stmt=\"" << (int)(*it) << "\" />\n";
	out << "</phiexp>\n";
	return;
    }
    RefExp *r = dynamic_cast<RefExp*>(e);
    if (r) {
	out << "<refexp id=\"" << (int)e << "\"";
	if (r->def)
	    out << " def=\"" << (int)r->def << "\"";
	out << " op=\"" << operStrings[r->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, r->subExp1);
	out << "</subexp1>\n";
	out << "</refexp>\n";
	return;
    }
    FlagDef *f = dynamic_cast<FlagDef*>(e);
    if (f) {
	out << "<flagdef id=\"" << (int)e << "\"";
	if (f->rtl)
	    out << " rtl=\"" << (int)f->rtl << "\"";
	out << " op=\"" << operStrings[f->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, f->subExp1);
	out << "</subexp1>\n";
	out << "</flagdef>\n";
	return;
    }
    TypedExp *ty = dynamic_cast<TypedExp*>(e);
    if (ty) {
	out << "<typedexp id=\"" << (int)e << "\" op=\"" << operStrings[ty->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, ty->subExp1);
	out << "</subexp1>\n";
	out << "<type>\n";
	persistToXML(out, ty->type);
	out << "</type>\n";
	out << "</typedexp>\n";
    	return;
    }
    Ternary *tn = dynamic_cast<Ternary*>(e);
    if (tn) {
	out << "<ternary id=\"" << (int)e << "\" op=\"" << operStrings[tn->op] << "\">\n";
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
    Binary *b = dynamic_cast<Binary*>(e);
    if (b) {
	out << "<binary id=\"" << (int)e << "\" op=\"" << operStrings[b->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, b->subExp1);
	out << "</subexp1>\n";
	out << "<subexp2>\n";
	persistToXML(out, b->subExp2);
	out << "</subexp2>\n";
	out << "</binary>\n";
	return;
    }
    Unary *u = dynamic_cast<Unary*>(e);
    if (u) {
	out << "<unary id=\"" << (int)e << "\" op=\"" << operStrings[u->op] << "\">\n";
	out << "<subexp1>\n";
	persistToXML(out, u->subExp1);
	out << "</subexp1>\n";
	out << "</unary>\n";
	return;
    }
    std::cerr << "unknown exp in persistToXML\n";
    assert(false);
}

void XMLProgParser::persistToXML(std::ostream &out, Cfg *cfg)
{
    out << "<cfg id=\"" << (int)cfg << "\" wellformed=\"" << (int)cfg->m_bWellFormed << "\" lastLabel=\"" << cfg->lastLabel << "\"";
    out << " entryBB=\"" << (int)cfg->entryBB << "\"";
    out << " exitBB=\"" << (int)cfg->exitBB << "\"";
    out << ">\n";

    for (std::list<PBB>::iterator it = cfg->m_listBB.begin(); it != cfg->m_listBB.end(); it++)
	persistToXML(out, *it);

    for (unsigned i = 0; i < cfg->Ordering.size(); i++)
	out << "<order bb=\"" << (int)cfg->Ordering[i] << "\"/>\n";

    for (unsigned i = 0; i < cfg->revOrdering.size(); i++)
	out << "<revorder bb=\"" << (int)cfg->revOrdering[i] << "\"/>\n";

    // TODO
    // MAPBB m_mapBB;
    // std::set<CallStatement*> callSites;
    // std::vector<PBB> BBs;               // Pointers to BBs from indices
    // std::map<PBB, int> indices;         // Indices from pointers to BBs
    // more
    out << "</cfg>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, BasicBlock *bb)
{
    out << "<bb id=\"" << (int)bb << "\" nodeType=\"" << bb->m_nodeType << "\" labelNum=\"" << bb->m_iLabelNum 
	<< "\" label=\"" << bb->m_labelStr << "\" labelneeded=\"" << (int)bb->m_labelneeded << "\""
	<< " incomplete=\"" << (int)bb->m_bIncomplete << "\" jumpreqd=\"" << (int)bb->m_bJumpReqd << "\" m_traversed=\"" << bb->m_iTraversed << "\"";
    out << " DFTfirst=\"" << bb->m_DFTfirst << "\" DFTlast=\"" << bb->m_DFTlast << "\" DFTrevfirst=\"" << bb->m_DFTrevfirst 
	<< "\" DFTrevlast=\"" << bb->m_DFTrevlast << "\"";
    out << " structType=\"" << bb->m_structType << "\"";
    out << " loopCondType=\"" << bb->m_loopCondType << "\"";
    if (bb->m_loopHead)
	out << " m_loopHead=\"" << (int)bb->m_loopHead << "\"";
    if (bb->m_caseHead)
	out << " m_caseHead=\"" << (int)bb->m_caseHead << "\"";
    if (bb->m_condFollow)
	out << " m_condFollow=\"" << (int)bb->m_condFollow << "\"";
    if (bb->m_loopFollow)
	out << " m_loopFollow=\"" << (int)bb->m_loopFollow << "\"";
    if (bb->m_latchNode)
	out << " m_latchNode=\"" << (int)bb->m_latchNode << "\"";
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
	out << " immPDom=\"" << (int)bb->immPDom << "\"";
    if (bb->loopHead)
	out << " loopHead=\"" << (int)bb->loopHead << "\"";
    if (bb->caseHead)
	out << " caseHead=\"" << (int)bb->caseHead << "\"";
    if (bb->condFollow)
	out << " condFollow=\"" << (int)bb->condFollow << "\"";
    if (bb->loopFollow)
	out << " loopFollow=\"" << (int)bb->loopFollow << "\"";
    if (bb->latchNode)
	out << " latchNode=\"" << (int)bb->latchNode << "\"";
    out << " sType=\"" << (int)bb->sType << "\"";
    out << " usType=\"" << (int)bb->usType << "\"";
    out << " lType=\"" << (int)bb->lType << "\"";
    out << " cType=\"" << (int)bb->cType << "\"";
    out << ">\n";

    for (unsigned i = 0; i < bb->m_InEdges.size(); i++)
	out << "<inedge bb=\"" << (int)bb->m_InEdges[i] << "\"/>\n";
    for (unsigned i = 0; i < bb->m_OutEdges.size(); i++)
	out << "<outedge bb=\"" << (int)bb->m_OutEdges[i] << "\"/>\n";

    LocationSet::iterator it;
    for (it = bb->liveIn.begin(); it != bb->liveIn.end(); it++) {
	out << "<livein>\n";
	persistToXML(out, *it);
	out << "</livein>\n";
    }

    if (bb->m_pRtls) {
	for (std::list<RTL*>::iterator it = bb->m_pRtls->begin(); it != bb->m_pRtls->end(); it++)
	    persistToXML(out, *it);
    }
    out << "</bb>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, RTL *rtl)
{
    out << "<rtl id=\"" << (int)rtl << "\" addr=\"" << (int)rtl->nativeAddr << "\">\n";
    for (std::list<Statement*>::iterator it = rtl->stmtList.begin(); it != rtl->stmtList.end(); it++) {
	out << "<stmt>\n";
	persistToXML(out, *it);
	out << "</stmt>\n";
    }
    out << "</rtl>\n";
}

void XMLProgParser::persistToXML(std::ostream &out, Statement *stmt)
{
    BoolStatement *b = dynamic_cast<BoolStatement*>(stmt);
    if (b) {
	out << "<boolstmt id=\"" << (int)stmt << "\" number=\"" << b->number << "\"";
	if (b->parent)
	    out << " parent=\"" << (int)b->parent << "\"";
	if (b->proc)
	    out << " proc=\"" << (int)b->proc << "\"";
	out << " jtcond=\"" << b->jtCond << "\"";
	out << " float=\"" << (int)b->bFloat << "\"";
	out << " size=\"" << b->size << "\"";
	out << ">\n";
	if (b->pCond) {
	    out << "<cond>\n";
	    persistToXML(out, b->pCond);
	    out << "</cond>\n";
	}
	if (b->pDest) {
	    out << "<dest>\n";
	    persistToXML(out, b->pDest);
	    out << "</dest>\n";
	}
	out << "</boolstmt>\n";
	return;
    }
    ReturnStatement *r = dynamic_cast<ReturnStatement*>(stmt);
    if (r) {
	out << "<returnstmt id=\"" << (int)stmt << "\" number=\"" << r->number 
	    << "\" computed=\"" << (int)r->m_isComputed << "\"";
	if (r->parent)
	    out << " parent=\"" << (int)r->parent << "\"";
	if (r->proc)
	    out << " proc=\"" << (int)r->proc << "\"";
	out << " bytesPopped=\"" << r->nBytesPopped << "\"";
	out << " retAddr=\"" << (int)r->retAddr << "\"";
	out << ">\n";

	for (unsigned i = 0; i < r->returns.size(); i++) {
	    out << "<returnexp>\n";
	    persistToXML(out, r->returns[i]);
	    out << "</returnexp>\n";
	}

	out << "</returnstmt>\n";
	return;
    }
    CallStatement *c = dynamic_cast<CallStatement*>(stmt);
    if (c) {
	out << "<callstmt id=\"" << (int)stmt << "\" number=\"" << c->number 
	    << "\" computed=\"" << (int)c->m_isComputed << "\"";
	if (c->parent)
	    out << " parent=\"" << (int)c->parent << "\"";
	if (c->proc)
	    out << " proc=\"" << (int)c->proc << "\"";
	out << " returnTypeSize=\"" << c->returnTypeSize << "\"";
	out << " returnAfterCall=\"" << (int)c->returnAfterCall << "\"";
	out << ">\n";

	if (c->pDest) {
	    out << "<dest";
	    if (c->procDest) 
		out << " proc=\"" << (int)c->procDest << "\"";
	    out << ">\n";
	    persistToXML(out, c->pDest);
	    out << "</dest>\n";
	}
	
	for (unsigned i = 0; i < c->arguments.size(); i++) {
	    out << "<argument>\n";
	    persistToXML(out, c->arguments[i]);
	    out << "</argument>\n";
	}

	for (unsigned i = 0; i < c->implicitArguments.size(); i++) {
	    out << "<implicitarg>\n";
	    persistToXML(out, c->implicitArguments[i]);
	    out << "</implicitarg>\n";
	}

	for (unsigned i = 0; i < c->returns.size(); i++) {
	    out << "<returnexp>\n";
	    persistToXML(out, c->returns[i]);
	    out << "</returnexp>\n";
	}

	out << "</callstmt>\n";
	return;
    }
    CaseStatement *ca = dynamic_cast<CaseStatement*>(stmt);
    if (ca) {
	out << "<casestmt id=\"" << (int)stmt << "\" number=\"" << ca->number 
	    << "\" computed=\"" << (int)ca->m_isComputed << "\"";
	if (ca->parent)
	    out << " parent=\"" << (int)ca->parent << "\"";
	if (ca->proc)
	    out << " proc=\"" << (int)ca->proc << "\"";
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
    BranchStatement *br = dynamic_cast<BranchStatement*>(stmt);
    if (br) {
	out << "<branchstmt id=\"" << (int)stmt << "\" number=\"" << br->number 
	    << "\" computed=\"" << (int)br->m_isComputed << "\""
	    << " jtcond=\"" << br->jtCond << "\" float=\"" << (int)br->bFloat << "\"";
	if (br->parent)
	    out << " parent=\"" << (int)br->parent << "\"";
	if (br->proc)
	    out << " proc=\"" << (int)br->proc << "\"";
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
    GotoStatement *g = dynamic_cast<GotoStatement*>(stmt);
    if (g) {
	out << "<gotostmt id=\"" << (int)stmt << "\" number=\"" << g->number << "\""
	    << " computed=\"" << (int) g->m_isComputed << "\"";
	if (g->parent)
	    out << " parent=\"" << (int)g->parent << "\"";
	if (g->proc)
	    out << " proc=\"" << (int)g->proc << "\"";
	out << ">\n";
	if (g->pDest) {
	    out << "<dest>\n";
	    persistToXML(out, g->pDest);
	    out << "</dest>\n";
	}
	out << "</gotostmt>\n";
	return;
    }
    Assign *a = dynamic_cast<Assign*>(stmt);
    if (a) {
	out << "<assign id=\"" << (int)stmt << "\" number=\"" << a->number << "\"";
	if (a->parent)
	    out << " parent=\"" << (int)a->parent << "\"";
	if (a->proc)
	    out << " proc=\"" << (int)a->proc << "\"";
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

