#include "MainWindow.h"
#include "gui/process/ProcessAttachDialog.h"
#include "gui/scan/ScanTabPage.h"
#include "gui/config/ConfigurationDialog.h"

#include <QMessageBox>
#include <QCloseEvent>

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
    QAction* actOpenProcess = new QAction(QIcon(":/MAIN/resources/Icons/openprocess128x128.png"), tr("&Open..."), this);
    actOpenProcess->setStatusTip("关联进程");
    actOpenProcess->setToolTip("关联进程");
    ui.mainToolBar->addAction(actOpenProcess);
    connect(actOpenProcess, &QAction::triggered, this, &MainWindow::OnActionOpenProcess);

    // 配置
    QAction* actConfig = new QAction(QIcon(":/MAIN/resources/Icons/gear128x128.png"), tr("&Settings"), this);
    actConfig->setStatusTip("配置");
    actConfig->setToolTip("配置");
    ui.mainToolBar->addAction(actConfig);
    connect(actConfig, &QAction::triggered, this, &MainWindow::OnActionOpenConfig);
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
    connect(page, &ScanTabPage::ES_MemoryFoundValue, this, &MainWindow::OnMemoryFoundValue, Qt::QueuedConnection);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    Engine.CloseProcess();
    QMainWindow::closeEvent(event);
}

void MainWindow::OnActionOpenProcess(bool checked)
{
    ProcessAttachDialog dialog;
    
    if (dialog.exec() == QDialog::Rejected) {
        return;
	}
    
    DWORD dwSelectPID = dialog.GetSelectedProcessID();
    QString qsProcessName = dialog.GetSelectedProcessName();
	if (!Engine.OpenProcess(dwSelectPID)) 
    {
        QMessageBox::critical(this, "错误", Engine.GetLastErrorMessage(), QMessageBox::Ok);
        return;
	}

	ui.UIProcessName->setVisible(true);
    ui.UIProcessName->setText(QString("%1-%2").arg(qsProcessName).arg(dwSelectPID));

    auto currentTab = (ScanTabPage*)ui.tabWidget->currentWidget();
    if (currentTab)
    {
        currentTab->ShowModules();
        currentTab->ShowUiStateInitial();
    }
}

void MainWindow::OnActionOpenConfig(bool checked)
{
    ConfigurationDialog dialog;
    dialog.exec();
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

void MainWindow::OnMemoryFoundValue(qint64 count)
{
    ui.ScanCount->setText(QString::number(count));
}
