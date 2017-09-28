#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "LoggingSettingsDlg.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"

#include "boomerang-gui/ui_LoggingSettingsDlg.h"


LoggingSettingsDlg::LoggingSettingsDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoggingSettingsDlg)
{
    ui->setupUi(this);
    ui->chkVerbose->setChecked(SETTING(vFlag));
    ui->chkLiveness->setChecked(SETTING(debugLiveness));
    ui->chkUnused->setChecked(SETTING(debugUnused));
    ui->chkTypeAnalysis->setChecked(SETTING(debugTA));
    ui->chkDecoder->setChecked(SETTING(debugDecoder));
    ui->chkCodegen->setChecked(SETTING(debugGen));
    ui->chkSwitch->setChecked(SETTING(debugSwitch));
}


LoggingSettingsDlg::~LoggingSettingsDlg()
{
    delete ui;
}


void LoggingSettingsDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);

    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        break;
    }
}


void LoggingSettingsDlg::on_btnApply_clicked()
{
    SETTING(vFlag)         = ui->chkVerbose->isChecked();
    SETTING(debugTA)       = ui->chkTypeAnalysis->isChecked();
    SETTING(debugLiveness) = ui->chkLiveness->isChecked();
    SETTING(debugUnused)   = ui->chkUnused->isChecked();
    SETTING(debugDecoder)  = ui->chkDecoder->isChecked();
    SETTING(debugGen)      = ui->chkCodegen->isChecked();
    SETTING(debugSwitch)   = ui->chkSwitch->isChecked();
}


void LoggingSettingsDlg::on_btnOk_clicked()
{
    on_btnApply_clicked();
    accept();
}
