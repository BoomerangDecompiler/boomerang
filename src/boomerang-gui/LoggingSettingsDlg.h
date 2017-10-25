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


#include <QDialog>

namespace Ui
{
class LoggingSettingsDlg;
}

class LoggingSettingsDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LoggingSettingsDlg(QWidget *parent = nullptr);
    ~LoggingSettingsDlg();

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void on_btnApply_clicked();

    void on_btnOk_clicked();

private:
    Ui::LoggingSettingsDlg *ui;
};
