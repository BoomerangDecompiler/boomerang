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

#include <QByteArray>

class Prog;
class IBinaryImage;


class Project : public IProject
{
public:
    Project();
    virtual ~Project();

    /// \copydoc IProject::loadBinaryFile
    bool loadBinaryFile(const QString& filePath) override;

    /// \copydoc IProject::loadSaveFile
    bool loadSaveFile(const QString& filePath) override;

    /// \copydoc IProject::writeSavefile
    bool writeSaveFile(const QString& filePath) override;

    /// \copydoc IProject::isBinaryLoaded
    bool isBinaryLoaded() const override;

    /// \copydoc IProject::unload
    void unload() override;


    QByteArray& getFiledata()       override { return m_fileBytes; }
    const QByteArray& getFiledata() const override { return m_fileBytes; }

    IBinaryImage *getOrCreateImage() override;

private:
    QByteArray m_fileBytes;
    IBinaryImage *m_image = nullptr; ///< raw memory interface
};
