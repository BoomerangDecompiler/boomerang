#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>

class Ui_AboutDialog
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QFrame *frame_2;
    QSpacerItem *spacerItem;
    QLabel *label;
    QSpacerItem *spacerItem1;
    QHBoxLayout *hboxLayout1;
    QSpacerItem *spacerItem2;
    QLabel *VersionLabel;
    QSpacerItem *spacerItem3;
    QHBoxLayout *hboxLayout2;
    QSpacerItem *spacerItem4;
    QLabel *label_3;
    QSpacerItem *spacerItem5;
    QHBoxLayout *hboxLayout3;
    QSpacerItem *spacerItem6;
    QPushButton *OK_Button;
    QSpacerItem *spacerItem7;

    void setupUi(QDialog *AboutDialog)
    {
    AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
    AboutDialog->resize(QSize(319, 174).expandedTo(AboutDialog->minimumSizeHint()));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(AboutDialog->sizePolicy().hasHeightForWidth());
    AboutDialog->setSizePolicy(sizePolicy);
    AboutDialog->setSizeIncrement(QSize(11, 0));
    AboutDialog->setWindowIcon(QIcon());
    AboutDialog->setSizeGripEnabled(false);
    AboutDialog->setModal(true);
    vboxLayout = new QVBoxLayout(AboutDialog);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    frame_2 = new QFrame(AboutDialog);
    frame_2->setObjectName(QString::fromUtf8("frame_2"));
    frame_2->setFrameShape(QFrame::StyledPanel);
    frame_2->setFrameShadow(QFrame::Raised);

    hboxLayout->addWidget(frame_2);

    spacerItem = new QSpacerItem(44, 33, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem);

    label = new QLabel(AboutDialog);
    label->setObjectName(QString::fromUtf8("label"));

    hboxLayout->addWidget(label);

    spacerItem1 = new QSpacerItem(43, 33, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout->addItem(spacerItem1);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    spacerItem2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem2);

    VersionLabel = new QLabel(AboutDialog);
    VersionLabel->setObjectName(QString::fromUtf8("VersionLabel"));
    QFont font;
    font.setFamily(QString::fromUtf8("MS Shell Dlg"));
    font.setPointSize(8);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(75);
    font.setStrikeOut(false);
    VersionLabel->setFont(font);

    hboxLayout1->addWidget(VersionLabel);

    spacerItem3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout1->addItem(spacerItem3);


    vboxLayout->addLayout(hboxLayout1);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    spacerItem4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem4);

    label_3 = new QLabel(AboutDialog);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    hboxLayout2->addWidget(label_3);

    spacerItem5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout2->addItem(spacerItem5);


    vboxLayout->addLayout(hboxLayout2);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    spacerItem6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout3->addItem(spacerItem6);

    OK_Button = new QPushButton(AboutDialog);
    OK_Button->setObjectName(QString::fromUtf8("OK_Button"));
    OK_Button->setDefault(true);

    hboxLayout3->addWidget(OK_Button);

    spacerItem7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout3->addItem(spacerItem7);


    vboxLayout->addLayout(hboxLayout3);

    retranslateUi(AboutDialog);
    QObject::connect(OK_Button, SIGNAL(clicked()), AboutDialog, SLOT(close()));

    QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
    AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Boomerang", 0, QApplication::UnicodeUTF8));
    label->setText(QApplication::translate("AboutDialog", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-size:14pt; font-weight:600; color:#a52a2a;\">Boomerang decompiler </span></p></body></html>", 0, QApplication::UnicodeUTF8));
    VersionLabel->setText(QApplication::translate("AboutDialog", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8pt; font-weight:600; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:400;\"><span style=\" font-size:12pt; font-weight:600;\">alpha 0.3 00/Jan/3000</span></p></body></html>", 0, QApplication::UnicodeUTF8));
    label_3->setText(QApplication::translate("AboutDialog", "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:MS Shell Dlg; font-size:8.25pt; font-weight:400; font-style:normal; text-decoration:none;\"><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-size:10pt;\">You are using the QT GUI interface to</span></p><p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:10pt;\">the Boomerang machine code decompiler.</p></body></html>", 0, QApplication::UnicodeUTF8));
    OK_Button->setText(QApplication::translate("AboutDialog", "OK", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(AboutDialog);
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

#endif // UI_ABOUT_H
