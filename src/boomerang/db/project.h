#pragma once

/*
 * Copyright (C) 2014-    Boomerang Project
 *
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file     project.h
 * \brief    This file contains the definition for the Project class
 ******************************************************************************/

#include "boomerang/db/IProject.h"

#include <QtCore/QObject>
#include <QtCore/QIODevice>

class Prog;
class IBinaryImage;

class Project : public QObject, public IProject
{
	Q_OBJECT

private:
	QByteArray m_fileBytes;
	IBinaryImage *m_image = nullptr; ///< raw memory interface
	Prog *m_program;                 ///< program interface
	ITypeRecovery *m_typeRecoveryEngine;

public:
	virtual ~Project();

	bool serializeTo(QIODevice& dev);
	bool serializeFrom(QIODevice& dev);

	QByteArray& getFiledata()       override { return m_fileBytes; }
	const QByteArray& getFiledata() const override { return m_fileBytes; }

	IBinaryImage *getOrCreateImage() override;

	const ITypeRecovery *getTypeEngine() const override { return m_typeRecoveryEngine; }
};
