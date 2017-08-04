#include "LoggingSettingsDlg.h"

#include "boomerang/core/Boomerang.h"
#include "boomerang/util/Log.h"

#include "boomerang-gui/ui_LoggingSettingsDlg.h"


LoggingSettingsDlg::LoggingSettingsDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoggingSettingsDlg)
{
    ui->setupUi(this);
    ui->chkVerbose->setChecked(Boomerang::get()->vFlag);
    ui->chkLiveness->setChecked(Boomerang::get()->debugLiveness);
    ui->chkUnused->setChecked(Boomerang::get()->debugUnused);
    ui->chkTypeAnalysis->setChecked(Boomerang::get()->debugTA);
    ui->chkDecoder->setChecked(Boomerang::get()->debugDecoder);
    ui->chkCodegen->setChecked(Boomerang::get()->debugGen);
    ui->chkSwitch->setChecked(Boomerang::get()->debugSwitch);
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
    Boomerang::get()->vFlag         = ui->chkVerbose->isChecked();
    Boomerang::get()->debugTA       = ui->chkTypeAnalysis->isChecked();
    Boomerang::get()->debugLiveness = ui->chkLiveness->isChecked();
    Boomerang::get()->debugUnused   = ui->chkUnused->isChecked();
    Boomerang::get()->debugDecoder  = ui->chkDecoder->isChecked();
    Boomerang::get()->debugGen      = ui->chkCodegen->isChecked();
    Boomerang::get()->debugSwitch   = ui->chkSwitch->isChecked();
}


void LoggingSettingsDlg::on_btnOk_clicked()
{
    on_btnApply_clicked();
    accept();
}
