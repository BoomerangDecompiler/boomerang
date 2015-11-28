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
#ifndef __PROJECT_H__
#define __PROJECT_H__

#include "IProject.h"

#include <QtCore/QObject>
#include <QtCore/QIODevice>

class Prog;
class IBinaryImage;

class Project : public QObject, public IProject {
    Q_OBJECT
    QByteArray file_bytes;
    IBinaryImage *Image=nullptr; // raw memory interface
    Prog *Program; // program interface

public:
    virtual ~Project();
    bool serializeTo(QIODevice &dev);
    bool serializeFrom(QIODevice &dev);

    QByteArray &filedata() override { return file_bytes; }
    IBinaryImage *image() override;

};
#endif
