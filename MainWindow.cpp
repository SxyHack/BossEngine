#include "MainWindow.h"
#include "gui/process/ProcessAttachDialog.h"
#include "gui/scan/ScanTabPage.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ScanProgress(nullptr)
{
    _Windows.AdjustPrivilege();

    ui.setupUi(this);

    SetupToolBar();
    SetupStatusBar();
    AddScanPage();
}

void MainWindow::SetupToolBar()
{
    QAction* actOpenProcess = new QAction(QIcon(":/MAIN/resources/Icons/_load128x128.png"), tr("&Open..."), this);
    actOpenProcess->setStatusTip("关联进程");
    ui.mainToolBar->addAction(actOpenProcess);
    
    connect(actOpenProcess, SIGNAL(triggered()), this, SLOT(onActionOpenProcess()));
}

void MainWindow::SetupStatusBar()
{
    if (_ScanProgress != nullptr)
        return;

    _ScanProgress = new QProgressBar(this);
    _ScanProgress->setTextVisible(false);
    _ScanProgress->setMaximum(100);
    _ScanProgress->setMinimum(0);
	_ScanProgress->setValue(25);
    _ScanProgress->setVisible(false);

	ui.statusBar->addPermanentWidget(_ScanProgress);
}

void MainWindow::AddScanPage()
{
    ScanTabPage* page = new ScanTabPage(this);
    auto id = ui.tabWidget->addTab(page, "扫描");
    page->SetUniqueID(id);
}

void MainWindow::onActionOpenProcess()
{
    ProcessAttachDialog dialog;
    
    if (dialog.exec() == QDialog::Rejected) {
        return;
	}
    
    DWORD dwSelectPID = dialog.GetSelectedProcessID();
    qDebug("Select PID: %d", dwSelectPID);
	if (!Engine.OpenProcess(dwSelectPID)) 
    {
        QMessageBox::critical(this, "错误", Engine.GetLastErrorMessage(), QMessageBox::Ok);
        return;
	}
}
