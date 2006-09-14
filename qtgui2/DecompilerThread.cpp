
#include <QtGui>

#include "DecompilerThread.h"

#include "gc.h"

#include "boomerang.h"
#include "log.h"
#include "prog.h"
#include "frontend.h"
#include "proc.h"
#include "signature.h"
#include "cluster.h"
#include <sstream>

#undef NO_ADDRESS
#define NO_ADDRESS ((ADDRESS)-1)

Qt::HANDLE threadToCollect = 0;

void* operator new(size_t n) {
	Qt::HANDLE curThreadId = QThread::currentThreadId();
	if (curThreadId == threadToCollect)
		return GC_malloc(n);
	else
		return GC_malloc_uncollectable(n);	// Don't collect, but mark
}

void operator delete(void* p) {
	Qt::HANDLE curThreadId = QThread::currentThreadId();
	if (curThreadId != threadToCollect)
		GC_free(p); // Important to call this if you call GC_malloc_uncollectable
}

void DecompilerThread::run()
{
	threadToCollect = QThread::currentThreadId();

	Boomerang::get()->setOutputDirectory(".\\output\\");
    //Boomerang::get()->vFlag = true;
	//Boomerang::get()->traceDecoder = true;

	decompiler = new Decompiler();
	decompiler->moveToThread(this);

	Boomerang::get()->addWatcher(decompiler);

	this->setPriority(QThread::LowPriority);

	exec();
}

Decompiler *DecompilerThread::getDecompiler()
{
	while (decompiler == NULL)
		msleep(10);
	return decompiler;
}

void Decompiler::setUseDFTA(bool d) {
	Boomerang::get()->dfaTypeAnalysis = d;
}

void Decompiler::setNoDecodeChildren(bool d) {
    Boomerang::get()->noDecodeChildren = d;
}

void Decompiler::addEntryPoint(ADDRESS a, const char *nam) {
    user_entrypoints.push_back(a);
    fe->AddSymbol(a, nam);
}

void Decompiler::removeEntryPoint(ADDRESS a) {
    for (std::vector<ADDRESS>::iterator it = user_entrypoints.begin(); it != user_entrypoints.end(); it++)
        if (*it == a) {
            user_entrypoints.erase(it);
            break;
        }
}

void Decompiler::changeInputFile(const QString &f) 
{
	filename = f;
}

void Decompiler::changeOutputPath(const QString &path)
{
	Boomerang::get()->setOutputDirectory((const char *)path.toAscii());
}

void Decompiler::load()
{
	emit loading();

	prog = new Prog();
	fe = FrontEnd::Load(strdup(filename.toAscii()), prog);
	if (fe == NULL) {
		emit machineType(QString("unavailable: Load Failed!"));
		return;
	}
	prog->setFrontEnd(fe);
	fe->readLibraryCatalog();

	switch(prog->getMachine()) {
		case MACHINE_PENTIUM:
			emit machineType(QString("pentium"));
			break;
		case MACHINE_SPARC:
			emit machineType(QString("sparc"));
			break;
		case MACHINE_HPRISC:
			emit machineType(QString("hprisc"));
			break;
		case MACHINE_PALM:
			emit machineType(QString("palm"));
			break;
		case MACHINE_PPC:
			emit machineType(QString("ppc"));
			break;
		case MACHINE_ST20:
			emit machineType(QString("st20"));
			break;
	}

	QStringList entrypointStrings;
	std::vector<ADDRESS> entrypoints = fe->getEntryPoints();
	for (unsigned int i = 0; i < entrypoints.size(); i++) {
        user_entrypoints.push_back(entrypoints[i]);
		emit newEntrypoint(entrypoints[i], fe->getBinaryFile()->SymbolByAddress(entrypoints[i]));
	}

	for (int i = 1; i < fe->getBinaryFile()->GetNumSections(); i++) {
		PSectionInfo section = fe->getBinaryFile()->GetSectionInfo(i);
		emit newSection(section->pSectionName, section->uNativeAddr, section->uNativeAddr + section->uSectionSize);
	}

	emit loadCompleted();
}

void Decompiler::decode()
{
	emit decoding();

	bool gotMain;
	ADDRESS a = fe->getMainEntryPoint(gotMain);
	for (unsigned int i = 0; i < user_entrypoints.size(); i++) 
        if (user_entrypoints[i] == a) {
        	fe->decode(prog, true, NULL);
            break;
        }

    for (unsigned int i = 0; i < user_entrypoints.size(); i++) {
        prog->decodeEntryPoint(user_entrypoints[i]);
	}

    if (!Boomerang::get()->noDecodeChildren) {
	    // decode anything undecoded
	    fe->decode(prog, NO_ADDRESS);
    }

	prog->finishDecode();

	emit decodeCompleted();
}

void Decompiler::decompile()
{
	emit decompiling();

	prog->decompile();

	emit decompileCompleted();
}

void Decompiler::emitClusterAndChildren(Cluster *root)
{
	emit newCluster(QString(root->getName()));
	for (unsigned int i = 0; i < root->getNumChildren(); i++)
		emitClusterAndChildren(root->getChild(i));
}

void Decompiler::generateCode()
{
	emit generatingCode();

	prog->generateCode();

	Cluster *root = prog->getRootCluster();
	if (root)
		emitClusterAndChildren(root);
	std::list<Proc*>::iterator it;
	for (UserProc *p = prog->getFirstUserProc(it); p; p = prog->getNextUserProc(it)) {
		emit newProcInCluster(QString(p->getName()), QString(p->getCluster()->getName()));
	}

	emit generateCodeCompleted();
}

const char *Decompiler::procStatus(UserProc *p)
{
	switch(p->getStatus()) {
	case PROC_UNDECODED:
		return "undecoded";
	case PROC_DECODED:
		return "decoded";
	case PROC_SORTED:
		return "sorted";
	case PROC_VISITED:
		return "visited";
	case PROC_INCYCLE:
		return "in cycle";
	case PROC_PRESERVEDS:
		return "preserveds";
	case PROC_EARLYDONE:
		return "early done";
	case PROC_FINAL:
		return "final";
	case PROC_CODE_GENERATED:
		return "code generated";
	}
	return "unknown";
}

void Decompiler::alert_considering(Proc *parent, Proc *p)
{
	emit consideringProc(QString(parent ? parent->getName() : ""), QString(p->getName()));
}

void Decompiler::alert_decompiling(UserProc *p)
{
	emit decompilingProc(QString(p->getName()));
}

void Decompiler::alert_new(Proc *p)
{
	if (p->isLib()) {
		QString params;
		if (p->getSignature() == NULL || p->getSignature()->isUnknown())
			params = "<unknown>";
		else {
			for (unsigned int i = 0; i < p->getSignature()->getNumParams(); i++) {
				Type *ty = p->getSignature()->getParamType(i);
				params.append(ty->getCtype());
				params.append(" ");
				params.append(p->getSignature()->getParamName(i));
				if (i != p->getSignature()->getNumParams()-1)
					params.append(", ");
			}
		}
		emit newLibProc(QString(p->getName()), params);
	} else {
		emit newUserProc(QString(p->getName()), p->getNativeAddress());
	}
}

void Decompiler::alert_remove(Proc *p)
{
	if (p->isLib()) {
		emit removeLibProc(QString(p->getName()));
	} else {
		emit removeUserProc(QString(p->getName()), p->getNativeAddress());
	}
}

void Decompiler::alert_update_signature(Proc *p)
{
	alert_new(p);
}


bool Decompiler::getRtlForProc(const QString &name, QString &rtl)
{
	Proc *p = prog->findProc((const char *)name.toAscii());
	if (p->isLib())
		return false;
	UserProc *up = (UserProc*)p;
	std::ostringstream os;
	up->print(os, true);
	rtl = os.str().c_str();
	return true;
}

void Decompiler::alert_decompile_debug_point(UserProc *p, const char *description)
{
    LOG << p->getName() << ": " << description << "\n";
	if (debugging) {
		waiting = true;
		emit debuggingPoint(QString(p->getName()), QString(description));
		while (waiting) {
			thread()->wait(10);
		}		
	}
}

void Decompiler::stopWaiting()
{
	waiting = false;
}

const char *Decompiler::getSigFile(const QString &name)
{
	Proc *p = prog->findProc((const char *)name.toAscii());
	if (p == NULL || !p->isLib() || p->getSignature() == NULL)
		return NULL;
	return p->getSignature()->getSigFile();
}

const char *Decompiler::getClusterFile(const QString &name)
{
	Cluster *c = prog->findCluster((const char *)name.toAscii());
	if (c == NULL)
		return NULL;
	return c->getOutPath("c");
}

void Decompiler::rereadLibSignatures()
{
	prog->rereadLibSignatures();
}

void Decompiler::renameProc(const QString &oldName, const QString &newName)
{
	Proc *p = prog->findProc((const char *)oldName.toAscii());
	if (p)
		p->setName((const char *)newName.toAscii());
}

void Decompiler::getCompoundMembers(const QString &name, QTableWidget *tbl)
{
	Type *ty = NamedType::getNamedType((const char *)name.toAscii());
	tbl->setRowCount(0);
	if (ty == NULL || !ty->resolvesToCompound())
		return;
	CompoundType *c = ty->asCompound();
	for (unsigned int i = 0; i < c->getNumTypes(); i++) {
		tbl->setRowCount(tbl->rowCount() + 1);
		tbl->setItem(tbl->rowCount() - 1, 0, new QTableWidgetItem(tr("%1").arg(c->getOffsetTo(i))));
		tbl->setItem(tbl->rowCount() - 1, 1, new QTableWidgetItem(tr("%1").arg(c->getOffsetTo(i) / 8)));
		tbl->setItem(tbl->rowCount() - 1, 2, new QTableWidgetItem(QString(c->getName(i))));
		tbl->setItem(tbl->rowCount() - 1, 3, new QTableWidgetItem(tr("%1").arg(c->getType(i)->getSize())));
	}
}
