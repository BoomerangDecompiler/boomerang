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

#include "IProject.h"

#include <QtCore/QObject>
#include <QtCore/QIODevice>

class Prog;
class IBinaryImage;

class Project : public QObject, public IProject
{
	Q_OBJECT
	QByteArray file_bytes;
	IBinaryImage *Image = nullptr; // raw memory interface
	Prog *Program;                 // program interface
	ITypeRecovery *type_recovery_engine;

public:
	virtual ~Project();
	bool serializeTo(QIODevice& dev);
	bool serializeFrom(QIODevice& dev);

	QByteArray& filedata() override { return file_bytes; }
	IBinaryImage *image() override;

	ITypeRecovery *typeEngine() override { return type_recovery_engine; }
	void typeEngine(ITypeRecovery *e) override { type_recovery_engine = e; }
};
