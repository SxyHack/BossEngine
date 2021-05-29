#include "ScanTabPage.h"
#include "BEngine.h"
#include "controls/BEHeaderView.h"
#include "utility/JsonMaker.h"

#include <QTableWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QtDebug>

const qint32 QtRoleSystemDll = Qt::UserRole;
const qint32 QtRoleData = Qt::UserRole + 1;


ScanTabPage::ScanTabPage(QWidget* parent)
	: QWidget(parent)
	, _UniqueID(-1)
{
	ui.setupUi(this);
	_BaseGroup.addButton(ui.Base2, EBaseMode_2);
	_BaseGroup.addButton(ui.Base10, EBaseMode_10);
	_BaseGroup.addButton(ui.Base16, EBaseMode_16);

	SetupScanValueType();
	SetupScanMethod();
	SetupModuleTableHead();

	connect(&_BaseGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnButtonBaseChanged(QAbstractButton*)));
	connect(ui.ScanMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxScanMethodChanged(int)));
	connect(ui.ScanValueType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxScanValueTypeChanged(int)));
	connect(ui.ScanBtn, SIGNAL(clicked()), this, SLOT(OnButtonScan()));
	connect(ui.ScanResetBtn, SIGNAL(clicked()), this, SLOT(OnButtonScanReset()));
	connect(ui.ScanUndoBtn, SIGNAL(clicked()), this, SLOT(OnButtonScanUndo()));

	connect(&_Workspace, SIGNAL(ES_MemorySearchStarted()), this, SLOT(OnMemorySearchStarted()), Qt::QueuedConnection);
	connect(&_Workspace, SIGNAL(ES_MemoryScanDone()), this, SLOT(OnMemorySearchDone()), Qt::QueuedConnection);

	connect(&_ScanRefreshTimer, SIGNAL(timeout()), this, SLOT(OnTimeScanRefresh()));

	//
	// UI默认设置
	ui.Base2->setVisible(false);          // 暂不支持2进制
	ui.ScanValueB->setVisible(false);
	ui.ScanValueType->setCurrentIndex(2); // 默认使用4字节
	ui.ScanMethod->setCurrentIndex(0);    // 默认使用精确查找
	ui.ScanValueA->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));
	ui.ScanValueB->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));

	//ui.MemoryRngBegin->setText(QString("%1").arg(_Workspace.ScanBegAddress, 16, 16, QLatin1Char('0')).toUpper());
	//ui.MemoryRngEnd->setText(QString("%1").arg(_Workspace.ScanEndAddress, 16, 16, QLatin1Char('0')).toUpper());

	// 备份
	_InputMask = ui.ScanValueA->inputMask();
}

ScanTabPage::~ScanTabPage()
{
}

void ScanTabPage::SetUniqueID(qint32 uid)
{
	_UniqueID = uid;
	_Workspace.UniqueID = uid;
}

qint32 ScanTabPage::UniqueID()
{
	return _UniqueID;
}

void ScanTabPage::SetupScanValueType()
{
	ui.ScanValueType->addItem("1字节", EScanValueType_Byte);
	ui.ScanValueType->addItem("2字节", EScanValueType_2_Byte);
	ui.ScanValueType->addItem("4字节", EScanValueType_4_Byte);
	ui.ScanValueType->addItem("8字节", EScanValueType_8_Byte);
	ui.ScanValueType->addItem("单浮点", EScanValueType_Float);
	ui.ScanValueType->addItem("双浮点", EScanValueType_Double);
	ui.ScanValueType->addItem("字符串", EScanValueType_String);
	ui.ScanValueType->addItem("全部", EScanValueType_All);
}

void ScanTabPage::SetupScanMethod()
{
	ui.ScanMethod->addItem("精确数值", EScanMethod_Exact);
	ui.ScanMethod->addItem("大于输入值", EScanMethod_Greater);
	ui.ScanMethod->addItem("小于输入值", EScanMethod_Less);
	ui.ScanMethod->addItem("介于..两数之间", EScanMethod_Between);
	ui.ScanMethod->addItem("未知初始值", EScanMethod_Unknown);
}

void ScanTabPage::SetupModuleTableHead()
{
	// 设置表头参数
	QStringList tableHead;
	tableHead << "模块" << "起始" << "结束";

	BEHeaderView* header = new BEHeaderView(this);
	ui.ModuleTable->setHorizontalHeader(header);
	ui.ModuleTable->setColumnCount(tableHead.length());
	ui.ModuleTable->setColumnWidth(0, 150);
	ui.ModuleTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.ModuleTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui.ModuleTable->setAlternatingRowColors(true);

	connect(header, SIGNAL(sgStateChanged(bool)), this, SLOT(OnButtonModuleAllSelect(bool)));

	for (qint32 i = 0; i < tableHead.length(); i++)
	{
		auto headItem = new QTableWidgetItem(tableHead[i]);
		QFont font = headItem->font();
		font.setBold(true);

		headItem->setFont(font);
		headItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
		if (i == 0)
		{
			headItem->setCheckState(Qt::PartiallyChecked);
		}

		ui.ModuleTable->setHorizontalHeaderItem(i, headItem);
	}
}

void ScanTabPage::ShowModules()
{
	WinExtras winExtras;
	auto      modules = Engine.GetModules();
	QString   qsSystemDir = winExtras.GetSystemDir();
	DWORD     dwReverseRow = modules.length() - 1;
	DWORD     dwRow = 0;

	ui.ModuleTable->setRowCount(modules.length());

	for (auto& mod : modules)
	{
		// + 变量定义
		QString qsExeFile = QString::fromWCharArray(mod.szExePath);
		QString qsModName = QString::fromWCharArray(mod.szModule);
		QFileInfo qfi(qsExeFile);

		bool    bSystemDll = true;
		QString qsExePath = QDir::toNativeSeparators(qfi.absolutePath());
		QColor  color = Qt::gray;
		DWORD   dwPos = 0;
		// - 变量定义

		// 判断哪些是应用程序模块, 哪些是系统自带的模块.
		// 主要根据是DLL的路径来进行判断, 位于c:\windows\system的下的文件,
		// 判定为系统DLL
		if (!qsExePath.contains(qsSystemDir, Qt::CaseInsensitive))
		{
			dwPos = dwRow++;
			color = Qt::green;
			bSystemDll = false;
		}
		else
		{
			bSystemDll = true;
			dwPos = dwReverseRow--;
		}

		auto itModName = new QTableWidgetItem(qsModName);
		itModName->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		itModName->setTextColor(color);
		itModName->setToolTip(qsExeFile);
		itModName->setCheckState(bSystemDll ? Qt::Unchecked : Qt::Checked);
		itModName->setData(QtRoleSystemDll, bSystemDll);
		itModName->setData(QtRoleData, JsonMaker::MakeWith(mod));

		ui.ModuleTable->setItem(dwPos, 0, itModName);

		auto qsBaseAddr = QString::number((DWORD64)mod.modBaseAddr, 16).toUpper();
		auto itBaseAddr = new QTableWidgetItem(qsBaseAddr);
		itBaseAddr->setTextColor(color);
		itBaseAddr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

		ui.ModuleTable->setItem(dwPos, 1, itBaseAddr);

		DWORD64 dwEndAddr = (DWORD64)mod.modBaseAddr + mod.modBaseSize;
		auto qsEndAddr = QString::number(dwEndAddr, 16).toUpper();
		auto itEndAddr = new QTableWidgetItem(qsEndAddr);
		itEndAddr->setTextColor(color);
		itEndAddr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);

		ui.ModuleTable->setItem(dwPos, 2, itEndAddr);
	}
}

void ScanTabPage::ChangeToBase16()
{
	QString inputMask;
	switch (_Workspace.ScanValueType)
	{
	case EScanValueType_Byte:
		inputMask = "HH";
		break;
	case EScanValueType_2_Byte:
		inputMask = "HH HH";
		break;
	case EScanValueType_4_Byte:
		inputMask = "HH HH HH HH";
		break;
	case EScanValueType_8_Byte:
		inputMask = "HH HH HH HH HH HH HH HH";
		break;
	}

	// 保存InputMask
	_InputMask = ui.ScanValueA->inputMask();

	auto oldA = ui.ScanValueA->text();
	if (!oldA.isEmpty())
	{
		quint64 ullOldA = oldA.toULongLong();
		QString hexNewA = QString::number(ullOldA, 16);

		ui.ScanValueA->setInputMask(inputMask);
		ui.ScanValueA->setText(hexNewA);
	}

	// 如果B值有显示出来,才进行转化
	if (ui.ScanValueB->isVisible())
	{
		auto oldB = ui.ScanValueB->text();
		if (!oldB.isEmpty())
		{
			quint64 ullOldB = oldB.toULongLong();
			QString hexNewB = QString::number(ullOldB, 16);

			// 要先setInputMask, 这样的话逻辑比较缜密
			ui.ScanValueB->setInputMask(inputMask);
			ui.ScanValueB->setText(hexNewB);
		}
	}
}

void ScanTabPage::ChangeToBase10()
{
	auto oldA = ui.ScanValueA->text().trimmed();
	if (!oldA.isEmpty())
	{
		quint64 ullOldA = oldA.toULongLong();
		ui.ScanValueA->setInputMask(_InputMask);
		ui.ScanValueA->setText(QString::number(ullOldA, 10));
	}

	if (ui.ScanValueB->isVisible())
	{
		auto oldB = ui.ScanValueB->text().trimmed();
		if (!oldB.isEmpty())
		{
			quint64 ullOldB = oldB.toULongLong();
			ui.ScanValueB->setInputMask(_InputMask);
			ui.ScanValueB->setText(QString::number(ullOldB, 10));
		}
	}
}

void ScanTabPage::ChangeToBase2()
{
}

void ScanTabPage::showEvent(QShowEvent* e)
{
}

void ScanTabPage::hideEvent(QHideEvent* e)
{

}

void ScanTabPage::OnButtonScan()
{
	MODULEENTRY32 mod;

	Engine.RemoveIncludeModules();

	for (int i = 0; i < ui.ModuleTable->rowCount(); i++)
	{
		auto item = ui.ModuleTable->item(i, 0);
		if (item->checkState() == Qt::Checked)
		{
			auto& json = item->data(QtRoleData).toJsonObject();
			JsonMaker::ReturnTo(json, mod);
			Engine.IncludeModule(mod);
		}
	}

	_Workspace.SearchMemory(ui.ScanValueA->text(), ui.ScanValueB->text());
}

void ScanTabPage::OnButtonScanReset()
{
	_Workspace.Reset();
}

void ScanTabPage::OnButtonScanUndo()
{

}

void ScanTabPage::OnButtonBaseChanged(QAbstractButton* button)
{
	qDebug() << "进制变动: " << _BaseGroup.checkedId();

	switch (_BaseGroup.checkedId())
	{
	case EBaseMode_10:
		ChangeToBase10();
		break;
	case EBaseMode_16:
		ChangeToBase16();
		break;
	case EBaseMode_2:
		ChangeToBase2();
		break;
	default:
		qWarning("不支持的进制");
		break;
	}
}

void ScanTabPage::OnButtonModuleAllSelect(bool checked)
{
	for (int i = 0; i < ui.ModuleTable->rowCount(); i++)
	{
		auto item = ui.ModuleTable->item(i, 0);
		bool bSystemDLL = item->data(Qt::UserRole).toBool();

		if (checked)
		{
			item->setCheckState(Qt::Checked);
		}
		else
		{
			if (bSystemDLL)
			{
				item->setCheckState(Qt::Unchecked);
			}
		}
	}
}

void ScanTabPage::OnComboBoxScanMethodChanged(int index)
{
	switch (EScanMethod(ui.ScanMethod->currentData().toInt()))
	{
	case EScanMethod_Exact:
		ui.ScanValueB->setVisible(false);
		break;
	case EScanMethod_Greater:
		ui.ScanValueB->setVisible(false);
		break;
	case EScanMethod_Less:
		ui.ScanValueB->setVisible(false);
		break;
	case EScanMethod_Between:
		ui.ScanValueA->setPlaceholderText("大于等于输入数值");
		ui.ScanValueB->setPlaceholderText("小于等于输入数值");
		ui.ScanValueB->setVisible(true);
		break;
	case EScanMethod_Unknown:
		ui.ScanValueA->setVisible(false);
		ui.ScanValueB->setVisible(false);
		break;
	case EScanMethod_MoreValue:
		break;
	case EScanMethod_AddValue:
		break;
	case EScanMethod_Smaller:
		break;
	case EScanMethod_CutValue:
		break;
	case EScanMethod_Changed:
		break;
	case EScanMethod_Equal:
		break;
	case EScanMethod_FirstDiff:
		break;
	default:
		break;
	};
}

void ScanTabPage::OnComboBoxScanValueTypeChanged(int)
{
	QIntValidator validator;
	switch (EScanValueType(ui.ScanValueType->currentData().toInt()))
	{
	case EScanValueType_Byte:
		ui.ScanValueA->setValidator(new QIntValidator(_I8_MIN, _I8_MAX));
		ui.ScanValueB->setValidator(new QIntValidator(_I8_MIN, _I8_MAX));
		break;
	case EScanValueType_2_Byte:
		ui.ScanValueA->setValidator(new QIntValidator(_I16_MIN, _I16_MAX));
		ui.ScanValueB->setValidator(new QIntValidator(_I16_MIN, _I16_MAX));
		break;
	case EScanValueType_4_Byte:
		ui.ScanValueA->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));
		ui.ScanValueB->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));
		break;
	case EScanValueType_8_Byte:
		ui.ScanValueA->setValidator(new QIntValidator(_I64_MIN, _I64_MAX));
		ui.ScanValueB->setValidator(new QIntValidator(_I64_MIN, _I64_MAX));
		break;
	case EScanValueType_Float:
		ui.ScanValueA->setValidator(new QDoubleValidator(FLT_MIN, FLT_MAX, 3));
		ui.ScanValueB->setValidator(new QDoubleValidator(FLT_MIN, FLT_MAX, 3));
		break;
	case EScanValueType_Double:
		ui.ScanValueA->setValidator(new QDoubleValidator(DBL_MIN, DBL_MAX, 6));
		ui.ScanValueB->setValidator(new QDoubleValidator(DBL_MIN, DBL_MAX, 6));
		break;
	case EScanValueType_String:
		ui.ScanValueA->setValidator(nullptr);
		ui.ScanValueB->setValidator(nullptr);
		break;
	case EScanValueType_All:
		ui.ScanValueA->setValidator(nullptr);
		ui.ScanValueB->setValidator(nullptr);
		break;
	default:
		break;
	}
}

void ScanTabPage::OnMemorySearchStarted()
{
	qDebug() << "扫描开始...";
	_ScanRefreshTimer.start(20);
	emit ES_MemoryScanStarted();
}

void ScanTabPage::OnMemorySearchDone()
{
	qDebug() << "扫描结束...";

	_ScanRefreshTimer.stop();
	emit ES_MemoryScanDone();
}

void ScanTabPage::OnTimeScanRefresh()
{
	emit ES_MemoryScanning(_Workspace.NumberOfScannedBytes, _Workspace.NumberOfScanTotalBytes);
}
