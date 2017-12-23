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

class Decompiler;

namespace Ui
{
class SettingsDlg;
}

class SettingsDlg : public QDialog
{
    Q_OBJECT

public:
    SettingsDlg(Decompiler *decompiler, QWidget *parent = nullptr);
    ~SettingsDlg();

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void on_btnApply_clicked();

    void on_btnOk_clicked();

private:
    Ui::SettingsDlg *ui;
};
