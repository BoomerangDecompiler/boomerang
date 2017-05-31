#include <QApplication>
#include <QCoreApplication>
#include "mainwindow.h"
#include "commandlinedriver.h"

int main(int argc, char *argv[])
{
	if (argc > 1) {
		QCoreApplication  app(argc, argv);
		CommandlineDriver driver;
		driver.applyCommandline(app.arguments());
		return driver.decompile();
	}
	else {
		QApplication app(argc, argv);
		MainWindow   mainWindow;
		mainWindow.show();
		return app.exec();
	}
}
