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
#include <memory>

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


    /// \copydoc IProject::getImage
    IBinaryImage* getImage() override { return m_image.get(); }

    /// \copydoc IProject::getImage
    const IBinaryImage* getImage() const override { return m_image.get(); }

    QByteArray& getFiledata()       override { return m_fileBytes; }
    const QByteArray& getFiledata() const override { return m_fileBytes; }

private:
    QByteArray m_fileBytes;
    std::shared_ptr<IBinaryImage> m_image; ///< raw memory interface
};
