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

#include "db/project.h"
#include "db/IBinaryImage.h"
#include "db/IBinarySymbols.h"

#include "loader/IFileLoader.h"

#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <cstdio>

#include <dlfcn.h> // for dlopen

#define LMMH(x)																								  \
	((unsigned)((Byte *)(&x))[0] + ((unsigned)((Byte *)(&x))[1] << 8) + ((unsigned)((Byte *)(&x))[2] << 16) + \
	 ((unsigned)((Byte *)(&x))[3] << 24))

QString BinaryFileFactory::m_basePath = "";


BinaryFileFactory::BinaryFileFactory()
{
	populatePlugins();
}


IFileLoader *BinaryFileFactory::load(const QString& sName)
{
	IBinaryImage       *image   = Boomerang::get()->getImage();
	IBinarySymbolTable *symbols = Boomerang::get()->getSymbols();

	image->reset();
	symbols->clear();

	// Find loader plugin to load file
	IFileLoader *ldr_iface = getInstanceFor(sName);

	if (ldr_iface == nullptr) {
		qWarning() << "unrecognised binary file format.";
		return nullptr;
	}

	ldr_iface->initialize(image, symbols);

	QFile srcFile(sName);

	if (false == srcFile.open(QFile::ReadOnly)) {
		qWarning() << "Opening '" << sName << "' failed";
		return nullptr;
	}

	Boomerang::get()->project()->getFiledata().clear();
	Boomerang::get()->project()->getFiledata() = srcFile.readAll();

	if (ldr_iface->loadFromMemory(Boomerang::get()->project()->getFiledata()) == 0) {
		qWarning() << "Loading '" << sName << "' failed";
		return nullptr;
	}

	image->calculateTextLimits();
	return ldr_iface;
}


IFileLoader *BinaryFileFactory::getInstanceFor(const QString& sName)
{
	QFile f(sName);

	if (!f.open(QFile::ReadOnly)) {
		qWarning() << "Unable to open binary file: " << sName;
		return nullptr;
	}

	// get the first plugin which is able to load the file
	for (std::shared_ptr<LoaderPlugin>& p : m_loaderPlugins) {
		f.seek(0); // reset the file offset for the next plugin
		IFileLoader *loader = p->get();

		if (loader->canLoad(f)) {
			return loader;
		}
	}

	return nullptr;
}


void BinaryFileFactory::populatePlugins()
{
	QDir pluginsDir(qApp->applicationDirPath());

	pluginsDir.cd("lib");

	if (!qApp->libraryPaths().contains(pluginsDir.absolutePath())) {
		qApp->addLibraryPath(pluginsDir.absolutePath());
	}

	for (QString fileName : pluginsDir.entryList(QDir::Files)) {
		std::string sofilename = pluginsDir.absoluteFilePath(fileName).toUtf8().constData();

		try {
			std::shared_ptr<LoaderPlugin> loaderPlugin(new LoaderPlugin(sofilename));
			m_loaderPlugins.push_back(loaderPlugin);
		}
		catch (const char *errmsg) {
			qCritical() << "Unable to load plugin: " << errmsg;
		}
	}

	if (m_loaderPlugins.empty()) {
		qCritical() << "No loader plugins found !";
	}
}


void BinaryFileFactory::unload()
{
}
