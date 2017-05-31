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
	explicit LoggingSettingsDlg(QWidget *parent = 0);
	~LoggingSettingsDlg();

protected:
	void changeEvent(QEvent *e);

private slots:
	void on_btnApply_clicked();

	void on_btnOk_clicked();

private:
	Ui::LoggingSettingsDlg *ui;
};
