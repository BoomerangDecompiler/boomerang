#pragma once

#include <QtTest/QTest>

#include <memory>

class IProject;


/**
 * Test the Project class.
 */
class ProjectTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    /// Test the import binary function.
    void testLoadBinaryFile();

    // test loading/writing to/from a save file
    void testLoadSaveFile();
    void testWriteSaveFile();

    // test whether a binary is loaded after loading unloading
    void testIsBinaryLoaded();
};
