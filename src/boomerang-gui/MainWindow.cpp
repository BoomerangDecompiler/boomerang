#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "MainWindow.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/type/TypeRecovery.h"

#include "boomerang-gui/Decompiler.h"
#include "boomerang-gui/RTLEditor.h"
#include "boomerang-gui/SettingsDlg.h"
#include "boomerang-gui/ui_MainWindow.h"
#include "boomerang-gui/ui_About.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QtWidgets>



Q_DECLARE_METATYPE(ITypeRecovery *)


MainWindow::MainWindow(QWidget *_parent)
    : QMainWindow(_parent)
    , ui(new Ui::MainWindow)
    , step(nullptr)
{
    ui->setupUi(this);

    qRegisterMetaType<Address>("ADDRESS");

    m_decompiler = new Decompiler();
    m_decompiler->moveToThread(&m_decompilerThread);

    m_decompilerThread.start();

    connect(m_decompiler, &Decompiler::moduleCreated, this, &MainWindow::showNewCluster);
    connect(m_decompiler, &Decompiler::functionAddedToModule, this, &MainWindow::showNewProcInCluster);
    connect(m_decompiler, &Decompiler::debugPointHit, this, &MainWindow::showDebuggingPoint);
    connect(m_decompiler, &Decompiler::loadingStarted, this, &MainWindow::showLoadPage);
    connect(m_decompiler, &Decompiler::decodingStarted, this, &MainWindow::showDecodePage);
    connect(m_decompiler, &Decompiler::decompilingStarted, this, &MainWindow::showDecompilePage);
    connect(m_decompiler, &Decompiler::generatingCodeStarted, this, &MainWindow::showGenerateCodePage);
    connect(m_decompiler, &Decompiler::loadCompleted, this, &MainWindow::loadComplete);
    connect(m_decompiler, &Decompiler::machineTypeChanged, this, &MainWindow::showMachineType);
    connect(m_decompiler, &Decompiler::entryPointAdded, this, &MainWindow::showNewEntrypoint);
    connect(m_decompiler, &Decompiler::decodeCompleted, this, &MainWindow::decodeComplete);
    connect(m_decompiler, &Decompiler::decompileCompleted, this, &MainWindow::decompileComplete);
    connect(m_decompiler, &Decompiler::generateCodeCompleted, this, &MainWindow::generateCodeComplete);
    connect(m_decompiler, &Decompiler::procDiscovered, this, &MainWindow::showConsideringProc);
    connect(m_decompiler, &Decompiler::procDecompileStarted, this, &MainWindow::showDecompilingProc);
    connect(m_decompiler, &Decompiler::userProcCreated, this, &MainWindow::showNewUserProc);
    connect(m_decompiler, &Decompiler::libProcCreated, this, &MainWindow::showNewLibProc);
    connect(m_decompiler, &Decompiler::userProcRemoved, this, &MainWindow::showRemoveUserProc);
    connect(m_decompiler, &Decompiler::libProcRemoved, this, &MainWindow::showRemoveLibProc);
    connect(m_decompiler, &Decompiler::sectionAdded, this, &MainWindow::showNewSection);

    connect(ui->btnToLoad, &QPushButton::clicked, this, [=]() {
        m_decompiler->loadInputFile(ui->inputFileComboBox->currentText(), ui->outputPathComboBox->currentText());
    });

    connect(ui->btnToDecode,         SIGNAL(clicked()), m_decompiler, SLOT(decode()));
    connect(ui->btnToDecompile,      SIGNAL(clicked()), m_decompiler, SLOT(decompile()));
    connect(ui->btnToGenerateCode,   SIGNAL(clicked()), m_decompiler, SLOT(generateCode()));

    connect(this, SIGNAL(librarySignaturesOutdated()), m_decompiler, SLOT(rereadLibSignatures()));
    connect(this, SIGNAL(entryPointAdded(Address, const QString&)), m_decompiler, SLOT(addEntryPoint(Address, const QString&)));
    connect(this, SIGNAL(entryPointRemoved(Address)), m_decompiler, SLOT(removeEntryPoint(Address)));

    ui->tblUserProcs->horizontalHeader()->disconnect(SIGNAL(sectionClicked(int)));
    connect(ui->tblUserProcs->horizontalHeader(), &QHeaderView::sectionClicked, this,
            &MainWindow::onUserProcsHorizontalHeaderSectionClicked);

    ui->tblUserProcs->verticalHeader()->hide();
    ui->tblLibProcs->verticalHeader()->hide();
    ui->sections->verticalHeader()->hide();
    ui->entrypoints->verticalHeader()->hide();
    ui->tblStructMembers->verticalHeader()->hide();

    QPushButton *closeButton = new QPushButton(QIcon(":/closetab.png"), "", ui->tabWidget);
    closeButton->setFixedSize(closeButton->iconSize());
    ui->tabWidget->setCornerWidget(closeButton);
    ui->tabWidget->cornerWidget()->show();
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));

    structs = ui->tabWidget->widget(1);
    ui->tabWidget->removeTab(1);

    showInitPage();
    setWindowTitle("Boomerang");

    loadingSettings = true;
    QSettings settings("Boomerang", "Boomerang");

    ui->inputFileComboBox->addItems(settings.value("inputfiles").toStringList());

    if (ui->inputFileComboBox->count() > 0) {
        int currentIdx = ui->inputFileComboBox->findText(settings.value("inputfile").toString());
        currentIdx = std::max(currentIdx, 0); // if selected input file could not be found, use last one
        ui->inputFileComboBox->setCurrentIndex(currentIdx);
    }

    ui->outputPathComboBox->addItems(settings.value("outputpaths").toStringList());

    if (ui->outputPathComboBox->count() > 0) {
        int currentIdx = ui->outputPathComboBox->findText(settings.value("outputpath").toString());
        currentIdx = std::max(currentIdx, 0); // if selected output path could not be found, use last one
        ui->outputPathComboBox->setCurrentIndex(currentIdx);
    }

    // check for a valid input file and output path
    ui->btnToLoad->setEnabled((ui->outputPathComboBox->count() > 0) && (ui->inputFileComboBox->count() > 0));

    loadingSettings = false;

    ui->inputFileComboBox->setEditable(false);
    ui->outputPathComboBox->setEditable(false);
    ui->inputFileComboBox->setMaxCount(15);
    ui->outputPathComboBox->setMaxCount(10);
}


MainWindow::~MainWindow()
{
    m_decompilerThread.quit();
    m_decompilerThread.wait();

    delete ui;
    delete m_decompiler;
}


void MainWindow::saveSettings()
{
    if (loadingSettings) {
        return;
    }

    QSettings settings("Boomerang", "Boomerang");

    // input files
    QStringList inputfiles;

    for (int n = 0; n < ui->inputFileComboBox->count(); n++) {
        inputfiles.append(ui->inputFileComboBox->itemText(n));
    }

    settings.setValue("inputfiles", inputfiles);
    settings.setValue("inputfile", ui->inputFileComboBox->currentText());

    // Output paths
    QStringList outputPaths;

    for (int n = 0; n < ui->outputPathComboBox->count(); n++) {
        outputPaths.append(ui->outputPathComboBox->itemText(n));
    }

    settings.setValue("outputpaths", outputPaths);
    settings.setValue("outputpath", ui->outputPathComboBox->currentText());
}


void MainWindow::on_btnInputFileBrowse_clicked()
{
    QString openFileDir = ".";

    if (ui->inputFileComboBox->currentIndex() != -1) {
        // try to use the directory of the last opened file as starting directory.
        QString lastUsedFile = ui->inputFileComboBox->itemText(ui->inputFileComboBox->currentIndex());

        if (!lastUsedFile.isEmpty()) {
            QFileInfo fi(lastUsedFile);
            openFileDir = fi.absolutePath();
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Select a file to decompile..."), openFileDir,
                                                    "Windows Binaries (*.exe *.dll *.scr *.sys);;Other Binaries (*)");

    if (fileName.isEmpty()) {
        // user cancelled
        return;
    }

    int existingIdx = ui->inputFileComboBox->findText(fileName);

    if (existingIdx == -1) {
        // file not in the cache
        ui->inputFileComboBox->insertItem(0, fileName);
        existingIdx = 0;
        saveSettings();
    }

    ui->inputFileComboBox->setCurrentIndex(existingIdx);

    // we now have at least one input file
    ui->btnToLoad->setEnabled(ui->outputPathComboBox->count() > 0);
}


void MainWindow::on_btnOutputPathBrowse_clicked()
{
    QString outputDir = QFileDialog::getExistingDirectory(this, tr("Select a location to write the output to..."), "output");

    if (outputDir.isEmpty()) {
        // user cancelled
        return;
    }

    int existingIdx = ui->outputPathComboBox->findText(outputDir);

    if (existingIdx == -1) {
        // directory not in the cache
        ui->outputPathComboBox->insertItem(0, outputDir);
        existingIdx = 0;
        saveSettings();
    }

    ui->outputPathComboBox->setCurrentIndex(existingIdx);

    // we now have at least one output directory
    ui->btnToLoad->setEnabled(ui->inputFileComboBox->count() > 0);
}


void MainWindow::on_inputFileComboBox_currentIndexChanged(const QString& )
{
    saveSettings();
}


void MainWindow::on_outputPathComboBox_currentIndexChanged(const QString& )
{
    saveSettings();
}


void MainWindow::closeCurrentTab()
{
    if (openFiles.find(ui->tabWidget->currentWidget()) != openFiles.end()) {
        on_actionCloseProject_triggered();
    }
    else if (ui->tabWidget->currentIndex() != 0) {
        ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    }
}


void MainWindow::currentTabTextChanged()
{
    QString text = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

    if (text.right(1) != "*") {
        ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), text.append("*"));
    }
}


void MainWindow::on_actionNewProject_triggered()
{
    // TODO handle action
}


void MainWindow::on_actionSaveProject_triggered()
{
    // TODO handle action
}


void MainWindow::on_actionCloseProject_triggered()
{
    // TODO handle action
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    ui->actionSaveProject->setEnabled(openFiles.find(ui->tabWidget->widget(index)) != openFiles.end());
    ui->actionCloseProject->setEnabled(openFiles.find(ui->tabWidget->widget(index)) != openFiles.end());
}


void MainWindow::errorLoadingFile()
{
}


void MainWindow::showInitPage()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecode->setEnabled(false);
    ui->btnToDecompile->setEnabled(false);
    ui->btnToGenerateCode->setEnabled(false);

    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);

    ui->stackedWidget->setCurrentIndex(0);
    ui->entrypoints->setRowCount(0);
    ui->tblUserProcs->setRowCount(0);
    ui->tblLibProcs->setRowCount(0);
    ui->twProcTree->clear();
    ui->twModuleTree->clear();

    decompiledCount = 0;
    codeGenCount = 0;

    ui->actionLoad->setEnabled(false);
    ui->actionDecode->setEnabled(false);
    ui->actionDecompile->setEnabled(false);
    ui->actionGenerate_Code->setEnabled(false);
}


void MainWindow::showLoadPage()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToLoad->setEnabled(false);

    ui->loadButton->setEnabled(true);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);

    ui->stackedWidget->setCurrentIndex(1);
    ui->actionLoad->setEnabled(true);
}


void MainWindow::showDecodePage()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecode->setEnabled(false);

    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(true);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);

    ui->stackedWidget->setCurrentIndex(2);

    if (!ui->actionDebugEnabled->isChecked()) {
        ui->tblUserProcs->removeColumn(2);
    }
    else {
        ui->tblUserProcs->setColumnCount(3);
        ui->tblUserProcs->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Debug")));
    }

    ui->actionDecode->setEnabled(true);
}


void MainWindow::showDecompilePage()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecompile->setEnabled(false);
    ui->loadButton->setEnabled(false);

    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(true);
    ui->generateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(3);

    ui->actionDecompile->setEnabled(true);
}


void MainWindow::showGenerateCodePage()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToGenerateCode->setEnabled(false);

    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(true);

    ui->stackedWidget->setCurrentIndex(4);
    ui->actionGenerate_Code->setEnabled(true);
}


void MainWindow::loadComplete()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecode->setEnabled(true);
    ui->btnToDecompile->setEnabled(false);
    ui->btnToGenerateCode->setEnabled(false);

    ui->loadButton->setEnabled(true);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);

    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::showMachineType(const QString& machine)
{
    ui->lbMachineType->setText(machine);
}


void MainWindow::showNewEntrypoint(Address addr, const QString& name)
{
    int nrows = ui->entrypoints->rowCount();

    ui->entrypoints->setRowCount(nrows + 1);
    ui->entrypoints->setItem(nrows, 0, new QTableWidgetItem(addr.toString()));
    ui->entrypoints->setItem(nrows, 1, new QTableWidgetItem(name));
    ui->entrypoints->resizeColumnsToContents();
    ui->entrypoints->resizeRowsToContents();
}


void MainWindow::decodeComplete()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecode->setEnabled(false);
    ui->btnToDecompile->setEnabled(true);
    ui->btnToGenerateCode->setEnabled(false);

    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(true);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(2);
}


void MainWindow::decompileComplete()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecode->setEnabled(false);
    ui->btnToDecompile->setEnabled(false);
    ui->btnToGenerateCode->setEnabled(true);

    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(true);
    ui->generateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(3);
}


void MainWindow::generateCodeComplete()
{
    ui->btnToLoad->setEnabled(false);
    ui->btnToDecode->setEnabled(false);
    ui->btnToDecompile->setEnabled(false);
    ui->btnToGenerateCode->setEnabled(false);

    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);

    ui->stackedWidget->setCurrentIndex(4);
}


void MainWindow::showConsideringProc(const QString& calledByName, const QString& procName)
{
    QList<QTreeWidgetItem *> foundit =
        ui->twProcTree->findItems(procName, Qt::MatchExactly | Qt::MatchRecursive);

    if (foundit.isEmpty()) {
        QStringList texts(procName);

        if (calledByName.isEmpty()) {
            ui->twProcTree->addTopLevelItem(new QTreeWidgetItem(texts));
        }
        else {
            QList<QTreeWidgetItem *> found =
                ui->twProcTree->findItems(calledByName, Qt::MatchExactly | Qt::MatchRecursive);

            if (!found.isEmpty()) {
                QTreeWidgetItem *n = new QTreeWidgetItem(found.first(), texts);
                n->setData(0, 1, procName);
                ui->twProcTree->expandItem(found.first());
                ui->twProcTree->scrollToItem(n);
                ui->twProcTree->setCurrentItem(n, 0);
            }
        }
    }
}


void MainWindow::showDecompilingProc(const QString& name)
{
    QList<QTreeWidgetItem *> foundit =
        ui->twProcTree->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);

    if (!foundit.isEmpty()) {
        ui->twProcTree->setCurrentItem(foundit.first(), 0);
        foundit.first()->setTextColor(0, QColor("blue"));
        decompiledCount++;
    }

    const int max = ui->tblUserProcs->rowCount();
    ui->progressDecompile->setRange(0, max);
    ui->progressDecompile->setValue(decompiledCount);
}


void MainWindow::showNewUserProc(const QString& name, Address addr)
{
    const int nrows = ui->tblUserProcs->rowCount();

    for (int i = 0; i < nrows; i++) {
        if (ui->tblUserProcs->item(i, 1)->text() == name) {
            return;
        }
    }

    const QString s = addr.toString();

    for (int i = 0; i < nrows; i++) {
        if (ui->tblUserProcs->item(i, 0)->text() == s) {
            return;
        }
    }

    ui->tblUserProcs->setRowCount(nrows + 1);
    ui->tblUserProcs->setItem(nrows, 0, new QTableWidgetItem(addr.toString()));
    ui->tblUserProcs->setItem(nrows, 1, new QTableWidgetItem(name));
    ui->tblUserProcs->item(nrows, 1)->setData(1, name);

    if (ui->actionDebugEnabled->isChecked()) {
        QTableWidgetItem *d = new QTableWidgetItem("");
        d->setCheckState(Qt::Checked);
        ui->tblUserProcs->setItem(nrows, 2, d);
    }

    ui->tblUserProcs->resizeColumnsToContents();
    ui->tblUserProcs->resizeRowsToContents();
}


void MainWindow::showNewLibProc(const QString& name, const QString& params)
{
    const int nrows = ui->tblLibProcs->rowCount();

    for (int i = 0; i < nrows; i++) {
        if (ui->tblLibProcs->item(i, 0)->text() == name) {
            ui->tblLibProcs->item(i, 1)->setText(params);
            return;
        }
    }

    ui->tblLibProcs->setRowCount(nrows + 1);
    ui->tblLibProcs->setItem(nrows, 0, new QTableWidgetItem(name));
    ui->tblLibProcs->setItem(nrows, 1, new QTableWidgetItem(params));
    ui->tblLibProcs->resizeColumnsToContents();
    ui->tblLibProcs->resizeRowsToContents();
}


void MainWindow::showRemoveUserProc(const QString& name, Address addr)
{
    Q_UNUSED(name);

    const int nrows = ui->tblUserProcs->rowCount();
    const QString addrString = addr.toString();

    for (int i = 0; i < nrows; i++) {
        if (ui->tblUserProcs->item(i, 0)->text() == addrString) {
            ui->tblUserProcs->removeRow(i);
            break;
        }
    }

    ui->tblUserProcs->resizeColumnsToContents();
    ui->tblUserProcs->resizeRowsToContents();
}


void MainWindow::showRemoveLibProc(const QString& name)
{
    const int nrows = ui->tblLibProcs->rowCount();

    for (int i = 0; i < nrows; i++) {
        if (ui->tblLibProcs->item(i, 0)->text() == name) {
            ui->tblLibProcs->removeRow(i);
            break;
        }
    }

    ui->tblLibProcs->resizeColumnsToContents();
    ui->tblLibProcs->resizeRowsToContents();
}


void MainWindow::showNewSection(const QString& name, Address start, Address end)
{
    const int nrows = ui->sections->rowCount();

    ui->sections->setRowCount(nrows + 1);
    ui->sections->setItem(nrows, 0, new QTableWidgetItem(name));
    ui->sections->setItem(nrows, 1, new QTableWidgetItem(start.toString()));
    ui->sections->setItem(nrows, 2, new QTableWidgetItem(end.toString()));
    ui->sections->sortItems(1, Qt::AscendingOrder);
    ui->sections->resizeColumnsToContents();
    ui->sections->resizeRowsToContents();
}


void MainWindow::showNewCluster(const QString& name)
{
    QString cname = name;

    cname = cname.append(".c");
    QTreeWidgetItem *n = new QTreeWidgetItem(QStringList(cname));
    ui->twModuleTree->addTopLevelItem(n);
    ui->twModuleTree->expandItem(n);
}


void MainWindow::showNewProcInCluster(const QString& name, const QString& cluster)
{
    QString cname = cluster;

    cname = cname.append(".c");
    QList<QTreeWidgetItem *> found = ui->twModuleTree->findItems(cname, Qt::MatchExactly);

    if (!found.isEmpty()) {
        QTreeWidgetItem *n = new QTreeWidgetItem(found.first(), QStringList(name));
        ui->twModuleTree->scrollToItem(n);
        ui->twModuleTree->setCurrentItem(n, 0);
        ui->twModuleTree->expandItem(found.first());
        codeGenCount++;
    }

    ui->progressGenerateCode->setRange(0, ui->tblUserProcs->rowCount());
    ui->progressGenerateCode->setValue(codeGenCount);
}


void MainWindow::showDebuggingPoint(const QString& name, const QString& description)
{
    QString msg = "debugging ";

    msg.append(name);
    msg.append(": ");
    msg.append(description);

    statusBar()->showMessage(msg);
    ui->actionDebugStep->setEnabled(true);

    for (int i = 0; i < ui->tblUserProcs->rowCount(); i++) {
        if ((ui->tblUserProcs->item(i, 1)->text() == name) &&
            (ui->tblUserProcs->item(i, 2)->checkState() != Qt::Checked)) {
            on_actionDebugStep_triggered();
            return;
        }
    }

    showRTLEditor(name);
}


void MainWindow::showRTLEditor(const QString& name)
{
    RTLEditor *n = nullptr;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->tabText(i) == name) {
            n = dynamic_cast<RTLEditor *>(ui->tabWidget->widget(i));
            break;
        }
    }

    if (n == nullptr) {
        n = new RTLEditor(m_decompiler, name);
        ui->tabWidget->addTab(n, name);
    }
    else {
        n->updateContents();
    }

    ui->tabWidget->setCurrentWidget(n);
}


void MainWindow::on_tblUserProcs_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    showRTLEditor(ui->tblUserProcs->item(row, 1)->text());
}


void MainWindow::on_tblUserProcs_cellChanged(int row, int column)
{
    if (column == 0) {
        // TODO: should we allow the user to change the address of a proc?
    }

    if (column == 1) {
        const QString oldName = ui->tblUserProcs->item(row, 1)->data(1).toString();
        const QString newName = ui->tblUserProcs->item(row, 1)->text();
        m_decompiler->renameProc(oldName, newName);
        ui->tblUserProcs->item(row, 1)->setData(1, newName);
    }
}


void MainWindow::on_twModuleTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QTreeWidgetItem *top = item;

    while (top->parent()) {
        top = top->parent();
    }

    QTextEdit *n = nullptr;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->tabText(i) == top->text(0)) {
            n = dynamic_cast<QTextEdit *>(ui->tabWidget->widget(i));
            break;
        }
    }

    if (n == nullptr) {
        n = new QTextEdit();
        QString name = top->text(0);
        name = name.left(name.lastIndexOf("."));
        QString filename = m_decompiler->getClusterFile(name);
        QFile   file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString     contents = in.readAll();
        file.close();
        n->insertPlainText(contents);
        openFiles[n] = filename;
        connect(n, SIGNAL(textChanged()), this, SLOT(currentTabTextChanged()));
        ui->tabWidget->addTab(n, top->text(0));
    }

    ui->tabWidget->setCurrentWidget(n);
}


void MainWindow::on_twProcTree_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    showRTLEditor(item->text(0));
}


void MainWindow::on_actionDebugEnabled_toggled(bool enabled)
{
    m_decompiler->setDebugEnabled(enabled);
    m_decompiler->stopWaiting();

    if (enabled) {
        statusBar()->show();

        if (step == nullptr) {
            step = new QToolButton();
            step->setToolButtonStyle(Qt::ToolButtonTextOnly);
            step->setText("Step");
            step->setDefaultAction(ui->actionDebugStep);
        }

        statusBar()->addPermanentWidget(step);
    }
    else {
        if (step) {
            statusBar()->removeWidget(step);
        }

        statusBar()->hide();
    }
}


void MainWindow::on_actionDebugStep_triggered()
{
    ui->actionDebugStep->setEnabled(false);
    m_decompiler->stopWaiting();
}


void MainWindow::onUserProcsHorizontalHeaderSectionClicked(int logicalIndex)
{
    if (logicalIndex == 2) {
        for (int i = 0; i < ui->tblUserProcs->rowCount(); i++) {
            if (ui->tblUserProcs->item(i, 2) == nullptr) {
                ui->tblUserProcs->setItem(i, 2, new QTableWidgetItem(""));
            }

            Qt::CheckState state = ui->tblUserProcs->item(i, 2)->checkState();
            ui->tblUserProcs->item(i, 2)->setCheckState(state == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    }
}


void MainWindow::on_tblLibProcs_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QString name = "";
    QString sigFile;
    QString params   = ui->tblLibProcs->item(row, 1)->text();
    bool    existing = true;

    if (params == "<unknown>") {
        existing = false;

        // uhh, time to guess?
        for (int i = row; i >= 0; i--) {
            params = ui->tblLibProcs->item(i, 1)->text();

            if (params != "<unknown>") {
                name = ui->tblLibProcs->item(i, 0)->text();
                break;
            }
        }

        if (name.isEmpty()) {
            return;
        }
    }
    else {
        name = ui->tblLibProcs->item(row, 0)->text();
    }

    sigFile = m_decompiler->getSigFile(name);
    QString filename = sigFile;

    int lastIndex = sigFile.lastIndexOf(QRegExp("[/\\\\]"));

    if (lastIndex != -1) {
        sigFile = sigFile.right(sigFile.length() - lastIndex - 1);
    }

    QString sigFileStar = sigFile;
    sigFileStar.append("*");

    QTextEdit *n = nullptr;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if ((ui->tabWidget->tabText(i) == sigFile) || (ui->tabWidget->tabText(i) == sigFileStar)) {
            n = dynamic_cast<QTextEdit *>(ui->tabWidget->widget(i));
            break;
        }
    }

    if (n == nullptr) {
        n = new QTextEdit();
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QTextStream in(&file);
        QString     contents = in.readAll();
        file.close();
        n->insertPlainText(contents);
        openFiles[n] = filename;
        signatureFiles.insert(n);
        connect(n, SIGNAL(textChanged()), this, SLOT(currentTabTextChanged()));
        ui->tabWidget->addTab(n, sigFile);
    }

    ui->tabWidget->setCurrentWidget(n);

    if (existing) {
        n->find(name, QTextDocument::FindBackward | QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords);
    }
    else {
        QTextCursor textCursor = n->textCursor();
        textCursor.clearSelection();
        textCursor.movePosition(QTextCursor::End);
        n->setTextCursor(textCursor);
        QString comment = "// unknown library proc: ";
        comment.append(ui->tblLibProcs->item(row, 0)->text());
        comment.append("\n");
        n->insertPlainText(comment);
    }
}


void MainWindow::on_actionCut_triggered()
{
    if (ui->tabWidget->currentIndex() != 0) {
        QTextEdit *n = dynamic_cast<QTextEdit *>(ui->tabWidget->currentWidget());

        if (n) {
            n->cut();
        }
    }
}


void MainWindow::on_actionCopy_triggered()
{
    if (ui->tabWidget->currentIndex() != 0) {
        QTextEdit *n = dynamic_cast<QTextEdit *>(ui->tabWidget->currentWidget());

        if (n) {
            n->copy();
        }
    }
}


void MainWindow::on_actionPaste_triggered()
{
    if (ui->tabWidget->currentIndex() != 0) {
        QTextEdit *n = dynamic_cast<QTextEdit *>(ui->tabWidget->currentWidget());

        if (n) {
            n->paste();
        }
    }
}


void MainWindow::on_actionDelete_triggered()
{
    if (ui->tabWidget->currentIndex() != 0) {
        QTextEdit *n = dynamic_cast<QTextEdit *>(ui->tabWidget->currentWidget());

        if (n) {
            n->textCursor().removeSelectedText();
        }
    }
}


void MainWindow::on_actionFind_triggered()
{
}


void MainWindow::on_actionFind_Next_triggered()
{
}


void MainWindow::on_actionGo_To_triggered()
{
}


void MainWindow::on_actionSelect_All_triggered()
{
    if (ui->tabWidget->currentIndex() != 0) {
        QTextEdit *n = dynamic_cast<QTextEdit *>(ui->tabWidget->currentWidget());

        if (n) {
            n->selectAll();
        }
    }
}


void MainWindow::on_actionLoad_triggered()
{
    saveSettings();
    showLoadPage();
}


void MainWindow::on_actionDecode_triggered()
{
    showDecodePage();
}


void MainWindow::on_actionDecompile_triggered()
{
    showDecompilePage();
}


void MainWindow::on_actionGenerate_Code_triggered()
{
    showGenerateCodePage();
}


void MainWindow::on_actionStructs_triggered()
{
    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->widget(i) == structs) {
            return;
        }
    }

    ui->tabWidget->addTab(structs, "Structs");
    ui->tabWidget->setCurrentWidget(structs);
}


void MainWindow::on_edStructName_returnPressed()
{
    m_decompiler->getCompoundMembers(ui->edStructName->text(), ui->tblStructMembers);
}


void MainWindow::on_actionBoomerang_Website_triggered()
{
    QDesktopServices::openUrl(QUrl("http://boomerang.sourceforge.net"));
}


void MainWindow::on_actionAboutBoomerang_triggered()
{
    QDialog *dlg = new QDialog;
    Ui::AboutDialog aboutUi;
    aboutUi.setupUi(dlg);
    aboutUi.lbVersion->setText(QString("<h3>%1</h3>").arg(Boomerang::getVersionStr()));
    dlg->show();
}


void MainWindow::on_actionAboutQt_triggered()
{
    QApplication::aboutQt();
}


void MainWindow::on_entrypoints_currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)
{
    ui->btnEntryPointRemove->setEnabled(true);
}


void MainWindow::on_btnEntryPointAdd_pressed()
{
    if ((ui->addressEdit->text() == "") ||
        (ui->edEntryPointName->text() == "")) {
        return;
    }

    bool    ok = false;
    Address a = Address(ui->addressEdit->text().toInt(&ok, 16));

    if (!ok) {
        return;
    }

    emit entryPointAdded(a, ui->edEntryPointName->text());
    int nrows = ui->entrypoints->rowCount();
    ui->entrypoints->setRowCount(nrows + 1);
    ui->entrypoints->setItem(nrows, 0, new QTableWidgetItem(ui->addressEdit->text()));
    ui->entrypoints->setItem(nrows, 1, new QTableWidgetItem(ui->edEntryPointName->text()));
    ui->addressEdit->clear();
    ui->edEntryPointName->clear();
}


void MainWindow::on_btnEntryPointRemove_pressed()
{
    bool    ok = false;
    Address a = Address(ui->entrypoints->item(ui->entrypoints->currentRow(), 0)->text().toInt(&ok, 16));

    if (!ok) {
        return;
    }

    emit entryPointRemoved(a);
    ui->entrypoints->removeRow(ui->entrypoints->currentRow());
}


void MainWindow::on_actionSettings_triggered()
{
    SettingsDlg(m_decompiler).exec();
}

