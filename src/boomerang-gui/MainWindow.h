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


#include "boomerang/util/Address.h"

#include <QMainWindow>
#include <QThread>

#include <map>
#include <set>
#include <vector>


class QToolButton;
class QTreeWidgetItem;
class QTableWidgetItem;
class Decompiler;


namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void errorLoadingFile();

signals:
    void librarySignaturesOutdated();
    void entryPointAdded(Address entryAddr, const QString &name);
    void entryPointRemoved(Address entryAddr);

public slots:
    void loadComplete();
    void decodeComplete();
    void decompileComplete();
    void generateCodeComplete();
    void showLoadPage();
    void showDecodePage();
    void showDecompilePage();
    void showGenerateCodePage();
    void on_btnInputFileBrowse_clicked();
    void on_btnOutputPathBrowse_clicked();
    void on_cbInputFile_currentIndexChanged(const QString &text);
    void on_cbOutputPath_currentIndexChanged(const QString &text);
    void showConsideringProc(const QString &parent, const QString &name);
    void showDecompilingProc(const QString &name);
    void showNewUserProc(const QString &name, Address addr);
    void showNewLibProc(const QString &name, const QString &params);
    void showRemoveUserProc(const QString &name, Address addr);
    void showRemoveLibProc(const QString &name);
    void showNewEntrypoint(Address addr, const QString &name);
    void showMachineType(const QString &machine);
    void showNewCluster(const QString &name);
    void showNewProcInCluster(const QString &name, const QString &cluster);
    void showDebuggingPoint(const QString &name, const QString &description);
    void showNewSection(const QString &name, Address start, Address end);
    void showRTLEditor(const QString &name);

    void on_twModuleTree_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_twProcTree_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_actDebugEnabled_toggled(bool b);
    void on_actDebugStep_triggered();
    void onUserProcsHorizontalHeaderSectionClicked(int logicalIndex);
    void on_tblUserProcs_cellDoubleClicked(int row, int column);
    void on_tblUserProcs_cellChanged(int row, int column);
    void on_tblLibProcs_cellDoubleClicked(int row, int column);
    void on_actNewProject_triggered();
    void on_actSaveProject_triggered();
    void on_actCloseProject_triggered();
    void on_actAboutBoomerang_triggered();
    void on_actAboutQt_triggered();
    void on_tabWidget_currentChanged(int index);

    void on_actCut_triggered();
    void on_actCopy_triggered();
    void on_actPaste_triggered();
    void on_actDelete_triggered();
    void on_actFind_triggered();
    void on_actFindNext_triggered();
    void on_actGoTo_triggered();
    void on_actSelectAll_triggered();

    void on_actLoad_triggered();
    void on_actDecode_triggered();
    void on_actDecompile_triggered();
    void on_actGenerateCode_triggered();
    void on_actStructs_triggered();
    void on_edtStructName_returnPressed();

    void on_actBoomerangWebsite_triggered();

    void on_tblEntryPoints_currentItemChanged(QTableWidgetItem *current,
                                              QTableWidgetItem *previous);
    void on_btnEntryPointAdd_pressed();
    void on_btnEntryPointRemove_pressed();

    void closeCurrentTab();
    void currentTabTextChanged();

protected:
    void showInitPage();
    void saveSettings();

private slots:
    void on_actSettings_triggered();

private:
    Ui::MainWindow *ui = nullptr;

    QThread m_decompilerThread;
    Decompiler *m_decompiler = nullptr;

    bool m_loadingSettings   = false;
    int m_numDecompiledProcs = 0;
    int m_numCodeGenProcs    = 0;

    std::map<QWidget *, QString> m_openFiles;
    std::set<QWidget *> m_signatureFiles;

    QToolButton *m_debugStep = nullptr;
    QWidget *m_structsView   = nullptr;
};
