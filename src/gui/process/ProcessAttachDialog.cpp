#include "ProcessAttachDialog.h"

ProcessAttachDialog::ProcessAttachDialog(QWidget* parent)
	: QDialog(parent)
	, _SelectProcessID(0)
{
	ui.setupUi(this);

	connect(ui.NameFilter, SIGNAL(textChanged(const QString&)), this, SLOT(OnNameFilterChanged(const QString&)));
	connect(ui.OpenBtn, SIGNAL(clicked()), this, SLOT(OnClickOpenProcess()));
	connect(ui.CancelBtn, SIGNAL(clicked()), this, SLOT(OnClickCancel()));

	SetupProcessTableHead();
	SetupProcessCategory();
}

ProcessAttachDialog::~ProcessAttachDialog()
{
}

DWORD ProcessAttachDialog::GetSelectedProcessID()
{
	return _SelectProcessID;
}

void ProcessAttachDialog::SetupProcessCategory()
{
	ui.TypeCombo->addItem("所有", 0);
	ui.TypeCombo->addItem("应用程序", 1);
	ui.TypeCombo->addItem("窗口", 2);
}

void ProcessAttachDialog::SetupProcessTableHead()
{
	// 设置表头参数
	QStringList tableHead;
	tableHead << "PID" << "进程名称";

	ui.ProcessList->setColumnCount(tableHead.length());
	ui.ProcessList->setColumnWidth(0, 100);
	ui.ProcessList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.ProcessList->setColumnWidth(2, 50);
	ui.ProcessList->setAlternatingRowColors(true);

	for (qint32 i = 0; i < tableHead.length(); i++)
	{
		QTableWidgetItem* headItem = new QTableWidgetItem(tableHead[i]);
		QFont font = headItem->font();
		font.setBold(true);

		//headItem->setTextColor(Qt::green);
		headItem->setFont(font);
		ui.ProcessList->setHorizontalHeaderItem(i, headItem);
	}
}

void ProcessAttachDialog::ShowProcessTable(const QString& filter)
{
	Engine.EnumProcess(filter);
}

void ProcessAttachDialog::ShowProcessCategory()
{
	ui.TypeCombo->setCurrentIndex(0);
}

void ProcessAttachDialog::showEvent(QShowEvent* e)
{
	//ShowProcessTable();
	connect(&Engine, SIGNAL(sgEnumProcessPrepare()), this, SLOT(OnEnumProcessPrepare()));
	connect(&Engine, SIGNAL(sgEnumProcess(qint32, PROCESSENTRY32)), this, SLOT(OnEnumProcess(qint32, PROCESSENTRY32)));
	connect(&Engine, SIGNAL(sgEnumProcessDone(qint32)), this, SLOT(OnEnumProcessDone(qint32)));

	ShowProcessTable();
}

void ProcessAttachDialog::hideEvent(QHideEvent* e)
{
	disconnect(&Engine, SIGNAL(sgEnumProcessPrepare()), this, SLOT(OnEnumProcessPrepare()));
	disconnect(&Engine, SIGNAL(sgEnumProcess(qint32, PROCESSENTRY32)), this, SLOT(OnEnumProcess(qint32, PROCESSENTRY32)));
	disconnect(&Engine, SIGNAL(sgEnumProcessDone(qint32)), this, SLOT(OnEnumProcessDone(qint32)));
}

void ProcessAttachDialog::OnEnumProcessPrepare()
{
	ui.ProcessList->clearContents(); // 只清除工作区,不清除表头
	ui.ProcessList->setRowCount(2048);
}

void ProcessAttachDialog::OnEnumProcess(qint32 dwRow, PROCESSENTRY32 processEntry32)
{
	QString processID = QString("%1(0x%2)")
		.arg(QString::number(processEntry32.th32ProcessID))
		.arg(QString::number(processEntry32.th32ProcessID, 16));
	QString processName = QString::fromWCharArray(processEntry32.szExeFile);

	auto item0 = new QTableWidgetItem(processID);
	item0->setTextAlignment(Qt::AlignCenter);
	item0->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item0->setData(Qt::UserRole, (qint64)processEntry32.th32ProcessID);

	auto item1 = new QTableWidgetItem(processName);
	item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item1->setData(Qt::UserRole, (qint64)processEntry32.th32ProcessID);

	QPixmap icon;
	if (_Windows.GetProcessICON(processEntry32.th32ProcessID, icon))
	{
		item1->setIcon(icon);
	}

	ui.ProcessList->setItem(dwRow, 0, item0);
	ui.ProcessList->setItem(dwRow, 1, item1);
}

void ProcessAttachDialog::OnEnumProcessDone(qint32 dwRow)
{
	ui.ProcessList->setRowCount(dwRow);
	ui.ProcessList->scrollToBottom();
}

void ProcessAttachDialog::OnNameFilterChanged(const QString& text)
{
	ShowProcessTable(text);
}

void ProcessAttachDialog::OnClickOpenProcess()
{
	auto current = ui.ProcessList->currentItem();
	if (current == nullptr)
		return;

	_SelectProcessID = current->data(Qt::UserRole).toLongLong();
	accept();
}

void ProcessAttachDialog::OnClickCancel()
{
	reject();
}
