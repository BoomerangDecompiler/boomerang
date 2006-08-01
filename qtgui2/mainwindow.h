
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_boomerang.h"
#include "ui_about.h"
#include "types.h"
#include <vector>
#include <map>
#include <set>

class DecompilerThread;
class QToolButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

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
	void on_inputFileComboBox_editTextChanged(const QString &text);
	void on_inputFileComboBox_currentIndexChanged(const QString &text);
	void on_outputPathComboBox_editTextChanged(QString &text);
	void showConsideringProc(const QString &parent, const QString &name);
	void showDecompilingProc(const QString &name);
	void showNewUserProc(const QString &name, unsigned int addr);
	void showNewLibProc(const QString &name, const QString &params);
	void showRemoveUserProc(const QString &name, unsigned int addr);
	void showRemoveLibProc(const QString &name);
	void showNewEntrypoint(unsigned int addr, const QString &name);
	void showMachineType(const QString &machine);
	void showNewCluster(const QString &name);
	void showNewProcInCluster(const QString &name, const QString &cluster);
	void showDebuggingPoint(const QString &name, const QString &description);
	void showNewSection(const QString &name, unsigned int start, unsigned int end);
	void showRTLEditor(const QString &name);

	void on_clusters_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_decompileProcsTreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_actionEnable_toggled(bool b);
	void on_actionStep_activated();
	void on_userProcs_horizontalHeader_sectionClicked(int logicalIndex);
	void on_userProcs_cellDoubleClicked(int row, int column);
	void on_userProcs_cellChanged(int row, int column);
	void on_libProcs_cellDoubleClicked(int row, int column);
	void on_actionOpen_activated();
	void on_actionSave_activated();
	void on_actionClose_activated();
	void on_actionAbout_activated();
	void on_actionAboutQt_activated();
	void on_tabWidget_currentChanged(int index);

	void on_actionCut_activated();
	void on_actionCopy_activated();
	void on_actionPaste_activated();
	void on_actionDelete_activated();
	void on_actionFind_activated();
	void on_actionFind_Next_activated();
	void on_actionGo_To_activated();
	void on_actionSelect_All_activated();

	void on_actionLoad_activated();
	void on_actionDecode_activated();
	void on_actionDecompile_activated();
	void on_actionGenerate_Code_activated();
	void on_actionStructs_activated();
	void on_structName_returnPressed();

	void on_actionBoomerang_Website_activated();

	void on_enableDFTAcheckBox_toggled(bool b);

    void on_entrypoints_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous);
    void on_addButton_pressed();
    void on_removeButton_pressed();

	void closeCurrentTab();
	void currentTabTextChanged();

protected:
	void showInitPage();
	void saveSettings();

private:
    Ui::MainWindow ui;
    DecompilerThread *decompilerThread;

	QToolButton *step;

	int decompiledCount, codeGenCount;
	std::map<QWidget*, QString> openFiles;
	std::set<QWidget*> signatureFiles;

	QWidget *structs;
	bool loadingSettings;
};

#endif

