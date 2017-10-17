#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "mainwindow.h"


#include "DecompilerThread.h"
#include "rtleditor.h"
#include "LoggingSettingsDlg.h"

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QtWidgets>


#include "boomerang-gui/ui_boomerang.h"
#include "boomerang-gui/ui_about.h"
#include "boomerang/type/TypeRecovery.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , decompilerThread(NULL)
    , step(NULL)
{
    ui->setupUi(this);

    qRegisterMetaType<Address>("ADDRESS");

    decompilerThread = new DecompilerThread();
    decompilerThread->start();
    Decompiler *d = decompilerThread->getDecompiler();
    connect(d, &Decompiler::newCluster, this, &MainWindow::showNewCluster);
    connect(d, &Decompiler::newProcInCluster, this, &MainWindow::showNewProcInCluster);
    connect(d, &Decompiler::debuggingPoint, this, &MainWindow::showDebuggingPoint);
    connect(d, &Decompiler::loading, this, &MainWindow::showLoadPage);
    connect(d, &Decompiler::decoding, this, &MainWindow::showDecodePage);
    connect(d, &Decompiler::decompiling, this, &MainWindow::showDecompilePage);
    connect(d, &Decompiler::generatingCode, this, &MainWindow::showGenerateCodePage);
    connect(d, &Decompiler::loadCompleted, this, &MainWindow::loadComplete);
    connect(d, &Decompiler::machineType, this, &MainWindow::showMachineType);
    connect(d, &Decompiler::newEntrypoint, this, &MainWindow::showNewEntrypoint);
    connect(d, &Decompiler::decodeCompleted, this, &MainWindow::decodeComplete);
    connect(d, &Decompiler::decompileCompleted, this, &MainWindow::decompileComplete);
    connect(d, &Decompiler::generateCodeCompleted, this, &MainWindow::generateCodeComplete);
    // connect(d, &Decompiler::changeProcedureState,this, &MainWindow::changeProcedureState);
    connect(d, &Decompiler::consideringProc, this, &MainWindow::showConsideringProc);
    connect(d, &Decompiler::decompilingProc, this, &MainWindow::showDecompilingProc);
    connect(d, &Decompiler::newUserProc, this, &MainWindow::showNewUserProc);
    connect(d, &Decompiler::newLibProc, this, &MainWindow::showNewLibProc);
    connect(d, &Decompiler::removeUserProc, this, &MainWindow::showRemoveUserProc);
    connect(d, &Decompiler::removeLibProc, this, &MainWindow::showRemoveLibProc);
    connect(d, &Decompiler::newSection, this, &MainWindow::showNewSection);
    connect(ui->toLoadButton, SIGNAL(clicked()), d, SLOT(load()));
    connect(ui->toDecodeButton, SIGNAL(clicked()), d, SLOT(decode()));
    connect(ui->toDecompileButton, SIGNAL(clicked()), d, SLOT(decompile()));
    connect(ui->toGenerateCodeButton, SIGNAL(clicked()), d, SLOT(generateCode()));
    connect(ui->inputFileComboBox, SIGNAL(editTextChanged(const QString&)), d, SLOT(changeInputFile(const QString&)));
    connect(ui->outputPathComboBox, SIGNAL(editTextChanged(const QString&)), d, SLOT(setOutputPath(const QString&)));
//     connect(ui->inputFileBrowseButton, SIGNAL(clicked()), this, SLOT(browseForInputFile()));
//     connect(ui->outputPathBrowseButton, SIGNAL(clicked()), this, SLOT(browseForOutputPath()));

    ui->userProcs->horizontalHeader()->disconnect(SIGNAL(sectionClicked(int)));
    connect(ui->userProcs->horizontalHeader(), &QHeaderView::sectionClicked, this,
            &MainWindow::onUserProcsHorizontalHeaderSectionClicked);

    ui->userProcs->verticalHeader()->hide();
    ui->libProcs->verticalHeader()->hide();
    ui->sections->verticalHeader()->hide();
    ui->entrypoints->verticalHeader()->hide();
    ui->structMembers->verticalHeader()->hide();

    QPushButton *closeButton = new QPushButton(QIcon("closetab.bmp"), "", ui->tabWidget);
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
        d->changeInputFile(ui->inputFileComboBox->currentText());
    }

    ui->outputPathComboBox->addItems(settings.value("outputpaths").toStringList());

    if (ui->outputPathComboBox->count() > 0) {
        int currentIdx = ui->outputPathComboBox->findText(settings.value("outputpath").toString());
        currentIdx = std::max(currentIdx, 0); // if selected output path could not be found, use last one
        ui->outputPathComboBox->setCurrentIndex(currentIdx);
        d->setOutputPath(ui->outputPathComboBox->currentText());
    }

    // check for a valid input file and output path
    ui->toLoadButton->setEnabled((ui->outputPathComboBox->count() > 0) && (ui->inputFileComboBox->count() > 0));

    loadingSettings = false;
    ui->cmb_typeRecoveryEngine->addItem("Constraint based recovery", QVariant::fromValue((void *)1));
    ui->cmb_typeRecoveryEngine->addItem("DFA based recovery", QVariant::fromValue((void *)2));
    ui->cmb_typeRecoveryEngine->setCurrentIndex(1);

    ui->inputFileComboBox->setEditable(false);
    ui->outputPathComboBox->setEditable(false);
    ui->inputFileComboBox->setMaxCount(15);
    ui->outputPathComboBox->setMaxCount(10);
}


MainWindow::~MainWindow()
{
    delete ui;
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


void MainWindow::on_inputFileBrowseButton_clicked()
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
    decompilerThread->getDecompiler()->changeInputFile(fileName);

    // we now have at least one input file
    ui->toLoadButton->setEnabled(ui->outputPathComboBox->count() > 0);
}


void MainWindow::on_outputPathBrowseButton_clicked()
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
    decompilerThread->getDecompiler()->setOutputPath(outputDir);

    // we now have at least one output directory
    ui->toLoadButton->setEnabled(ui->inputFileComboBox->count() > 0);
}


void MainWindow::on_inputFileComboBox_currentIndexChanged(const QString& text)
{
    decompilerThread->getDecompiler()->changeInputFile(text);
    saveSettings();
}


void MainWindow::on_outputPathComboBox_currentIndexChanged(const QString& text)
{
    decompilerThread->getDecompiler()->setOutputPath(text);
    saveSettings();
}


void MainWindow::closeCurrentTab()
{
    if (openFiles.find(ui->tabWidget->currentWidget()) != openFiles.end()) {
        on_actionClose_triggered();
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


void MainWindow::on_actionOpen_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select a file to open..."));

    if (!filename.isEmpty()) {
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QTextEdit   *n = new QTextEdit();
        QTextStream in(&file);
        QString     contents = in.readAll();
        file.close();
        n->insertPlainText(contents);
        openFiles[n] = filename;

        if (filename.endsWith(".h")) {
            signatureFiles.insert(n);
        }

        connect(n, SIGNAL(textChanged()), this, SLOT(currentTabTextChanged()));
        QString name = filename;
        name = name.right(name.length() - filename.lastIndexOf(QRegExp("[/\\\\]")) - 1);
        ui->tabWidget->addTab(n, name);
        ui->tabWidget->setCurrentWidget(n);
    }
}


void MainWindow::on_actionSave_triggered()
{
    if (openFiles.find(ui->tabWidget->currentWidget()) != openFiles.end()) {
        QString filename = openFiles[ui->tabWidget->currentWidget()];
        QFile   file(filename);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            return;
        }

        QTextEdit *edit = (QTextEdit *)ui->tabWidget->currentWidget();
        file.write(edit->toPlainText().toLatin1());
        file.close();
        QString text = ui->tabWidget->tabText(ui->tabWidget->currentIndex());

        if (text.right(1) == "*") {
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), text.left(text.length() - 1));
        }

        if (signatureFiles.find(ui->tabWidget->currentWidget()) != signatureFiles.end()) {
            decompilerThread->getDecompiler()->rereadLibSignatures();
        }
    }
}


void MainWindow::on_actionClose_triggered()
{
    if (openFiles.find(ui->tabWidget->currentWidget()) != openFiles.end()) {
        on_actionSave_triggered();
        openFiles.erase(ui->tabWidget->currentWidget());
        signatureFiles.erase(ui->tabWidget->currentWidget());
        ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
    }
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    ui->actionSave->setEnabled(openFiles.find(ui->tabWidget->widget(index)) != openFiles.end());
    ui->actionClose->setEnabled(openFiles.find(ui->tabWidget->widget(index)) != openFiles.end());
}


void MainWindow::errorLoadingFile()
{
}


void MainWindow::showInitPage()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecodeButton->setEnabled(false);
    ui->toDecompileButton->setEnabled(false);
    ui->toGenerateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->entrypoints->setRowCount(0);
    ui->userProcs->setRowCount(0);
    ui->libProcs->setRowCount(0);
    ui->decompileProcsTreeWidget->clear();
    decompiledCount = 0;
    ui->clusters->clear();
    codeGenCount = 0;
    ui->actionLoad->setEnabled(false);
    ui->actionDecode->setEnabled(false);
    ui->actionDecompile->setEnabled(false);
    ui->actionGenerate_Code->setEnabled(false);
}


void MainWindow::showLoadPage()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(true);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->toLoadButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(1);
    ui->actionLoad->setEnabled(true);
}


void MainWindow::showDecodePage()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(true);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(2);

    if (!ui->actionEnable->isChecked()) {
        ui->userProcs->removeColumn(2);
    }
    else {
        ui->userProcs->setColumnCount(3);
        ui->userProcs->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Debug")));
    }

    ui->actionDecode->setEnabled(true);
}


void MainWindow::showDecompilePage()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(true);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecompileButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(3);

    ui->actionDecompile->setEnabled(true);
}


void MainWindow::showGenerateCodePage()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(true);
    ui->toGenerateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(4);
    ui->actionGenerate_Code->setEnabled(true);
}


void MainWindow::loadComplete()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(true);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecodeButton->setEnabled(true);
    ui->toDecompileButton->setEnabled(false);
    ui->toGenerateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(1);
}


void MainWindow::showMachineType(const QString& machine)
{
    ui->machineTypeLabel->setText(machine);
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
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(true);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecodeButton->setEnabled(false);
    ui->toDecompileButton->setEnabled(true);
    ui->toGenerateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(2);
}


void MainWindow::decompileComplete()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(true);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecodeButton->setEnabled(false);
    ui->toDecompileButton->setEnabled(false);
    ui->toGenerateCodeButton->setEnabled(true);
    ui->stackedWidget->setCurrentIndex(3);
}


void MainWindow::generateCodeComplete()
{
    ui->toLoadButton->setEnabled(false);
    ui->loadButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);
    ui->decompileButton->setEnabled(false);
    ui->generateCodeButton->setEnabled(false);
    ui->toDecodeButton->setEnabled(false);
    ui->toDecompileButton->setEnabled(false);
    ui->toGenerateCodeButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(4);
}


void MainWindow::showConsideringProc(const QString& parent, const QString& name)
{
    QList<QTreeWidgetItem *> foundit =
        ui->decompileProcsTreeWidget->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);

    if (foundit.isEmpty()) {
        QStringList texts(name);

        if (parent.isEmpty()) {
            ui->decompileProcsTreeWidget->addTopLevelItem(new QTreeWidgetItem(texts));
        }
        else {
            QList<QTreeWidgetItem *> found =
                ui->decompileProcsTreeWidget->findItems(parent, Qt::MatchExactly | Qt::MatchRecursive);

            if (!found.isEmpty()) {
                QTreeWidgetItem *n = new QTreeWidgetItem(found.first(), texts);
                n->setData(0, 1, name);
                ui->decompileProcsTreeWidget->expandItem(found.first());
                ui->decompileProcsTreeWidget->scrollToItem(n);
                ui->decompileProcsTreeWidget->setCurrentItem(n, 0);
            }
        }
    }
}


void MainWindow::showDecompilingProc(const QString& name)
{
    QList<QTreeWidgetItem *> foundit =
        ui->decompileProcsTreeWidget->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);

    if (!foundit.isEmpty()) {
        ui->decompileProcsTreeWidget->setCurrentItem(foundit.first(), 0);
        foundit.first()->setTextColor(0, QColor("blue"));
        decompiledCount++;
    }

    int max = ui->userProcs->rowCount();
    ui->progressDecompile->setRange(0, max);
    ui->progressDecompile->setValue(decompiledCount);
}


void MainWindow::showNewUserProc(const QString& name, Address addr)
{
    int nrows = ui->userProcs->rowCount();

    for (int i = 0; i < nrows; i++) {
        if (ui->userProcs->item(i, 1)->text() == name) {
            return;
        }
    }

    const QString s = addr.toString();

    for (int i = 0; i < nrows; i++) {
        if (ui->userProcs->item(i, 0)->text() == s) {
            return;
        }
    }

    ui->userProcs->setRowCount(nrows + 1);
    ui->userProcs->setItem(nrows, 0, new QTableWidgetItem(addr.toString()));
    ui->userProcs->setItem(nrows, 1, new QTableWidgetItem(name));
    ui->userProcs->item(nrows, 1)->setData(1, name);

    if (ui->actionEnable->isChecked()) {
        QTableWidgetItem *d = new QTableWidgetItem("");
        d->setCheckState(Qt::Checked);
        ui->userProcs->setItem(nrows, 2, d);
    }

    ui->userProcs->resizeColumnsToContents();
    ui->userProcs->resizeRowsToContents();
}


void MainWindow::showNewLibProc(const QString& name, const QString& params)
{
    int nrows = ui->libProcs->rowCount();

    for (int i = 0; i < nrows; i++) {
        if (ui->libProcs->item(i, 0)->text() == name) {
            ui->libProcs->item(i, 1)->setText(params);
            return;
        }
    }

    ui->libProcs->setRowCount(nrows + 1);
    ui->libProcs->setItem(nrows, 0, new QTableWidgetItem(name));
    ui->libProcs->setItem(nrows, 1, new QTableWidgetItem(params));
    ui->libProcs->resizeColumnsToContents();
    ui->libProcs->resizeRowsToContents();
}


void MainWindow::showRemoveUserProc(const QString& name, Address addr)
{
    Q_UNUSED(name);
    int nrows = ui->userProcs->rowCount();

    const QString addrString = addr.toString();

    for (int i = 0; i < nrows; i++) {
        if (ui->userProcs->item(i, 0)->text() == addrString) {
            ui->userProcs->removeRow(i);
            break;
        }
    }

    ui->userProcs->resizeColumnsToContents();
    ui->userProcs->resizeRowsToContents();
}


void MainWindow::showRemoveLibProc(const QString& name)
{
    int nrows = ui->libProcs->rowCount();

    for (int i = 0; i < nrows; i++) {
        if (ui->libProcs->item(i, 0)->text() == name) {
            ui->libProcs->removeRow(i);
            break;
        }
    }

    ui->libProcs->resizeColumnsToContents();
    ui->libProcs->resizeRowsToContents();
}


void MainWindow::showNewSection(const QString& name, Address start, Address end)
{
    int nrows = ui->sections->rowCount();

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
    ui->clusters->addTopLevelItem(n);
    ui->clusters->expandItem(n);
}


void MainWindow::showNewProcInCluster(const QString& name, const QString& cluster)
{
    QString cname = cluster;

    cname = cname.append(".c");
    QList<QTreeWidgetItem *> found = ui->clusters->findItems(cname, Qt::MatchExactly);

    if (!found.isEmpty()) {
        QTreeWidgetItem *n = new QTreeWidgetItem(found.first(), QStringList(name));
        ui->clusters->scrollToItem(n);
        ui->clusters->setCurrentItem(n, 0);
        ui->clusters->expandItem(found.first());
        codeGenCount++;
    }

    ui->progressGenerateCode->setRange(0, ui->userProcs->rowCount());
    ui->progressGenerateCode->setValue(codeGenCount);
}


void MainWindow::showDebuggingPoint(const QString& name, const QString& description)
{
    QString msg = "debugging ";

    msg.append(name);
    msg.append(": ");
    msg.append(description);
    statusBar()->showMessage(msg);
    ui->actionStep->setEnabled(true);

    for (int i = 0; i < ui->userProcs->rowCount(); i++) {
        if ((ui->userProcs->item(i, 1)->text() == name) && (ui->userProcs->item(i, 2)->checkState() != Qt::Checked)) {
            on_actionStep_triggered();
            return;
        }
    }

    showRTLEditor(name);
}


void MainWindow::showRTLEditor(const QString& name)
{
    RTLEditor *n = NULL;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->tabText(i) == name) {
            n = dynamic_cast<RTLEditor *>(ui->tabWidget->widget(i));
            break;
        }
    }

    if (n == NULL) {
        n = new RTLEditor(decompilerThread->getDecompiler(), name);
        ui->tabWidget->addTab(n, name);
    }
    else {
        n->updateContents();
    }

    ui->tabWidget->setCurrentWidget(n);
}


void MainWindow::on_userProcs_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    showRTLEditor(ui->userProcs->item(row, 1)->text());
}


void MainWindow::on_userProcs_cellChanged(int row, int column)
{
    if (column == 0) {
        // TODO: should we allow the user to change the address of a proc?
    }

    if (column == 1) {
        QString old_name = ui->userProcs->item(row, 1)->data(1).toString();
        decompilerThread->getDecompiler()->renameProc(old_name, ui->userProcs->item(row, 1)->text());
        ui->userProcs->item(row, 1)->setData(1, ui->userProcs->item(row, 1)->text());
    }
}


void MainWindow::on_clusters_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QTreeWidgetItem *top = item;

    while (top->parent()) {
        top = top->parent();
    }

    QTextEdit *n = NULL;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if (ui->tabWidget->tabText(i) == top->text(0)) {
            n = dynamic_cast<QTextEdit *>(ui->tabWidget->widget(i));
            break;
        }
    }

    if (n == NULL) {
        n = new QTextEdit();
        QString name = top->text(0);
        name = name.left(name.lastIndexOf("."));
        QString filename = decompilerThread->getDecompiler()->getClusterFile(name);
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


void MainWindow::on_decompileProcsTreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    showRTLEditor(item->text(0));
}


void MainWindow::on_actionEnable_toggled(bool b)
{
    decompilerThread->getDecompiler()->setDebugging(b);
    decompilerThread->getDecompiler()->stopWaiting();

    if (b) {
        statusBar()->show();

        if (step == nullptr) {
            step = new QToolButton();
            step->setToolButtonStyle(Qt::ToolButtonTextOnly);
            step->setText("Step");
            step->setDefaultAction(ui->actionStep);
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


void MainWindow::on_actionStep_triggered()
{
    ui->actionStep->setEnabled(false);
    decompilerThread->getDecompiler()->stopWaiting();
}


void MainWindow::onUserProcsHorizontalHeaderSectionClicked(int logicalIndex)
{
    if (logicalIndex == 2) {
        for (int i = 0; i < ui->userProcs->rowCount(); i++) {
            if (ui->userProcs->item(i, 2) == NULL) {
                ui->userProcs->setItem(i, 2, new QTableWidgetItem(""));
            }

            Qt::CheckState state = ui->userProcs->item(i, 2)->checkState();
            ui->userProcs->item(i, 2)->setCheckState(state == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    }
}


void MainWindow::on_libProcs_cellDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    QString name = "";
    QString sigFile;
    QString params   = ui->libProcs->item(row, 1)->text();
    bool    existing = true;

    if (params == "<unknown>") {
        existing = false;

        // uhh, time to guess?
        for (int i = row; i >= 0; i--) {
            params = ui->libProcs->item(i, 1)->text();

            if (params != "<unknown>") {
                name = ui->libProcs->item(i, 0)->text();
                break;
            }
        }

        if (name.isEmpty()) {
            return;
        }
    }
    else {
        name = ui->libProcs->item(row, 0)->text();
    }

    sigFile = decompilerThread->getDecompiler()->getSigFile(name);
    QString filename = sigFile;

    int pos = sigFile.lastIndexOf(QRegExp("[/\\\\]"));

    if (pos != -1) {
        sigFile = sigFile.right(sigFile.length() - pos - 1);
    }

    QString sigFileStar = sigFile;
    sigFileStar.append("*");

    QTextEdit *n = NULL;

    for (int i = 0; i < ui->tabWidget->count(); i++) {
        if ((ui->tabWidget->tabText(i) == sigFile) || (ui->tabWidget->tabText(i) == sigFileStar)) {
            n = dynamic_cast<QTextEdit *>(ui->tabWidget->widget(i));
            break;
        }
    }

    if (n == NULL) {
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
        QTextCursor cursor = n->textCursor();
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::End);
        n->setTextCursor(cursor);
        QString comment = "// unknown library proc: ";
        comment.append(ui->libProcs->item(row, 0)->text());
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


void MainWindow::on_structName_returnPressed()
{
    decompilerThread->getDecompiler()->getCompoundMembers(ui->structName->text(), ui->structMembers);
}


void MainWindow::on_actionBoomerang_Website_triggered()
{
    QDesktopServices::openUrl(QUrl("http://boomerang.sourceforge.net"));
}


void MainWindow::on_actionAbout_triggered()
{
    QDialog *dlg = new QDialog;

    Ui::AboutDialog aboutUi;
    aboutUi.setupUi(dlg);
    aboutUi.VersionLabel->setText(QString("<h3>").append(Boomerang::getVersionStr()).append("</h3>"));
    dlg->show();
}


void MainWindow::on_actionAboutQt_triggered()
{
    QApplication::aboutQt();
}


void MainWindow::on_enableNoDecodeChildren_toggled(bool b)
{
    decompilerThread->getDecompiler()->setNoDecodeChildren(b);
}


void MainWindow::on_entrypoints_currentItemChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    Q_UNUSED(current);
    Q_UNUSED(previous);
    ui->removeButton->setEnabled(true);
}


void MainWindow::on_addButton_pressed()
{
    if ((ui->addressEdit->text() == "") || (ui->nameEdit->text() == "")) {
        return;
    }

    bool    ok;
    Address a = Address(ui->addressEdit->text().toInt(&ok, 16));

    if (!ok) {
        return;
    }

    decompilerThread->getDecompiler()->addEntryPoint(a, (const char *)qPrintable(ui->nameEdit->text()));
    int nrows = ui->entrypoints->rowCount();
    ui->entrypoints->setRowCount(nrows + 1);
    ui->entrypoints->setItem(nrows, 0, new QTableWidgetItem(ui->addressEdit->text()));
    ui->entrypoints->setItem(nrows, 1, new QTableWidgetItem(ui->nameEdit->text()));
    ui->addressEdit->clear();
    ui->nameEdit->clear();
}


void MainWindow::on_removeButton_pressed()
{
    bool    ok;
    Address a = Address(ui->entrypoints->item(ui->entrypoints->currentRow(), 0)->text().toInt(&ok, 16));

    if (!ok) {
        return;
    }

    decompilerThread->getDecompiler()->removeEntryPoint(a);
    ui->entrypoints->removeRow(ui->entrypoints->currentRow());
}


void MainWindow::on_actionLoggingOptions_triggered()
{
    LoggingSettingsDlg dlg;

    dlg.exec();
}


void MainWindow::on_cmb_typeRecoveryEngine_currentIndexChanged(int index)
{
    QVariant      sel_data = ui->cmb_typeRecoveryEngine->itemData(index);
    ITypeRecovery *ptr     = (ITypeRecovery *)sel_data.value<void *>();
    Boomerang     *boom    = Boomerang::get();

    if (!boom->getOrCreateProject()) {
        QMessageBox::warning(this, "Error", "Cannot set type recovery without active project");
        return;
    }

    if ((intptr_t)ptr == 1) {
        decompilerThread->getDecompiler()->setUseDFTA(false);
    }
    else {
        decompilerThread->getDecompiler()->setUseDFTA(true);
    }
}
