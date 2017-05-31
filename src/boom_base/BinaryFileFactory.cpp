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
#include "db/IBinaryImage.h"
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

QString BinaryFileFactory::m_basePath = "";


BinaryFileFactory::BinaryFileFactory()
{
	populatePlugins();
}


QObject *BinaryFileFactory::load(const QString& sName)
{
	Boomerang    *boom  = Boomerang::get();
	IBinaryImage *image = boom->getImage();

	image->reset();
	boom->getSymbols()->clear();
	QObject         *pBF       = getInstanceFor(sName);
	LoaderInterface *ldr_iface = qobject_cast<LoaderInterface *>(pBF);

	if (ldr_iface == nullptr) {
		qWarning() << "unrecognised binary file format.";
		return nullptr;
	}

	ldr_iface->initialize(boom);
	ldr_iface->close();
	QFile srcFile(sName);

	if (false == srcFile.open(QFile::ReadOnly)) {
		qWarning() << "Opening '" << sName << "' failed";
		delete pBF;
		return nullptr;
	}

	boom->project()->filedata().clear();
	boom->project()->filedata() = srcFile.readAll();

	if (ldr_iface->loadFromMemory(boom->project()->filedata()) == 0) {
		qWarning() << "Loading '" << sName << "' failed";
		delete pBF;
		return nullptr;
	}

	image->calculateTextLimits();
	return pBF;
}


QObject *BinaryFileFactory::getInstanceFor(const QString& sName)
{
	QFile f(sName);

	if (!f.open(QFile::ReadOnly)) {
		qWarning() << "Unable to open binary file: " << sName;
		return nullptr;
	}

	std::vector<std::pair<QObject *, int> > scores;

	for (QObject *plug : m_loaderPlugins) {
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
			m_loaderPlugins.push_back(plugin);
		}
		else {
			qCritical() << loader.errorString();
		}
	}

	if (m_loaderPlugins.empty()) {
		qCritical() << "No loader plugins found !";
	}
}


void BinaryFileFactory::unload()
{
}
