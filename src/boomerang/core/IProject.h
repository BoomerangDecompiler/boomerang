#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


class BinaryFile;
class ITypeRecovery;
class Prog;
class QString;
class Module;


/**
 * The Project interface class
 */
class IProject
{
public:
    virtual ~IProject() = default;

public:
    /**
     * Import a binary file from \p filePath.
     * Loads the binary file and decodes it.
     * If a binary file is already loaded, it is unloaded first (all unsaved data is lost).
     * \returns whether loading was successful.
     */
    virtual bool loadBinaryFile(const QString& filePath) = 0;

    /**
     * Checks if the project contains a loaded binary.
     */
    virtual bool isBinaryLoaded() const = 0;

    /**
     * Unloads the loaded binary file.
     * If there is no loaded binary, nothing happens.
     */
    virtual void unloadBinaryFile() = 0;

    /**
     * Loads a saved file from \p filePath.
     * If a binary file is already loaded, it is unloaded first (all unsaved data is lost).
     * \returns whether loading was successful.
     */
    virtual bool loadSaveFile(const QString& filePath) = 0;

    /**
     * Saves data to the save file at \p filePath.
     * If the file already exists, it is overwritten.
     * \returns whether saving was successful.
     */
    virtual bool writeSaveFile(const QString& filePath) = 0;

    /**
     * Decodes the loaded binary file.
     * \returns true on success, false if no binary is loaded or an error occurred.
     */
    virtual bool decodeBinaryFile() = 0;

    /**
     * Decompiles the decoded binary file.
     * \returns true on success, false if no binary is decoded or an error occurred.
     */
    virtual bool decompileBinaryFile() = 0;

    /**
     * Genereate code for the decompiled binary file.
     */
    virtual bool generateCode(Module *module = nullptr) = 0;

public:
    virtual BinaryFile *getLoadedBinaryFile() = 0;
    virtual const BinaryFile *getLoadedBinaryFile() const = 0;

    /// \returns the type recovery engine
    virtual ITypeRecovery *getTypeRecoveryEngine() const = 0;

    virtual const Prog *getProg() const = 0;
    virtual Prog *getProg() = 0;
};
