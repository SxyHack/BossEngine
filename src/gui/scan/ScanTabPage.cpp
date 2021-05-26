#include "ScanTabPage.h"
#include "BEngine.h"

#include <QtDebug>

ScanTabPage::ScanTabPage(QWidget *parent)
	: QWidget(parent)
	, _UniqueID(-1)
{
	ui.setupUi(this);
	_BaseGroup.addButton(ui.Base2,  EBaseMode_2);
	_BaseGroup.addButton(ui.Base10, EBaseMode_10);
	_BaseGroup.addButton(ui.Base16, EBaseMode_16);

	SetupScanValueType();
	SetupScanMethod();

	connect(&_BaseGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(OnButtonBaseChanged(QAbstractButton*)));
	connect(ui.ScanMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxScanMethodChanged(int)));
	connect(ui.ScanValueType, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxScanValueTypeChanged(int)));
	connect(ui.ScanBtn, SIGNAL(clicked()), this, SLOT(OnButtonScan()));
	connect(ui.ScanResetBtn, SIGNAL(clicked()), this, SLOT(OnButtonScanReset()));
	connect(ui.ScanUndoBtn, SIGNAL(clicked()), this, SLOT(OnButtonScanUndo()));

	//
	// UI默认设置
	ui.Base2->setVisible(false);          // 暂不支持2进制
	ui.ScanValueB->setVisible(false);
	ui.ScanValueType->setCurrentIndex(2); // 默认使用4字节
	ui.ScanMethod->setCurrentIndex(0);    // 默认使用精确查找
	ui.ScanValueA->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));
	ui.ScanValueB->setValidator(new QIntValidator(_I32_MIN, _I32_MAX));

	ui.MemoryRngBegin->setText(QString("%1").arg(_Workspace.ScanBegAddress, 16, 16, QLatin1Char('0')).toUpper());
	ui.MemoryRngEnd->setText(QString("%1").arg(_Workspace.ScanEndAddress, 16, 16, QLatin1Char('0')).toUpper());

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
	ui.ScanValueType->addItem("1字节",  EScanValueType_Byte);
	ui.ScanValueType->addItem("2字节",  EScanValueType_2_Byte);
	ui.ScanValueType->addItem("4字节",  EScanValueType_4_Byte);
	ui.ScanValueType->addItem("8字节",  EScanValueType_8_Byte);
	ui.ScanValueType->addItem("单浮点", EScanValueType_Float);
	ui.ScanValueType->addItem("双浮点", EScanValueType_Double);
	ui.ScanValueType->addItem("字符串", EScanValueType_String);
	ui.ScanValueType->addItem("全部",   EScanValueType_All);
}

void ScanTabPage::SetupScanMethod()
{
	ui.ScanMethod->addItem("精确数值",    EScanMethod_Exact);
	ui.ScanMethod->addItem("大于输入值",  EScanMethod_Greater);
	ui.ScanMethod->addItem("小于输入值",  EScanMethod_Less);
	ui.ScanMethod->addItem("介于..两数之间", EScanMethod_Between);
	ui.ScanMethod->addItem("未知初始值",  EScanMethod_Unknown);
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
	_Workspace.ScanValue(ui.ScanValueA->text(), ui.ScanValueB->text());
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
