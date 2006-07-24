#ifndef UI_BOOMERANG_H
#define UI_BOOMERANG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QProgressBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QTableWidget>
#include <QtGui/QTreeWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

class Ui_MainWindow
{
public:
    QAction *actionNew;
    QAction *actionExit;
    QAction *actionOpen;
    QAction *actionClose;
    QAction *actionStep;
    QAction *actionEnable;
    QAction *actionSave;
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionDelete;
    QAction *actionSelect_All;
    QAction *actionFind;
    QAction *actionFind_Next;
    QAction *actionGo_To;
    QAction *actionSelect_All1;
    QAction *actionStructs;
    QAction *actionLoad;
    QAction *actionDecode;
    QAction *actionDecompile;
    QAction *actionGenerate_Code;
    QAction *actionBoomerang_Website;
    QAction *actionLoggingOptions;
    QAction *actionDecodeOptions;
    QAction *actionDecompileOptions;
    QAction *actionAbout;
    QAction *actionAboutQt;
    QWidget *centralwidget;
    QHBoxLayout *hboxLayout;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *vboxLayout;
    QVBoxLayout *vboxLayout1;
    QHBoxLayout *hboxLayout1;
    QPushButton *toLoadButton;
    QVBoxLayout *vboxLayout2;
    QPushButton *loadButton;
    QLabel *label;
    QPushButton *toDecodeButton;
    QVBoxLayout *vboxLayout3;
    QPushButton *decodeButton;
    QLabel *label_2;
    QPushButton *toDecompileButton;
    QVBoxLayout *vboxLayout4;
    QPushButton *decompileButton;
    QLabel *label_3;
    QPushButton *toGenerateCodeButton;
    QVBoxLayout *vboxLayout5;
    QPushButton *generateCodeButton;
    QLabel *label_4;
    QStackedWidget *stackedWidget;
    QWidget *start;
    QVBoxLayout *vboxLayout6;
    QHBoxLayout *hboxLayout2;
    QLabel *label_5;
    QComboBox *inputFileComboBox;
    QPushButton *inputFileBrowseButton;
    QHBoxLayout *hboxLayout3;
    QLabel *label_8;
    QComboBox *outputPathComboBox;
    QPushButton *outputPathBrowseButton;
    QCheckBox *enableDebugCheckBox;
    QSpacerItem *spacerItem;
    QWidget *load;
    QVBoxLayout *vboxLayout7;
    QHBoxLayout *hboxLayout4;
    QLabel *label_10;
    QLabel *machineTypeLabel;
    QSpacerItem *spacerItem1;
    QHBoxLayout *hboxLayout5;
    QVBoxLayout *vboxLayout8;
    QHBoxLayout *hboxLayout6;
    QLabel *label_12;
    QSpacerItem *spacerItem2;
    QTableWidget *entrypoints;
    QHBoxLayout *hboxLayout7;
    QLineEdit *addressEdit;
    QLineEdit *nameEdit;
    QPushButton *AddButton;
    QPushButton *removeButton;
    QVBoxLayout *vboxLayout9;
    QHBoxLayout *hboxLayout8;
    QLabel *label_11;
    QSpacerItem *spacerItem3;
    QTableWidget *sections;
    QWidget *decode;
    QVBoxLayout *vboxLayout10;
    QHBoxLayout *hboxLayout9;
    QVBoxLayout *vboxLayout11;
    QLabel *label_7;
    QTableWidget *libProcs;
    QVBoxLayout *vboxLayout12;
    QLabel *label_6;
    QTableWidget *userProcs;
    QWidget *Decompile;
    QVBoxLayout *vboxLayout13;
    QHBoxLayout *hboxLayout10;
    QLabel *label_9;
    QProgressBar *progressDecompile;
    QTreeWidget *decompileProcsTreeWidget;
    QWidget *Generate_Code;
    QVBoxLayout *vboxLayout14;
    QHBoxLayout *hboxLayout11;
    QLabel *label_13;
    QProgressBar *progressGenerateCode;
    QTreeWidget *clusters;
    QHBoxLayout *hboxLayout12;
    QLineEdit *newFileEdit;
    QPushButton *newFileButton;
    QWidget *tab_2;
    QVBoxLayout *vboxLayout15;
    QHBoxLayout *hboxLayout13;
    QLabel *label_14;
    QLineEdit *structName;
    QTableWidget *structMembers;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuWindow;
    QMenu *menuSettings;
    QMenu *menuDebug_2;
    QMenu *menuView;
    QMenu *menuHelp;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
    MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(QSize(895, 638).expandedTo(MainWindow->minimumSizeHint()));
    actionNew = new QAction(MainWindow);
    actionNew->setObjectName(QString::fromUtf8("actionNew"));
    actionExit = new QAction(MainWindow);
    actionExit->setObjectName(QString::fromUtf8("actionExit"));
    actionOpen = new QAction(MainWindow);
    actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
    actionOpen->setEnabled(true);
    actionClose = new QAction(MainWindow);
    actionClose->setObjectName(QString::fromUtf8("actionClose"));
    actionClose->setEnabled(false);
    actionStep = new QAction(MainWindow);
    actionStep->setObjectName(QString::fromUtf8("actionStep"));
    actionStep->setEnabled(false);
    actionEnable = new QAction(MainWindow);
    actionEnable->setObjectName(QString::fromUtf8("actionEnable"));
    actionEnable->setCheckable(true);
    actionSave = new QAction(MainWindow);
    actionSave->setObjectName(QString::fromUtf8("actionSave"));
    actionSave->setEnabled(false);
    actionCut = new QAction(MainWindow);
    actionCut->setObjectName(QString::fromUtf8("actionCut"));
    actionCopy = new QAction(MainWindow);
    actionCopy->setObjectName(QString::fromUtf8("actionCopy"));
    actionPaste = new QAction(MainWindow);
    actionPaste->setObjectName(QString::fromUtf8("actionPaste"));
    actionDelete = new QAction(MainWindow);
    actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
    actionSelect_All = new QAction(MainWindow);
    actionSelect_All->setObjectName(QString::fromUtf8("actionSelect_All"));
    actionFind = new QAction(MainWindow);
    actionFind->setObjectName(QString::fromUtf8("actionFind"));
    actionFind_Next = new QAction(MainWindow);
    actionFind_Next->setObjectName(QString::fromUtf8("actionFind_Next"));
    actionGo_To = new QAction(MainWindow);
    actionGo_To->setObjectName(QString::fromUtf8("actionGo_To"));
    actionSelect_All1 = new QAction(MainWindow);
    actionSelect_All1->setObjectName(QString::fromUtf8("actionSelect_All1"));
    actionStructs = new QAction(MainWindow);
    actionStructs->setObjectName(QString::fromUtf8("actionStructs"));
    actionLoad = new QAction(MainWindow);
    actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
    actionDecode = new QAction(MainWindow);
    actionDecode->setObjectName(QString::fromUtf8("actionDecode"));
    actionDecompile = new QAction(MainWindow);
    actionDecompile->setObjectName(QString::fromUtf8("actionDecompile"));
    actionGenerate_Code = new QAction(MainWindow);
    actionGenerate_Code->setObjectName(QString::fromUtf8("actionGenerate_Code"));
    actionBoomerang_Website = new QAction(MainWindow);
    actionBoomerang_Website->setObjectName(QString::fromUtf8("actionBoomerang_Website"));
    actionLoggingOptions = new QAction(MainWindow);
    actionLoggingOptions->setObjectName(QString::fromUtf8("actionLoggingOptions"));
    actionDecodeOptions = new QAction(MainWindow);
    actionDecodeOptions->setObjectName(QString::fromUtf8("actionDecodeOptions"));
    actionDecompileOptions = new QAction(MainWindow);
    actionDecompileOptions->setObjectName(QString::fromUtf8("actionDecompileOptions"));
    actionAbout = new QAction(MainWindow);
    actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
    actionAboutQt = new QAction(MainWindow);
    actionAboutQt->setObjectName(QString::fromUtf8("actionAboutQt"));
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    hboxLayout = new QHBoxLayout(centralwidget);
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(9);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    tabWidget = new QTabWidget(centralwidget);
    tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
    tabWidget->setSizePolicy(sizePolicy);
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    vboxLayout = new QVBoxLayout(tab);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    toLoadButton = new QPushButton(tab);
    toLoadButton->setObjectName(QString::fromUtf8("toLoadButton"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(toLoadButton->sizePolicy().hasHeightForWidth());
    toLoadButton->setSizePolicy(sizePolicy1);
    toLoadButton->setMinimumSize(QSize(38, 38));
    toLoadButton->setMaximumSize(QSize(38, 38));
    toLoadButton->setIcon(QIcon(QString::fromUtf8("rarrow.bmp")));
    toLoadButton->setIconSize(QSize(32, 32));

    hboxLayout1->addWidget(toLoadButton);

    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setSpacing(0);
    vboxLayout2->setMargin(0);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    loadButton = new QPushButton(tab);
    loadButton->setObjectName(QString::fromUtf8("loadButton"));
    loadButton->setEnabled(true);
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(loadButton->sizePolicy().hasHeightForWidth());
    loadButton->setSizePolicy(sizePolicy2);
    loadButton->setMinimumSize(QSize(70, 70));
    loadButton->setMaximumSize(QSize(70, 70));
    loadButton->setIcon(QIcon(QString::fromUtf8("load1.bmp")));
    loadButton->setIconSize(QSize(64, 64));

    vboxLayout2->addWidget(loadButton);

    label = new QLabel(tab);
    label->setObjectName(QString::fromUtf8("label"));
    label->setAlignment(Qt::AlignCenter);

    vboxLayout2->addWidget(label);


    hboxLayout1->addLayout(vboxLayout2);

    toDecodeButton = new QPushButton(tab);
    toDecodeButton->setObjectName(QString::fromUtf8("toDecodeButton"));
    toDecodeButton->setEnabled(false);
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy3.setHorizontalStretch(0);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(toDecodeButton->sizePolicy().hasHeightForWidth());
    toDecodeButton->setSizePolicy(sizePolicy3);
    toDecodeButton->setMinimumSize(QSize(38, 38));
    toDecodeButton->setMaximumSize(QSize(38, 38));
    toDecodeButton->setIcon(QIcon(QString::fromUtf8("rarrow.bmp")));
    toDecodeButton->setIconSize(QSize(32, 32));

    hboxLayout1->addWidget(toDecodeButton);

    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setSpacing(0);
    vboxLayout3->setMargin(0);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    decodeButton = new QPushButton(tab);
    decodeButton->setObjectName(QString::fromUtf8("decodeButton"));
    decodeButton->setEnabled(false);
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy4.setHorizontalStretch(0);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(decodeButton->sizePolicy().hasHeightForWidth());
    decodeButton->setSizePolicy(sizePolicy4);
    decodeButton->setMinimumSize(QSize(70, 70));
    decodeButton->setMaximumSize(QSize(70, 70));
    decodeButton->setIcon(QIcon(QString::fromUtf8("decode.bmp")));
    decodeButton->setIconSize(QSize(64, 64));

    vboxLayout3->addWidget(decodeButton);

    label_2 = new QLabel(tab);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setAlignment(Qt::AlignCenter);

    vboxLayout3->addWidget(label_2);


    hboxLayout1->addLayout(vboxLayout3);

    toDecompileButton = new QPushButton(tab);
    toDecompileButton->setObjectName(QString::fromUtf8("toDecompileButton"));
    toDecompileButton->setEnabled(false);
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(toDecompileButton->sizePolicy().hasHeightForWidth());
    toDecompileButton->setSizePolicy(sizePolicy5);
    toDecompileButton->setMaximumSize(QSize(38, 38));
    toDecompileButton->setIcon(QIcon(QString::fromUtf8("rarrow.bmp")));
    toDecompileButton->setIconSize(QSize(32, 32));

    hboxLayout1->addWidget(toDecompileButton);

    vboxLayout4 = new QVBoxLayout();
    vboxLayout4->setSpacing(0);
    vboxLayout4->setMargin(0);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    decompileButton = new QPushButton(tab);
    decompileButton->setObjectName(QString::fromUtf8("decompileButton"));
    decompileButton->setEnabled(false);
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(decompileButton->sizePolicy().hasHeightForWidth());
    decompileButton->setSizePolicy(sizePolicy6);
    decompileButton->setMinimumSize(QSize(70, 70));
    decompileButton->setMaximumSize(QSize(70, 70));
    decompileButton->setIcon(QIcon(QString::fromUtf8("decompile.bmp")));
    decompileButton->setIconSize(QSize(64, 64));

    vboxLayout4->addWidget(decompileButton);

    label_3 = new QLabel(tab);
    label_3->setObjectName(QString::fromUtf8("label_3"));
    label_3->setAlignment(Qt::AlignCenter);

    vboxLayout4->addWidget(label_3);


    hboxLayout1->addLayout(vboxLayout4);

    toGenerateCodeButton = new QPushButton(tab);
    toGenerateCodeButton->setObjectName(QString::fromUtf8("toGenerateCodeButton"));
    toGenerateCodeButton->setEnabled(false);
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(toGenerateCodeButton->sizePolicy().hasHeightForWidth());
    toGenerateCodeButton->setSizePolicy(sizePolicy7);
    toGenerateCodeButton->setMinimumSize(QSize(38, 38));
    toGenerateCodeButton->setMaximumSize(QSize(38, 38));
    toGenerateCodeButton->setIcon(QIcon(QString::fromUtf8("rarrow.bmp")));
    toGenerateCodeButton->setIconSize(QSize(32, 32));

    hboxLayout1->addWidget(toGenerateCodeButton);

    vboxLayout5 = new QVBoxLayout();
    vboxLayout5->setSpacing(0);
    vboxLayout5->setMargin(0);
    vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
    generateCodeButton = new QPushButton(tab);
    generateCodeButton->setObjectName(QString::fromUtf8("generateCodeButton"));
    generateCodeButton->setEnabled(false);
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(generateCodeButton->sizePolicy().hasHeightForWidth());
    generateCodeButton->setSizePolicy(sizePolicy8);
    generateCodeButton->setMinimumSize(QSize(70, 70));
    generateCodeButton->setMaximumSize(QSize(70, 70));
    generateCodeButton->setIcon(QIcon(QString::fromUtf8("gencode.bmp")));
    generateCodeButton->setIconSize(QSize(64, 64));

    vboxLayout5->addWidget(generateCodeButton);

    label_4 = new QLabel(tab);
    label_4->setObjectName(QString::fromUtf8("label_4"));
    label_4->setAlignment(Qt::AlignCenter);
    label_4->setWordWrap(true);

    vboxLayout5->addWidget(label_4);


    hboxLayout1->addLayout(vboxLayout5);


    vboxLayout1->addLayout(hboxLayout1);


    vboxLayout->addLayout(vboxLayout1);

    stackedWidget = new QStackedWidget(tab);
    stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
    start = new QWidget();
    start->setObjectName(QString::fromUtf8("start"));
    vboxLayout6 = new QVBoxLayout(start);
    vboxLayout6->setSpacing(6);
    vboxLayout6->setMargin(9);
    vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    label_5 = new QLabel(start);
    label_5->setObjectName(QString::fromUtf8("label_5"));

    hboxLayout2->addWidget(label_5);

    inputFileComboBox = new QComboBox(start);
    inputFileComboBox->setObjectName(QString::fromUtf8("inputFileComboBox"));
    QSizePolicy sizePolicy9(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy9.setHorizontalStretch(0);
    sizePolicy9.setVerticalStretch(0);
    sizePolicy9.setHeightForWidth(inputFileComboBox->sizePolicy().hasHeightForWidth());
    inputFileComboBox->setSizePolicy(sizePolicy9);
    inputFileComboBox->setEditable(true);

    hboxLayout2->addWidget(inputFileComboBox);

    inputFileBrowseButton = new QPushButton(start);
    inputFileBrowseButton->setObjectName(QString::fromUtf8("inputFileBrowseButton"));
    inputFileBrowseButton->setMinimumSize(QSize(75, 23));
    inputFileBrowseButton->setMaximumSize(QSize(75, 23));

    hboxLayout2->addWidget(inputFileBrowseButton);


    vboxLayout6->addLayout(hboxLayout2);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    label_8 = new QLabel(start);
    label_8->setObjectName(QString::fromUtf8("label_8"));

    hboxLayout3->addWidget(label_8);

    outputPathComboBox = new QComboBox(start);
    outputPathComboBox->setObjectName(QString::fromUtf8("outputPathComboBox"));
    QSizePolicy sizePolicy10(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy10.setHorizontalStretch(0);
    sizePolicy10.setVerticalStretch(0);
    sizePolicy10.setHeightForWidth(outputPathComboBox->sizePolicy().hasHeightForWidth());
    outputPathComboBox->setSizePolicy(sizePolicy10);
    outputPathComboBox->setEditable(true);

    hboxLayout3->addWidget(outputPathComboBox);

    outputPathBrowseButton = new QPushButton(start);
    outputPathBrowseButton->setObjectName(QString::fromUtf8("outputPathBrowseButton"));
    outputPathBrowseButton->setMinimumSize(QSize(75, 23));
    outputPathBrowseButton->setMaximumSize(QSize(75, 23));

    hboxLayout3->addWidget(outputPathBrowseButton);


    vboxLayout6->addLayout(hboxLayout3);

    enableDebugCheckBox = new QCheckBox(start);
    enableDebugCheckBox->setObjectName(QString::fromUtf8("enableDebugCheckBox"));

    vboxLayout6->addWidget(enableDebugCheckBox);

    spacerItem = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout6->addItem(spacerItem);

    stackedWidget->addWidget(start);
    load = new QWidget();
    load->setObjectName(QString::fromUtf8("load"));
    vboxLayout7 = new QVBoxLayout(load);
    vboxLayout7->setSpacing(6);
    vboxLayout7->setMargin(9);
    vboxLayout7->setObjectName(QString::fromUtf8("vboxLayout7"));
    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(6);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    label_10 = new QLabel(load);
    label_10->setObjectName(QString::fromUtf8("label_10"));

    hboxLayout4->addWidget(label_10);

    machineTypeLabel = new QLabel(load);
    machineTypeLabel->setObjectName(QString::fromUtf8("machineTypeLabel"));

    hboxLayout4->addWidget(machineTypeLabel);

    spacerItem1 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout4->addItem(spacerItem1);


    vboxLayout7->addLayout(hboxLayout4);

    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    vboxLayout8 = new QVBoxLayout();
    vboxLayout8->setSpacing(6);
    vboxLayout8->setMargin(0);
    vboxLayout8->setObjectName(QString::fromUtf8("vboxLayout8"));
    hboxLayout6 = new QHBoxLayout();
    hboxLayout6->setSpacing(6);
    hboxLayout6->setMargin(0);
    hboxLayout6->setObjectName(QString::fromUtf8("hboxLayout6"));
    label_12 = new QLabel(load);
    label_12->setObjectName(QString::fromUtf8("label_12"));

    hboxLayout6->addWidget(label_12);

    spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout6->addItem(spacerItem2);


    vboxLayout8->addLayout(hboxLayout6);

    entrypoints = new QTableWidget(load);
    entrypoints->setObjectName(QString::fromUtf8("entrypoints"));
    entrypoints->setShowGrid(false);

    vboxLayout8->addWidget(entrypoints);

    hboxLayout7 = new QHBoxLayout();
    hboxLayout7->setSpacing(6);
    hboxLayout7->setMargin(0);
    hboxLayout7->setObjectName(QString::fromUtf8("hboxLayout7"));
    addressEdit = new QLineEdit(load);
    addressEdit->setObjectName(QString::fromUtf8("addressEdit"));
    QSizePolicy sizePolicy11(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
    sizePolicy11.setHorizontalStretch(0);
    sizePolicy11.setVerticalStretch(0);
    sizePolicy11.setHeightForWidth(addressEdit->sizePolicy().hasHeightForWidth());
    addressEdit->setSizePolicy(sizePolicy11);

    hboxLayout7->addWidget(addressEdit);

    nameEdit = new QLineEdit(load);
    nameEdit->setObjectName(QString::fromUtf8("nameEdit"));

    hboxLayout7->addWidget(nameEdit);

    AddButton = new QPushButton(load);
    AddButton->setObjectName(QString::fromUtf8("AddButton"));
    AddButton->setEnabled(false);
    AddButton->setMaximumSize(QSize(51, 16777215));

    hboxLayout7->addWidget(AddButton);

    removeButton = new QPushButton(load);
    removeButton->setObjectName(QString::fromUtf8("removeButton"));
    removeButton->setEnabled(false);
    removeButton->setMaximumSize(QSize(71, 16777215));

    hboxLayout7->addWidget(removeButton);


    vboxLayout8->addLayout(hboxLayout7);


    hboxLayout5->addLayout(vboxLayout8);

    vboxLayout9 = new QVBoxLayout();
    vboxLayout9->setSpacing(6);
    vboxLayout9->setMargin(0);
    vboxLayout9->setObjectName(QString::fromUtf8("vboxLayout9"));
    hboxLayout8 = new QHBoxLayout();
    hboxLayout8->setSpacing(6);
    hboxLayout8->setMargin(0);
    hboxLayout8->setObjectName(QString::fromUtf8("hboxLayout8"));
    label_11 = new QLabel(load);
    label_11->setObjectName(QString::fromUtf8("label_11"));

    hboxLayout8->addWidget(label_11);

    spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout8->addItem(spacerItem3);


    vboxLayout9->addLayout(hboxLayout8);

    sections = new QTableWidget(load);
    sections->setObjectName(QString::fromUtf8("sections"));
    sections->setShowGrid(false);

    vboxLayout9->addWidget(sections);


    hboxLayout5->addLayout(vboxLayout9);


    vboxLayout7->addLayout(hboxLayout5);

    stackedWidget->addWidget(load);
    decode = new QWidget();
    decode->setObjectName(QString::fromUtf8("decode"));
    vboxLayout10 = new QVBoxLayout(decode);
    vboxLayout10->setSpacing(6);
    vboxLayout10->setMargin(9);
    vboxLayout10->setObjectName(QString::fromUtf8("vboxLayout10"));
    hboxLayout9 = new QHBoxLayout();
    hboxLayout9->setSpacing(6);
    hboxLayout9->setMargin(0);
    hboxLayout9->setObjectName(QString::fromUtf8("hboxLayout9"));
    vboxLayout11 = new QVBoxLayout();
    vboxLayout11->setSpacing(6);
    vboxLayout11->setMargin(0);
    vboxLayout11->setObjectName(QString::fromUtf8("vboxLayout11"));
    label_7 = new QLabel(decode);
    label_7->setObjectName(QString::fromUtf8("label_7"));

    vboxLayout11->addWidget(label_7);

    libProcs = new QTableWidget(decode);
    libProcs->setObjectName(QString::fromUtf8("libProcs"));
    libProcs->setEditTriggers(QAbstractItemView::NoEditTriggers);
    libProcs->setSelectionMode(QAbstractItemView::SingleSelection);
    libProcs->setSelectionBehavior(QAbstractItemView::SelectRows);
    libProcs->setShowGrid(false);
    libProcs->setSortingEnabled(true);

    vboxLayout11->addWidget(libProcs);


    hboxLayout9->addLayout(vboxLayout11);

    vboxLayout12 = new QVBoxLayout();
    vboxLayout12->setSpacing(6);
    vboxLayout12->setMargin(0);
    vboxLayout12->setObjectName(QString::fromUtf8("vboxLayout12"));
    label_6 = new QLabel(decode);
    label_6->setObjectName(QString::fromUtf8("label_6"));

    vboxLayout12->addWidget(label_6);

    userProcs = new QTableWidget(decode);
    userProcs->setObjectName(QString::fromUtf8("userProcs"));
    userProcs->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::NoEditTriggers);
    userProcs->setShowGrid(false);
    userProcs->setColumnCount(3);
    userProcs->setSortingEnabled(true);

    vboxLayout12->addWidget(userProcs);


    hboxLayout9->addLayout(vboxLayout12);


    vboxLayout10->addLayout(hboxLayout9);

    stackedWidget->addWidget(decode);
    Decompile = new QWidget();
    Decompile->setObjectName(QString::fromUtf8("Decompile"));
    vboxLayout13 = new QVBoxLayout(Decompile);
    vboxLayout13->setSpacing(6);
    vboxLayout13->setMargin(9);
    vboxLayout13->setObjectName(QString::fromUtf8("vboxLayout13"));
    hboxLayout10 = new QHBoxLayout();
    hboxLayout10->setSpacing(6);
    hboxLayout10->setMargin(0);
    hboxLayout10->setObjectName(QString::fromUtf8("hboxLayout10"));
    label_9 = new QLabel(Decompile);
    label_9->setObjectName(QString::fromUtf8("label_9"));

    hboxLayout10->addWidget(label_9);

    progressDecompile = new QProgressBar(Decompile);
    progressDecompile->setObjectName(QString::fromUtf8("progressDecompile"));
    progressDecompile->setValue(24);
    progressDecompile->setOrientation(Qt::Horizontal);

    hboxLayout10->addWidget(progressDecompile);


    vboxLayout13->addLayout(hboxLayout10);

    decompileProcsTreeWidget = new QTreeWidget(Decompile);
    decompileProcsTreeWidget->setObjectName(QString::fromUtf8("decompileProcsTreeWidget"));
    decompileProcsTreeWidget->setEditTriggers(QAbstractItemView::AnyKeyPressed|QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::NoEditTriggers);
    decompileProcsTreeWidget->setItemsExpandable(true);

    vboxLayout13->addWidget(decompileProcsTreeWidget);

    stackedWidget->addWidget(Decompile);
    Generate_Code = new QWidget();
    Generate_Code->setObjectName(QString::fromUtf8("Generate_Code"));
    vboxLayout14 = new QVBoxLayout(Generate_Code);
    vboxLayout14->setSpacing(6);
    vboxLayout14->setMargin(9);
    vboxLayout14->setObjectName(QString::fromUtf8("vboxLayout14"));
    hboxLayout11 = new QHBoxLayout();
    hboxLayout11->setSpacing(6);
    hboxLayout11->setMargin(0);
    hboxLayout11->setObjectName(QString::fromUtf8("hboxLayout11"));
    label_13 = new QLabel(Generate_Code);
    label_13->setObjectName(QString::fromUtf8("label_13"));

    hboxLayout11->addWidget(label_13);

    progressGenerateCode = new QProgressBar(Generate_Code);
    progressGenerateCode->setObjectName(QString::fromUtf8("progressGenerateCode"));
    progressGenerateCode->setValue(24);
    progressGenerateCode->setOrientation(Qt::Horizontal);

    hboxLayout11->addWidget(progressGenerateCode);


    vboxLayout14->addLayout(hboxLayout11);

    clusters = new QTreeWidget(Generate_Code);
    clusters->setObjectName(QString::fromUtf8("clusters"));

    vboxLayout14->addWidget(clusters);

    hboxLayout12 = new QHBoxLayout();
    hboxLayout12->setSpacing(6);
    hboxLayout12->setMargin(0);
    hboxLayout12->setObjectName(QString::fromUtf8("hboxLayout12"));
    newFileEdit = new QLineEdit(Generate_Code);
    newFileEdit->setObjectName(QString::fromUtf8("newFileEdit"));

    hboxLayout12->addWidget(newFileEdit);

    newFileButton = new QPushButton(Generate_Code);
    newFileButton->setObjectName(QString::fromUtf8("newFileButton"));
    newFileButton->setEnabled(false);

    hboxLayout12->addWidget(newFileButton);


    vboxLayout14->addLayout(hboxLayout12);

    stackedWidget->addWidget(Generate_Code);

    vboxLayout->addWidget(stackedWidget);

    tabWidget->addTab(tab, QApplication::translate("MainWindow", "Workflow", 0, QApplication::UnicodeUTF8));
    tab_2 = new QWidget();
    tab_2->setObjectName(QString::fromUtf8("tab_2"));
    vboxLayout15 = new QVBoxLayout(tab_2);
    vboxLayout15->setSpacing(6);
    vboxLayout15->setMargin(9);
    vboxLayout15->setObjectName(QString::fromUtf8("vboxLayout15"));
    hboxLayout13 = new QHBoxLayout();
    hboxLayout13->setSpacing(6);
    hboxLayout13->setMargin(0);
    hboxLayout13->setObjectName(QString::fromUtf8("hboxLayout13"));
    label_14 = new QLabel(tab_2);
    label_14->setObjectName(QString::fromUtf8("label_14"));

    hboxLayout13->addWidget(label_14);

    structName = new QLineEdit(tab_2);
    structName->setObjectName(QString::fromUtf8("structName"));

    hboxLayout13->addWidget(structName);


    vboxLayout15->addLayout(hboxLayout13);

    structMembers = new QTableWidget(tab_2);
    structMembers->setObjectName(QString::fromUtf8("structMembers"));
    structMembers->setShowGrid(false);

    vboxLayout15->addWidget(structMembers);

    tabWidget->addTab(tab_2, QApplication::translate("MainWindow", "Structs", 0, QApplication::UnicodeUTF8));

    hboxLayout->addWidget(tabWidget);

    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 895, 21));
    menuFile = new QMenu(menubar);
    menuFile->setObjectName(QString::fromUtf8("menuFile"));
    menuWindow = new QMenu(menubar);
    menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
    menuSettings = new QMenu(menubar);
    menuSettings->setObjectName(QString::fromUtf8("menuSettings"));
    menuDebug_2 = new QMenu(menubar);
    menuDebug_2->setObjectName(QString::fromUtf8("menuDebug_2"));
    menuView = new QMenu(menubar);
    menuView->setObjectName(QString::fromUtf8("menuView"));
    menuHelp = new QMenu(menubar);
    menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    statusbar->setGeometry(QRect(0, 619, 895, 19));
    MainWindow->setStatusBar(statusbar);

    menubar->addAction(menuFile->menuAction());
    menubar->addAction(menuWindow->menuAction());
    menubar->addAction(menuView->menuAction());
    menubar->addAction(menuSettings->menuAction());
    menubar->addAction(menuDebug_2->menuAction());
    menubar->addAction(menuHelp->menuAction());
    menuFile->addAction(actionOpen);
    menuFile->addAction(actionSave);
    menuFile->addAction(actionClose);
    menuFile->addSeparator();
    menuFile->addAction(actionExit);
    menuWindow->addAction(actionCut);
    menuWindow->addAction(actionCopy);
    menuWindow->addAction(actionPaste);
    menuWindow->addAction(actionDelete);
    menuWindow->addSeparator();
    menuWindow->addAction(actionFind);
    menuWindow->addAction(actionFind_Next);
    menuWindow->addAction(actionGo_To);
    menuWindow->addSeparator();
    menuWindow->addAction(actionSelect_All);
    menuSettings->addAction(actionLoggingOptions);
    menuSettings->addAction(actionDecodeOptions);
    menuSettings->addAction(actionDecompileOptions);
    menuDebug_2->addAction(actionEnable);
    menuDebug_2->addAction(actionStep);
    menuView->addAction(actionLoad);
    menuView->addAction(actionDecode);
    menuView->addAction(actionDecompile);
    menuView->addAction(actionGenerate_Code);
    menuView->addAction(actionStructs);
    menuHelp->addAction(actionAbout);
    menuHelp->addAction(actionAboutQt);
    menuHelp->addAction(actionBoomerang_Website);
    retranslateUi(MainWindow);
    QObject::connect(actionExit, SIGNAL(activated()), MainWindow, SLOT(close()));
    QObject::connect(enableDebugCheckBox, SIGNAL(toggled(bool)), actionEnable, SLOT(setChecked(bool)));

    stackedWidget->setCurrentIndex(2);


    QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
    MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
    actionNew->setText(QApplication::translate("MainWindow", "New", 0, QApplication::UnicodeUTF8));
    actionExit->setText(QApplication::translate("MainWindow", "Exit", 0, QApplication::UnicodeUTF8));
    actionOpen->setText(QApplication::translate("MainWindow", "Open", 0, QApplication::UnicodeUTF8));
    actionClose->setText(QApplication::translate("MainWindow", "Close", 0, QApplication::UnicodeUTF8));
    actionStep->setText(QApplication::translate("MainWindow", "Step", 0, QApplication::UnicodeUTF8));
    actionEnable->setText(QApplication::translate("MainWindow", "Enable", 0, QApplication::UnicodeUTF8));
    actionSave->setText(QApplication::translate("MainWindow", "Save", 0, QApplication::UnicodeUTF8));
    actionSave->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
    actionCut->setText(QApplication::translate("MainWindow", "Cut", 0, QApplication::UnicodeUTF8));
    actionCut->setShortcut(QApplication::translate("MainWindow", "Ctrl+X", 0, QApplication::UnicodeUTF8));
    actionCopy->setText(QApplication::translate("MainWindow", "Copy", 0, QApplication::UnicodeUTF8));
    actionCopy->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0, QApplication::UnicodeUTF8));
    actionPaste->setText(QApplication::translate("MainWindow", "Paste", 0, QApplication::UnicodeUTF8));
    actionPaste->setShortcut(QApplication::translate("MainWindow", "Ctrl+V", 0, QApplication::UnicodeUTF8));
    actionDelete->setText(QApplication::translate("MainWindow", "Delete", 0, QApplication::UnicodeUTF8));
    actionDelete->setShortcut(QApplication::translate("MainWindow", "Del", 0, QApplication::UnicodeUTF8));
    actionSelect_All->setText(QApplication::translate("MainWindow", "Select All", 0, QApplication::UnicodeUTF8));
    actionSelect_All->setShortcut(QApplication::translate("MainWindow", "Ctrl+A", 0, QApplication::UnicodeUTF8));
    actionFind->setText(QApplication::translate("MainWindow", "Find...", 0, QApplication::UnicodeUTF8));
    actionFind->setShortcut(QApplication::translate("MainWindow", "Ctrl+F", 0, QApplication::UnicodeUTF8));
    actionFind_Next->setText(QApplication::translate("MainWindow", "Find Next", 0, QApplication::UnicodeUTF8));
    actionFind_Next->setShortcut(QApplication::translate("MainWindow", "F3", 0, QApplication::UnicodeUTF8));
    actionGo_To->setText(QApplication::translate("MainWindow", "Go To...", 0, QApplication::UnicodeUTF8));
    actionGo_To->setShortcut(QApplication::translate("MainWindow", "Ctrl+G", 0, QApplication::UnicodeUTF8));
    actionSelect_All1->setText(QApplication::translate("MainWindow", "Select All", 0, QApplication::UnicodeUTF8));
    actionStructs->setText(QApplication::translate("MainWindow", "Structs", 0, QApplication::UnicodeUTF8));
    actionLoad->setText(QApplication::translate("MainWindow", "Load", 0, QApplication::UnicodeUTF8));
    actionDecode->setText(QApplication::translate("MainWindow", "Decode", 0, QApplication::UnicodeUTF8));
    actionDecompile->setText(QApplication::translate("MainWindow", "Decompile", 0, QApplication::UnicodeUTF8));
    actionGenerate_Code->setText(QApplication::translate("MainWindow", "Generate Code", 0, QApplication::UnicodeUTF8));
    actionBoomerang_Website->setText(QApplication::translate("MainWindow", "Boomerang Website", 0, QApplication::UnicodeUTF8));
    actionLoggingOptions->setText(QApplication::translate("MainWindow", "Logging", 0, QApplication::UnicodeUTF8));
    actionDecodeOptions->setText(QApplication::translate("MainWindow", "Decode", 0, QApplication::UnicodeUTF8));
    actionDecompileOptions->setText(QApplication::translate("MainWindow", "Decompile", 0, QApplication::UnicodeUTF8));
    actionAbout->setText(QApplication::translate("MainWindow", "About Boomerang", 0, QApplication::UnicodeUTF8));
    actionAboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0, QApplication::UnicodeUTF8));
    toLoadButton->setToolTip(QApplication::translate("MainWindow", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Load</p></body></html>", 0, QApplication::UnicodeUTF8));
    toLoadButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    loadButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("MainWindow", "Load", 0, QApplication::UnicodeUTF8));
    toDecodeButton->setToolTip(QApplication::translate("MainWindow", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Decode</p></body></html>", 0, QApplication::UnicodeUTF8));
    toDecodeButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    decodeButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    label_2->setText(QApplication::translate("MainWindow", "Decode", 0, QApplication::UnicodeUTF8));
    toDecompileButton->setToolTip(QApplication::translate("MainWindow", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Decompile</p></body></html>", 0, QApplication::UnicodeUTF8));
    toDecompileButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    decompileButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("MainWindow", "Decompile", 0, QApplication::UnicodeUTF8));
    toGenerateCodeButton->setToolTip(QApplication::translate("MainWindow", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Generate</p></body></html>", 0, QApplication::UnicodeUTF8));
    toGenerateCodeButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    generateCodeButton->setText(QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    label_4->setText(QApplication::translate("MainWindow", "Generate Code", 0, QApplication::UnicodeUTF8));
    label_5->setText(QApplication::translate("MainWindow", "Input file:", 0, QApplication::UnicodeUTF8));
    inputFileBrowseButton->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
    label_8->setText(QApplication::translate("MainWindow", "Output path:", 0, QApplication::UnicodeUTF8));
    outputPathComboBox->clear();
    outputPathComboBox->addItem(QApplication::translate("MainWindow", "./output", 0, QApplication::UnicodeUTF8));
    outputPathBrowseButton->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
    enableDebugCheckBox->setText(QApplication::translate("MainWindow", "Enable Debugging", 0, QApplication::UnicodeUTF8));
    label_10->setText(QApplication::translate("MainWindow", "Source architecture is", 0, QApplication::UnicodeUTF8));
    machineTypeLabel->setText(QApplication::translate("MainWindow", "<machine type>", 0, QApplication::UnicodeUTF8));
    label_12->setText(QApplication::translate("MainWindow", "Found entrypoints:", 0, QApplication::UnicodeUTF8));
    entrypoints->clear();
    entrypoints->setColumnCount(2);

    QTableWidgetItem *__colItem = new QTableWidgetItem();
    __colItem->setText(QApplication::translate("MainWindow", "Address", 0, QApplication::UnicodeUTF8));
    entrypoints->setHorizontalHeaderItem(0, __colItem);

    QTableWidgetItem *__colItem1 = new QTableWidgetItem();
    __colItem1->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    entrypoints->setHorizontalHeaderItem(1, __colItem1);
    entrypoints->setRowCount(0);
    AddButton->setText(QApplication::translate("MainWindow", "Add", 0, QApplication::UnicodeUTF8));
    removeButton->setText(QApplication::translate("MainWindow", "Remove", 0, QApplication::UnicodeUTF8));
    label_11->setText(QApplication::translate("MainWindow", "Sections:", 0, QApplication::UnicodeUTF8));
    sections->clear();
    sections->setColumnCount(3);

    QTableWidgetItem *__colItem2 = new QTableWidgetItem();
    __colItem2->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    sections->setHorizontalHeaderItem(0, __colItem2);

    QTableWidgetItem *__colItem3 = new QTableWidgetItem();
    __colItem3->setText(QApplication::translate("MainWindow", "Start", 0, QApplication::UnicodeUTF8));
    sections->setHorizontalHeaderItem(1, __colItem3);

    QTableWidgetItem *__colItem4 = new QTableWidgetItem();
    __colItem4->setText(QApplication::translate("MainWindow", "End", 0, QApplication::UnicodeUTF8));
    sections->setHorizontalHeaderItem(2, __colItem4);
    sections->setRowCount(0);
    label_7->setText(QApplication::translate("MainWindow", "Library Procedures:", 0, QApplication::UnicodeUTF8));
    libProcs->clear();
    libProcs->setColumnCount(2);

    QTableWidgetItem *__colItem5 = new QTableWidgetItem();
    __colItem5->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    libProcs->setHorizontalHeaderItem(0, __colItem5);

    QTableWidgetItem *__colItem6 = new QTableWidgetItem();
    __colItem6->setText(QApplication::translate("MainWindow", "Parameters", 0, QApplication::UnicodeUTF8));
    libProcs->setHorizontalHeaderItem(1, __colItem6);
    libProcs->setRowCount(0);
    label_6->setText(QApplication::translate("MainWindow", "User Procedures:", 0, QApplication::UnicodeUTF8));
    userProcs->clear();
    userProcs->setColumnCount(3);

    QTableWidgetItem *__colItem7 = new QTableWidgetItem();
    __colItem7->setText(QApplication::translate("MainWindow", "Address", 0, QApplication::UnicodeUTF8));
    userProcs->setHorizontalHeaderItem(0, __colItem7);

    QTableWidgetItem *__colItem8 = new QTableWidgetItem();
    __colItem8->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    userProcs->setHorizontalHeaderItem(1, __colItem8);

    QTableWidgetItem *__colItem9 = new QTableWidgetItem();
    __colItem9->setText(QApplication::translate("MainWindow", "Debug", 0, QApplication::UnicodeUTF8));
    userProcs->setHorizontalHeaderItem(2, __colItem9);
    userProcs->setRowCount(0);
    label_9->setText(QApplication::translate("MainWindow", "Decompiling... ", 0, QApplication::UnicodeUTF8));
    decompileProcsTreeWidget->headerItem()->setText(0, QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    label_13->setText(QApplication::translate("MainWindow", "Generating code...", 0, QApplication::UnicodeUTF8));
    clusters->headerItem()->setText(0, QApplication::translate("MainWindow", "", 0, QApplication::UnicodeUTF8));
    newFileButton->setText(QApplication::translate("MainWindow", "Add File", 0, QApplication::UnicodeUTF8));
    tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Workflow", 0, QApplication::UnicodeUTF8));
    label_14->setText(QApplication::translate("MainWindow", "Name:", 0, QApplication::UnicodeUTF8));
    structMembers->clear();
    structMembers->setColumnCount(4);

    QTableWidgetItem *__colItem10 = new QTableWidgetItem();
    __colItem10->setText(QApplication::translate("MainWindow", "Offset", 0, QApplication::UnicodeUTF8));
    structMembers->setHorizontalHeaderItem(0, __colItem10);

    QTableWidgetItem *__colItem11 = new QTableWidgetItem();
    __colItem11->setText(QApplication::translate("MainWindow", "Offset (bytes)", 0, QApplication::UnicodeUTF8));
    structMembers->setHorizontalHeaderItem(1, __colItem11);

    QTableWidgetItem *__colItem12 = new QTableWidgetItem();
    __colItem12->setText(QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
    structMembers->setHorizontalHeaderItem(2, __colItem12);

    QTableWidgetItem *__colItem13 = new QTableWidgetItem();
    __colItem13->setText(QApplication::translate("MainWindow", "Size", 0, QApplication::UnicodeUTF8));
    structMembers->setHorizontalHeaderItem(3, __colItem13);
    structMembers->setRowCount(0);
    tabWidget->setTabText(tabWidget->indexOf(tab_2), QApplication::translate("MainWindow", "Structs", 0, QApplication::UnicodeUTF8));
    menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    menuWindow->setTitle(QApplication::translate("MainWindow", "Edit", 0, QApplication::UnicodeUTF8));
    menuSettings->setTitle(QApplication::translate("MainWindow", "Settings", 0, QApplication::UnicodeUTF8));
    menuDebug_2->setTitle(QApplication::translate("MainWindow", "Debug", 0, QApplication::UnicodeUTF8));
    menuView->setTitle(QApplication::translate("MainWindow", "View", 0, QApplication::UnicodeUTF8));
    menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

#endif // UI_BOOMERANG_H
