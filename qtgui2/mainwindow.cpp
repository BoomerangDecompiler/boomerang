

#include <QtGui>

#include "mainwindow.h"
#include "DecompilerThread.h"
#include "rtleditor.h"

MainWindow::MainWindow(QWidget *parent) : 
    QMainWindow(parent), 
    decompilerThread(NULL),
	step(NULL)
{
    ui.setupUi(this);

    decompilerThread = new DecompilerThread();
	decompilerThread->start();
	Decompiler *d = decompilerThread->getDecompiler();
	connect(d, SIGNAL(newCluster(const QString &)), this, SLOT(showNewCluster(const QString &)));
	connect(d, SIGNAL(newProcInCluster(const QString &, const QString &)), this, SLOT(showNewProcInCluster(const QString &, const QString &)));
	connect(d, SIGNAL(debuggingPoint(const QString &, const QString &)), this, SLOT(showDebuggingPoint(const QString &, const QString &)));
	connect(d, SIGNAL(loading()), this, SLOT(showLoadPage()));
	connect(d, SIGNAL(decoding()), this, SLOT(showDecodePage()));
	connect(d, SIGNAL(decompiling()), this, SLOT(showDecompilePage()));
	connect(d, SIGNAL(generatingCode()), this, SLOT(showGenerateCodePage()));
	connect(d, SIGNAL(loadCompleted()), this, SLOT(loadComplete()));
	connect(d, SIGNAL(machineType(const QString &)), this, SLOT(showMachineType(const QString &)));
	connect(d, SIGNAL(newEntrypoint(unsigned int, const QString &)), this, SLOT(showNewEntrypoint(unsigned int, const QString &)));
	connect(d, SIGNAL(decodeCompleted()), this, SLOT(decodeComplete()));
	connect(d, SIGNAL(decompileCompleted()), this, SLOT(decompileComplete()));
	connect(d, SIGNAL(generateCodeCompleted()), this, SLOT(generateCodeComplete()));
	connect(d, SIGNAL(changeProcedureState(const QString &, const QString &)), this, SLOT(changeProcedureState(const QString &, const QString &)));
	connect(d, SIGNAL(consideringProc(const QString &, const QString &)), this, SLOT(showConsideringProc(const QString &, const QString &)));
	connect(d, SIGNAL(decompilingProc(const QString &)), this, SLOT(showDecompilingProc(const QString &)));
	connect(d, SIGNAL(newUserProc(const QString &, unsigned int)), this, SLOT(showNewUserProc(const QString &, unsigned int)));
	connect(d, SIGNAL(newLibProc(const QString &, const QString &)), this, SLOT(showNewLibProc(const QString &, const QString &)));
	connect(d, SIGNAL(newSection(const QString &, unsigned int, unsigned int)), this, SLOT(showNewSection(const QString &, unsigned int, unsigned int)));
    connect(ui.toLoadButton, SIGNAL(clicked()), d, SLOT(load()));
    connect(ui.toDecodeButton, SIGNAL(clicked()), d, SLOT(decode()));
    connect(ui.toDecompileButton, SIGNAL(clicked()), d, SLOT(decompile()));
    connect(ui.toGenerateCodeButton, SIGNAL(clicked()), d, SLOT(generateCode()));
	//connect(ui.inputFileComboBox, SIGNAL(editTextChanged(const QString &)), d,  SLOT(changeInputFile(const QString &)));
	//connect(ui.outputPathComboBox, SIGNAL(editTextChanged(const QString &)), d,  SLOT(changeOutputPath(const QString &)));
	//connect(ui.inputFileBrowseButton, SIGNAL(clicked()), this, SLOT(browseForInputFile()));
	//connect(ui.outputPathBrowseButton, SIGNAL(clicked()), this, SLOT(browseForOutputPath()));

	ui.userProcs->horizontalHeader()->disconnect(SIGNAL(sectionClicked(int)));
	connect(ui.userProcs->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(on_userProcs_horizontalHeader_sectionClicked(int)));

	ui.userProcs->verticalHeader()->hide();
	ui.libProcs->verticalHeader()->hide();
	ui.sections->verticalHeader()->hide();
	ui.entrypoints->verticalHeader()->hide();
	ui.structMembers->verticalHeader()->hide();

	QPushButton *closeButton = new QPushButton(QIcon("closetab.bmp"), "", ui.tabWidget);
	closeButton->setFixedSize(closeButton->iconSize());
	ui.tabWidget->setCornerWidget(closeButton);
	ui.tabWidget->cornerWidget()->show();
	connect(closeButton, SIGNAL(clicked()), this, SLOT(closeCurrentTab()));

	structs = ui.tabWidget->widget(1);
	ui.tabWidget->removeTab(1);

	showInitPage();
	setWindowTitle("Boomerang");

	loadingSettings = true;
	QSettings settings("Boomerang", "Boomerang");
	QStringList inputfiles = settings.value("inputfiles").toStringList();
	for (int n = 0; n < inputfiles.count(); n++) {
		if (ui.inputFileComboBox->findText(inputfiles.at(n)) == -1)
			ui.inputFileComboBox->addItem(inputfiles.at(n));
	}
	QString inputfile = settings.value("inputfile").toString();
	int i = ui.inputFileComboBox->findText(inputfile);
	if (i != -1)
		ui.inputFileComboBox->setCurrentIndex(i);
	QStringList outputpaths = settings.value("outputpaths").toStringList();
	for (int n = 0; n < outputpaths.count(); n++) {
		if (ui.outputPathComboBox->findText(outputpaths.at(n)) == -1)
			ui.outputPathComboBox->addItem(outputpaths.at(n));
	}
	i = ui.outputPathComboBox->findText(settings.value("outputpath").toString());
	ui.outputPathComboBox->setCurrentIndex(i);
	if (!ui.inputFileComboBox->currentText().isEmpty()) {
		d->changeInputFile(ui.inputFileComboBox->currentText());
		ui.toLoadButton->setDisabled(false);
	}
	loadingSettings = false;
}

void MainWindow::saveSettings()
{
	if (loadingSettings)
		return;
	QSettings settings("Boomerang", "Boomerang");			
	QStringList inputfiles;
	for (int n = 0; n < ui.inputFileComboBox->count(); n++) {
		inputfiles.append(ui.inputFileComboBox->itemText(n));
	}
	settings.setValue("inputfiles", inputfiles);
	settings.setValue("inputfile", ui.inputFileComboBox->itemText(ui.inputFileComboBox->currentIndex()));
	QStringList outputPaths;
	for (int n = 0; n < ui.outputPathComboBox->count(); n++) {
		outputPaths.append(ui.outputPathComboBox->itemText(n));
	}
	settings.setValue("outputpaths", outputPaths);
	settings.setValue("outputpath", ui.outputPathComboBox->itemText(ui.outputPathComboBox->currentIndex()));
}

void MainWindow::on_inputFileBrowseButton_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Select a file to decompile..."), "test", "Windows Binaries (*.exe *.dll *.scr *.sys);;Other Binaries (*.*)");
    if (!s.isEmpty()) {
		if (ui.inputFileComboBox->findText(s) == -1) {
			ui.inputFileComboBox->addItem(s);
			ui.inputFileComboBox->setCurrentIndex(ui.inputFileComboBox->findText(s));
			saveSettings();
		}
		decompilerThread->getDecompiler()->changeInputFile(s);
		if (!ui.outputPathComboBox->currentText().isEmpty())
			ui.toLoadButton->setDisabled(false);
	}
}

void MainWindow::on_outputPathBrowseButton_clicked()
{
    QString s = QFileDialog::getExistingDirectory(this, tr("Select a location to write output..."), "output");
    if (!s.isEmpty()) {
		if (ui.outputPathComboBox->findText(s) == -1) {
			ui.outputPathComboBox->addItem(s);
			saveSettings();
		}
		ui.outputPathComboBox->setEditText(s);
		if (!ui.inputFileComboBox->currentText().isEmpty())
			ui.toLoadButton->setDisabled(false);
    }
}

void MainWindow::on_inputFileComboBox_editTextChanged(const QString &text)
{
	decompilerThread->getDecompiler()->changeInputFile(text);
	if (ui.inputFileComboBox->findText(text) == -1) {
		ui.inputFileComboBox->addItem(text);
		ui.inputFileComboBox->setCurrentIndex(ui.inputFileComboBox->findText(text));
		saveSettings();
	}
	if (!ui.outputPathComboBox->currentText().isEmpty())
		ui.toLoadButton->setDisabled(false);
}

void MainWindow::on_inputFileComboBox_currentIndexChanged(const QString &text)
{
	decompilerThread->getDecompiler()->changeInputFile(text);
	saveSettings();
}

void MainWindow::on_outputPathComboBox_editTextChanged(QString &text)
{
	decompilerThread->getDecompiler()->changeOutputPath(text);
	ui.outputPathComboBox->addItem(text);
	saveSettings();
	if (!ui.inputFileComboBox->currentText().isEmpty())
		ui.toLoadButton->setDisabled(false);
}

void MainWindow::closeCurrentTab()
{
	if (openFiles.find(ui.tabWidget->currentWidget()) != openFiles.end())
		on_actionClose_activated();
	else if (ui.tabWidget->currentIndex() != 0)
		ui.tabWidget->removeTab(ui.tabWidget->currentIndex());
}

void MainWindow::currentTabTextChanged()
{
	QString text = ui.tabWidget->tabText(ui.tabWidget->currentIndex());
	if (text.right(1) != "*")
		ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), text.append("*"));
}

void MainWindow::on_actionOpen_activated()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Select a file to open..."));
	if (!filename.isEmpty()) {
		QTextEdit *n = new QTextEdit();
		QFile file(filename);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;
		QTextStream in(&file);
		QString contents = in.readAll();
		file.close();
		n->insertPlainText(contents);
		openFiles[n] = filename;
		if (filename.endsWith(".h"))
			signatureFiles.insert(n);
		connect(n, SIGNAL(textChanged()), this, SLOT(currentTabTextChanged()));
		QString name = filename; 
		name = name.right(name.length() - filename.lastIndexOf(QRegExp("[/\\\\]")) - 1);
		ui.tabWidget->addTab(n, name);
		ui.tabWidget->setCurrentWidget(n);
	}
}

void MainWindow::on_actionSave_activated()
{
	if (openFiles.find(ui.tabWidget->currentWidget()) != openFiles.end()) {
		QString filename = openFiles[ui.tabWidget->currentWidget()];
		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
			return;
		QTextEdit *edit = (QTextEdit*)ui.tabWidget->currentWidget();
		file.write(edit->toPlainText().toAscii());
		file.close();
		QString text = ui.tabWidget->tabText(ui.tabWidget->currentIndex());
		if (text.right(1) == "*")
			ui.tabWidget->setTabText(ui.tabWidget->currentIndex(), text.left(text.length()-1));
		if (signatureFiles.find(ui.tabWidget->currentWidget()) != signatureFiles.end()) {
			decompilerThread->getDecompiler()->rereadLibSignatures();
		}
	}
}

void MainWindow::on_actionClose_activated()
{
	if (openFiles.find(ui.tabWidget->currentWidget()) != openFiles.end()) {
		on_actionSave_activated();
		openFiles.erase(ui.tabWidget->currentWidget());
		signatureFiles.erase(ui.tabWidget->currentWidget());
		ui.tabWidget->removeTab(ui.tabWidget->currentIndex());
	}
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
	ui.actionSave->setEnabled(openFiles.find(ui.tabWidget->widget(index)) != openFiles.end());
	ui.actionClose->setEnabled(openFiles.find(ui.tabWidget->widget(index)) != openFiles.end());
}

void MainWindow::errorLoadingFile()
{
}

void MainWindow::showInitPage()
{
	ui.toLoadButton->setDisabled(true);
	ui.loadButton->setDisabled(true);
    ui.decodeButton->setDisabled(true);
    ui.decompileButton->setDisabled(true);
    ui.generateCodeButton->setDisabled(true);
    ui.toDecodeButton->setDisabled(true);
    ui.toDecompileButton->setDisabled(true);
    ui.toGenerateCodeButton->setDisabled(true);
    ui.stackedWidget->setCurrentIndex(0);
	ui.entrypoints->setRowCount(0);
	ui.userProcs->setRowCount(0);
	ui.libProcs->setRowCount(0);
	ui.decompileProcsTreeWidget->clear();
	decompiledCount = 0;
	ui.clusters->clear();
	codeGenCount = 0;
	ui.actionLoad->setDisabled(true);
	ui.actionDecode->setDisabled(true);
	ui.actionDecompile->setDisabled(true);
	ui.actionGenerate_Code->setDisabled(true);
}

void MainWindow::showLoadPage()
{
	ui.toLoadButton->setDisabled(true);
	ui.loadButton->setDisabled(false);
	ui.decodeButton->setDisabled(true);
	ui.decompileButton->setDisabled(true);
	ui.generateCodeButton->setDisabled(true);
	ui.toLoadButton->setDisabled(true);
	ui.stackedWidget->setCurrentIndex(1);
	ui.actionLoad->setDisabled(false);
}

void MainWindow::showDecodePage()
{
	ui.toLoadButton->setDisabled(true);
    ui.loadButton->setDisabled(true);
    ui.decodeButton->setDisabled(false);
    ui.decompileButton->setDisabled(true);
    ui.generateCodeButton->setDisabled(true);
	ui.toDecodeButton->setDisabled(true);
    ui.stackedWidget->setCurrentIndex(2);

	if (!ui.actionEnable->isChecked()) {
		ui.userProcs->removeColumn(2);
	} else {
		ui.userProcs->setColumnCount(3);
		ui.userProcs->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Debug")));
	}

	ui.actionDecode->setDisabled(false);
}

void MainWindow::showDecompilePage()
{
	ui.toLoadButton->setDisabled(true);
    ui.loadButton->setDisabled(true);
    ui.decodeButton->setDisabled(true);
    ui.decompileButton->setDisabled(false);
    ui.generateCodeButton->setDisabled(true);
	ui.toDecompileButton->setDisabled(true);
    ui.stackedWidget->setCurrentIndex(3);

	ui.actionDecompile->setDisabled(false);
}

void MainWindow::showGenerateCodePage()
{
	ui.toLoadButton->setDisabled(true);
    ui.loadButton->setDisabled(true);
    ui.decodeButton->setDisabled(true);
    ui.decompileButton->setDisabled(true);
    ui.generateCodeButton->setDisabled(false);
	ui.toGenerateCodeButton->setDisabled(true);
    ui.stackedWidget->setCurrentIndex(4);
	ui.actionGenerate_Code->setDisabled(false);
}

void MainWindow::loadComplete()
{
	ui.toLoadButton->setDisabled(true);
    ui.loadButton->setDisabled(false);
    ui.decodeButton->setDisabled(true);
    ui.decompileButton->setDisabled(true);
    ui.generateCodeButton->setDisabled(true);
    ui.toDecodeButton->setDisabled(false);
    ui.toDecompileButton->setDisabled(true);
    ui.toGenerateCodeButton->setDisabled(true);
    ui.stackedWidget->setCurrentIndex(1);
}

void MainWindow::showMachineType(const QString &machine)
{
	ui.machineTypeLabel->setText(machine);
}

void MainWindow::showNewEntrypoint(unsigned int addr, const QString &name)
{
	int nrows = ui.entrypoints->rowCount();
	ui.entrypoints->setRowCount(nrows + 1);
	ui.entrypoints->setItem(nrows, 0, new QTableWidgetItem(tr("%1").arg(addr, 8, 16, QChar('0'))));
	ui.entrypoints->setItem(nrows, 1, new QTableWidgetItem(name));
	ui.entrypoints->resizeColumnsToContents();
	ui.entrypoints->resizeRowsToContents();
}

void MainWindow::decodeComplete()
{
	ui.toLoadButton->setDisabled(true);
	ui.loadButton->setDisabled(true);
	ui.decodeButton->setDisabled(false);
	ui.decompileButton->setDisabled(true);
	ui.generateCodeButton->setDisabled(true);
	ui.toDecodeButton->setDisabled(true);
	ui.toDecompileButton->setDisabled(false);
	ui.toGenerateCodeButton->setDisabled(true);
	ui.stackedWidget->setCurrentIndex(2);
}

void MainWindow::decompileComplete()
{
	ui.toLoadButton->setDisabled(true);
    ui.loadButton->setDisabled(true);
    ui.decodeButton->setDisabled(true);
    ui.decompileButton->setDisabled(false);
    ui.generateCodeButton->setDisabled(true);
    ui.toDecodeButton->setDisabled(true);
    ui.toDecompileButton->setDisabled(true);
    ui.toGenerateCodeButton->setDisabled(false);
    ui.stackedWidget->setCurrentIndex(3);
}

void MainWindow::generateCodeComplete()
{
	ui.toLoadButton->setDisabled(true);
    ui.loadButton->setDisabled(true);
    ui.decodeButton->setDisabled(true);
    ui.decompileButton->setDisabled(true);
    ui.generateCodeButton->setDisabled(true);
    ui.toDecodeButton->setDisabled(true);
    ui.toDecompileButton->setDisabled(true);
    ui.toGenerateCodeButton->setDisabled(true);
    ui.stackedWidget->setCurrentIndex(4);
}

void MainWindow::showConsideringProc(const QString &parent, const QString &name)
{
	QList<QTreeWidgetItem *> foundit = ui.decompileProcsTreeWidget->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);
	if (foundit.isEmpty()) {
		QStringList texts(name);
		if (parent.isEmpty()) {
			ui.decompileProcsTreeWidget->addTopLevelItem(new QTreeWidgetItem(texts));
		} else {
			QList<QTreeWidgetItem *> found = ui.decompileProcsTreeWidget->findItems(parent, Qt::MatchExactly | Qt::MatchRecursive);
			if (!found.isEmpty()) {
				QTreeWidgetItem *n = new QTreeWidgetItem(found.first(), texts);
				n->setData(0, 1, name);
				ui.decompileProcsTreeWidget->expandItem(found.first());
				ui.decompileProcsTreeWidget->scrollToItem(n);
				ui.decompileProcsTreeWidget->setCurrentItem(n, 0);				
			}
		}
	}
}

void MainWindow::showDecompilingProc(const QString &name)
{
	QList<QTreeWidgetItem *> foundit = ui.decompileProcsTreeWidget->findItems(name, Qt::MatchExactly | Qt::MatchRecursive);
	if (!foundit.isEmpty()) {
		ui.decompileProcsTreeWidget->setCurrentItem(foundit.first(), 0);
		foundit.first()->setTextColor(0, QColor("blue"));
		decompiledCount++;
	}
	ui.progressDecompile->setRange(0, ui.userProcs->rowCount());
	ui.progressDecompile->setValue(decompiledCount);
}

void MainWindow::showNewUserProc(const QString &name, unsigned int addr)
{
	int nrows = ui.userProcs->rowCount();
	for (int i = 0; i < nrows; i++)
		if (ui.userProcs->item(i, 1)->text() == name)
			return;
	ui.userProcs->setRowCount(nrows + 1);
	ui.userProcs->setItem(nrows, 0, new QTableWidgetItem(tr("%1").arg(addr, 8, 16, QChar('0'))));
	ui.userProcs->setItem(nrows, 1, new QTableWidgetItem(name));
	ui.userProcs->item(nrows, 1)->setData(1, name);
	if (ui.actionEnable->isChecked()) {
		QTableWidgetItem *d = new QTableWidgetItem("");
		d->setCheckState(Qt::Checked);
		ui.userProcs->setItem(nrows, 2, d);
	}
	ui.userProcs->resizeColumnsToContents();
	ui.userProcs->resizeRowsToContents();
}

void MainWindow::showNewLibProc(const QString &name, const QString &params)
{
	int nrows = ui.libProcs->rowCount();
	for (int i = 0; i < nrows; i++)
		if (ui.libProcs->item(i, 0)->text() == name) {
			ui.libProcs->item(i, 1)->setText(params);
			return;
		}
	ui.libProcs->setRowCount(nrows + 1);
	ui.libProcs->setItem(nrows, 0, new QTableWidgetItem(name));
	ui.libProcs->setItem(nrows, 1, new QTableWidgetItem(params));
	ui.libProcs->resizeColumnsToContents();
	ui.libProcs->resizeRowsToContents();
}

void MainWindow::showNewSection(const QString &name, unsigned int start, unsigned int end)
{
	int nrows = ui.sections->rowCount();
	ui.sections->setRowCount(nrows + 1);
	ui.sections->setItem(nrows, 0, new QTableWidgetItem(name));
	ui.sections->setItem(nrows, 1, new QTableWidgetItem(tr("%1").arg(start, 8, 16, QChar('0'))));
	ui.sections->setItem(nrows, 2, new QTableWidgetItem(tr("%1").arg(end, 8, 16, QChar('0'))));
	ui.sections->sortItems(1, Qt::AscendingOrder);
	ui.sections->resizeColumnsToContents();
	ui.sections->resizeRowsToContents();
}

void MainWindow::showNewCluster(const QString &name)
{
	QString cname = name;
	cname = cname.append(".c");
	QTreeWidgetItem *n = new QTreeWidgetItem(QStringList(cname));
	ui.clusters->addTopLevelItem(n);
	ui.clusters->expandItem(n);
}

void MainWindow::showNewProcInCluster(const QString &name, const QString &cluster)
{
	QString cname = cluster;
	cname = cname.append(".c");
	QList<QTreeWidgetItem *> found = ui.clusters->findItems(cname, Qt::MatchExactly);
	if (!found.isEmpty()) {
		QTreeWidgetItem *n = new QTreeWidgetItem(found.first(), QStringList(name));
		ui.clusters->scrollToItem(n);
		ui.clusters->setCurrentItem(n, 0);
		ui.clusters->expandItem(found.first());
		codeGenCount++;
	}
	ui.progressGenerateCode->setRange(0, ui.userProcs->rowCount());
	ui.progressGenerateCode->setValue(codeGenCount);
}

void MainWindow::showDebuggingPoint(const QString &name, const QString &description)
{
	QString msg = "debugging ";
	msg.append(name);
	msg.append(": ");
	msg.append(description);
	statusBar()->showMessage(msg);
	ui.actionStep->setEnabled(true);

	for (int i = 0; i < ui.userProcs->rowCount(); i++)
		if (ui.userProcs->item(i, 1)->text() == name && ui.userProcs->item(i, 2)->checkState() != Qt::Checked) {
			on_actionStep_activated();
			return;
		}

	showRTLEditor(name);
}

void MainWindow::showRTLEditor(const QString &name)
{
	RTLEditor *n = NULL;
	for (int i = 0; i < ui.tabWidget->count(); i++)
		if (ui.tabWidget->tabText(i) == name) {
			n = dynamic_cast<RTLEditor*>(ui.tabWidget->widget(i));
			break;
		}
	if (n == NULL) {
		n = new RTLEditor(decompilerThread->getDecompiler(), name);
		ui.tabWidget->addTab(n, name);
	} else
		n->updateContents();
	ui.tabWidget->setCurrentWidget(n);
}

void MainWindow::on_userProcs_cellDoubleClicked(int row, int column)
{
	showRTLEditor(ui.userProcs->item(row, 1)->text());
}

void MainWindow::on_userProcs_cellChanged(int row, int column)
{
	if (column == 0) {
		// TODO: should we allow the user to change the address of a proc?
	}
	if (column == 1) {
		QString old_name = ui.userProcs->item(row, 1)->data(1).toString();
		decompilerThread->getDecompiler()->renameProc(old_name, ui.userProcs->item(row, 1)->text());
		ui.userProcs->item(row, 1)->setData(1, ui.userProcs->item(row, 1)->text());		
	}
}

void MainWindow::on_clusters_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	QTreeWidgetItem *top = item;
	while (top->parent())
		top = top->parent();
	QTextEdit *n = NULL;
	for (int i = 0; i < ui.tabWidget->count(); i++)
		if (ui.tabWidget->tabText(i) == top->text(0)) {
			n = dynamic_cast<QTextEdit*>(ui.tabWidget->widget(i));
			break;
		}
	if (n == NULL) {
		n = new QTextEdit();	
		QString name = top->text(0);
		name = name.left(name.lastIndexOf("."));
		QString filename = decompilerThread->getDecompiler()->getClusterFile(name);
		QFile file(filename);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;
		QTextStream in(&file);
		QString contents = in.readAll();
		file.close();
		n->insertPlainText(contents);
		openFiles[n] = filename;
		connect(n, SIGNAL(textChanged()), this, SLOT(currentTabTextChanged()));
		ui.tabWidget->addTab(n, top->text(0));
	}
	ui.tabWidget->setCurrentWidget(n);
}

void MainWindow::on_decompileProcsTreeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	showRTLEditor(item->text(0));
}

void MainWindow::on_actionEnable_toggled(bool b)
{
	decompilerThread->getDecompiler()->setDebugging(b);
	decompilerThread->getDecompiler()->stopWaiting();
	if (b) {
		statusBar()->show();
		if (step == NULL) {
			step = new QToolButton();
			step->setToolButtonStyle(Qt::ToolButtonTextOnly);
			step->setText("Step");
			step->setDefaultAction(ui.actionStep);
		}
		statusBar()->addPermanentWidget(step);
	} else {
		if (step)
			statusBar()->removeWidget(step);
		statusBar()->hide();
	}
	
}

void MainWindow::on_actionStep_activated()
{
	ui.actionStep->setEnabled(false);
	decompilerThread->getDecompiler()->stopWaiting();
}

void MainWindow::on_userProcs_horizontalHeader_sectionClicked(int logicalIndex)
{
	if (logicalIndex == 2) {
		for (int i = 0; i < ui.userProcs->rowCount(); i++) {
			if (ui.userProcs->item(i, 2) == NULL) {
				ui.userProcs->setItem(i, 2, new QTableWidgetItem(""));
			}
			Qt::CheckState state = ui.userProcs->item(i, 2)->checkState();
			ui.userProcs->item(i, 2)->setCheckState(state == Qt::Checked ? Qt::Unchecked : Qt::Checked);
		}
	}
}

void MainWindow::on_libProcs_cellDoubleClicked(int row, int column)
{
	QString name = "";
	QString sigFile;
	QString params = ui.libProcs->item(row, 1)->text();
	bool existing = true;
	if (params == "<unknown>") {
		existing = false;
		// uhh, time to guess?
		for (int i = row; i >= 0; i--) {
			params = ui.libProcs->item(i, 1)->text();
			if (params != "<unknown>") {
				name = ui.libProcs->item(i, 0)->text();
				break;
			}
		}
		if (name.isEmpty())
			return;
	} else
		name = ui.libProcs->item(row, 0)->text();

	sigFile = decompilerThread->getDecompiler()->getSigFile(name);
	QString filename = sigFile;

	int pos = sigFile.lastIndexOf(QRegExp("[/\\\\]"));
	if (pos != -1)
		sigFile = sigFile.right(sigFile.length() - pos - 1);
	QString sigFileStar = sigFile;
	sigFileStar.append("*");

	QTextEdit *n = NULL;
	for (int i = 0; i < ui.tabWidget->count(); i++)
		if (ui.tabWidget->tabText(i) == sigFile || ui.tabWidget->tabText(i) == sigFileStar) {
			n = dynamic_cast<QTextEdit*>(ui.tabWidget->widget(i));
			break;
		}
	if (n == NULL) {
		n = new QTextEdit();
		QFile file(filename);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			return;
		QTextStream in(&file);
		QString contents = in.readAll();
		file.close();
		n->insertPlainText(contents);
		openFiles[n] = filename;
		signatureFiles.insert(n);
		connect(n, SIGNAL(textChanged()), this, SLOT(currentTabTextChanged()));
		ui.tabWidget->addTab(n, sigFile);
	}
	ui.tabWidget->setCurrentWidget(n);
	if (existing)
		n->find(name, QTextDocument::FindBackward | QTextDocument::FindCaseSensitively | QTextDocument::FindWholeWords);
	else {
		QTextCursor cursor = n->textCursor();
		cursor.clearSelection();
		cursor.movePosition(QTextCursor::End);
		n->setTextCursor(cursor);
		QString comment = "// unknown library proc: ";
		comment.append(ui.libProcs->item(row, 0)->text());
		comment.append("\n");
		n->insertPlainText(comment);
	}
}

void MainWindow::on_actionCut_activated()
{
	if (ui.tabWidget->currentIndex() != 0) {
		QTextEdit *n = dynamic_cast<QTextEdit*>(ui.tabWidget->currentWidget());
		if (n)
			n->cut();
	}
}

void MainWindow::on_actionCopy_activated()
{
	if (ui.tabWidget->currentIndex() != 0) {
		QTextEdit *n = dynamic_cast<QTextEdit*>(ui.tabWidget->currentWidget());
		if (n)
			n->copy();
	}
}

void MainWindow::on_actionPaste_activated()
{
	if (ui.tabWidget->currentIndex() != 0) {
		QTextEdit *n = dynamic_cast<QTextEdit*>(ui.tabWidget->currentWidget());
		if (n)
			n->paste();
	}
}

void MainWindow::on_actionDelete_activated()
{
	if (ui.tabWidget->currentIndex() != 0) {
		QTextEdit *n = dynamic_cast<QTextEdit*>(ui.tabWidget->currentWidget());
		if (n)
			n->textCursor().removeSelectedText();
	}
}

void MainWindow::on_actionFind_activated()
{
}

void MainWindow::on_actionFind_Next_activated()
{
}

void MainWindow::on_actionGo_To_activated()
{
}

void MainWindow::on_actionSelect_All_activated()
{
	if (ui.tabWidget->currentIndex() != 0) {
		QTextEdit *n = dynamic_cast<QTextEdit*>(ui.tabWidget->currentWidget());
		if (n)
			n->selectAll();
	}
}

void MainWindow::on_actionLoad_activated()
{
	showLoadPage();
}

void MainWindow::on_actionDecode_activated()
{
	showDecodePage();
}

void MainWindow::on_actionDecompile_activated()
{
	showDecompilePage();
}

void MainWindow::on_actionGenerate_Code_activated()
{
	showGenerateCodePage();
}

void MainWindow::on_actionStructs_activated()
{
	for (int i = 0; i < ui.tabWidget->count(); i++)
		if (ui.tabWidget->widget(i) == structs)
			return;
	ui.tabWidget->addTab(structs, "Structs");
	ui.tabWidget->setCurrentWidget(structs);
}

void MainWindow::on_structName_returnPressed()
{
	decompilerThread->getDecompiler()->getCompoundMembers(ui.structName->text(), ui.structMembers);
}

void MainWindow::on_actionBoomerang_Website_activated()
{

}

