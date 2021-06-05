#include "ScanTabPage.h"
#include "BEngine.h"
#include "controls/BEHeaderView.h"
#include "utility/JsonMaker.h"

#include <QTableWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QClipboard>
#include <QtDebug>
#include <QMessageBox>

// Module Table
#define MODULE_SYSTEM_DLL Qt::UserRole
#define MODULE_DATA       Qt::UserRole + 1

// Found Table
#define FOUND_PTR_ADDRESS Qt::UserRole
#define FOUND_PTR_VALUE   Qt::UserRole + 1
#define FOUND_PTR         Qt::UserRole + 2
#define FOUND_PTR_CHANGED Qt::UserRole + 3

ScanTabPage::ScanTabPage(QWidget* parent)
	: QWidget(parent)
	, _UniqueID(-1)
	, _IsBase(true)
	, _MenuFoundTable(this)
{
	ui.setupUi(this);

	_BaseGroup.addButton(ui.Base2, EBaseMode_2);
	_BaseGroup.addButton(ui.Base10, EBaseMode_10);
	_BaseGroup.addButton(ui.Base16, EBaseMode_16);

	SetupScanValueType();
	SetupScanMethod();
	SetupModuleTableHead();
	SetupFoundTableHead();
	SetupFoundTableMenu();

	connect(&_BaseGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnButtonBaseChanged(QAbstractButton*)));
	connect(ui.ScanMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxScanMethodChanged(int)));
	connect(ui.ScanValueType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxScanValueTypeChanged(int)));
	connect(ui.ScanBtn, SIGNAL(clicked()), this, SLOT(OnButtonScan()));
	connect(ui.ScanResetBtn, SIGNAL(clicked()), this, SLOT(OnButtonScanReset()));
	connect(ui.ScanUndoBtn, SIGNAL(clicked()), this, SLOT(OnButtonScanUndo()));
	connect(ui.FoundTable, &QTableWidget::customContextMenuRequested, this, &ScanTabPage::OnMenuFoundTable);

	connect(&_Workspace, &BEWorkspace::ES_MemorySearchStarted, this, &ScanTabPage::OnMemorySearchStarted, Qt::QueuedConnection);
	connect(&_Workspace, &BEWorkspace::ES_MemoryScanDone, this, &ScanTabPage::OnMemorySearchDone, Qt::QueuedConnection);
	connect(&_Workspace, &BEWorkspace::ES_FoundPointer, this, &ScanTabPage::OnFoundPointer, Qt::QueuedConnection);

	connect(&_ScanningTimer, &QTimer::timeout, this, &ScanTabPage::OnTimeProcScanning);
	connect(&_UpdateTimer,   &QTimer::timeout, this, &ScanTabPage::OnTimeProcUpdateResults);

	//
	// UI默认设置
	ui.Base2->setVisible(false);          // 暂不支持2进制
	ui.ScanValueType->setCurrentIndex(2); // 默认使用4字节
	ui.ScanMethod->setCurrentIndex(0);    // 默认使用精确查找
	ui.ScanValueA->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));
	ui.ScanValueB->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));
	ui.ScanValueB->setVisible(false);     // 

	ui.ScanBtn->setEnabled(false);
	ui.ScanResetBtn->setEnabled(false);
	ui.ScanUndoBtn->setEnabled(false);

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
	ui.ScanMethod->addItem("精确数值", ECompareMethod_Exact);
	ui.ScanMethod->addItem("大于输入值", ECompareMethod_MoreThan);
	ui.ScanMethod->addItem("小于输入值", ECompareMethod_LessThan);
	ui.ScanMethod->addItem("介于..两数之间", ECompareMethod_Between);
	ui.ScanMethod->addItem("未知初始值", ECompareMethod_Unknown);
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

void ScanTabPage::SetupFoundTableHead()
{
	QStringList tableHead;
	tableHead << "地址" << "当前值" << "先前值";
	ui.FoundTable->setColumnCount(tableHead.length());
	ui.FoundTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.FoundTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui.FoundTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui.FoundTable->setAlternatingRowColors(true);

	for (qint32 i = 0; i < tableHead.length(); i++)
	{
		auto headItem = new QTableWidgetItem(tableHead[i]);
		headItem->setTextColor(Qt::lightGray);
		headItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		ui.FoundTable->setHorizontalHeaderItem(i, headItem);
	}
}

void ScanTabPage::SetupFoundTableMenu()
{
	ui.FoundTable->setContextMenuPolicy(Qt::CustomContextMenu);

	QAction* pAction = nullptr;

	pAction = _MenuFoundTable.addAction(QIcon(":/MAIN/resources/Icons/copy128x128.png"), "复制地址");
	connect(pAction, &QAction::triggered, this, &ScanTabPage::OnActionFoundTableCopyAddress);
	_MenuFoundTable.addSeparator();
	
	_MenuFoundTable.addAction(QIcon(":/MAIN/resources/Icons/x128x128.png"), "删除条目");
	
	_MenuFoundTable.addSeparator();
	
	_MenuFoundTable.addAction("显示反汇编");
	_MenuFoundTable.addAction("显示内存");
	
	_MenuFoundTable.addSeparator();

	pAction = _MenuFoundTable.addAction("16进制");
	pAction->setCheckable(true);

	pAction = _MenuFoundTable.addAction("10进制");
	pAction->setCheckable(true);
	pAction->setChecked(true);
}

void ScanTabPage::AppendFoundPointer(BEPointer* pPtr)
{
	QString moduleName;
	auto ulOffset = Engine.QueryStaticAddress(pPtr->Address, moduleName);

	auto qsAddr = ulOffset < 0 
		? QString::number(pPtr->Address, 16).toUpper()
		: QString("%1+%2").arg(moduleName).arg(QString::number(ulOffset, 16).toUpper());
	auto itemAddr = new QTableWidgetItem(qsAddr);
	itemAddr->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	itemAddr->setTextColor(ulOffset >= 0 ? Qt::darkGreen : Qt::white);
	itemAddr->setData(FOUND_PTR_ADDRESS, pPtr->Address);
	ui.FoundTable->setItem(pPtr->Index, 0, itemAddr);

	auto qsValue = pPtr->Value.toString();
	auto itemValue = new QTableWidgetItem(qsValue);
	itemValue->setData(FOUND_PTR_ADDRESS, pPtr->Address);
	itemValue->setData(FOUND_PTR_VALUE, pPtr->Value);
	ui.FoundTable->setItem(pPtr->Index, 1, itemValue);

	auto itemCopy = new QTableWidgetItem(qsValue);
	itemCopy->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	ui.FoundTable->setItem(pPtr->Index, 2, itemCopy);
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
		itModName->setData(MODULE_SYSTEM_DLL, bSystemDll);
		itModName->setData(MODULE_DATA, JsonMaker::MakeWith(mod));

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

void ScanTabPage::ShowUiStateOpenProcess()
{
	ui.ScanBtn->setEnabled(true);
}

void ScanTabPage::ShowUiStateNext()
{
	if (_IsBase)
	{
		_IsBase = false;
		ui.FrameMemory->setEnabled(false);
		ui.ScanValueType->setEnabled(false);

		auto lastIndex = ui.ScanMethod->count() - 1;
		ui.ScanMethod->removeItem(lastIndex);
		ui.ScanMethod->addItem("增加的数值", ECompareMethod_Bigger);
		ui.ScanMethod->addItem("减少的数值", ECompareMethod_Smaller);
		ui.ScanMethod->addItem("数值增加了N", ECompareMethod_IncreaseN);
		ui.ScanMethod->addItem("数值减少了N", ECompareMethod_DecreaseN);
		ui.ScanMethod->addItem("变动的数值", ECompareMethod_Changed);
		ui.ScanMethod->addItem("没变动的数值", ECompareMethod_Unchanged);
		ui.ScanMethod->addItem("对比首次扫描", ECompareMethod_EqualBase);
	}
}

void ScanTabPage::ShowUiStateInitial()
{
	_IsBase = true;
	_Workspace.Reset();

	ui.ScanBtn->setEnabled(true);
	ui.ScanResetBtn->setEnabled(false);
	ui.ScanUndoBtn->setEnabled(false);
	ui.ScanValueType->setEnabled(true);
	ui.FrameMemory->setEnabled(true);

	ui.FoundTable->clearContents();
	ui.FoundTable->setRowCount(0);
	ui.ScanValueA->clear();
	ui.ScanValueB->clear();
	ui.ScanMethod->clear();

	SetupScanMethod();
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
	auto method = _Workspace.CompareMethod;
	auto qsValueA = ui.ScanValueA->text();
	auto qsValueB = ui.ScanValueB->text();

	if (qsValueA.isEmpty() || (method == ECompareMethod_Between && qsValueB.isEmpty())) {
		QMessageBox::warning(this, "提示", "搜索值不能为空");
		return;
	}

	MODULEENTRY32 mod;
	ui.ScanResetBtn->setEnabled(true);
	ui.ScanUndoBtn->setEnabled(true);
	ui.ScanValueType->setEnabled(false);

	Engine.RemoveIncludeModules();

	for (int i = 0; i < ui.ModuleTable->rowCount(); i++)
	{
		auto item = ui.ModuleTable->item(i, 0);
		if (item->checkState() == Qt::Checked)
		{
			auto& json = item->data(MODULE_DATA).toJsonObject();
			JsonMaker::ReturnTo(json, mod);
			Engine.IncludeModule(mod);
		}
	}

	_Workspace.ScanValueInMemory(ui.ScanValueA->text(), ui.ScanValueB->text());
}

void ScanTabPage::OnButtonScanReset()
{
	_Workspace.Reset();

	ShowUiStateInitial();
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

void ScanTabPage::OnMenuFoundTable(const QPoint& pos)
{
	auto gPos = ui.FoundTable->mapToGlobal(pos);
	auto offset = QPoint(6, 20);

	auto item = ui.FoundTable->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	_MenuFoundTable.move(gPos + offset);
	_MenuFoundTable.show();
}

void ScanTabPage::OnActionFoundTableCopyAddress(bool checked)
{
	auto clip = QApplication::clipboard();
	if (clip == nullptr)
		return;
	
	auto items = ui.FoundTable->selectedItems();
	if (items.isEmpty())
		return;

	auto itemSelected = items.at(0);
	auto itemText = itemSelected->text();
	qDebug() << "复制的内容:" << itemText;
	clip->setText(itemText);
}

void ScanTabPage::OnComboBoxScanMethodChanged(int index)
{
	auto method = ECompareMethod(ui.ScanMethod->currentData().toInt());
	_Workspace.CompareMethod = method;

	switch (method)
	{
	case ECompareMethod_Exact:
		ui.ScanValueB->setVisible(false);
		break;
	case ECompareMethod_MoreThan:
		ui.ScanValueB->setVisible(false);
		break;
	case ECompareMethod_LessThan:
		ui.ScanValueB->setVisible(false);
		break;
	case ECompareMethod_Between:
		ui.ScanValueA->setPlaceholderText("大于等于输入数值");
		ui.ScanValueB->setPlaceholderText("小于等于输入数值");
		ui.ScanValueB->setVisible(true);
		break;
	case ECompareMethod_Unknown:
		ui.ScanValueA->setVisible(false);
		ui.ScanValueB->setVisible(false);
		break;
	case ECompareMethod_Bigger:
		break;
	case ECompareMethod_IncreaseN:
		break;
	case ECompareMethod_Smaller:
		break;
	case ECompareMethod_DecreaseN:
		break;
	case ECompareMethod_Changed:
		break;
	case ECompareMethod_Unchanged:
		break;
	case ECompareMethod_EqualBase:
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
	// 清空数据
	ui.FoundTable->clearContents();

	_UpdateTimer.stop();
	_ScanningTimer.start(20);
	emit ES_MemoryScanStarted();
}

void ScanTabPage::OnMemorySearchDone(qint64 count)
{
	_ScanningTimer.stop();

	if (count > 0)
	{
		ShowUiStateNext();
		_UpdateTimer.start(1000);
	}
	else
	{
		ui.FoundTable->setRowCount(0);
	}

	emit ES_MemoryScanDone(count);
}

void ScanTabPage::OnFoundPointer(BEPointer* pValue, qint64 count)
{
	ui.FoundTable->setRowCount(count);

	AppendFoundPointer(pValue);
}

void ScanTabPage::OnTimeProcScanning()
{
	auto count = _Workspace.GetPointerCount();
	emit ES_MemoryFoundValue(count);
	emit ES_MemoryScanning(_Workspace.NumberOfScannedBytes, _Workspace.NumberOfScanTotalBytes);
}

void ScanTabPage::OnTimeProcUpdateResults()
{
	for (int i = 0; i < ui.FoundTable->rowCount(); i++)
	{
		auto itemValue = ui.FoundTable->item(i, 1);
		if (itemValue == nullptr)
			break;

		QVariant newValue;

		auto address = itemValue->data(FOUND_PTR_ADDRESS).toULongLong();
		auto value = itemValue->data(FOUND_PTR_VALUE);
		if (!_Workspace.ReadMemory(address, newValue))
		{
			itemValue->setText("???");
			itemValue->setBackground(QBrush(Qt::red));
			continue;
		}

		itemValue->setText(newValue.toString());

		if (value != newValue)
		{
			itemValue->setBackground(QBrush(Qt::red));
		}
		else
		{
			itemValue->setBackground(QBrush());
		}
	}

	auto count = _Workspace.GetPointerCount();
	emit ES_MemoryFoundValue(count);
}
