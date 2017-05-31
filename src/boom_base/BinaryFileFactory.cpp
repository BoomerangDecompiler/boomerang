/* File: BinaryFileFactory.cpp
 * Desc: This file contains the implementation of the factory function
 * BinaryFile::getInstanceFor(), and also BinaryFile::Load()
 *
 * This function determines the type of a binary and loads the appropriate
 * loader class dynamically.
 */

#include "BinaryFile.h"
#include "log.h"
#include "boomerang.h"

#include "include/project.h"
#include "include/IBinaryImage.h"
#include "include/IBinarySymbols.h"

#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <cstdio>

#define LMMH(x)																								  \
	((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
	 ((unsigned)((Byte *)(&x))[3] << 24))

QString BinaryFileFactory::m_base_path = "";

QObject *BinaryFileFactory::Load(const QString& sName)
{
	Boomerang    *boom  = Boomerang::get();
	IBinaryImage *Image = boom->getImage();

	Image->reset();
	boom->getSymbols()->clear();
	QObject         *pBF       = getInstanceFor(sName);
	LoaderInterface *ldr_iface = qobject_cast<LoaderInterface *>(pBF);

	if (ldr_iface == nullptr) {
		qWarning() << "unrecognised binary file format.";
		return nullptr;
	}

	ldr_iface->initialize(boom);
	ldr_iface->Close();
	QFile src_file(sName);

	if (false == src_file.open(QFile::ReadOnly)) {
		qWarning() << "Opening '" << sName << "' failed";
		delete pBF;
		return nullptr;
	}

	boom->project()->filedata().clear();
	boom->project()->filedata() = src_file.readAll();

	if (ldr_iface->LoadFromMemory(boom->project()->filedata()) == 0) {
		qWarning() << "Loading '" << sName << "' failed";
		delete pBF;
		return nullptr;
	}

	Image->calculateTextLimits();
	return pBF;
}


//static QString selectPluginForFile(QIODevice &f) {
//    QString libName;
//    unsigned char buf[64];
//    f.read((char *)buf,sizeof(buf));
//    if (buf[0] == 0x4c && buf[1] == 0x01) {
//        libName = "IntelCoffFile";
//    } else {
//        qWarning() << "Unrecognised binary format";
//        return "";
//    }
//    return libName;
//}

/**
 * Test all plugins agains the file, select the one with the best match, and then return an
 * instance of the appropriate subclass.
 * \param sName - name of the file to load
 * \return Instance of the plugin that can load the file with given \a sName
 */
QObject *BinaryFileFactory::getInstanceFor(const QString& sName)
{
	QFile f(sName);

	if (!f.open(QFile::ReadOnly)) {
		qWarning() << "Unable to open binary file: " << sName;
		return nullptr;
	}

	std::vector<std::pair<QObject *, int> > scores;

	for (QObject *plug : m_loader_plugins) {
		LoaderInterface *ldr_iface = qobject_cast<LoaderInterface *>(plug);
		f.seek(0); // reset the file offset for the next plugin
		int score = ldr_iface->canLoad(f);

		if (score) {
			scores.emplace_back(plug, score);
		}
	}

	int     best_score = 0;
	QObject *result    = nullptr;

	for (auto& pr : scores) {
		if (pr.second > best_score) {
			result     = pr.first;
			best_score = pr.second;
		}
	}

	return result;
}


void BinaryFileFactory::populatePlugins()
{
	QDir pluginsDir(qApp->applicationDirPath());

	pluginsDir.cd("lib");

	if (!qApp->libraryPaths().contains(pluginsDir.absolutePath())) {
		qApp->addLibraryPath(pluginsDir.absolutePath());
	}

	for (QString fileName : pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject       *plugin = loader.instance();

		if (plugin) {
			m_loader_plugins.push_back(plugin);
		}
		else {
			qCritical() << loader.errorString();
		}
	}

	if (m_loader_plugins.empty()) {
		qCritical() << "No loader plugins found !";
	}
}


void BinaryFileFactory::UnLoad()
{
}
