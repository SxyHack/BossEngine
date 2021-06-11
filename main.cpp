#include "MainWindow.h"
#include "DarkStyle.h"
#include "framelesswindow.h"
#include "MainWindow.h"
#include "log/CustomLogMessageHandler.h"
#include "utility/Random.h"

#include <QtDebug>
#include <QtWidgets/QApplication>

#pragma warning(disable: 4828)

const QString Welcome = R"(
 ######  #######  #####   #####     ####### #     #  #####  ### #     # ####### 
 #     # #     # #     # #     #    #       ##    # #     #  #  ##    # #       
 #     # #     # #       #          #       # #   # #        #  # #   # #       
 ######  #     #  #####   #####     #####   #  #  # #  ####  #  #  #  # #####   
 #     # #     #       #       #    #       #   # # #     #  #  #   # # #       
 #     # #     # #     # #     #    #       #    ## #     #  #  #    ## #       
 ######  #######  #####   #####     ####### #     #  #####  ### #     # ####### 
)";

int main(int argc, char *argv[])
{
	CustomLogMessageHandler::Instance();
	qInstallMessageHandler(CustomLogMessageHandler::handle);

	qDebug() << Welcome.toUtf8().data();

	QApplication a(argc, argv);
	QApplication::setStyle(new DarkStyle);
	QApplication::setPalette(QApplication::style()->standardPalette());

	// create frameless window (and set windowState or title)
	FramelessWindow framelessWindow;
	//framelessWindow.setWindowState(Qt::WindowFullScreen);
	framelessWindow.setWindowTitle(Random::GenerateString(16));
	framelessWindow.setMinimumWidth(1024);
	framelessWindow.setMinimumHeight(1000);
	framelessWindow.setWindowIcon(a.style()->standardIcon(QStyle::SP_DesktopIcon));

	MainWindow* mainWindow = new MainWindow;
	mainWindow->setWindowTitle(Random::GenerateString(32));
	mainWindow->show();
	// add the mainwindow to our custom frameless window
	//framelessWindow.setContent(mainWindow);
	//framelessWindow.showNormal();
	return a.exec();
}
