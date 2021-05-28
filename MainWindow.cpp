#include "MainWindow.h"
#include "gui/process/ProcessAttachDialog.h"
#include "gui/scan/ScanTabPage.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , _ScanProgress(nullptr)
{
    _Windows.EnableDebugPrivilege();

    ui.setupUi(this);

    SetupToolBar();
    SetupStatusBar();
    AddScanPage();
}

void MainWindow::SetupToolBar()
{
    QAction* actOpenProcess = new QAction(QIcon(":/MAIN/resources/Icons/_load128x128.png"), tr("&Open..."), this);
    actOpenProcess->setStatusTip("关联进程");
    actOpenProcess->setToolTip("关联进程");
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
    page->SetUniqueID(ui.tabWidget->addTab(page, "扫描"));

    connect(page, &ScanTabPage::ES_MemoryScanning, this, &MainWindow::OnMemoryScanning, Qt::QueuedConnection);
    connect(page, &ScanTabPage::ES_MemoryScanStarted, this, &MainWindow::OnMemoryScanStarted, Qt::QueuedConnection);
    connect(page, &ScanTabPage::ES_MemoryScanDone, this, &MainWindow::OnMemoryScanDone, Qt::QueuedConnection);
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

    auto currentTab = (ScanTabPage*)ui.tabWidget->currentWidget();
    if (currentTab)
    {
        currentTab->ShowModules();
    }
}

void MainWindow::OnMemoryScanning(qint64 scanned, qint64 total)
{
    _ScanProgress->setMaximum(total);
    _ScanProgress->setMinimum(0);
    _ScanProgress->setValue(scanned);
}

void MainWindow::OnMemoryScanStarted()
{
	_ScanProgress->setVisible(true);
}

void MainWindow::OnMemoryScanDone()
{
	_ScanProgress->setVisible(false);
}
