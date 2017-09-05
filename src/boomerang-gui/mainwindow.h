#pragma once

#include "boomerang/util/Address.h"

#include <QMainWindow>
#include <vector>
#include <map>
#include <set>


class DecompilerThread;
class QToolButton;
class QTreeWidgetItem;
class QTableWidgetItem;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void errorLoadingFile();

public slots:
    void loadComplete();
    void decodeComplete();
    void decompileComplete();
    void generateCodeComplete();
    void showLoadPage();
    void showDecodePage();
    void showDecompilePage();
    void showGenerateCodePage();
    void on_inputFileBrowseButton_clicked();
    void on_outputPathBrowseButton_clicked();
    void on_inputFileComboBox_currentIndexChanged(const QString& text);
    void on_outputPathComboBox_currentIndexChanged(const QString& text);
    void showConsideringProc(const QString& parent, const QString& name);
    void showDecompilingProc(const QString& name);
    void showNewUserProc(const QString& name, Address addr);
    void showNewLibProc(const QString& name, const QString& params);
    void showRemoveUserProc(const QString& name, Address addr);
    void showRemoveLibProc(const QString& name);
    void showNewEntrypoint(Address addr, const QString& name);
    void showMachineType(const QString& machine);
    void showNewCluster(const QString& name);
    void showNewProcInCluster(const QString& name, const QString& cluster);
    void showDebuggingPoint(const QString& name, const QString& description);
    void showNewSection(const QString& name, Address start, Address end);
    void showRTLEditor(const QString& name);

    void on_clusters_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_decompileProcsTreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_actionEnable_toggled(bool b);
    void on_actionStep_triggered();
    void onUserProcsHorizontalHeaderSectionClicked(int logicalIndex);
    void on_userProcs_cellDoubleClicked(int row, int column);
    void on_userProcs_cellChanged(int row, int column);
    void on_libProcs_cellDoubleClicked(int row, int column);
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionClose_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();
    void on_tabWidget_currentChanged(int index);

    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionDelete_triggered();
    void on_actionFind_triggered();
    void on_actionFind_Next_triggered();
    void on_actionGo_To_triggered();
    void on_actionSelect_All_triggered();

    void on_actionLoad_triggered();
    void on_actionDecode_triggered();
    void on_actionDecompile_triggered();
    void on_actionGenerate_Code_triggered();
    void on_actionStructs_triggered();
    void on_structName_returnPressed();

    void on_actionBoomerang_Website_triggered();

    void on_enableNoDecodeChildren_toggled(bool b);

    void on_entrypoints_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_addButton_pressed();
    void on_removeButton_pressed();

    void closeCurrentTab();
    void currentTabTextChanged();

protected:
    void showInitPage();
    void saveSettings();

private slots:
    void on_actionLoggingOptions_triggered();
    void on_cmb_typeRecoveryEngine_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    DecompilerThread *decompilerThread;

    QToolButton *step;

    int decompiledCount, codeGenCount;
    std::map<QWidget *, QString> openFiles;
    std::set<QWidget *> signatureFiles;

    QWidget *structs;
    bool loadingSettings;
};
